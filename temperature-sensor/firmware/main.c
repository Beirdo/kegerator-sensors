#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>

int main(void) __attribute__((noreturn));

int main(void)
{
    uart_setup();
    i2c_setup();
    sensor_setup();

    _delay_ms(10);

    sei();

    tcn75a_setup();

    set_sleep_mode(SLEEP_MODE_IDLE);

    /* Sit on this and rotate, don't return */
    while(1)
    {
        sleep_mode();

        if( sensor_in_use )
        {
            sensor_handle();
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

