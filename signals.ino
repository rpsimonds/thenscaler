void refreshSignals() {
  // First pass, set Stop (RED) state; default is Go (GREEN) otherwise
  // from block occupancy or turnout states
  for(int i = 0; i < NUM_SIGNALS; i++){ // for each signal
    // default state
    int state = SIGNAL_GREEN;
    SIGNAL_DEF sig = signals[i];
    SIGNAL_DEF next;
    // FIrst Pass to set RED states
    for(int j = 0; j < max(sig.numTurnouts, sig.numBlocks); j++){ // for each linked turnout and block
      if(j < sig.numTurnouts){
        // if the turnout is in motion OR if turnout alignment does not equal the required alignment defined in SIGNAL_DEF
        if(turnout[sig.turnouts[j].id].is_moving || turnout[sig.turnouts[j].id].alignment != sig.turnouts[j].align){
         state = SIGNAL_RED;
        } 
      }
      if(j < sig.numBlocks){ // for each linked block in the SIGNAL_DEF
        if(blocks[sig.blocks[j]].occ){
          state = SIGNAL_RED;
        }
      }
    }
    setSignalBits(i, state);
  }
  
  // Second pass to set caution states on
  // signals that support it and are currently set to GO
  // from "sig.next" and "sig.following" signal or block states
  for(int i = 0; i < NUM_SIGNALS; i++){ // for each signal
    SIGNAL_DEF sig = signals[i];
    if(bitRead(sig.type, 2)){ // if the signal supports the caution state
      SIGNAL_DEF next;
      if(sig.nextSignal >= 0 && sig.state != SIGNAL_RED){
        next = signals[sig.nextSignal];
        if(next.state == SIGNAL_RED){
          setSignalBits(i, SIGNAL_YELLOW);
        } 
      }
      if(sig.numFollowing > 0 && sig.state == SIGNAL_GREEN){
        for(int j = 0; j < sig.numFollowing; j++){
          if(blocks[sig.following[j]].occ == true){
            setSignalBits(i, SIGNAL_YELLOW);
          }
        }
      }
    }
  }
  // Refresh the nodes to show signals in updated state
  nodeRefresh();
}

void setSignalBits(int signalID, byte signalState) {
  SIGNAL_DEF sig = signals[signalID];
  if (sig.state != signalState) {
    signals[signalID].state = signalState;
    byte nodeBits = nodeGet(sig.addr);
    switch (signalState) {
      case SIGNAL_OFF:
        if(bitRead(sig.type, 0)) bitWrite(nodeBits, sig.R_ID, LOW);
        if(bitRead(sig.type, 1)) bitWrite(nodeBits, sig.G_ID, LOW);
        if(bitRead(sig.type, 2)) bitWrite(nodeBits, sig.Y_ID, LOW);
        break;
      case SIGNAL_RED:
        if(bitRead(sig.type, 0)) bitWrite(nodeBits, sig.R_ID, HIGH);
        if(bitRead(sig.type, 1)) bitWrite(nodeBits, sig.G_ID, LOW);
        if(bitRead(sig.type, 2)) bitWrite(nodeBits, sig.Y_ID, LOW);
        break;
      case SIGNAL_GREEN:
        if(bitRead(sig.type, 0)) bitWrite(nodeBits, sig.R_ID, LOW);
        if(bitRead(sig.type, 1)) bitWrite(nodeBits, sig.G_ID, HIGH);
        if(bitRead(sig.type, 2)) bitWrite(nodeBits, sig.Y_ID, LOW);
        break;
      case SIGNAL_YELLOW:
        if(bitRead(sig.type, 0)) bitWrite(nodeBits, sig.R_ID, LOW);
        if(bitRead(sig.type, 1)) bitWrite(nodeBits, sig.G_ID, LOW);
        if(bitRead(sig.type, 2)) bitWrite(nodeBits, sig.Y_ID, HIGH);
        break;
    }
    nodeSet(sig.addr, nodeBits);
  }
  return;
}

void setSignal(int signalID, byte signalState) {
  setSignalBits(signalID, signalState);
  nodeRefresh();
}

