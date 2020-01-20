/**
  ******************************************************************************
  * @file    IWDG/IWDG_Reset/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "inits.h"
#include "modbus.h"
#include "mb_regs.h"
#include "main.h"

uint16_t adc_end[5];
uint32_t adc_flag = 0;
uint16_t adc_data[ADC_ARR_LENGT*2][ADC_CH_CNT];

uint32_t buttonStateTimer = 0;

uint32_t toggle50hz = 0;
uint32_t timer50hz = 0;
uint32_t toggle1hz = 0;
uint32_t timer1hz = 0;
uint8_t update50hz = 0;

struct adc_math board_va;


int main(void)
{
  PWR->CR |= PWR_CR_PLS_0 | PWR_CR_PLS_1 | PWR_CR_PLS_2;
  PWR->CR |= PWR_CR_PVDE;
  while(PWR->CSR & PWR_CSR_PVDO);
  SystemInit();
  //init_sysclk();
  //IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  __disable_irq ();
  init_modbus(115200);
  init_gpio();
  init_adc(); 
  init_ppm();
  mb.u8id = 1;
  init_ws2812();
  __enable_irq ();   
  
  /*GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);*/
  while (1)
  {
    IWDG_ReloadCounter(); 
    if(mb.flag & 0x01)
    {
      mb.flag &=~ 1;
      mb_poll();
      USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
      mb.u16InCnt = 0;
    }   
    
    if (adc_flag & 2)
    {
      adc_flag &=~ 2;
      uint16_t part = 0;
      if (adc_flag & 1)
        part = ADC_ARR_LENGT;
      uint32_t adc_middle[ADC_CH_CNT];
      for (uint16_t cnt_ch = 0; cnt_ch < ADC_CH_CNT; cnt_ch++)
      {
        adc_middle[cnt_ch] = 0;
        for (uint16_t cnt = 0; cnt < ADC_ARR_LENGT; cnt++)
        {
          adc_middle[cnt_ch] += adc_data[cnt + part][cnt_ch];
        }
        adc_end[cnt_ch] = adc_middle[cnt_ch] / ADC_ARR_LENGT;
        board_va.ch[cnt_ch] = adc_end[cnt_ch];
      }
      board_va.V_bat = board_va.ch[0] * (3.3 / 4096.0) * 11;
      board_va.I_bat = (board_va.ch[1] * (3.3 / 4096.0) - 1.65) / 0.066;
      board_va.V_jet = board_va.ch[3] * (3.3 / 4096.0) * 11;
      board_va.I_jet = (board_va.ch[2] * (3.3 / 4096.0) - 1.65) / 0.066;
      board_va.V_5v = board_va.ch[4] * (3.3 / 4096.0) * 2;
      board_va.W_bat = board_va.I_bat * board_va.V_bat;
      board_va.W_jet = board_va.I_jet * board_va.V_bat;
      board_va.Wh_bat += board_va.W_bat /60/60/ 50;
      board_va.Wh_jet += board_va.W_jet /60/60/ 50;
    } 
    
    if (update50hz != toggle50hz)
    {
      update50hz = toggle50hz;      
      
      GPIO_WriteBit(GPIOC, GPIO_Pin_14, mb.registers.one[mbREG_ALL_PON]);
      GPIO_WriteBit(GPIOA, GPIO_Pin_6, mb.registers.one[mbREG_JETSON_PON]);
      GPIO_WriteBit(GPIOB, GPIO_Pin_1, mb.registers.one[mbREG_JETSON_ON]);
      GPIO_WriteBit(GPIOB, GPIO_Pin_5, !mb.registers.one[mbREG_PPM_PON]);
      if (!mb.registers.one[mbREG_PPM_PON]) //do not on servo is ppm off
        mb.registers.one[mbREG_SRV_PON] = 0;      
      GPIO_WriteBit(GPIOA, GPIO_Pin_5, mb.registers.one[mbREG_SRV_PON]);
      
      TIM_SetCompare1(TIM4, mb.registers.one[mbREG_PPM_4]);
      TIM_SetCompare2(TIM4, mb.registers.one[mbREG_PPM_3]);
      TIM_SetCompare3(TIM4, mb.registers.one[mbREG_PPM_2]);
      TIM_SetCompare4(TIM4, mb.registers.one[mbREG_PPM_1]);
      
      mb.registers.one[mbREG_adc_Vbat] = (uint16_t)(board_va.V_bat * 1000);
      mb.registers.one[mbREG_adc_Ibat] = (int16_t)(board_va.I_bat * 1000);
      mb.registers.one[mbREG_adc_Vjet] = (uint16_t)(board_va.V_jet * 1000);
      mb.registers.one[mbREG_adc_Ijet] = (int16_t)(board_va.I_jet * 1000);
      mb.registers.one[mbREG_adc_5V] = (uint16_t)(board_va.V_5v * 1000);
      mb.registers.one[mbREG_adc_Wh_bat] = (uint16_t)(board_va.Wh_bat);
      mb.registers.one[mbREG_adc_Wh_jet] = (uint16_t)(board_va.Wh_jet);
      mb.registers.one[mbREG_adc_W_bat] = (uint16_t)(board_va.W_bat*1000);
      mb.registers.one[mbREG_adc_W_jet] = (uint16_t)(board_va.W_jet*1000);
      
      mb.registers.one[mbREG_button] = !GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15);
      mb.registers.one[mbREG_jetson_usb] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15);
      
      
      mb.registers.one[mbREG_mb_timeout] = mb.u16timeOut;
      mb.u16time++;
      
      if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15))   //button function
      {
        if (timer1hz > 10)
          buttonStateTimer++;
      }
      else
      {
        if (buttonStateTimer > 125)
        {
          mb.registers.one[mbREG_button_cmd] = 2;
          buttonStateTimer = 0;
        }
        else if (buttonStateTimer > 20)
        {          
          mb.registers.one[mbREG_button_cmd] = 1;
          buttonStateTimer = 0;
        }
        else
        {
          if (buttonStateTimer)
             buttonStateTimer--;
        }
      }    
      
      led_loop();
    }
    
    if (mb.registers.one[mbREG_button_cmd] == 2)
       mb.registers.one[mbREG_ALL_PON] = 0;
  }
}

void led_toggle(void)
{
  if (GPIOC->ODR & GPIO_ODR_ODR13)
    GPIOC->BSRR |= GPIO_BSRR_BR13; 
  else
    GPIOC->BSRR |= GPIO_BSRR_BS13;  
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}

#endif

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
