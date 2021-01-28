#include "SYSTEM.hpp"


void SYSTEM::initFB(uint16_t _FB_WIDTH, uint16_t _FB_HEIGHT) {

  FB_WIDTH = _FB_WIDTH;
  FB_HEIGHT = _FB_HEIGHT;

  /*
  spk.begin();
  spk.tone(500); delay(100);
  spk.tone(800); delay(100);
  spk.tone(400); delay(100);
  spk.mute();
  */
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK); 
  tft.setPivot(tft.width()/2, tft.height()/2);
  
  // 8 bit Frame Buffer ------------------------
  fb.setColorDepth(8);
  fb.createSprite(FB_WIDTH, FB_HEIGHT);  
  fb.fillSprite(TFT_TRANSPARENT);
  
  fb.setTextSize(1);
  fb.setTextColor(TFT_WHITE);

  fb.drawCentreString("FB INIT OK!", FB_WIDTH/2, FB_HEIGHT/2, 2);

  int fbCx = (tft.width() - FB_WIDTH)/2;
  int fbCy = (tft.height() - FB_HEIGHT)/2;
  
  fb.pushSprite(fbCx, fbCy, TFT_TRANSPARENT);

  np.begin();
  np.setBrightness(100);    
  
  delay(1000);
}

void SYSTEM::BtnUpdate() {
  //Button update
  BtnA.read();
  BtnB.read();
  BtnC.read();

  // Show Menu
  if( BtnB.wasPressed() ) {
    showMenu = true;
    menuLastActive = millis()/1000;
  } 
  
  // Button Actions
  if( BtnA.pressedFor(1000) ) flashNP(3,200, 255,0,0);
  if( BtnB.pressedFor(1000) ) showMenu=false;
  if( BtnC.pressedFor(1000) ) flashNP(3,200, 0,0,255);

  // Auto-hide Menu
  if( ( (millis()/1000) - menuLastActive ) >= autoHideMenuSecs) showMenu=false;
}

void SYSTEM::flashNP(int times, int delayMs, int r, int g, int b) {
  for(int i=0; i<times; i++) {
    
    for(int i=0;i<NUMPIXELS;i++){
      np.setPixelColor(i, np.Color(r,g,b));
      np.show();
    }

    delay(delayMs);

    for(int i=0;i<NUMPIXELS;i++){
      np.setPixelColor(i, np.Color(0,0,0));
      np.show();
    }
    
    delay(delayMs);

  }
}


void SYSTEM::flashNP(int times, int delayMs) {
  for(int i=0; i<times; i++) {
    
    for(int i=0;i<NUMPIXELS;i++){
      np.setPixelColor(i, np.Color(255,255,255));
      np.show();
    }

    delay(delayMs);

    for(int i=0;i<NUMPIXELS;i++){
      np.setPixelColor(i, np.Color(0,0,0));
      np.show();
    }
    
    delay(delayMs);

  }
}


void SYSTEM::setNP(int r, int g, int b) {
  for(int i=0;i<NUMPIXELS;i++){
    np.setPixelColor(i, np.Color(r,g,b));
    np.show();
  }
}
