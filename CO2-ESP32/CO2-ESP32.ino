#include "DHTesp.h"
#include "esp_sleep.h"
#include "WIFI-credentials.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define PWMPIN 25
#define DHTPIN 23
#define LEDPIN 2

DHTesp dht;

unsigned long th, tl,ppm, ppm2, ppm3, p1, p2, tpwm = 0;

const uint32_t SLEEP_DURATION = 20 * 1000000; // µs

const String vCO2 = "ESP32CO23";
const String vHum = "ESP32Humidity3";
const String vTemp = "ESP32Temperature3";
const String vLog = "ESP32Logger3"; 


void lightSleep() {
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION);
    delay(100);
    esp_light_sleep_start();
}

void connectToNetwork() {
  if(WiFi.status() != WL_CONNECTED){  
    WiFi.begin( WIFI_NAME, WIFI_PASSWORD); 
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Establishing connection to WiFi..");
    } 
    Serial.println("Connected to network"); 
  }
}

int readCO2PWM(){ 
  int ppm = -1;
  do {
    th = pulseIn(PWMPIN, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm = 5000 * (th-2)/(th+tl-4);
  } while (th == 0);
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
  digitalWrite(LEDPIN, LOW);                                   
}

void ledOff(){
  digitalWrite(LEDPIN, HIGH);                                   
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
  pinMode(LEDPIN, OUTPUT);
  dht.setup(DHTPIN, DHTesp::AM2302);    
}

void loop() { // run over and over 
  ledOn(); 
  connectToNetwork();
  log("awaken");
  int ppmCO2 = readCO2PWM();
  putItemValue(vCO2,String(ppmCO2));
  log("reading DHT");
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() == 0) {
    putItemValue(vHum,String(newValues.humidity));
    putItemValue(vTemp,String(newValues.temperature));
  } else
    log("DHT READ FAILED...");     
  
  log("zzZZ");
  ledOff();
  lightSleep();
}




