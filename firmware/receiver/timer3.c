#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define PRESCALE    256 
#define BAUD        38400
#define DELAY_BITS  100
#define TIMER_TOP ((F_CPU * DELAY_BITS) / (PRESCALE * BAUD))
/* Comes to 81 for 8MHz clock, 100 bit delay at 38400 baud */

void timer3_setup(void)
{
    /* No output pin, CTC mode, 256 prescale */
    TCCR3A = 0;
    TCCR3B = (1 << WGM32) || (1 << CS32);

    /* Setup the timer top */
    OCR3AH = (TIMER_TOP >> 8);
    OCR3AL = (TIMER_TOP & 0xFF);

    /* Clear the timer */
    TCNT3H = 0;
    TCNT3L = 0;

    /* Disable its interrrupt */
    timer3_disable();
}

void timer3_enable(void)
{
    TCNT3H = 0;
    TCNT3L = 0;

    ETIMSK |= (1 << OCIE3A);
}

void timer3_disable(void)
{
    ETIMSK &= ~(1 << OCIE3A);
}

ISR(SIG_OUTPUT_COMPARE3A)
{
    uart0_rx_timeout();
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

