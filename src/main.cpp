#include <Arduino.h>

#define TRS 8  //output
#define TCK 9  //output
#define TDI 10 //input (pic32 TD0)
#define TDO 11 //output (pic32 TDI)
/*
email:
subject: SELE_T4_B01
2 links, separated git.fe.up.pt repositories
1 for JTAG
1 for SYNC
*/
void setup() {
  pinMode(TRS,OUTPUT);
  pinMode(TCK,OUTPUT);
  pinMode(TDI,INPUT);     //input (pic32 TD0)
  pinMode(TDO,OUTPUT);    //input (pic32 TDI)
}

void loop() {
  
}