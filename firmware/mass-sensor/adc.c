#include "local.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <stdint.h>

uint16_t EEMEM ee_adc_zero_offset;
uint16_t EEMEM ee_adc_inverse_slope;
uint16_t EEMEM ee_adc_tare_grams;

uint16_t adc_zero_offset;
uint16_t adc_inverse_slope;
uint16_t adc_tare_grams;

uint8_t adcsra_val;

void adc_setup(void)
{
    adc_zero_offset   = eeprom_read_word( &ee_adc_zero_offset );
    adc_inverse_slope = eeprom_read_word( &ee_adc_inverse_slope );
    adc_tare_grams    = eeprom_read_word( &ee_adc_tare_grams );

    /* Setup ADC reference (Aref/Avcc), channel mux */
    ADMUX = 0;  /* Aref, using ADC0 */
    adcsra_val = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0); /* 250kHz clock */
    ADCSRA = adcsra_val;
}

uint16_t adc_read_raw(void)
{
    uint16_t raw;

    /* Start conversion, clear old interrupt, enable interrupt */
    ADCSRA = adcsra_val | (1 << ADSC) | (1 << ADIF) | (1 << ADIE);

    while( ADCSRA & (1 << ADSC) )
    {
        sleep_mode();
    }

    raw = ADCL | (ADCH << 8);

    return( raw );
}

uint16_t adc_read(void)
{
    uint16_t raw;
    uint32_t value;
    uint16_t grams;

    raw = adc_read_raw();

    /* Note: adc_inverse_slope is 8.8 bits (fractional) */
    value = (raw - adc_zero_offset) * adc_inverse_slope;
    grams = (uint16_t)(value >> 8);

    if( value > adc_tare_grams )
        return( value - adc_tare_grams );

    return( 0 );
}

ISR(SIG_ADC)
{
    /* Shut the interrupt back off */
    ADCSRA = adcsra_val;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

