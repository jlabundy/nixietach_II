#define TIMER0_TICK_RATE        25

#define TIMER1_EDGE_NONE        0x00
#define TIMER1_EDGE_POS         0x01
#define TIMER1_EDGE_NEG         0x02
#define TIMER1_EDGE_POS_TRASH   0x81
#define TIMER1_EDGE_NEG_TRASH   0x82

extern volatile uint8_t timer0_tick_flag, timer1_edge_type;

void timer0_init (void);
void timer1_init (void);