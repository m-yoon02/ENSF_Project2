#include <xc.h>
#include <stdint.h>
#include "timedelay.h"
#include "IOs.h"
#include "ADC.h"
#include "uart.h"

#pragma config FNOSC = FRC
#pragma config FCKSM = CSECMD
#pragma config POSCMOD = NONE
#pragma config OSCIOFNC = ON
#pragma config FWDTEN = OFF
#pragma config MCLRE = ON

typedef enum {
    OFF_IDLE,
    OFF_BLINK,
    ON_SOLID,
    ON_BLINK
} state_t;

static state_t state = OFF_IDLE;
static uint8_t active_led = 1;
static uint8_t uart_on = 0;

static void enter_off_idle(void)
{
    state = OFF_IDLE;
    uart_on = 0;
    blink_stop();
    pwm_stop();
    tick_stop();
}

static void enter_off_blink(void)
{
    state = OFF_BLINK;
    blink_start();
    pwm_set(active_led, 100);
    tick_start();
}

static void enter_on_solid(void)
{
    state = ON_SOLID;
    blink_stop();
    tick_start();
    pwm_set(active_led, ADC_to_percent(do_ADC()));
}

static void enter_on_blink(void)
{
    state = ON_BLINK;
    blink_start();
    pwm_set(active_led, ADC_to_percent(do_ADC()));
}

static void handle_events(void)
{

    if (short_press_flag) {
        short_press_flag = 0;
        if (state == OFF_IDLE || state == OFF_BLINK) {
            active_led = 1;
            enter_on_solid();
        } else {
            enter_off_idle();
        }
    }

    if (long_press_flag) {
        long_press_flag = 0;
        if (state == ON_SOLID || state == ON_BLINK) {
            active_led = (active_led == 1) ? 2 : 1;

            pwm_set(active_led, ADC_to_percent(do_ADC()));
        }
    }

    if (pb2_click_flag) {
        pb2_click_flag = 0;
        switch (state) {
            case OFF_IDLE:  enter_off_blink(); break;
            case OFF_BLINK: enter_off_idle();  break;
            case ON_SOLID:  enter_on_blink();  break;
            case ON_BLINK:  enter_on_solid();  break;
        }
    }

    if (pb3_click_flag) {
        pb3_click_flag = 0;
        if (state == ON_SOLID || state == ON_BLINK)
            uart_on ^= 1;
    }

    if (state != OFF_IDLE && !T1CONbits.TON)
        tick_start();
}

static void on_tick(void)
{
    uint16_t adc;
    uint8_t  percent, out;

    switch (state) {
        case ON_SOLID:
        case ON_BLINK:
            adc = do_ADC();
            percent = ADC_to_percent(adc);

            out = (state == ON_BLINK && !blink_on) ? 0 : percent;
            pwm_set(active_led, out);
            if (uart_on)
                uart_send_reading(out, adc);
            break;

        case OFF_BLINK:
            pwm_set(active_led, blink_on ? 100 : 0);
            break;

        case OFF_IDLE:
        default:
            break;
    }
}

int main(void)
{
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0xFFFF;

    timer_init();
    IOinit();
    ADC_init();
    uart_init();

    enter_off_idle();

    while (1) {

        SRbits.IPL = 7;
        timer_task();
        if (!debounce_flag && !tick_flag && !short_press_flag &&
            !long_press_flag && !pb2_click_flag && !pb3_click_flag) {
            Idle();
        }
        SRbits.IPL = 0;

        timer_task();
        IOcheck();
        handle_events();

        if (tick_flag) {
            tick_flag = 0;
            on_tick();
        }
    }
    return 0;
}
