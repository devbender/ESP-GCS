#include "esp_gcs_datalink.h"

//#####################################################################################

uint32_t ESP_GCS_DATALINK::time_since_last_hb = 0;
esp_gcs_config_t ESP_GCS_DATALINK::config;

mavlink_message_t ESP_GCS_DATALINK::msg;
mavlink_status_t ESP_GCS_DATALINK::status;

mavlink_heartbeat_t ESP_GCS_DATALINK::hb;
mavlink_vfr_hud_t ESP_GCS_DATALINK::hud;
mavlink_attitude_t ESP_GCS_DATALINK::atti;

AsyncClient ESP_GCS_DATALINK::tcp;



//#####################################################################################
const char* ESP_GCS_DATALINK::mav_state_to_string(int state) {
    switch (state) {
        case 0: return "UNINIT (Booting up)";
        case 1: return "BOOT (Boot process)";
        case 2: return "CALIBRATING (Not flight-ready)";
        case 3: return "STANDBY (Ready to arm)";
        case 4: return "ACTIVE (Fully operational)";
        case 5: return "CRITICAL (Reduced ability)";
        case 6: return "EMERGENCY (Emergency state)";
        case 7: return "POWEROFF (System is off)";
        case 8: return "FLIGHT_TERMINATION (Failsafe)";
        default: return "UNKNOWN (Invalid state)";
    }
}




//#####################################################################################
void ESP_GCS_DATALINK::processMavlink(const mavlink_message_t mvMsg) {
  
  switch(mvMsg.msgid) {

    // HB_MSG --------------------------------------------
    case MAVLINK_MSG_ID_HEARTBEAT: {
      mavlink_msg_heartbeat_decode(&mvMsg, &hb);
      time_since_last_hb = millis()/1000;
      //log_d("mavlink_heartbeat_t | System Status: %s | %i", mav_state_to_string(hb.system_status), millis()/1000 );
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
      //log_d("mavlink_attitude_t | %.2f \t %.2f \t %.2f", degrees(atti.pitch), degrees(atti.roll), degrees(atti.yaw) );
      break;
    }

    default: {}
    
  }
}




//#####################################################################################
void ESP_GCS_DATALINK::processTCP(void* arg, AsyncClient* client, void *data, size_t len) {
      
  uint8_t* p = (uint8_t*)data;

  for(int i=0; i<len; i++) {    
    if(mavlink_parse_char(MAVLINK_COMM_0, p[i], &msg, &status))
      processMavlink(msg);
  }
}





//#####################################################################################
void ESP_GCS_DATALINK::init(esp_gcs_config_t* _config) {

  log_d("datalink init");

  config.ip = _config->ip;
  config.port = _config->port;

  WiFi.mode(WIFI_STA);
  WiFi.begin(_config->ssid, _config->password);

  if (WiFi.waitForConnectResult() != WL_CONNECTED)  
    log_d("wifi connection error");    
  else  
    log_d("wifi connected!  IP: %s", WiFi.localIP().toString().c_str());      
  
  delay(100); 


  auto onConnectLambda = [](void* arg, AsyncClient* tcp) { 
    log_d("TCP connected: %s | %i", tcp->remoteIP().toString().c_str(), tcp->remotePort() ); 
  };
  
  auto reconnectLambda = [](void* arg, AsyncClient* tcp) { 
    log_d("*** TCP DISCONNECTED ***");
    
    atti.pitch = 0.00;
    atti.roll = 0.00;
    
    delay(1000);
    reconnect();
  };

  tcp.onData(&ESP_GCS_DATALINK::processTCP, &tcp);
  tcp.onConnect(onConnectLambda, &tcp);      
  tcp.connect(_config->ip, _config->port);
  tcp.onDisconnect( reconnectLambda, &tcp  );
  delay(100);

}


//#####################################################################################
void ESP_GCS_DATALINK::reconnect(){
  tcp.connect(config.ip, config.port);

}