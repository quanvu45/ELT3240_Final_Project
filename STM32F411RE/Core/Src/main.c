#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
#include "uart.h"
#include "rgb.h"
#include "i2c.h"
#include "lcd.h"
#include <math.h>

#define VREF   3.3f
#define ADC_MAX 4095.0f

float MQ2_ConvertPPM(uint16_t adc_value) {
    float voltage = (adc_value * 3.3f) / 4095.0f;
    float Rs = (3.3f - voltage) * 10.0f / voltage; // RL = 10k
    float R0 = 92.8f;  // Đặt theo giá trị đo thực tế
    float ratio = Rs / R0;
    float ppm = 400.0f * powf(ratio, -1.5f);  // Thử với -1.4f hoặc -1.3f nếu cần hạ nhạy
    return ppm;
}




void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void TIM4_IRQHandler(void);
volatile uint32_t millis = 0;
uint32_t last_adc_time = - 10;
char msg[40];
typedef enum {
    SYS_IDLE = 0,
    SYS_ACTIVE = 1
} SystemState;
SystemState sys_state = SYS_ACTIVE;

typedef enum {
    GAS_NONE = 0,        // 0 – Không có khí
    GAS_LOW = 1,             // 1 – Nồng độ thấp
    GAS_HIGH = 2,            // 2 – Nồng độ cao
    GAS_DANGEROUS = 3        // 3 – Nguy hiểm
} GasState;
GasState detect_gas_level(uint32_t adc) {
	    if (adc < 400) return GAS_NONE;
	    else if (adc < 500) return GAS_LOW;
	    else if (adc < 700) return GAS_HIGH;
	    else return GAS_DANGEROUS;
}
static uint32_t last_toggle = 0;
static uint8_t toggle = 0;

void handle_gas_state(GasState state, SystemState sys,uint32_t x) {
    static GasState prev_state = -1;
    static SystemState prev = -1;

    if ((state != prev_state) || (sys != prev)) {
        lcd_set_cursor(1, 0);

        if (sys == SYS_IDLE) {
            RGB_SetPWM(0, 255, 0);  // LED xanh lá
            lcd_send_string("Idle ");
            // Tắt hết thiết bị: Quạt PB13, Van PA8, Còi PB10
            GPIOB->BSRR = (1 << 13);          // PB13 HIGH → Quạt TẮT
            GPIOA->BSRR = (1 << 8);           // PA8 HIGH  → Van TẮT
            GPIOB->BSRR = (1 << (10 + 16));   // PB10 LOW  → Còi TẮT
        } else {
            switch (state) {
                case GAS_NONE:
                    RGB_SetPWM(0, 0, 255);  // Xanh dương
                    lcd_send_string("0     ");
                    GPIOB->BSRR = (1 << 13);         // Quạt TẮT
                    GPIOA->BSRR = (1 << 8 );   // Van BẬT
                    GPIOB->BSRR = (1 << (10 + 16));  // Còi TẮT
                    break;

                case GAS_LOW:
                    RGB_SetPWM(255, 255, 0); // Vàng
                    lcd_send_string("1     ");
                    GPIOB->BSRR = (1 << (13 + 16));  // Quạt BẬT
                    GPIOA->BSRR = (1 << 8);          // Van TẮT
                    GPIOB->BSRR = (1 << (10 + 16));  // Còi TẮT
                    break;

                case GAS_HIGH:
            	    sprintf(msg, "%lu\r\n", x);
            	    USART1_SendString(msg);
                    last_toggle = millis;
                    toggle = 1;
                    RGB_SetPWM(255, 0, 0);
                    lcd_send_string("2     ");
                    GPIOB->BSRR = (1 << (13 + 16));  // Quạt BẬT
                    GPIOA->BSRR = (1 << (8 + 16));   // Van BẬT
                    GPIOB->BSRR = (1 << 10);         // Còi BẬT
                    break;

                case GAS_DANGEROUS:
            	    sprintf(msg, "%lu\r\n", x);
            	    USART1_SendString(msg);
                    last_toggle = millis;
                    toggle = 1;
                    RGB_SetPWM(255, 0, 0);
                    lcd_send_string("3     ");
                    GPIOB->BSRR = (1 << (13 + 16));  // Quạt BẬT
                    GPIOA->BSRR = (1 << (8 + 16));   // Van BẬT
                    GPIOB->BSRR = (1 << 10);         // Còi BẬT
                    break;
            }
        }

        prev_state = state;
        prev = sys;
    }

    // Nháy LED đỏ nếu cần
    if (sys == SYS_ACTIVE) {
        if (state == GAS_HIGH && millis - last_toggle >= 500) {
            toggle ^= 1;
            RGB_SetPWM(toggle * 255, 0, 0);
            last_toggle = millis;
        }

        if (state == GAS_DANGEROUS) {
            int min_delay = 10;
            int max_delay = 200;
            if (x < 700) x = 700;
            if (x > 1023) x = 1023;

            int dynamic_delay = max_delay - ((x - 700) * (max_delay - min_delay)) / (1023 - 700);

            if (millis - last_toggle >= dynamic_delay) {
                toggle ^= 1;
                RGB_SetPWM(toggle * 255, 0, 0);
                last_toggle = millis;
            }
        }
    }
}



// ===== MAIN =====
int main(void) {
    SystemCoreClockUpdate();
    USART1_Init();
    USART2_Init();
    GPIO_Init();         // PC1 - ADC
    Buttons_Init();      // SW1, SW2
    ADC_Init();
    I2C1_Init();
    TIM4_Init();
    RGB_PWM_Init();      // PA5, PA6, PA7 dùng PWM cho LED RGB
//  TIM3_Init();         // Nếu dùng millis để nhấp nháy, bật lại
    USART1_SendString("Hello STM32 + I2C Scan\r\n");
    I2C_Scan();
    lcd_init();


    uint32_t adc_value;

    while (1) {
    	if (sys_state == SYS_ACTIVE) {
    	    if ((millis - last_adc_time) >=1000) {
    	        last_adc_time = millis;
        		lcd_set_cursor(0,0);
        		lcd_send_string("Gas:");
            	lcd_set_cursor(0, 15);
            	lcd_send_string("1");
        	    ADC1->CR2 |= ADC_CR2_SWSTART;
        	    while (!(ADC1->SR & ADC_SR_EOC));
        	    adc_value = ADC1->DR;
        	    float gas_ppm_f = MQ2_ConvertPPM(adc_value);         // Giá trị thực dạng float
        	    adc_value = (int)(gas_ppm_f + 0.5f);               // Làm tròn về int

        	    sprintf(msg, "%lu\r\n", adc_value);                     // In ra UART
        	    USART2_SendString(msg);

        	    // Gửi UART + LCD

        	    sprintf(msg, "%lu", adc_value);
        	    lcd_set_cursor(0, 5);
        	    lcd_send_string("    ");
        	    lcd_set_cursor(0, 5);
        	    lcd_send_string(msg);
        	    lcd_set_cursor(0, 10);
        	    lcd_send_string("ppm");
    	    }
    	    // Máy trạng thái
    	    GasState current = detect_gas_level(adc_value);
    	    handle_gas_state(current,sys_state,adc_value);
    	} else {
    	    RGB_SetPWM(0, 255, 0); // Xanh lá
    		lcd_set_cursor(0,0);
    		lcd_send_string("Gas:");
    		lcd_set_cursor(0,15);
    		lcd_send_string("0");
    	}
    }
    while (1);
}



void delay(uint32_t ms) {
    uint32_t start = millis;
    while ((millis - start) < ms);
}

void EXTI0_IRQHandler(void) {
    if (EXTI->PR & (1 << 0)) {
        EXTI->PR |= (1 << 0);
        if (sys_state){
        	sys_state = SYS_IDLE;
        }else{
        	sys_state = SYS_ACTIVE;
        }
    }
}


void EXTI1_IRQHandler(void) {
    if (EXTI->PR & (1 << 1)) {
        EXTI->PR |= (1 << 1); // Clear pending bit
        NVIC_SystemReset();
    }
}

void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {
        TIM4->SR &= ~TIM_SR_UIF;
        millis++;
    }
}

