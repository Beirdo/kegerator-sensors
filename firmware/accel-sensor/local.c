#include "local.h"
#include <avr/io.h>
#include <string.h>

uint8_t sensor_type[] = "accelerometer";

void sensor_main_setup(void)
{
    sensor_common_setup();
}

void sensor_local_setup(void)
{
    ad7991_setup();
    ds1374_setup();
}

uint8_t sensor_handle_set( message *msg )
{
    uint32_t now;

    switch( msg->subaddress )
    {
        case 0:     /* Firmware version */
            return( 0 );
        case 1:     /* Set RTC */
            if( sensor_size < 10 )
                return( 0 );

            now = *(uint32_t *)msg->data;
            ds1374_write(now);
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = 1;
            break;
        default:
            return( 0 );
    }

    return( 1 );
}

uint8_t sensor_handle_get( message *msg )
{
    uint8_t offset;
    accel_t  readings;
    uint32_t now;

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
        case 1:  /* 3D accelerometer ADC (3 channels) over I2C, remote 0 */
            u_tx_size = 16;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            readings = ad7991_read(0);
            now = ds1374_read();
            memcpy(&(u_tx_buf[4]), &now, 4);
            memcpy(&(u_tx_buf[8]), &readings, 6);
            break;
        case 2:  /* 3D accelerometer ADC (3 channels) over I2C, remote 1 */
            u_tx_size = 12;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            readings = ad7991_read(1);
            now = ds1374_read();
            memcpy(&(u_tx_buf[4]), &now, 4);
            memcpy(&(u_tx_buf[8]), &readings, 6);
            break;
        default:
            return( 0 );
    }

    return( 1 );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

