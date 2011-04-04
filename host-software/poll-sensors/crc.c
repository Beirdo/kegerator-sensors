#include <stdint.h>
#include "local.h"

uint16_t crc16_update(uint16_t crc, uint8_t a)
{
	int i;

	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
	    if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
	    else
            crc = (crc >> 1);
	}

	return crc;
}

uint16_t calc_crc( uint8_t *buffer, uint8_t length )
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    for( i = 0; i < length; i++ )
        crc = crc16_update(crc, buffer[i]);

    return( crc );
}

uint16_t check_crc( uint8_t *buffer, uint8_t length )
{
    /* Assumes the last 2 bytes are the CRC16 */
    return( calc_crc(buffer, length) );
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
