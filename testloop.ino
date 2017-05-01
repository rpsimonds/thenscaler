// System Identifiers
#define VERSION "1.008.029"
#define SYS_ID "TL1"
// Includes
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>
#include "defines.h"
#include "standard_types.h"
#include <Servo.h>

// Preprocessor Directives
#define INCLUDE_SERIAL

// Duino node definitions and data
nodeState ndata0[] = {0,0,0,0};
nodeGroup nodeGroups[] = {{7, 6, 5, 4, ndata0}};

// an array of pointers to servo objects
Servo *servos[NUM_TURNOUTS];

// A EthernetUDP instance 
EthernetUDP Udp;
String delimiter = "/"; // Packet delimiter character
#define ACK "255/"

// constants for block sensor functions
// sample over 58 ms; 100 times the nominal pulse width for binary 1 in DCC
const unsigned long sampleTime = 58000UL; 
const unsigned long numSamples = 200UL; // choose the number of samples to divide sampleTime exactly, but low enough for the ADC to keep up
const unsigned long sampleInterval = sampleTime / numSamples; // the sampling interval-- here 290µs -- must be longer than then ADC conversion time
// ADC conversion time is about 50µs (+/- without specifically allowing for jitter), so longer sample intervals are better

// data arrays
// Block data
BLOCK_DEF blocks[NUM_BLOCKS] = {
    {0, 0, 0, false, false},
    {1, 0, 0, false, false},
    {2, 0, 0, false, false},
    {3, 0, 0, false, false}}; 
// Master track current sensor    
BLOCK_DEF master = {4, 0, 0, false, false}; 

// Signals 
T_ALIGN tdata0A[] = {0, ALIGN_DIVERGENT};
T_ALIGN tdata0B[] = {0, ALIGN_MAIN};
T_ALIGN tdatanull[] = {};
byte bdatanull[] = {};
byte bdata0[] = {0};
byte bdata2[] = {2};
byte bdata3[] = {3};
SIGNAL_DEF signals[NUM_SIGNALS] = {
  {3, {3, 0}, -1, 0, 1, -1, SIGNAL_OFF, tdata0A, 1, bdata2, 1, bdatanull, 0},
  {3, {3, 0}, -1, 2, 3, -1, SIGNAL_OFF, tdata0B, 1, bdata2, 1, bdatanull, 0},
  {7, {1, 0}, 1, 0, 1, 2, SIGNAL_OFF, tdatanull, 0, bdata0, 1, bdatanull, 0},
  {7, {0, 0}, -1, 0, 1, 2, SIGNAL_OFF, tdata0A, 1, bdatanull, 0, bdata3, 1},
  {3, {0, 0}, -1, 3, 4, -1, SIGNAL_OFF, tdata0B, 1, bdata2, 1, bdatanull, 0}
};

// turnouts
TURNOUT_DATA turnout[NUM_TURNOUTS] = {
  {{8, 93, 117}, false, ALIGN_MAIN, 94, 94, 0}
};

// data subscribers
SUBSCRIBER block_subs[MAX_SUBSCRIPTIONS];
SUBSCRIBER turnout_subs[MAX_SUBSCRIPTIONS];
int num_block_subs = 0;
int num_turnout_subs = 0;

void setup() {
  int i;
  // Initialize Duino Nodes
  for(int g = 0; g < NODE_GROUPS; g++){
    pinMode(nodeGroups[g].latchPin, OUTPUT);
    pinMode(nodeGroups[g].clockPin, OUTPUT);
    pinMode(nodeGroups[g].dataPin, OUTPUT);
  }
  nodeSet({2, 0}, 31);
  nodeRefresh();
  
  // start Ethernet and UDP:
  IPMAC ipm = readIPMAC(); 
  Ethernet.begin(ipm.mac, ipm.ip);
  Udp.begin(UDP_PORT);
  
  #ifdef INCLUDE_SERIAL
  String dot = String(F("."));
  Serial.begin(9600);
  Serial.println(String(F(SYS_ID)) + String(F(" - SW:")) + String(F(VERSION)));
  Serial.println(String(ipm.ip[0]) + dot + String(ipm.ip[1]) + dot + String(ipm.ip[2]) + dot +  String(ipm.ip[3]));
  #endif
  
  // initialize turnouts
  for(i = 0; i < NUM_TURNOUTS; i++){
    // create a SERVO instance and store it in the servos array
    servos[i] = new Servo; 
    servos[i]->attach(turnout[i].data.pin);
    // test by setting both positions in turn ending at MAIN
    servos[i]->write(turnout[i].data.pos_main);
    delay(500);
    servos[i]->write(turnout[i].data.pos_div);
    delay(500);
    setTurnout(i, ALIGN_MAIN);  // set default turnout state
  }
  
  // initialize and calibrate block detection system
  #ifdef INCLUDE_SERIAL
  Serial.println(F("Calibrating Master Sensor: "));
  #endif
  // signal state changes are for monitoring progress of calibration
  for(i = 0; i < NUM_SIGNALS; i++){
    if(i % 2 == 0) {
      setSignal(i, SIGNAL_RED);
    } else {
      setSignal(i, SIGNAL_GREEN);
    }
  }
  master.aqv = determineVQ(master.pin);
  for(i = 0; i < NUM_SIGNALS; i++){
    if(i % 2 != 0) {
      setSignal(i, SIGNAL_RED);
    } else {
      setSignal(i, SIGNAL_GREEN);
    }
  }
  master.aqc = determineCQ(master.pin, master.aqv);
  master.occ = false;
  for(i = 0; i < NUM_SIGNALS; i++){
    setSignal(i, SIGNAL_YELLOW);
  }
  for (i = 0; i < NUM_BLOCKS; i++) { // for each block
    #ifdef INCLUDE_SERIAL
    Serial.print(F("Calibrating Block Sensor "));
    Serial.print(i);
    Serial.println(F(": "));
    #endif
    
    setSignal(0, SIGNAL_RED);
    setSignal(1, SIGNAL_GREEN);
    blocks[i].aqv = determineVQ(blocks[i].pin);
    setSignal(0, SIGNAL_GREEN);
    setSignal(1, SIGNAL_RED);
    blocks[i].aqc = determineCQ(blocks[i].pin, blocks[i].aqv);
    blocks[i].occ = false;
  }
  
  // last signal state changes indicate successful setup
  for(i = 0; i < NUM_SIGNALS; i++){
    setSignal(i, SIGNAL_RED);
  }
  delay(1000);
  for(i = 0; i < NUM_SIGNALS; i++){
    setSignal(i, SIGNAL_GREEN);
  }
  delay(1000);
  for(i = 0; i < NUM_SIGNALS; i++){
    setSignal(i, SIGNAL_RED);
  }
  delay(1000);
  announce(F("1,2")); // broadcast data availability
  nodeSet({2, 0}, 255);
  #ifdef INCLUDE_SERIAL
  Serial.println("\nSetup Complete");
  #endif
}

void loop()
{
  bool power_state, block_state;
  // get elapsed milliseconds at loop start
  unsigned long currentMillis = millis();

  ////////////////////////////////////////////////
  // Timed actions or animations in progress
  ///////////////////////////////////////////////
  // Turnout Control
  for(int i = 0; i < NUM_TURNOUTS; i++){
    if (turnout[i].is_moving) {
      if ( (currentMillis - turnout[i].last_move) >= STEP_DELAY ) {
        turnout[i].last_move = currentMillis;
        if (turnout[i].pos_now < turnout[i].target_pos) { // if the new angle is higher
          servos[i]->write(++turnout[i].pos_now);
        } else {  // otherwise the new angle is equal or lower
          if (turnout[i].pos_now != turnout[i].target_pos) { // not already at destination
            servos[i]->write(--turnout[i].pos_now);
          }
        }
      }
      if (turnout[i].pos_now == turnout[i].target_pos) {
        turnout[i].is_moving = false;
        turnout[i].last_move = 0;
        notifySubscribers(2, String(i), String(turnout[i].alignment));
      }
    }
  }
  
  ////////////////////////////////////////////////
  // Block occupancy detection
  ////////////////////////////////////////////////
  power_state = isOccupied(master.pin, master.aqv, master.aqc * MASTER_DETECTION_MULTIPLIER);
  if(power_state != master.occ) {
    if(master.chg_pending) {
      master.occ = power_state;
      master.chg_pending = false;
      #ifdef INCLUDE_SERIAL
      Serial.print(F("Track Power is "));
      power_state ? Serial.println(F("on")): Serial.println(F("off"));
      #endif
    } else {
      master.chg_pending = true;
    }
  } else {
    master.chg_pending = false;
  }
  if(master.occ){
    for (int i = 0; i < NUM_BLOCKS; i++) {
      block_state = isOccupied(blocks[i].pin, blocks[i].aqv, blocks[i].aqc * DETECTION_MULTIPLIER);
      if (block_state != blocks[i].occ) { // if occupancy state has changed
        if(blocks[i].chg_pending && !master.chg_pending) { // if a pending change has been previously detected
          blocks[i].occ = block_state;
          blocks[i].chg_pending = false; 
          #ifdef INCLUDE_SERIAL
          Serial.print(F("Block "));
          Serial.print(i);
          Serial.print(F(" is "));
          block_state ? Serial.println(F(" occupied")): Serial.println(F(" unoccupied"));
          #endif
          String id = String(i);
          notifySubscribers(1, id, String(block_state));
        } else { // initial change detection
          blocks[i].chg_pending = true; 
        }     
      } else {
        blocks[i].chg_pending = false; 
      }
    }
  }

  // Now refresh the signals, as affected by the state
  // of automatic layout processes
  
  refreshSignals();
  
  ////////////////////////////////////////////////
  // Communications
  ///////////////////////////////////////////////
  // check for new message
  PKT_DEF pkt = pollNet();

  // process the message
  switch (pkt.function.toInt()) {
    case 1:  // function 1, turnout control
      sendMessage(pkt.from, pkt.port, ACK + String(SYS_ID));
      setTurnout(pkt.option.toInt(), pkt.data.toInt());
      break;
    case 99:  // function 99, subscribe to notifications
      sendMessage(pkt.from, pkt.port, ACK + String(SYS_ID));
      registerSubscription( pkt.data, pkt.option, Udp.remoteIP(),  Udp.remotePort());
      break;
  } 
  
} // end main loop

////////////////////////////////////////////////////////////////
// Utility Functions
////////////////////////////////////////////////////////////////

void setTurnout(int id, int align){
   switch(align){
        case ALIGN_MAIN:
          turnout[id].is_moving = true;
          turnout[id].last_move = 0;
          turnout[id].target_pos = turnout[id].data.pos_main;
          turnout[id].alignment = ALIGN_MAIN;
          break;
        case ALIGN_DIVERGENT:
          turnout[id].is_moving = true;
          turnout[id].last_move = 0;
          turnout[id].target_pos = turnout[id].data.pos_div;
          turnout[id].alignment = ALIGN_DIVERGENT;
          break;
      }
      notifySubscribers(2, String(id), String(0));
}

bool isOccupied(int block, float aqv, float threshold) {
  float current = readCurrent(block, aqv);
  return (current > threshold);
}


