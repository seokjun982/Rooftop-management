#ifndef __UART4_H__
#define __UART4_H__
void UART4_init();
void Serial4_Event();
void Serial4_Send(unsigned char t);
void Serial4_Send_String(char* s);
#endif