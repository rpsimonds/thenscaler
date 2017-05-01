// Common Function to retrieve IP and MAC addresses stored in eprom
struct IPMAC readIPMAC(){
  IPMAC ipm;
  for (int i = MAC_START; i < MAC_LENGTH; i++)
     ipm.mac[i] = EEPROM.read(i);
   for (int j = 0, i = IP_START; j < IP_LENGTH; j++, i++)
     ipm.ip[j] = EEPROM.read(i);
   return ipm;
}
// Network polling function
PKT_DEF pollNet(){
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
  String reply, strpkt;
  PKT_DEF pkt;
  int packetSize;
  // if there's data available, read a packet
  packetSize = Udp.parsePacket();
  if(packetSize)
  {     
    // read the packet into packetBufffer
    Udp.read(packetBuffer,packetSize);
    for(int i = 0; i < packetSize; i++){
      strpkt += packetBuffer[i];
    }
    pkt = parsePKT(strpkt);
    pkt.from = Udp.remoteIP();
    pkt.port = Udp.remotePort();
  } else {
    pkt.function = "0";
  }
  return pkt;
}

void announce(String services){
  IPAddress broadcast(255, 255, 255, 255);
  self_identify(broadcast, UDP_PORT, services);
}
 
void self_identify(IPAddress ip, int port, String services){
  String msg;
  msg = delimiter + String(F("254")) + delimiter;
  msg += F(SYS_ID);
  msg += delimiter + services; 
  sendMessage(ip, port, msg);
}

void sendMessage(IPAddress to, int port, String msg){
  char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1];
  msg.toCharArray(packetBuffer, msg.length() + 1);  
  Udp.beginPacket(to, port);
  Udp.write(packetBuffer);
  Udp.endPacket();
}


struct PKT_DEF parsePKT(String packet) {
   PKT_DEF pkt;
   int segment = 1;
   char delimiter = '/';
   int delimIndex = packet.indexOf(delimiter);
   if(delimIndex == 0) { // drop leading delimiter, if any
    packet.remove(0,1);
    delimIndex = packet.indexOf(delimiter);
   }
   int lastDelim = -1;
   while(delimIndex >= 0) {
     switch(segment){
       case 1:
         pkt.function = packet.substring(lastDelim + 1, delimIndex);
         break;
       case 2:
         pkt.option = packet.substring(lastDelim + 1, delimIndex);
         break;
       case 3:
         pkt.data = packet.substring(lastDelim + 1, delimIndex);
         break;
       default:      
         break;
     }
     segment++;
     lastDelim = delimIndex;
     delimIndex = packet.indexOf(delimiter, lastDelim + 1);  
   }
   // if we don't already have a data field  
   // any trailing element without a deliminter is data
   if(pkt.data.length() == 0 && lastDelim < packet.length()){ 
     pkt.data = packet.substring(lastDelim + 1);
   }
   return pkt;
 }
 

