#include <SoftwareSerial.h>

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


SoftwareSerial co2Serial(D2, D1); // RX, TX

byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};  
byte response[9]; // for answer

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    blinkLed(500);
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

  // set the data rate for the SoftwareSerial port
  co2Serial.begin(9600);

  blinkLed(2000);
}

void loop() { // run over and over  
  int ppmCO2 = readCO2();
  Serial.printf("read PPM: %d\n", ppmCO2);
  delay(10000);
}


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
