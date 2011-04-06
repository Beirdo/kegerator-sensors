#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define TCN75A_ADDR 0x90

void tcn75a_setup(void)
{
    /* Get 11-bit resolution, continuous */
    i2c_write_16bit(TCN75A_ADDR, 0x0140 );  
}

uint16_t tcn75a_read( uint8_t regnum )
{
    uint16_t retval;

    if( regnum > 3 ) 
        return(0);

    /* Set to the temperature register */
    i2c_write_8bit(TCN75A_ADDR, regnum);     

    if( regnum == 1 )
        retval = i2c_read_8bit(TCN75A_ADDR);
    else
        retval = i2c_read_16bit(TCN75A_ADDR);

    return( retval );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

