#ifndef _local_h
#define _local_h

/* Includes */
#include <stdint.h>

/* Typedefs */
typedef enum {
    S_UNKNOWN = 0,
    S_RECEIVER,
    S_ACCELEROMETER,
    S_MASS,
    S_TEMPERATURE
} sensor_t;

/* Defines */

/* Externs */

/* Prototypes */
int serial_setup(void);
int serial_write(uint8_t *buf, int len);
int serial_read(uint8_t *buf, int maxlen);
void serial_close(void);

uint16_t crc16_update(uint16_t crc, uint8_t a);
uint16_t calc_crc(uint8_t *buffer, uint8_t length);
uint16_t check_crc(uint8_t *buffer, uint8_t length);


int sensor_setup(void);
int sensor_send( uint8_t *buf, uint8_t maxlen, uint8_t address, uint8_t get_set,
                 uint8_t subaddress, uint8_t *data, uint8_t len );
sensor_t sensor_type_find(uint8_t *string, int len);
uint8_t *sensor_version_find(uint8_t *string, int len);
void sensor_poll(void);
void sensor_handle( sensor_t type, uint8_t *buf );
void buffer_dump(char *pre, uint8_t *buf, int len);
int sensor_probe( uint8_t *buf, int maxlen, int address, int subaddr, 
                  sensor_t type );

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

