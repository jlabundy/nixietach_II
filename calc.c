#include <avr/io.h>
#include <stdint.h>

#include "calc.h"
#include "gpio.h"

static uint16_t calc_rpm_buffer[CALC_AVG_DEPTH];
static uint16_t calc_rpm;

static uint8_t calc_dwell_buffer[CALC_AVG_DEPTH];
static uint8_t calc_dwell;

static uint8_t calc_rpm_wptr, calc_dwell_wptr;

void calc_rpm_dwell_upd (uint16_t ICR1p, uint16_t ICR1n)
{

    uint32_t m1, m2, n;

    uint32_t sum1;
    uint16_t sum2;

    uint8_t i;

    // form divisor
    n = (uint32_t)ICR1p + (uint32_t)ICR1n;

    // determine dividends based on number of cylinders
    switch (CSW_STATE)
    {

        case CSW_STATE_8CYL:

            m1 = CALC_RPM_8CYL;
            m2 = ((uint32_t)ICR1p + CALC_DWELL_OFFSET) * CALC_THETA_8CYL;
            break;

        case CSW_STATE_6CYL:

            m1 = CALC_RPM_6CYL;
            m2 = ((uint32_t)ICR1p + CALC_DWELL_OFFSET) * CALC_THETA_6CYL;
            break;

        case CSW_STATE_4CYL:

            m1 = CALC_RPM_4CYL;
            m2 = ((uint32_t)ICR1p + CALC_DWELL_OFFSET) * CALC_THETA_4CYL;
            break;

        default:

            m1 = 0;
            m2 = 0;

    } // switch (cylinder select)

    // store RPM calculation and manage write pointer
    calc_rpm_buffer[calc_rpm_wptr++] = (uint16_t)(m1 / n);
    calc_rpm_wptr %= CALC_AVG_DEPTH;

    // store dwell calculation and manage write pointer
    calc_dwell_buffer[calc_dwell_wptr++] = (uint8_t)(m2 / n);
    calc_dwell_wptr %= CALC_AVG_DEPTH;

    // zero out sums
    sum1 = 0;
    sum2 = 0;

    // build sums
    for (i = 0; i < CALC_AVG_DEPTH; i++)
    {

        sum1 += (uint32_t)calc_rpm_buffer[i];
        sum2 += (uint16_t)calc_dwell_buffer[i];

    } // for (i)

    // calculate readings as latest moving averages
    calc_rpm = (uint16_t)(sum1 >> CALC_AVG_LEVEL);
    calc_dwell = (uint8_t)(sum2 >> CALC_AVG_LEVEL);

}

void calc_rpm_dwell_clear (void)
{

    uint8_t i;

    // flush moving-average buffers
    for (i = 0; i < CALC_AVG_DEPTH; i++)
    {

        calc_rpm_buffer[i] = 0;
        calc_dwell_buffer[i] = 0;

    } // for (i)

    // zero out readings to indicate stall condition
    calc_rpm = 0;
    calc_dwell = 0;

}

uint16_t calc_rpm_get (void)
{

    // return most recently calculated RPM if timer 1 has not overflowed
    if (TIFR & (1 << TOV1)) return (0); else return (calc_rpm);

}

uint8_t calc_dwell_get (void)
{

    // return most recently calculated dwell if timer 1 has not overflowed
    if (TIFR & (1 << TOV1)) return (0); else return (calc_dwell);

}