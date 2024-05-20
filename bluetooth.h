//https://github.com/fbiego/ESP32_OLED_BLE use Chronos: edited by TheJadeResistor
#include "config.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <ESP32Time.h>

#define BUILTINLED 2
//BLEDevice::deinit(false);
uint8_t type[] = {0xAB, 0x00, 0x11, 0xFF, 0x92, 0xC0, 0x01, 0x01, 0x38, 0x81, 0x10, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xA2, 0x00, 0x80};  // id for DT78 app

#define SERVICE_UUID              "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_RX    "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define CHARACTERISTIC_UUID_TX    "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

static BLECharacteristic* pCharacteristicTX;
static BLECharacteristic* pCharacteristicRX;

ESP32Time rtc;

extern uint8_t SmallFont[], MediumNumbers[];

static bool deviceConnected = false;
static int id = 0;
long timeout = 2000, timer = 0, scrTimer = 0;
bool rotate = false, flip = false, hr24 = true, notify = true;
int scroll = 0, bat = 0, lines = 0, msglen = 0;
char msg[126];
String msg0, msg1, msg2, msg3, msg4, msg5;
bool notification = false;
String notificationData = "";
int notifyCounter = 0;
int notifyScreen = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Connected");
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      id = 0;
      Serial.println("Disconnected");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {

    void onStatus(BLECharacteristic* pCharacteristic, int s, uint32_t code) {
      Serial.print("Status ");
      Serial.print(s);
      Serial.print(" on characteristic ");
      Serial.print(pCharacteristic->getUUID().toString().c_str());
      Serial.print(" with code ");
      Serial.println(code);
    }

    void onNotify(BLECharacteristic *pCharacteristic) {
      uint8_t* pData;
      std::string value = pCharacteristic->getValue();
      int len = value.length();
      pData = pCharacteristic->getData();
      if (pData != NULL) {
        Serial.print("Notify callback for characteristic ");
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(" of data length ");
        Serial.println(len);
        Serial.print("TX  ");
        for (int i = 0; i < len; i++) {
          Serial.printf("%02X ", pData[i]);
        }
        Serial.println();
      }
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t* pData;
      std::string value = pCharacteristic->getValue();
      int len = value.length();
      pData = pCharacteristic->getData();
      if (pData != NULL) {
        /*Serial.print("Write callback for characteristic "); //notification info hex and stuff
        Serial.print(pCharacteristic->getUUID().toString().c_str());
        Serial.print(" of data length ");
        Serial.println(len);
        Serial.print("RX  ");
        for (int i = 0; i < len; i++) {
          Serial.printf("%02X ", pData[i]);
        }*/ //comment
        //Serial.println();

        if (pData[0] == 0xAB) {
          switch (pData[4]) {
            case 0x93:
              rtc.setTime(pData[13], pData[12], pData[11], pData[10], pData[9], pData[7] * 256 + pData[8]);
              break;
            case 0x7C:
              hr24 = pData[6] == 0;
              break;
            case 0x78:
              rotate = pData[6] == 1;
              break;
            case 0x74:
              flip = pData[10] == 1;
              break;
            case 0x7B:
              if (pData[6] >= 5 && pData[6] <= 30) {
                timeout = pData[6] * 1000;
              }

              break;
            case 0x23:
              //screenOff = pData[6] == 1;
              break;
            case 0x91:
              bat = pData[7];
              break;
            case 0x72:
              timer = millis();
              msglen = pData[2] - 5;
              lines = ceil(float(msglen) / 21);
              scroll = 0;
              msg[msglen] = 0;
              //scrOff = false;
              if (pData[6] == 1) {
                //call
                timer = millis() + 15000;
                for (int x = 0; x < len; x++) {
                  msg[x] = char(pData[x + 8]);
                }
              } else if (pData[6] == 2) {
                //cancel call
                timer = millis() - timeout;
                //scrOff = true;
              } else {
                //notification
                for (int x = 0; x < len; x++) {
                  msg[x] = char(pData[x + 8]);
                }

              }
              break;
          }

        } else {
          switch (pData[0]) {
            case 0:
              for (int x = 0; x < len - 1; x++) {
                msg[x + 12] = char(pData[x + 1]);
              }
              break;
            case 1:
              for (int x = 0; x < len - 1; x++) {
                msg[x + 31] = char(pData[x + 1]);
              }
              break;
            case 2:
              for (int x = 0; x < len - 1; x++) {
                msg[x + 50] = char(pData[x + 1]);
              }
              break;
            case 3:
              for (int x = 0; x < len - 1; x++) {
                msg[x + 69] = char(pData[x + 1]);
              }
              break;
            case 4:
              for (int x = 0; x < len - 1; x++) {
                msg[x + 88] = char(pData[x + 1]);
              }
              break;
            case 5:
              for (int x = 0; x < len - 1; x++) {
                msg[x + 107] = char(pData[x + 1]);
              }
              break;
          }
        }


      }

    }


};

void initBLE() {
  BLEDevice::deinit(false);
  BLEDevice::init("Esp32 Watch");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristicTX = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY );
  pCharacteristicRX = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  pCharacteristicRX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->setCallbacks(new MyCallbacks());
  pCharacteristicTX->addDescriptor(new BLE2902());
  pCharacteristicTX->setNotifyProperty(true);
  pService->start();


  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void deinitBLE(){
  //change!!
  //BLEDevice::stopAdvertising();
  BLEDevice::deinit(true);
}

void printLocalTime() {
  if (!hr24) {
    Serial.println(rtc.getAmPm(true)); //causes weird formatting
  }
  Serial.print(rtc.getTime());Serial.println(rtc.getTime(" %a %d %b")); //print time then date

  if (deviceConnected) {
    //Serial.println("Connected");
  }
}

void copyMsg(String ms) {
  switch (lines) {
    case 1:
      msg0 = ms.substring(0, msglen);
      msg1 = "";
      msg2 = "";
      msg3 = "";
      msg4 = "";
      msg5 = "";
      break;
    case 2:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, msglen);
      msg2 = "";
      msg3 = "";
      msg4 = "";
      msg5 = "";
      break;
    case 3:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, msglen);
      msg3 = "";
      msg4 = "";
      msg5 = "";
      break;
    case 4:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, 63);
      msg3 = ms.substring(63, msglen);
      msg4 = "";
      msg5 = "";
      break;
    case 5:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, 63);
      msg3 = ms.substring(63, 84);
      msg4 = ms.substring(84, msglen);
      msg5 = "";
      break;
    case 6:
      msg0 = ms.substring(0, 21);
      msg1 = ms.substring(21, 42);
      msg2 = ms.substring(42, 63);
      msg3 = ms.substring(63, 84);
      msg4 = ms.substring(84, 105);
      msg5 = ms.substring(105, msglen);
      break;
  }
}

String showNotification() {
  String s = String(msg);
  copyMsg(s);
  if (s.length() == 0) {
    msg0 = "No notifications"; //prints out a lot-not needed
    //printLocalTime(); //has timeout already
  }
  return msg0+msg1+msg2+msg3+msg4+msg5; //return the notification
}

bool isNotification(){
  if(!(showNotification() == "NULL")){
    return true;
  }
  return false;
}

String getNotification(){ 
}
