/*
 * gpio.c
 *
 *  Created on: Jun 18, 2025
 *      Author: DELL
 */

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
void TIM4_Init(void) {
	RCC->APB1ENR &= ~(1 << 2);                    // Clear TIM4 clock enable (bit 2)
	RCC->APB1ENR |= (1 << 2);                     // Enable TIM4 clock    TIM4->PSC = 16 - 1;
    TIM4->PSC = 16 - 1;                    // Bộ chia tần số: 16 - 1 = 15
    TIM4->ARR = 1000 - 1;       // 1ms (1kHz)
    TIM4->DIER |= TIM_DIER_UIE;
    TIM4->CR1 |= TIM_CR1_CEN;
    NVIC_EnableIRQ(TIM4_IRQn);
}

void Buttons_Init(void) {
	RCC->APB2ENR &= ~(1 << 14);                    // Clear SYSCFG clock enable (bit 14)
	RCC->APB2ENR |= (1 << 14);                     // Enable SYSCFG clock
    // PA0 và PA1 là input + pull-up
    GPIOA->MODER &= ~((3 << 0) | (3 << 2));  // Input mode
    GPIOA->PUPDR &= ~((3 << 0) | (3 << 2));
    GPIOA->PUPDR |=  (1 << 0) | (1 << 2);    // Pull-up

    // Kết nối EXTI0 và EXTI1 tới PA0 và PA1
    SYSCFG->EXTICR[0] &= ~((0xF << 0) | (0xF << 4));

    // Cho phép EXTI0 và EXTI1
    EXTI->IMR |= (1 << 0) | (1 << 1);
    EXTI->FTSR |= (1 << 0) | (1 << 1); // Ngắt sườn xuống

    // Bật NVIC
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
}
void GPIO_Init(void) {
	RCC->AHB1ENR &= ~((1 << 0) | (1 << 1) | (1 << 2));    // Clear GPIOA, GPIOB, GPIOC clock enable (bits 0, 1, 2)
	RCC->AHB1ENR |= ((1 << 0) | (1 << 1) | (1 << 2));     // Enable GPIOA, GPIOB, GPIOC clock
    // PC1 - ADC input
    GPIOC->MODER |= (3 << (1 * 2));  // Analog

    // PA8 - van (output)
    GPIOA->MODER &= ~(3 << (8 * 2));
    GPIOA->MODER |=  (1 << (8 * 2));       // Output
    GPIOA->OTYPER &= ~(1 << 8);            // Push-pull
    GPIOA->ODR &= ~(1 << 8);               // Tắt ban đầu

    // PB13 - quạt, PB10 - còi (output)
    GPIOB->MODER &= ~((3 << (13 * 2)) | (3 << (10 * 2)));
    GPIOB->MODER |=  ((1 << (13 * 2)) | (1 << (10 * 2))); // Output mode

    GPIOB->OTYPER &= ~((1 << 13) | (1 << 10)); // Push-pull
    GPIOB->ODR &= ~((1 << 13) | (1 << 10));    // Tắt ban đầu
}

void ADC_Init(void) {
	RCC->APB2ENR &= ~(1 << 8);                    // Clear ADC1 clock enable (bit 8)
	RCC->APB2ENR |= (1 << 8);                     // Enable ADC1 clock
    ADC1->CR2 = 0; // Tắt ADC để cấu hình
    ADC1->SQR1 &= ~(0xF << 20); // 1 lần chuyển đổi
    ADC1->SQR3 = 11; // Kênh 11 (PC1)
    ADC1->SMPR1 |= (7 << 3); // Sampling time 480 cycles cho kênh 11 (bit 3-5)
    ADC1->CR2 &= ~(1 << 0);                    // Clear ADON bit (bit 0)
    ADC1->CR2 |= (1 << 0);                     // Enable ADC (set ADON bit)
}

