#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define PRESCALE    256 
#define BAUD        38400
#define DELAY_BITS  100
#define TIMER_TOP ((F_CPU * DELAY_BITS) / (PRESCALE * BAUD))
/* Comes to 81 for 8MHz clock, 100 bit delay at 38400 baud */

void timer_setup(void)
{
    /* No output pin, CTC mode, 256 prescale */
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) || (1 << CS12);

    /* Setup the timer top */
    OCR1AH = (TIMER_TOP >> 8);
    OCR1AL = (TIMER_TOP & 0xFF);

    /* Clear the timer */
    TCNT1H = 0;
    TCNT1L = 0;

    /* Disable its interrrupt */
    timer_disable();
}

void timer_enable(void)
{
    TCNT1H = 0;
    TCNT1L = 0;

    TIMSK |= (1 << OCIE1A);
}

void timer_disable(void)
{
    TIMSK &= ~(1 << OCIE1A);
}

ISR(SIG_OUTPUT_COMPARE1A)
{
    uart_rx_timeout();
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

