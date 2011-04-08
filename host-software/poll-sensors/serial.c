#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/select.h>
#include <sys/time.h>
#include "local.h"
#include <stdio.h>

#define SERIAL_PORT	"/dev/kegerator"

int serial_fd = -1;

int serial_setup(void)
{
    struct termios pTermios;

    if( serial_fd != -1 )
    {
        serial_fd = -1;
        close( serial_fd );
    }

    serial_fd  = open( SERIAL_PORT, O_RDWR );

    printf( "Using %s: fd %d\n", SERIAL_PORT, serial_fd );

    cfmakeraw( &pTermios );
    cfsetospeed( &pTermios, B38400 );
    cfsetispeed( &pTermios, 0 );

    tcsetattr( serial_fd,  TCSANOW, &pTermios );

    return( (serial_fd <= 0) );
}

int serial_write( uint8_t *buf, int len )
{
    /* The buffer is expected to have 2 extra bytes at the end for CRC */
    uint16_t crc;

    crc = calc_crc(buf, len);
    buf[len++] = (uint8_t)(crc & 0xFF);
    buf[len++] = (uint8_t)(crc >> 8);

    buffer_dump( "Sent: ", buf, len );
    crc = calc_crc(buf, len);

    return( write( serial_fd, buf, len ) );
}

int serial_read( uint8_t *buf, int maxlen )
{
    /* The buffer is expected to be big enough to contain the while packet */
    int len;
    int totlen = 0;
    int pktlen;
    uint8_t *bufptr = buf;
    fd_set fds;
    struct timeval tv;
    int retval;

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while( totlen < 2 )
    {
        FD_SET(serial_fd, &fds);
        if( (retval = select( serial_fd+1, &fds, NULL, NULL, &tv )) <= 0 )
            return( retval );

        len = read( serial_fd, buf, 2-totlen );
        if( len <= 0 ) 
            return( len );

        buf    += len;
        totlen += len;
    }

    pktlen = bufptr[1];   /* the second byte is the length */
    if( pktlen < 6 || pktlen > maxlen )
        return( -1 );

    while( totlen < pktlen )
    {
        FD_SET(serial_fd, &fds);
        if( (retval = select( serial_fd+1, &fds, NULL, NULL, &tv )) <= 0 )
            return( retval );

        len = read( serial_fd, buf, pktlen-totlen );
        if( len <= 0 ) 
            return( len );

        buf    += len;
        totlen += len;
    }

    if( check_crc(bufptr, totlen) )
        return( -1 );

    return( totlen );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
