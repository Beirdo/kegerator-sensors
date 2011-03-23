#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "local.h"

#define TCN75A_ADDR 0x90

void tcn75a_setup(void)
{
    /* Get 11-bit resolution, continuous */
    i2c_write_16bit(TCN75A_ADDR, 0x0140 );  
}

uint16_t tcn75a_read( uint8_t regnum )
{
    /* Set to the temperature register */
    i2c_write_8bit(TCN75A_ADDR, 0x00);     
    return( i2c_read_16bit(TCN75A_ADDR) );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

