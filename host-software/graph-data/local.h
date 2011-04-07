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
extern char *graphuri;

/* Prototypes */
void load_data(int sensor, int addr, int type, int days);
void create_graph(void);

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

