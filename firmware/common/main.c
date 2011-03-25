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
#if FUSE_MEMORY_SIZE == 3
    .extended = EFUSE_DEFAULT,
#endif
};


int main(void) __attribute__((noreturn));

int main(void)
{
    uart_setup();
    i2c_setup();
    sensor_main_setup();

    _delay_ms(10);

    sei();

    sensor_local_setup();

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

