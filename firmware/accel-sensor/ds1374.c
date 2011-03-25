#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

#define DS1374_ADDR 0xB0

void ds1374_setup(void)
{
    uint8_t setup_buf[4] = { 0x07, 0x08, 0x00, 0xAB };

    /* Disable watchdog and squarewave output, enable trickle charge with
     * diode + 4k resistor
     */
    i2c_write_buffer(DS1374_ADDR, setup_buf, 4);  
}

uint32_t ds1374_read(void)
{
    uint32_t now;

    /* Read the 32bit RTC value */
    i2c_read_buffer_chained(DS1374_ADDR, 0x00, (uint8_t *)&now, 4);

    return( now );
}

void ds1374_write(uint32_t now)
{
    uint8_t buf[5];

    buf[0] = 0x00;
    memcpy(&(buf[1]), &now, 4);

    /* Write the subaddress, then 32bit RTC value */
    i2c_write_buffer(DS1374_ADDR, buf, 5);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

