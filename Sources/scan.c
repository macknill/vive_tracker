#include "stm32f10x.h"
#include "scan.h"

#define PACK_FULL_TIME 8333
#define PACK_FULL_REJECT 5
#define PACK_STROBE_TIME 409
#define PACK_STROBE_REJECT 6

#define CHANNELS 3
#define IMP_DATA_SIZE 30


struct channel_struct sensor[CHANNELS];  



struct data_struct imp_data[CHANNELS][IMP_DATA_SIZE];
uint8_t imp_data_cnt[CHANNELS] = { 0, 0, 0 };

uint8_t scan(uint8_t ch, uint32_t *time, uint16_t *base, uint16_t *lenght)
{
    struct data_struct tempo_imp;
    get_imp(&tempo_imp, 0, 0);

    if ((tempo_imp.per > (PACK_STROBE_TIME - PACK_STROBE_REJECT))
        && (tempo_imp.per < (PACK_STROBE_TIME + PACK_STROBE_REJECT)))
    {
        sensor[ch].scan_cnt = 1;
    }
    else
    {
        if (sensor[ch].scan_cnt < 0)
        {
            uint16_t sum = tempo_imp.per;
            for (int16_t s = sensor[ch].scan_cnt; s < 0; s++)
            {
                get_imp(&tempo_imp, 0, s);
                sum += tempo_imp.per; 
            }
            if ((sum > (PACK_FULL_TIME - PACK_FULL_REJECT))
                && (sum < (PACK_FULL_TIME + PACK_FULL_REJECT)))
            {
                //printf("%d", tempo_imp.time);
                get_imp(&tempo_imp, 0, sensor[ch].scan_cnt);
                *time = tempo_imp.time;
                *base = tempo_imp.imp; 
                *lenght = tempo_imp.per;               
                get_imp(&tempo_imp, 0, sensor[ch].scan_cnt + 1);
                *lenght += tempo_imp.per;   
                return 1;
            }
            else if (sum > (PACK_FULL_TIME + PACK_FULL_REJECT))
            {
                sensor[ch].scan_cnt = 0;
            }
            else
            {
                sensor[ch].scan_cnt--;
            }

        }
    }
    return 0;
}

void get_imp(struct data_struct* temp, uint8_t ch, int8_t older)
{
    int8_t count = imp_data_cnt[ch];
    if (older < 0)
    {
        count += older;
        if (count < 0)
            count += (IMP_DATA_SIZE - 1);
    }
    temp->time = imp_data[ch][count].time;
    temp->imp = imp_data[ch][count].imp;
    temp->per = imp_data[ch][count].per;
}

void set_imp(uint32_t time, uint8_t ch, uint16_t per, uint16_t imp)
{
    uint8_t count = imp_data_cnt[ch];
    if (count < (IMP_DATA_SIZE - 1))
        count++;
    else
        count = 0;
    imp_data[ch][count].time = time;
    imp_data[ch][count].imp = imp;
    imp_data[ch][count].per = per;
    imp_data_cnt[ch] = count;
}
