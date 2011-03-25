#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

/* 38400 to the sensors over BLVDS */
#define BAUD 38400
#include <util/setbaud.h>

uint8_t u1_rx_buf[MAX_BUF_LEN];
uint8_t u1_rx_index;
uint8_t u1_rx_size;
uint8_t u1_tx_buf[MAX_BUF_LEN];
uint8_t u1_tx_index;
uint8_t u1_tx_size;
uint8_t ucsr1a_val;

void uart1_setup(void)
{
    UBRR1H = UBRRH_VALUE;
    UBRR1L = UBRRL_VALUE;
#if USE_2X
    ucsr1a_val = (1 << U2X) | (1 << MPCM);
#else
    ucsr1a_val = (1 << MPCM);
#endif
    UCSR1A = ucsr1a_val;

    UCSR1B = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN) | (1 << UCSZ2);
    UCSR1C = (3 << UCSZ0);                  /* N91 */

    /* Setup the enables for the BLVDS part as outputs */
    DDRD |= (1 << PD5) | (1 << PD4);

    /* Enable the receiver, disable transmitter */
    PORTD &= ~((1 << PD5) | (1 << PD4));

    u1_rx_index = 0;
    u1_tx_index = 0;
}

/* UART1 Rx interrupt */
ISR(SIG_UART1_RECV)
{
    uint8_t byte;

    if( (UCSR1A & (1 << MPCM)) ) {
        /* Wait for the address */
        if( (UDR1 == sensor_address) ) {
            /* Allow it to read the data now */
            UCSR1A = (ucsr1a_val & ~(1 << MPCM));
        }
    }
    else
    {
        /* We are in the data portion */
        if( u1_rx_index < MAX_BUF_LEN )
        {
            byte = UDR1;
            u1_rx_buf[u1_rx_index++] = byte;
        }
    }

    if( u1_rx_index == 2 )
    {
        u1_rx_size = MIN(byte, MAX_BUF_LEN);
    }

    if( u1_rx_index >= u1_rx_size )
    {
        if( check_crc( u1_rx_buf, u1_rx_size ) != 0 )
        {
            /* Bad CRC.  */
        }
        else
        {
            /* Good CRC. */
            /* Transmit it to the PC over USB */
            u_tx_size = u1_rx_size;
            memcpy(u_tx_buf, u1_rx_buf, u_tx_size);
            uart_transmit(0xFF);
        }
    }
}

/* UART1 Tx Data Empty interrupt */
ISR(SIG_UART1_DATA)
{
    if( u1_tx_index == u1_tx_size )
    {
        UCSR1B &= ~(1 << UDRIE);
    }
    else
    {
        UCSR1B &= ~(1 << TXB8);  /* In data mode, no address bit */
        UDR1 = u1_tx_buf[u1_tx_index++];
    }
}

/* UART1 Tx Complete interrupt */
ISR(SIG_UART1_TRANS)
{
    UCSR1B &= ~(1 << TXCIE);

    /* Enable the receiver, disable transmitter */
    PORTD &= ~((1 << PD5) | (1 << PD4));

    u1_tx_size = 0;
}

void uart1_transmit(uint8_t target)
{
    u1_tx_size = MIN(u1_tx_size, MAX_BUF_LEN);
    if( u1_tx_size )
    {
        /* Enable the transmitter, disable receiver */
        PORTD |= (1 << PD5) | (1 << PD4);

        u1_tx_index = 0;
        UCSR1B |= (1 << TXB8);  /* Set the address bit */
        UDR1 = target;
        UCSR1B |= (1 << TXCIE) | (1 << UDRIE);
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

