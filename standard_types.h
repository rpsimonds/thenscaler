//////////////////////////
// Networking
//////////////////////////
typedef struct IPMAC{
  byte mac[6];
  byte ip[4];
};

typedef struct SUBSCRIBER {
  String id;
  IPAddress ip;
  int port;
};
typedef struct PKT_DEF {
  String function; 
  String option;
  String data;
  IPAddress from;
  int port;
};

//////////////////////////
// Duino Nodes
//////////////////////////
typedef struct nodeState {
  byte pins;
};
typedef struct nodeGroup {
  byte clockPin;
  byte latchPin;
  byte dataPin;
  byte numNodes;
  nodeState *nodes;
};
typedef struct nodeAddress {
  word node; // high byte is chain id 0 - 255; low byte is node id 0 - 255
  byte pin;
};

//////////////////////////
// Blocks
//////////////////////////
typedef struct BLOCK_DEF {
  int pin;
  int aqv;
  float aqc;
  bool occ;
  bool chg_pending;
};

//////////////////////////
// Turnouts
//////////////////////////

typedef struct TURNOUT_DEF {
  int pin;
  int pos_main;
  int pos_div;
};
typedef struct TURNOUT_DATA {
  TURNOUT_DEF data;
  bool is_moving;
  byte alignment;
  int pos_now;
  int target_pos;
  unsigned long last_move;
};

typedef struct T_ALIGN {
  int id;
  byte align;
};

//////////////////////////
// Signals
//////////////////////////
typedef struct SIGNAL_DEF {
  byte type; // 1=R; 2=G; 3=RG; 4=Y; 5=RY; 6=GY; 7=RGY (bit 1 = Red; bit 2 = Green; bit 3 = Yellow)
  nodeAddress addr; // root address of the Duino node for this signal
  int nextSignal; // index in signals array, for a "following" signal
  byte R_ID; // pin/bit id for red indication
  byte G_ID; // pin/bit id for green indication
  byte Y_ID; // pin/bit id for yellow indication
  byte state; // current signal state
  // data elements for running the signal
  T_ALIGN *turnouts; // these turnouts must be aligned as defined to get SIGNAL_GREEN
  byte numTurnouts; // counter for *turnouts because its an array or structs; limit of 255 turnouts is more than needed here
  byte *blocks; // list of that blocks must be unoccuppied to get SIGNAL_GREEN
  byte numBlocks;
  byte *following; // additional blocks ahead (depends on direction signal faces) watched for occupancy, resulting in SIGNAL_YELLOW caution 
  byte numFollowing;
};

