#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

/* Receiver is at 0xFF */
uint8_t sensor_address = 0xFF;
uint8_t sensor_in_use = 0;
uint8_t sensor_buf[MAX_BUF_LEN];
uint8_t sensor_size;
uint8_t sensor_data_size;
uint8_t sensor_type[] = "receiver";

void sensor_handle_fast(void)
{
    memcpy(sensor_buf, u0_rx_buf, u0_rx_size);
    sensor_size = u0_rx_size;
    sensor_data_size = u0_rx_data_size;
    sensor_in_use = 1;
}

void sensor_handle(void)
{
    message *msg;
    uint16_t crc;
    uint8_t  offset;

    sensor_in_use = 0;

    msg = (message *)u0_rx_buf;

    if( msg->get_set )
    {
        /* Don't handle set at this time */
        return;
    }

    switch( msg->subaddress )
    {
        case 0:     /* Firmware version */
            u0_tx_size = 7 + strlen(sensor_fw_version) + strlen(sensor_type);
            memcpy(u0_tx_buf, sensor_buf, 4);
            offset = 4;
            memcpy(&(u0_tx_buf[offset]), sensor_type, strlen(sensor_type));
            offset += strlen(sensor_type);
            u0_tx_buf[offset++] = ' ';
            memcpy(&(u0_tx_buf[offset]), sensor_fw_version, 
                   strlen(sensor_fw_version));
            break;
        case 1:     /* Temperature sensor over I2C */
            u0_tx_size = 8;
            memcpy(u0_tx_buf, sensor_buf, 4);
            *(uint16_t *)&(u0_tx_buf[4]) = tcn75a_read(0);
            break;
        default:
            return;
    }

    u0_tx_buf[1] = u0_tx_size;
    memset(&(u0_tx_buf[u0_tx_size-2]), 0x00, 2);

    crc = calc_crc( u0_tx_buf, u0_tx_size );

    *(uint16_t *)&(u0_tx_buf[u0_tx_size-2]) = crc;

    uart0_transmit();
    uart0_restart_rx();
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

