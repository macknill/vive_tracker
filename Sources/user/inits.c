#include "stm32f10x.h"
#include "modbus.h"
#include "mb_regs.h"
#include "main.h"
#include "ws2812.h"


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
    
  
  //JETSON_RESET 0 is ON
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOB, GPIO_Pin_1, 0);
  
  //JETSON_PON
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOA, GPIO_Pin_6, 0);  
  
  //JETSON_USB_VOLTAGE_READ
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15); 
       
  //ACCELERATION_PON
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOA, GPIO_Pin_5, 0);   
  
  //ALL_POWER_HOLD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOC, GPIO_Pin_14, 1); 
  
  //PWR_BUTTON_READ
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15); 
  
  mb.registers.one[mbREG_ALL_PON] = 1;
  mb.registers.one[mbREG_JETSON_PON] = 1;
  mb.registers.one[mbREG_JETSON_ON] = 1;
  mb.registers.one[mbREG_PPM_PON] = 0;
  mb.registers.one[mbREG_SRV_PON] = 0;
}



#define USARTy_Tx_DMA_Channel    DMA1_Channel4
#define USARTy_Tx_DMA_FLAG       DMA1_FLAG_TC4
#define USARTy_DR_Base           0x40013804

void init_modbus(uint32_t speed)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  
  //uarts pin config
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  //DMA init
  //DMA_DeInit(USARTy_Tx_DMA_Channel);
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)mb.u8BufferOut;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = mb.u16OutCnt;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);
  DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
  //DMA_Cmd(DMA1_Channel4, ENABLE);
    
  //interrupt init
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
  NVIC_Init(&NVIC_InitStructure);
  
  //timer init
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  uint16_t PrescalerValue = (uint16_t) (SystemCoreClock / 100000) - 1;
  TIM_TimeBaseStructure.TIM_Period = 4000000/speed;    //(1/speed)/(1/timeout)*4
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
  TIM_Cmd(TIM2, ENABLE);
  TIM_SetCounter(TIM2, 0);
  
  //Usart init
  USART_InitStructure.USART_BaudRate = speed;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART1, &USART_InitStructure);
  
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  
  //init reg
  mb.registers.one[mbREG_PPM_PON] = 0;
  mb.registers.one[mbREG_PPM_1] = 150;
  mb.registers.one[mbREG_PPM_2] = 150;
  mb.registers.one[mbREG_PPM_3] = 150;
  mb.registers.one[mbREG_PPM_4] = 150;
  
  
  
  
  DMA_SetCurrDataCounter(DMA1_Channel4, 14);
  DMA_Cmd(DMA1_Channel4, ENABLE);
  USART_Cmd(USART1, ENABLE);      
  mb.u8regsize = REGISTERS_SIZE;
}

void init_adc(void)
{    
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;  
  NVIC_InitTypeDef NVIC_InitStructure;  
  TIM_OCInitTypeDef         TIM_OCInitStructure;
  
  RCC_ADCCLKConfig(RCC_CFGR_ADCPRE_DIV8);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE); 
  
  //GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    
__IO uint16_t ADCConvertedValue;
  // DMA1 channel1 configuration ----------------------------------------------
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_data;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = ADC_CH_CNT * ADC_ARR_LENGT * 2;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel1 , ENABLE ) ;
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC | DMA_IT_HT, ENABLE);
  // Enable DMA1 channel1  
  
  //NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 5;
  ADC_Init(ADC1, &ADC_InitStructure);
    
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_71Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_55Cycles5);
  
  //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);  
  ADC_DMACmd(ADC1 , ENABLE );   
 // ADC_ExternalTrigConvCmd(ADC1, ENABLE);
  
  ADC_Cmd(ADC1 , ENABLE );
  
  ADC_ResetCalibration(ADC1);
  while(ADC_GetResetCalibrationStatus(ADC1));
  ADC_StartCalibration(ADC1);
  while(ADC_GetCalibrationStatus(ADC1));
  
  //start to 50 Hz
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);  
}

void init_ppm(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;  
  #define TIM_PPM TIM4
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;  
    
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  //GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);	
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, !mb.registers.one[mbREG_PPM_PON]);
  
  
  uint16_t PrescalerValue = (uint16_t) (SystemCoreClock / 100000) - 1;
  TIM_TimeBaseStructure.TIM_Period = 2000;    //(1/speed)/(1/timeout)*4
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM_PPM, &TIM_TimeBaseStructure);
  
  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 150;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM_PPM, &TIM_OCInitStructure);
  
  TIM_OC2Init(TIM_PPM, &TIM_OCInitStructure);
  
  TIM_OC3Init(TIM_PPM, &TIM_OCInitStructure);
  
  TIM_OC4Init(TIM_PPM, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM_PPM, TIM_OCPreload_Enable);

  TIM_ARRPreloadConfig(TIM_PPM, ENABLE);
  TIM_ITConfig(TIM_PPM, TIM_IT_Update, ENABLE);
  
  TIM_Cmd(TIM_PPM, ENABLE);
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
}

void init_ws2812(void)
{  
  DMA_InitTypeDef DMA_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    
  //GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);	
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  TIM_TimeBaseStructure.TIM_Period = 89;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;  
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC3Init(TIM3, &TIM_OCInitStructure);
  
  TIM_CtrlPWMOutputs(TIM3, ENABLE);
  
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  //TIM_DMAConfig(TIM3,TIM_DMABase_CCR3, 1);
  TIM_DMACmd(TIM3, TIM_DMA_Update, ENABLE);
  //TIM3->DMAR = 0x81;
  TIM_Cmd(TIM3, ENABLE);
  /*
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(TIM3->CCR3);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)led_ws2812;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
  DMA_InitStructure.DMA_BufferSize = ARRAY_LEN;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel2 , ENABLE ) ;
  DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);*/
  
  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  TIM3->CCER   |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
  TIM3->CCMR1  |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE;
  TIM3->CCMR2  |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;
  TIM3->DIER   |= TIM_DIER_CC1DE | TIM_DIER_CC3DE;
  TIM3->CR1    |= TIM_CR1_CEN | TIM_CR1_ARPE;
  TIM3->PSC = 0;
  TIM3->ARR = 89;
  TIM3->CCR3 = 0;
  DBGMCU->CR |= DBGMCU_CR_DBG_TIM3_STOP;
  
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;
  DMA1_Channel2->CPAR = (uint32_t) &TIM3->CCR3;
  DMA1_Channel2->CMAR = (uint32_t) &led_ws2812[0];
  DMA1_Channel2->CNDTR = ARRAY_LEN;
  DMA1_Channel2->CCR = DMA_CCR2_MINC | DMA_CCR2_CIRC | DMA_CCR2_DIR | DMA_CCR2_EN | DMA_CCR2_MSIZE_0 | DMA_CCR2_PSIZE_0;
  DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
  //NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /*
  for (uint16_t clr = DELAY_LEN; clr < ARRAY_LEN - 24 * 2; clr++)
  {    
    if (cnt < 7) 
      cnt++;
    else 
      cnt = 0;
    led_ws2812[clr] = led_pwm[85][cnt];
  }  */
  led_wheel(0, 127, 0, 0);
  led_wheel(1, 0, 127, 0);
  led_wheel(2, 0, 0, 127);
  led_wheel(3, 127, 0, 0);
  led_wheel(4, 127, 127, 0);
  led_wheel(5, 127, 0, 127);
  led_wheel(6, 0, 127, 127);
  led_wheel(7, 127, 0, 0);
  led_wheel(8, 127, 0, 0);
  led_wheel(9, 64, 64, 64);
  led_wheel(10, 0, 0, 0);
  led_wheel(11, 127, 127, 127);
}















