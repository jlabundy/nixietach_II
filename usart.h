#define USART_RX_SIZE           8

extern volatile char usart_rx_buffer[USART_RX_SIZE];
extern volatile uint8_t usart_rx_rptr, usart_rx_wptr;

void usart_init (void);

void usart_print_char (char print_char);
void usart_print_str (char *print_str);
void usart_print_str_P (char *print_str);