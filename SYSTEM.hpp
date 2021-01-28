#pragma once

#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>

#include "Button.h"
#include "Speaker.h"

#define BUTTON_A_PIN 39
#define BUTTON_B_PIN 38
#define BUTTON_C_PIN 37

#define NUMPIXELS 10
#define NP_DATA_PIN 15

//===========================================================================================
// SYSTEM CLASS
//===========================================================================================
class SYSTEM {

protected:
  bool showMenu = false;
  uint16_t menuLastActive;
  uint8_t autoHideMenuSecs = 5;

public:
  uint16_t FB_WIDTH, FB_HEIGHT;  

  TFT_eSPI tft = TFT_eSPI();
  TFT_eSprite fb = TFT_eSprite(&tft);
  Adafruit_NeoPixel np = Adafruit_NeoPixel(NUMPIXELS, NP_DATA_PIN, NEO_GRB + NEO_KHZ800);

  // Speaker
  SPEAKER spk;

  // Button API
  #define DEBOUNCE_MS 10
  Button BtnA = Button(BUTTON_A_PIN, true, DEBOUNCE_MS);
  Button BtnB = Button(BUTTON_B_PIN, true, DEBOUNCE_MS);
  Button BtnC = Button(BUTTON_C_PIN, true, DEBOUNCE_MS);

  void BtnUpdate();
  void setNP(int r, int g, int b);
  void flashNP(int times, int delayMs);
  void flashNP(int times, int delayMs, int r, int g, int b);
  void initFB(uint16_t _FB_WIDTH, uint16_t _FB_HEIGHT);
};
