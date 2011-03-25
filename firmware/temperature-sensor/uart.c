#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

/* 38400 to the sensors over BLVDS */
#define BAUD 38400
#include <util/setbaud.h>

uint8_t u_rx_buf[MAX_BUF_LEN];
uint8_t u_rx_index;
uint8_t u_rx_size;
uint8_t u_tx_buf[MAX_BUF_LEN];
uint8_t u_tx_index;
uint8_t u_tx_size;
uint8_t ucsra_val;

void uart_setup(void)
{
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
#if USE_2X
    ucsra_val = (1 << U2X) | (1 << MPCM);
#else
    ucsra_val = (1 << MPCM);
#endif
    UCSRA = ucsra_val;

    UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN) | (1 << UCSZ2);
    UCSRC = (3 << UCSZ0);                  /* N91 */

    /* Setup the enables for the BLVDS part */
    DDRD  |= (1 << PD3) | (1 << PD2);

    /* Enable the reciever, disable the transmitter */
    PORTD &= ~((1 << PD3) | (1 << PD2));

    u_rx_index = 0;
    u_tx_index = 0;
}

/* UART Rx interrupt */
ISR(SIG_UART_RECV)
{
    uint8_t byte;

    if( (UCSRA & (1 << MPCM)) ) {
        /* Wait for the address */
        if( (UDR == sensor_address) ) {
            /* Allow it to read the data now */
            UCSRA = (ucsra_val & ~(1 << MPCM));
        }
    }
    else
    {
        /* We are in the data portion */
        if( u_rx_index < MAX_BUF_LEN )
        {
            byte = UDR;
            u_rx_buf[u_rx_index++] = byte;
        }
    }

    if( u_rx_index == 2 )
    {
        u_rx_size = MIN(byte, MAX_BUF_LEN);
    }

    if( u_rx_index >= u_rx_size )
    {
        if( check_crc( u_rx_buf, u_rx_size ) != 0 )
        {
            /* Bad CRC.  */
        }
        else
        {
            /* Good CRC. */
            /* This is to this board */
            if( u_rx_size >= 6 ) {
                /* Disable RX interrupts while this runs */
                UCSRB &= ~(1 << RXCIE);

                /* Hand the buffer over to the sensor handling code */
                sensor_handle_fast();
            }
        }
    }
}

/* UART Tx Data Empty interrupt */
ISR(SIG_UART_DATA)
{
    if( u_tx_index == u_tx_size )
    {
        UCSRB &= ~(1 << UDRIE);
    }
    else
    {
        UCSRB &= ~(1 << TXB8);  /* In data mode, no address bit */
        UDR = u_tx_buf[u_tx_index++];
    }
}

/* UART Tx Complete interrupt */
ISR(SIG_UART_TRANS)
{
    UCSRB &= ~(1 << TXCIE);

    /* Enable the reciever, disable the transmitter */
    PORTD &= ~((1 << PD3) | (1 << PD2));

    u_tx_size = 0;
}

void uart_transmit(uint8_t target)
{
    u_tx_size = MIN(u_tx_size, MAX_BUF_LEN);
    if( u_tx_size )
    {
        /* Enable the transmitter, disable the receiver */
        PORTD |= (1 << PD3) | (1 << PD2);

        u_tx_index = 0;
        UCSRB |= (1 << TXB8);  /* Set the address bit */
        UDR = target;
        UCSRB |= (1 << TXCIE) | (1 << UDRIE);
    }
}

void uart_restart_rx(void)
{
    uint8_t dummy;

    /* Drain the RX register */
    while( UCSRA && (1 << RXC) )
        dummy = UDR;

    /* Reenable RX interrupts */
    UCSRB |= (1 << RXCIE);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

