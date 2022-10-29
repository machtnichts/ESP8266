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

#define PWMPIN D5

#include "DHTesp.h"
#define DHTPIN D6       // what digital pin the DHT22 is conected to

DHTesp dht;

ESP8266WiFiMulti WiFiMulti;
//SoftwareSerial co2Serial(D2, D1); // RX, TX

//byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
//byte response[9]; // for answer

unsigned long th, tl,ppm, ppm2, ppm3, p1, p2, tpwm = 0;

int readCO2PWM()
{ 
  int ppm = -1;
  do {
    th = pulseIn(PWMPIN, HIGH, 1004000) / 1000;
    tl = 1004 - th;
    ppm = 5000 * (th-2)/(th+tl-4);
  } while (th == 0);
//  Serial.printf("PWM: received PPM: %d\n",ppm);  
  return ppm;
}

/*
int readCO2() {  
  digitalWrite(LED_BUILTIN, LOW);  
  int ppm = -1;
  co2Serial.write(cmd, 9);
  //delay(100);  
  
  if (co2Serial.available()) {
    //Serial.println("receiving data from sensor...");
    memset(response, 0, 9);
    co2Serial.readBytes(response, 9);  
    //Serial.println("CO2 response: ");
    //for (int i = 0; i < 9; i++) {
    //  Serial.print(response[i], HEX);
    //}
    //Serial.println();
    if (response[1] == 0x86) {
    //Serial.println("response to 0x86, calculating CRC...");
      byte crc = 0;
      for (int i = 1; i < 8; i++) {
        crc += response[i];
      }
      crc = 255 - crc + 1;
      //Serial.printf("calculated CRC: %d\n",crc);
      //Serial.printf("received CRC: %d\n",response[8]);

      if (response[8] == crc) {
        int responseHigh = (int) response[2];
        int responseLow = (int) response[3];
        ppm = (256 * responseHigh) + responseLow; 
//        Serial.printf("SER: received PPM: %d\n",ppm);
      } else {
        Serial.println("CRC error!");      
        ppm=-4;
        }
    }
    else{
      Serial.println("Invalid response from co2 sensor!");
      ppm=-3;
    }  
  }
  else
    ppm=-2;
  digitalWrite(LED_BUILTIN, HIGH);
  return ppm;
}
*/

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
    Serial.println("configured WIFI.");

    //co2Serial.begin(9600);
    pinMode(PWMPIN, INPUT);

    dht.setup(DHTPIN, DHTesp::AM2302);
}

void loop() { // run over and over  
  if((WiFiMulti.run() == WL_CONNECTED)) {    
    int ppmCO2 = readCO2PWM();     
    Serial.printf("PWM read PPM: %d\n", ppmCO2);  
    if(ppmCO2>0)
    {
      unsigned long ts = millis();
      putItemValue("ESP8266CO2",String(ppmCO2));
      putItemValue("ESP8266CO2Source",String("PWM"));
      putItemValue("ESP8266TS",String(ts));
    }
    else
      putItemValue("ESP8266CO2Source",String("PWM-INVALID-VALUE"));  

    TempAndHumidity newValues = dht.getTempAndHumidity();
    if (dht.getStatus() == 0) {
      putItemValue("ESP8266Humidity",String(newValues.humidity));
      putItemValue("ESP8266Temperature",String(newValues.temperature));
    }
    else
      Serial.println("DHT READ FAILED...");   
    
  }
  else 
    Serial.println("waiting for WIFI...");   
  delay(10000);
}
/*
void loop() { // run over and over  
  if((WiFiMulti.run() == WL_CONNECTED)) {    
    int ppmCO2 = readCO2();
    Serial.printf("SER read PPM: %d\n", ppmCO2);
    if(ppmCO2>0)
    { 
      unsigned long ts = millis();   
      putItemValue("ESP8266CO2",String(ppmCO2));
      putItemValue("ESP8266CO2Source",String("Serial"));
      putItemValue("ESP8266TS",String(ts));
    }
    else   
    {
      ppmCO2 = readCO2PWM();     
      Serial.printf("PWM read PPM: %d\n", ppmCO2);  
      if(ppmCO2>0)
      {
        unsigned long ts = millis();
        putItemValue("ESP8266CO2",String(ppmCO2));
        putItemValue("ESP8266CO2Source",String("PWM"));
        putItemValue("ESP8266TS",String(ts));
      }
    }
  }
  else 
    Serial.println("waiting for WIFI...");   
  delay(10000);
}
*/

void putItemValue(String itemName, String itemValue)
{  
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.PUT(itemValue);
}

void blinkLed(int dlay){ 
    digitalWrite(LED_BUILTIN, LOW);                                   
    delay(dlay);
    digitalWrite(LED_BUILTIN, HIGH);    
}
