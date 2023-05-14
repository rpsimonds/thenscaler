#include <Servo.h>

// constant variables used to set servo angles, in degrees
const int straight = 90; 
const int divergent = 110;

// constant variables holding the ids of the pins we are using
const int buttonpin = 8;
const int servopin = 9;

// servo movement step delay, in milliseconds
const int step_delay = 70;

// create a servo object
Servo myservo;  
 
// global variables to store servo position
int pos = straight; // current
int old_pos = pos; // previous

void setup() 
{
  Serial.begin(115200); 
  // set the mode for the digital pins in use
  pinMode(buttonpin, INPUT);
   
  // setup the servo
  myservo.attach(servopin);  // attach to the servo on pin 9
  myservo.write(pos); // set the initial servo position
  Serial.println("Starting Pos: " + String(pos));
}

void loop() 
{ 
 // start each iteration of the loop by reading the button
 // if the button is pressed (reads HIGH), move the servo
  int button_state = digitalRead(buttonpin);
  if(button_state == HIGH){
    
    old_pos = pos;   // save the current position
    
    // Toggle the position to the opposite value
    pos = pos == straight ? divergent: straight;
       
    // Move the servo to its new position
    if(old_pos < pos){   // if the new angle is higher
      // increment the servo position from oldpos to pos
      for(int i = old_pos + 1; i <= pos; i++){  
        myservo.write(i); // write the next position to the servo
        Serial.println(i);
        delay(step_delay); // wait
      }
    } else {  // otherwise the new angle is equal or lower
      // decrement the servo position from oldpos to pos
      for(int i = old_pos - 1; i >= pos; i--){ 
        myservo.write(i); // write the next position to the servo
        Serial.println(i);
        delay(step_delay); // wait
      }
    }
  } 
}// end of loop
