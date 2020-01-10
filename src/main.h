#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

//uC pins
#define TMS 8  //output
#define TCK 9  //output
#define TDI 10 //input (pic32 TD0)
#define TDO 11 //output (pic32 TDI)

#define ON 1
#define OFF 0

//Sets the TDO and TMS pin values and Performs a pulse (Rise and Falling Edge)
void advanceCLK(int tdo, int tms);
/*
useconst0: 1 => TDO from DR_array, 0 => TDO = 0
Shifts a certain ammount of values "into" tdo, through advanceCLK
*/
void shift(unsigned int ammount, char useconst0, uint8_t* array);

//Go from Idle to shift-IR, shift in the instruction with shift and go back to Idle
void shiftIR();

/*
Go from Idle to shift-DR, shift in the given ammount from DR with shift and go back to Idle
useconst0: Value to pass to the shift function
*/
void shiftDR(unsigned int ammount, char useconst0);

//Performs 5 cycles with TMS = 1 to go to the Reset state, and then advance to the Idle state
void reset();

/*
Uses shiftIR to shift in the IDCODE instruction, and uses shiftDR to shift out the IDCODE,
and then prints it
*/
void printIDCODE();
/*
Uses shiftIR to shift in the Sample instruction, and uses shiftDR to shift out the entire
boundary chain into the DR_Array
*/
void BSRCapture();
/*
Uses BSRCapture to obtain the current values fro mthe boundary chain, checks the button's
state and prints it.
*/
void printButtonStatus();
/*
Changes the LEDs' state to status (if it isn't already)
*/
void turnLED(uint8_t status);
/*
Assumes the LED was forced OFF, and checks if it should be ON
Turns it ON if so
*/
void checkLED();


#include "main.cpp"

#endif