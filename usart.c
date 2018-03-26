#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>

#include "usart.h"

void usart_init (void)
{

    // set baud rate to 19.2 kbps based on 16-MHz core clock
    UBRRH = 0x00;
    UBRRL = 0x33;

    // enable receiver interrupt, receiver and transmitter
    UCSRB |= (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);

    // configure 8-bit character size
    UCSRC |= (3 << UCSZ0);

}

void usart_print_char (char print_char)
{

    // wait until transmit buffer is free
    while (!(UCSRA & (1 << UDRE)));

    // send character
    UDR = print_char;

}

void usart_print_str (char *print_str)
{

    uint8_t i;
    i = 0;

    // print each character
    while (print_str[i]) usart_print_char (print_str[i++]);

}

void usart_print_str_P (char *print_str)
{

    uint8_t i;
    i = 0;

    // print each character (from program memory)
    while (pgm_read_byte (&print_str[i])) usart_print_char (pgm_read_byte (&print_str[i++]));

}

ISR (USART_RX_vect)
{

    // queue character and increment write pointer
    usart_rx_buffer[usart_rx_wptr++] = UDR;

    // wrap write pointer
    usart_rx_wptr %= USART_RX_SIZE;

}