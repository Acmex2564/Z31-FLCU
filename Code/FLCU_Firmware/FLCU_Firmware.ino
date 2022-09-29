
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
#define NUM_SUB 10

#define SERIAL_SPEED 9600


unsigned long timeCurrent = 0;
const unsigned int intervalReadLevels = 200;
unsigned long timeLastRead = 0;
const unsigned int intervalSerialStatus = 2000;
unsigned long timeLastStatus = 0;
const unsigned int intervalBlink = 1000;
unsigned long timeLastBlink;


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


}

void loop() {
  // put your main code here, to run repeatedly:

  digitalWrite(LED, HIGH);
  Serial.println("Heartbeat");
  Serial.print("Module Volts:"); Serial.println((analogRead(MODULE_VOLTAGE) * 5.0 / 1023.0) * divVolts);
  Serial.print("Supply Volts:"); Serial.println(readVcc());
  Serial.print("Main Level:"); Serial.println(analogRead(MAIN_IN));
  //readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN));
  analogWrite(MAIN_OUT, constrain(map(readLevel(NUM_MAIN, rMain, false, tMain, analogRead(MAIN_IN))*100, 0, 2000, 13, 246), 13, 246));
  Serial.print("Sub Level:"); Serial.println(analogRead(SUB_IN));
  //readLevel(NUM_SUB, rSub, true, tSub, analogRead(SUB_IN));
  analogWrite(SUB_OUT, constrain(map(readLevel(NUM_MAIN, rSub, true, tSub, analogRead(SUB_IN))*100, 0, 7, 25, 245), 25, 245));
  //Serial.print("Low Level:"); Serial.println(analogRead(LL_IN));
  delay(100);
  digitalWrite(LED, LOW);
  delay(1000);
  

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

float readLevel(int numElem, int * table, bool reverse, int rTop, int reading) {
  float reading2;
  float result;
  int index;

  //convert adc reading to resistance
  reading2 = constrain(reading, 1, 1023);
  reading2 = rTop / (((1024.0)/(reading))-1);
  Serial.print("  Reading (Ohm): "); Serial.println(reading2);
  
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
    //Serial.println("Interp debug");
    //Serial.println(table[index-1]);
    //Serial.println(table[index]);
    result = float(index-1) + ((reading2 - float(table[index-1])) * ((float(index) - float(index-1))/float(table[index] - table[index-1])));
  }
  else
  {
    result = index;
  }
  Serial.print("  Level   (Gal): "); Serial.println(result);

  return result;
}
