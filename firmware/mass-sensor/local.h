#ifndef _local_h_
#define _local_h_

#include "common.h"
#include "sensor-common.h"
#include <avr/eeprom.h>

/* Macros */

/* Types */

/* Externals */
extern uint16_t EEMEM ee_adc_zero_offset;
extern uint16_t EEMEM ee_adc_inverse_slope;
extern uint16_t EEMEM ee_adc_tare_grams;

extern uint16_t adc_zero_offset;
extern uint16_t adc_inverse_slope;
extern uint16_t adc_tare_grams;

/* Prototypes */
uint16_t adc_read(void);

void ad5252_setup(void);
void ad5252_set_rdac( uint8_t rdacnum, uint8_t value );
uint8_t ad5252_get_rdac( uint8_t rdacnum );
void ad5252_set_eemem( uint8_t address, uint8_t value );
uint8_t ad5252_get_eemem( uint8_t address );


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

