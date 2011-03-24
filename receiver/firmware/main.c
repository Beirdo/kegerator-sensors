#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/signature.h>
#include <avr/fuse.h>
#include <util/delay.h>
#include <stdint.h>

FUSES =
{
    .low = (FUSE_CKSEL3 & FUSE_CKSEL1 & FUSE_CKSEL0 & FUSE_SUT1 & FUSE_SUT0 &
            FUSE_BODEN),
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};


int main(void) __attribute__((noreturn));

int main(void)
{
    uart0_setup();
    uart1_setup();
    i2c_setup();

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

