#include "DHTesp.h"
#include "esp_sleep.h"
#include "WIFI-credentials.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <esp_bt.h>

#define PWMPIN 25
#define DHTPIN 23

const String FIRMWARE_VERSION = "v1.04dlay";

DHTesp dht;

unsigned long th, tl,ppm, ppm2, ppm3, p1, p2, tpwm = 0;

const uint32_t SLEEP_DURATION = 30 * 1000000; // µs

const String vCO2 = "ESP32CO21";
const String vHum = "ESP32Humidity1";
const String vTemp = "ESP32Temperature1";
const String vLog = "ESP32Logger1";

void connectToNetwork() {
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin( WIFI_NAME, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Establishing connection to WiFi..");
      delay(1000);      
    }
    Serial.println("Connected to network");
  }
}

int readCO2PWM(){  
  int ppm = -1;
  ledOn();
  do {    
    th = pulseIn(PWMPIN, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm = 5000 * (th-2)/(th+tl-4);
    if(th==0){
      ledOff();
      delay(250);
      ledOn();
    }
  } while (th == 0);
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
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
    
  connectToNetwork();
  pinMode(PWMPIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  dht.setup(DHTPIN, DHTesp::AM2302);   
}

void sendValues(){
  //ledOn(); 
  connectToNetwork();
  Serial.println("Connected.");
  log("awaken");
  Serial.println("Reading CO2");
  int ppmCO2 = readCO2PWM();
  putItemValue(vCO2,String(ppmCO2));
  log("reading DHT");
  Serial.println("Reading DHT"); 
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() == 0) {
    putItemValue(vHum,String(newValues.humidity));
    putItemValue(vTemp,String(newValues.temperature));
  } else
    log("DHT READ FAILED...");       
  
  log("zzZZ ("+FIRMWARE_VERSION+")");
  //ledOff();
  Serial.println("Sleeping");
}


void loop() {
  //ledOn();   // turn the LED on (HIGH is the voltage level)
  delay(2000);                       // wait for a second
  sendValues();
  //ledOff();    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
}

void loop1(){ // run over and over 
 sendValues();  
 delay(30000);
}
/*
void deepSleep() {
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
  esp_wifi_stop();
  esp_bt_controller_disable();
  delay(100);
  esp_deep_sleep_start();
}

void lightSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    delay(100);
    esp_light_sleep_start();
}
*/
