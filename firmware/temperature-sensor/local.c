#include "local.h"
#include <avr/io.h>
#include <string.h>

uint8_t sensor_type[] = "temperature";

void sensor_main_setup(void)
{
    sensor_common_setup();
}

void sensor_local_setup(void)
{
    tcn75a_setup();
}

uint8_t sensor_handle_set( message *msg )
{
    /* Don't handle set at this time */
    return( 0 );
}

uint8_t sensor_handle_get( message *msg )
{
    uint8_t offset;
    uint16_t value;

    switch( msg->subaddress )
    {
        case 0:     /* Firmware version */
            u_tx_size = 7 + strlen((char *)sensor_fw_version) + 
                        strlen((char *)sensor_type);
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            offset = 4;
            memcpy(&(u_tx_buf[offset]), sensor_type, 
                   strlen((char *)sensor_type));
            offset += strlen((char *)sensor_type);
            u_tx_buf[offset++] = ' ';
            memcpy(&(u_tx_buf[offset]), sensor_fw_version, 
                   strlen((char *)sensor_fw_version));
            break;
        case 1:     /* Temperature sensor over I2C */
            u_tx_size = 8;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            value = tcn75a_read(0);
            u_tx_buf[4] = value >> 8;
            u_tx_buf[5] = value & 0xFF;
            break;
        default:
            return( 0 );
    }

    return( 1 );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

