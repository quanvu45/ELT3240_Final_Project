/*
 * rgb.c
 *
 *  Created on: Jun 18, 2025
 *      Author: DELL
 */


#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "rgb.h"
void RGB_PWM_Init(void) {
	RCC->AHB1ENR &= ~(1 << 0);                    // Clear GPIOA clock enable (bit 0)
	RCC->AHB1ENR |= (1 << 0);                     // Enable GPIOA clock

	RCC->APB1ENR &= ~((1 << 0) | (1 << 1));       // Clear TIM2 and TIM3 clock enable (bits 0, 1)
	RCC->APB1ENR |= ((1 << 0) | (1 << 1));        // Enable TIM2 and TIM3 clock

    GPIOA->MODER &= ~((3 << (5*2)) | (3 << (6*2)) | (3 << (7*2)));
    GPIOA->MODER |=  ((2 << (5*2)) | (2 << (6*2)) | (2 << (7*2))); // AF mode

    GPIOA->AFR[0] &= ~((0xF << (5*4)) | (0xF << (6*4)) | (0xF << (7*4)));
    GPIOA->AFR[0] |=  ((1 << (5*4)) | (2 << (6*4)) | (2 << (7*4))); // AF1, AF2, AF2

    // TIM2 - Red
    TIM2->PSC = 16 - 1;
    TIM2->ARR = 255;
    TIM2->CCMR1 &= ~((0x7 << 4) | (1 << 3));      // Xóa OC1M[2:0] và OC1PE (bits 4-6, 3)
    TIM2->CCMR1 |= ((6 << 4) | (1 << 3));         // Set OC1M = 110 (PWM mode 1) và OC1PE

    TIM2->CCER &= ~(1 << 0);                      // Xóa CC1E (bit 0)
    TIM2->CCER |= (1 << 0);                       // Kích hoạt CC1E (capture/compare output enable)

    TIM2->CR1 &= ~((1 << 7) | (1 << 0));          // Xóa ARPE và CEN (bits 7, 0)
    TIM2->CR1 |= ((1 << 7) | (1 << 0));           // Bật ARPE và CEN (auto-reload preload và counter enable)

    // TIM3 - Green & Blue
    TIM3->PSC = 16 - 1;
	TIM3->ARR = 255;
	TIM3->CCMR1 &= ~((0x7 << 4) | (1 << 3));       // Xóa OC1M[2:0] và OC1PE (bits 4-6, 3) cho CH1
	TIM3->CCMR1 |= ((6 << 4) | (1 << 3));          // Set OC1M = 110 (PWM mode 1) và OC1PE cho CH1

	TIM3->CCMR1 &= ~((0x7 << 12) | (1 << 11));     // Xóa OC2M[2:0] và OC2PE (bits 12-14, 11) cho CH2
	TIM3->CCMR1 |= ((6 << 12) | (1 << 11));        // Set OC2M = 110 (PWM mode 1) và OC2PE cho CH2

	TIM3->CCER &= ~((1 << 0) | (1 << 4));          // Xóa CC1E và CC2E (bits 0, 4)
	TIM3->CCER |= ((1 << 0) | (1 << 4));           // Kích hoạt CC1E và CC2E (capture/compare output enable)

	TIM3->CR1 &= ~((1 << 7) | (1 << 0));           // Xóa ARPE và CEN (bits 7, 0)
	TIM3->CR1 |= ((1 << 7) | (1 << 0));            // Bật ARPE và CEN (auto-reload preload và counter enable)
}

void RGB_SetPWM(uint8_t r, uint8_t g, uint8_t b) {
    // LED anode chung nên phải đảo
    TIM2->CCR1 = 255 - r;
    TIM3->CCR1 = 255 - g;
    TIM3->CCR2 = 255 - b;
}

