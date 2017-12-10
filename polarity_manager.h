#define RELAY_BISTABLE 1
#define RELAY_REVERTS 2
#define POLARITY_NORMAL 1
#define POLARITY_REVERSE 0
#define TRIGGER_PERIOD 50

class polarity_manager
{
  // Properties
  private:
  int relayType;
  byte pinNormal;
  byte pinReverse;
  int state;
  public:
  // Constructor
  polarity_manager(int rtype, byte pnormal, byte preverse){
    relayType = rtype;
    pinNormal = pnormal;
    pinReverse = preverse;
    
    if(relayType == RELAY_BISTABLE && pnormal >= 0){
      pinMode(pinNormal, OUTPUT);
    }
    pinMode(pinReverse, OUTPUT);
    digitalWrite(pinReverse, LOW);
    setNormal();
  }
  
  int getPolarity(){
    return state;
  }
  void setNormal(){
    switch(relayType){
      case RELAY_BISTABLE:
        digitalWrite(pinNormal, HIGH);
        delay(TRIGGER_PERIOD);
        digitalWrite(pinNormal, LOW);
        break;
      case RELAY_REVERTS:
        digitalWrite(pinReverse, LOW);
        break;
    }
    state = POLARITY_NORMAL;
  }
  void setReverse(){
     switch(relayType){
      case RELAY_BISTABLE:
        digitalWrite(pinReverse, HIGH);
        delay(TRIGGER_PERIOD);
        digitalWrite(pinReverse, LOW);
        break;
      case RELAY_REVERTS:
        digitalWrite(pinReverse, HIGH);
        break;
    }
    state = POLARITY_REVERSE;
  }
};

