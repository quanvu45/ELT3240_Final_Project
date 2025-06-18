/*
 * uart.h
 *
 *  Created on: Jun 18, 2025
 *      Author: DELL
 */

#ifndef INC_UART_H_
#define INC_UART_H_

void USART1_SendString(const char *str);
void USART1_Init(void);
void USART1_SendChar(char c);
char USART1_ReadChar(void);
void USART1_ReadString(char *buffer, uint16_t max_len);

void USART2_Init(void);
void USART2_SendChar(char c);
void USART2_SendString(const char *str);
#endif /* INC_UART_H_ */
