#include "stm32f10x.h"
#include "main.h"


GPIO_InitTypeDef GPIO_InitStructure;

void init_gpio(void)
{
  //GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIOC->BSRR |= GPIO_BSRR_BS13;    
    
  
  //TIM1_input_config
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

#define TIM2_CCR1_Address    ((u32)0x40000000+0x34)
#define DMA_BUFF_SIZE 				20				

uint16_t buff[DMA_BUFF_SIZE];//Буфер
uint16_t volatile T;

void DMA1_Channel7_IRQHandler (void) 
{
  T = (buff[1] > buff[0]) ? (buff[1] - buff[0]) : (65535+ buff[1] - buff[0]);
	
  DMA1->IFCR |= DMA_IFCR_CGIF7;
  DMA_ClearFlag(DMA1_FLAG_GL5 | DMA1_FLAG_TC5 | DMA1_FLAG_HT5);
}

void DMA1_Channel5_IRQHandler (void) 
{
  T = (buff[1] > buff[0]) ? (buff[1] - buff[0]) : (65535+ buff[1] - buff[0]);
  DMA_ClearFlag(DMA1_FLAG_GL5 | DMA1_FLAG_TC5 | DMA1_FLAG_HT5);
}

uint32_t array[50];
uint16_t array_cnt = 0;
uint32_t old = 0;
uint32_t temp = 0;
void TIM2_IRQHandler(void)
{
  
  led_toggle();
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
  if (array_cnt < 50)
  {
    array[array_cnt] = TIM2->CCR2;
    array_cnt++;
    array[array_cnt] = TIM2->CCR1;
    /*uint32_t now = TIM2->CCR1;
    if (now < old) 
      array[array_cnt] = now + 0xFFFF - old;
    else
      array[array_cnt] = now - old;
    old = now;*/
    array_cnt++;
  }
  else
  {
    temp++;
  }
}

void init_tim1_dma(void)
{  
  DMA_InitTypeDef DMA_InitStructure;
  /*
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(TIM2->CCR1);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFF_SIZE*2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);
  DMA_ITConfig(DMA1_Channel7, DMA_IT_TE, ENABLE);*/
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//Разрешаем тактирование первого DMA модуля
  DMA1_Channel5->CPAR = (uint32_t)&(TIM2->DMAR); //Указываем адрес периферии - регистр результата преобразования АЦП для регулярных каналов
  DMA1_Channel5->CMAR = (uint32_t)buff; //Задаем адрес памяти - базовый адрес массива в RAM
  DMA1_Channel5->CCR &= ~DMA_CCR1_DIR; //Указываем направление передачи данных, из периферии в память
  DMA1_Channel5->CNDTR = DMA_BUFF_SIZE; //Количество пересылаемых значений
  DMA1_Channel5->CCR &= ~DMA_CCR1_PINC; //Адрес периферии не инкрементируем после каждой пересылки
  DMA1_Channel5->CCR |= DMA_CCR1_MINC; //Адрес памяти инкрементируем после каждой пересылки.
  DMA1_Channel5->CCR |= DMA_CCR1_PSIZE_0; //Размерность данных периферии - 16 бит
  DMA1_Channel5->CCR |= DMA_CCR1_MSIZE_0; //Размерность данных памяти - 16 бит
  DMA1_Channel5->CCR |= DMA_CCR1_PL; //Приоритет - очень высокий 
  DMA1_Channel5->CCR |= DMA_CCR1_CIRC; //Разрешаем работу DMA в циклическом режиме
  DMA1_Channel5->CCR |= DMA_CCR1_TCIE;//Разрешаем прерывание по окончанию передачи
  DMA1_Channel5->CCR |= DMA_CCR1_EN; //Разрешаем работу 1-го канала DMA
  
  
  //включаем тактирование порта А, альтернативных функций и таймера
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
  
  TIM_TimeBaseInitTypeDef timer_base;
  TIM_TimeBaseStructInit(&timer_base);
  timer_base.TIM_Prescaler = 14;
  TIM_TimeBaseInit(TIM2, &timer_base);
  
  TIM_DMACmd(TIM2,TIM_DMA_CC1,ENABLE);
  
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);
  TIM_SelectInputTrigger(TIM2, TIM_TS_TI2FP2);
  TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
  TIM_Cmd(TIM2, ENABLE);
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
  
  TIM2->DCR |= TIM_DCR_DBL_0 | 13; //читаем 2 регистра начиная с 13 CCR1 to CCR2

  TIM_Cmd(TIM2, ENABLE);
  NVIC_EnableIRQ(TIM2_IRQn);
  NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

















