#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>

uint8_t sensor_address;
uint8_t sensor_in_use = 0;
uint8_t sensor_buf[MAX_BUF_LEN];
uint8_t sensor_size;

void sensor_setup(void)
{
    /* Make the DIP switch bits (PD7-4) inputs, pull up resistor enabled */
    DDRD  &= 0x0F;
    PORTD |= 0xF0;

    /* Let the inputs settle */
    _delay_ms(10);

    /* Read the DIP switch */
    sensor_address = PIND >> 4;
}

void sensor_handle_fast(void)
{
    memcpy(sensor_buf, u_rx_buf, u_rx_size);
    sensor_size = u_rx_size;
    sensor_in_use = 1;
}

void sensor_handle(void)
{
    message *msg;
    uint16_t crc;
    accel_t  readings;
    uint32_t now;

    sensor_in_use = 0;

    msg = (message *)u_rx_buf;

    if( msg->get_set )
    {
        switch( msg->subaddress )
        {
            case 0:     /* Firmware version */
                return;
            case 1:     /* Set RTC */
                if( sensor_size < 10 )
                    return;

                now = *(uint32_t *)msg->data;
                ds1374_write(now);
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = 1;
                break;
            default:
                return;
        }
    }
    else
    {
        switch( msg->subaddress )
        {
            case 0:     /* Firmware version */
                u_tx_size = 6 + strlen(sensor_fw_version);
                memcpy(u_tx_buf, sensor_buf, 4);
                memcpy(&(u_tx_buf[4]), sensor_fw_version, 
                       strlen(sensor_fw_version));
                break;
            case 1:  /* 3D accelerometer ADC (3 channels) over I2C, remote 0 */
                u_tx_size = 16;
                memcpy(u_tx_buf, sensor_buf, 4);
                readings = ad7991_read(0);
                now = ds1374_read();
                memcpy(&(u_tx_buf[4]), &now, 4);
                memcpy(&(u_tx_buf[8]), &readings, 6);
                break;
            case 2:  /* 3D accelerometer ADC (3 channels) over I2C, remote 1 */
                u_tx_size = 12;
                memcpy(u_tx_buf, sensor_buf, 4);
                readings = ad7991_read(1);
                now = ds1374_read();
                memcpy(&(u_tx_buf[4]), &now, 4);
                memcpy(&(u_tx_buf[8]), &readings, 6);
                break;
            default:
                return;
        }
    }

    u_tx_buf[1] = u_tx_size;
    memset(&(u_tx_buf[u_tx_size-2]), 0x00, 2);

    crc = calc_crc( u_tx_buf, u_tx_size );

    *(uint16_t *)&(u_tx_buf[u_tx_size-2]) = crc;

    uart_transmit(0xFF);
    uart_restart_rx();
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

