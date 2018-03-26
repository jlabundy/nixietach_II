#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdlib.h>

#include "usart.h"
#include "cmd.h"
#include "timers.h"
#include "gpio.h"
#include "calc.h"
#include "spi.h"
#include "display.h"
#include "ble.h"

volatile char usart_rx_buffer[USART_RX_SIZE];
volatile uint8_t usart_rx_rptr, usart_rx_wptr;

volatile uint8_t timer0_tick_flag, timer1_edge_type;

char ble_rx_buffer[BLE_RX_SIZE];
uint8_t ble_rx_rptr, ble_rx_wptr;

int main (void)
{

    char cmd_char;
    char cmd_buffer_usart[CMD_BUFFER_SIZE + 1];
    char cmd_buffer_ble[CMD_BUFFER_SIZE + 1];

    uint8_t cmd_wptr_usart, cmd_wptr_ble, timer0_tock;

    uint16_t ICR1p, ICR1n;

    // reset write pointers
    cmd_wptr_usart = 0;
    cmd_wptr_ble = 0;

    // prepare for new period
    ICR1p = 0;
    ICR1n = 0;

    // initialize USART
    usart_init ();

    // initialize timers
    timer0_init ();
    timer1_init ();

    // initialize GPIO
    gpio_init ();

    // initialize SPI
    spi_init ();

    // enable interrupts
    sei ();

    // initialize display
    display_init ();

    // reset BLE module
    ble_reset ();

    // executive loop
    while (1)
    {

        // determine if more characters are available from USART (i.e. pointers not aligned)
        if (usart_rx_rptr != usart_rx_wptr)
        {

            // store character and manage write pointer
            cmd_char = usart_rx_buffer[usart_rx_rptr++];
            usart_rx_rptr %= USART_RX_SIZE;

            // cancel USART logging or process character
            if (cmd_log_target_get () == CMD_INTF_USART)
            {

                // reset logging target interface
                cmd_log_target_set (CMD_INTF_NONE, CMD_INTF_USART);

            } else {

                // process character
                cmd_wptr_usart = cmd_char_process (cmd_wptr_usart, cmd_char, cmd_buffer_usart, CMD_INTF_USART);

            } // if (logging)

        } // if (pointers not aligned)

        // determine if more characters are available from BLE (i.e. pointers not aligned)
        if (ble_rx_rptr != ble_rx_wptr)
        {

            // store character and manage write pointer
            cmd_char = ble_rx_buffer[ble_rx_rptr++];
            ble_rx_rptr %= BLE_RX_SIZE;

            // process character
            cmd_wptr_ble = cmd_char_process (cmd_wptr_ble, cmd_char, cmd_buffer_ble, CMD_INTF_BLE);

        } // if (pointers not aligned)

        // service heartbeat flag
        if (timer0_tick_flag)
        {

            // reset heartbeat flag
            timer0_tick_flag = 0;

            // poll BLE module
            ble_send_cmd (BLE_CMD_TYPE_UARTRX, NULL);

            // update display (if not in demo mode)
            if (display_mode_get () == DISPLAY_MODE_NORM) display_update ();

            // log RPM and dwell at tocker rate (if target is selected)
            if ((cmd_log_target_get () != CMD_INTF_NONE) && (timer0_tock++ & 0x01)) cmd_log (cmd_log_target_get ());

        } // if (heartbeat flag)

        // service input capture interrupt, if available
        switch (timer1_edge_type)
        {

            case TIMER1_EDGE_NONE:

                // simply continue waiting for next edge
                break;

            case TIMER1_EDGE_NEG:

                // reset edge type
                timer1_edge_type = TIMER1_EDGE_NONE;

                // ICR1 represents +width, record if waiting for new period (ICR1p = ICR1n = 0)
                if (!ICR1p && !ICR1n)
                {

                    // briefly disable interrupts
                    cli ();

                    // record +width
                    ICR1p = ICR1;

                    // re-enable interrupts
                    sei ();

                } else {

                    // edge was unexpected, prepare for new period
                    ICR1p = 0;
                    ICR1n = 0;

                } // if (new period)

                break;

            case TIMER1_EDGE_POS:

                // reset edge type
                timer1_edge_type = TIMER1_EDGE_NONE;

                // ICR1 represents -width, record if in middle of period (ICR1p > 0 and ICR1n = 0)
                if (ICR1p && !ICR1n)
                {

                    // briefly disable interrupts
                    cli ();

                    // record -width
                    ICR1n = ICR1;

                    // re-enable interrupts
                    sei ();

                    // update readings based on newly completed period (ICR1p > 0 and ICR1n > 0)
                    calc_rpm_dwell_upd (ICR1p, ICR1n);

                } // if (middle of period)

                // prepare for new period (whether edge was expected or not)
                ICR1p = 0;
                ICR1n = 0;

                break;

            default:

                // reset edge type
                timer1_edge_type = TIMER1_EDGE_NONE;

                // ICR1 is invalid (stall condition), prepare for new period
                ICR1p = 0;
                ICR1n = 0;

                // clear readings to indicate stall condition
                calc_rpm_dwell_clear ();

        } // switch (edge type)

    } // while (1)

}