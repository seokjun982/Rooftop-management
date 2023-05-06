#ifndef __UART2_H__
#define __UART2_H__
void UART2_init();
void Serial2_Event();
void Serial2_Send(unsigned char t);
void Serial2_Send_String(char* s);
int putchar(int ch);            //printf() 사용시 필요
void print_2d1(double number);
#endif