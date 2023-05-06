// 핀 연결
//PC0~PC7 : FND A~H 
//PC8~PC11 : FND C0~C3

#include "stm32f4xx.h"
#include "tim7_fnd.h"

// 7-세그먼트 폰트를 배열로 지정한다.
unsigned char Font[18] = {0x3F, 0X06, 0X5B, 0X4F, 
                                         0X66, 0X6D, 0X7C, 0X07,
                                         0X7F, 0X67, 0X77, 0X7C, 
                                         0X39, 0X5E, 0X79, 0X71, 
                                         0X08, 0X80};

static volatile int t_cnt = 0;                                      //4ms
static volatile int m_cnt = 0;
volatile unsigned long systick_count = 0;        //4ms
volatile unsigned long systick_sec=0;            //1sec
volatile unsigned long systick_ten=0;            //10sec
volatile int fndNumber = 0;
volatile int systick_secFlag = 0;                                         
volatile int systick_tenFlag = 0;          
void TIM7_Fnd_Init()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef   NVIC_InitStructure; 

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|
                                GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|
                                GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  //system_stm32f4xx.c 참조
  //TIM7 input clock (TIM7CLK) is set to 2 * APB1 clock (PCLK1),
  //since APB1 prescaler is different from 1.
  //TIM7CLK = 2 * PCLK1
  //PCLK1 = HCLK / 4  => TIM7CLK = HCLK / 2 = SystemCoreClock /2
  //TIM_CounterMode를 TIM_CounterMode_Up로 설정하면, 카운트값이 TIMx_ARR값과
  //같아질때, Update 이벤트가 발생한다.
  //TIM_Prescaler를 83으로, TIM_ClockDivision를 TIM_CKD_DIV1(1분주)로
  //설정하면, 타이머7 카운트 클럭은 (84MHz/1) /(83+1) = 1MHz (1us)이다.
  //TIM_Period를 999으로 설정했으니(TIMx_ARR), 카운터가 이값과 같아지면,
  //(1us x 1000(999+1) = 1ms)
  TIM_TimeBaseStructure.TIM_Prescaler = 84-1;         //(168Mhz/2)/84 = 1MHz(1us)
  TIM_TimeBaseStructure.TIM_Period = 4000-1;          // 1uS * 4000 = 4ms
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
  //타이머7을 동작시킨다.
  TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM7, ENABLE);
}
  
void TIM7_IRQHandler(void)              //4ms
{
  static int fnd_digit=1;

  if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    t_cnt++ ;
    systick_count++;
    m_cnt++;
 
//    display_fnd(fnd_digit,systick_sec);
    display_fnd(fnd_digit,fndNumber);
    if(fnd_digit == 4)          //4ms
      fnd_digit = 1;
    else
      fnd_digit++;

    if(t_cnt >= 250)             //1s
    {
      t_cnt = 0;
      systick_sec++; 
      systick_secFlag = 1;
    }
    if(m_cnt >= 2500)
    {
      m_cnt=0;
      systick_ten++;
      systick_tenFlag=1;
    }
  }
}

void display_fnd(int digit,int val)
{
	static int n1000,n100,n10,n1,buff;
	if(digit == DIGIT1)
        {
          	buff = val % 1000;
          	n1000 = val / 1000;
		display_digit(DIGIT1,n1000);
        }
	else if(digit == DIGIT2)
        {
          	n100 = buff / 100;
                buff = buff % 100;
		display_digit(DIGIT2,n100);
        }
	else if(digit == DIGIT3)
        {
          	n10 = buff /10;
		display_digit(DIGIT3,n10); 
        }
	else if(digit == DIGIT4)	
        {
          	n1 = buff % 10;
		display_digit(DIGIT4,n1);	
        }

}
void display_digit(int positon,int number)
{
        GPIO_Write(FND_PORT,GPIO_ReadInputData(FND_PORT)  | 0x0f00);  
        GPIO_Write(FND_PORT,GPIO_ReadInputData(FND_PORT)  & ~( 0x0100 << (positon - 1) | 0xff));  
        GPIO_Write(FND_PORT,(GPIO_ReadInputData(FND_PORT) & 0x0f00) | Font[number]);
}