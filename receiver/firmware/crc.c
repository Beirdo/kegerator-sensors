#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "local.h"

#include <util/crc16.h>

uint16_t check_crc( uint8_t *buffer, uint8_t length )
{
    /* Assumes the last 2 bytes are the CRC16 */
    return( calc_crc(buffer, length) );
}

uint16_t calc_crc( uint8_t *buffer, uint8_t length )
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    for( i = 0; i < length; i++ )
        crc = _crc16_update(crc, buffer[i]);

    return( crc );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

