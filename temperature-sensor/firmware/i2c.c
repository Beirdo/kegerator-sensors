#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <string.h>
#include <util/twi.h>

typedef enum {
    TW_MASTER_TX,
    TW_MASTER_RX,
    TW_SLAVE_TX,
    TW_SLAVE_RX
} i2c_mode_t;

typedef enum {
    I2C_START,
    I2C_SEND_ADDR,
    I2C_SEND_DATA,
    I2C_RECEIVE_DATA,
    I2C_STOP,
    I2C_DONE,
    I2C_ERROR
} i2c_state_t;


i2c_state_t i2c_state;
i2c_mode_t i2c_mode;
uint8_t i2c_slaveaddr;
uint8_t i2c_tx_data[MAX_BUF_LEN];
uint8_t i2c_tx_index;
uint8_t i2c_tx_size;
uint8_t i2c_rx_data[MAX_BUF_LEN];
uint8_t i2c_rx_index;
uint8_t i2c_rx_size;
uint8_t i2c_chain;
uint8_t i2c_twcr_val;

void i2c_setup(void)
{
    TWSR = 0;       /* No prescaling, please */
    TWBR = 3;       /* 400kHz I2C clock */
    i2c_twcr_val = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
}

void i2c_write_8bit( uint8_t addr, uint8_t data )
{
    i2c_tx_data[0] = data;
    i2c_write_buffer( addr, i2c_tx_data, 1 );
}

void i2c_write_16bit( uint8_t addr, uint16_t data )
{
    i2c_tx_data[0] = (data >> 8);
    i2c_tx_data[1] = (data & 0xFF);
    i2c_write_buffer( addr, i2c_tx_data, 2 );
}

void i2c_write_buffer( uint8_t addr, uint8_t *buffer, uint8_t bytes )
{
    bytes = MIN(bytes, MAX_BUF_LEN);

    if( buffer != i2c_tx_data )
        memcpy( i2c_tx_data, buffer, bytes );

    i2c_tx_size = bytes;

    i2c_mode = TW_MASTER_TX;
    i2c_state = I2C_START;
    i2c_slaveaddr = addr;

    TWCR = i2c_twcr_val | (1 << TWSTA);

    while( i2c_state != I2C_DONE && i2c_state != I2C_ERROR )
    {
        sleep_mode();
    }
}

uint16_t i2c_read_16bit( uint8_t addr )
{
    uint16_t retval;

    i2c_read_buffer(addr, 2);
    retval = (i2c_rx_data[0] << 8) | i2c_rx_data[1];
    return( retval );
}

void i2c_read_buffer( uint8_t addr, uint8_t bytes )
{
    i2c_mode = TW_MASTER_RX;
    i2c_state = I2C_START;
    i2c_chain = 0;
    i2c_rx_size = bytes;
    i2c_slaveaddr = addr;

    TWCR = i2c_twcr_val | (1 << TWSTA);

    while( i2c_state != I2C_DONE && i2c_state != I2C_ERROR )
    {
        sleep_mode();
    }
}

void i2c_master_tx_do_state(void)
{
    uint8_t twst;
    twst = TW_STATUS;

    switch( i2c_state )
    {
        case I2C_START:
            if( twst == TW_REP_START || twst == TW_START )
            {
                i2c_state = I2C_SEND_ADDR;
                TWDR = i2c_slaveaddr | TW_WRITE;
                TWCR = i2c_twcr_val;
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_SEND_ADDR:
            if( twst == TW_MT_SLA_ACK )
            {
                i2c_state = I2C_SEND_DATA;
                i2c_tx_index = 0;
                TWDR = i2c_tx_data[i2c_tx_index++];
                TWCR = i2c_twcr_val;
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_SEND_DATA:
            if( twst == TW_MT_DATA_ACK )
            {
                if( i2c_tx_index < i2c_tx_size )
                {
                    TWDR = i2c_tx_data[i2c_tx_index++];
                    TWCR = i2c_twcr_val;
                }
                else if( i2c_chain ) 
                {
                    i2c_mode = TW_MASTER_RX;
                    i2c_state = I2C_START;
                    TWCR = i2c_twcr_val | (1 << TWSTA);
                }
                else
                {
                    i2c_state = I2C_STOP;
                    TWCR = i2c_twcr_val | (1 << TWSTO);
                }
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_STOP:
            i2c_state = I2C_DONE;
            TWCR = i2c_twcr_val & ~(1 << TWIE);
            break;
        default:
            i2c_state = I2C_ERROR;
            break;
    }

    if( i2c_state == I2C_ERROR )
    {
        TWCR = (i2c_twcr_val | (1 << TWSTO)) & ~(1 << TWIE);
    }
}

void i2c_master_rx_do_state(void)
{
    uint8_t twst;
    twst = TW_STATUS;

    switch( i2c_state )
    {
        case I2C_START:
            if( twst == TW_REP_START || twst == TW_START )
            {
                i2c_state = I2C_SEND_ADDR;
                TWDR = i2c_slaveaddr | TW_READ;
                TWCR = i2c_twcr_val;
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_SEND_ADDR:
            if( twst == TW_MR_SLA_ACK )
            {
                i2c_state = I2C_RECEIVE_DATA;
                i2c_rx_index = 0;
                i2c_rx_data[i2c_rx_index++] = TWDR;
                if( (i2c_rx_size--) == 1 )
                    TWCR = i2c_twcr_val;
                else
                    TWCR = i2c_twcr_val | (1 << TWEA);
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_SEND_DATA:
            if( twst == TW_MR_DATA_ACK )
            {
                if( i2c_rx_size )
                {
                    i2c_rx_data[i2c_rx_index++] = TWDR;
                    if( (i2c_rx_size--) == 1 )
                        TWCR = i2c_twcr_val;
                    else
                        TWCR = i2c_twcr_val | (1 << TWEA);
                }
                else
                {
                    i2c_state = I2C_STOP;
                    TWCR = i2c_twcr_val | (1 << TWSTO);
                }
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_STOP:
            i2c_state = I2C_DONE;
            TWCR = i2c_twcr_val & ~(1 << TWIE);
            break;
        default:
            i2c_state = I2C_ERROR;
            break;
    }

    if( i2c_state == I2C_ERROR )
    {
        TWCR = (i2c_twcr_val | (1 << TWSTO)) & ~(1 << TWIE);
    }
}

ISR(SIG_2WIRE_SERIAL)
{
    switch( i2c_mode )
    {
        case TW_MASTER_TX:
            i2c_master_tx_do_state();
            break;
        case TW_MASTER_RX:
            i2c_master_rx_do_state();
            break;
        default:
            i2c_state = I2C_ERROR;
            TWCR = 0;
            return;
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

