#include <TCP_over_Serial.h>
#include <PubSubClient.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include "arduino_secrets.h"

MPU6050 mpu6050(Wire);

const int buzzer = 9;

TCPOverSerialClient* s;       // Protocol Client running over USB Serial
PubSubClient         client;  // MQTT Client

//delete this (user and key var definitions)
//replace every "user" var with your Adafruit username
//and replace every "key" var with your Adafruit key
const char* user[] = SECRET_USER;
const char* key[] = SECRET_KEY;

//create feeds with these names first
//defines feeds to publish to
const char* broker   = "io.adafruit.com";
const char* ledTopic = "LedIn";
//const char* outTopic = user + "/feeds/outtopic";
const char* accelX = user + "/feeds/accelx";
const char* accelY = user + "/feeds/accely";
const char* accelZ = user + "/feeds/accelz";
const char* currentPublish  = user + "/feeds/currentstatus";

uint32_t lastPub = 0;

float accelYRecord [10];
float accelZRecord [10];

String currentStatus;

void setup() {
  pinMode(13, OUTPUT);
  s = new TCPOverSerialClient();
  client.setClient(*s);

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  // MQTT Broker running on same PC the Arduino is connected to.
  client.setServer(broker, 1883);
  client.setCallback(callback);
}

void evalForm (float arrY[], float arrZ[], String& stat){
  mpu6050.update();
  for (int i = 0; i < 10; i++) {
        // Append sensor value to the array
        arrY[i] = mpu6050.getAccY();
        arrZ[i] = mpu6050.getAccZ();
  }
  
  float maxY = arrY[0];
  float maxZ = arrZ[0];

  for (int i = 0; i < 10; i++) {
        // Append sensor value to the array
        if (arrY[i]>maxY){
          maxY = arrY[i];
        }
        if (arrY[i]>maxY){
          maxY = arrY[i];
        }
  }

 if(maxY>0.0){
  stat = "Good form!";
 }
 else if(maxY>-0.05){
  stat = "Bend backwards, not just down";
 }
 else if(maxY>=-0.1){
  stat = "Knee bending inwards";
 }
}

void loop() {
  mpu6050.update();
  // If not connected, retry indefinitely.
  
  while(!client.loop()) {
    while(!connectToBroker()) {

      // If connection attempt unsuccessful, flash led fast.
      for (uint8_t fc = 0; fc < 10; fc++) {
        digitalWrite(13, !digitalRead(13));
        delay(200);
      }
    }
  }
  
  float accYf = mpu6050.getAccY();
  
  if((accYf>-0.2)){
    evalForm(accelYRecord, accelZRecord, currentStatus);
    client.publish(currentPublish, currentStatus.c_str());
    if((currentStatus == "Knee bending inwards")||(currentStatus == "Bend backwards, not just down")){
      tone(buzzer, 1000); // Send 1KHz sound signal...
    }
    else if (currentStatus == "Good form!"){
      noTone(buzzer);
    }
    //Serial.println("");
  }
  else{
    client.publish(currentPublish, "No activity detected");
    //Serial.println("");
    noTone(buzzer);     // Stop sound...
    delay(500);
  }
  
  //String accX = String(mpu6050.getAccX());
  //client.publish(accelX, accX.c_str());
  String accYs = String(mpu6050.getAccY());
  client.publish(accelY, accYs.c_str());
  String accZ = String(mpu6050.getAccZ());
  client.publish(accelZ, accZ.c_str());

  delay(6000);
}

// MQTT incoming message from broker callback
void callback(char* topic, byte* payload, unsigned int length) {
  // Only proceed if incoming message's topic matches.
  // Toggle led if according to message character.
  if ((strcmp(topic, ledTopic) == 0) && (length == 1)) {
    if (payload[0] == '1') {
      digitalWrite(13, HIGH);
    } else if (payload[0] == '2') {
      digitalWrite(13, LOW);
    }
  }
}

boolean connectToBroker() {
  if (client.connect("arduinoClient", user, key)) {
  //comment previous statement and use next statement if broker does not need credentials
  //if (client.connect("arduinoClient")) {
    // Publish first message on connect and subscribe to Led controller.
    //client.publish(outTopic, "Arduino connected!");
    client.subscribe(ledTopic);
    return true;
  } else {
    return false;
  }
}
