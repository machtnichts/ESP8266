
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


#define DHTPIN D6       // what digital pin the DHT22 is conected to

DHTesp dht;

const float factor = 4.16/591;

ESP8266WiFiMulti WiFiMulti;

void setup() {   
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_NAME,WIFI_PASSWORD);
  while(WiFiMulti.run() != WL_CONNECTED)
    delay(10);      
  dht.setup(DHTPIN, DHTesp::AM2302);
  
  putItemValue("ESP8266Logger",String("-/-\-"));    
  TempAndHumidity newValues = dht.getTempAndHumidity();
  if (dht.getStatus() == 0) {
    putItemValue("ESP8266Humidity2",String(newValues.humidity));
    putItemValue("ESP8266Temperature2",String(newValues.temperature));
  }
  int value = analogRead(A0);
  Serial.printf("raw value from A0: %d\n",value); 
  float voltage = value * factor;
  Serial.println(voltage);  
  putItemValue("ESP8266Voltage2",String(voltage));  
  unsigned long ts = millis();
  putItemValue("ESP8266TS2",String(ts));  
  putItemValue("ESP8266Logger",String("ZZzz.."));
  //ESP.deepSleep(30e6); // 30e6 is 30 seconds    
  ESP.deepSleep(15*60e6); // 60e6 is 60 seconds / 1 min * 15 = 15min
}

void loop() {  
}

void putItemValue(String itemName, String itemValue)
{
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.PUT(itemValue);
}

void blinkLed(int dlay){ 
    digitalWrite(LED_BUILTIN, LOW);                                   
    delay(dlay);
    digitalWrite(LED_BUILTIN, HIGH);    
}
