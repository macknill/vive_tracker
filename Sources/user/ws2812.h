
#define DELAY_LEN 48
#define LED_COUNT 12
#define ARRAY_LEN DELAY_LEN + LED_COUNT*24

void led_wheel(uint8_t led, uint8_t r ,uint8_t g, uint8_t b);
void led_loop(void);

extern uint16_t led_ws2812[ARRAY_LEN];
extern const uint16_t led_pwm[128][8];