///////////////////////////////////////
// CLASS fire
// A demonstration C++ class 
// for simulating a fire with LEDS
//
// Author: Robin Simonds, theNscaler.com
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
///////////////////////////////////////

class fire {
  private:
    int ledPin;
    int high;  // maximum brightness 
    int low;   // minimum brightness 
    int rate;  // update frequency in milliseconds 
    bool dirUp; // true if brightness is increasing 
    int state;
    unsigned long lastUpdate; // in milliseconds; can be a big number

  public: 
    fire(int pin, int zhigh, int zlow, int zrate){
      ledPin = pin;
      high = zhigh;
      low = zlow;
      rate = zrate;

      lastUpdate = 0;
      // seed the psuedo random number generator by
      // reading an unconnected analog pin
      randomSeed(analogRead(0));
      // randomize the starting state of the object
      state = random(low + 1, high);
      dirUp = random(2) == 1;
      analogWrite(ledPin, state);
    }
    // call the Update method frequently to run the animation
    void Update(unsigned long curMillis) {
      if(curMillis - lastUpdate >= rate){
        lastUpdate = curMillis;
        dirUp ? state++ : state--;
        if(random(80) == 1){ // a possible spark
          analogWrite(ledPin, 255);
        } else {
          analogWrite(ledPin, state);
        }
        if(state == high || state == low){
          dirUp = !dirUp; // boolean logic flip
        }
      }
    } 
};

// create global instances of the fire class
fire demo_fire1 = fire(6, 80, 20, 30);
fire demo_fire2 = fire(5, 75, 15, 35);

void setup() {
  // no setup currently needed
}

void loop() {
  unsigned long current_millis = millis();
  
  demo_fire1.Update(current_millis);
  demo_fire2.Update(current_millis);
}
