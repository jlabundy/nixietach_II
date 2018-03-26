#include <avr/io.h>
#include <stdint.h>

#include "spi.h"

void spi_init (void)
{

    // configure USI peripheral for SPI mode 0
    USICR |= (1 << USIWM0) | (2 << USICS0);

}

uint8_t spi_xfer (uint8_t spi_dout)
{

    // load buffer
    USIDR = spi_dout;

    // clear counter overflow
    USISR |= (1 << USIOIF);

    // strobe clock
    while (!(USISR & (1 << USIOIF))) USICR |= (1 << USICLK) | (1 << USITC);

    // return buffer contents
    return (USIDR);

}