/*
 * timedelay.h - Timer functions and shared variables for App Project 2
 *
 * Timer1 = 100 ms tick (ADC read, UART send, blink, long press)
 * Timer2 = software PWM for the LED
 * Timer3 = button debounce
 */
#ifndef TIMEDELAY_H
#define TIMEDELAY_H

#include <xc.h>
#include <stdint.h>


#define LED1 LATBbits.LATB9   
#define LED2 LATBbits.LATA6

// for CN ISR in IO.c
#define DEBOUNCE_START() 
  do { 
    TMR3 = 0; 
    IFS0bits.T3IF = 0; 
    T3CONbits.TON = 1; 
    } while (0)

// Variables shared with main.c, IO.c, ADC.c 
extern volatile uint8_t  tick_flag;      // set every 100 ms -> read ADC, send UART 
extern volatile uint8_t  blink_on;       // flips every 500 ms while blinking       
extern volatile uint8_t  debounce_flag;  // set 30 ms after last button edge        
//extern volatile uint16_t hold_count;     // counts 100 ms ticks while PB1 held      
//extern volatile uint8_t  pb1_held;       // IO.c sets 1 on press, 0 on release      
#define LONG_PRESS_TICKS 30                // flag for if button is pressed more than 3 seconds
extern volatile uint8_t short_press_flag;  // if button is pressed for less than or equal to 3s
extern volatile uint8_t long_press_flag;  // button pressed more than 3 seconds

// Functions 
void timer_init(void);
void tick_start(void);
void tick_stop(void);
void blink_start(void);
void blink_stop(void);
void pwm_set(uint8_t led, uint8_t percent);  // led: 1 = LED1, 2 = LED2, percent 0-100 
void pwm_stop(void);
void timer_task(void);
void timer_press_timer_start(void); // called when pb1 is pressed and after debouncing
void press_timer_stop(void);         // called when pb1 is released. to keep track on how long the button has been pressed. 

#endif
