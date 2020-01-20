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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_WriteBit(GPIOA, GPIO_Pin_15, 0);
}

#define TIM2_CCR1_Address    ((u32)0x40000000+0x34)
#define DMA_BUFF_SIZE 				2				

uint16_t buff[DMA_BUFF_SIZE];//�����
uint16_t volatile T;

void DMA1_Channel5_IRQHandler (void) 
{
  T = (buff[1] > buff[0]) ? (buff[1] - buff[0]) : (65535+ buff[1] - buff[0]);
	
  DMA1->IFCR |= DMA_IFCR_CGIF1;
}

uint32_t temp = 0;
void TIM2_IRQHandler(void)
{
  temp++;
  //TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
  //USART_ITConfig(USART1, USART_IT_RXNE, DISABLE); 
  //mb.flag |= 1;  
  //led_toggle();
}

void init_tim1_dma(void)
{  
  DMA_InitTypeDef DMA_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  /*DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(TIM2->CCR1);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = DMA_BUFF_SIZE;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);
  DMA_ITConfig(DMA1_Channel5, DMA_IT_TE, ENABLE);
  */
  
  RCC->AHBENR |= TIM2_CCR1_Address; //��������� ������������ ������� DMA ������
  DMA1_Channel5->CPAR = TIM2_CCR1_Address; //��������� ����� ��������� - ������� ���������� �������������� ��� ��� ���������� �������
  DMA1_Channel5->CMAR = (uint32_t)buff; //������ ����� ������ - ������� ����� ������� � RAM
  DMA1_Channel5->CCR &= ~DMA_CCR1_DIR; //��������� ����������� �������� ������, �� ��������� � ������
  DMA1_Channel5->CNDTR = DMA_BUFF_SIZE; //���������� ������������ ��������
  DMA1_Channel5->CCR &= ~DMA_CCR1_PINC; //����� ��������� �� �������������� ����� ������ ���������
  DMA1_Channel5->CCR |= DMA_CCR1_MINC; //����� ������ �������������� ����� ������ ���������.
  DMA1_Channel5->CCR |= DMA_CCR1_PSIZE_0; //����������� ������ ��������� - 16 ���
  DMA1_Channel5->CCR |= DMA_CCR1_MSIZE_0; //����������� ������ ������ - 16 ���
  DMA1_Channel5->CCR |= DMA_CCR1_PL; //��������� - ����� ������� 
  DMA1_Channel5->CCR |= DMA_CCR1_CIRC; //��������� ������ DMA � ����������� ������
  DMA1_Channel5->CCR |= DMA_CCR1_TCIE;//��������� ���������� �� ��������� ��������
  DMA1_Channel5->CCR |= DMA_CCR1_EN; //��������� ������ 1-�� ������ DMA
  //�������� ������������ ����� �, �������������� ������� � �������
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; 
	
  TIM2->PSC = 56000-1;//����� ������� 1Khz

  TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;//�������� TI1 ��� TIM2_CH1
  TIM2->CCMR1 &= ~(TIM_CCMR1_IC1F | TIM_CCMR1_IC1PSC);//�� ��������� � �������� �� ����������
	
  TIM2->CCER &= ~TIM_CCER_CC1P;//�������� ������ �� ��������� ������
  TIM2->CCER |= TIM_CCER_CC1E;//�������� ����� ������� ��� 4-�� ������         

  TIM2->DIER |= TIM_DIER_CC1DE;//��������� ����������� ������ � DMA
	
  TIM2->CR1 |= TIM_CR1_CEN; //�������� �������
  
  NVIC_InitTypeDef NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
/*
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);*/
}

















