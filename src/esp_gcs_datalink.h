#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "mavlink/common/mavlink.h"

#include "esp_gcs_types.h"
#include "esp_gcs_adsb_decoder.h"

#include <Arduino.h>
#include <unordered_map>
#include <string>
#include <cmath>


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

private:
  static adsb_context m_adsb_ctx;
  static std::string m_tcp_rx_buffer;

public:  
  void init(esp_gcs_config_t* config);
  void reconnect(void);

  static const adsb_context& get_adsb_context() {
    return m_adsb_ctx;
  }

  static void set_adsb_local_reference(double lat, double lon);
  static void print_aircrafts();

private:  
  static const char* mav_state_to_string(int state);  
  
  static void process_mavlink(void* arg, AsyncClient* client, void *data, size_t len);
  static void process_sbs1(void* arg, AsyncClient* client, void *data, size_t len);
  static void process_raw(void* arg, AsyncClient* client, void *data, size_t len);
};

//#####################################################################################

