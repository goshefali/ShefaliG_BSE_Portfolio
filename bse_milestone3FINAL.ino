//Reference: https://lastminuteengineers.com/mpu6050-accel-gyro-arduino-tutorial/
//works better with 115200 baud but also works with 9600 
//for bluetooth module
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
const int buzzer = 9; //buzzer to arduino pin 9

double accelX;
double accelY;
double accelZ;
double rotateX;
double rotateY;
double rotateZ;
double temp;

float accelYRecord [10];
float accelZRecord [10];

String currentStatus = "";

Adafruit_MPU6050 mpu;

void setup(void) {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output

  // Try to initialize!
  if (!mpu.begin()) {
    //Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // set accelerometer range to +-8G
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  // set gyro range to +- 500 deg/s
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  // set filter bandwidth to 21 Hz
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);
}

void evalForm (float arrY[], float arrZ[], String& stat){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  for (int i = 0; i < 10; i++) {
        // Append sensor value to the array
        arrY[i] = a.acceleration.y;
        arrZ[i] = a.acceleration.z;
        delay(500);
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

 if(maxY>2.5){
  stat = "Good form!";
 }
 else if(maxY<1.5){
  stat = "Knee bending inwards";
 }
 else if(maxY<2.5){
  stat = "Bend backwards, not just down";
 }
}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  if((a.acceleration.z>0.0) && (a.acceleration.y>0.0)){
    evalForm(accelYRecord, accelZRecord, currentStatus);
    Serial.println(currentStatus);
    if((currentStatus == "Knee bending inwards")||(currentStatus == "Bend backwards, not just down")){
      tone(buzzer, 1000); // Send 1KHz sound signal...
    }
    else if (currentStatus == "Good form!"){
      noTone(buzzer);
    }
    Serial.println("");
  }
  else{
    Serial.println("No activity detected");
    Serial.println("");
    noTone(buzzer);     // Stop sound...
    delay(500);
  }
  delay(500);
}
