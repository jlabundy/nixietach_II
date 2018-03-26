#include <avr/io.h>

#include "gpio.h"

void gpio_init (void)
{

    // enable port B input pull-ups
    PORTB |= (1 << BLANK) | (1 << MCU_DI);

    // configure port B output states
    PORTB |= (1 << BST_SHDN) | (1 << BLE_RSTn) | (1 << BLE_CSn) | (1 << DISP_CSn);

    // enable port B output drivers
    DDRB |= (1 << BST_SHDN) | (1 << BLE_RSTn) | (1 << BLE_CSn) | (1 << DISP_CSn) | (1 << MCU_DO) | (1 << MCU_USCK);

    // enable port D input pull-ups
    PORTD |= (1 << PNL_RPMn) | (1 << BLE_IRQn) | (1 << CSW_4n) | (1 << CSW_8n);

}