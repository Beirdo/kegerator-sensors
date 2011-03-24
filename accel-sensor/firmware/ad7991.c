#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define AD7991_ADDR 0x50

void ad7991_setup(void)
{
    /* Scan 3 channels, with Vref = Vin3 */
    i2c_write_8bit(AD7991_ADDR, 0x78);  

    /* Set PB0 and PB1 as outputs, for gating the I2C to each remote.  The
     * IOs are active high, so drive both low for now
     */
    DDRB  |= (1 << PB1) | (1 << PB0);
    PORTB &= ~((1 << PB1) | (1 << PB0));
}

accel_t ad7991_read(uint8_t remotenum)
{
    uint16_t raw[3];
    accel_t  readings = { 0, 0, 0 }; 
    uint8_t  i;
    uint8_t  chan;
    uint16_t value;

    if( remotenum == 0 )
    {
        /* Enable the appropriate I2C bus */
        PORTB |= (1 << PB0);
    } 
    else if( remotenum == 1 )
    {
        /* Enable the appropriate I2C bus */
        PORTB |= (1 << PB1);
    } 
    else 
    {
        return( readings );
    }

    /* Read the three readings in order.  The AD7991 starts the reading on 
     * demand and does it in parallel with the reads
     */
    i2c_read_buffer(AD7991_ADDR, (uint8_t *)&raw[0], 6);     
    
    /* Disable the I2C bus gating */
    PORTB &= ~((1 << PB1) | (1 << PB0));

    for( i = 0; i < 3; i++ )
    {
        value = raw[i] & 0x0FFF;
        chan  = (raw[i] >> 12);

        if( chan == 0 )
            readings.x = value;
        else if( chan == 1 )
            readings.y = value;
        else
            readings.z = value;
    }

    return( readings );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

