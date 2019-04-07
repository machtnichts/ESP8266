
#include <SoftwareSerial.h>

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

// MUX address lines 
#define MUX_S0 D1
#define MUX_S1 D2
#define MUX_S2 D3
#define MUX_S3 D4

// RELAIES 
#define REL1 D5
#define REL2 D6
#define REL3 D7
#define REL4 D8


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

#define ANALOG_INPUT A0

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    blinkLed(1000);

    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);     
    pinMode(MUX_S2, OUTPUT);  
    pinMode(MUX_S3, OUTPUT);  

    pinMode(REL1, OUTPUT);
    pinMode(REL2, OUTPUT);     
    pinMode(REL3, OUTPUT);  
    pinMode(REL4, OUTPUT);  

    digitalWrite(REL1, HIGH);
    digitalWrite(REL2, HIGH);
    digitalWrite(REL3, HIGH);
    digitalWrite(REL4, HIGH);

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
}

void loop() { // run over and over  
  for(int i=0; i<16;i++){
    changeMux(i);
    int value = analogRead(A0);
    if(value>10)
    {
      Serial.printf("raw value from A0: %d at C%d\n",value,i);     
      switchRelais(i);
    }
  }
  delay(250);  
}

void switchRelais(int relNumber)
{
  if(relNumber==0)
  {
    Serial.printf("setting R1 to LOW...\n");
    digitalWrite(REL1, LOW);  
    delay(10000);
    Serial.printf("setting R1 to HIGH...\n");
    digitalWrite(REL1, HIGH);  
  }

}


void changeMux(byte channelNumber){  
  digitalWrite(MUX_S0, (channelNumber & 0x01)==0?LOW:HIGH);
  digitalWrite(MUX_S1, (channelNumber & 0x02)==0?LOW:HIGH);
  digitalWrite(MUX_S2, (channelNumber & 0x04)==0?LOW:HIGH);
  digitalWrite(MUX_S3, (channelNumber & 0x08)==0?LOW:HIGH);
  delay(10);
}

void blinkLed(int dlay){ 
    digitalWrite(LED_BUILTIN, LOW);                                   
    delay(dlay);
    digitalWrite(LED_BUILTIN, HIGH);    
}
