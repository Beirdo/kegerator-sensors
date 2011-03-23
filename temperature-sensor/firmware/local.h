#ifndef _local_h_
#define _local_h_

#include <stdint.h>

/* Macros */
#define F_CPU 16000000
#define MAX_BUF_LEN 32

#define MIN(x,y)    ((x) < (y) ? (x) : (y))
#define MAX(x,y)    ((x) > (y) ? (x) : (y))

/* Types */
typedef struct {
    uint8_t  address;
    uint8_t  length;    /* From address to end of CRC16 */
    uint8_t  get_set;   /* 1 = set, 0 = get */
    uint8_t  subaddress;
    uint8_t  data[1];   /* actually of size length - 5 */
    uint16_t crc16;
} message;


/* Externals */
extern uint8_t sensor_address;
extern uint8_t sensor_in_use;
extern uint8_t sensor_fw_version[MAX_BUF_LEN];

extern uint8_t u_rx_buf[MAX_BUF_LEN];
extern uint8_t u_rx_size;

extern uint8_t u_tx_buf[MAX_BUF_LEN];
extern uint8_t u_tx_size;

/* Prototypes */
void uart_setup(void);
void uart_transmit(uint8_t target);
void uart_restart_rx(void);

uint16_t check_crc( uint8_t *buffer, uint8_t length );
uint16_t calc_crc( uint8_t *buffer, uint8_t length );

void sensor_setup(void);
void sensor_handle(void);
void sensor_handle_fast(void);

void tcn75a_setup(void);
uint16_t tcn75a_read( uint8_t regnum );

void i2c_setup(void);
void i2c_write_8bit( uint8_t addr, uint8_t data );
void i2c_write_16bit( uint8_t addr, uint16_t data );
void i2c_write_buffer( uint8_t addr, uint8_t *buffer, uint8_t bytes );
uint16_t i2c_read_16bit( uint8_t addr );
void i2c_read_buffer( uint8_t addr, uint8_t bytes );

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

