///////////////////////////////////////
// CLASS turnout
// A device independent C++ class 
// for managing Model Railroad Turnouts.
// Hardware implimentation in setServo()
// private method; modify to implement any
// hardware access method.
//
// Author: Robin Simonds, theNscaler.com
// License: CC BY-SA 4.0
// https://creativecommons.org/licenses/by-sa/4.0/
//
// Class declaration and implementation
///////////////////////////////////////
typedef struct TURNOUT_PARAMS {
  int pin;
  int pos_main;
  int pos_div;
  int align_default;
  int move_delay;
};

class turnout
{
private:
   // object properties
  int pin;
  int pos_main;
  int pos_div;
  int align_default;
  int alignment;
  
  // motion data
  bool is_moving;
  int move_delay;
  int increment;
  int pos_now;
  int target_pos;
  int target_alignment;
  unsigned long last_move;

public:
  
  // Constructor
  turnout(TURNOUT_PARAMS *parameters, int movement_increment = 1){
    pin = parameters->pin;
    pos_main = parameters->pos_main;
    pos_div = parameters->pos_div;
    align_default = parameters->align_default;
    move_delay = parameters->move_delay;
    is_moving = false;
    increment = movement_increment;
    init_servo();
  }
  
  // Public Methods
  int getAlignment(){
    return alignment;
  }
   
  void toggle(){
    if(alignment == ALIGN_MAIN){
      set(ALIGN_DIVERGENT);
    } else {
      set(ALIGN_MAIN);
    }
  }
  
  void set(int align){
    if(align != alignment){
      is_moving = true;
      last_move = 0;
      target_alignment = align;
      alignment = ALIGN_NONE;
      switch(align){
        case ALIGN_MAIN:
          target_pos = pos_main;
          break;
        case ALIGN_DIVERGENT:
          target_pos = pos_div;
          break;
      }
    }
  }

  void update(unsigned long curMillis) {
    if(is_moving && (curMillis - last_move) >= move_delay){
      last_move = curMillis;
      if (pos_now < target_pos) { // if the new position is higher
          pos_now = min(pos_now + increment, target_pos);
          setServo(pos_now);
      } else {  // otherwise the new position is equal or lower
          if (pos_now != target_pos) { // not already at destination
            pos_now = max(pos_now - increment, target_pos);
            setServo(pos_now);
          }
      }
      if (pos_now == target_pos) {
        is_moving = false;
        last_move = 0;
        alignment = target_alignment;
      }
    }
  }
  
  // private methods
private:
  void init_servo(){
    int data;
    switch(align_default){
      case ALIGN_MAIN:
        data = pos_main;
      break;
      case ALIGN_DIVERGENT:
        data = pos_div;
      break;
    }
    setServo(data);
    is_moving = false;
    pos_now = data;
    alignment = align_default;
    
  }
  // hardware interface 
  void setServo(int data){
    // use compiler directives to determine which
    // method is used to drive the actual hardware
    #ifdef ADAF_DRIVER
    extern Adafruit_PWMServoDriver pwm;
    pwm.setPWM(pin, 0, data);
    #endif
    #ifdef SERVO_LIB
    extern servo servos;
    servos[pin]->write(data);
    #endif
  }
}; // end CLASS turnout

