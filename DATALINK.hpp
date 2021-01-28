#pragma once

#include <WiFi.h>
#include <AsyncUDP.h>
#include <AsyncTCP.h>
#include "external/mavlink/common/mavlink.h"


enum proto_t {
  UDP,
  TCP
};


struct config_t {  
  const char* ssid;
  const char* pwd;  
  
  IPAddress ip;
  proto_t proto;
  int port;
};


//===========================================================================================
// DATALINK CLASS
//===========================================================================================
class DATALINK {

private:
  static AsyncUDP udp;
  static AsyncClient tcp;  

public:
  static mavlink_message_t msg;
  static mavlink_status_t status;

  static mavlink_heartbeat_t hb; 
  static mavlink_vfr_hud_t hud;
  static mavlink_attitude_t atti;

public:  
  static void init(config_t* config);

private:  
  static void processMavlink(const mavlink_message_t mvMsg);
  static void processUDP(AsyncUDPPacket packet);  
  static void processTCP(void* arg, AsyncClient* client, void *data, size_t len);
  
};
