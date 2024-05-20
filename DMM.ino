#include "config.h"

//to be implemented with a ESP-NOW connection to a ESP-01
//acting as a wireless mini multimeter to this watch (with a dedicated OLED display)
//this watch will act as a secondary screen for the DMM
//esp01 pocket device to use as dmm and sig gen? universal remote, etc. etc.
//add hold functiuon button? max val

int half_width = 120;
int curTime=0;

void appDMM(void) {
  idleBLE(1);
  Serial.println("DMMESP start");

  ttgo->tft->fillScreen(TFT_BLACK);
  ttgo->tft->setTextFont(4);
  ttgo->tft->setTextSize(1);
  ttgo->tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  ttgo->tft->drawCentreString("Digital Multimeter",  half_width, 10, 2);
  ttgo->tft->drawCentreString("65V Max",  half_width, 202, 2);
  ttgo->tft->drawCentreString("ADS1115 - 16 Bit",  half_width, 220, 2);
  ttgo->tft->setTextColor(TFT_BLUE, TFT_BLACK);
  ttgo->tft->setTextSize(1);
  ttgo->tft->setCursor(100, 140);
  ttgo->tft->print("VDC");
  
  while(1) {
    idleBLE(1);
    drawNotificationsBar(); //displayBatt();

    ttgo->tft->setTextSize(2);
    ttgo->tft->setTextColor(TFT_BLUE, TFT_BLACK);
    ttgo->tft->setCursor(75, 85);
    ttgo->tft->print(voltage); //ttgo->tft->print("88:88");
    //voltage = 0.00;

    curTime = millis();
    while(millis() < curTime+100){} //delay for 100ms
    if(ttgo->getTouch(x, y)) {  // if a touch
        break;
      }
  } //end loop

}
