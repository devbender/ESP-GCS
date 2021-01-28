#include "DATALINK.hpp"


mavlink_message_t DATALINK::msg;
mavlink_status_t DATALINK::status;

mavlink_heartbeat_t DATALINK::hb;
mavlink_vfr_hud_t DATALINK::hud;
mavlink_attitude_t DATALINK::atti;

AsyncUDP DATALINK::udp;
AsyncClient DATALINK::tcp;


void DATALINK::init(config_t* config) {

  WiFi.setSleep(false);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(config->ssid, config->pwd);
  
  if (WiFi.waitForConnectResult() != WL_CONNECTED)  log_d("WIFI CONNECTION ERROR");    
  else  log_d("WIFI CONNECTED!  IP: %s", WiFi.localIP().toString().c_str());      
  delay(100);  

  if(config->proto == UDP) {
  
    if(udp.connect(config->ip, config->port)) {
        log_d("UDP up on IP: %s", WiFi.localIP().toString().c_str());
        udp.onPacket(DATALINK::processUDP);
        //reqMavlinkStream();
    }
  }    
  else if(config->proto == TCP) {
    auto onConnectLambda = [](void* arg, AsyncClient* tcp) { log_d(">> TCP CONNECTED"); };
    
    tcp.onData(&DATALINK::processTCP, &tcp);
    tcp.onConnect(onConnectLambda, &tcp);      
    tcp.connect(config->ip, config->port);
    delay(100);
  }

  delay(1000);  
}




void DATALINK::processMavlink(const mavlink_message_t mvMsg) {
  
  switch(mvMsg.msgid) {

    // HB_MSG --------------------------------------------
    case MAVLINK_MSG_ID_HEARTBEAT: {
      mavlink_msg_heartbeat_decode(&mvMsg, &hb);
      break;
    }

    // VFR_HUD_MSG ---------------------------------------
    case MAVLINK_MSG_ID_VFR_HUD: {
      mavlink_msg_vfr_hud_decode(&mvMsg, &hud);
      break;
    }

    // ATTIUDE_MSG ---------------------------------------- 
    case MAVLINK_MSG_ID_ATTITUDE: {
      mavlink_msg_attitude_decode(&mvMsg, &atti);
      break;
    }

    default: {}
    
  }
}



void DATALINK::processUDP(AsyncUDPPacket packet) {

  while(packet.available()) {          
    if(mavlink_parse_char(MAVLINK_COMM_0, packet.read(), &msg, &status))
      processMavlink(msg);      
  }
}




void DATALINK::processTCP(void* arg, AsyncClient* client, void *data, size_t len) {
      
  uint8_t* p = (uint8_t*)data;

  for(int i=0; i<len; i++) {    
    if(mavlink_parse_char(MAVLINK_COMM_0, p[i], &msg, &status))
      processMavlink(msg);
  }
}
