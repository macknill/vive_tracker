
#ifndef DMA_IR_CH1
#define DMA_IR_CH1 DMA1_Channel5
#define DMA_IR_CH2 DMA1_Channel6
#define DMA_IR_CH3 DMA1_Channel1
#define IR_CHANNELS 				3	
#define DMA_BUFF_SIZE 				90
#define TX_BUFFER_SIZE 50
#endif


void init_sysclk(void);
void init_gpio(void);
void init_signal_gpio_timers_dma(void);



extern uint8_t tx_buffer[TX_BUFFER_SIZE];
extern uint16_t ir_buffer[IR_CHANNELS][DMA_BUFF_SIZE];

