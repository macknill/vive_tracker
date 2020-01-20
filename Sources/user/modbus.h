#include <stdint.h>

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

typedef struct
{
    uint8_t u8id;          /*!< Slave address between 1 and 247. 0 means broadcast */
    uint8_t u8fct;         /*!< Function code: 1, 2, 3, 4, 5, 6, 15 or 16 */
    uint16_t u16RegAdd;    /*!< Address of the first register to access at slave/s */
    uint16_t u16CoilsNo;   /*!< Number of coils or registers to access */
    uint16_t *au16reg;     /*!< Pointer to memory image in master */
}
modbus_t;

enum
{
    RESPONSE_SIZE = 6,
    EXCEPTION_SIZE = 3,
    CHECKSUM_SIZE = 2
};

/**
 * @enum MESSAGE
 * @brief
 * Indexes to telegram frame positions
 */
enum MESSAGE
{
    ID                             = 0, //!< ID field
    FUNC, //!< Function code position
    ADD_HI, //!< Address high byte
    ADD_LO, //!< Address low byte
    NB_HI, //!< Number of coils or registers high byte
    NB_LO, //!< Number of coils or registers low byte
    BYTE_CNT  //!< byte counter
};

/**
 * @enum MB_FC
 * @brief
 * Modbus function codes summary.
 * These are the implement function codes either for Master or for Slave.
 *
 * @see also fctsupported
 * @see also modbus_t
 */
enum MB_FC
{
    MB_FC_NONE                     = 0,
    MB_FC_READ_COILS               = 1,
    MB_FC_READ_DISCRETE_INPUT      = 2,
    MB_FC_READ_REGISTERS           = 3,
    MB_FC_READ_INPUT_REGISTER      = 4,
    MB_FC_WRITE_COIL               = 5,
    MB_FC_WRITE_REGISTER           = 6,
    MB_FC_WRITE_MULTIPLE_COILS     = 15,
    MB_FC_WRITE_MULTIPLE_REGISTERS = 16,
    MB_FC_REBOOT                   = 170,
    MB_FC_LENGHT_FIRMWARE          = 171,
    MB_FC_FIRMWARE_PART            = 172
};

enum COM_STATES
{
    COM_IDLE                     = 0,
    COM_WAITING                  = 1
};

enum ERR_LIST
{
    ERR_NOT_MASTER                = -1,
    ERR_POLLING                   = -2,
    ERR_BUFF_OVERFLOW             = -3,
    ERR_BAD_CRC                   = -4,
    ERR_EXCEPTION                 = -5
};

enum
{
    NO_REPLY = 255,
    EXC_FUNC_CODE = 1,
    EXC_ADDR_RANGE = 2,
    EXC_REGS_QUANT = 3,
    EXC_EXECUTE = 4
};

#define  T35  5
#define  SAVE_REGISTERS_SIZE  40
#define  REGISTERS_SIZE  100
#define  BUFFER_TX_SIZE  REGISTERS_SIZE*2 + 20
#define  BUFFER_RX_SIZE  256 + 20	

union twobyte{
  float flt[REGISTERS_SIZE/2];
  uint16_t one[REGISTERS_SIZE];
  uint8_t two[REGISTERS_SIZE * 2];
};

struct modbus_struct{
  uint8_t u8id; //!< 0=master, 1..247=slave number
  uint16_t u16speed;
  uint8_t u8state;
  uint8_t u8lastError;
  uint8_t u8BufferOut[BUFFER_TX_SIZE];
  uint8_t u8BufferIn[BUFFER_RX_SIZE];
  uint16_t u16BufferInCount;
  uint8_t u8lastRec;
  uint16_t *au16regs;
  uint16_t u16InCnt, u16OutCnt, u16errCnt;
  uint16_t u16timeOut, u16time;
  uint8_t u8regsize;
  uint8_t flag;
  union twobyte registers;
  uint16_t time;
};

extern struct modbus_struct mb;

int8_t mb_poll(void);