#ifndef _local_h_
#define _local_h_

#include "common.h"

/* Macros */

/* Types */

/* Externals */
extern uint8_t u0_rx_buf[MAX_BUF_LEN];
extern uint8_t u0_rx_size;
extern uint8_t u0_rx_data_size;

extern uint8_t u0_tx_buf[MAX_BUF_LEN];
extern uint8_t u0_tx_size;

extern uint8_t u1_tx_buf[MAX_BUF_LEN];
extern uint8_t u1_tx_size;

/* Prototypes */
void uart0_setup(void);
void uart0_transmit(void);
void uart0_restart_rx(void);

void uart1_setup(void);
void uart1_transmit(uint8_t target);

void tcn75a_setup(void);
uint16_t tcn75a_read( uint8_t regnum );

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

