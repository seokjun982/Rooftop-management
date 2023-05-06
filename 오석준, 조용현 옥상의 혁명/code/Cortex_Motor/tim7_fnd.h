#ifndef __FND_TIM7_H__
#define __FND_TIM7_H__

#define DIGIT1 1
#define DIGIT2 2
#define DIGIT3 3
#define DIGIT4 4
#define FND_PORT GPIOC

void TIM7_Fnd_Init();
void display_fnd(int digit,int val);
void display_digit(int positon,int number);

#endif