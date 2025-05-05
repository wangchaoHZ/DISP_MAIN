/*
 * @Author: wangchao
 * @Date: 2025-05-05 12:05:38
 * @LastEditors: wangchao
 * @LastEditTime: 2025-05-05 14:06:18
 * @FilePath: \DISP_MAIN.X\main.c
 * @Description:
 * Copyright (c) 2025 by Bingshan Guardian, All Rights Reserved.
 */
#include "u_conf.h"
#include "u_uart.h"

// CONFIG1H
#pragma config OSC = HS     // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = OFF  // Fail-Safe Clock Monitor Enable bit
#pragma config IESO = OFF   // Internal/External Oscillator Switchover bit

// CONFIG2L
#pragma config PWRT = OFF   // Power-up Timer Enable bit
#pragma config BOREN = OFF  // Brown-out Reset Enable bits
#pragma config BORV = 3     // Brown-out Reset Voltage bits

// CONFIG2H
#pragma config WDT = OFF      // Watchdog Timer Enable bit
#pragma config WDTPS = 32768  // Watchdog Timer Postscale Select bits

// CONFIG3H
#pragma config CCP2MX = PORTC  // CCP2 MUX bit

// CONFIG4L
#pragma config LVP = OFF    // Single-Supply ICSP Enable bit
#pragma config DEBUG = OFF  // Background Debugger Enable bit

void delay_ms(unsigned int ms)
{
    while (ms--)
    {
        __delay_ms(1);
    }
}

void main(void)
{
    delay_ms(1000);

    UART_Init();
    UART_SendString("UART Init OK\r\n");

    while (1)
    {
        if (uart_frame_ready)
        {
            uart_frame_ready = false;  // Reset the flag
            UART_SendString("Received: ");
            for (uint8_t i = 0; i < uart_rx_index; i++)
            {
                UART_SendChar(uart_rx_buffer[i]);
            }
            UART_SendString("\r\n");
            uart_rx_index = 0;  // Reset the index for next frame
        }
    }
}
