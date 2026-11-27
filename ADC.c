#include <xc.h>
#include "ADC.h"

void ADC_init(void)
{

    TRISAbits.TRISA2 = 1;
    AD1PCFG &= ~(1u << POT_ADC_CHANNEL);

    AD1CON1bits.ADON = 0;
    AD1CON1bits.FORM = 0b00;
    AD1CON1bits.SSRC = 0b111;
    AD1CON1bits.ASAM = 0;

    AD1CON2bits.VCFG = 0b000;
    AD1CON2bits.CSCNA = 0;
    AD1CON2bits.SMPI = 0b0000;
    AD1CON2bits.BUFM = 0;
    AD1CON2bits.ALTS = 0;

    AD1CON3bits.ADRC = 0;
    AD1CON3bits.SAMC = 0b11111;
    AD1CON3bits.ADCS = 1;

    AD1CHSbits.CH0SA = POT_ADC_CHANNEL;
    AD1CHSbits.CH0NA = 0;
}

uint16_t do_ADC(void)
{
    uint16_t value;

    AD1CON1bits.ADON = 1;
    AD1CON1bits.DONE = 0;
    AD1CON1bits.SAMP = 1;

    while (!AD1CON1bits.DONE);

    value = ADC1BUF0;
    AD1CON1bits.SAMP = 0;
    AD1CON1bits.ADON = 0;
    return value;
}

uint8_t ADC_to_percent(uint16_t adc)
{
    if (adc > 1023) adc = 1023;

    return (uint8_t)(((uint32_t)adc * 100u + 511u) / 1023u);
}
