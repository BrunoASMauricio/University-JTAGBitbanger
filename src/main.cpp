#ifndef MAIN_CPP
#define MAIN_CPP

#include "main.h"

uint8_t byte_array[148];
uint16_t iter;               //Multipurpose iterator
uint16_t iter2;               //Multipurpose iterator
uint8_t ledstatus;

void advanceCLK(int tdo, int tms){
  digitalWrite(TDO, tdo);
  digitalWrite(TMS,tms);
  digitalWrite(TCK, HIGH);
  //delayMicroseconds(2);
  digitalWrite(TCK, LOW);
  //delayMicroseconds(2);
}

void shift(unsigned int ammount, char useconst0){
  char temp;
  if(ammount){
    for(iter = 0; iter < ammount-1; iter++){
      temp = ((char)digitalRead(TDI))+'0';
      advanceCLK(useconst0 ? 0 : byte_array[iter]=='1',0);
      byte_array[iter] = temp;
    }
  }
  temp = ((char)digitalRead(TDI))+'0';
  advanceCLK(useconst0 ? 0 : byte_array[iter]=='1',1);
  byte_array[iter] = temp;
}

void shiftIR(unsigned int ammount, char useconst0){
  advanceCLK(0,1);
  advanceCLK(0,1);
  advanceCLK(0,0);
  advanceCLK(0,0);
  shift(ammount, useconst0);
  advanceCLK(0,1);
  advanceCLK(0,0);
}
void shiftDR(unsigned int ammount, char useconst0){
  advanceCLK(0,1);
  advanceCLK(0,0);
  advanceCLK(0,0);
  shift(ammount, useconst0);
  advanceCLK(0,1);
  advanceCLK(0,0);
}

void reset(){
  for(int i = 0; i < 6; i++){
    advanceCLK(0,1);
  }
  advanceCLK(0,0);
  Serial.println("Reset");
}
void printIDCODE(){
  //DEVICE ID CA05090A
  byte_array[0] = '1';
  byte_array[1] = '0';
  byte_array[2] = '0';
  byte_array[3] = '0';
  byte_array[4] = '0';
  shiftIR(5,0);
  shiftDR(32,1);
  for(int i = 24; i >= 0; i-=8){
    uint8_t helper = 0;
    for(int j = 0; j < 8; j++){
      helper |= (byte_array[i+j]-'0')<<j;
    }
    Serial.print(helper, HEX);
  }
  Serial.println();
}
void printButtonStatus(){
  byte_array[0] = '0';
  byte_array[1] = '1';
  byte_array[2] = '0';
  byte_array[3] = '0';
  byte_array[4] = '0';

  shiftIR(5,0);
  shiftDR(0,1);//Force PIN sample
  shiftDR(148,1);
  shiftDR(0,1);

  if(byte_array[3] == '1'){
    Serial.println("ON");
  }else{
    Serial.println("OFF");
  }
  if(ledstatus == ON){
    //ledstatus = OFF;
    //turnLED(ON);
  }
}
void turnLED(uint8_t status){
  //Serial.print("LED: ");
  //Serial.println(status);
  if(status == ledstatus){
    //return;
  }
  ledstatus = status;
  byte_array[0] = '0';
  byte_array[1] = '1';
  byte_array[2] = '1';
  byte_array[3] = '0';
  byte_array[4] = '0';

  shiftIR(5,0);
  shiftDR(148,1);                 //Read BSR
  byte_array[137] = status+'0';   //Change LED status
  byte_array[138] = '1';
  Serial.println(status);
  shiftDR(148,0);

}
void setup() {
  Serial.begin(9600);
  ledstatus = OFF;
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
      break;
      case 'a'://IDCODE: 1001 0100 0000 1010 0001 0010 0001 0100
        printIDCODE();
      break;
      case 'b'://Button
        printButtonStatus();
      break;
      case '1'://Turn led on  SCK2 RG6
        turnLED(ON);
      break;
      case '0'://Turn led off
        turnLED(OFF);
      break;
      case 'e':
        shiftDR(32,1);
      break;
    }
  }
}

#endif