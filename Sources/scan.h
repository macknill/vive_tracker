

#define CHANNELS 3

struct channel_struct
{
    DMA_Channel_TypeDef* dma;
    uint16_t irCnt;
    uint16_t irBase;
    uint16_t irLenght;
    int8_t scan_cnt;
    uint32_t time;
};

struct data_struct
{
    uint32_t time;
    uint16_t imp;
    uint16_t per;
};
uint8_t scan(uint8_t ch, uint32_t *time, uint16_t *base, uint16_t *lenght);
void set_imp(uint32_t time, uint8_t ch, uint16_t per, uint16_t imp);
void get_imp(struct data_struct* temp, uint8_t ch, int8_t older);

extern struct channel_struct sensor[CHANNELS];  
