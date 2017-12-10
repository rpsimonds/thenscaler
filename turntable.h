#define NUM_POSITIONS 16
typedef struct ttp {
  unsigned int steps;
  byte polarity;
};

ttp positions[NUM_POSITIONS] = {
  {1797, REVERSE},
  {1967, REVERSE},
  {1973, REVERSE},
  {2122, REVERSE},
  {2264, REVERSE},
  {2417, REVERSE},
  {2672, REVERSE},
  {2916, REVERSE},
  {5804, NORMAL},
  {5961, NORMAL},
  {5973, NORMAL},
  {6122, NORMAL},
  {6279, NORMAL},
  {6429, NORMAL},
  {6672, NORMAL},
  {6925, NORMAL}
};

class turntable{
  private:
    // Easy Driver Pins
    byte stp;
    byte dir;
    byte ms1;
    byte ms2;
    byte en;

    byte beacon;

    // reed switches
    byte sw1;
    byte sw2;

    polarity_manager *reverser;
    byte current_direction;
    byte current_position;
    unsigned int current_step;
    unsigned int destination;
    unsigned long last_update;
    int step_delay;
    bool in_motion;
    
  public:
    turntable(byte p_stp, byte p_dir, byte p_ms1, byte p_ms2, byte p_en, byte p_beacon, byte p_sw1, byte p_sw2, byte p_norm, byte p_rev, int s_delay){
      stp = p_stp;
      dir = p_dir;
      ms1 = p_ms1;
      ms2 = p_ms2;
      en = p_en;
      beacon = p_beacon;
      sw1 = p_sw1;
      sw2 = p_sw2;
      step_delay = s_delay;
      pinMode(stp, OUTPUT);
      pinMode(dir, OUTPUT);
      pinMode(ms1, OUTPUT);
      pinMode(ms2, OUTPUT);
      pinMode(en, OUTPUT);
      pinMode(beacon, OUTPUT);
      digitalWrite(beacon, LOW);
      setEDPins();
      reverser = new polarity_manager(RELAY_BISTABLE, p_norm, p_rev);
      GoToPosition(0);
    }

    void GoToPosition(int id){
      struct ttp *pdata;
      digitalWrite(ROTATING_BEACON, HIGH);
      
      if(id == 0) {
        FindAnchor();
      } else {
        if(id > 0 && id <= NUM_POSITIONS && id != current_position) {
          pdata = &positions[id - 1];
          in_motion = true;
          destination = pdata->steps;
          if(destination < current_step) { // counterclockwise to destination
            digitalWrite(dir, COUNTERCLOCKWISE);
            if(current_direction != COUNTERCLOCKWISE){
              current_direction = COUNTERCLOCKWISE;
              current_step += SLACK; // compensate for change in direction
            }
          } else { // clockwise to destination
            digitalWrite(dir, CLOCKWISE);
            if(current_direction != CLOCKWISE){
              current_direction = CLOCKWISE;
              current_step -= SLACK; // compensate for change in direction
            } 
          }
          pdata->polarity == NORMAL ? reverser->setNormal(): reverser->setReverse();
        }
      }
    }

    void setEDPins(){
      digitalWrite(stp, LOW);
      digitalWrite(dir, CLOCKWISE);
      digitalWrite(ms1, HIGH); //Pull ms1, and ms2 high to set logic to 1/8th microstep resolution
      digitalWrite(ms2, HIGH);
      digitalWrite(en, LOW); // Enables motor control
    }
    
    void doSteps(int steps, int step_delay){
      for(int i = 0; i < steps; i++){
        doStep();
        delay(step_delay);
      }
    }
    
    void doStep(){
      digitalWrite(stp,HIGH); //Trigger one step
      delay(1); // wait
      digitalWrite(stp,LOW); //Reset trigger
    }
  
    void FindAnchor(){
      digitalWrite(dir, CLOCKWISE);
      // if sensors currently tripped, rotate until both open
      while(analogRead(sw1) > 1000 || analogRead(sw2) > 1000){
        doStep();
        delay(step_delay * 2);
      }
      // additional steps to clear sensors
      doSteps(100, step_delay);
      // find the sensors again
      digitalWrite(dir, COUNTERCLOCKWISE);
      while(analogRead(sw1) < 1000){
        doStep();
        delay(step_delay);
      }
      while(analogRead(sw2) < 1000){
        doStep();
        delay(step_delay * 2);
      }
      current_direction = COUNTERCLOCKWISE;
      current_position = 0;
      current_step = 0;
      digitalWrite(ROTATING_BEACON, LOW);
      in_motion = false;
    }
    
    void update(unsigned long current_millis){
      if(in_motion){
        if(current_millis - last_update >= step_delay){
          last_update = current_millis;
          doStep();
          current_direction == CLOCKWISE ? current_step++: current_step--;
          in_motion = current_step == destination ? false: true;
          if(!in_motion){
            digitalWrite(ROTATING_BEACON, LOW);
            setEDPins();
          }
        }
      }
    }
};

