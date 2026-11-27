#include <xc.h>
#include "IOs.h"
#include "timedelay.h"

#define PB1_BIT 0x01
#define PB2_BIT 0x02
#define PB3_BIT 0x04

volatile uint8_t pb2_click_flag = 0;
volatile uint8_t pb3_click_flag = 0;

static uint8_t last_state = 0;

void IOinit(void)
{

    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN23PUE = 1;
    CNEN2bits.CN23IE = 1;

    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;

    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;

    IPC4bits.CNIP = 5;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
}

void IOcheck(void)
{
    uint8_t now, pressed, released;

    if (!debounce_flag)
        return;
    debounce_flag = 0;

    now = 0;
    if (PORTBbits.RB7 == 0) now |= PB1_BIT;
    if (PORTBbits.RB4 == 0) now |= PB2_BIT;
    if (PORTAbits.RA4 == 0) now |= PB3_BIT;

    pressed = now & ~last_state;
    released = ~now &  last_state;
    last_state = now;

    if (pressed  & PB1_BIT) press_timer_start();
    if (released & PB1_BIT) press_timer_stop();
    if (released & PB2_BIT) pb2_click_flag = 1;
    if (released & PB3_BIT) pb3_click_flag = 1;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    IFS1bits.CNIF = 0;
    DEBOUNCE_START();
}
