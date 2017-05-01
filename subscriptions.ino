void registerSubscription(String type, String id, IPAddress ip, int port) {
  switch(type.toInt()){
    case 1: // block occupancy
      block_subs[num_block_subs] = {id, ip, port};         
      #ifdef INCLUDE_SERIAL
      Serial.print(F("Block occupancy subscriber: "));
      Serial.print( block_subs[num_block_subs].id  + " ");
      Serial.println( block_subs[num_block_subs].ip );
      #endif
      num_block_subs++;
      break;
    case 2: // turnout state
      turnout_subs[num_turnout_subs] = {id, ip, port};         
      #ifdef INCLUDE_SERIAL
      Serial.print(F("Turnout State subscriber: "));
      Serial.print( turnout_subs[num_turnout_subs].id  + " ");
      Serial.println( turnout_subs[num_turnout_subs].ip );
      #endif
      num_turnout_subs++;
      break;
    case 3: // reserved for signal state
      break;
  }
}

void notifySubscribers(int type, String data, String option){
  String msg;
  switch(type){
    case 1: // block occupancy
      msg = delimiter + F("101") + delimiter + data + delimiter + option;
      for(int i = 0; i < num_block_subs; i++){
        sendMessage(block_subs[i].ip, block_subs[i].port, msg);
      }
      break;
    case 2: // turnout state
      msg = delimiter + F("102") + delimiter + data + delimiter + option;
      for(int i = 0; i < num_turnout_subs; i++){
        sendMessage(turnout_subs[i].ip, turnout_subs[i].port, msg);
      }
      break;
    case 3: // reserved
      break;
  }
}
