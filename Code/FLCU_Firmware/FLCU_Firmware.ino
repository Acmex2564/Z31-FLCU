
//Libaries
#include <mcp_can.h>

//Pin assignments
#define MAIN_IN A7
#define MAIN_OUT 5

#define SUB_IN A0
#define SUB_OUT 6

#define LL_IN A1
#define LL_OUT 3

#define AUX_OUT 2

#define MODULE_VOLTAGE A2

#define LED 13

#define CAN_INT 7
#define CAN_CS 8
#define CANT_CS 9

#define HIST_ARRAY_SIZE 5

#define NUM_MAIN 20
#define NUM_MAIN_OUT 21
#define NUM_SUB 10

#define SERIAL_SPEED 9600




unsigned long timeCurrent = 0;
const unsigned int intervalReadLevels = 200;
unsigned long timeLastRead = 0;
const unsigned int intervalSerialStatus = 2000;
unsigned long timeLastStatus = 0;
const unsigned int intervalBlink = 1000;
unsigned long timeLastBlink;

const unsigned int intervalCan = 1000;
unsigned long timeLastCan = 0;

MCP_CAN CAN0(CAN_CS);
const unsigned long int idPacketLevelOut = 0x720;
const uint8_t canTxLen = 6;
uint8_t canTxBuf[8];
byte canSendStat; 

//Calibration Constants

// Module Voltage
// 150k and 47k
const float divVolts = (150.0+47.0) / 47.0 ;

// Fuel level senders are configured as voltage dividers
// Here we have two, configured as "Main" and "Sub"
// Variables "t_" represent the resistor value on the top side of the divider
// Variables "b_Min" and "b_Max" represent the range of expected resistance values in the tank
// All resistance values are times 100 to keep the math as integers
int tMain = 110;
int tSub = 235;

int rMain[NUM_MAIN] = {
  80, 72,
  66, 61,
  56, 51,
  45, 42, 
  38, 34,
  31, 29, 
  26, 23,
  19, 14,
  10, 5,
  3, 1
};


int rSub[NUM_SUB] = {
  3, 6,
  12, 19,
  30, 49,
  96, 127,
  273, 275
};

int oMain[NUM_MAIN_OUT] = {
  13, 15, 
  17, 22,
  28, 34, 
  43, 52,
  62, 69,
  80, 96,
  112, 122, 
  139, 157,
  170, 188, 
  211, 230, 
  246
};

int oSub[NUM_SUB]= {
  13, 21,
  29, 47,
  107, 180,
  232, 237,
  243, 246
};

int levelPrevPos = 0;

void setup() {
  // put your setup code here, to run once:

pinMode(MAIN_IN, INPUT);
pinMode(SUB_IN, INPUT);
pinMode(LL_IN, INPUT);
pinMode(MODULE_VOLTAGE, INPUT);
pinMode(CAN_INT, INPUT);

pinMode(MAIN_OUT, OUTPUT);
pinMode(SUB_OUT, OUTPUT);
pinMode(LL_OUT, OUTPUT);
pinMode(AUX_OUT, OUTPUT);
pinMode(LED, OUTPUT);
pinMode(CAN_CS, OUTPUT);
pinMode(CANT_CS, OUTPUT);

Serial.begin(SERIAL_SPEED);

//TCCR0B = TCCR0B & B11111000 | B00000010; // for PWM frequency of 7812.50 Hz
TCCR2B = TCCR2B & B11111000 | B00000010; // for PWM frequency of 3921.16 Hz
  writeLevel(NUM_MAIN_OUT, oMain, readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN)), MAIN_OUT );
  writeLevel(NUM_SUB, oSub, readLevel(NUM_SUB, rSub, true, tSub, analogRead(SUB_IN)), SUB_OUT );

//  digitalWrite(CAN_RESET, HIGH);
  //delay(50);
  digitalWrite(CANT_CS, LOW);
  int canstat = CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  if (canstat == CAN_OK)
  {
    Serial.println("MCP2515 Initialized Successfully!");
  }
  else
  {
    Serial.print("Error Initializing MCP2515..."); Serial.println(canstat);
  }
  // can controller into normal mode
  CAN0.setMode(MCP_NORMAL);



}

void loop() {
  // put your main code here, to run repeatedly:

  timeCurrent = millis();

  if (timeCurrent - timeLastRead > intervalReadLevels)
  {

  digitalWrite(LED, HIGH);
  writeLevel(NUM_MAIN_OUT, oMain, readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN)), MAIN_OUT );
  
  
  writeLevel(NUM_SUB, oSub, readLevel(NUM_SUB, rSub, true, tSub, analogRead(SUB_IN)), SUB_OUT );
  timeLastRead = timeCurrent;
  digitalWrite(LED, LOW);
  }

  if (timeCurrent - timeLastStatus > intervalSerialStatus)
  {
      
    Serial.println("Heartbeat");
    Serial.print("Module Volts:"); Serial.println((analogRead(MODULE_VOLTAGE) * 5.0 / 1023.0) * divVolts);
    Serial.print("Supply Volts:"); Serial.println(readVcc());
    Serial.print(" Main Level (Gal): "); Serial.println(readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN)));
    Serial.print(" Main Level (PWM): "); Serial.println(writeLevel(NUM_MAIN_OUT, oMain, readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN)), MAIN_OUT ));
    Serial.print(" Sub Level (Gal): "); Serial.println(readLevel(NUM_SUB, rSub, true, tSub, analogRead(SUB_IN)));
    Serial.print(" Sub Level (PWM): "); Serial.println(writeLevel(NUM_SUB, oSub, readLevel(NUM_SUB, rSub, true, tSub, analogRead(SUB_IN)), SUB_OUT ));
    timeLastStatus = timeCurrent;
  }
  

  if (timeCurrent - timeLastCan > intervalCan)
  {
    canTxBuf[0] = readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN)) * 10;
    canTxBuf[1] = readLevel(NUM_SUB, rSub, true, tSub, analogRead(SUB_IN)) * 10;
    canTxBuf[2] = 0xFF;
    canTxBuf[3] = 0xFF;
    canTxBuf[4] = 0xFF;
    canTxBuf[5] = 0xFF;
    canSendStat = CAN0.sendMsgBuf(idPacketLevelOut, 0, canTxLen, canTxBuf);
    if (canSendStat == CAN_OK){
      Serial.println("CanOK");
    }
    else {
      Serial.print("CanErr: "); Serial.println(canSendStat);
    }
    timeLastCan = timeCurrent;
    
  }

  
  

}

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}

int writeLevel(int numElem, int * table, float level, int pinOut) {
  int index;
  int result;

  index = constrain(floor(level), 0, numElem-1);

  level = floor(level*100.0);
  level = (int)level;
  //Serial.print("  Index: "); Serial.println(index);
  
  
  
  result = map(level, index*100, (index+1)*100, table[index], table[index+1] );
  analogWrite(pinOut, result);
  return result;
  //Serial.print("  PWM: "); Serial.println(result);
};

float readLevel(int numElem, int * table, bool reverse, int rTop, int reading) {
  float reading2;
  float result;
  int index;

  //convert adc reading to resistance
  reading2 = constrain(reading, 1, 1023);
  reading2 = rTop / (((1024.0)/(reading))-1);
  //Serial.print("  Reading (Ohm): "); Serial.println(reading2);
  
  //cycle through lookup table until value is greater than (normal) or less than (reverse)the current table value
  for (index = 0; index <= numElem - 1; index++) 
  {
    //Serial.println(index);
    if ((!reverse && reading2 >= table[index]) || ( reverse && reading2 <= table[index])) 
    {
      break;
    }   
  }
  
  //linear interpolation between last and next value
  if (index < numElem-1) 
  {
    result = float(index-1) + ((reading2 - float(table[index-1])) * ((float(index) - float(index-1))/float(table[index] - table[index-1])));
  }
  else
  {
    result = index;
  }
  //Serial.print("  Level   (Gal): "); Serial.println(result);

  return result;
}
