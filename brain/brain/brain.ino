#include <WiFi.h>
#include <esp_now.h>

// eeprom
#include <EEPROM.h>
#define EEPROM_SIZE 10

#define pwm_A 27
#define INA_1 12
#define INA_2 14

#define pwm_B 26
#define INB_1 4
#define INB_2 2

#define button 23

unsigned long last_loop_update = 0;
unsigned int loop_hz = 100;
unsigned int loop_ms = 1000/loop_hz;

uint8_t id = 1;

typedef struct struct_message {
  byte mode;
  byte id  ;
  byte ch01;
  byte ch02;
  byte ch03;
  byte ch04;
  byte ch05;
  byte ch06;
  byte ch07;
  byte ch08;
  byte ch09;
  byte ch10;
  byte ch11;
  byte ch12;
  byte ch13;
  byte ch14;
  byte ch15;
  byte ch16;
} struct_message;

struct_message myData;

unsigned long last_packet=0;

// servo
const int servoPin = 21;
unsigned long servo_start_time = 0;
unsigned long servo_off_time = 0;
unsigned long servo__next_on_time = 0;
const int servoPeriod = 20000;
uint8_t current_duty = 0;
bool servo_on = true;

void init_servo_pwm() {
  servo_start_time = micros();
  digitalWrite(servoPin, HIGH);
  servo_on = true;
  servo_off_time = servo_start_time + map(current_duty, 0, 255, 1000, 2000);
  servo__next_on_time = servo_start_time + servoPeriod;
}

void update_servo() {
  unsigned long currentTime = micros();
  if(servo_on){
    if (currentTime >= servo_off_time) {
      digitalWrite(servoPin, LOW);
      digitalWrite(22, LOW);
      servo_on = false;
    }
  }else{
    if (currentTime >= servo__next_on_time) {
      digitalWrite(servoPin, HIGH);
      digitalWrite(22, HIGH);
      servo_on = true;
      servo_off_time = servo__next_on_time + map(current_duty, 0, 255, 1000, 2000);
      servo__next_on_time += servoPeriod;
    }
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  last_packet=millis();
  

  // Serial.print(myData.mode);
  // Serial.print("\t");
  // Serial.print(myData.id);
  // Serial.print("\t");
  // Serial.print(myData.ch01);
  // Serial.print("\t");
  // Serial.print(myData.ch02);
  // Serial.print("\t");
  // Serial.print(myData.ch03);
  // Serial.print("\t");
  // Serial.print(myData.ch04);
  // Serial.print("\t");
  // Serial.print(myData.ch05);
  // Serial.print("\t");
  // Serial.print(myData.ch06);
  // Serial.print("\t");
  // Serial.print(myData.ch07);
  // Serial.print("\t");
  // Serial.print(myData.ch08);
  // Serial.print("\t");
  // Serial.print(myData.ch09);
  // Serial.print("\t");
  // Serial.print(myData.ch10);
  // Serial.print("\t");
  // Serial.print(myData.ch11);
  // Serial.print("\t");
  // Serial.print(myData.ch12);
  // Serial.print("\t");
  // Serial.print(myData.ch13);
  // Serial.print("\t");
  // Serial.print(myData.ch14);
  // Serial.print("\t");
  // Serial.print(myData.ch15);
  // Serial.print("\t");
  // Serial.println(myData.ch16);
}


const int ledPin1 = 27; //a
const int ledPin2 = 26; //b
// const int ledPin3 = 21; //esc

const int freq_DRVR = 5000;
const int resolution_DRVR = 8;
// the number of the LED pin

const int ledChannel1 = 1; // Use channel 0 for ledPin1
const int ledChannel2 = 2; // Use channel 1 for ledPin2



void setup() {
  
  EEPROM.begin(EEPROM_SIZE);


  Serial.begin(115200);
  pinMode(22, OUTPUT);

  pinMode(INA_1, OUTPUT);
  pinMode(INA_2, OUTPUT);
  pinMode(INB_1, OUTPUT);
  pinMode(INB_2, OUTPUT);
  pinMode(servoPin, OUTPUT);
  pinMode(button, INPUT);

  digitalWrite(INA_1, HIGH);
  digitalWrite(INA_2, LOW);
  digitalWrite(INB_1, HIGH);
  digitalWrite(INB_2, LOW);

  digitalWrite(22, HIGH);

  id = EEPROM.read(0);

  ledcSetup(ledChannel1, freq_DRVR, resolution_DRVR);
  ledcAttachPin(ledPin1, ledChannel1);

  ledcSetup(ledChannel2, freq_DRVR, resolution_DRVR);
  ledcAttachPin(ledPin2, ledChannel2);
  

  

  delay(500);


  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  
  init_servo_pwm();
}

void loop() {
    update_servo();
  if(millis()-last_loop_update>=loop_ms){
    last_loop_update=millis();

    if(!digitalRead(button)){
      id++;
      if(id>4){
        id=1;
      }
      delay(500);
      digitalWrite(22, LOW);
      for(uint8_t i=0; i < id; i++){
        digitalWrite(22, HIGH);
        delay(200);
        digitalWrite(22, LOW);
        delay(200);
      }
      EEPROM.write(0, id);
      EEPROM.commit();
      
      Serial.print("NEW_ID: ");
      Serial.println(id);
      delay(200);
    }
    // if no packets for 100ms assume FS_RC
    // if( (millis()-last_packet) > 50 || id!=myData.ch16-127){
    if( (millis()-last_packet) > 50 ){
      digitalWrite(INA_1, LOW);
      digitalWrite(INA_2, LOW);
      digitalWrite(INB_1, LOW);
      digitalWrite(INB_2, LOW);
      ledcWrite(ledChannel1, 0);
      ledcWrite(ledChannel2, 0);

      current_duty = 0;
      Serial.print("NO SIGNAL my ch: ");
      Serial.print(id);
      Serial.print(" received id: ");
      Serial.println(myData.ch16-127);
    }else if(id==myData.ch16-127){
        if(myData.ch01>128){
        digitalWrite(INA_1, HIGH);
        digitalWrite(INA_2, LOW);
        ledcWrite(ledChannel1, ((myData.ch01-128)*2)+1);
        Serial.print("A for ward: ");
        Serial.println(((myData.ch01-128)*2)+1);
      }else if(myData.ch01==128){
        digitalWrite(INA_1, LOW);
        digitalWrite(INA_2, LOW);
        ledcWrite(ledChannel1, 0);
        Serial.println("A STOP");
      }else if(myData.ch01<128){
        digitalWrite(INA_1, LOW);
        digitalWrite(INA_2, HIGH);
        ledcWrite(ledChannel1, ((128-myData.ch01)*2)-1);
        Serial.print("A backward: ");
        Serial.println(((128-myData.ch01)*2)-1);
      }

      if(myData.ch02>128){
        digitalWrite(INB_1, HIGH);
        digitalWrite(INB_2, LOW);
        ledcWrite(ledChannel2, ((myData.ch02-128)*2)+1);
        Serial.print("B for ward: ");
        Serial.println(((myData.ch02-128)*2)+1);
      }else if(myData.ch02==128){
        digitalWrite(INB_1, LOW);
        digitalWrite(INB_2, LOW);
        ledcWrite(ledChannel2, 0);
        Serial.println("B STOP");
      }else if(myData.ch02<128){
        digitalWrite(INB_1, LOW);
        digitalWrite(INB_2, HIGH);
        ledcWrite(ledChannel2, ((128-myData.ch02)*2)-1);
        Serial.print("B backward: ");
        Serial.println(((128-myData.ch02)*2)-1);
      }
      
      current_duty = myData.ch03;
      Serial.print("SERVO: ");
      Serial.println(myData.ch03);
      // Serial.println(map(myData.ch03,0,255,0,180));
    }
  }
}
