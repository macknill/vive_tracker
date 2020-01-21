#include "stm32f10x.h"
#include "main.h"


void init_tim_dma(TIM_TypeDef* TIMx, DMA_Channel_TypeDef* DMAy_Channelx, uint16_t* tim_array, uint16_t array_lenght);
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
    
  
  
}

void DMA1_Channel5_IRQHandler (void) 
{
  DMA_ClearFlag(DMA1_FLAG_GL5 | DMA1_FLAG_TC5 | DMA1_FLAG_HT5);
}

uint32_t array[50];
uint16_t array_cnt = 0;
uint32_t old = 0;
uint32_t temp = 0;

void TIM3_IRQHandler(void)
{
  
  led_toggle();
  TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
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

#define DMA_BUFF_SIZE 				60				

uint16_t buff[DMA_BUFF_SIZE];//Буфер

void init_signal_gpio_timers_dma(void)
{ 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  
  //TIM2__CH1(PA2), TIM3__CH1(PA7) input_config
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  //TIM4__CH1(PB7) input_config;  
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  //GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);
  //GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); 
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 
  
  init_tim_dma(TIM2, DMA1_Channel5, buff, DMA_BUFF_SIZE);
  init_tim_dma(TIM3, DMA1_Channel6, buff, DMA_BUFF_SIZE);
  init_tim_dma(TIM4, DMA1_Channel1, buff, DMA_BUFF_SIZE);
}

void init_tim_dma(TIM_TypeDef* TIMx, DMA_Channel_TypeDef* DMAy_Channelx, uint16_t* tim_array, uint16_t array_lenght)
{  
  DMA_InitTypeDef DMA_InitStructure;
  DMAy_Channelx->CPAR = (uint32_t)&(TIMx->DMAR); //Указываем адрес периферии - регистр результата преобразования АЦП для регулярных каналов
  DMAy_Channelx->CMAR = (uint32_t)tim_array; //Задаем адрес памяти - базовый адрес массива в RAM
  DMAy_Channelx->CCR &= ~DMA_CCR1_DIR; //Указываем направление передачи данных, из периферии в память
  DMAy_Channelx->CNDTR = array_lenght; //Количество пересылаемых значений
  DMAy_Channelx->CCR &= ~DMA_CCR1_PINC; //Адрес периферии не инкрементируем после каждой пересылки
  DMAy_Channelx->CCR |= DMA_CCR1_MINC; //Адрес памяти инкрементируем после каждой пересылки.
  DMAy_Channelx->CCR |= DMA_CCR1_PSIZE_0; //Размерность данных периферии - 16 бит
  DMAy_Channelx->CCR |= DMA_CCR1_MSIZE_0; //Размерность данных памяти - 16 бит
  DMAy_Channelx->CCR |= DMA_CCR1_PL; //Приоритет - очень высокий 
  DMAy_Channelx->CCR |= DMA_CCR1_CIRC; //Разрешаем работу DMA в циклическом режиме
  DMAy_Channelx->CCR |= DMA_CCR1_TCIE;//Разрешаем прерывание по окончанию передачи
  DMAy_Channelx->CCR |= DMA_CCR1_EN; //Разрешаем работу 1-го канала DMA
    
  TIM_TimeBaseInitTypeDef timer_base;
  TIM_TimeBaseStructInit(&timer_base);
  timer_base.TIM_Prescaler = 14;
  TIM_TimeBaseInit(TIMx, &timer_base);
  
  TIM_DMAConfig(TIMx, TIM_DMABase_CCR1, TIM_DMABurstLength_3Transfers);
  TIM_DMACmd(TIMx,TIM_DMA_CC1,ENABLE);
  
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;

  TIM_PWMIConfig(TIMx, &TIM_ICInitStructure);
  TIM_SelectInputTrigger(TIMx, TIM_TS_TI2FP2);
  TIM_SelectSlaveMode(TIMx, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIMx, TIM_MasterSlaveMode_Enable);
  TIM_Cmd(TIMx, ENABLE);
  //TIM_ITConfig(TIMx, TIM_IT_CC1, ENABLE);
  //NVIC_EnableIRQ(TIM3_IRQn);
  //NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

















