// Port B GPIO
#define BLANK                   0
#define BST_SHDN                1
#define BT_RSTn                 2
#define BT_CSn                  3
#define DISP_CSn                4
#define MCU_DI                  5
#define MCU_DO                  6
#define MCU_USCK                7

// Port D GPIO
#define MCU_RXD                 0
#define MCU_TXD                 1
#define PNL_RPMn                2
#define BT_IRQn                 3
#define CSW_4n                  4
#define CSW_8n                  5
#define MCU_ICP                 6

// cylinder select switch
#define CSW_STATE               (PIND & ((1 << CSW_8n) | (1 << CSW_4n)))
#define CSW_STATE_4CYL          (1 << CSW_8n)
#define CSW_STATE_8CYL          (1 << CSW_4n)
#define CSW_STATE_6CYL          ((1 << CSW_8n) | (1 << CSW_4n))

// panel switch
#define PNL_STATE               (PIND & (1 << PNL_RPMn))
#define PNL_STATE_DWELL         (1 << PNL_RPMn)
#define PNL_STATE_RPM           0

// display state
#define DISPLAY_STATE           (PORTB & (1 << BST_SHDN))
#define DISPLAY_STATE_ON        0
#define DISPLAY_STATE_OFF       (1 << BST_SHDN)

void gpio_init (void);