#ifndef _local_h_
#define _local_h_

#include "common.h"
#include "sensor-common.h"

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
} accel_t;

/* Externals */

/* Prototypes */
void ad7991_setup(void);
accel_t ad7991_read(uint8_t remotenum);

void ds1374_setup(void);
uint32_t ds1374_read(void);
void ds1374_write(uint32_t now);

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

