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
char  ReplyBuffer[] = "done";       // a string to send back

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

const unsigned int M1_I_SENSE = 35;
const unsigned int M2_I_SENSE = 34;

const int freq = 5000;
const int resolution = 10;

const unsigned int PWM_VALUE = 512;

//encoder stuff
const unsigned int M1_ENC_A = 39;
const unsigned int M1_ENC_B = 38;
const unsigned int M2_ENC_A = 37;
const unsigned int M2_ENC_B = 36;

//mpu
Adafruit_MPU6050 mpu;

void setRightMotor(int value){
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
    ledcWrite(M2_IN_1_CHANNEL, -value);
    ledcWrite(M2_IN_2_CHANNEL, 0);
  }
}
void setLeftMotor(int value){
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
    ledcWrite(M1_IN_1_CHANNEL,  -1*value);
    ledcWrite(M1_IN_2_CHANNEL, 0);
  }
}

void brake(){
  ledcWrite(M1_IN_1_CHANNEL, PWM_VALUE);
  ledcWrite(M1_IN_2_CHANNEL, PWM_VALUE);
  ledcWrite(M2_IN_1_CHANNEL, PWM_VALUE);
  ledcWrite(M2_IN_2_CHANNEL, PWM_VALUE);
}

void forward(int distance, Encoder &encleft, Encoder &encright){
  encleft.write(0);
  encright.write(0);
  int r_enc_count = 0;
  int l_enc_count = 0;

  int e = l_enc_count + r_enc_count;

  //pid stuff
  float eprev = 0;
  float de = 0;
  float ei = 0;

  // PID constants
  float kp = 10;
  float kd = 5;
  float ki = 0;

  int base_pwm = 330;

  int u = 0;

  //11cm c
  //get encoder count target based on target distance
  int target_count = distance*360/11;

  while(l_enc_count<target_count){
    r_enc_count = encright.read();
    l_enc_count = encleft.read();

    e = l_enc_count + r_enc_count;

    de = e-eprev;
    ei = ei + e;
    u = kp*e + kd*de + ki*ei;

    setLeftMotor(base_pwm - u);
    setRightMotor(base_pwm + u);

    eprev = e;
    // Serial.print("e = ");
    // Serial.print(e);
    // Serial.print(" lenc = ");
    // Serial.print(l_enc_count);
    // Serial.print(" renc = ");
    // Serial.println(r_enc_count);
    
  }
  brake();
  // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  // Udp.printf("%s", ReplyBuffer);
  // Udp.endPacket();
}

void speak(){
  //stop motors
  //brake();
  ledcAttachPin(BUZZ, BUZZ_CHANNEL);
  ledcWriteNote(BUZZ_CHANNEL, NOTE_C, 8);
  delay(1000);
  ledcDetachPin(BUZZ);
  brake();
  // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  // Udp.printf("%s", ReplyBuffer);
  // Udp.endPacket();
}

void rotate(float target_degrees){
  //convert target degrees to radians
  target_degrees *= -0.01745; //degrees*pi/180
  //radians rotated
  float r = 0;
  
  //accuracy of rotation
  float acc = 0.02;

  //motor speed 0-255
  float speed = 350;

  //time step in milliseconds
  unsigned int step = 5;
  //time step in seconds
  float step_time = (float) step/1000.0;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  while (r-target_degrees > acc || r-target_degrees < -acc){
    if (target_degrees > r){
      setLeftMotor(-speed);
      setRightMotor(speed);
    }
    if (target_degrees < r){
      //rotate 
      setLeftMotor(speed);
      setRightMotor(-speed);
    }
    delay(step);
    mpu.getEvent(&a, &g, &temp);
    delay(step);
    r += g.gyro.z * step_time*2;
  }
  //stop motors
  brake();
  // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  // Udp.printf("%s", ReplyBuffer);
  // Udp.endPacket();
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

  pinMode(M1_I_SENSE, INPUT);
  pinMode(M2_I_SENSE, INPUT);

  //ledcAttachPin(BUZZ, BUZZ_CHANNEL);
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
  Encoder encleft(M1_ENC_A, M1_ENC_B);
  Encoder encright(M2_ENC_A, M2_ENC_B);
  while(1){
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
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.printf("%s", packetBuffer);
        Udp.endPacket();
        Serial.print("message: ");
        Serial.println(packetBuffer[0]);
        switch(packetBuffer[0]){
          case 's':
            Serial.println("speaking");
            speak();
            Serial.println("spoken");
            break;
          case 'l':
            Serial.println("rotate left");
            rotate(-1*atoi(packetBuffer+1));
            break;
          case 'r':
            Serial.println("rotate right");
            rotate(atoi(packetBuffer+1));
            break;
          case 'b':
            Serial.println("braking");
            brake();
            break;
          case 'f':
            Serial.println("forward!");
            forward(atoi(packetBuffer+1), encleft, encright);
            break;
          default:
            brake();
        }
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.printf("%s", ReplyBuffer);
        Udp.endPacket();
      }
    }
  }
}