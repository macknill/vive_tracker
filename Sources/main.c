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
#include "main.h"

    
volatile uint32_t ticks_delay = 0;
void delay(uint32_t milliseconds);



int main(void)
{
  SystemInit();
  init_gpio();
  SysTick_Config(SystemCoreClock/1000);
  init_signal_gpio_timers_dma();
  while (1)
  {
    IWDG_ReloadCounter(); 
    led_toggle();
    delay(1000);
  }
}

void delay(uint32_t milliseconds) {
  uint32_t start = ticks_delay;
  while((ticks_delay - start) < milliseconds);
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
