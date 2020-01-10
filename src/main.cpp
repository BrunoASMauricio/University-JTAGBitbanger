#ifndef MAIN_CPP
#define MAIN_CPP

#include "main.h"

uint8_t DR_array[148];      //Store boundary scan and idcode
uint8_t IR_array[5];        //Store Instruction

uint16_t iter;              //Multipurpose iterator
uint16_t iter2;             //Multipurpose iterator
uint8_t ledstatus;          //Store the current led status

void advanceCLK(int tdo, int tms){
  digitalWrite(TDO, tdo);
  digitalWrite(TMS,tms);
  digitalWrite(TCK, LOW);
  delayMicroseconds(2);
  digitalWrite(TCK, HIGH);
  delayMicroseconds(2);
}

void shift(unsigned int ammount, char useconst0, uint8_t* array){
  if(ammount){
    for(iter = 0; iter < ammount-1; iter++){
      advanceCLK(useconst0 ? 0 : array[iter]=='1',0);
      array[iter] = ((char)digitalRead(TDI))+'0';
    }
  }
  //Exit "while" writting/reading the last byte
  advanceCLK(useconst0 ? 0 : array[iter]=='1',1);
  array[iter] = ((char)digitalRead(TDI))+'0';
}
void shiftIR(){
  advanceCLK(0,1);                    //Go from Idle to shift-IR
  advanceCLK(0,1);
  advanceCLK(0,0);
  advanceCLK(0,0);
  shift(5, 0, IR_array); //Shift Instruction in
  advanceCLK(0,1);
  advanceCLK(0,0);                    //Go back to Idle
}
void shiftDR(unsigned int ammount, char useconst0){
  advanceCLK(0,1);                    //Go from Idle to shift-DR
  advanceCLK(0,0);
  advanceCLK(0,0);
  shift(ammount, useconst0,DR_array); //Shift Data in
  advanceCLK(0,1);
  advanceCLK(0,0);                    //Go back to Idle
}

void reset(){
  for(int i = 0; i < 6; i++){
    advanceCLK(0,1);
  }
  advanceCLK(0,0);                    //Go to Idle
  Serial.println("Reset");
}

void printIDCODE(){
  IR_array[0] = '1';
  IR_array[1] = '0';
  IR_array[2] = '0';
  IR_array[3] = '0';
  IR_array[4] = '0';
  shiftIR();

  shiftDR(32,1);
  for(int i = 24; i >= 0; i-=8){
    uint8_t helper = 0;
    for(int j = 0; j < 8; j++){
      helper |= (DR_array[i+j]-'0')<<j;
    }
    Serial.print(helper, HEX);
  }
  Serial.println();
}

void BSRCapture(){
  IR_array[0] = '0';
  IR_array[1] = '1';
  IR_array[2] = '0';
  IR_array[3] = '0';
  IR_array[4] = '0';

  shiftIR();
  shiftDR(0,1);     //Force Pin sample
  shiftDR(148,1);
}

void printButtonStatus(){
  BSRCapture();
  shiftDR(0,1);
  if(DR_array[3] == '1'){
    Serial.println("ON");
  }else{
    Serial.println("OFF");
  }
}

void turnLED(uint8_t status){
  if(status == ledstatus){
    return;
  }
  ledstatus = status;
  BSRCapture();                 //Sample boundary registers and change only the LED control and output registers
  DR_array[137] = status+'0';
  DR_array[138] = status+'0';
  
  IR_array[0] = '0';
  IR_array[1] = '1';
  IR_array[2] = '1';
  IR_array[3] = '0';
  IR_array[4] = '0';

  shiftIR();
  shiftDR(148,0);
}
void checkLED(){
  if(ledstatus == ON){
    ledstatus = OFF;
    turnLED(ON);
  }
}
void setup() {
  Serial.begin(9600);
  pinMode(TDI,INPUT);     //input (pic32 TD0)
  pinMode(TMS,OUTPUT);
  pinMode(TCK,OUTPUT);
  pinMode(TDO,OUTPUT);    //input (pic32 TDI)
  digitalWrite(TMS,LOW);
  digitalWrite(TCK, LOW);
  digitalWrite(TDO,LOW);
  Serial.println("Initialized\n\n");
}

void loop() {
  if(Serial.available()){
    switch(Serial.read()){//ia is returning correctly
      case 'r'://reset
        reset();
        ledstatus = OFF;
      break;
      case 'd'://IDCODE: 5090A053
        printIDCODE();
        checkLED();
      break;
      case 'b'://Button
        printButtonStatus();
        checkLED();
      break;
      case '1'://Turn led on  SCK2 RG6
        turnLED(ON);
      break;
      case '0'://Turn led off
        turnLED(OFF);
      break;
    }
  }
}

#endif