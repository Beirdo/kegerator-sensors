#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "local.h"

/* 38400 to the PC over USB */
#define F_CPU 16000000
#define BAUD 38400
#include <util/setbaud.h>

uint8_t u0_rx_buf[MAX_BUF_LEN];
uint8_t u0_rx_index;
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
    u0_tx_index = 0;
}

/* UART0 Rx interrupt */
ISR(SIG_UART0_RECV)
{
    if( u0_rx_index < MAX_BUF_LEN )
    {
        u0_rx_buf[u0_rx_index++] = UDR0;
    }

    /* TODO: end of frame handling */
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

