
#define ADC_CH_CNT 5
#define ADC_ARR_LENGT 199


extern uint32_t adc_flag;
extern uint16_t adc_data[ADC_ARR_LENGT*2][ADC_CH_CNT];
void led_toggle(void);
void init_ws2812(void);

extern uint32_t toggle50hz;
extern uint32_t timer50hz;
extern uint32_t toggle1hz;
extern uint32_t timer1hz;

struct adc_math
{
  uint16_t ch[ADC_CH_CNT];
  float V_bat;
  float I_bat;
  float V_jet;
  float I_jet;
  float V_5v;
  float Wh_bat;
  float Wh_jet;
  float W_bat;
  float W_jet;
};