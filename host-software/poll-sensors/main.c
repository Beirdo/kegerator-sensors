#include "local.h"
#include <unistd.h>
#include <stdio.h>

int main( int argc, char **argv )
{
    int count;

    serial_setup();
    count = sensor_setup();
    printf( "%d sensors found\n", count );

    if( !count )
        return( 0 );

    while( 1 )
    {
        sensor_poll();
        sleep(5);
    }

    return( 0 );
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
