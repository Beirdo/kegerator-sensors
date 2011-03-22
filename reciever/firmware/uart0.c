#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "local.h"

/* 38400 to the PC over USB */
#define F_CPU 16000000
#define BAUD 38400
#include <util/setbaud.h>

typedef struct {
    uint8_t  address;
    uint8_t  length;    /* From address to end of CRC16 */
    uint8_t  get_set;
    uint8_t  subaddress;
    uint8_t  data[1];   /* actually of size length - 5 */
    uint16_t crc16;
} u0_message;

uint8_t u0_rx_buf[MAX_BUF_LEN];
uint8_t u0_rx_index;
uint8_t u0_rx_size;
uint8_t u0_rx_data_size;

uint8_t u0_tx_buf[MAX_BUF_LEN];
uint8_t u0_tx_index;
uint8_t u0_tx_size;

void uart0_setup(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
#if USE_2X
    UCSR0A |= (1 << U2X);
#else
    UCSR0A &= ~(1 << U2X);
#endif

    UCSR0B = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN);
    UCSR0C = (3 << UCSZ0);                  /* N81 */

    u0_rx_index = 0;
    u0_rx_size  = MAX_BUF_LEN;
    u0_tx_index = 0;
}

/* UART0 Rx interrupt */
ISR(SIG_UART0_RECV)
{
    uint8_t byte;

    if( u0_rx_index >= u0_rx_size )
        return;

    byte = UDR0;
    u0_rx_buf[u0_rx_index++] = byte;

    if( u0_rx_index == 2 )
    {
        u0_rx_size = MIN(byte, MAX_BUF_LEN);
        u0_rx_data_size = u0_rx_size < 5 ? 0 : u0_rx_size - 5;
    }

    if( u0_rx_index >= u0_rx_size )
    {
        if( check_crc( u0_rx_buf, u0_rx_size ) != 0 )
        {
            /* Bad CRC.  */
        }
        else
        {
            /* Good CRC. */
            if( u0_rx_buf[0] == sensor_address )
            {
                /* This is to this board */
            }
            else
            {
                /* Transmit it to the BLVDS */
                u1_tx_size = u0_rx_size - 1;
                memcpy(u1_tx_buf, &(u0_rx_buf), u1_tx_size);
                uart1_transmit(u0_rx_buf[0]);
            }
        }
    }
}

/* UART0 Tx Data Empty interrupt */
ISR(SIG_UART0_DATA)
{
    if( u0_tx_index == u0_tx_size )
    {
        UCSR0B &= ~(1 << UDRIE);
    }
    else
    {
        UDR0 = u0_tx_buf[u0_tx_index++];
    }
}

/* UART0 Tx Complete interrupt */
ISR(SIG_UART0_TRANS)
{
    UCSR0B &= ~(1 << TXCIE);
    u0_tx_size = 0;
}

void uart0_transmit(void)
{
    u0_tx_size = MIN(u0_tx_size, MAX_BUF_LEN);
    if( u0_tx_size )
    {
        u0_tx_index = 1;
        UDR0 = u0_tx_buf[0];
        UCSR0B |= (1 << TXCIE) | (1 << UDRIE);
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

