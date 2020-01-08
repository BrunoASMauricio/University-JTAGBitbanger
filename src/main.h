#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

//uC pins
#define TMS 8  //output
#define TCK 9  //output
#define TDI 10 //input (pic32 TD0)
#define TDO 11 //output (pic32 TDI)
#define LED 13 //built-in led

#define BYPASS 0x1F         //Used for bypassing a device in a test chain; this allows the testing of off-chip circuitry and board level interconnections.
#define HIGHZ 0x00          //Places device in a high-impedance state, all pins are forced to inputs
#define SAMPLE_PRELOAD 0x02 //Samples all pins or loads a specific value into output latch. Captures the I/O states of the component, providing a snapshot of its operation.
#define EXTEST 0x06         //Boundary scan. Allows the external circuitry and interconnections to be tested, by either forcing various test patterns on the output pins, or capturing test results from the input pins.
#define IDCODE 0x01         //Shifts out the device’s ID code

#define ON 1
#define OFF 0

void advanceCLK(int tdo, int tms);
void shift(unsigned int ammount, char useconst0);
void shiftIR(unsigned int ammount, char useconst0);
void shiftDR(unsigned int ammount, char useconst0);
void reset();
void printIDCODE();
void printButtonStatus();
void turnLED(uint8_t status);

#include "main.cpp"

#endif