#ifndef __LCD_H__
#define __LCD_H__
#define TLCD_RS  GPIO_Pin_0                  // LCD RS 신호 
#define TLCD_RW  GPIO_Pin_1                  // LCD RW 신호 
#define TLCD_E   GPIO_Pin_2                  // LCD E 신호 
#define TLCD_EN { GPIO_ResetBits(GPIOB, TLCD_E); GPIO_SetBits(GPIOB, TLCD_E); }
#define DATA     GPIOA                       // LCD 데이터 신호 

#define ON      1
#define OFF     2

#define RIGHT   1
#define LEFT    2

#define DATAPIN 5
static void Delay(const uint32_t Count);
static void Delay_us(const uint32_t Count);
void GPIOAB_LCD_Init(void);
// LCD 에 모든 명령을 전송한후 반드시 Enable Pulse를 주어야 한다.
void E_Pulse(void);
// LCD에 데이터를 전송하는 함수이다   
void TLCD_DATA(unsigned char data);
void Func_Set(void);
// LCD 초기화 함수
void Init_LCD(void);
  /******************************************
     lcd_char() : 1 문자를 디스플레이 하는 함수
  *******************************************/
void lcd_char(char s);
void lcd_disp(char x, char y);
void move_disp(char p );
void disp_ON_OFF(char d, char c, char b);
void clrscr(void);
void lcd(char x, char y, char *str);

#endif