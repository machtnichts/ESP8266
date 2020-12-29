#include "DHTesp.h"
#include "esp_sleep.h"
#include "WIFI-credentials.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <esp_bt.h>
#include <esp_task_wdt.h>

#define PWMPIN 25
#define DHTPIN 23

#define BOARD_NUMBER "1"

const String FIRMWARE_VERSION = "v1.07dslp";

DHTesp dht;

unsigned long th, tl,ppm, ppm2, ppm3, p1, p2, tpwm = 0;

const uint32_t SLEEP_DURATION = 60 * 1000000; // µs

const String vCO2  = String("ESP32CO2")+String(BOARD_NUMBER);
const String vHum  = String("ESP32Humidity")+String(BOARD_NUMBER);
const String vTemp = String("ESP32Temperature")+String(BOARD_NUMBER);
const String vLog  = String("ESP32Logger")+String(BOARD_NUMBER);

void connectToNetwork() {
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin( WIFI_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);      
    }
  }
}

int readCO2PWM(){  
  int ppm = -1;
  int counter = 1000;
  ledOn();
  do {    
    counter--;
    th = pulseIn(PWMPIN, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm = 5000 * (th-2)/(th+tl-4);
    if(th==0){
      ppm=-1;
      ledOff();
      delay(250);
      ledOn();
    }
  } while (th == 0 && counter>0);
  ledOff();
  return ppm;
}

void putItemValue(String itemName, String itemValue){
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.PUT(itemValue);
}

void log(String message){
  putItemValue(vLog,message);
}

void ledOn(){
  digitalWrite(LED_BUILTIN, HIGH);  
}

void ledOff(){
  digitalWrite(LED_BUILTIN, LOW); 
}

void setup() {
  esp_task_wdt_init(5,true);
  esp_task_wdt_add(NULL);
  pinMode(PWMPIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);  
  dht.setup(DHTPIN, DHTesp::AM2302);
  esp_task_wdt_reset();
  sendValues();
  esp_task_wdt_reset();
  deepSleep();
}

void sendValues(){ 
  connectToNetwork();
  log("awaken");
  int ppmCO2 = readCO2PWM();
  if(ppmCO2>0 && ppmCO2<=5000)  
    putItemValue(vCO2,String(ppmCO2));
  else
    log("PWM READ FAILED: "+ppmCO2);
  log("reading DHT");
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() == 0) {
    putItemValue(vHum,String(newValues.humidity));
    putItemValue(vTemp,String(newValues.temperature));
  } else
    log("DHT READ FAILED...");
  
  log("zzZZ ("+FIRMWARE_VERSION+")");
}


void loop() {
}

void deepSleep() {
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
  esp_wifi_stop();
  esp_bt_controller_disable();
  delay(100);
  esp_deep_sleep_start();
}
