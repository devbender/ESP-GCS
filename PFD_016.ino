#include "PFD.hpp"
#include "DATALINK.hpp"

PFD pfd;
DATALINK datalink;

void setup() { 
  pfd.init(400, 240);
  
  //{SSID, PASS, IP, PROTO, PORT}
  config_t config{"ssid", "pass", IPAddress(10,0,0,32), TCP, 14001};
  datalink.init( &config );
}

void loop() {  
  pfd.render( DATALINK::hb,  DATALINK::atti, DATALINK::hud );
}
