// Arthur: Huy Duong Gia. Borre. Date: 21/05/2023
// Master thesis: Fourier Ptychography Microscopy with UV and IR light
// Hardware: Custom drive electronics uses three STLED524 Integrated Chips (IC) to control 289 LEDs arranged in a 17x17 matrix. 
//           Each STLED524 IC is capable of controlling up to 120 LEDs individually and can adjust the LED current from 0 to 35mA in 256 levels. 
//           The first IC control the first 119 LEDs, from row 1 to row 7 
//           The second IC control the second 119 LEDs, from row 8 to row 14
//           The third IC control the remained 51 LEDs, from row 15 to row 17 
//           The communication between the microcontroller and the STLED524 ICs is established through the SPI communication protocol. 
//           An Arduino board is used as the microcontroller to receive commands from the computer and send control commands to the STLED524 ICs
// Function: Receive commands from computer to
//           - Turn on the LEDs in 17x17 LED matrix one by one
//           - Adjust the current through LED from 0 to 35mA in 255 levels


#include<SPI.h>
#include<math.h>
#include<stdio.h>


int SS1 =53;
int SS2 = 2;
int SS3 = 3;
int SYNC = 4;
int Reset = 5;
int CLKIN1 = 7;
int CLKIN2 = 8;
int CLKIN3 = 9;

char serialReceivedData[20];
int serialDataCount =0;
int cnt=0;
int CLKIN_pulse_width = 1; // in miliseconds
int CLKIN_pulse_width_2 = 10; //in microseconds
int number_of_clock_to_turn_on_row[3][5] = {{1, 300, 812, 1324, 1836}, {10, 522, 1034, 1546, 2058}, {30, 542, 1054, 1566, 2078}};

void setup() {
  // put your setup code here, to run once:
  // initial hardware setup
  pinMode(SS1, OUTPUT);
  digitalWrite(SS1, HIGH);
  pinMode(SS2, OUTPUT);
  digitalWrite(SS2, HIGH);
  pinMode(SS3, OUTPUT);
  digitalWrite(SS3, HIGH);
  pinMode(Reset, OUTPUT);
  digitalWrite(Reset, HIGH);
  pinMode(SYNC, OUTPUT);
  digitalWrite(SYNC, HIGH);

  pinMode(CLKIN1, OUTPUT);
  digitalWrite(CLKIN1, HIGH);
  
  Serial.begin(9600);
  Serial.println("Hello I'm SPI Mega_Master");
  SPI.begin();        //PB2 - PB4 are converted to SS/(OUT-H), MOSI(OUT-L), MISO(IN), SCK(OUT-L)
  //bitSet(SPCR, 4);    //UNO_1 is Master
  SPI.setBitOrder(MSBFIRST);  //bit transmission order
  SPI.setDataMode(SPI_MODE0); //CPOL and CPHA (Clock polarity; Clock phase)
  SPI.setClockDivider(SPI_CLOCK_DIV128); //data rate = fosc/128 = 125 kbit

  resetLedMatrix();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()>0){
    char data = Serial.read();
    switch (data){
      case 's':
      memset(serialReceivedData, '\0', sizeof(serialReceivedData));
      serialReceivedData[serialDataCount] = data;
      serialDataCount++;
      case 'e':
      serialReceivedData[serialDataCount] = data;
      serialDataCount = 0;
      decodeCommand(serialReceivedData);
      break;
      default:
      serialReceivedData[serialDataCount] = data;
      serialDataCount++;
    }
  }
}

void resetLedMatrix(){
  digitalWrite(Reset, LOW);
  delay(1);
  digitalWrite(Reset, HIGH);
  delay(1);
}

void make_many_clock(int pin, int pulse_width, int NoOfClock){
  int i = 1;
  for(i=1; i<=NoOfClock; i++){
    make_one_clock(pin, pulse_width);
  }
}

void make_one_clock(int pin, int pulse_width){
  digitalWrite(pin, LOW);
  delay(pulse_width);
  digitalWrite(pin, HIGH);
  delay(pulse_width);
}

void make_many_clock_in_us(int pin, int pulse_width, int NoOfClock){
  int i = 1;
  for(i=1; i<=NoOfClock; i++){
    make_one_clock_in_us(pin, pulse_width);
  }
}

void make_one_clock_in_us(int pin, int pulse_width){
  digitalWrite(pin, LOW);
  delayMicroseconds(pulse_width);
  digitalWrite(pin, HIGH);
  delayMicroseconds(pulse_width);
}

void decodeCommand(byte serialReceivedData[]){
  switch (serialReceivedData[0]){
    case 'a':
    int row, col;
    row = (serialReceivedData[1] - 48)*10+serialReceivedData[2] - 48;
    col = (serialReceivedData[3] - 48)*10+serialReceivedData[4] - 48;
    displayLedIntermOfRowAndCol17x17(row, col);
    break;
    case 'b':
    // set PWM
    byte PWM_value;
    PWM_value = (serialReceivedData[1]-48)*100 + (serialReceivedData[2]-48)*10 + (serialReceivedData[3]-48);
    setPWMforAll(PWM_value);
    break;
    case 'c':
    resetLedMatrix();
    break;
  }
}

void setClockRegister(int slaveNo, byte ClockRegisterValue){
  int ss;
  ss = getSSPin(slaveNo);

  // choose slave
  digitalWrite(ss, LOW);
  // writing data
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x10); // blank time register address
  SPI.transfer(ClockRegisterValue); // 
  
  // close slave
  digitalWrite(ss, HIGH);
}

void setBlankTime(int slaveNo, byte blankValue){
  int ss;
  ss = getSSPin(slaveNo);

  // choose slave
  digitalWrite(ss, LOW);
  // writing data
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x15); // blank time register address
  SPI.transfer(blankValue); // 
  
  // close slave
  digitalWrite(ss, HIGH);
}

void displayPattern1(int slaveNo) {
  int ss;
  ss = getSSPin(slaveNo);

  // choose slave
  digitalWrite(ss, LOW);
  // writing data
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x00);
  SPI.transfer(0x01);
  SPI.transfer(0x01);
  
  // close slave
  digitalWrite(ss, HIGH);
}

void turnOffAllDisplay(){
  turnOffDisplay(1);
  turnOffDisplay(2);
  turnOffDisplay(3);
}

void turnOffDisplay(int slaveNo){
  int ss;
  ss = getSSPin(slaveNo);
  // choose slave
  digitalWrite(ss, LOW);
  // writing data
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x00); // software control register address
  SPI.transfer(0x00); // EN = 0
  SPI.transfer(0x00); // DISP 2 and DISP1 = 0, no pattern is displayed
  // close slave
  digitalWrite(ss, HIGH);
}

void clearAllPattern(){
  clearPattern(1);
  clearPattern(2);
  clearPattern(3);
}

void clearPattern(int slaveNo){
  int ss;
  ss = getSSPin(slaveNo);
  // choose slave
  digitalWrite(ss, LOW);
  // writing data
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x00); // software control register address
  SPI.transfer(0x06); // CLR2 and CLR1 = 1
  // close slave
  digitalWrite(ss, HIGH);

  // delay between two command
  delay(1);

  // return software control register to default 
  // choose slave
  digitalWrite(ss, LOW);
  // writing data
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x00); // software control register address
  SPI.transfer(0x00); // CLR2 and CLR1 = 0
  // close slave
  digitalWrite(ss, HIGH);
}

void setPWMforAll(byte PWM_value){
  setPWM(1, PWM_value);
  setPWM(2, PWM_value);
  setPWM(3, PWM_value);
}

void setPWM(int slaveNo, byte PWM_value){
  int ss;
  ss = getSSPin(slaveNo);

  // choose slave
  digitalWrite(ss, LOW);
  
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x20);
  SPI.transfer(0x02);
  SPI.transfer(PWM_value);

  // close slave
  digitalWrite(ss, HIGH);
}

void disablePWM(int slaveNo){
  int ss;
  ss = getSSPin(slaveNo);

  // choose slave
  digitalWrite(ss, LOW);
  
  SPI.transfer(0x00); // write to controll resister
  SPI.transfer(0x20);
  SPI.transfer(0x00);

  // close slave
  digitalWrite(ss, HIGH);
}

void scanLedMatrix(){
  int row, col;
  for(row=1; row<=17; row++){
    for(col=1; col<=17; col++){
      displayLedIntermOfRowAndCol17x17(row, col);
      delay(500);
    }
  }
}

void displayLedIntermOfRowAndCol17x17(int row, int col){
  // this function turn on only 1 LED in LED matrix 17x17 
  // the LED position is call in term of row and colume
  // row <= 17, col <= 17
  // (IC,row,col) is the position of LED in Matrix
  // digram show the LED postion
  // (1,1,1) (1,1,2) (1,1,3) ... (1,1,17)                                                              // 17 LEDs
  // (1,2,1) (1,2,2) (1,2,3) ... (1,2,17)                                                              // 17 LEDs
  // (1,3,1) (1,3,2) (1,3,3) ... (1,3,17)                                                              // 17 LEDs
  // (1,4,1) (1,4,2) (1,4,3) ... (1,4,17)                                                              // 17 LEDs
  // (1,5,1) (1,5,2) (1,5,3) ... (1,5,17)                                                              // 17 LEDs
  // (1,1,18) (1,1,19) ... (1,1,24) (1,2,18) (1,2,19) ... (1,2,24) (1,5,18) (1,5,19) (1,5,20)          // 17 LEDs
  // (1,3,18) (1,3,19) ... (1,3,24) (1,4,18) (1,4,19) ... (1,4,24) (1,5,21) (1,5,22) (1,5,23)          // 17 LEDs
  // (2,1,1) (2,1,2) (2,1,3) ... (2,1,17)                                                              // 17 LEDs
  // (2,2,1) (2,2,2) (2,2,3) ... (2,2,17)                                                              // 17 LEDs
  // (2,3,1) (2,3,2) (2,3,3) ... (2,3,17)                                                              // 17 LEDs
  // (2,4,1) (2,4,2) (2,4,3) ... (2,4,17)                                                              // 17 LEDs
  // (2,5,1) (2,5,2) (2,5,3) ... (2,5,17)                                                              // 17 LEDs
  // (2,1,18) (2,1,19) ... (2,1,24) (2,2,18) (2,2,19) ... (2,2,24) (2,5,18) (2,5,19) (2,5,20)          // 17 LEDs
  // (2,3,18) (2,3,19) ... (2,3,24) (2,4,18) (2,4,19) ... (2,4,24) (2,5,21) (2,5,22) (2,5,23)          // 17 LEDs
  // (3,1,1) (3,1,2) (3,1,3) ... (3,1,17)                                                              // 17 LEDs
  // (3,2,1) (3,2,2) (3,2,3) ... (3,2,17)                                                              // 17 LEDs
  // (3,3,1) (3,3,2) (3,3,3) ... (3,3,17)                                                              // 17 LEDs
  resetLedMatrix();

  int slaveNo;
  slaveNo = encodeSlaveNo(row);
  
  int encoded_row_interm_of_7x17;
  encoded_row_interm_of_7x17 = row - (slaveNo-1)*7;

  int encoded_row_interm_of_5x24;
  encoded_row_interm_of_5x24 = encodeRowIntermOf5x24(encoded_row_interm_of_7x17, col);

  int encoded_col_interm_of_5x24;
  encoded_col_interm_of_5x24 = encodeColumnIntermOf5x24(encoded_row_interm_of_7x17, col);

  setupAndDisplay(slaveNo, encoded_row_interm_of_5x24, encoded_col_interm_of_5x24);
  
}

void setupAndDisplay(int slaveNo, int row_interm_of_5x24, int col_interm_of_5x24){
  if(slaveNo == 1){
    setupAndDisplaySlave1(row_interm_of_5x24, col_interm_of_5x24);
  }
  else if (slaveNo == 2){
    setupAndDisplaySlave2(row_interm_of_5x24, col_interm_of_5x24);
  }
  else {
    setupAndDisplaySlave3(row_interm_of_5x24, col_interm_of_5x24);
  }
}

void setupAndDisplaySlave1(int row_interm_of_5x24, int col_interm_of_5x24){
  int slaveNo = 1;
  setBlankTime(slaveNo, 0x00); // 2 T_CLKs
  setPWM(slaveNo, 0xFF);
  setClockRegister(slaveNo, 0x0D); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 0; CLKIN = 0; CLKOUT = 1
  writePattern1DataIntermOfRowAndCol5x24(slaveNo, row_interm_of_5x24, col_interm_of_5x24);
  displayPattern1(slaveNo);
  
  make_many_clock(CLKIN1, CLKIN_pulse_width, 11);
  setClockRegister(slaveNo, 0x0F);
  
  int number_of_clock;
  number_of_clock = getNumberOfClockToTurnOnRow(1, row_interm_of_5x24);
  make_many_clock_in_us(CLKIN1, CLKIN_pulse_width_2, number_of_clock);
}

void setupAndDisplaySlave2(int row_interm_of_5x24, int col_interm_of_5x24){
  // set clock IC 1
  setClockRegister(1, 0x0D); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 0; CLKIN = 0; CLKOUT = 0
  make_many_clock(CLKIN1, CLKIN_pulse_width, 10);
  setClockRegister(1, 0x0F);
  make_one_clock(CLKIN1, CLKIN_pulse_width);

  // set IC 2
  setBlankTime(2, 0x00); // 2 T_CLKs
  setPWM(2, 0xFF);
  setClockRegister(2, 0x0F); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 1; CLKIN = 1; CLKOUT = 1
  writePattern1DataIntermOfRowAndCol5x24(2, row_interm_of_5x24, col_interm_of_5x24);
  displayPattern1(2);
  //make_many_clock(CLKIN1, CLKIN_pulse_width, 512);
  make_many_clock_in_us(CLKIN1, CLKIN_pulse_width_2, 512);

  int number_of_clock;
  number_of_clock = getNumberOfClockToTurnOnRow(2, row_interm_of_5x24);
  //make_many_clock(CLKIN1, CLKIN_pulse_width, number_of_clock);
  make_many_clock_in_us(CLKIN1, CLKIN_pulse_width_2, number_of_clock);
}

void setupAndDisplaySlave3(int row_interm_of_5x24, int col_interm_of_5x24){
  // set clock IC 1
  setClockRegister(1, 0x0D); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 0; CLKIN = 0; CLKOUT = 1
  make_many_clock(CLKIN1, CLKIN_pulse_width, 10);
  setClockRegister(1, 0x0F);
  make_one_clock(CLKIN1, CLKIN_pulse_width);

  // set clock IC 2
  setClockRegister(2, 0x0F); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 1; CLKIN = 1; CLKOUT = 1

  // set IC 3
  setBlankTime(3, 0x00); // 2 T_CLKs
  setPWM(3, 0xFF);
  setClockRegister(3, 0x0E); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 1; CLKIN = 1; CLKOUT = 0
  writePattern1DataIntermOfRowAndCol5x24(3, row_interm_of_5x24, col_interm_of_5x24);
  displayPattern1(3);
  //make_many_clock(CLKIN1, CLKIN_pulse_width, 512);
  make_many_clock_in_us(CLKIN1, CLKIN_pulse_width_2, 512);

  int number_of_clock;
  number_of_clock = getNumberOfClockToTurnOnRow(3, row_interm_of_5x24);
  //make_many_clock(CLKIN1, CLKIN_pulse_width, number_of_clock);
  make_many_clock_in_us(CLKIN1, CLKIN_pulse_width_2, number_of_clock);
}

int getNumberOfClockToTurnOnRow(int slaveNo, int row){
  return number_of_clock_to_turn_on_row[slaveNo-1][row-1];
}

int encodeSlaveNo(int row){
  float tempRow = row;
  int slaveNo;
  slaveNo = ceil(tempRow/7.0);
  return slaveNo;
}

int encodeRowIntermOf5x24(int row, int col){
  if (row<=5){
    return row;
  }
  else if (row == 6){
    // write data pattern to slaver number 1
    if (col<=7){
      return 1;
    }
    else if (col>7 && col<=14){
      return 2;
    }
    else{
      return 5;
    }
  }
  else if (row == 7){
    if (col<=7){
      return 3;
    }
    else if (col>7 && col<=14){
      return 4;
    }
    else{
      return 5;
    }
  }
}

int encodeColumnIntermOf5x24(int row, int col){
  if (row<=5){
    return col;
  }
  else if (row == 6){
    if (col<=7){
      return col+17;
    }
    else if (col>7 && col<=14){
      return col+10;
    }
    else{
      return col+3;
    }
  }
  else if (row == 7){
    if (col<=7){
      return col+17;
    }
    else if (col>7 && col<=14){
      return col+10;
    }
    else{
      return col+6;
    }
  }
}

// void encodeLedPositionAndSendPatternData(int slaveNo, int row, int col){
//   // this function encode the LED position in LED matrix 17x17
//   // the LED position is call in term of row and colume and IC or slaver
//   // row <= 17, col <= 17
//   // (IC,row,col) is the position of LED in Matrix
//   // digram show the LED postion
//   // (1,1,1) (1,1,2) (1,1,3) ... (1,1,17)                                                              // 17 LEDs
//   // (1,2,1) (1,2,2) (1,2,3) ... (1,2,17)                                                              // 17 LEDs
//   // (1,3,1) (1,3,2) (1,3,3) ... (1,3,17)                                                              // 17 LEDs
//   // (1,4,1) (1,4,2) (1,4,3) ... (1,4,17)                                                              // 17 LEDs
//   // (1,5,1) (1,5,2) (1,5,3) ... (1,5,17)                                                              // 17 LEDs
//   // (1,1,18) (1,1,19) ... (1,1,24) (1,2,18) (1,2,19) ... (1,2,24) (1,5,18) (1,5,19) (1,5,20)          // 17 LEDs
//   // (1,3,18) (1,3,19) ... (1,3,24) (1,4,18) (1,4,19) ... (1,4,24) (1,5,21) (1,5,22) (1,5,23)          // 17 LEDs
//   // (2,1,1) (2,1,2) (2,1,3) ... (2,1,17)                                                              // 17 LEDs
//   // (2,2,1) (2,2,2) (2,2,3) ... (2,2,17)                                                              // 17 LEDs
//   // (2,3,1) (2,3,2) (2,3,3) ... (2,3,17)                                                              // 17 LEDs
//   // (2,4,1) (2,4,2) (2,4,3) ... (2,4,17)                                                              // 17 LEDs
//   // (2,5,1) (2,5,2) (2,5,3) ... (2,5,17)                                                              // 17 LEDs
//   // (2,1,18) (2,1,19) ... (2,1,24) (2,2,18) (2,2,19) ... (2,2,24) (2,5,18) (2,5,19) (2,5,20)          // 17 LEDs
//   // (2,3,18) (2,3,19) ... (2,3,24) (2,4,18) (2,4,19) ... (2,4,24) (2,5,21) (2,5,22) (2,5,23)          // 17 LEDs
//   // (3,1,1) (3,1,2) (3,1,3) ... (3,1,17)                                                              // 17 LEDs
//   // (3,2,1) (3,2,2) (3,2,3) ... (3,2,17)                                                              // 17 LEDs
//   // (3,3,1) (3,3,2) (3,3,3) ... (3,3,17)                                                              // 17 LEDs

//   if (row<=5){
//     // write data pattern to slaver number 1
//     writePattern1DataIntermOfRowAndCol5x24(slaveNo, row, col);
//   }
//   else if (row == 6){
//     // write data pattern to slaver number 1
//     if (col<=7){
//       writePattern1DataIntermOfRowAndCol5x24(slaveNo, 1, col+17);
//     }
//     else if (col>7 && col<=14){
//       writePattern1DataIntermOfRowAndCol5x24(slaveNo, 2, col+10);
//     }
//     else{
//       writePattern1DataIntermOfRowAndCol5x24(slaveNo, 5, col+3);
//     }
//   }
//   else if (row == 7){
//     // write data pattern to slaver number 1
//     if (col<=7){
//       writePattern1DataIntermOfRowAndCol5x24(slaveNo, 3, col+17);
//     }
//     else if (col>7 && col<=14){
//       writePattern1DataIntermOfRowAndCol5x24(slaveNo, 4, col+10);
//     }
//     else{
//       writePattern1DataIntermOfRowAndCol5x24(slaveNo, 5, col+6);
//     }
//   }
// }

void writePattern1DataIntermOfRowAndCol5x24(int slaveNo, int row, int col){
  // function write the pattern data to pattern 1 of IC
  // The LED is called in term of row and col of IC 5x24
  // row <=5, col <=24
  // slaveNo is the slaver number, should be <= 3
  int ss;
  ss = getSSPin(slaveNo);

  int LedPositionInPattern;
  LedPositionInPattern = 1+(col-1)*5 + row-1;
  // choose slave
  digitalWrite(ss, LOW);
  
  SPI.transfer(0x02);
  SPI.transfer(0x00);
  int i = 1;
  for(i=1; i<121; i++){
    if (i==LedPositionInPattern){
      SPI.transfer(0xFF);
    }
    else{
      SPI.transfer(0x00);
    }
    if (i<120){
      SPI.transfer(0x00);
    }
  }

  // close slave
  digitalWrite(ss, HIGH);
}

// void writePattern1DataToDisplayLedNo(int slaveNo, int LED_No){
//   //LED_No is the LED number, should be <= 120
//   //slaveNo is the slaver number, should be <= 3
//   int ss;
//   ss = getSSPin(slaveNo);

//   int LedPositionInPattern, rowPos, colPos;
//   float LedNumber = LED_No;
//   rowPos = ceil(LedNumber/24);
//   if (fmod(LED_No,24) == 0)
//   {
//     colPos = 24;
//   }
//   else
//   {
//     colPos = fmod(LED_No,24);
//   }
//   LedPositionInPattern = 1+(colPos-1)*5 + rowPos-1;
//   // choose slave
//   digitalWrite(ss, LOW);
  
//   SPI.transfer(0x02);
//   SPI.transfer(0x00);
//   int i = 1;
//   for(i=1; i<121; i++){
//     if (i==LedPositionInPattern){
//       SPI.transfer(0xFF);
//     }
//     else{
//       SPI.transfer(0x00);
//     }
//     if (i<120){
//       SPI.transfer(0x00);
//     }
//   }

//   // close slave
//   digitalWrite(ss, HIGH);
// }

// void scanAllLed(int slaveNo){
//   //LED_No is the LED number, should be <= 120
//   //slaveNo is the slaver number, should be <= 3
//   int ss;
//   ss = getSSPin(slaveNo);

//   delay(1);
//   setBlankTime(slaveNo, 0x00); // 2 T_CLKs
//   delay(1);
//   setClockRegister(slaveNo, 0x80); //REFSEL = 0; SYNCSEL = 1; SYNCEN = 0; CLKIN = 0; CLKOUT = 0
//   delay(1);
//   setPWM(slaveNo, 0x80);
//   delay(1);
//   displayPattern1(slaveNo);
//   delay(1);

//   int i = 1;
//   for (i=1; i<121; i++){
//     writePattern1DataToDisplayLedNo(slaveNo, i);
//     delay(1000);
//   }
// }

int getSSPin(int slaveNo){
  int ss;
  switch (slaveNo) {
  case 1: ss = SS1; break;
  case 2: ss = SS2; break;
  case 3: ss = SS3; break;
  }
  return ss;
}

int getClkInPin(int slaveNo){
  int CLKIN;
  switch (slaveNo) {
  case 1: CLKIN = CLKIN1; break;
  case 2: CLKIN = CLKIN2; break;
  case 3: CLKIN = CLKIN3; break;
  }
  return CLKIN;
}
