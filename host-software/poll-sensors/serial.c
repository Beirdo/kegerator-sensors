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

int read_fd  = -1;
int write_fd = -1;

int serial_setup(void)
{
    struct termios pTermios;

    read_fd  = open( SERIAL_PORT, O_RDWR );
    write_fd = read_fd; /* open( SERIAL_PORT, O_WRONLY ); */

    printf( "Using %s: read fd %d, write fd %d\n", SERIAL_PORT, read_fd, write_fd);

    cfmakeraw( &pTermios );
    cfsetospeed( &pTermios, B38400 );
    cfsetispeed( &pTermios, 0 );

    tcsetattr( read_fd,  TCSANOW, &pTermios );
    tcsetattr( write_fd, TCSANOW, &pTermios );

    return( (read_fd <= 0) || (write_fd <= 0) );
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

    return( write( write_fd, buf, len ) );
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

    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while( totlen < 2 )
    {
        FD_SET(read_fd, &fds);
        if( select( read_fd+1, &fds, NULL, NULL, &tv ) <= 0 )
            return( -1 );

        len = read( read_fd, buf, 2-totlen );
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
        FD_SET(read_fd, &fds);
        if( select( read_fd+1, &fds, NULL, NULL, &tv ) <= 0 )
            return( -1 );

        len = read( read_fd, buf, pktlen-totlen );
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
