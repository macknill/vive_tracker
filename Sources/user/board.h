#include "stm32f10x.h"
#include "inits.h"
#include "modbus.h"
#include "mb_regs.h"
#include "main.h"

enum BoardState {
    START_ON,
    GO_ON,             //power on process
    WORK,               //work now
    GO_OFF,             //process power off
    BRD_OFF
};

