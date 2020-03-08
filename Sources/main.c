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
#include "scan.h"

    
volatile uint32_t ticks_delay = 0;
volatile uint32_t tick_ms = 0;
void delay(uint32_t milliseconds);



union package_union {
uint16_t two;
uint8_t one[2];
};

union package_union uart_package[9];

int main(void)
{
  SystemInit();
  init_gpio();
  SysTick_Config(SystemCoreClock/1000);
  init_signal_gpio_timers_dma();
  init_uart();
  
  volatile uint16_t ch1_old_lenght = 0;
  
  uart_package[0].two = 0xAAAA;
  uart_package[1].one[0] = 0x02;
  uart_package[1].one[1] = 0x2A;
  uart_package[2].two = 8000;
  uart_package[3].two = 6000;
  uart_package[4].two = 7000;
  uart_package[5].two = 0123;
  uart_package[6].two = 4567;
  uart_package[7].two = 8910;
  uart_package[8].two = 0x5555;
  
  sensor[0].dma = DMA_IR_CH1;
  sensor[1].dma = DMA_IR_CH2;
  sensor[2].dma = DMA_IR_CH3;
  while (1)
  {
    IWDG_ReloadCounter(); 
    //led_toggle();
    /*delay(100);
    volatile uint16_t temp_lenght = DMA_IR_CH2->CNDTR;
    for(uint32_t j = 0; j < 9; j++)
    {
      tx_buffer[j*2] = uart_package[j].one[0];
      tx_buffer[j*2 + 1] = uart_package[j].one[1];
    }
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel4, 18);
    DMA_Cmd(DMA1_Channel4, ENABLE);
    */
    for (uint8_t ch = 0; ch < IR_CHANNELS; ch++)
    {
      if (sensor[ch].irCnt != DMA_GetCurrDataCounter(sensor[ch].dma))
      {
        uint16_t addr = DMA_GetCurrDataCounter(sensor[ch].dma);
        if(addr < DMA_BUFF_SIZE)
        if (ir_buffer[ch][addr] == 0)
        {
          ir_buffer[ch][addr] == 1;
          if (addr == 0)
            set_imp(tick_ms, ch, ir_buffer[ch][DMA_BUFF_SIZE - 1], ir_buffer[ch][DMA_BUFF_SIZE - 2]);
          else if (addr < 1)
            set_imp(tick_ms, ch, ir_buffer[ch][addr - 1], ir_buffer[ch][DMA_BUFF_SIZE - 1]);
          else
            set_imp(tick_ms, ch, ir_buffer[ch][addr - 1], ir_buffer[ch][addr - 2]);
          if (scan(ch, &sensor[ch].time, &sensor[ch].irBase, &sensor[ch].irLenght))
          {
            uint8_t temp = 0;
          }
        }
      }
    }
    //
    //if (dma_cnt) 
    //scan(0);
  }
}

void delay(uint32_t milliseconds) {
  uint64_t start = ticks_delay;
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
