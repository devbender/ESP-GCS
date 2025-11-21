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

adsb_context ESP_GCS_DATALINK::m_adsb_ctx;
std::string ESP_GCS_DATALINK::m_tcp_rx_buffer;


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
void ESP_GCS_DATALINK::process_mavlink(void* arg, AsyncClient* client, void *data, size_t len) {
      
  uint8_t* p = (uint8_t*)data;

  for(int i=0; i<len; i++) {    
    if(mavlink_parse_char(MAVLINK_COMM_0, p[i], &msg, &status)) {

      switch(msg.msgid) {

        // HB_MSG --------------------------------------------
        case MAVLINK_MSG_ID_HEARTBEAT: {
          mavlink_msg_heartbeat_decode(&msg, &hb);
          time_since_last_hb = millis()/1000;
          log_d("mavlink_heartbeat_t | System Status: %s | %i", mav_state_to_string(hb.system_status), millis()/1000 );
          break;
        }

        // VFR_HUD_MSG ---------------------------------------
        case MAVLINK_MSG_ID_VFR_HUD: {
          mavlink_msg_vfr_hud_decode(&msg, &hud);      
          break;
        }

        // ATTIUDE_MSG ---------------------------------------- 
        case MAVLINK_MSG_ID_ATTITUDE: {
          mavlink_msg_attitude_decode(&msg, &atti);
          log_d("mavlink_attitude_t | %.2f \t %.2f \t %.2f", degrees(atti.pitch), degrees(atti.roll), degrees(atti.yaw) );
          break;
        }

        default: {}        
      }
    
    }
  }
}


void ESP_GCS_DATALINK::process_sbs1(void* arg, AsyncClient* client, void *data, size_t len) {
  const char* cp = reinterpret_cast<const char*>(data);
  std::string s(cp, len);
  while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
  log_d("sbs1_data | %s", s.c_str());
}


void ESP_GCS_DATALINK::process_raw(void* arg, AsyncClient* client, void *data, size_t len) {
  
  // 1. Append the new data to our persistent member buffer
  const char* cp = reinterpret_cast<const char*>(data);
  m_tcp_rx_buffer.append(cp, len);

  // 2. Get the current time ONCE.  
  float current_time = millis() / 1000.0f;

  // 3. Process every complete message in the buffer
  while (true) {
    
    // Find the start of a message
    size_t start = m_tcp_rx_buffer.find('*');
    if (start == std::string::npos) {
      // No start marker found. The buffer might just be junk.
      // To prevent infinite buffer growth, we can clear it if it's junk
      // and getting too big.
      if (m_tcp_rx_buffer.length() > 2048) {
          m_tcp_rx_buffer.clear();
      }
      break; // Exit loop, wait for more data
    }

    // Find the end of the message *after* the start
    size_t end = m_tcp_rx_buffer.find(';', start);
    if (end == std::string::npos) {
      // We found a '*' but no ';'. This is a partial message.
      // We must wait for the rest of it.
      
      // Optimization: If there was junk *before* the partial message,
      // (e.g., "abc*123"), we can erase the "abc".
      if (start > 0) {
          m_tcp_rx_buffer.erase(0, start);
      }
      break; // Exit loop, wait for more data
    }

    // ---
    // If we are here, we have a complete message from 'start' to 'end'
    // ---
    
    // 4. Extract the message. This does one small allocation,
    // which is acceptable in the network handler.
    std::string msg = m_tcp_rx_buffer.substr(start, end - start + 1);

    // 5. Call decoder with the context, message, and time
    adsb_decode_message(m_adsb_ctx, msg.c_str(), current_time);

    log_v("adsb_raw_data: %s | time: %f", msg.c_str(), current_time);

    // 6. Erase the processed message (and any junk before it)
    // from the buffer, so we can look for the next one.
    m_tcp_rx_buffer.erase(0, end + 1);
  }
}


void ESP_GCS_DATALINK::set_cpr_local_reference(double lat, double lon) {
  // Call the global 'inline' function from esp_gcs_adsb_decoder.h
  // and pass it our private static context
  adsb_set_local_reference(m_adsb_ctx, lat, lon);
  log_i("ADSB local reference set to: %.6f, %.6f", lat, lon);
}


//#####################################################################################
void ESP_GCS_DATALINK::init(esp_gcs_config_t* _config) {

  log_d("datalink init");

  config.network.ip = _config->network.ip;
  config.network.port = _config->network.port;

  WiFi.mode(WIFI_STA);
  WiFi.begin(_config->network.ssid, _config->network.password);

  if (WiFi.waitForConnectResult() != WL_CONNECTED)  {
    log_d("wifi connection error");
    return;  
  }
  
  log_i("WIFI connected IP: %s", WiFi.localIP().toString().c_str());
  delay(100); 


  tcp.onConnect( [this](void* arg, AsyncClient* tcp) { 
    log_d("TCP connected: %s | %i", 
      tcp->remoteIP().toString().c_str(), 
      tcp->remotePort() ); 
  }, nullptr);
  
  tcp.onDisconnect( [this](void* arg, AsyncClient* tcp) { 
    log_d("*** TCP DISCONNECTED ***");
    
    atti.pitch = 0.0f;
    atti.roll = 0.0f;
    
    this->reconnect();
  }, nullptr );

  
  if( _config->protocol == ADSB_MAVLINK ) {
    tcp.onData( &ESP_GCS_DATALINK::process_mavlink, &tcp );
    log_d("ADSB_MAVLINK protocol selected");
  }
  
  else if( _config->protocol == ADSB_SBS1 ) {
    tcp.onData( &ESP_GCS_DATALINK::process_sbs1, &tcp );
    log_d("ADSB_SBS1 protocol selected");
  }

  else if( _config->protocol == ADSB_RAW ) {
    tcp.onData( &ESP_GCS_DATALINK::process_raw, &tcp );
    log_d("ADSB_RAW protocol selected");
  }
  
  
  tcp.connect(_config->network.ip, _config->network.port);  
  delay(100);
}


//#####################################################################################
void ESP_GCS_DATALINK::reconnect(){
  tcp.connect(config.network.ip, config.network.port);

}







//#####################################################################################
void ESP_GCS_DATALINK::print_aircrafts() {

  uint32_t current_time_ms = millis();
  float current_time_sec = current_time_ms / 1000.0f;

  // Get a read-only reference to the ADS-B context
  const adsb_context& ctx = ESP_GCS_DATALINK::get_adsb_context();

  int active_aircraft = 0;

  // ---Loop through the aircraft array ---  
  log_i("\e[2J\e[H");    // Clear screen + move cursor home  

  log_i("--------------------------------------------------------------------------------------------------------------------");
  for (int i = 0; i < ADSB_MAX_AIRCRAFT; ++i) {
      
      // Get a reference to the aircraft data
      const adsb_data& ac = ctx.aircraft[i];

      // icao == 0 slot empty, so we skip it
      if (ac.icao != 0) {

          active_aircraft++;
          
          // Print all available data in a formatted string
          char buffer[256];
          snprintf(buffer, sizeof(buffer),
              " * ICAO: %06X | CS: %-8s | ALT: %5ld ft | SPD: %3.0f kt | HDG: %3.0f | POS: %7.4f, %7.4f | SEEN: %.0fs ago",
              ac.icao,
              ac.valid_callsign ? ac.callsign : "------",
              (ac.alt == INT32_MIN) ? 0 : ac.alt, // Show 0 for invalid Gillham
              ac.valid_vel ? ac.speed : 0.0f,
              ac.valid_vel ? ac.heading : 0.0f,
              ac.valid_pos ? ac.lat : 0.0f,
              ac.valid_pos ? ac.lon : 0.0f,
              current_time_sec - ac.last_seen
          );
          // print buffer
          log_i("%s", buffer);                
      }
  }  

  if (active_aircraft == 0) {
      log_i(" ***  No active aircraft *** ");
  }
  
  log_i("--------------------------------------------------------------------------------------------------------------------");
}