void nodeWrite(struct nodeAddress addr, byte state) {
  // write the state to the pin bit defined by addr
  bitWrite(nodeGroups[highByte(addr.node)].nodes[lowByte(addr.node)].pins, addr.pin, state);
  nodeRefresh();
}
void nodeSet(struct nodeAddress addr, byte state) {
  // set the pins element with a byte value
  nodeGroups[highByte(addr.node)].nodes[lowByte(addr.node)].pins = state;
}
byte nodeGet(struct nodeAddress addr){
  // get the pins element for a node
  return nodeGroups[highByte(addr.node)].nodes[lowByte(addr.node)].pins;
}
void nodeRefresh(){
  // Shift out current node data, one node group at a time
  for(int i = (NODE_GROUPS - 1); i>=0; i--) {
    // shift all bits out in MSB order (last node first):
    // Prepare to shift by turning off the output
    digitalWrite(nodeGroups[i].latchPin, LOW);
    // for each node in the group
    for(int j = (nodeGroups[i].numNodes - 1); j>=0; j--) {
      shiftOut(nodeGroups[i].dataPin, nodeGroups[i].clockPin, MSBFIRST, nodeGroups[i].nodes[j].pins);
    }
    // turn on the output to activate
    digitalWrite(nodeGroups[i].latchPin, HIGH);
  }
}

