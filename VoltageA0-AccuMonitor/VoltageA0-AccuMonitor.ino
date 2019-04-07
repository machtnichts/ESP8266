
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
static const uint8_t D6   = 12;
#include "DHTesp.h"

/*
 * content of WIFI-credentials.h
 * #define WIFI_NAME "your WIFI name"
 * #define WIFI_PASSWORD "your WIFI password"
*/
#include "WIFI-credentials.h"


#define DHTPIN D6       // what digital pin the DHT22 is connected to

#define ANALOG_READ_COUNT 10

DHTesp dht;

const float factor = 16.5/783;

ESP8266WiFiMulti WiFiMulti;

void setup() {  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_NAME,WIFI_PASSWORD);  
  
  while(WiFiMulti.run() != WL_CONNECTED)
    delay(10);  
    
  // read temp / hum
  dht.setup(DHTPIN, DHTesp::AM2302);
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() == 0) {
    putItemValue("ESP8266Humidity3",String(newValues.humidity));
    putItemValue("ESP8266Temperature3",String(newValues.temperature));
  } else {
    putItemValue("ESP8266Logger32",String("AM:Temp Prob. 1 try 2"));
    delay(50);
    newValues = dht.getTempAndHumidity();
    if (dht.getStatus() == 0) {
      putItemValue("ESP8266Humidity3",String(newValues.humidity));
      putItemValue("ESP8266Temperature3",String(newValues.temperature));
    } else {
      putItemValue("ESP8266Logger32",String("AM:Temp Prob. 2"));
    }    
  }

  //read voltage
  int value = readVoltage(); 
  putItemValue("ESP8266Logger31",String(value));
  float voltage = value * factor; 
  putItemValue("ESP8266Voltage3",String(voltage));  
  unsigned long ts = millis();
  putItemValue("ESP8266TS3",String(ts));  
  putItemValue("ESP8266Logger",String("AM:ZZzz.."));
  ESP.deepSleep(60e6); // 60 seconds
}

void loop() {  
}

int readVoltage(){
  int sum=0;
  for(int i=0;i<ANALOG_READ_COUNT;i++){        
    sum+=analogRead(A0);
  }
  return sum/ANALOG_READ_COUNT;
}


void putItemValue(String itemName, String itemValue)
{
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.PUT(itemValue);
}
