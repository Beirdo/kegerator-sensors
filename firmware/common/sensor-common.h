#ifndef _sensor_common_h_
#define _sensor_common_h_

/* Macros */

/* Types */

/* Externals */
extern uint8_t u_rx_buf[MAX_BUF_LEN];
extern uint8_t u_rx_size;

extern uint8_t u_tx_buf[MAX_BUF_LEN];
extern uint8_t u_tx_size;

/* Prototypes */
void uart_setup(void);
void uart_transmit(uint8_t target);
void uart_restart_rx(void);

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

