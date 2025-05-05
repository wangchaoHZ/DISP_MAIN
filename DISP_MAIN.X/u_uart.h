/*
 * @Author: wangchao
 * @Date: 2025-05-05 12:07:43
 * @LastEditors: wangchao
 * @LastEditTime: 2025-05-05 13:59:12
 * @FilePath: \DISP_MAIN.X\u_uart.h
 * @Description:
 * Copyright (c) 2025 by Bingshan Guardian, All Rights Reserved.
 */
#ifndef _U_UART_H
#define _U_UART_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#define _XTAL_FREQ 10000000  // 10MHz 外部晶振

#define UART_RX_FRAME_SIZE 16

extern volatile uint8_t uart_rx_buffer[UART_RX_FRAME_SIZE];
extern volatile uint8_t uart_rx_index;
extern volatile bool uart_frame_ready;

void UART_Init(void);
void UART_SendChar(char ch);
void UART_SendString(const char* str);

#endif
