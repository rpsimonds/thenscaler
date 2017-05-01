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
      float adc_raw = (float) analogRead(PIN) - adc_zero; // Electical offset voltage
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
  #ifdef INCLUDE_SERIAL
  Serial.print("Calculating avg. quiescent voltage:");
  #endif
  float VQ = 0;
  //read a large number of samples to stabilize value
  for (int i = 0; i < CALIBRATION_READS; i++) {
    VQ += analogRead(PIN);
    delayMicroseconds(200);
  }
  VQ /= CALIBRATION_READS;
  #ifdef INCLUDE_SERIAL
  Serial.print(map(VQ, 0, 1023, 0, 5000));
  Serial.println(" mV");
  #endif
  return int(VQ);
}

float determineCQ(int pin, float aqv) {
  float CQ = 0;
  #ifdef INCLUDE_SERIAL
  Serial.print("Calculating avg. quiescent current:");
  #endif
  // set reps so the total actual analog reads == CALIBRAION_READS
  int reps = (CALIBRATION_READS / numSamples);
  for (int i = 0; i < reps; i++) {
    CQ += readCurrent(pin, aqv);
  }
  CQ /= reps;
  #ifdef INCLUDE_SERIAL
  Serial.print(CQ * 1000, 4);
  Serial.println(" mA");
  #endif
  return CQ;
}

