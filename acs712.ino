///////////////////////////////////////////////////////////////// 
//
// acs712.ino -- ASC712 Sensor
// Current Sensing / Block Occupancy Detection
// @copyright 2017 Robin Simonds
// This work is licensed under a Creative Commons 
// Attribution-NonCommercial-ShareAlike 4.0 International License.
// 
//////////////////////////////////////////////////////////////////
#define VERSION "1.001.006"
#define SYS_ID "ACS712 Sensor Test"
const int pin = 2;

// Sampling Parameters
const unsigned long sampleTime = 58000UL; // 58 ms
const unsigned long numSamples = 300UL; 
// sample interval is in microseconds
// must be greater than 100μs, the conversion time of the internal ADC
const unsigned long sampleInterval = sampleTime/numSamples;

#define SENSITIVITY 185 // per ACS712 5A data sheet, in mv/A
#define DETECTION_MULTIPLIER 1.095 // change as necessary to improve detection accuracy
#define CALIBRATION_READS 5000

// variables to hold sensor quiescent readings
int aqv;  
float aqc;

void setup()
{
  float sense;
  Serial.begin(9600);
  Serial.println(String(F(SYS_ID)) + String(F(" - SW:")) + String(F(VERSION)));
  Serial.print("\n\nCalibrating the sensor at pin ");
  Serial.println(pin);
  aqv = determineVQ(pin);
  Serial.print("AQV: ");
  Serial.print(aqv, 4);
  Serial.println(" mV");
  aqc = determineCQ(pin, aqv);
  Serial.print("AQC: ");
  Serial.print(aqc, 4);
  Serial.println(" mA");
  sense = (aqc * DETECTION_MULTIPLIER) - aqc;
  Serial.print("Detection Threshold: ");
  Serial.print(sense * 1000, 4);
  Serial.println(" mA\n\n");
  delay(5000);
}

void loop(){
  float current = readCurrent(pin, aqv);
  bool occupied = current > (aqc * DETECTION_MULTIPLIER);
  
  Serial.print("Current Sensed:");
  Serial.print(current * 1000,1);
  Serial.print(" mA\t\t");
  Serial.print("The block is ");
  if(occupied){
    Serial.println("occupied");
  } else {
    Serial.println("not occupied");
  }
  delay(1000);
}

//////////////////////////////////////////
// ACS712 Current Sensor Functions
//////////////////////////////////////////
float readCurrent(int PIN, float adc_zero)
{
  float currentAcc = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval ;
  while (count < numSamples)
  {
    if (micros() - prevMicros >= sampleInterval)
    {
      float adc_raw = (float) analogRead(PIN) - adc_zero;
      adc_raw /= SENSITIVITY; // convert to amperes
      currentAcc += (adc_raw * adc_raw);
      ++count;
      prevMicros += sampleInterval;
    }
  }
  //https://en.wikipedia.org/wiki/Root_mean_square
  float rms = sqrt((float)currentAcc / (float)numSamples);
  return rms;
}

//////////////////////////////////////////
// Calibration
// Track Power must be OFF during calibration
//////////////////////////////////////////

int determineVQ(int PIN) {
  float VQ = 0;
  //read a large number of samples to stabilize value
  for (int i = 0; i < CALIBRATION_READS; i++) {
    VQ += analogRead(PIN);
    delayMicroseconds(sampleInterval);
  }
  VQ /= CALIBRATION_READS;
  return int(VQ);
}

float determineCQ(int pin, float aqv) {
  float CQ = 0;
  // set reps so the total actual analog reads == CALIBRATION_READS
  int reps = (CALIBRATION_READS / numSamples);
  for (int i = 0; i < reps; i++) {
    CQ += readCurrent(pin, aqv);
  }
  CQ /= reps;
  return CQ;
}

