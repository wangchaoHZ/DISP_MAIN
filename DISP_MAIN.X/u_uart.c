/*
 * @Author: wangchao
 * @Date: 2025-05-05 12:06:41
 * @LastEditors: wangchao
 * @LastEditTime: 2025-05-05 14:04:18
 * @FilePath: \DISP_MAIN.X\u_uart.c
 * @Description:
 * Copyright (c) 2025 by Bingshan Guardian, All Rights Reserved.
 */
#include "u_uart.h"
#include "u_conf.h"

volatile uint8_t uart_rx_buffer[UART_RX_FRAME_SIZE];
volatile uint8_t uart_rx_index = 0;
volatile bool uart_frame_ready = false;

void UART_Init(void)
{
    TRISC6 = 0;  // TX
    TRISC7 = 1;  // RX

    SPBRG = 15;  // 9600bps @ 10MHz, BRGH=0
    BRGH = 0;
    BRG16 = 0;

    SYNC = 0;
    SPEN = 1;
    TXEN = 1;
    CREN = 1;

    RCIF = 0;
    RCIE = 1;  // 使能串口接收中断
    PEIE = 1;  // 外设中断
    GIE = 1;   // 总中断
}

void UART_SendChar(char ch)
{
    while (!TXIF)
        ;
    TXREG = ch;
}

void UART_SendString(const char* str)
{
    while (*str)
        UART_SendChar(*str++);
}

// 串口接收中断
void __interrupt() ISR(void)
{
    if (RCIF)
    {
        uint8_t received = RCREG;

        if (uart_frame_ready == false)
        {
            uart_rx_buffer[uart_rx_index++] = received;
            if (uart_rx_index >= UART_RX_FRAME_SIZE)
            {
                uart_frame_ready = true;
                uart_rx_index = 0;
            }
        }
    }
}