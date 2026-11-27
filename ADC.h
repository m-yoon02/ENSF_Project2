#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define POT_ADC_CHANNEL 4

void     ADC_init(void);
uint16_t do_ADC(void);
uint8_t  ADC_to_percent(uint16_t adc);

#endif
