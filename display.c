#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "display.h"
#include "gpio.h"
#include "calc.h"
#include "spi.h"

static uint8_t display_mode;

void display_init (void)
{

    // clear display
    display_print_pair (0xFF, 0xFF);

    // enable display
    display_force_on ();

}

void display_print_num (uint16_t print_num)
{

    char display_temp[8];
    uint8_t display_upper, display_lower;

    // convert number to temporary string
    utoa (print_num, display_temp, 10);

    // pack both halves of display contents
    switch (strlen (display_temp))
    {

        case 0:

            display_upper = 0xFF;
            display_lower = 0xFF;
            break;

        case 1:

            display_upper = 0xFF;
            display_lower = 0xF0 + (display_temp[0] - 0x30);
            break;

        case 2:

            display_upper = 0xFF;
            display_lower = ((display_temp[0] - 0x30) << 4) + (display_temp[1] - 0x30);
            break;

        case 3:

            display_upper = 0xF0 + (display_temp[0] - 0x30);
            display_lower = ((display_temp[1] - 0x30) << 4) + (display_temp[2] - 0x30);
            break;

        default:

            display_upper = ((display_temp[0] - 0x30) << 4) + (display_temp[1] - 0x30);
            display_lower = ((display_temp[2] - 0x30) << 4) + (display_temp[3] - 0x30);

    } // switch (length)

    // print formatted number to display
    display_print_pair (display_upper, display_lower);

}

void display_print_pair (uint8_t print_upper, uint8_t print_lower)
{

    // assert chip select
    PORTB &= ~(1 << DISP_CSn);

    // print both halves of display contents
    spi_xfer (print_upper);
    spi_xfer (print_lower);

    // de-assert chip select
    PORTB |= (1 << DISP_CSn);

}

void display_update (void)
{

    static uint8_t display_index;
    uint16_t display_temp;

    // update display with either reading (unless engine is stalled)
    if (calc_rpm_get ())
    {

        // determine which reading to print based on panel switch selection
        if (PNL_STATE == PNL_STATE_RPM) display_print_num (calc_rpm_get ()); else display_print_num ((uint16_t)calc_dwell_get ());

    } else {

        // increment display index
        display_index++;

        // wrap display index
        display_index %= DISPLAY_WIDTH;

        // generate scrolling zero
        display_temp = ~(0xF << (display_index << 2));

        // print stall message
        display_print_pair ((uint8_t)(display_temp >> 8), (uint8_t)display_temp);

    } // if (stalled)

}

uint8_t display_mode_get (void)
{

    // get display mode
    return (display_mode);

}

void display_mode_set (uint8_t mode)
{

    // set display mode
    display_mode = mode;

}

void display_force_on (void)
{

    // force display on
    PORTB &= ~(1 << BST_SHDN);

}

void display_force_off (void)
{

    // force display off
    PORTB |= (1 << BST_SHDN);

}