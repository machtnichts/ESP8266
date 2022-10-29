
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

#include "DHTesp.h"

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

/*
 * content of WIFI-credentials.h
 * #define WIFI_NAME "your WIFI name"
 * #define WIFI_PASSWORD "your WIFI password"
*/
#include "WIFI-credentials.h"


#define DHTPIN D6       // what digital pin the DHT22 is connected to

// MUX address lines 
#define MUX_S0 D1
#define MUX_S1 D2
#define MUX_S2 D3
#define MUX_S3 D4

// RELAIES 

#define REL1 D5
//on grid
#define REL2G D7
//on akku
#define REL2A D8

/*
#define REL2 D6
#define REL4 D8 
*/


// defines for analog input channels
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

#define ANALOG_READ_COUNT 3

DHTesp dht;

//const float factor = 16.5/783;

const float factorOne = 3.85/944;
const float factorTwo = 7.7/905;
const float factorThree = 11.55/876;
const float factorAll = 15.4/859;

bool charging;
bool ongrid;
const String STATE_ON = String("ON");
const int delayInSeconds = 60;

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

    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);     
    pinMode(MUX_S2, OUTPUT);  
    pinMode(MUX_S3, OUTPUT);  

    pinMode(REL1, OUTPUT);
    pinMode(REL2G, OUTPUT);     
    pinMode(REL2A, OUTPUT);
//    pinMode(REL3, OUTPUT);  
//    pinMode(REL4, OUTPUT);  

    digitalWrite(REL1, LOW);
//    digitalWrite(REL2, HIGH);
//    digitalWrite(REL3, HIGH);
//    digitalWrite(REL4, HIGH);

  // read temp / hum
  dht.setup(DHTPIN, DHTesp::AM2302);

 charging = false;
 ongrid = false;
}

void loop() {  
  if(emergencyCheck()){
    delay(delayInSeconds*1000);
  }
  else
  {
    putItemValue("ESP8266Logger32",String("..."));
    //read voltage
    monitorVoltage(); 
  
    checkRelais();
    readTemp();
    unsigned long ts = millis();
    putItemValue("ESP8266TS3",String(ts));  

    putItemValue("ESP8266Logger32",String("---"));
    if (charging){
      putItemValue("ESP8266Logger",String("AM: Zz..."));
      delay(delayInSeconds*1000);
    }
    else{
      putItemValue("ESP8266Logger",String("AM: ZZZzzz.."));
      ESP.deepSleep(delayInSeconds*1e6);
    }
  }
}


boolean emergencyCheck(){
  int firstCell = readVoltage(0);
  double firstCellV = factorOne * firstCell;
  if(firstCellV<3.65f){
    emergencyLoad();
    return true;    
  }
  return false;
}

void emergencyLoad()
{
  digitalWrite(REL1, HIGH);
  charging = true;
}

void readTemp(){
  putItemValue("ESP8266Logger32",String("readTemp"));
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
}

void checkRelais(){  
  putItemValue("ESP8266Logger32",String("checkRelais"));
  String REL_STATE = getItemValue("ESP8266_REL1");
  if(STATE_ON == REL_STATE){    
    digitalWrite(REL1, HIGH);    
    charging = true;
  }else{
    digitalWrite(REL1, LOW);
    charging = false;
  }
  REL_STATE = getItemValue("ESP8266_REL2");
  if(STATE_ON == REL_STATE){
    digitalWrite(REL2A, LOW);
    delay(10);
    digitalWrite(REL2G, HIGH);
    delay(350);
    digitalWrite(REL2G, LOW);
    ongrid = true;
  }else{
    digitalWrite(REL2A, LOW);
    delay(10);
    digitalWrite(REL2A, HIGH);
    delay(350);
    digitalWrite(REL2A, LOW);
    ongrid = false;
  }
}

void monitorVoltage(){
  putItemValue("ESP8266Logger32",String("monitorVoltage"));
  Serial.printf("monitorVoltage\n");
  int firstCell = readVoltage(0);
  double firstCellV = factorOne * firstCell;
  Serial.printf("first cell: raw %d, calculated: %f\n",firstCell,firstCellV);    
  int firstAndSecondCell = readVoltage(1);
  double firstAndSecondCellV = factorTwo * firstAndSecondCell;
  Serial.printf("1+2 cells: raw %d, calculated: %f\n",firstAndSecondCell,firstAndSecondCellV);
  int first23Cell = readVoltage(2);
  double first23CellV = factorThree * first23Cell;
  Serial.printf("1+2+3 cells: raw %d, calculated: %f\n",first23Cell,first23CellV);
  int first234Cell = readVoltage(3);
  double first234CellV = factorAll * first234Cell;
  Serial.printf("1+2+3+4 cells: raw %d, calculated: %f\n",first234Cell,first234CellV);

  double secondCellV = firstAndSecondCellV-firstCellV;
  double thirdCellV  = first23CellV - firstAndSecondCellV;
  double fourthCellV = first234CellV -first23CellV ;
  Serial.printf("1,2,3,4 cells: %f - %f - %f - %f\n",firstCellV,secondCellV,thirdCellV,fourthCellV);  

  putItemValue("ESP8266VoltageCell1",String(firstCellV));
  putItemValue("ESP8266VoltageCell2",String(secondCellV));
  putItemValue("ESP8266VoltageCell3",String(thirdCellV));
  putItemValue("ESP8266VoltageCell4",String(fourthCellV));
  putItemValue("ESP8266Voltage3",String(first234CellV));  
}

int readVoltage(int channel){ 
  putItemValue("ESP8266Logger32",String("readVoltage"));
  Serial.printf("readVoltage(%d)\n",channel);
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
  http.addHeader("Content-Type", "text/plain");
  int httpCode = http.PUT(itemValue);
  http.end();
}

String getItemValue(String itemName){
  String url = "http://192.168.178.9:8080/rest/items/" + itemName + "/state";
  HTTPClient http;
  http.begin(url); //HTTP
  int httpCode = http.GET();
  String response=http.getString();
  String dbgMessage = "getItemValue("+itemName+"): response code "+httpCode+", response '"+response+"'\n";
  Serial.printf(string2char(dbgMessage)); 
  http.end();
  return response;
}

char* string2char(String command){
  if(command.length()!=0){
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void changeMUXC0(){
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);
  delay(10);
}

void changeMux(byte channelNumber){  
  Serial.printf("changeMux(%d)\n",channelNumber);
  digitalWrite(MUX_S0, (channelNumber & 0x01)==0?LOW:HIGH);
  digitalWrite(MUX_S1, (channelNumber & 0x02)==0?LOW:HIGH);
  digitalWrite(MUX_S2, (channelNumber & 0x04)==0?LOW:HIGH);
  digitalWrite(MUX_S3, (channelNumber & 0x08)==0?LOW:HIGH);
  delay(10);
}
