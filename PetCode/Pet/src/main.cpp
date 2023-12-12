#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Encoder.h>
#include <WiFi.h>
#include <WiFiUdp.h>

//buzzer
const unsigned int BUZZ = 26; // Check schematic and see pin connection to buzzer
const unsigned int BUZZ_CHANNEL = 0; //Selecting PWM channel 0

//wifi
int status = WL_IDLE_STATUS;
char ssid[] = "AndroidAP79C6"; //  your network SSID (name)
char pass[] = "enee408i";

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

WiFiUDP Udp;

//motors
//motor 
const unsigned int M1_IN_1 = 13;
const unsigned int M1_IN_2 = 12;
const unsigned int M2_IN_1 = 25;
const unsigned int M2_IN_2 = 14;

const unsigned int M1_IN_1_CHANNEL = 0;
const unsigned int M1_IN_2_CHANNEL = 1;
const unsigned int M2_IN_1_CHANNEL = 2;
const unsigned int M2_IN_2_CHANNEL = 3;

const int freq = 5000;
const int resolution = 8;

const unsigned int PWM_VALUE = 512;

//mpu
Adafruit_MPU6050 mpu;

void speak(){
  ledcWriteNote(BUZZ_CHANNEL, NOTE_C, 8);
  delay(500);
  ledcWrite(BUZZ_CHANNEL, 0);
}

void setRightMotor(float value){
  if (value == 0){
    ledcWrite(M2_IN_1_CHANNEL, 0);
    ledcWrite(M2_IN_2_CHANNEL, 0);
    return;
  }
  if (value > 0){
    if (value > PWM_VALUE){
      value = PWM_VALUE;
    }
    ledcWrite(M2_IN_1_CHANNEL, 0);
    ledcWrite(M2_IN_2_CHANNEL, (uint32_t) value);
  }
  if (value < 0){
    if (value < -PWM_VALUE){
      value = -PWM_VALUE;
    }
    ledcWrite(M2_IN_1_CHANNEL, (uint32_t) -value);
    ledcWrite(M2_IN_2_CHANNEL, 0);
  }
}
void setLeftMotor(float value){
  if (value == 0){
    ledcWrite(M1_IN_1_CHANNEL, 0);
    ledcWrite(M1_IN_2_CHANNEL, 0);
    return;
  }
  if (value > 0){
    if (value > PWM_VALUE){
      value = PWM_VALUE;
    }
    ledcWrite(M1_IN_1_CHANNEL, 0);
    ledcWrite(M1_IN_2_CHANNEL, (uint32_t) value);
  }
  if (value < 0){
    if (value < -PWM_VALUE){
      value = -PWM_VALUE;
    }
    ledcWrite(M1_IN_1_CHANNEL, (uint32_t) -value);
    ledcWrite(M1_IN_2_CHANNEL, 0);
  }
}

void turn(int e){
  int speed = 200;
  if(e>0){
    setRightMotor(speed);
    setLeftMotor(-speed);
  }
  else{
    setRightMotor(-speed);
    setLeftMotor(speed);
  }
}

void brake(){
  ledcWrite(M1_IN_1_CHANNEL, PWM_VALUE);
  ledcWrite(M1_IN_2_CHANNEL, PWM_VALUE);
  ledcWrite(M1_IN_1_CHANNEL, PWM_VALUE);
  ledcWrite(M1_IN_2_CHANNEL, PWM_VALUE);
}

void rotate(float target_degrees){
  //convert target degrees to radians
  target_degrees *= 0.01745; //degrees*pi/180
  //radians rotated
  float r = 0;
  
  //accuracy of rotation
  float acc = 0.02;

  //motor speed 0-255
  float speed = 180;

  //time step in milliseconds
  unsigned int step = 1;
  //time step in seconds
  float step_time = (float) step/1000.0;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  while (r-target_degrees > acc || r-target_degrees < -acc){
    if (target_degrees > r){
      setRightMotor(speed);
      setLeftMotor(-speed);
    }
    if (target_degrees < r){
      //rotate 
      setRightMotor(-speed);
      setLeftMotor(speed);
    }
    delay(step);
    mpu.getEvent(&a, &g, &temp);
    delay(step);
    r += g.gyro.z * step_time*2;
  }
  //stop motors
  setRightMotor(0);
  setLeftMotor(0);
}

void setup() {
  ledcSetup(M1_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M1_IN_2_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_2_CHANNEL, freq, resolution);

  ledcAttachPin(M1_IN_1, M1_IN_1_CHANNEL);
  ledcAttachPin(M1_IN_2, M1_IN_2_CHANNEL);
  ledcAttachPin(M2_IN_1, M2_IN_1_CHANNEL);
  ledcAttachPin(M2_IN_2, M2_IN_2_CHANNEL);

  //set right motor to stop
  ledcWrite(M2_IN_2_CHANNEL, 0);
  
  if (!mpu.begin()) {
    //Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  delay(100);

  ledcAttachPin(BUZZ, BUZZ_CHANNEL);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  Udp.begin(localPort);
}

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {

    Serial.print("Received packet of size ");

    Serial.println(packetSize);

    Serial.print("From ");

    IPAddress remoteIp = Udp.remoteIP();

    Serial.print(remoteIp);

    Serial.print(", port ");

    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer

    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {

      packetBuffer[len] = 0;

    }
    Serial.print("message: ");
    Serial.println(packetBuffer[0]);
    switch(packetBuffer[0]){
      case 's':
        speak();
        Serial.println("spoken");
        break;
      case 'l':
        rotate(-1*atoi(packetBuffer+1));
        break;
      case 'r':
        rotate(atoi(packetBuffer+1));
        break;
      case 'b':
        brake();
        break;
    }
  }
}