#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>

/* 38400 to the PC over USB */
#define BAUD 38400
#include <util/setbaud.h>

#ifndef URSEL
#define URSEL 7
#endif

uint8_t u_rx_buf[MAX_BUF_LEN];
uint8_t u_rx_index;
uint8_t u_rx_size;

uint8_t u_tx_buf[MAX_BUF_LEN];
uint8_t u_tx_index;
uint8_t u_tx_size;

void uart_setup(void)
{
    uart0_setup();
    uart1_setup();
}

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
    UCSR0C = (1 << URSEL) | (3 << UCSZ0);                  /* N81 */

    u_rx_index = 0;
    u_rx_size  = MAX_BUF_LEN;
    u_tx_index = 0;
}

void uart0_rx_timeout(void)
{
    timer3_disable();
    u_rx_index = 0;
}


/* UART0 Rx interrupt */
ISR(SIG_UART0_RECV)
{
    uint8_t byte;

#if 0
    timer3_enable();
#endif

    byte = UDR0;
    if( u_rx_index >= u_rx_size )
        return;

    u_rx_buf[u_rx_index++] = byte;

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
            if( u_rx_buf[0] == sensor_address )
            {
                /* This is to this board */
                if( u_rx_size >= 6 ) {
                    /* Disable RX interrupts while this runs */
                    UCSR0B &= ~(1 << RXCIE);

                    /* Hand the buffer over to the sensor handling code */
                    sensor_handle_fast();
                }
            }
            else
            {
                /* Transmit it to the BLVDS */
                u1_tx_size = u_rx_size;
                memcpy(u1_tx_buf, u_rx_buf, u1_tx_size);
                uart1_transmit();
            }
            u_rx_index = 0;
        }
    }
}

/* UART0 Tx Data Empty interrupt */
ISR(SIG_UART0_DATA)
{
    if( u_tx_index == u_tx_size )
    {
        UCSR0B &= ~(1 << UDRIE);
    }
    else
    {
        UDR0 = u_tx_buf[u_tx_index++];
    }
}

/* UART0 Tx Complete interrupt */
ISR(SIG_UART0_TRANS)
{
    UCSR0B &= ~(1 << TXCIE);
    u_tx_size = 0;
}

void uart_transmit(void)
{
    u_tx_size = MIN(u_tx_size, MAX_BUF_LEN);
    if( u_tx_size )
    {
        u_tx_index = 1;
        UDR0 = u_tx_buf[0];
        UCSR0B |= (1 << TXCIE) | (1 << UDRIE);
    }
}


void uart_restart_rx(void)
{
    uint8_t dummy;

    /* Drain the RX register */
    while( UCSR0A & (1 << RXC) )
        dummy = UDR0;

    u_rx_index = 0;

    /* Reenable RX interrupts */
    UCSR0B |= (1 << RXCIE);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

