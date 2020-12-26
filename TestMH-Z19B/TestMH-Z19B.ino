
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

/*
 * content of WIFI-credentials.h
 * #define WIFI_NAME "your WIFI name"
 * #define WIFI_PASSWORD "your WIFI password"
*/
#include "WIFI-credentials.h"

#define D0  16
#define D1  5
#define D2  4
#define D3  0
#define D4  2
#define D5  14
#define D6  12
#define D7  13
#define D8  15
#define D9  3
#define D10 1

#define PWM1 D5
#define PWM2 D6
#define PWM3 D7
#define CO2_PWR D8

const int delayInSeconds = 900; //15 min

ESP8266WiFiMulti WiFiMulti;

void setup() {  
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_NAME,WIFI_PASSWORD);    
  while(WiFiMulti.run() != WL_CONNECTED)
    delay(10);  

  pinMode(PWM1, INPUT);
  pinMode(PWM2, INPUT);
  pinMode(PWM3, INPUT);
  pinMode(CO2_PWR, OUTPUT);
  digitalWrite(CO2_PWR, LOW);
}

void loop() {  
  log2("Starting...");
  monitorCO2();
  log2("Sleeping for 15 minutes...");
  delay(delayInSeconds*1000);
}

void monitorCO2(){
  powerOn();
  log2("waiting 120 sec...");
  for(uint8_t t = 24; t > 0; t--) {
    delay(5000);
    log2(String(t));
  }
  log2("reading D5...");
  putItemValue("MM_D5",String(readCO2PWM(PWM1)));
  log2("reading D6...");
  putItemValue("MM_D6",String(readCO2PWM(PWM2)));
  log2("reading D7...");
  putItemValue("MM_D7",String(readCO2PWM(PWM3)));
  log2("reading done.");
  powerOff();
}

void powerOn(){
  log1("power on");
  digitalWrite(CO2_PWR, HIGH);
}

void powerOff(){  
  digitalWrite(CO2_PWR, LOW);
  log1("power off");
}

void putItemValue(String itemName, String itemValue){
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.PUT(itemValue);
  http.end();
}

void log1(String msg){
  putItemValue("MM_LOG1",msg);
}

void log2(String msg){
  putItemValue("MM_LOG2",msg);
}

int readCO2PWM(uint8_t pin)
{ 
  unsigned long th, tl, ppm = 0;
  do {
    th = pulseIn(pin, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm = 5000 * (th-2)/(th+tl-4);
  } while (th == 0);
  return ppm;
}
