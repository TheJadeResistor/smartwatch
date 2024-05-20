#include "config.h"



void appNotifyTab(){ //crashes on secoind time opening it??? doesnt have numbers
  idleBLE(3);
  curTime = millis();
  int16_t x, y, per;
  int notifyCounter;
  listSetup(); //<-you sus?
  //notifyListTest(); //test list <--YOU :(
  while(millis() < curTime+100){} //delay for 100ms, time for touch to release

  while(1) {
    idleBLE(3);
    listDraw(); //kills? - no? i dont know anymore
/*
    //battery
    ttgo->tft->setTextColor(TFT_GREEN, TFT_DARKCYAN); //TFT_GREENYELLOW
    ttgo->tft->setTextSize(1);
    per = ttgo->power->getBattPercentage();
    ttgo->tft->drawString(String(per) + "%", 204, 2, 2);
    //battery end
*/
    drawNotificationsBarApp();
    if(!deviceConnected) ttgo->tft->fillRect(191, 5, 8, 15, TFT_DARKCYAN); //x, y, w, h

    if(ttgo->getTouch(x, y)) {  // if a touch
      break; //go to homescreen if touch
    }
    //sleep(); //funtion checks for timout and goes to lwpwr
  } //end loop

}


void drawNotificationsBarApp(){
  //battery
  ttgo->tft->fillRect(200, 1, 20, 20, TFT_DARKCYAN); //x, y, w, h
  ttgo->tft->setTextSize(1);
  ttgo->tft->setTextColor(TFT_GREEN, TFT_DARKCYAN); //TFT_GREENYELLOW
  per = ttgo->power->getBattPercentage();
  ttgo->tft->drawString(String(per) + "%", 204, 2, 2);

  //https://ai.thestempedia.com/docs/evive/evive-arduino-ide-tutorials/how-to-draw-shapes-on-tft-display/ 
//device connected
  if(deviceConnected) //does not show up a second time
    ttgo->tft->fillCircle(195, 10, 3, TFT_GREEN); //fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
  else
    ttgo->tft->fillRect(191, 5, 8, 15, TFT_DARKCYAN); //x, y, w, h
}


void listSetup(){ //Create screen setup
  ttgo->tft->fillScreen(TFT_BLACK);
  ttgo->tft->fillRect(0, 0, 300, 22, TFT_DARKCYAN); //TFT_DARKCYAN is grey
  ttgo->tft->setTextSize(1);
  ttgo->tft->setTextColor(TFT_BLACK, TFT_DARKCYAN);
  ttgo->tft->drawString("Notifications", 80, 7, 1);

  for(int i=0; i<maxNotifications; i++){ 
        ttgo->tft->setTextColor(TFT_MAGENTA, TFT_BLACK);
        ttgo->tft->drawString("-",  5, (i*20)+30, 1);   
        //Serial.println(notifyList[i]); 
      }
}

void listDraw(){ //higher number = more recent
  ttgo->tft->setTextFont(4);
  ttgo->tft->setTextSize(1);
  ttgo->tft->setTextColor(TFT_MAGENTA, TFT_BLACK);
  int num = maxNotifications;
  for(int i=0; i<=maxNotifications; i++){ 
    ttgo->tft->drawString(notifyList[num],  17, (i*20)+10, 1); //print notifications
    //Serial.println(notifyList[num]); //not printint zero pos -- fix by i<=max to include zero position
    num--;  
  }
}

//test kills on second opening of list app --fixed(i guess?)
void notifyListTest(){ //0tomax fill 0tomax
  for(int i=0; i<maxNotifications; i++){
    notifyList[i] = String(i);
  }
}

void clearList(){ //black rect over words. I dont like it but it just overwrites the pixels with black
  ttgo->tft->fillRect(17, 22, 300, 300, TFT_BLACK);
}