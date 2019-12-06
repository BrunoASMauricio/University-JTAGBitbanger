#include <Arduino.h>

/*
Project Specs
Authors: André Sousa (up201504227) e Bruno Maurício (up201604107)
To send to: hsm@fe.up.pt
subject: SELE_T4_B01
content: JTAG: https://git.fe.up.pt/up201604107/final_jtag_t4_b01   SYNC: ........TO ADD........
Functionalities:
If the D command is issued through the Serial input, the IDCODE from the device should be send to the Serial output
If the B command is issued through the Serial input, whether the button is pressed or not, should be send to the Serial output
If the 1 command is issued through the Serial input, the LED should be turned ON
If the 0 command is issued through the Serial input, the LED should be turned OFF
*/
//TCK on period in microseconds/2   (to read or perform other operations in between 2 sample delays)
#define SampleDelay 0.5
#define maxBSRelements 64
//uC pins
#define TMS 8  //output
#define TCK 9  //output
#define TDI 10 //input (pic32 TD0)
#define TDO 11 //output (pic32 TDI)

//Bitwise Utilities
#define Clear_Bit(Address, Bit) Address &= ~(1<<Bit)
#define Set_Bit(Address, Bit) Address |= 1<<Bit
#define Check_Bit(Address, Bit) ((Address & (1<<Bit))>>Bit)

//TAP states
#define TestLogicReset 0
#define RunTestIdle 1

#define SelectDrScan 2
#define CaptureDR 3
#define ShiftDR 4
#define Exit1DR 5
#define PauseDR 6
#define Exit2DR 7
#define UpdateDR 8

#define SelectIRScan 9
#define CaptureIR 10
#define ShiftIR 11
#define Exit1IR 12
#define PauseIR 13
#define Exit2IR 14
#define UpdateIR 15

#define GetState 16

//JTag specific defines
//IR codes
#define BYPASS 0x1F         //Used for bypassing a device in a test chain; this allows the testing of off-chip circuitry and board level interconnections.
#define HIGHZ 0x00          //Places device in a high-impedance state, all pins are forced to inputs
#define SAMPLE_PRELOAD 0x02 //Samples all pins or loads a specific value into output latch. Captures the I/O states of the component, providing a snapshot of its operation.
#define EXTEST 0x06         //Boundary scan. Allows the external circuitry and interconnections to be tested, by either forcing various test patterns on the output pins, or capturing test results from the input pins.
#define IDCODE 0x01         //Shifts out the device’s ID code

//Set the first 4 bits of ISR
#define setIR(ir_in) IR = ir_in&0x0f
#define GoToLogicReset() while(TAP(1) != TestLogicReset){ TAP(1); }

uint8_t IR;             //Instruction Register. ONLY 4 bits, LSb first
uint32_t DEVID;         //The devices' 32-bit part identifier
uint32_t BSRAmmount;    //Ammount of BSR elements
uint64_t TDOBuffer;     //64 bit Buffer that holds the current TDO output
uint64_t TDIBuffer;     //64 bit Buffer that holds the received TDI input

int iter;               //Multipurpose iterator
//------------------------------ GENERAL Purpose ------------------------------
/*
On error, send a message through the Serial
*/
void ERROR(){
}
/*
Simulates and controls the device TAP controller
*/
uint8_t TAP(uint8_t next_bit){
  static uint8_t state = 0;
  uint8_t activate = 1;
  switch(state){
    case TestLogicReset:
      if(next_bit){
        state = TestLogicReset;
      }else{
        state = RunTestIdle;
      }
      break;
    case RunTestIdle:
      if(next_bit){
        state = SelectDrScan;
      }else{
        state = RunTestIdle;
      }
      break;
    case SelectDrScan:
      if(next_bit){
        state = SelectIRScan;
      }else{
        state = CaptureDR;
      }
      break;
    case CaptureDR:
      if(next_bit){
        state = Exit1DR;
      }else{
        state = ShiftDR;
      }
      break;
    case ShiftDR:
      if(next_bit){
        state = Exit1DR;
      }else{
        state = ShiftDR;
      }
      break;
    case Exit1DR:
      if(next_bit){
        state = UpdateDR;
      }else{
        state = PauseDR;
      }
      break;
    case PauseDR:
      if(next_bit){
        state = Exit2DR;
      }else{
        state = PauseDR;
      }
      break;
    case Exit2DR:
      if(next_bit){
        state = UpdateDR;
      }else{
        state = ShiftDR;
      }
      break;
    case UpdateDR:
      if(next_bit){
        state = SelectDrScan;
      }else{
        state = RunTestIdle;
      }
      break;
    case SelectIRScan:
      if(next_bit){
        state = TestLogicReset;
      }else{
        state = CaptureIR;
      }
      break;
    case CaptureIR:
      if(next_bit){
        state = Exit1DR;
      }else{
        state = ShiftIR;
      }
      break;
    case ShiftIR:
      if(next_bit){
        state = Exit1IR;
      }else{
        state = ShiftIR;
      }
      break;
    case Exit1IR:
      if(next_bit){
        state = UpdateIR;
      }else{
        state = PauseIR;
      }
      break;
    case PauseIR:
      if(next_bit){
        state = Exit2IR;
      }else{
        state = PauseIR;
      }
      break;
    case Exit2IR:
      if(next_bit){
        state = UpdateIR;
      }else{
        state = ShiftIR;
      }
      break;
    case UpdateIR:
      if(next_bit){
        state = SelectDrScan;
      }else{
        state = RunTestIdle;
      }
      break;
    case GetState:
      activate = 0;
      break;
    default:
      ERROR();
      break;
  }
  if(activate){
    digitalWrite(TCK,LOW);
    digitalWrite(TMS,next_bit);
    digitalWrite(TCK,HIGH);
    delayMicroseconds(2*SampleDelay);
    digitalWrite(TCK,LOW);
  }
  return state;
}
/*
Sets TAP to a specified destination state
*/
void setTAP(uint8_t destination_state){
  uint16_t path;
  uint8_t path_length;
  if(TAP(GetState) != destination_state){           //Check if TAP is already at the destination
    GoToLogicReset();
    switch(destination_state){
      case ShiftIR:
        path = 0b00110;
        path_length = 5;
        break;
      case ShiftDR:
        path = 0b0010;
        path_length = 4;
        break;
      default:
        ERROR();
        break;
    }
    for(iter = 0; iter < path_length; iter++){
      TAP(path&0x01);
      path = path >> 1;
    }
  }
}
/*
Write ammount bits from TDO, and reads ammount of bits into TDI
Clears TDO in the process
An iterable by TMS 0 state (RunTestIdle, ShiftDR, ShiftIR, PauseDr or PauseIR) is assumed
LSb first
*/
void communicate(uint8_t ammount){
  if(ammount > 64){
    ERROR();
    return;
  }
  digitalWrite(TCK, LOW);
  digitalWrite(TMS, LOW);
  for(iter = 0; iter < ammount; iter++){
    digitalWrite(TDO, TDOBuffer&0x01);
    digitalWrite(TCK, HIGH);
    delayMicroseconds(SampleDelay);
    Set_Bit(TDIBuffer,digitalRead(TDI));
    delayMicroseconds(SampleDelay);
    digitalWrite(TCK, LOW);
    TDOBuffer = TDOBuffer>>1;
  }
}
/*
Writes a certain ammount of data into TDOBuffer, LSb first
*/
void setTDO(uint64_t data, uint8_t ammount){
  for(iter = 0; iter < ammount; iter++){
    if(Check_Bit(data,iter)){
      Set_Bit(TDOBuffer,iter);
    }else{
      Clear_Bit(TDOBuffer,iter);
    }
  }
}
/*
Reads a certain ammount of TDIBuffer into data
*/
void getTDI(uint64_t* data, uint8_t ammount){
  for(iter = 0; iter < ammount; iter++){
    if(Check_Bit(TDIBuffer,iter)){
      Set_Bit(*data,iter);
    }else{
      Clear_Bit(*data,iter);
    }
  }
}
//------------------------------ JTAG functions ------------------------------
/*
Load Instruction Register with the scpecified Instruction Register Code
*/
void loadIR(uint8_t IRC){
  if(IR != IRC){            //Check if the IRC is already loaded in the IR
    setTAP(ShiftIR);        //Set TAP to ShiftIR
    setIR(IRC);             //Prepare and output IRC
    setTDO(IR,4);
    communicate(4);
  }
}
/*
Load Data Register with the scpecified ammount of data
*/
void loadDR(uint64_t data, uint8_t ammount){
  setTAP(ShiftDR);        //Set TAP to ShiftIR
  setTDO(data,ammount);   //Output data
  communicate(ammount);
}
//------------------------------ Functions ------------------------------
/*
Test Boundary Scan Register integrity
*/
uint8_t testJTAG(){
  uint64_t data;
  GoToLogicReset();
  //Test bypass bit
  loadIR(BYPASS);
  setTDO(1,1);          //Clear TDOBuffer
  communicate(3);       //If everything is ok, should receive 01
  getTDI(&data,1);
  if(data != 1){
    ERROR();
  }
  //Test BSR
  loadIR(EXTEST);
  setTDO(1,1);          //Clear TDOBuffer
  communicate(3);       //If everything is ok, should receive 01
  getTDI(&data,1);
  if(data != 1){
    ERROR();
  }

  return 1;
}

uint32_t detectBSRAmmount(){
  GoToLogicReset();

  return BSRAmmount;
}

void setup() {
  Serial.begin(9600);
  setIR(0x00);
  DEVID = 0;
  BSRAmmount = 0;    
  TDOBuffer = 0;
  TDIBuffer = 0;
  pinMode(TMS,OUTPUT);
  pinMode(TCK,OUTPUT);
  pinMode(TDI,INPUT);     //input (pic32 TD0)
  pinMode(TDO,OUTPUT);    //input (pic32 TDI)
  digitalWrite(TMS,LOW);
  digitalWrite(TCK,LOW);
  digitalWrite(TDO,LOW);
}

void loop() {
  testBSR();
}