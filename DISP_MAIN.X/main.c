/*
 * @Author: wangchao
 * @Date: 2025-05-05 12:05:38
 * @LastEditors: wangchao
 * @LastEditTime: 2025-05-17 20:38:21
 * @FilePath: \DISP_MAIN.X\main.c
 * @Description:
 * Copyright (c) 2025 by Bingshan Guardian, All Rights Reserved.
 */
#include "u_conf.h"
#include "u_uart.h"
#include "u_cmd.h"
#include <stdio.h>
#include <string.h>

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

void latch_data(uint8_t seg_data)
{
    // 将段码写入 RD0~RD7
    LATD = seg_data;

    // 产生锁存脉冲（RB1上升沿）
    LATBbits.LATB1 = 0;
    __delay_us(10);
    LATBbits.LATB1 = 1;
    __delay_us(10);
    LATBbits.LATB1 = 0;
}

void init_inputs_RC0_to_RC4(void)
{
    TRISCbits.TRISC0 = 1;  // RC0 设置为输入
    TRISCbits.TRISC1 = 1;  // RC1 设置为输入
    TRISCbits.TRISC2 = 1;  // RC2 设置为输入
    TRISCbits.TRISC3 = 1;  // RC3 设置为输入
    TRISCbits.TRISC4 = 1;  // RC4 设置为输入
}

uint8_t read_RC_inputs(void)
{
    uint8_t result = 0;

    result |= (PORTCbits.RC0 << 0);
    result |= (PORTCbits.RC1 << 1);
    result |= (PORTCbits.RC2 << 2);
    result |= (PORTCbits.RC3 << 3);
    result |= (PORTCbits.RC4 << 4);
    return result;
}

void init_ports(void)
{
    // 设置端口方向
    // TRISD = 0x00;          // 段码输出
    TRISBbits.TRISB1 = 0;  // CLK 输出
    // TRISC &= 0xF0;         // RC0~RC2 输出，RC3 EN 也要控制
    // TRISCbits.TRISC3 = 0;

    TRISAbits.TRISA0 = 0;  // CLK 输出
    TRISAbits.TRISA1 = 0;  // CLK 输出
    TRISAbits.TRISA2 = 0;  // CLK 输出
    TRISAbits.TRISA3 = 0;  // CLK 输出
    TRISAbits.TRISA5 = 0;  // CLK 输出
    TRISEbits.TRISE0 = 0;  // CLK 输出
    // 初始状态
    LATBbits.LATB1 = 0;

    TRISCbits.TRISC5 = 0;
    LATCbits.LATC5 = 0;

    // 设置 RB0 为输出
    TRISBbits.TRISB0 = 0;
    // 输出低电平
    LATBbits.LATB0 = 0;

    init_inputs_RC0_to_RC4();
}

// 选择对应 IO 输出高电平（1 ~6），其余为低电平
void SelectIO(uint8_t index)
{
    // 先清除所有相关引脚
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    LATAbits.LATA5 = 0;
    LATEbits.LATE0 = 0;

    // 根据输入 index 设置对应引脚为1
    switch (index)
    {
        case 6:
            LATAbits.LATA0 = 1;
            break;  // PA0
        case 5:
            LATAbits.LATA1 = 1;
            break;  // PA1
        case 4:
            LATAbits.LATA2 = 1;
            break;  // PA2
        case 3:
            LATAbits.LATA3 = 1;
            break;  // PA3

        case 2:
            LATEbits.LATE0 = 1;
            break;  // PE0

        case 1:
            LATAbits.LATA5 = 1;
            break;  // PA5
        default:
            break;  // 输入非法则不操作
    }
}

char buffer[20];  // 用于存储接收的字符串

void main(void)
{
    delay_ms(500);

    UART_Init();
    //    UART_SendString("UART Init OK\r\n");

    init_ports();  // 初始化 IO

    SelectIO(0);       // 选择 IO 0
    latch_data(0xFF);  // 显示全亮

    while (1)
    {
        if (uart_frame_ready)
        {
            uart_frame_ready = false;  // Reset the flag

            if (strstr((char*)uart_rx_buffer, APP_TEST_PORT_CONN))
            {
                UART_SendString("CONN_ACK\r\n");
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_1))
            {
                //                UART_SendString("SHOW_001\r\n");
                SelectIO(1);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_2))
            {
                // UART_SendString("SHOW_002\r\n");
                SelectIO(2);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_3))
            {
                // UART_SendString("SHOW_003\r\n");
                SelectIO(3);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_4))
            {
                // UART_SendString("SHOW_004\r\n");
                SelectIO(4);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_5))
            {
                // UART_SendString("SHOW_005\r\n");
                SelectIO(5);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_6))
            {
                // UART_SendString("SHOW_006\r\n");
                SelectIO(6);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SHOW_0))
            {
                // UART_SendString("SHOW_006\r\n");
                SelectIO(0);  // 选择 IO 1
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_READ_PORT))
            {
                uint8_t rc_input = read_RC_inputs();
                sprintf(buffer, "RC:%02X\r\n", rc_input);
                UART_SendString(buffer);  // 发送 RC 输入状态
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SET1_RC5))
            {
                // UART_SendString("SET1_RC5\r\n");
                LATCbits.LATC5 = 1;  // 设置 RC5 为高电平
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SET0_RC5))
            {
                // UART_SendString("SET0_RC5\r\n");
                LATCbits.LATC5 = 0;  // 设置 RC5 为低电平
            }
            else
            {
                UART_SendString("ERROR:UNKNOWN_CMD:");
                for (char i = 0; i < UART_RX_FRAME_SIZE; i++)
                {
                    UART_SendChar(uart_rx_buffer[i]);
                }
                UART_SendString("\r\n");
            }

            uart_rx_index = 0;  // Reset the index for next frame
        }
    }
}
