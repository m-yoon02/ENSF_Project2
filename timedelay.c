/*
 * timedelay.c - Timer functions for App Project 2
 * Clock: 8 MHz FRC -> Fcy = 4 MHz
 */
#include "timedelay.h"

#define PWM_PERIOD 4000   // (4 MHz / 4000 )= 1 kHz PWM, no flicker 

//shared variable 
volatile uint8_t  tick_flag     = 0;
volatile uint8_t  blink_on      = 1;
volatile uint8_t  debounce_flag = 0;
volatile uint8_t  short_press_flag = 0;
volatile uint8_t  long_press_flag  = 0;

//private variable
static volatile uint8_t  blinking   = 0;
static volatile uint8_t  blink_tick = 0;
static volatile uint16_t on_time    = 0;
static volatile uint16_t off_time   = 0;
static volatile uint8_t  pwm_high   = 0;
static volatile uint8_t  pwm_led    = 1;
static volatile uint8_t  press_running = 0;
static volatile uint16_t press_count   = 0;
static uint8_t           press_started_t1 = 0;  // check if timre 1 is on


void timer_init(void)
{
    TRISBbits.TRISB8 = 0;  LED1 = 0;
    TRISBbits.TRISB9 = 0;  LED2 = 0;

    // Timer1: 100 ms tick, 1:64 prescale -> 4 MHz/64 = 62500 Hz 
    T1CON = 0;
    T1CONbits.TCKPS = 0b10;
    PR1 = 6250 - 1;
    IPC0bits.T1IP = 4;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;

    // Timer2: PWM, 1:1 prescale 
    T2CON = 0;
    IPC1bits.T2IP = 6;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;

    // Timer3: 30 ms debounce, 1:8 prescale -> 500 kHz 
    T3CON = 0;
    T3CONbits.TCKPS = 0b01;
    PR3 = 15000 - 1;
    IPC2bits.T3IP = 5;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
}

// Timer1 tick 
void tick_start(void)
{
    TMR1 = 0;
    T1CONbits.TON = 1;
}

void tick_stop(void)
{
    T1CONbits.TON = 0;
    tick_flag = 0;
}

void blink_start(void)
{
    blink_tick = 0;
    blink_on = 1;
    blinking = 1;
}

void blink_stop(void)
{
    blinking = 0;
    blink_on = 1;
}

// Timer2 PWM 
void pwm_set(uint8_t led, uint8_t percent)
{
    uint16_t on = (uint16_t)(((uint32_t)percent * PWM_PERIOD) / 100);

    if (led != pwm_led) {          // switching LEDs: turn both off first 
        pwm_stop();
        pwm_led = led;
    }

    if (percent == 0) {            // fully off: no timer needed 
        T2CONbits.TON = 0;
        if (led == 1) LED1 = 0;
        else LED2 = 0;
    }
    else if (percent >= 100) {     //fully on: no timer needed 
        T2CONbits.TON = 0;
        if (led == 1) LED1 = 1;
        else LED2 = 1;
    }
    else {
        on_time  = on;
        off_time = PWM_PERIOD - on;
        if (!T2CONbits.TON) {      //start PWM if not already running 
            pwm_high = 1;
            if (led == 1) LED1 = 1;
            else LED2 = 1;
            
            TMR2 = 0;
            PR2 = on_time - 1;
            T2CONbits.TON = 1;
        }
    }
}

void pwm_stop(void)
{
    T2CONbits.TON = 0;
    IFS0bits.T2IF = 0;
    LED1 = 0;
    LED2 = 0;
}

// timer for short and long press
void press_timer_start(void)
{
    press_count = 0;
    press_running = 1;
    if (!T1CONbits.TON) {          // T1 off (OFF mode): run it just for the press 
        press_started_t1 = 1;
        tick_start();
    }
}

void press_timer_stop(void)
{
    if (!press_running) 
        return;    // release without a matching press: ignore 
    press_running = 0;

    if (press_count >= LONG_PRESS_TICKS) 
        long_press_flag = 1;
    else
        short_press_flag = 1;

    if (press_started_t1) {        // give T1 back to its previous (off) state 
        press_started_t1 = 0;
        tick_stop();
    }
}


//Call timer_task() every time main wakes up from Idle(). 
static volatile uint8_t t1_event = 0;
static volatile uint8_t t3_event = 0;

void timer_task(void)
{
    if (t1_event) {                          // 100 ms tick happened 
        t1_event = 0;
        tick_flag = 1;

        if (blinking && ++blink_tick >= 5) { // 5 x 100 ms = 500 ms 
            blink_tick = 0;
            blink_on ^= 1;
        }

        if (press_running && press_count < 0xFFFF) 
            press_count++;
    }

    if (t3_event) {                          // debounce time is over 
        t3_event = 0;
        T3CONbits.TON = 0;                   // one-shot: stop Timer3 
        debounce_flag = 1;
    }
}



// Timer1: every 100 ms 
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    t1_event = 1;
}

// Timer3: buttons stopped bouncing 
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
    t3_event = 1;
}

// Timer2: PWM edge. 
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    if (pwm_high) {
        pwm_high = 0;
        PR2 = off_time - 1;
        if (pwm_led == 1) 
            LED1 = 0; 
        else LED2 = 0;
        
    } else {
        pwm_high = 1;
        PR2 = on_time - 1;
        if (pwm_led == 1) 
            LED1 = 1; 
        else LED2 = 1;
    }
}
