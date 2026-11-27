#ifndef IOS_H
#define IOS_H

#include <stdint.h>

extern volatile uint8_t pb2_click_flag;
extern volatile uint8_t pb3_click_flag;

void IOinit(void);
void IOcheck(void);

#endif
