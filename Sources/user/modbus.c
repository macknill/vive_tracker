#include "modbus.h"
#include "main.h"
#include "mbcrc.h"
#include "stm32f10x.h"

struct modbus_struct mb;

const unsigned char fctsupported[] =
{
    //MB_FC_READ_COILS,
    //MB_FC_READ_DISCRETE_INPUT,
    MB_FC_READ_REGISTERS,
    MB_FC_READ_INPUT_REGISTER,
    //MB_FC_WRITE_COIL,
    MB_FC_WRITE_REGISTER,
    //MB_FC_WRITE_MULTIPLE_COILS,
    MB_FC_WRITE_MULTIPLE_REGISTERS,
    MB_FC_REBOOT,
    //MB_FC_LENGHT_FIRMWARE,
    //MB_FC_FIRMWARE_PART
};

void mb_sendTxBuffer(void)
{
  uint16_t u16crc = calcCRC(&mb.u8BufferOut[0], mb.u16OutCnt );
  mb.u8BufferOut[ mb.u16OutCnt ] = lowByte(u16crc);
  mb.u16OutCnt++;
  mb.u8BufferOut[ mb.u16OutCnt ] = highByte(u16crc);
  mb.u16OutCnt++;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  DMA_SetCurrDataCounter(DMA1_Channel4, mb.u16OutCnt);
  DMA_Cmd(DMA1_Channel4, ENABLE);
  mb.u16timeOut = mb.u16time;
  mb.u16time = 0;
  /*
  DMA1_Channel4->CCR &=~ DMA_CCR_EN;      //выключение дћј
  DMA1_Channel4->CNDTR = mb.u16OutCnt;    //—колько байт надо отправить?
  DMA1_Channel4->CCR |= DMA_CCR_EN;       //начало отправки
  */
}

uint16_t word(uint8_t high, uint8_t low)
{
    return ((uint16_t)(high) << 8) | low;
}
    
uint8_t mb_validateRequest(void)
{
    // check message crc vs calculated crc
    volatile uint16_t u16CalcCRC = calcCRC( &mb.u8BufferIn[0], mb.u16InCnt - 2 ); // combine the crc Low & High bytes
    volatile uint16_t u16MsgCRC = word(mb.u8BufferIn[mb.u16InCnt - 1], mb.u8BufferIn[mb.u16InCnt - 2]); // combine the crc Low & High bytes
    if (u16CalcCRC != u16MsgCRC)
    {
        mb.u16errCnt ++;
        return NO_REPLY;
    }

    // check fct code
    uint8_t isSupported = 0;
    for (uint8_t i = 0; i< sizeof( fctsupported ); i++)
    {
        if (fctsupported[i] == mb.u8BufferIn[FUNC])
        {
            isSupported = 1;
            break;
        }
    }
    if (!isSupported)
    {
        mb.u16errCnt ++;
        return EXC_FUNC_CODE;
    }

    // check start address & nb range
    uint16_t u16regs = 0;
    uint8_t u8regs;
    switch ( mb.u8BufferIn[ FUNC ] )
    {
    /*case MB_FC_READ_COILS:
    case MB_FC_READ_DISCRETE_INPUT:
    case MB_FC_WRITE_MULTIPLE_COILS:
        u16regs = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ]) / 16;
        u16regs += word( au8Buffer[ NB_HI ], au8Buffer[ NB_LO ]) /16;
        u8regs = (uint8_t) u16regs;
        if (u8regs > u8regsize) return EXC_ADDR_RANGE;
        break;
    case MB_FC_WRITE_COIL:
        u16regs = word( au8Buffer[ ADD_HI ], au8Buffer[ ADD_LO ]) / 16;
        u8regs = (uint8_t) u16regs;
        if (u8regs > u8regsize) return EXC_ADDR_RANGE;
        break;*/
    case MB_FC_WRITE_REGISTER :
        u16regs = word( mb.u8BufferIn[ ADD_HI ], mb.u8BufferIn[ ADD_LO ]);
        u8regs = (uint8_t) u16regs;
        if (u8regs > mb.u8regsize) return EXC_ADDR_RANGE;
        break;
    case MB_FC_READ_REGISTERS :
    case MB_FC_READ_INPUT_REGISTER :
    case MB_FC_WRITE_MULTIPLE_REGISTERS :
        u16regs = word( mb.u8BufferIn[ ADD_HI ], mb.u8BufferIn[ ADD_LO ]);
        u16regs += word( mb.u8BufferIn[ NB_HI ], mb.u8BufferIn[ NB_LO ]);
        u8regs = (uint8_t) u16regs;
        if (u8regs > mb.u8regsize) return EXC_ADDR_RANGE;
        break;
  /*      
    case MB_FC_REBOOT:
    case MB_FC_LENGHT_FIRMWARE:
    case MB_FC_FIRMWARE_PART:
        mb.u16speed = 0;
        break;*/
    }
    
    return 0; // OK, no exception code thrown
}
      
/**
 * @brief
 * This method processes functions 3 & 4
 * This method reads a word array and transfers it to the master
 *
 * @return u8BufferOutSize Response to master length
 * @ingroup register
 */
int8_t process_FC3( uint16_t *regs, uint8_t u8size )
{

    uint8_t u8StartAdd = word( mb.u8BufferIn[ ADD_HI ], mb.u8BufferIn[ ADD_LO ] );
    uint8_t u8regsno = word( mb.u8BufferIn[ NB_HI ], mb.u8BufferIn[ NB_LO ] );
    uint8_t u8CopyBufferSize;
    uint8_t i;
    mb.u8BufferOut[ 2 ]       = u8regsno * 2;
    mb.u16OutCnt         = 3;

    for (i = u8StartAdd; i < u8StartAdd + u8regsno; i++)
    {
        mb.u8BufferOut[ mb.u16OutCnt ] = highByte(regs[i]);
        mb.u16OutCnt++;
        mb.u8BufferOut[ mb.u16OutCnt ] = lowByte(regs[i]);
        mb.u16OutCnt++;
    }
    u8CopyBufferSize = mb.u16OutCnt +2;
    mb_sendTxBuffer();

    return u8CopyBufferSize;
}


/**
 * @brief
 * This method processes function 6
 * This method writes a value assigned by the master to a single word
 *
 * @return u8BufferOutSize Response to master length
 * @ingroup register
 */
int8_t process_FC6( uint16_t *regs, uint8_t u8size )
{

    uint8_t u8add = word( mb.u8BufferIn[ ADD_HI ],  mb.u8BufferIn[ ADD_LO ] );
    uint8_t u8CopyBufferSize;
    uint16_t u16val = word( mb.u8BufferIn[ NB_HI ], mb.u8BufferIn[ NB_LO ] );

    mb.registers.one[ u8add ] = u16val;
    if (u8add < SAVE_REGISTERS_SIZE) mb.flag |= 0x02;
    

    // keep the same header
    mb.u16OutCnt         = RESPONSE_SIZE;

    u8CopyBufferSize = mb.u16OutCnt +2;
    mb_sendTxBuffer();

    return u8CopyBufferSize;
}



/**
 * @brief
 * This method processes function 16
 * This method writes a word array assigned by the master
 *
 * @return u8BufferOutSize Response to master length
 * @ingroup register
 */
int8_t process_FC16( uint16_t *regs, uint8_t u8size )
{
    uint16_t u8StartAdd = mb.u8BufferIn[ ADD_HI ] << 8 | mb.u8BufferIn[ ADD_LO ];
    uint8_t u8regsno = mb.u8BufferIn[ NB_HI ] << 8 | mb.u8BufferIn[ NB_LO ];
    uint8_t u8CopyBufferSize;
    uint8_t i;
    uint16_t temp;
    if (u8StartAdd < SAVE_REGISTERS_SIZE) mb.flag |= 0x02;
    // build header
    mb.u8BufferOut[ ADD_HI ]   = u8StartAdd >> 8;
    mb.u8BufferOut[ ADD_LO ]   = u8StartAdd & 0xFF;
    mb.u8BufferOut[ NB_HI ]   = 0;
    mb.u8BufferOut[ NB_LO ]   = u8regsno;
    mb.u16OutCnt         = RESPONSE_SIZE;

    // write registers
    for (i = 0; i < u8regsno; i++)
    {
        temp = word(
                   mb.u8BufferIn[ (BYTE_CNT + 1) + i * 2 ],
                   mb.u8BufferIn[ (BYTE_CNT + 2) + i * 2 ]);

        regs[ u8StartAdd + i ] = temp;
    }
    u8CopyBufferSize = mb.u16OutCnt +2;
    mb_sendTxBuffer();

    return u8CopyBufferSize;
}


int8_t processes_bootloader( uint16_t *regs, uint8_t u8size )
{
    uint8_t u8func = mb.u8BufferIn[ FUNC ];  // get the original FUNC code
    
    switch (u8func)
    {
      case MB_FC_REBOOT:
        mb.u8BufferOut[ 2 ]   = 0;
        mb.u8BufferOut[ 3 ]   = 1;
        mb.u16OutCnt  = 4;
        mb.flag |= 4;   //reboot;
        break;
    }
    
    mb_sendTxBuffer();

    return  mb.u16OutCnt;
}

void mb_buildException( uint8_t u8exception )
{
    uint8_t u8func = mb.u8BufferIn[ FUNC ];  // get the original FUNC code

    mb.u8BufferOut[ ID ]      = mb.u8id;
    mb.u8BufferOut[ FUNC ]    = u8func + 0x80;
    mb.u8BufferOut[ 2 ]       = u8exception;
    mb.u16OutCnt         = EXCEPTION_SIZE;
}

int8_t mb_poll(void)
{
  if (mb.u16InCnt < 4)  return 0;
  
  // check slave id
  if (mb.u8BufferIn[ ID ] != mb.u8id) return 0;
    
  // validate message: CRC, FCT, address and size
  uint8_t u8exception = mb_validateRequest();
  if (u8exception > 0)
  {
      if (u8exception != NO_REPLY)
      {
          mb_buildException( u8exception );
          mb_sendTxBuffer();
      }
      mb.u8lastError = u8exception;
      return u8exception;
  }
  // process message
  mb.u8BufferOut[0] = mb.u8BufferIn[ID];
  mb.u8BufferOut[1] = mb.u8BufferIn[FUNC];
  
  switch( mb.u8BufferIn[ FUNC ] )
  {
  //case MB_FC_READ_COILS:
  //case MB_FC_READ_DISCRETE_INPUT:
  //    return process_FC1( regs, u8size );
  //    break;
  case MB_FC_READ_INPUT_REGISTER:
  case MB_FC_READ_REGISTERS :
      return process_FC3( &mb.registers.one[0], mb.u8regsize );
      break;
  //case MB_FC_WRITE_COIL:
  //    return process_FC5( regs, u8size );
  //    break;
  case MB_FC_WRITE_REGISTER :
      return process_FC6( &mb.registers.one[0], mb.u8regsize );
      break;
  //case MB_FC_WRITE_MULTIPLE_COILS:
  //    return process_FC15( regs, u8size );
  //    break;
  case MB_FC_WRITE_MULTIPLE_REGISTERS :
      return process_FC16( &mb.registers.one[0], mb.u8regsize );
      break;
  case MB_FC_REBOOT :
      return processes_bootloader( &mb.registers.one[0], mb.u8regsize );
      break;
  case MB_FC_LENGHT_FIRMWARE :
      return processes_bootloader( &mb.registers.one[0], mb.u8regsize );
      break;
  case MB_FC_FIRMWARE_PART :
      return processes_bootloader( &mb.registers.one[0], mb.u8regsize );
      break;
  default:
      break;
  }
  return 1;
}
