#include <SoftwareSerial.h>

#define RX_PIN 14  // D5 on NodeMCU
#define TX_PIN 12  // D6 on NodeMCU
#define SOFTWARE_BAUD 100000
#define MAX_BYTES 25

SoftwareSerial mySerial(RX_PIN, TX_PIN);

// parser
uint8_t buffer[25];
int bufferIndex = 0;
enum State {
  IDLE = 0,
  HEADER,
  HEADER2,
  HEADER3,
};
bool runonce = true;
uint16_t values[16];
uint8_t state = IDLE;
uint8_t b = 0;
uint32_t found_sequence = 0;
int indexx = 0;


void extractValues(const uint8_t* buffer, uint16_t* outValues, int numValues = 16) {
  int bitIndex = 0; // Index to track the current bit position in the buffer

  for (int i = 0; i < numValues; ++i) {
      // Extract 11 bits for each value
      uint16_t value = 0;
      for (int bit = 0; bit < 11; ++bit) {
          // Calculate the index of the byte and the bit within that byte
          int byteIndex = (bitIndex + bit) / 8;
          int bitInByte = (bitIndex + bit) % 8;

          // Extract the bit and add it to value
          if (buffer[byteIndex] & (1 << bitInByte)) {
              value |= (1 << bit);
          }
      }

      // Store the extracted value
      outValues[i] = value;

      // Move to the next 11-bit block
      bitIndex += 11;
  }
}

void serial_report(){
  if(buffer[22]==0x00){
    
    extractValues(buffer, values, 16);
    for (int i = 0; i < 16; ++i) {
      Serial.print(values[i]);
      Serial.print("\t");
    }              
    Serial.println();
    // printBufferAsHex(buffer, 23);
  }
  else{
    memset(buffer, 0, sizeof(buffer));
    // clearRXBuffer();
  }

  // Serial.flush();
  // switch_to_RX();
}

void parse_sbus(){
  if (mySerial.available() > 0) {
    switch(state){
      case IDLE:
        while(mySerial.available()>0){
          yield();
          b = mySerial.read();
          if(b==0x00){
            state=HEADER2;
          }
          else{
            state=IDLE;
          }
        }
        break;
      case HEADER2:
        if(mySerial.available()>23){
          b = mySerial.read();
          if(b==0x0F){
            memset(buffer, 0, sizeof(buffer));
            mySerial.readBytes(buffer, 23);
            state=IDLE;
            found_sequence++;
            
            // serial_count_values();
            // serial_report_periodically();
            serial_report();
            // serial_report_mapped();
            // serial_report_mapped_assembled();

          }else if(b==0x00){
            break;
          }else{
            state=IDLE;
          }
        }
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize software serial with inversion
  mySerial.begin(SOFTWARE_BAUD, SWSERIAL_8E2, RX_PIN, TX_PIN, true);  // true for inverted signal
}

void loop() {
  // static int byteCount = 0;

  // if (mySerial.available()) {
  //   char data = mySerial.read();
    
  //   // Print each byte in HEX format
  //   if (data < 16) Serial.print("0");  // Print leading zero for single digit hex values
  //   Serial.print(data, HEX);
  //   Serial.print(" ");  // Space for readability

  //   byteCount++;
  //   if (byteCount >= MAX_BYTES) {
  //     Serial.println();  // New line after 25 bytes
  //     byteCount = 0;     // Reset counter
  //   }
  // }

  parse_sbus();

}




























/*

#include <ESP8266WiFi.h>
// hex display
#include <stdint.h>

#define BAUD_RATE_READ 100000
// #define BAUD_RATE_WRITE 115200
#define BAUD_RATE_WRITE 115200

uint8_t buffer[25];
int bufferIndex = 0;

uint8_t code[11];

// parser
enum State {
  IDLE = 0,
  HEADER,
  HEADER2,
  HEADER3,
};

bool runonce = true;
uint16_t values[16];
uint8_t state = IDLE;
uint8_t b = 0;
uint32_t found_sequence = 0;
int indexx = 0;

// stats
unsigned long last_report = 0;
uint32_t count_suc = 0;
uint32_t count_fai = 0;
uint32_t uneque_packet_count = 0;
uint16_t old_value = 0;
uint32_t SUCCES = 0;
uint32_t FAILS = 0;

// mapped
int mappedValue;

void switch_to_TX(){
  Serial.end();
  Serial.begin(BAUD_RATE_WRITE);
}
void switch_to_RX(){
  Serial.end();
  Serial.begin(BAUD_RATE_READ, SERIAL_8E2);
}

void clearRXBuffer(){
  int x;
  while (x = Serial.available() > 0)
  {
     while (x--) Serial.read();
  }
}


void serial_count_values(){
  if(buffer[22]==0x00){
    
    extractValues(buffer, values, 16);
    if(values[0]!=old_value){
      switch_to_TX();
      uneque_packet_count++;
      Serial.print(uneque_packet_count);
      Serial.print(", ");
      Serial.println(values[0]);
      old_value = values[0];
      Serial.flush();
      switch_to_RX();
    }
  }
}

void serial_report_periodically(){  
  if(buffer[22]==0x00){
    count_suc++;
  }else{
    count_fai++;
  }

  if(millis()-last_report > 1000){
    last_report=millis();
    int readable_bytes = Serial.available();
    switch_to_TX();    

    Serial.println();

    Serial.print("Succes: ");
    Serial.print(count_suc);
    Serial.print(" Fails: ");
    Serial.print(count_fai);
    Serial.print("           ");
    Serial.println(readable_bytes);
    Serial.println();
    Serial.flush();
    switch_to_RX();
    count_suc=0;
    count_fai=0;
  }
}

void printBufferAsHex(uint8_t* buffer, size_t bufferSize) {
    for (size_t i = 0; i < bufferSize; i++) {
        // Print each byte in hex format with 0x prefix and padded with zeros if necessary
        if (buffer[i] < 0x10) {
            Serial.print("0x0");
        } else {
            Serial.print("0x");
        }
        Serial.print(buffer[i], HEX);

        Serial.print(" ");
    }
    Serial.println();
}

void setup() {
  Serial.begin(BAUD_RATE_READ,SERIAL_8E2);
  Serial.setTimeout(5000);
  Serial.println("Ready to receive data...");
}

void loop() {
  if (Serial.available() > 0) {
    switch(state){
      case IDLE:
        while(Serial.available()>0){
          yield();
          b = Serial.read();
          if(b==0x00){
            state=HEADER2;
          }
          else{
            state=IDLE;
          }
        }
        break;
      case HEADER2:
        if(Serial.available()>0){
          b = Serial.read();
          if(b==0x0F){
            memset(buffer, 0, sizeof(buffer));
            Serial.readBytes(buffer, 23);
            state=IDLE;
            found_sequence++;
            
            // serial_count_values();
            // serial_report_periodically();
            // serial_report();
            // serial_report_mapped();
            serial_report_mapped_assembled();

          }else if(b==0x00){
            break;
          }else{
            state=IDLE;
          }
        }
        break;
    }
  }
}

*/