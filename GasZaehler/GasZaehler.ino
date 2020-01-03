
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>


/*
 * content of WIFI-credentials.h
 * #define WIFI_NAME "your WIFI name"
 * #define WIFI_PASSWORD "your WIFI password"
*/
#include "WIFI-credentials.h"

ESP8266WiFiMulti WiFiMulti;

void setup() {  
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    Serial.println();
    Serial.println();
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    } 
  Serial.flush();
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_NAME,WIFI_PASSWORD);    
  while(WiFiMulti.run() != WL_CONNECTED)
    delay(10);  
  Serial.printf("[SETUP] WIFI done.");
}

void loop() {  
  putItemValue("ESP8266_GAS_ZHLR_IMPULSE",String("ON"));
  ESP.deepSleep(86400000000); //24*60*60*1000000
}

void putItemValue(String itemName, String itemValue){
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.PUT(itemValue);
  http.end();
}
