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

/*
 * content of WIFI-credentials.h
 * #define WIFI_NAME "your WIFI name"
 * #define WIFI_PASSWORD "your WIFI password"
*/
#include "WIFI-credentials.h"

#define PWMPIN 5

ESP8266WiFiMulti WiFiMulti;



unsigned long th, tl,ppm, ppm2, ppm3, p1, p2, tpwm = 0;

int readCO2() {
  digitalWrite(LED_BUILTIN, LOW);                                   
  do {
    th = pulseIn(PWMPIN, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm2 = 2000 * (th-2)/(th+tl-4);
    ppm3 = 5000 * (th-2)/(th+tl-4);
  } while (th == 0);

//  th = pulseIn(PWMPIN, HIGH, 3000000);
//  unsigned long ppm4 = 5000 * (th-p1)/(tpwm-p2);
  
  Serial.printf("th: %d\n",th);
  Serial.printf("ppm2: %d\n", ppm2);
  Serial.printf("ppm3: %d\n", ppm3);
 // Serial.printf("ppm4: %d\n", ppm4);
  Serial.println("-----------");

      
  digitalWrite(LED_BUILTIN, HIGH);
  return ppm3;
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
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(WIFI_NAME,WIFI_PASSWORD);  
    pinMode(PWMPIN, INPUT);

//    th = pulseIn(PWMPIN, HIGH, 3000000); // use microseconds
//    tl = pulseIn(PWMPIN, LOW, 3000000);
//    tpwm = th + tl; // actual pulse width
//    Serial.print("PWM-time: ");
//    Serial.print(tpwm);
//    Serial.println(" us");
//    p1 = tpwm/502; // start pulse width
//    p2 = tpwm/251; // start and end pulse width combined
    
}

void loop() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
        String co2 = String(readCO2());
        HTTPClient http;
        http.begin("http://192.168.178.9:8080/rest/items/ESP8266CO2/state"); //HTTP               
        int httpCode = http.PUT(co2);
        Serial.printf("[HTTP] PUT response code: %d\n", httpCode);
        http.end();
    }
    delay(10000);
}

