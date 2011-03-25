#ifndef _common_h_
#define _common_h_

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

/* Externals */
extern uint8_t sensor_address;
extern uint8_t sensor_in_use;
extern uint8_t sensor_fw_version[MAX_BUF_LEN];

/* Prototypes */
void uart_setup(void);

uint16_t check_crc( uint8_t *buffer, uint8_t length );
uint16_t calc_crc( uint8_t *buffer, uint8_t length );

void sensor_main_setup(void);
void sensor_local_setup(void);
void sensor_handle(void);
void sensor_handle_fast(void);

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

