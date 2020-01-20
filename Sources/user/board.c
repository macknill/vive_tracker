#include "board.h"

uint32_t BaordState = GO_ON;
//call in loop cycle
void PowerStart(void)
{
  mb.registers.one[mbREG_back_timer] = 10;
}

uint32_t GoOnStep = 0;
void PowerLogic(void)
{
  switch(BaordState)
  {
    case GO_ON:        
      if (mb.registers.one[mbREG_adc_Vbat] < 9000) 
      {
        mb.registers.one[mbREG_back_timer] = 10;
        mb.registers.one[mbREG_shutdown] = 1;  
        mb.registers.one[mbREG_PPM_PON] = 0;
        mb.registers.one[mbREG_SRV_PON] = 0;  
        BaordState = GO_OFF;
      }
      else
      {
        
      }
      break;
    case WORK:    
      if (mb.registers.one[mbREG_adc_Vbat] < 9000)              //if low battery
      {
        mb.registers.one[mbREG_back_timer] = 60;
        mb.registers.one[mbREG_shutdown] = 1;  
        mb.registers.one[mbREG_PPM_PON] = 0;
        mb.registers.one[mbREG_SRV_PON] = 0;
        BaordState = GO_OFF;
      }
      else
      {
        if (mb.registers.one[mbREG_button_cmd] == 1)            //soft off all board
        {
          mb.registers.one[mbREG_button_cmd] = 0;
          mb.registers.one[mbREG_back_timer] = 60;
          mb.registers.one[mbREG_shutdown] = 1; 
          mb.registers.one[mbREG_PPM_PON] = 0;
          mb.registers.one[mbREG_SRV_PON] = 0; 
        }
        else if (mb.registers.one[mbREG_button_cmd] == 2)       //hard off all board
        {                    
          mb.registers.one[mbREG_button_cmd] = 0;
          mb.registers.one[mbREG_JETSON_ON] = 0;
          mb.registers.one[mbREG_back_timer] = 3;
          mb.registers.one[mbREG_shutdown] = 1; 
          mb.registers.one[mbREG_PPM_PON] = 0;
          mb.registers.one[mbREG_SRV_PON] = 0; 
        }
      }
      //if ()
      break;
    case GO_OFF:
      if (mb.registers.one[mbREG_jetson_usb])
      {
        //mb.registers.one[mbREG_JETSON_ON] = 0;
        mb.registers.one[mbREG_JETSON_PON] = 0;
        mb.registers.one[mbREG_back_timer] = 3;
      }
      break;
    case BRD_OFF:
      if (!mb.registers.one[mbREG_back_timer])
      {
        mb.registers.one[mbREG_ALL_PON] = 0;
      }
      break;
  }
}

//need to call one per seconds
uint32_t BoardBackTimer(void)
{  
  if (mb.registers.one[mbREG_back_timer]) mb.registers.one[mbREG_back_timer]--;
  else 
    return 1;
  return 0;
}