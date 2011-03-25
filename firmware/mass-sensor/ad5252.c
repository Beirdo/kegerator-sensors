#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define AD5252_ADDR 0x58

void ad5252_setup(void)
{
    /* Restore RDAC1 from EEMEM */
    i2c_write_8bit( AD5252_ADDR, 0x89 );

    /* Restore RDAC3 from EEMEM */
    i2c_write_8bit( AD5252_ADDR, 0x8B );

    /* Send NOP */
    i2c_write_8bit( AD5252_ADDR, 0x80 );

    /* Set the WP pin as output, and turn off WP */
    DDRC  |= (1 << PC3);
    PORTC |= (1 << PC3);
}

void ad5252_set_rdac( uint8_t rdacnum, uint8_t value )
{
    if( rdacnum == 1 || rdacnum == 3 )
    {
        /* Write the value */
        i2c_write_16bit( AD5252_ADDR, (rdacnum << 8) | value );

        /* Commit to the EEMEM */
        i2c_write_8bit( AD5252_ADDR, 0x90 | rdacnum );
    }
}

uint8_t ad5252_get_rdac( uint8_t rdacnum )
{
    if( rdacnum == 1 || rdacnum == 3 )
        return( i2c_read_8bit_chained( AD5252_ADDR, rdacnum ) );

    return( 0 );
}

void ad5252_set_eemem( uint8_t address, uint8_t value )
{
    /* Write the value */
    i2c_write_16bit( AD5252_ADDR, (0x20 | (address & 0x0F) << 8) | value );
}

uint8_t ad5252_get_eemem( uint8_t address )
{
    return( i2c_read_8bit_chained( AD5252_ADDR, (0x20 | (address & 0x0F))) );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

