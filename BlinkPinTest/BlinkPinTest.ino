/*
 ESP8266 Blink by Simon Peter
 Blink the blue LED on the ESP-01 module
 This example code is in the public domain
 
 The blue LED on the ESP-01 module is connected to GPIO1 
 (which is also the TXD pin; so we cannot use Serial.print() at the same time)
 
 Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/


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

int pins[]={D0,D1,D2,D3,D4,D5,D6,D7,D8,D9,D10};



void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  for(int i=0; i<=10;i++)
    pinMode(pins[i], OUTPUT);     // Initialize the LED_BUILTIN pin as an output   
}

void blink(int pin) {
  digitalWrite(pin, HIGH);                                   
  delay(1000);
  digitalWrite(pin, LOW);    
}


// the loop function runs over and over again forever
void loop() {
  for (int i=0; i<=10; i++)
    blink(pins[i]);   
}
