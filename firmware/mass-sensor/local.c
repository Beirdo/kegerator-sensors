#include "local.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>

uint8_t sensor_type[] = "mass";

void sensor_main_setup(void)
{
    sensor_common_setup();
}

void sensor_local_setup(void)
{
    ad5252_setup();
    adc_setup();
}

uint8_t sensor_handle_set( message *msg )
{
    switch( msg->subaddress )
    {
        case 0:
            return( 0 );
        case 1:     /* Tare weight of keg in g */
            if( sensor_size < 8 )
                return( 0 );

            adc_tare_grams = (msg->data[0] << 8) | msg->data[1];
            eeprom_update_word( &ee_adc_tare_grams, adc_tare_grams );
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = 1;
            break;
        case 2:     /* RDAC1 from AD5252 via I2C */
            if( sensor_size < 7 )
                return( 0 );

            ad5252_set_rdac(1, msg->data[0]);
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = 1;
            break;
        case 3:     /* RDAC2 from AD5252 via I2C */
            if( sensor_size < 7 )
                return( 0 );

            ad5252_set_rdac(3, msg->data[0]);
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = 1;
            break;
        case 4:     /* Zero offset of ADC reading */
            if( sensor_size < 8 )
                return( 0 );

            adc_zero_offset = (msg->data[0] << 8) | msg->data[1];
            eeprom_update_word( &ee_adc_zero_offset, adc_zero_offset );
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = 1;
            break;
        case 5:     /* Inverse slope of ADC readings */
            if( sensor_size < 8 )
                return( 0 );

            adc_inverse_slope = (msg->data[0] << 8) | msg->data[1];
            eeprom_update_word( &ee_adc_inverse_slope, adc_inverse_slope );
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
        case 1:     /* Mass in g via ADC */
            u_tx_size = 8;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            value = adc_read();
            u_tx_buf[4] = value >> 8;
            u_tx_buf[5] = value & 0xFF;
            break;
        case 2:     /* RDAC1 from AD5252 via I2C */
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = ad5252_get_rdac(1);
            break;
        case 3:     /* RDAC2 from AD5252 via I2C */
            u_tx_size = 7;
            memcpy(u_tx_buf, (uint8_t *)sensor_buf, 4);
            u_tx_buf[4] = ad5252_get_rdac(3);
            break;
        default:
            return( 0 );
    }

    return( 1 );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

