#pragma once

#include "SYSTEM.hpp"
#include "external/mavlink/common/mavlink.h"


struct hbmode_t {
  uint8_t baseMode;
  uint8_t customMode;  
};


struct fmode_t {
  char* fmode;
  uint16_t color;
  hbmode_t modeA, modeD;
};


static inline bool operator ==(const hbmode_t &m1, const hbmode_t &m2) {
  return m1.baseMode == m2.baseMode && m1.customMode == m2.customMode;  
}

static inline bool operator !=(const hbmode_t &m1, const hbmode_t &m2) {
  return m1.baseMode != m2.baseMode || m1.customMode != m2.customMode;  
}

//===========================================================================================
// PDF CLASS
//===========================================================================================
class PFD: public SYSTEM {

private:
  TFT_eSprite ai = TFT_eSprite(&tft);
  TFT_eSprite lvl1 = TFT_eSprite(&tft);
  TFT_eSprite lvl2 = TFT_eSprite(&tft);
  
  TFT_eSprite ti = TFT_eSprite(&tft);
  TFT_eSprite da = TFT_eSprite(&tft);

  TFT_eSprite br = TFT_eSprite(&tft);
  TFT_eSprite tbr = TFT_eSprite(&tft);

  TFT_eSprite mbr = TFT_eSprite(&tft);

  int RollRefLines[10][4];

  uint16_t SKY = 0x02B5;
  uint16_t GND = 0x5140;

  uint16_t counter = 0;
  long startMillis = millis();
  uint16_t interval = 50;

  char txtBuff[4];
  hbmode_t prevMode;

  
  
public:
  PFD() {};  
  void init(uint16_t FB_WIDTH, uint16_t FB_HEIGHT);
  void render(mavlink_heartbeat_t hb, mavlink_attitude_t atti, mavlink_vfr_hud_t hud);

private:

  void checkButtons();
  void calculateFPS();
  void preCalcRollReferenceLines();
  void renderPFDTurnIndicator(TFT_eSprite *sp);
  void renderPFDPitchScale(TFT_eSprite *sp, int pitchPx);  
  
};
