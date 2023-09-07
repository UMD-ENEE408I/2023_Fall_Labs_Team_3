#include <Arduino.h>
#include <Encoder.h>

const unsigned int M1_ENC_A = 39;
const unsigned int M1_ENC_B = 38;
const unsigned int M2_ENC_A = 37;
const unsigned int M2_ENC_B = 36;

const unsigned int M1_IN_1 = 13;
const unsigned int M1_IN_2 = 12;
const unsigned int M2_IN_1 = 25;
const unsigned int M2_IN_2 = 14;

const unsigned int M1_IN_1_CHANNEL = 8;
const unsigned int M1_IN_2_CHANNEL = 9;
const unsigned int M2_IN_1_CHANNEL = 10;
const unsigned int M2_IN_2_CHANNEL = 11;

const unsigned int M1_I_SENSE = 35;
const unsigned int M2_I_SENSE = 34;

const unsigned int PWM_VALUE = 512; // Max PWM given 8 bit resolution

const int freq = 5000;
const int resolution = 8;

boolean status = true;

void setup() {
  // Stop the right motor by setting pin 14 low
  // this pin floats high or is pulled
  // high during the bootloader phase for some reason
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  delay(100);

  Serial.begin(115200);
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

}

void forward() {
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, 100);
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, 100);
}

void backward() {
  //rotate wheels backwards
  ledcWrite(M1_IN_1_CHANNEL, 100);
  ledcWrite(M1_IN_2_CHANNEL, 0);
  ledcWrite(M2_IN_1_CHANNEL, 100);
  ledcWrite(M2_IN_2_CHANNEL, 0);
}

void stop() {
  //stop motors
  ledcWrite(M1_IN_1_CHANNEL, 512);
  ledcWrite(M1_IN_2_CHANNEL, 512);
  ledcWrite(M2_IN_1_CHANNEL, 512);
  ledcWrite(M2_IN_2_CHANNEL, 512);
}
//rotate the wheels forward 360◦, stop for 5 seconds and rotate backwards 360◦
void rotateWheels(Encoder &enc1, Encoder &enc2){

  if (status) {
    forward();

    if(enc1.read() > 360) {
      enc1.write(0);
      enc2.write(0);
      stop();
      delay(5000);
      status = false;
    }
  } else {

    backward();

    if(enc1.read() < -360){
      enc1.write(0);
      enc2.write(0);
      stop();
      delay(5000);
      status = true;
    }
  }
}

//set left motor pwm
void setLeftMtr(float value){
  if (value == 0){
    ledcWrite(M1_IN_1_CHANNEL, 0);
    ledcWrite(M1_IN_2_CHANNEL, 0);
    return;
  }
  if (value > 0){
    if (value > 255.0){
      value = 255.0;
    }
    ledcWrite(M1_IN_1_CHANNEL, 0);
    ledcWrite(M1_IN_2_CHANNEL, (uint32_t) value);
  }
  if (value < 0){
    if (value < -255.0){
      value = -255.0;
    }
    ledcWrite(M1_IN_1_CHANNEL, (uint32_t) -value);
    ledcWrite(M1_IN_2_CHANNEL, 0);
  }
}

//set left motor speed
void setLeftSpd(float target_spd, Encoder &enc){
  float spd = 0; //encoder counts per second
  float pwm = 0; //motor pwm
  float thrshld = 1; //threshold for speed
  int step = 10; //delay in ms for detecting speed

  while(target_spd-spd > thrshld || target_spd-spd < -thrshld){
    //get speed
    spd = enc.read();
    delay(step);
    spd -= enc.read();
    spd *= 1000/step; //multiply to get counts/s

    //determine difference in speed
    if(spd < target_spd){
      //make motor faster

    }
  }
  
}

void loop() {
  // Create the encoder objects after the motor has
  // stopped, else some sort exception is triggered
  Encoder enc1(M1_ENC_A, M1_ENC_B);
  Encoder enc2(M2_ENC_A, M2_ENC_B);
  // while(true) {
  //   delay(500);
  //   Serial.print("Encoder 1 Value: ");
  //   Serial.print(enc1.read());
  //   Serial.print("  Encoder 2 Value: ");
  //   Serial.println(enc2.read());
  // }
  while(true){
    rotateWheels(enc1, enc2);
  } 

}
