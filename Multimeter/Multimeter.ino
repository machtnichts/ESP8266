
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

//MUX address lines 
#define MUX_S0 D1
#define MUX_S1 D2
#define MUX_S2 D3
#define MUX_S3 D4

//defines for analog input channels
#define C0 0x00
#define C1 0x01
#define C2 0x02
#define C3 0x03
#define C4 0x04
#define C5 0x05
#define C6 0x06
#define C7 0x07
#define C8 0x08
#define C9 0x09
#define C10 0x0A
#define C11 0x0B
#define C12 0x0C
#define C13 0x0D
#define C14 0x0E
#define C15 0x0F


#define MAX_CHANNEL_COUNT 16
#define ANALOG_READ_COUNT 5

#define PWM1 D5
#define PWM2 D6
#define PWM3 D7
#define CO2_PWR D8

const int delayIn100MS = 100;

const float c33v = 3.22;

const float factorA = 5.0/512.0;

const float factor1v = c33v*1.0/1024.0;
const float factor1024 = factor1v;
const float factor2v = c33v*2.0/1024.0;
const float factor32v = c33v*3.2/1024.0;
const float factor568v = c33v*5.68/1024.0;
const float factor43v = c33v*4.3/1024.0;
const float factors[] = {
  factor1v,
  factor2v,
  factor32v,
  factor568v,
  factor43v,
  factor1024,
  factor1024,
  factor1024,
  factor1024,
  factor1024,
  factor1024,
  factor1024,
  factor1024,
  factor1024,
  factor32v,
  factor1024};

const String itemNames[] = {
  "MM_C0","MM_C1","MM_C2","MM_C3","MM_C4","MM_C5","MM_C6","MM_C7","MM_C8","MM_C9","MM_C10","MM_C11","MM_C12","MM_C13","MM_C14","MM_C15"};

ESP8266WiFiMulti WiFiMulti;

void setup() {  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
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
  
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  
  pinMode(PWM1, INPUT);
  pinMode(PWM2, INPUT);
  pinMode(PWM3, INPUT);
  pinMode(CO2_PWR, OUTPUT);
  digitalWrite(CO2_PWR, LOW);
}

void loop() {  
  log2("Starting...");
  Serial.println("--------------------------");
  monitorVoltage(); 
  monitorCO2();
  Serial.println("--------------------------");
  log2("Sleeping...");
  delay(delayIn100MS*100);
}

void monitorCO2(){
  powerOn();
  log2("waiting 120 sec...");
  for(uint8_t t = 24; t > 0; t--) {
    delay(5000);
    log2(String(t));
  }
  log2("reading values...");
  putItemValue("MM_D5",String(readCO2PWM(PWM1)));
  putItemValue("MM_D6",String(readCO2PWM(PWM2)));
  //putItemValue("MM_D7",String(readCO2PWM(PWM3)));
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

void monitorVoltage(){  
  int correction = readVoltageRaw(15);
  for(uint8_t i = 0; i<MAX_CHANNEL_COUNT; i++) {  
    int vRaw = readVoltageRaw(i);    
    vRaw = vRaw - correction;
    double vCalc = factors[i]*vRaw;
    Serial.printf("voltage[%2d]: raw %d, calculated: %f\n",i,vRaw,vCalc);
    
    putItemValue(itemNames[i],String(vCalc));
  } 
  int aRaw = readVoltageRaw(13);
  int aCalc = aRaw - 512;
  double amps = factorA*aCalc;
  putItemValue(String("MM_C13"),String(amps));
}

int readVoltageRaw(int channel){     
  changeMux(channel);
  int sum=0;
  for(int i=0;i<ANALOG_READ_COUNT;i++){        
    sum+=analogRead(A0);
  }
  return sum/ANALOG_READ_COUNT;
}

void putItemValue(String itemName, String itemValue){
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.PUT(itemValue);
  http.end();
}

void changeMux(byte channelNumber){  
  digitalWrite(MUX_S0, (channelNumber & 0x01)==0?LOW:HIGH);
  digitalWrite(MUX_S1, (channelNumber & 0x02)==0?LOW:HIGH);
  digitalWrite(MUX_S2, (channelNumber & 0x04)==0?LOW:HIGH);
  digitalWrite(MUX_S3, (channelNumber & 0x08)==0?LOW:HIGH);
  delay(20);
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
