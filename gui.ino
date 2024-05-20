#include "gui.h"
#include "config.h"
#include <time.h>

#define maxNotifications 10

int hh = 0;
int mm = 0;
int ss = 0;
int dday = 0;
int mmonth = 0;
int yyear = 0;
int mSelect;

int xmidL, ymidL = 100;
int xmidH, ymidH = 120;
bool swipped = false;
int currentTime;
int per = 0;
int notificationCounter = 0;
bool silented = true;

String notifyList[maxNotifications] = ""; //keep 10 notifications in storage

void getTime(){ //https://development.libelium.com/rtc-programming-guide/setting-time-and-date
    // Get the current data
    RTC_Date tnow = ttgo->rtc->getDateTime();
    //rtc.getTime();
      
    hh = tnow.hour;
    mm = tnow.minute + 2; // + 3.5; //add offset cause wrong--changeee??? goes to past 60 in min until 63.5
    ss = tnow.second;
    dday = tnow.day;
    mmonth = tnow.month;
    yyear = tnow.year;
}

void setupSetupGUI(){ //GUI code to run once
  setCpuFrequencyMhz(80);
  ttgo->displayWakeup();
  ttgo->rtc->syncToSystem();
  ttgo->openBL();
  ttgo->startLvglTick();
  ttgo->tft->fillScreen(TFT_BLACK); //background color
  //ttgo->tft->setRotation(3); 3 is facing away from uUSB
  ttgo->setBrightness(100); //set backlight brightness (0-100) no v3

/*
  //draw button for turn screen off
  ttgo->tft->setTextSize(3);
  ttgo->tft->fillRoundRect(80, 190, 75, 40, 5, TFT_PURPLE); //x, y, w, h, radius
  ttgo->tft->setTextColor(TFT_BLACK);
  ttgo->tft->drawString("OFF", 93, 200, 1);
  //off button: x:80to155 y:190to230
*/

  //arrow boxes
  ttgo->tft->fillRoundRect(-3, 100, 25, 40, 3, TFT_DARKCYAN); //x, y, w, h, radius
  ttgo->tft->fillTriangle(3,120,15,110,15,130,TFT_BLACK); //x1,y1,x2,y2,x3,y3,color
  ttgo->tft->fillRoundRect(218, 100, 25, 40, 3, TFT_DARKCYAN); //x, y, w, h, radius x-max:240
  ttgo->tft->fillTriangle(237,120,225,110,225,130,TFT_BLACK); //x1,y1,x2,y2,x3,y3,color
  ttgo->tft->fillRoundRect(100, -3, 40, 25, 3, TFT_DARKCYAN); //x, y, w, h, radius x-max:240
  ttgo->tft->fillTriangle(110,17,120,3,130,17,TFT_BLACK); //x1,y1,x2,y2,x3,y3,color

  //ttgo->tft->fillRoundRect(100, -3, 40, 25, 3, TFT_DARKCYAN); //x, y, w, h, radius, top
//        (x2,y2)
//(x1,y1)
//        (x3,y3)
//left side arrow box: x:0-25, y:100-140
//right side arrow box: x:220-20, y:100-140

//flashlight button?? 
  //silent
  ttgo->tft->fillRoundRect(3, 3, 30, 30, 2, TFT_DARKCYAN); //x, y, w, h, radius
}


void setupGui(){
  idleBLE(0);
  lv_task_handler(); 
  getTime();
  currentTime = millis();

  // Display Time
  //font 7 is 7-seg: good for time display
  ttgo->tft->setTextSize(1);
  ttgo->tft->setTextColor(0x7FF, TFT_BLACK); //0x7FF cyan, 0xFBE0 orange
  int xpos = 47;
  int ypos = 60;
  if (hh < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
  xpos += ttgo->tft->drawNumber(hh, xpos, ypos, 7);
  xpos += 3;
  xpos += ttgo->tft->drawChar(':', xpos, ypos, 7);
  if (mm < 10) xpos += ttgo->tft->drawChar('0', xpos, ypos, 7);
  ttgo->tft->drawNumber(mm, xpos, ypos, 7);
/*
//battery
  ttgo->tft->setTextColor(TFT_GREEN); //TFT_GREENYELLOW
  per = ttgo->power->getBattPercentage();
  ttgo->tft->drawString(String(per) + "%", 204, 2, 2);*/
//date
  ttgo->tft->setTextSize(2);
  ttgo->tft->setTextColor(TFT_PURPLE, TFT_BLACK); //0x7FF cyan, 0xFBE0 orange
  ttgo->tft->drawString((String(mmonth) + "/" + String(dday) + "/" + String(yyear)), 65, 122, 1); //print the date
//https://ai.thestempedia.com/docs/evive/evive-arduino-ide-tutorials/how-to-draw-shapes-on-tft-display/ 
//device connected
 /* if(deviceConnected)
    ttgo->tft->fillCircle(195, 10, 3, TFT_GREEN); //fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
  else
    ttgo->tft->fillRect(191, 5, 8, 15, TFT_BLACK); //x, y, w, h
*/
  drawNotificationsBar();
  //if(ttgo->getTouch(x,y)){ //button is x=130, y=210
  // Serial.print("x: ");Serial.print(x);Serial.print(" y: ");Serial.print(y);Serial.println(); //test touch
  //}
  
  //check for swipes
  /*if(swipeRight()){
    Serial.println("Swipe right");
  }
  if(swipeLeft()){
    Serial.println("Swipe left");
  }
  if(swipeUp()){
    Serial.println("Swipe up");
  }
  if(swipeDown()){
    Serial.println("Swipe down");
  }*/
  /*if(offBut()){
    lwEnergy();
  }*/
/*
  if(notification){
    notifyPopup(showNotification(), 0);
  }*/

  if(touchRight()){ //multimter screen
    Serial.println("Right Button");
    appDMM();
    setupSetupGUI(); //setup homescreen on exit
  }

  if(touchLeft()){ //notification screen
    Serial.println("Left Button");
    appNotifyTab(); //changeeee
    setupSetupGUI(); //counter = 0;?
    //for(int i=0; i<10; i++){ //test notification storage array
    //  Serial.println(notifyList[i]); 
    //  delay(1000);
    //}
  }

  if(touchUp()){ //notification screen
    Serial.println("Up Button");
    appTVbGone();
    setupSetupGUI();
  }

  //silent button
  if(silentBut()){
    Serial.println("Silented pressed"); //is silented a word?
    silented = !silented; //equals opposite
  }
  if(!silented){ //if not silented
    ttgo->tft->fillRoundRect(3, 3, 30, 30, 2, TFT_DARKCYAN); //x, y, w, h, radius
  }
  else{ //if it is silented
    //ttgo->tft->fillRoundRect(3, 3, 30, 30, 2, TFT_DARKCYAN); //x, y, w, h, radius
    ttgo->tft->fillRoundRect(5, 5, 26, 26, 2, TFT_BLACK); //hollow square
  }

  //getMostRecentNotification();
  //notifyBox(int x, int y, String text);
  if(!notifyListEmpty())notifyBox(45, 170, getMostRecentNotification()); //make box stay with most recent notification


  currentTime = millis();
  while(millis() < currentTime+100){} //delay for buttons (sus debouncer)
}//end gui


void drawNotificationsBar(){
  //battery
  ttgo->tft->fillRect(200, 1, 20, 20, TFT_BLACK); //x, y, w, h
  ttgo->tft->setTextSize(1);
  ttgo->tft->setTextColor(TFT_GREEN, TFT_BLACK); //TFT_GREENYELLOW
  per = ttgo->power->getBattPercentage();
  ttgo->tft->drawString(String(per) + "%", 204, 2, 2);

  //https://ai.thestempedia.com/docs/evive/evive-arduino-ide-tutorials/how-to-draw-shapes-on-tft-display/ 
//device connected
  if(deviceConnected) //does not show up a second time
    ttgo->tft->fillCircle(195, 10, 3, TFT_GREEN); //fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
  else
    ttgo->tft->fillRect(191, 5, 8, 15, TFT_BLACK); //x, y, w, h
} //change



//middle: x:100-130 y:100-140
//right:x:0-70 y: 90-140
//left: x:190-230 y:90-140
//up: x:110-150 y:0-40
//down: x:110-150 y: 180-230
bool swipeRight(){ //get start point of touch. compare to end point. if the points are connected, that function was executed
  if(millis() > currentTime+1000){ //more than 1s elapsed since currentTime
    swipped = false; //timout of 1000ms for swipe
  }
  if(ttgo->getTouch(x,y)){ //middle of watch: x from 100-120 : y from 100 to 120
    if((x>100 && x<130) && (y>100 && y<140)){ //if detect touch in middle starting point //watch end is x from 0-50 and y 100-120... x start is on the left
      swipped = true;
      currentTime = millis();
    }
   else if(((x>0 && x<70) && (y>90 && y<140)) && swipped){ //if detect touch in middle starting point
      return true; //swipe right is true
    }
  }
  return false;
}
bool swipeLeft(){ //left: x:190-230 y:90-140
  if(millis() > currentTime+1000){ 
    swipped = false; 
  }
  if(ttgo->getTouch(x,y)){ 
    if((x>100 && x<130) && (y>100 && y<140)){ 
      swipped = true;
      currentTime = millis();
    }
   else if(((x>190 && x<230) && (y>90 && y<140)) && swipped){ 
      return true;
    }
  }
  return false;
}
bool swipeUp(){ //up: x:110-150 y:0-40
  if(millis() > currentTime+1000){ 
    swipped = false; 
  }
  if(ttgo->getTouch(x,y)){ 
    if((x>100 && x<130) && (y>100 && y<140)){ 
      swipped = true;
      currentTime = millis();
    }
   else if(((x>110 && x<150) && (y>0 && y<50)) && swipped){ 
      return true;
    }
  }
  return false;
}
bool swipeDown(){ //down: x:110-150 y: 180-230
  if(millis() > currentTime+1000){ 
    swipped = false; 
  }
  if(ttgo->getTouch(x,y)){ 
    if((x>100 && x<130) && (y>100 && y<140)){ 
      swipped = true;
      currentTime = millis();
    }
   else if(((x>110 && x<150) && (y>180 && y<230)) && swipped){ 
      return true;
    }
  }
  return false;
}

bool touchLeft(){
  if(ttgo->getTouch(x,y)){
    if((x>0 && x<60) && (y>80 && y<160)){
      return true;
    }
  }
  return false;
}
bool touchRight(){
  if(ttgo->getTouch(x,y)){
    if((x>180 && x<240) && (y>80 && y<160)){
      return true;
    }
  }
  return false;
}
bool touchUp(){ //change
  if(ttgo->getTouch(x,y)){
    if((x>90 && x<150) && (y>0 && y<50)){
      return true;
    }
  }
  return false;
}
//left side arrow box: x:0-25, y:100-140
//right side arrow box: x:220-240, y:100-140
//up side arrow box: x:100-140, y:0-25

bool offBut(){ //button to make screen sleep. Unused, used sleep timer instead
  if(ttgo->getTouch(x,y)){
      if((x>80 && x<155) && (y>190 && y<230)){
        return true;
      }
    }
    return false;
}
//off button: x:80to155 y:190to230

bool silentBut(){
  if(ttgo->getTouch(x,y)){
      if((x>0 && x<40) && (y>0 && y<40)){
        return true;
      }
    }
    return false;
}
//off button: x:3to33 y:3to33


//notifications
void notifyPopup(String notification, int screen){ //popup notification on bottom of main screen
  //keeps most recent notification on bottom of main screen
  //bool of main screen or other screen. 0 = main screen call, 1=other side screen (DMM, etc), 2=sleeping screen
  Serial.println(notification);
  if(notificationCounter == maxNotifications){ //0to10 higher # is most recent
    notificationCounter = 0; //crashes at max (fixed by making maxNotify a constant)
    clearNotifyList();
    Serial.println("clear list");
    if(screen==3){ //clear list screen on display
      clearList();
    }
  }
  notifyList[notificationCounter] = notification;

  if(screen == 0){ //main screen
  //notify main
    Serial.println("main");
    notifyBox(45, 170, notificationData); //notification?
    //screenTimeout = 7500; //make screen stay on longer wehn there is a notifiction (??)
  }
  if(screen == 1){ //DMM
  //notify dmm
    Serial.println("dmm");
  }
  if(screen == 2){ //screen off, should turn on screen
  //call differnt function which turns screen on
  //notify off
    Serial.println("off");
  }
  if(screen == 3){ //screen notify
  //notify screen
    Serial.println("notifyScreen");
  }
  if(screen == 4){ //screen tv
  //tv-b-gone
    Serial.println("tv-b-gone");
  }
  notificationCounter++;
}

void notifyBox(int x, int y, String text){
  ttgo->tft->fillRoundRect(x, y, 150, 60, 3, TFT_DARKCYAN); //x, y, w, h, radius
  //ttgo->tft->setTextSize(2);
  ttgo->tft->setTextColor(TFT_PURPLE);
  //ttgo->tft->drawString(text, x+3, y+3, 1);
  //text loop, goes off screen
  wrapText(x+5, x+145, y+5, y+55, text, 2, 1);
}

String getMostRecentNotification(){ //crashes on notification?
  for(int i=10; i>0; i--){ //nofity overwrite issue?
    if(notifyList[i] != NULL || notifyList[i] != ""){
      return notifyList[i];
    }    
  }
  return notifyList[0];
}

bool notifyListEmpty(){
  if(getMostRecentNotification()=="" || getMostRecentNotification()==NULL){
    return true;
  }
  return false;
}

void clearNotifyList(){
  for(int i=0; i<maxNotifications; i++){
    notifyList[i] = "";
  }
}

//notifybox: 150x60
//I only did this for size2 and font1
void wrapText(int xStart, int xEnd, int yStart, int yEnd, String text, int textSize, int textFont){
  //text loop to certain area 
  ttgo->tft->setTextSize(textSize); //textsize=2 and font=1: char max size is about x:10 y:12
  //ttgo->tft->drawString(text, x+3, y+3, textFont);
  //int textLength = text.length() + 1; //https://circuits4you.com/2018/03/08/arduino-convert-string-to-character-array/
  //char char_array[textLength];
  //text.toCharArray(char_array, textLength);
  int xVal = (xEnd-xStart)/12;
  int yVal = (yEnd-yStart)/16; //((170+55)-(170+5))/16=3.125 -> 3 lines
  if(text.length() > xVal){
    //if too long for box wrap 
    int sub=0;
    for(int i=0; i<yVal; i++){
      ttgo->tft->drawString(text.substring(sub,sub+xVal), xStart, yStart+(i*15), textFont); //print first x characters of array      
      sub= sub+xVal; //increase through the substring  
    }
  }
  else{
    ttgo->tft->drawString(text, xStart+3, yStart+3, textFont);
  }
}
//end notifications





//bluetooth disconnects after 30min
void idleBLE(int screen) { //should be constaly running to maintain BLE connection
    if (deviceConnected) {
    //Serial.print("connected");
    if (id < 512) {
      if (id >= 510) {
        pCharacteristicTX->setValue(type, 20);
        pCharacteristicTX->notify();
        delay(10);
      }
      id++;
    }
  } else {
    //Serial.println("Disconnected");
  }

  long cur = millis(); //current time
  if (cur < timer + timeout) { //timeout set above (5s). time the feed shows messages
    if(isNotification() && (showNotification() != notificationData)){ //show notification once when it arrives, not spam it a bunch
      notificationData = showNotification();
      //Serial.println(notificationData); // show notification
      notification = true;
      notifyScreen = screen;
      if(!silented)quickDualBuzz(); //buzz motor if unsilented
      notifyPopup(notificationData, screen);
    }
  }  
  else {
    //printLocalTime();   // display time --comment
    notification = false;
  }

}







/*
//Draw the back graphics - Top of display
    ttgo->tft->fillScreen(TFT_BLACK);
    ttgo->tft->fillRoundRect(0, 0, 239, 120, 40, TFT_PURPLE);
    ttgo->tft->fillRect(215, 0, 24, 20, TFT_DARKCYAN);
    ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
    ttgo->tft->drawString("Temp", 66, 158, 2);

    // Display Temp Marker - you may need to adjust the x value based on your normal ADC results
    float temp = ttgo->power->getTemp();
    ttgo->tft->fillRoundRect(int(temp) - 20, 170, 10, 20, 5, TFT_WHITE);

    // Display Time
    // Font 7 is a 7-seg display but only contains
    // characters [space] 0 1 2 3 4 5 6 7 8 9 0 : .

    ttgo->tft->setTextColor(0xFBE0, TFT_BLACK);
    int xpos = 55;
    if (hh < 10) xpos += ttgo->tft->drawChar('0', xpos, 35, 7);
    xpos += ttgo->tft->drawNumber(hh, xpos, 35, 7);
    xpos += 3;
    xpos += ttgo->tft->drawChar(':', xpos, 35, 7);
    if (mm < 10) xpos += ttgo->tft->drawChar('0', xpos, 35, 7);
    ttgo->tft->drawNumber(mm, xpos, 35, 7);

    // Display Battery Level
    ttgo->tft->setTextSize(1);
    ttgo->tft->setTextColor(TFT_YELLOW);
    ttgo->tft->drawString("Power", 124, 2, 2);
    ttgo->tft->setTextColor(TFT_GREEN);
    int per = ttgo->power->getBattPercentage();
    per = ttgo->power->getBattPercentage();
    ttgo->tft->drawString(String(per) + "%", 179, 2, 2);
    ttgo->tft->setTextColor(TFT_GREENYELLOW);
    ttgo->tft->drawString(String(dday), 50, 188, 6);
    

   // Build a bargraph every 10 seconds
  int secmod = ss % 10;
  if (secmod) { // Show growing bar every 10 seconds
    ttgo->tft->fillRect(126 + secmod * 10, 215, 6, 15, TFT_ORANGE);
  } else {
    ttgo->tft->fillRoundRect(119, 210, 120, 29, 15, TFT_DARKCYAN);
  }
  
  */
