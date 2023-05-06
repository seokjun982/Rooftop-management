// ÇÉ ¿¬°á
// PB7 :  M_SEN(Motor Sensor)
#include "stm32f4xx.h"
#include "tim4_counter.h"
volatile unsigned int tim1_counter = 0;
void TIM4_Counter_Init()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;     //TIM4_Counter  //TIM4_CH2
  GPIO_Init(GPIOB, &GPIO_InitStructure);
 
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  TIM_TIxExternalClockConfig(TIM4,TIM_TIxExternalCLK1Source_TI2,TIM_ICPolarity_Rising,0x3);

  TIM_ClearITPendingBit(TIM4, TIM_IT_Trigger);
  TIM_ITConfig(TIM4, TIM_IT_Trigger, ENABLE);
  
  TIM_Cmd(TIM4,ENABLE); 
}

void TIM4_IRQHandler()
{
  tim1_counter++;
  if(TIM_GetITStatus(TIM4, TIM_IT_Trigger) != RESET)
  {
    tim1_counter++;
    TIM_ClearITPendingBit(TIM4, TIM_IT_Trigger);
  } 
}