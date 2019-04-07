/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>
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

static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;

ESP8266WiFiMulti WiFiMulti;
SoftwareSerial co2Serial(D2, D1); // RX, TX

byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte response[9]; // for answer

int readCO2() {
  int ppm = -1;
  co2Serial.write(cmd, 9);
  delay(100);  
  
  if (co2Serial.available()) {
    Serial.println("receiving data from sensor...");
    memset(response, 0, 9);
    co2Serial.readBytes(response, 9);  
    Serial.println("CO2 response: ");
    for (int i = 0; i < 9; i++) {
      Serial.print(response[i], HEX);
    }
    Serial.println();
    if (response[1] == 0x86) {
    Serial.println("response to 0x86, calculating CRC...");
    byte crc = 0;
    for (int i = 1; i < 8; i++) {
      crc += response[i];
    }
    crc = 255 - crc + 1;
    Serial.printf("calculated CRC: %d\n",crc);
    Serial.printf("received CRC: %d\n",response[8]);

    if (response[8] == crc) {
      int responseHigh = (int) response[2];
      int responseLow = (int) response[3];
      ppm = (256 * responseHigh) + responseLow; 
      Serial.printf("received PPM: %d\n",ppm);
    } else {
      Serial.println("CRC error!");    
    }
  }
  else
    Serial.println("Invalid response from co2 sensor!");
    
  Serial.println();
  Serial.println();
  Serial.println("----------------");
  }
  return ppm;
}


void blinkLed(int dlay){ 
    digitalWrite(LED_BUILTIN, LOW);                                   
    delay(dlay);
    digitalWrite(LED_BUILTIN, HIGH);    
}

void logOH(String message){ 
    HTTPClient http;
    http.begin("http://192.168.178.9:8080/rest/items/ESP8266Logger/state"); //HTTP
    http.PUT(message);
}

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

    blinkLed(1000);

    Serial.begin(115200);
    //Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_NAME,WIFI_PASSWORD);  

    co2Serial.begin(9600);
}


void loop() {

    //blinkLed(250);

    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

       logOH("WIFI Connected...");
       //blinkLed(500);
       //delay(1000);
       //blinkLed(500);


        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        logOH("[HTTP] begin...");

        http.begin("http://192.168.178.9:8080/rest/items/ESP8266CO2/state"); //HTTP

        Serial.print("[HTTP] PUT...\n");
        logOH("[HTTP] PUT...");
        // start connection and send HTTP header
        String co2 = String(readCO2());
        logOH(co2);
        int httpCode = http.PUT(co2);

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] PUT... code: %d\n", httpCode);            
            
            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println(payload);
                logOH(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            logOH(http.errorToString(httpCode).c_str());            
        }

        http.end();
    }

    delay(10000);
}

