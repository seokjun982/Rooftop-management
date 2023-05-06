// 핀 연결
// PA0,PA1 : UART4 RX,TX

#include "stm32f4xx.h"
#include "uart4.h"

volatile unsigned char rx4Flag = 0;
char rx4Data[50];
void UART4_init()
{
  
  GPIO_InitTypeDef   GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);    //UART4_TX
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);    //UART4_RX

  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UART4, &USART_InitStructure);

  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); // UART4 Interrupt enable
  USART_Cmd(UART4, ENABLE);

}
void Serial4_Send(unsigned char t)
{
  USART_SendData(UART4, t);
  while (USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
  
}

void Serial4_Send_String(char* s)
{
  while( *s != '\0')
  {
   Serial4_Send((unsigned char)(*s));
   s++;   //s = s + 1;
  }
}

void UART4_IRQHandler(void)
{
  if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
  {
	static int i=0;
	rx4Data[i] = USART_ReceiveData(UART4);
	if(rx4Data[i] == '\n')  
	{
		rx4Data[i] = '\0';
		rx4Flag = 1;
		i = 0;
	}
	else
	{
		i++;
	}
  }
}
