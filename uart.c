#include <xc.h>
#include "uart.h"

void uart_init(void)
{

    AD1PCFGbits.PCFG2 = 1;
    AD1PCFGbits.PCFG3 = 1;

    U2MODE = 0;
    U2MODEbits.BRGH = 1;
    U2BRG = 103;
    U2STA = 0;
    U2MODEbits.UARTEN = 1;
    U2STAbits.UTXEN = 1;
}

static void uart_putc(char c)
{
    while (U2STAbits.UTXBF);
    U2TXREG = c;
}

static void uart_put_uint(uint16_t n)
{
    char buf[5];
    int8_t i = 0;
    do {
        buf[i++] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    while (i > 0)
        uart_putc(buf[--i]);
}

void uart_send_reading(uint8_t percent, uint16_t adc)
{
    uart_put_uint(percent);
    uart_putc(',');
    uart_put_uint(adc);
    uart_putc('\r');
    uart_putc('\n');
}
