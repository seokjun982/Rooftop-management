#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx.h"
#include "uart2.h"
#include "uart4.h"
#include "adc1.h"
#include "tim7_fnd.h"
#include "tim10_motor.h"
#include "tim4_counter.h"
#include "key.h"
#include "lcd.h"
#include "led.h"


/******************************************************************************
* General-purpose timers TIM10

    포트 연결:

    PA0,PA1 : UART4 RX,TX
    PA2,PA3 : UART2 RX,TX
    PA5 ~ PA12 : CLCD D0~D7

    PB0 ~ PB2  : CLCD RS, RW, E

    PA7 :  M_SEN(Motor Sensor)
    PB8  : DC Motor M_EN
    PB9  : DC Motor M_D2
    PB10 : DC Motor M_D1

    PC0~PC11 ==> FND A~H, C0~C3
    PC12~PC15 ==> Button0 ~ 3 

    FND 출력값
    FND12 ==> PWM
    FND34 ==> Duty rate

******************************************************************************/
#define CMD_SIZE 50
#define ARR_CNT 6
#define PORTC_FND 

static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16800 * Count); index != 0; index--);
}

volatile int pwm = 50;
volatile int counter = 0;
extern uint16_t adc_data;
extern volatile int fndNumber;
extern volatile int adc1Flag;
extern volatile unsigned long systick_sec;            //1sec
extern volatile int systick_secFlag;
extern volatile unsigned long systick_ten;            //1sec
extern volatile int systick_tenFlag;
extern volatile unsigned char rx2Flag;
extern char rx2Data[50];
extern volatile unsigned char rx4Flag;
extern char rx4Data[50];
extern int key;
extern unsigned int tim1_counter;
extern unsigned int tim10_counter;
long map(long x, long in_min, long in_max, long out_min, long out_max);
void Motor_ON();
void Motor_OFF();
void Servo_ON();
void Servo_OFF();
void Motor_Pwm(int val);
void Servo(int Angle);
       int main()
{
  int old_pwm=50;
  int adc_pwm=50;
  int ccr1;
  int pwmCount = 0;
  int vresFlag = 0;
 
 
  Key_Init();

#ifdef PORTC_FND
  TIM7_Fnd_Init();
#else
  PORTC_Led_Init();  
#endif   
  UART2_init();
  UART4_init();
  TIM10_Motor_Init();
  TIM4_Counter_Init();        //PORTA  사용
  GPIOAB_LCD_Init();
  //lcd(0,0,"AIoT Embedded");    // 문자열 출력
  //cd(0,1,"Bluetooth Test");    // 문자열 출력
  Init_ADC1();
//  ADC_SoftwareStartConv(ADC1); 
  
  Serial2_Send_String("start main()\r\n");
  Serial4_Send_String("start BLU()\r\n");
  pwmCount = pwm * 100;
  while(1)
  {
    if(tim10_counter>0)
    {
      tim10_counter=0;
      GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_SET);      
      GPIO_WriteBit(GPIOB,GPIO_Pin_10,Bit_RESET);  
    }
    if(rx2Flag)
    {
      printf("rx2Data : %s\r\n",rx2Data);
//      Serial4_Send_String(rx2Data);
//      Serial4_Send_String("\r");
      rx2Flag = 0;
    }  
    if(rx4Flag)
    {
      Serial4_Event();
      rx4Flag = 0;
    }
    if(systick_tenFlag)
    {
      Serial4_Send_String("[SQL]GETSENSOR\r\n");
      Delay(10);
      systick_tenFlag=0;
    }  
    
    if(key != 0)
    {
      printf("Key : %d  \r\n",key);
      if(key == 1) //motor right
      {
        Motor_ON();
      }
      else if(key == 2) //motor stop 
      {
        Motor_OFF();    
      }
      else if(key == 3) //SERVOmotor on 
      {
        Servo_ON();    
      }
      else if(key == 4) //motor stop 
      {
        Servo_OFF();    
      }
      else if(key == 5)
      {
        if(vresFlag)
          vresFlag = 0;
        else
          vresFlag = 1;
      }
      key = 0;
    } 
    
    if(systick_secFlag && vresFlag )
    {
      ADC_SoftwareStartConv(ADC1); 
      systick_secFlag = 0;
    }
    
    if(adc1Flag)
    {  
//    adc_pwm = (int)((adc_data/4096.0) * 100);      
      adc_pwm = map(adc_data,0,4095,0,100);
      if(abs(adc_pwm - old_pwm)>=5)
          pwm = adc_pwm;
      adc1Flag = 0;
    }

    if(pwm != old_pwm  )
    {
        if(pwm == 0)          
             ccr1 = 1;
        else if(pwm == 100)
             ccr1 = 177 * 100 - 1;
        else
             ccr1 = 177 * pwm;
        
        TIM10->CCR1 = ccr1;
        old_pwm = pwm;
        pwmCount = pwm * 100;
        printf("PWM : %d\r\n",pwm);
    }

//    counter = TIM_GetCounter(TIM1);
//    if(counter >= 100)
//      counter = counter % 100;

#ifdef PORTC_FND
    fndNumber = pwmCount + tim1_counter%100;
#endif  

  }
}

void Serial4_Event()
{
  
//  Serial2_Send_String(rx4Data);
//  Serial2_Send_String("\r\n");

  int i=0;
  int num = 0;
  char * pToken;
  char temp[20] = {0, };
  char humi[20] = {0, };
  char cds[20] = {0, };
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0};
  char sendBuf[CMD_SIZE]={0}; 
  strcpy(recvBuf,rx4Data);
  i=0; 
//  Serial2_Send_String(recvBuf);
//  Serial2_Send_String("\n\r");
  printf("rx4Data : %s\r\n",recvBuf);
     
  pToken = strtok(recvBuf,"[@]");

  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
  
  printf("pArray[1] : %s\r\n", pArray[1]); 
  printf("pArray[2] : %s\r\n", pArray[2]); 
       
  if(!strcmp(pArray[1],"SENSOR"))
  {
    if(pArray[3] != NULL) {             
      sprintf(temp,"T:%s",pArray[3]);
      lcd(0,1,temp);
      sprintf(humi,"H:%s",pArray[4]);
      lcd(5,1,humi);
      sprintf(cds,"I:%s",pArray[5]);
      lcd(10,1,cds);
    }
  } 
  
  else if(!strcmp(pArray[1],"PUMP"))
  {
    if(!strcmp(pArray[2],"ON"))  
      Motor_ON();
    else if(!strcmp(pArray[2],"OFF")) 
      Motor_OFF();
    else if(!strcmp(pArray[2],"PWM")) 
    {
      num = atoi(pArray[3]);
      Motor_Pwm(num);
    }
  }
  else if(!strcmp(pArray[1],"SERVO"))
  {
    if(!strcmp(pArray[2],"ON"))  
      Servo_ON();
    else if(!strcmp(pArray[2],"OFF")) 
      Servo_OFF();
    else if(!strcmp(pArray[2],"PWM")) 
    {
      num = atoi(pArray[3]);
      Motor_Pwm(num);
    }
  }
  else if(!strncmp(pArray[1]," New conn",sizeof(" New conn")))
  {
    return;
  }
  else if(!strncmp(pArray[1]," Already log",sizeof(" Already log")))
  {
    return;
  }    
  else
    return;
  
  sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  Serial4_Send_String(sendBuf);
}


void Motor_ON()
{
  GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_RESET); 

  lcd(0,0,"MOTOR ON "); 
}

void Motor_OFF()
{
  GPIO_WriteBit(GPIOB,GPIO_Pin_9,Bit_SET); 
  lcd(0,0,"MOTOR OFF "); 
}

void Servo_ON()
{
 GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);
 lcd(0,0,"Survo ON ");
 //Servo(90);
 TIM10->CCR1 = 500;
 
}

void Servo_OFF()
{
  GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);
  lcd(0,0,"Survo OFF ");
  // Servo(-90);
  TIM10->CCR1 = 1900;
}

void Motor_Pwm(int val)
{
    pwm = val;
}
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}