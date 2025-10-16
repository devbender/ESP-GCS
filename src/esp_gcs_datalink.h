#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "mavlink/common/mavlink.h"

#include "esp_gcs_types.h"

//#####################################################################################

class ESP_GCS_DATALINK {

  
public:
  static AsyncClient tcp;
  static uint32_t time_since_last_hb;  
  static esp_gcs_config_t config;

  static mavlink_message_t msg;
  static mavlink_status_t status;

  static mavlink_heartbeat_t hb; 
  static mavlink_vfr_hud_t hud;
  static mavlink_attitude_t atti; 

public:  
  void init(esp_gcs_config_t* config);
  void reconnect(void);

private:  
  static const char* mav_state_to_string(int state);
  static void processMavlink(const mavlink_message_t mvMsg);  
  static void processTCP(void* arg, AsyncClient* client, void *data, size_t len);
  
};

//#####################################################################################

