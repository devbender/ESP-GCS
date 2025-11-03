#pragma once

#define LGFX_USE_V1
#include <LovyanGFX.hpp>



enum esp_gcs_display_t {
    SPI_9342,
    PARALLEL_9488    
};



enum esp_gcs_touch_t {
  NONE,
  FT6236
};


enum esp_gcs_proto_t {
    ADSB_MAVLINK,
    ADSB_SBS1,
    ADSB_RAW
};



enum esp_gcs_link_t {
    TCP,
    UDP,
    BLUETOOTH,
    SERIAL_ADSB,
    ESPNOW
};



struct esp_gcs_config_t {
  
  struct {
    esp_gcs_display_t type;
    uint16_t width, height;
  } display;

  esp_gcs_link_t link;
  esp_gcs_proto_t protocol;
  
  struct {
    const char* ssid;
    const char* password;
    const uint8_t* mac;

    IPAddress ip;  
    int port;
  } network;
  
};



// struct adsb_data {
//   int32_t alt;         // altitude (ft)
//   float lat, lon;      // degrees
//   float heading;       // degrees, 0–359
//   float speed;         // knots
//   int32_t v_speed;     // vertical speed (ft/min)
//   float last_seen;     // seconds since boot or last update
//   bool valid_pos;      // true if lat/lon valid
//   bool valid_vel;      // true if speed/heading valid
// };




struct hbmode_t {
  uint8_t baseMode;
  uint8_t customMode;  
};



static inline bool operator ==(const hbmode_t &m1, const hbmode_t &m2) {
  return m1.baseMode == m2.baseMode && m1.customMode == m2.customMode;  
}



static inline bool operator !=(const hbmode_t &m1, const hbmode_t &m2) {
  return m1.baseMode != m2.baseMode || m1.customMode != m2.customMode;  
}



struct fmode_t {
  const char* fmode;
  uint16_t color;
  hbmode_t modeA, modeD;
};



const fmode_t flightModes[] = {
  //{FLIGHTMODE,COLOR,        ARMED,    DISARMED}
  {"STABILIZE", TFT_WHITE,    {209,0},  {81,0}  },
  {"ACRO",      TFT_WHITE,    {209,1},  {81,1}  },
  {"ALT HOLD",  TFT_SKYBLUE,  {209,2},  {81,2}  },
  {"AUTO",      TFT_CYAN,     {0,0},    {89,3}  }, // >> not armable mode
  {"GUIDED",    TFT_ORANGE,   {217,4},  {89,4}  },
  {"LOITER",    TFT_BLUE,     {217,5},  {89,5}  },
  {"RTL",       TFT_CYAN,     {0,0},    {89,6}  }, // >> not armable mode
  {"CIRCLE",    TFT_CYAN,     {0,0},    {89,7}  }, // >> not armable mode
  {"LAND",      TFT_CYAN,     {0,0},    {81,9}  }, // >> not armable mode
  {"DRIFT",     TFT_CYAN,     {0,0},    {81,11} }, // >> not armable mode
  {"SPORT",     TFT_CYAN,     {0,0},    {81,13} }, // >> not armable mode
  {"POS HOLD",  TFT_BLUE,     {217,16}, {89,16} }                          
};



enum filter_t {
  NO_FILTER,
  MEDIAN,
  M_AVERAGE,
  W_AVERAGE,  
  KALMAN_FILTER
};