///////////////////////////////////////////////////////////////// 
//
// CT_Sensor.ino
//
// Current Sensing / Block Occupancy Detection
// using current transformers and the Mayhew Labs
// Extended ADC Shield
// Example code with sample block data
// @copyright 2017 Robin Simonds
// This work is licensed under a Creative Commons 
// Attribution-NonCommercial-ShareAlike 4.0 International License.
// 
//////////////////////////////////////////////////////////////////
#include <ExtendedADCShield.h>
#include <SPI.h>

#define pinA 7
#define pinB 6
#define pinC 5

#define NUM_BLOCKS 24
#define SENSITIVITY 3.3
#define DETECTION_THRESHOLD .0006
#define CALIBRATION_READS 300
//////////////////////////
// Blocks
//////////////////////////
typedef struct BLOCK_DEF {
  int bank;
  int pin;
  float aqv;
  float aqc;
  bool occ;
  bool changing;
};
// Example Block data  
BLOCK_DEF blocks[NUM_BLOCKS] = {
    {0, 0, 0, 0, false, false},
    {0, 4, 0, 0, false, false},
    {1, 4, 0, 0, false, false},
    {2, 4, 0, 0, false, false},
    {3, 4, 0, 0, false, false},
    {4, 4, 0, 0, false, false},
    {5, 4, 0, 0, false, false},
    {6, 4, 0, 0, false, false},
    {1, 0, 0, 0, false, false},
    {2, 0, 0, 0, false, false},
    {3, 0, 0, 0, false, false},
    {0, 2, 0, 0, false, false},
    {1, 2, 0, 0, false, false},
    {2, 2, 0, 0, false, false},
    {3, 2, 0, 0, false, false},
    {4, 2, 0, 0, false, false},
    {5, 2, 0, 0, false, false},
    {6, 2, 0, 0, false, false},
    {7, 2, 0, 0, false, false},
    {7, 4, 0, 0, false, false},
    {4, 0, 0, 0, false, false},
    {5, 0, 0, 0, false, false},
    {6, 0, 0, 0, false, false},
    {7, 0, 0, 0, false, false}}; 
int sensor_map[8][3] = {
  {0, 11, 5},
  {8, 12, 6},
  {9, 13, 7},
  {10, 14, 19},
  {20, 15, 1},
  {21, 16, 2},
  {22, 17, 3},
  {23, 18, 4}};
typedef struct adcAddr {
  int A;
  int B;
  int C;
};
adcAddr addrBits[8] = {
  {LOW, LOW, LOW},
  {HIGH, LOW, LOW},
  {LOW, HIGH, LOW},
  {HIGH, HIGH, LOW},
  {LOW, LOW, HIGH},
  {HIGH, LOW, HIGH},
  {LOW, HIGH, HIGH},
  {HIGH, HIGH, HIGH},
};
//Initialize the ADC Shield
ExtendedADCShield extendedADCShield(8, 9, 9, 14);
//////////////////////////////////////////
// Current Sensor Parameters
//////////////////////////////////////////
// sample over 10µs
const unsigned long sampleTime = 500UL; // in microseconds
const unsigned long numSamples = 50UL; 
// the sampling interval-- here 10µs -- must be longer than then ADC conversion time
const unsigned long sampleInterval = sampleTime / numSamples; 

void setup() {
  // initialize Block Occupancy Detection system
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  for (int i = 0; i < NUM_BLOCKS; i++) { // for each block
    blocks[i].aqv = determineVQ(blocks[i].bank, blocks[i].pin);
    blocks[i].aqc = determineCQ(blocks[i].bank, blocks[i].pin, blocks[i].aqv);
    blocks[i].occ = false;
  }

}

void loop() {
  // Read Optimized for Mayhew Labs Shield with 24 sensors
  // Example code is for 8 "rows" of 3 sensors
  // Each row (0 - 7) is a channel for a sensor on
  // each of the 3 sensor boards (8 sensors each)
  
  for(int i = 0; i < 8; i++){
    readSensorRow(i);
  }
}

//////////////////////////////////////////
// Current Sensor Functions
//////////////////////////////////////////

//////////////////////////////////////////
// readSensorRow() -- sequential read optimized
// for Mayhew Labs ADC Shield
//////////////////////////////////////////
void readSensorRow(int row){
  extern BLOCK_DEF blocks[];
  float accumulator[3];
  float raw[3];
  float delta, rms;
  unsigned int count;
  unsigned long curMicros, prevMicros = micros() - sampleInterval;
  
  digitalWrite(pinA, addrBits[row].A);
  digitalWrite(pinB, addrBits[row].B);
  digitalWrite(pinC, addrBits[row].C);
  count = 0;
  for(int j = 0; j < 3;  j++){
    accumulator[j] = 0;
  }
  while(count < numSamples){
    curMicros = micros();
    if (curMicros - prevMicros >= sampleInterval) {
      prevMicros = curMicros;
      // prepare channel 0
      extendedADCShield.analogReadConfigNext(0, DIFFERENTIAL, BIPOLAR, RANGE5V);
      // read/prepare subsequent 3 channel pairs; 0-1, 2-3, 4-5
      raw[0] = extendedADCShield.analogReadConfigNext(2, DIFFERENTIAL, BIPOLAR, RANGE5V) - blocks[sensor_map[row][0]].aqv;
      raw[1] = extendedADCShield.analogReadConfigNext(4, DIFFERENTIAL, BIPOLAR, RANGE5V) - blocks[sensor_map[row][1]].aqv;
      raw[2] = extendedADCShield.analogReadConfigNext(6, DIFFERENTIAL, BIPOLAR, RANGE5V) - blocks[sensor_map[row][2]].aqv;
      for(int j = 0; j < 3; j++){
        raw[j] /= SENSITIVITY; // convert to amperes
      }
      for(int j = 0; j < 3; j++){
        accumulator[j] += (raw[j] * raw[j]); // sum the squares
      }
      ++count;
    }
  }
  for(int j = 0; j < 3; j++){
    //https://en.wikipedia.org/wiki/Root_mean_square
    rms = sqrt((float)accumulator[j] / (float)numSamples);
    delta = abs(blocks[sensor_map[row][j]].aqc - rms);
    blocks[sensor_map[row][j]].occ = delta > DETECTION_THRESHOLD;
  }
  
}
/////////////////////////////////////
// readCurrent() - Standard Sensor Reading Function
/////////////////////////////////////
float readCurrent(int bank, int pin, float adc_zero)
{
  float currentAcc = 0;
  unsigned int count = 0;
  unsigned long prevMicros = micros() - sampleInterval ;
  while (count < numSamples)
  {
    if (micros() - prevMicros >= sampleInterval)
    {
      float adc_raw = adcRead(bank, pin) - adc_zero; // Electical offset voltage
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

/////////////////////////////////////////////////////////////////
// adcRead() - Set up shield and read a sensor
/////////////////////////////////////////////////////////////////
float adcRead(int bank, int pin) {
  int next = 6;
  digitalWrite(pinA, addrBits[bank].A);
  digitalWrite(pinB, addrBits[bank].B);
  digitalWrite(pinC, addrBits[bank].C);
  // setup the pin
  extendedADCShield.analogReadConfigNext(pin, DIFFERENTIAL, BIPOLAR, RANGE5V);
  // read it
  float reading =  extendedADCShield.analogReadConfigNext(next, DIFFERENTIAL, BIPOLAR, RANGE5V);
  return reading;
}


//////////////////////////////////////////
// Calibration
// Track Power must be OFF during calibration
//////////////////////////////////////////

float determineVQ(int bank, int pin) {
  float VQ = 0;
  //read a large number of samples to stabilize value
  for (int i = 0; i < CALIBRATION_READS; i++) {
    VQ += adcRead(bank, pin);
    delayMicroseconds(sampleInterval);
  }
  VQ /= CALIBRATION_READS;
  return VQ;
}


float determineCQ(int bank, int pin, float aqv) {
  float CQ = 0;
  // set reps so the total actual analog reads == CALIBRAION_READS
  int reps = (CALIBRATION_READS / numSamples);
  for (int i = 0; i < reps; i++) {
    CQ += readCurrent(bank, pin, aqv);
  }
  CQ /= reps;
  return CQ;
}


