#ifndef _local_h_
#define _local_h_

#include <stdint.h>

/* Macros */
#define F_CPU 8000000
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

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} accel_t;


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

void ad7991_setup(void);
accel_t ad7991_read(uint8_t remotenum);

void ds1374_setup(void);
uint32_t ds1374_read(void);
void ds1374_write(uint32_t now);

void i2c_setup(void);
void i2c_write_8bit( uint8_t addr, uint8_t data );
void i2c_write_16bit( uint8_t addr, uint16_t data );
void i2c_write_buffer( uint8_t addr, uint8_t *buffer, uint8_t bytes );
uint8_t i2c_read_8bit( uint8_t addr );
uint16_t i2c_read_16bit( uint8_t addr );
uint8_t i2c_read_8bit_chained( uint8_t addr, uint8_t subaddr );
void i2c_read_buffer_chained( uint8_t addr, uint8_t subaddr, uint8_t *buffer,
                              uint8_t bytes );
void i2c_read_buffer( uint8_t addr, uint8_t *buffer, uint8_t bytes );

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
