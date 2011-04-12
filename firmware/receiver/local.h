#ifndef _local_h_
#define _local_h_

#include "common.h"

/* Macros */

/* Types */

/* Externals */
extern uint8_t u1_tx_buf[MAX_BUF_LEN];
extern uint8_t u1_tx_size;

/* Prototypes */
void uart0_setup(void);
void uart0_rx_timeout(void);
void uart1_setup(void);
void uart1_transmit(void);

void tcn75a_setup(void);
uint16_t tcn75a_read( uint8_t regnum );

void timer3_setup(void);
void timer3_enable(void);
void timer3_disable(void);

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

