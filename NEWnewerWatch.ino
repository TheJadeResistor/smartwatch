#include <Arduino.h>
#include "config.h"
#include "gui.h"
#include "bluetooth.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include <soc/rtc.h>
#include "esp_sleep.h"
#include <esp32-hal-gpio.h>
#include "esp_intr_alloc.h"
#include "analogWrite.h"

#define DEFAULT_SCREEN_TIMEOUT  5*1000 //set screen timeout (10 seconds) <-10??? idk dont ask

//esp_now_send_status_t status;

enum {
    Q_EVENT_BMA_INT,
    Q_EVENT_AXP_INT,
} ;

TTGOClass * ttgo;
QueueHandle_t g_event_queue_handle = NULL;
EventGroupHandle_t g_event_group = NULL;
bool lenergy = false;

#define WATCH_FLAG_SLEEP_MODE   _BV(1)
#define WATCH_FLAG_SLEEP_EXIT   _BV(2)
#define WATCH_FLAG_BMA_IRQ      _BV(3)
#define WATCH_FLAG_AXP_IRQ      _BV(4)
#define G_EVENT_VBUS_PLUGIN     _BV(0)
#define G_EVENT_VBUS_REMOVE     _BV(1)
#define G_EVENT_CHARGE_DONE     _BV(2)

int last_activity, mainTime = 0;
int16_t x, y;
bool pwrBut = 0;
int counter = 0;
int motorPin = 4; //gpio4
int screenTimeout = 5000;
bool btReset = false;





//espnow-----------------------------------------------------------------------------------
//espnow
// REPLACE WITH THE MAC Address of your receiver
uint8_t broadcastAddress[] = {0xD4, 0xD4, 0xDA, 0x84, 0xAF, 0xB8}; //D4:D4:DA:84:AF:B8

String message = "";
int val = 0;
int values [383];
bool newArray = false;
int counterDMM = 0;
float R1 = 9851; //10000; //nom
float R2 = 996.1; //1000; //nom
float factor = R2/(R1+R2);
float voltage = 0.00;
//bool ESPrecieve = false;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    char msg[50];
} struct_message;

// Create a struct_message called DHTReadings to hold sensor readings
struct_message outgoingReadings;

struct_message incomingReadings;

// Variable to store if sending data was successful
String success;
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0) {
    success = "Delivery Success :)";
  }
  else {
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  if(counterDMM == 384){
    counterDMM = 0;
    newArray = false;
  }
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  message = incomingReadings.msg;
  Serial.print("Data: ");Serial.println(message); //incomingReadings.msg
  //val = (int16_t)*incomingData;
  //Serial.print("Data#: ");Serial.println(val); //if sent int
  //float voltage; //this
  /* Be sure to update this value based on the IC and the gain settings! */
  //float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */ //this
  //adc0 = ads.readADC_SingleEnded(0);s
  //voltage = (val * multiplier)/factor; //vo=vin(R2/(R1+R2)) //this
  //if(message != "-2" || message != "-2")
  voltage = (message.toFloat() * multiplier)/factor; //change so not always printing to serial
  voltage /= 1000; //mv-->V
  Serial.print("Differential: "); Serial.print(message); Serial.print("("); Serial.print(voltage); Serial.println("V)"); //this
  //ESPrecieve = true;
  //Serial.println(val);
  //Serial.println(incomingReadings.msg);
  /*if(message == "starter"){
    message = "";
    newArray = true;
    counterDMM = 0;
    Serial.println("Start new array!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }
  else if(newArray){
    val = (int)*incomingData;
    values[counterDMM] = val;
    Serial.println(val);
    counterDMM++;
  }*/
  //counterDMM++;
  //Serial.println(counterDMM);
} 
//espnow
//espnow-----------------------------------------------------------------------------------





void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  initBLE();

  ttgo = TTGOClass::getWatch(); //setup watch
  ttgo->begin();
  ttgo->lvgl_begin();

  //Create a program that allows the required message objects and group flags
  g_event_queue_handle = xQueueCreate(20, sizeof(uint8_t));
  g_event_group = xEventGroupCreate();

  // Turn on the IRQ used (power managment chip setup)
  ttgo->power->adc1Enable(AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1, AXP202_ON);
  ttgo->power->enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_FINISHED_IRQ, AXP202_ON);
  ttgo->power->clearIRQ();

// Enable BMA423 interrupt ，
  // The default interrupt configuration,
  // you need to set the acceleration parameters, please refer to the BMA423_Accel example
  ttgo->bma->attachInterrupt();

  pinMode(BMA423_INT1, INPUT); //gyro managment
    attachInterrupt(BMA423_INT1, [] {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        uint8_t data = Q_EVENT_BMA_INT;
        xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR ();
        }
    }, RISING);

    // Connection interrupted to the specified pin
    pinMode(AXP202_INT, INPUT); //pwr managment
    attachInterrupt(AXP202_INT, [] { //this interrupt crashes my watch so I made a new interrupt thing
    ///*
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        uint8_t data = Q_EVENT_AXP_INT;
        xQueueSendFromISR(g_event_queue_handle, &data, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR ();
        }//*/
        Serial.println("AXP202_INT interupt triggered");
        pwrBut = true;
    }, FALLING);

  //Check if the RTC clock matches, if not, use compile time
  ttgo->rtc->check(); //default to RTC if no phone is connected
  ttgo->rtc->syncToSystem(); //Synchronize time to system time

  setupSetupGUI(); //setup the gui (in gui.cpp)
  setupGui();
  //ttgo->closeBL();
  lv_disp_trig_activity(NULL); //activity trigger declaration
  ttgo->tft->fillScreen(TFT_BLACK); //background color
  //ttgo->openBL(); //turn on backlight
  pinMode(motorPin, OUTPUT);
  esp_err_t esp_bt_controller_init(ESP_BT_MODE_CLASSIC_BT); //esp_err_t esp_bt_controller_init(BT);
  //ESP_BT_MODE_BLE, ESP_BT_MODE_CLASSIC_BT

  setupESP();

  mainTime = millis();
}

void loop() {
  //if(btReset){
    //esp_err_t esp_bt_controller_init(ESP_BT_MODE_BLE);
  //  esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);   //ESP_BT_MODE_BLE, ESP_BT_MODE_CLASSIC_BT
  //  btReset = false;
  //  Serial.print("Bluetooth ON");
  //}
  checkCharge();
  sleep();
  if(counter != 0){
    //GUI one-time setup on screen on
    setupSetupGUI();
    counter = 0;
    Serial.println("RISE");
  }
  setupGui();
  //watch ble disconnecting after 20min
  restartCheck();
  //voltage = 0;

  //more loop code
}

void lwEnergy(){
  idleBLE(2);
  //Serial.println("low energy");
  lenergy = true;
  ttgo->closeBL(); //turn off backlight
  //Serial.println("Low Energy Mode"); //comment
  gpio_wakeup_enable ((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable ((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
  esp_sleep_enable_gpio_wakeup (); //Modem-sleep?
  //setCpuFrequencyMhz(40); //low pwr clock speed - makes BLE not work
}

void my_idle(void) {
  last_activity = millis()+1000;
}

void checkCharge(){
  bool  rlst;
  uint8_t data;
  if (xQueueReceive(g_event_queue_handle, &data, 5 / portTICK_RATE_MS) == pdPASS) {
      switch (data) {
      case Q_EVENT_AXP_INT:
          ttgo->power->readIRQ();
          if (ttgo->power->isVbusPlugInIRQ()) {
            Serial.println("Watch plugged in");
            //watch is plugged in -- change this
          }
          if (ttgo->power->isVbusRemoveIRQ()) {
            Serial.println("Watch unplugged");
            //watch is unplugged -- change this
          }
          if (ttgo->power->isChargingDoneIRQ()) {
            Serial.println("Charging done");
            //watch charging is done -- change this
          }
          if (ttgo->power->isPEKShortPressIRQ()) {
            Serial.println("PEK short press");
            ttgo->power->clearIRQ();
            lwEnergy(); //change this 
            return;
          }
          ttgo->power->clearIRQ();
          break;
      default:
          break;
      }
  }
}


void sleep(){
  while(lv_disp_get_inactive_time(NULL) > screenTimeout){ //DEFAULT_SCREEN_TIMEOUT //this loop turns the watch off
    //if the screen is on for x time or the button is pressed
    //Serial.println(lv_disp_get_inactive_time(NULL));
    lv_task_handler(); 
    lwEnergy();
    counter++; //huh?
    restartCheck();
  }
}

void restartCheck(){ //android will not repair after forced disconnect
  if(millis() > mainTime+60000){ //reset watch after 15min 900000 120000
    //Serial.println("Reset Bluetooth");
    Serial.println("1 min timer");
    btReset = true;
    //ESP.restart(); //works but loses stored notifications
    //Serial.println("aye yo?");
    //deinitBLE(); //change
    //while(millis() < mainTime + 100){} //wait to turn BT back on
    //initBLE();

    //esp_bt_controller_disable(); //esp_bt_controller_enable(BT);
    //esp_bt_controller_deinit();
    //BLEDevice::deinit(true);
    //esp_bt_controller_deinit(); //esp_err_t esp_bt_controller_deinit();
    //btStop(); //no
    //mainTime = millis();
    //while(millis() < mainTime + 10000){} //wait to turn BT back on
    //esp_err_t esp_bt_controller_init(ESP_BT_MODE_CLASSIC_BT);
    //esp_err_t esp_bt_controller_init(ESP_BT_MODE_CLASSIC_BT); 
    //esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);   //ESP_BT_MODE_BLE, ESP_BT_MODE_CLASSIC_BT, ESP_BT_MODE_BTDM
    //esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT); //Doesnt work: fix the issue of bluetooth disconnecting after ~10min
    //esp_err_t esp_bt_controller_init(ESP_BT_MODE_CLASSIC_BT);
    //btStart(); //no
    //BLEDevice::deinit(false);
    //Serial.println("Bluetooth ON");
    mainTime = millis();
  }
  //esp_err_t esp_bt_controller_init(ESP_BT_MODE_CLASSIC_BT);
  //esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);   //ESP_BT_MODE_BLE, ESP_BT_MODE_CLASSIC_BT
}

void quickDualBuzz(){ //buzz motor quickly twice
  //motorPin
  digitalWrite(motorPin, HIGH); //on

  int motorTime = millis();
  while(millis() < motorTime+200){} //what happens after int time
  digitalWrite(motorPin, LOW); //off

  motorTime = millis();
  while(millis() < motorTime+100){}
  digitalWrite(motorPin, HIGH); //on

  motorTime = millis();
  while(millis() < motorTime+200){}
  digitalWrite(motorPin, LOW); //off

  //int curTime1 = millis();
  //while(millis() < curTime1+100){} //delay for 100ms
}


void setupESP(){
  //espnow
  // Init Serial Monitor
    //Serial.println("DMMESP start");

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA); //WiFi.mode(WIFI_STA_AT);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Register peer
    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    for (int ii = 0; ii < 6; ++ii )
    {
      peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
    }
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(OnDataRecv);
  //espnow
}

//add accelerometer wakeup thing