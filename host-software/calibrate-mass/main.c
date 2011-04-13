#include "local.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int sensornum;
uint16_t capacity;
uint16_t tare;
int tared = 0;

int main( int argc, char **argv )
{
    char temp[2];
    char *dummy;
    int count;
    uint8_t val1;
    uint8_t val2;
    uint16_t reading, reading1, reading2;
    uint16_t target;
    uint16_t invslope;

    if( argc < 3 ) {
        printf("You must indicate which sensor address and capacity\n\n");
        return( 1 );
    }

    sensornum = atoi(argv[1]);
    capacity  = atoi(argv[2]);
    if( argc >= 4 ) {
        tare = atoi(argv[3]);
        tared = 1;
    }

    serial_setup();
    count = sensor_setup(sensornum);
    printf( "%d sensors found\n", count );

    if( !count )
        return( 0 );

    /* Off to do the calibration */
    sensor_set_reg(1, 0);   /* Zero the tare weight */
    printf( "Remove all weight from the scale, and hit enter\n" );
    dummy = fgets( temp, 1, stdin );

    val1 = sensor_poll_reg(2);
    reading = sensor_poll(4);

    /* First, decrease the offset until it hits 0, or we get the lowest reading
     * we can
     */
    for( ; val1 && reading ; val1-- )
    {
        sensor_set_reg(2, val1);
        reading = sensor_poll(4);
    }

    /* Now increase until non-zero */
    for( ; val1 < 255 && !reading ; val1++ )
    {
        sensor_set_reg(2, val1);
        reading = sensor_poll(4);
    }

    sensor_set_reg(2, --val1);
    reading1 = sensor_poll(4);

    printf("Place the 2kg calibration weight on the scale and hit enter\n");
    dummy = fgets( temp, 1, stdin );

    target = (0x3FFF * 2000) / capacity;
    printf("Target reading = %04X\n", target);

    val2 = sensor_poll_reg(3);
    reading = sensor_poll(4);

    /* If we are above target, reduce the gain until it works */
    for( ; reading > target; val2-- )
    {
        sensor_set_reg(3, val2);
        reading = sensor_poll(4);
    }

    /* If we are below target, increase gain until it works */
    for( ; reading < target; val2++ )
    {
        sensor_set_reg(3, val2);
        reading = sensor_poll(4);
    }

    sensor_set_reg(3, --val2);
    reading2 = sensor_poll(4);

    /* Set the zero offset */
    sensor_set_reg(4, reading1);
    
    /* Scaled up by 256 */
    invslope = (uint16_t)((uint32_t)(2000 * 256)) / (reading2 - reading1);
    sensor_set_reg(5, invslope);

    reading = sensor_poll(1);
    printf("Should be 2000, or dang close: %d\n", reading);

    printf( "Remove all weight from the scale, and hit enter\n" );
    dummy = fgets( temp, 1, stdin );

    reading = sensor_poll(1);
    printf("Should be 0, or dang close: %d\n", reading);

    if( tared ) {
        printf("Setting tare value to %d\n", tare);
        sensor_set_reg(1, tare);
    }

    return( 0 );
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
