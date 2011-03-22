#ifndef _local_h_
#define _local_h_

#define MAX_BUF_LEN 32

#define MIN(x,y)    ((x) < (y) ? (x) : (y))
#define MAX(x,y)    ((x) > (y) ? (x) : (y))

/* Externals */
extern uint8_t sensor_address;

extern uint8_t u1_tx_buf[MAX_BUF_LEN];
extern uint8_t u1_tx_size;

/* Prototypes */
void uart0_setup(void);
void uart0_transmit(void);

void uart1_setup(void);
void uart1_transmit(uint8_t target);

uint16_t check_crc( uint8_t *buffer, uint8_t length );
uint16_t calc_crc( uint8_t *buffer, uint8_t length );


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

