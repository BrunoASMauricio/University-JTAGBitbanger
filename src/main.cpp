#include <Arduino.h>
/*
Logic Analyzer
Anotations
Timing Marker Pair
|A1 - A2| = ###

8MS/s for 1 s
TCK rising edge

Analyzers:
JTAG
TAP initial: Run-Test/Idle
LSB first
show TDI TDO count

Async Serial -> tx
*/

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

uint8_t byte_array[148];
uint16_t iter;               //Multipurpose iterator
uint16_t iter2;               //Multipurpose iterator

void print(const char* message, int ammount){
  if(ammount == -1){
    Serial.println(message);
  }else{
    Serial.print(message);
    Serial.println(ammount);
  }
}
void print_bin(const char* message,uint16_t in){
  Serial.println(message);
  for(iter2 = 0; iter2 < 16; iter2++){
    Serial.write((char)(in>>iter2&0x01)+48);
  }
  Serial.println();
}
void clock() {
  digitalWrite(TCK, HIGH);
  delayMicroseconds(2);
  digitalWrite(TCK, LOW);
  delayMicroseconds(2);
}
void advanceCLK(int tdo, int tms){
  digitalWrite(TDO, tdo);
  digitalWrite(TMS,tms);
  clock();
}


void setup() {
  Serial.begin(9600);
  print("Starting\n\n",-1);
  pinMode(TDI,INPUT);     //input (pic32 TD0)
  pinMode(TMS,OUTPUT);
  pinMode(TCK,OUTPUT);
  pinMode(TDO,OUTPUT);    //input (pic32 TDI)
  digitalWrite(TMS,LOW);
  digitalWrite(TCK, HIGH);
  digitalWrite(TDO,LOW);
}
void shiftIR(unsigned int ammount, char useconst0){
  advanceCLK(0,1);
  advanceCLK(0,0);
  advanceCLK(0,0);
  if(useconst0){
    for(iter = 0; iter < ammount-1; iter++){
      advanceCLK(0,0);
      byte_array[iter] = ((char)digitalRead(TDI))+'0';
      Serial.print((char)byte_array[iter]);
    }
    advanceCLK(0,1);
    byte_array[iter] = ((char)digitalRead(TDI))+'0';
    Serial.print((char)byte_array[iter]);

  }else{
    for(iter = 0; iter < ammount-1; iter++){
      advanceCLK(byte_array[iter]=='1',0);
      byte_array[iter] = ((char)digitalRead(TDI))+'0';
    }
    advanceCLK(byte_array[iter]=='1',1);
    byte_array[iter] = ((char)digitalRead(TDI))+'0';
  }
  advanceCLK(0,1);
  advanceCLK(0,1);
}
void shiftDR(unsigned int ammount, char useconst0){
  advanceCLK(0,0);
  advanceCLK(0,0);
  char temp;
  if(useconst0){
    for(iter = 0; iter < ammount-1; iter++){
      byte_array[iter] = ((char)digitalRead(TDI))+'0';
      advanceCLK(0,0);
    }
    byte_array[iter] = ((char)digitalRead(TDI))+'0';
    advanceCLK(0,1);
  }else{
    for(iter = 0; iter < ammount-1; iter++){
      temp = ((char)digitalRead(TDI))+'0';
      advanceCLK(byte_array[iter]=='1',0);
      byte_array[iter] = temp;
    }
    temp = ((char)digitalRead(TDI))+'0';
    advanceCLK(byte_array[iter]=='1',1);
    byte_array[iter] = temp;
  }
  advanceCLK(0,1);
  advanceCLK(0,1);
}

void loop() {
  if(Serial.available()){
    switch(Serial.read()){//ia is returning correctly
      case 'r'://reset
        digitalWrite(TMS,LOW);
        digitalWrite(TCK, HIGH);
        digitalWrite(TDO,LOW);
        for(int i = 0; i < 6; i++){
          advanceCLK(0,1);
        }
        advanceCLK(0,0);
        advanceCLK(0,1);
        Serial.println("Initialized");
      break;
      case 'a'://IDCODE: 1001 0100 0000 1010 0001 0010 0001 0100
        //DEVICE ID 0A 0000 1010
        byte_array[0] = '1';
        byte_array[1] = '0';
        byte_array[2] = '0';
        byte_array[3] = '0';
        byte_array[4] = '0';
        shiftIR(5,0);
        shiftDR(32,1);
        for(int i = 0; i < 32; i++) Serial.print((char)byte_array[i]);
        Serial.println();
      break;
      case 'b'://Button
        //
        byte_array[0] = '0';
        byte_array[1] = '1';
        byte_array[2] = '0';
        byte_array[3] = '0';
        byte_array[4] = '0';
        shiftIR(5,0);
        shiftDR(148,1);
        for(int i = 0; i < 148; i++) Serial.print((char)byte_array[i]);
        Serial.println();
        Serial.println((char)byte_array[135]);
        /*

        */
        /*
10 1 1011011011010000111011011011011011011010011010011010010010011011010011011011010010010011011010011011011010010010011011010010011001000011101101100
10 0 1011011011010000111011011011011011011010011010011010010010011011010011011011010010010011011010011011011010010010011011010010011001000011101101100

        10 1 10110100110100001110110110111
        10 0 10110100110100001110110110111
        */
       if(byte_array[3] == '1'){
         Serial.println("ON");
       }else{
         Serial.println("OFF");
       }
      break;
      case '1'://Turn led on  SCK2 RG6
        byte_array[0] = '0';
        byte_array[1] = '1';
        byte_array[2] = '1';
        byte_array[3] = '0';
        byte_array[4] = '0';
        shiftIR(5,0);
        /*
        advanceCLK(0,0);
        advanceCLK(0,0);
        
        advanceCLK(1,0);//147
        advanceCLK(1,0);//146
        advanceCLK(1,1);//145
        
        advanceCLK(0,1);
        advanceCLK(0,1);
        */
        shiftDR(148,1);
        byte_array[137] = '1';
        byte_array[138] = '1';
        shiftDR(148,0);
      break;
      case '0'://Turn led off
        byte_array[0] = '0';
        byte_array[1] = '1';
        byte_array[2] = '1';
        byte_array[3] = '0';
        byte_array[4] = '0';
        shiftIR(5,0);
        /*
        advanceCLK(0,0);
        advanceCLK(0,0);
        
        advanceCLK(0,0);//147
        advanceCLK(1,0);//146
        
        advanceCLK(0,1);
        advanceCLK(0,1);
        */
        shiftDR(148,1);
        byte_array[137] = '0';
        byte_array[138] = '1';
        shiftDR(148,0);
      break;
      case 'e':
        shiftDR(32,1);
      break;
    }
  }
}