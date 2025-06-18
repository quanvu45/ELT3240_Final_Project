#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>
#include "uart.h"

void USART1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // PA9 = TX, PA10 = RX (AF7)
    GPIOA->MODER &= ~((3 << 18) | (3 << 20));     // Clear MODER9,10
    GPIOA->MODER |=  ((2 << 18) | (2 << 20));     // Alternate Function

    GPIOA->AFR[1] &= ~((0xF << 4) | (0xF << 8));  // Clear AFR for PA9, PA10
    GPIOA->AFR[1] |=  ((7 << 4) | (7 << 8));      // AF7 = USART1

    USART1->BRR = SystemCoreClock / 9600;         // Baudrate = 9600 (với HSI 16 MHz → BRR ≈ 1667)

    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;   // Bật cả gửi và nhận
    USART1->CR1 |= USART_CR1_UE;                  // Bật USART
}

char USART1_ReadChar(void) {
    while (!(USART1->SR & USART_SR_RXNE));
    return USART1->DR;
}

void USART1_ReadString(char *buffer, uint16_t max_len) {
    uint16_t i = 0;
    char c;

    while (i < (max_len - 1)) {
        while (!(USART1->SR & USART_SR_RXNE));
        c = USART1->DR;
        if (c == '\n') break;
        buffer[i++] = c;
    }
    buffer[i] = '\0';
}

void USART1_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

void USART1_SendString(const char *str) {
    while (*str) {
        USART1_SendChar(*str++);
    }
}
void USART2_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 = TX (debug), PA3 = RX (nếu cần)
    GPIOA->MODER &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER |=  ((2 << 4));       // PA2: Alternate Function
    GPIOA->AFR[0] |= (7 << 8);         // AF7: USART2

    USART2->BRR = SystemCoreClock / 9600;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;
}

void USART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void USART2_SendString(const char *str) {
    while (*str) USART2_SendChar(*str++);
}
