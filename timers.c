#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "timers.h"

void timer0_init (void)
{

    // place timer 0 in CTC mode
    TCCR0A |= (2 << WGM00);

    // set timer 0 clock source to core clock divided by 1,024
    TCCR0B |= (5 << CS00);

    // enable timer 0 compare match A interrupt
    TIMSK |= (1 << OCIE0A);

    // set compare match A frequency to 100 Hz based on 16-MHz core clock
    OCR0A = 0x9C;

}

void timer1_init (void)
{

    // set timer 1 clock source to core clock divided by 8
    TCCR1B |= (2 << CS10);

    // enable timer 1 input capture interrupt
    TIMSK |= (1 << ICIE1);

    // reset edge type
    timer1_edge_type = TIMER1_EDGE_NONE;

}

ISR (TIMER0_COMPA_vect)
{

    static uint8_t timer0_tick;

    // set heartbeat flag based on divided-down tick rate
    if (timer0_tick < TIMER0_TICK_RATE)
    {

        timer0_tick++;

    } else {

        timer0_tick = 0;
        timer0_tick_flag = 1;

    } // if (tick)

}

ISR (TIMER1_CAPT_vect)
{

    // reset counter
    TCNT1 = 0x0000;

    // record edge type based on edge-detect polarity
    if (TCCR1B & (1 << ICES1))
    {

        // mark positive edge as invalid if timer previously overflowed
        if (TIFR & (1 << TOV1))
        {

            TIFR |= (1 << TOV1);
            timer1_edge_type = TIMER1_EDGE_POS_TRASH;

        } else {

            timer1_edge_type = TIMER1_EDGE_POS;

        } // if (overflow)

    } else {

        // mark negative edge as invalid if timer previously overflowed
        if (TIFR & (1 << TOV1))
        {

            TIFR |= (1 << TOV1);
            timer1_edge_type = TIMER1_EDGE_NEG_TRASH;

        } else {

            timer1_edge_type = TIMER1_EDGE_NEG;

        } // if (overflow)

    } // if (polarity)

    // invert edge-detect polarity
    TCCR1B ^= (1 << ICES1);

    // clear interrupt flag following polarity change (as per data sheet)
    TIFR |= (1 << ICF1);

}