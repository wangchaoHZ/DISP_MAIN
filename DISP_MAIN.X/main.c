/*
 * @Author: wangchao
 * @Date: 2025-05-05 12:05:38
 * @LastEditors: wangchao
 * @LastEditTime: 2025-06-07 21:36:46
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

uint8_t read_spi_inputs(void)
{
    uint8_t result = 0;

    uint8_t rc3 = PORTCbits.RC3;
    uint8_t rc4 = PORTCbits.RC4;

    // if (rc3)
    // {
    //     UART_SendString("rc3=1\n");
    // }
    // else
    // {
    //     UART_SendString("rc3=0\n");
    // }

    // if (rc4)
    // {
    //     UART_SendString("rc4=1\n");
    // }
    // else
    // {
    //     UART_SendString("rc4=0\n");
    // }

    result |= (rc3 << 0);
    result |= (rc4 << 1);
    return result;
}

uint8_t read_key_inputs(void)
{
    uint8_t result = 0;

    result |= (PORTCbits.RC0 << 0);
    result |= (PORTCbits.RC1 << 1);
    result |= (PORTCbits.RC2 << 2);
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
    // LATCbits.LATC5 = 0;
    LATCbits.LATC5 = 0;  // 设置 RC5 为高电平

    // 设置 RB0 为输出
    TRISBbits.TRISB0 = 0;
    // 输出低电平
    LATBbits.LATB0 = 1;

    TRISDbits.TRISD0 = 0;
    TRISDbits.TRISD1 = 0;
    TRISDbits.TRISD2 = 0;
    TRISDbits.TRISD3 = 0;
    TRISDbits.TRISD4 = 0;
    TRISDbits.TRISD5 = 0;
    TRISDbits.TRISD6 = 0;
    TRISDbits.TRISD7 = 0;

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

    latch_data(0xff);

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

uint8_t RED_KEY_BYTE = 0;
uint8_t GREEN_KEY_BYTE = 0;

char red_str[9];  // 8位 + '\0'
char green_str[9];

void byte_to_bit_string(uint8_t byte, char* str)
{
    for (int i = 7; i >= 0; i--)
    {
        *str++ = (byte & (1 << i)) ? '1' : '0';
    }
    *str = '\0';  // 末尾加结束符
}

void reset_usr_key_value(void)
{
    RED_KEY_BYTE = 0;
    GREEN_KEY_BYTE = 0;
}

uint16_t get_key_value(void)
{
    return (uint16_t)((RED_KEY_BYTE << 8) | GREEN_KEY_BYTE);
}

// read_key_inputs
void scan_usr_key(void)
{
    uint8_t retx = 0;
    latch_data(0x00);  // NO DISPLAY

    delay_ms(20);
    LATAbits.LATA0 = 1;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    LATEbits.LATE0 = 0;
    LATAbits.LATA5 = 0;
    retx = read_key_inputs();
    if (retx == 6 || retx == 5)
    {
        switch (retx)
        {
            case 6:
                RED_KEY_BYTE |= (1 << 0);
                break;
            case 5:
                GREEN_KEY_BYTE |= (1 << 0);
                break;
        }
    }

    delay_ms(20);
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 1;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    LATEbits.LATE0 = 0;
    LATAbits.LATA5 = 0;
    retx = read_key_inputs();
    if (retx == 6 || retx == 5)
    {
        switch (retx)
        {
            case 6:
                RED_KEY_BYTE |= (1 << 1);
                break;
            case 5:
                GREEN_KEY_BYTE |= (1 << 1);
                break;
        }
    }

    delay_ms(20);
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 1;
    LATAbits.LATA3 = 0;
    LATEbits.LATE0 = 0;
    LATAbits.LATA5 = 0;
    retx = read_key_inputs();
    if (retx == 6 || retx == 5)
    {
        switch (retx)
        {
            case 6:
                RED_KEY_BYTE |= (1 << 2);
                break;
            case 5:
                GREEN_KEY_BYTE |= (1 << 2);
                break;
        }
    }

    delay_ms(20);
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 1;
    LATEbits.LATE0 = 0;
    LATAbits.LATA5 = 0;
    retx = read_key_inputs();
    if (retx == 6 || retx == 5)
    {
        switch (retx)
        {
            case 6:
                RED_KEY_BYTE |= (1 << 3);
                break;
            case 5:
                GREEN_KEY_BYTE |= (1 << 3);
                break;
        }
    }

    delay_ms(20);
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    LATEbits.LATE0 = 1;
    LATAbits.LATA5 = 0;
    retx = read_key_inputs();
    if (retx == 6 || retx == 5)
    {
        switch (retx)
        {
            case 6:
                RED_KEY_BYTE |= (1 << 4);
                break;
            case 5:
                GREEN_KEY_BYTE |= (1 << 4);
                break;
        }
    }

    delay_ms(20);
    LATAbits.LATA0 = 0;
    LATAbits.LATA1 = 0;
    LATAbits.LATA2 = 0;
    LATAbits.LATA3 = 0;
    LATEbits.LATE0 = 0;
    LATAbits.LATA5 = 1;
    retx = read_key_inputs();
    if (retx == 6 || retx == 5)
    {
        switch (retx)
        {
            case 6:
                RED_KEY_BYTE |= (1 << 5);
                break;
            case 5:
                GREEN_KEY_BYTE |= (1 << 5);
                break;
        }
    }

    char xbuffer[10];
    sprintf(xbuffer, "RC:%04X\r\n", get_key_value());
    UART_SendString(xbuffer);  // 发送 RC 输入状态
}

void main(void)
{
    delay_ms(500);

    UART_Init();

    init_ports();  // 初始化 IO

    SelectIO(0);       // 选择 IO 0
    latch_data(0x00);  // 显示全亮

    reset_usr_key_value();
    uint8_t read_pin_cmd = 0;
    uint8_t read_spi_cmd = 0;

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
                read_pin_cmd = 1;
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_STOP_READ))
            {
                reset_usr_key_value();
                read_pin_cmd = 0;
                LATAbits.LATA0 = 0;
                LATAbits.LATA1 = 0;
                LATAbits.LATA2 = 0;
                LATAbits.LATA3 = 0;
                LATEbits.LATE0 = 0;
                LATAbits.LATA5 = 0;
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SET1_RC5))
            {
                LATBbits.LATB0 = 0;
                LATCbits.LATC5 = 0;
                char xbuffer[10];
                sprintf(xbuffer, "RC:%02X\r\n", read_spi_inputs());
                UART_SendString(xbuffer);  // 发送 RC 输入状态
            }
            else if (strstr((char*)uart_rx_buffer, APP_TEST_SET0_RC5))
            {
                LATBbits.LATB0 = 0;
                LATCbits.LATC5 = 1;  // 设置 RC5 为低电平
                char xbuffer[10];
                sprintf(xbuffer, "RC:%02X\r\n", read_spi_inputs());
                UART_SendString(xbuffer);  // 发送 RC 输入状态
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

        if (read_pin_cmd)
        {
            scan_usr_key();
        }
        else
        {
            reset_usr_key_value();
        }
    }
}
