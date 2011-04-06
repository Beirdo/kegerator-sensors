#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <string.h>
#include <util/twi.h>

#define TW_MASTER_TX    0
#define TW_MASTER_RX    1
#define TW_SLAVE_TX     2
#define TW_SLAVE_RX     3

typedef uint8_t i2c_mode_t;

#define I2C_START           0
#define I2C_SEND_ADDR       1
#define I2C_SEND_DATA       2
#define I2C_RECEIVE_DATA    3
#define I2C_STOP            4
#define I2C_ERROR           5

typedef uint8_t i2c_state_t;


volatile i2c_state_t i2c_state;
volatile i2c_mode_t i2c_mode;
uint8_t i2c_slaveaddr;
uint8_t i2c_tx_data[MAX_BUF_LEN];
uint8_t i2c_tx_index;
uint8_t i2c_tx_size;
uint8_t i2c_rx_data[MAX_BUF_LEN];
uint8_t i2c_rx_index;
uint8_t i2c_rx_size;
uint8_t i2c_chain;
uint8_t i2c_twcr_val;

void i2c_wait( void );

void i2c_setup(void)
{
    TWSR = 0;       /* No prescaling, please */
    TWBR = 2;       /* 400kHz I2C clock (32 for 100kHz) */
    i2c_twcr_val = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
    TWCR = i2c_twcr_val & ~(1 << TWIE);
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
    i2c_chain = 0;

    TWCR = i2c_twcr_val | (1 << TWSTA);

    i2c_wait();
}

uint8_t i2c_read_8bit_chained( uint8_t addr, uint8_t subaddr )
{
    uint8_t retval;

    i2c_read_buffer_chained(addr, subaddr, i2c_rx_data, 1);
    retval = i2c_rx_data[0];
    return( retval );
}

void i2c_read_buffer_chained( uint8_t addr, uint8_t subaddr, uint8_t *buffer,
                              uint8_t bytes )
{
    i2c_tx_data[0] = subaddr;
    i2c_tx_size = 1;
    i2c_rx_size = bytes;

    i2c_mode = TW_MASTER_TX;
    i2c_state = I2C_START;
    i2c_slaveaddr = addr;
    i2c_chain = 1;

    TWCR = i2c_twcr_val | (1 << TWSTA);

    i2c_wait();

    if( buffer != i2c_rx_data )
        memcpy( buffer, i2c_rx_data, bytes );
}

uint8_t i2c_read_8bit( uint8_t addr )
{
    uint8_t retval;

    i2c_read_buffer(addr, i2c_rx_data, 1);
    retval = i2c_rx_data[0];
    return( retval );
}

uint16_t i2c_read_16bit( uint8_t addr )
{
    uint16_t retval;

    i2c_read_buffer(addr, i2c_rx_data, 2);
    retval = (i2c_rx_data[0] << 8) | i2c_rx_data[1];
    return( retval );
}

void i2c_read_buffer( uint8_t addr, uint8_t *buffer, uint8_t bytes )
{
    i2c_mode = TW_MASTER_RX;
    i2c_state = I2C_START;
    i2c_chain = 0;
    i2c_rx_size = bytes;
    i2c_slaveaddr = addr;

    TWCR = i2c_twcr_val | (1 << TWSTA);

    i2c_wait();

    if( buffer != i2c_rx_data )
        memcpy( buffer, i2c_rx_data, bytes );
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
                    TWCR = (i2c_twcr_val | (1 << TWSTO)) & ~(1 << TWIE);
                }
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
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
                if( (i2c_rx_size) == 1 )
                    TWCR = i2c_twcr_val;
                else
                    TWCR = i2c_twcr_val | (1 << TWEA);
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
            break;
        case I2C_RECEIVE_DATA:
            if( twst == TW_MR_DATA_ACK || twst == TW_MR_DATA_NACK )
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
                    TWCR = (i2c_twcr_val | (1 << TWSTO)) & ~(1 << TWIE);
                }
            }
            else
            {
                i2c_state = I2C_ERROR;
            }
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
            TWCR = (i2c_twcr_val | (1 << TWSTO)) & ~(1 << TWIE);
            return;
    }
}

void i2c_wait( void )
{
    while( i2c_state < I2C_STOP )
    {
        sleep_mode();
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

