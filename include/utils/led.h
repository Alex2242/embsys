#ifndef LED_H
#define LED_H

#include <stdint.h>

void useGpio(uint8_t pn);
void flipLed();
void disableGpio();

#endif