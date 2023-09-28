#include <Arduino.h>
#include <Encoder.h>
#include <Adafruit_MCP3008.h>

//line sensor stuff
Adafruit_MCP3008 adc1;
Adafruit_MCP3008 adc2;

const unsigned int ADC_1_CS = 2;
const unsigned int ADC_2_CS = 17;

int adc1_buf[8];
int adc2_buf[8];

int lines[13];

//encoder stuff
const unsigned int M1_ENC_A = 39;
const unsigned int M1_ENC_B = 38;
const unsigned int M2_ENC_A = 37;
const unsigned int M2_ENC_B = 36;

//motor stuff
const unsigned int M1_IN_1 = 13;
const unsigned int M1_IN_2 = 12;
const unsigned int M2_IN_1 = 25;
const unsigned int M2_IN_2 = 14;

const unsigned int M1_IN_1_CHANNEL = 8;
const unsigned int M1_IN_2_CHANNEL = 9;
const unsigned int M2_IN_1_CHANNEL = 10;
const unsigned int M2_IN_2_CHANNEL = 11;

const unsigned int PWM_VALUE = 512; // Max PWM given 8 bit resolution

const int freq = 5000;
const int resolution = 10;

//line position
int position = 0;

//pid stuff
long prevT = 0;
float eprev = 0;
float ei = 0;

// PID constants
float kp = 0.5;
float kd = -0.0025;
float ki = 0;

int base_pwm;

//light = 1, dark = 0
void readADC() {
  for (int i = 0; i < 8; i++) {
    adc1_buf[i] = adc1.readADC(i);
    adc2_buf[i] = adc2.readADC(i);

    if (i<7) {
      //Serial.print((adc1_buf[i]>700)?"0":"1"); Serial.print("\t");
      lines[i*2] = (adc1_buf[i]>700)?1:0;
    }

    if (i<6) {
      //Serial.print((adc2_buf[i]>700)?"0":"1"); Serial.print("\t");
      lines[i*2+1] = (adc2_buf[i]>700)?1:0;
    }
  }
}

void detectLinePosition() {
  readADC();
  int prev = 0;
  int curr = 0;
  int upi = 0;
  for (int i = 0; i<13; i++){
    curr = lines[i];
    if (prev != curr){
      if (curr > 0){
        upi = i;
      }
      else{
        position = (upi+i)/2;
      }
    }
    prev = curr;
  }
  if(curr > 0){
    position = (upi+12)/2;
  }
}

void setup() {
  // Stop the right motor by setting pin 14 low
  // this pin floats high or is pulled
  // high during the bootloader phase for some reason
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  delay(100);

  Serial.begin(115200);
  //line sensor setup
  adc1.begin(ADC_1_CS);  
  adc2.begin(ADC_2_CS);

  //motor setup
  ledcSetup(M1_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M1_IN_2_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_2_CHANNEL, freq, resolution);

  ledcAttachPin(M1_IN_1, M1_IN_1_CHANNEL);
  ledcAttachPin(M1_IN_2, M1_IN_2_CHANNEL);
  ledcAttachPin(M2_IN_1, M2_IN_1_CHANNEL);
  ledcAttachPin(M2_IN_2, M2_IN_2_CHANNEL);
}

void setrightpwm(float value){
  if (value == 0){
    ledcWrite(M2_IN_1_CHANNEL, 0);
    ledcWrite(M2_IN_2_CHANNEL, 0);
    return;
  }
  if (value > 0){
    if(value > PWM_VALUE){
      value = PWM_VALUE;
    }
    ledcWrite(M2_IN_1_CHANNEL, 0);
    ledcWrite(M2_IN_2_CHANNEL, (int) value);
  }
  if (value < 0){
    if(value < -PWM_VALUE){
      value = -PWM_VALUE;
    }
    ledcWrite(M2_IN_1_CHANNEL, (int) -value);
    ledcWrite(M2_IN_2_CHANNEL, 0);
  }
}

void setleftpwm(float value){
  if (value == 0){
    ledcWrite(M1_IN_1_CHANNEL, 0);
    ledcWrite(M1_IN_2_CHANNEL, 0);
    return;
  }
  if (value > 0){
    if(value > PWM_VALUE){
      value = PWM_VALUE;
    }
    ledcWrite(M1_IN_1_CHANNEL, 0);
    ledcWrite(M1_IN_2_CHANNEL, (int) value);
  }
  if (value < 0){
    if(value < -PWM_VALUE){
      value = -PWM_VALUE;
    }
    ledcWrite(M1_IN_1_CHANNEL, (int) -value);
    ledcWrite(M1_IN_2_CHANNEL, 0);
  }
}

void loop() {
  Encoder encleft(M1_ENC_A, M1_ENC_B);
  Encoder encright(M2_ENC_A, M2_ENC_B);

  base_pwm = 320;
  while(1){
    detectLinePosition();

    int r_enc_count = encright.read();
    int l_enc_count = encleft.read();

    int e = 100*(position-6);

    int de = e-eprev;
    eprev = e;

    // integral
    ei = ei + e;
  
    // control signal
    int u = kp*e + kd*de + ki*ei;

    setleftpwm(base_pwm - u);
    setrightpwm(base_pwm + u);
  }
}