// networking
#define MAC_START 0
#define MAC_LENGTH 6
#define IP_START 6
#define IP_LENGTH 4
#define STRING_CMD_SIZE 24
#define UDP_PORT 8888

// turnout values
#define NUM_TURNOUTS 1
#define STEP_DELAY 200
#define ALIGN_MAIN  1
#define ALIGN_DIVERGENT 2

// Block Detection
#define CALIBRATION_READS 5000
#define MASTER_DETECTION_MULTIPLIER 1.095
#define DETECTION_MULTIPLIER 1.095
#define SENSITIVITY 185  // ACS712 5A version, in mV/A
#define NUM_BLOCKS 4

// Duino Nodes
#define NODE_GROUPS 1
#define NODES 3

// Signals
#define NUM_SIGNALS 5
// Signal state values
#define SIGNAL_OFF 0
#define SIGNAL_RED 1
#define SIGNAL_GREEN 2
#define SIGNAL_YELLOW 3

// Data subscribers
#define MAX_SUBSCRIPTIONS 8
