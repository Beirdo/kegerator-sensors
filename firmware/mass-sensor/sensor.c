#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <string.h>

uint8_t sensor_address;
uint8_t sensor_in_use = 0;
uint8_t sensor_buf[MAX_BUF_LEN];
uint8_t sensor_size;
uint8_t sensor_type[] = "mass";

void sensor_main_setup(void)
{
    /* Make the DIP switch bits (PD7-4) inputs, pull up resistor enabled */
    DDRD  &= 0x0F;
    PORTD |= 0xF0;

    /* Let the inputs settle */
    _delay_ms(10);

    /* Read the DIP switch */
    sensor_address = PIND >> 4;
}

void sensor_local_setup(void)
{
    ad5252_setup();
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
    uint8_t  offset;

    sensor_in_use = 0;

    msg = (message *)u_rx_buf;

    if( msg->get_set )
    {
        switch( msg->subaddress )
        {
            case 0:
                return;
            case 1:     /* Tare weight of keg in g */
                if( sensor_size < 8 )
                    return;

                adc_tare_grams = *(uint16_t *)msg->data;
                eeprom_update_word( &ee_adc_tare_grams, adc_tare_grams );
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = 1;
                break;
            case 2:     /* RDAC1 from AD5252 via I2C */
                if( sensor_size < 7 )
                    return;

                ad5252_set_rdac(1, msg->data[0]);
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = 1;
                break;
            case 3:     /* RDAC2 from AD5252 via I2C */
                if( sensor_size < 7 )
                    return;

                ad5252_set_rdac(3, msg->data[0]);
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = 1;
                break;
            case 4:     /* Zero offset of ADC reading */
                if( sensor_size < 8 )
                    return;

                adc_zero_offset = *(uint16_t *)msg->data;
                eeprom_update_word( &ee_adc_zero_offset, adc_zero_offset );
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = 1;
                break;
            case 5:     /* Inverse slope of ADC readings */
                if( sensor_size < 8 )
                    return;

                adc_inverse_slope = *(uint16_t *)msg->data;
                eeprom_update_word( &ee_adc_inverse_slope, adc_inverse_slope );
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
                u_tx_size = 7 + strlen(sensor_fw_version) + 
                            strlen(sensor_type);
                memcpy(u_tx_buf, sensor_buf, 4);
                offset = 4;
                memcpy(&(u_tx_buf[offset]), sensor_type, strlen(sensor_type));
                offset += strlen(sensor_type);
                u_tx_buf[offset++] = ' ';
                memcpy(&(u_tx_buf[offset]), sensor_fw_version, 
                       strlen(sensor_fw_version));
                break;
            case 1:     /* Mass in g via ADC */
                u_tx_size = 8;
                memcpy(u_tx_buf, sensor_buf, 4);
                *(uint16_t *)&(u_tx_buf[4]) = adc_read();
                break;
            case 2:     /* RDAC1 from AD5252 via I2C */
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = ad5252_get_rdac(1);
                break;
            case 3:     /* RDAC2 from AD5252 via I2C */
                u_tx_size = 7;
                memcpy(u_tx_buf, sensor_buf, 4);
                u_tx_buf[4] = ad5252_get_rdac(3);
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

