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

//print encoder counts
void printEnc(){
  Encoder enc1(M1_ENC_A, M1_ENC_B);
  Encoder enc2(M2_ENC_A, M2_ENC_B);
  while(true) {
    delay(500);
    Serial.print("Encoder 1 Value: ");
    Serial.print(enc1.read());
    Serial.print("  Encoder 2 Value: ");
    Serial.println(enc2.read());
  }
}

//rotate the wheels forward 360◦, stop for 5 seconds and rotate backwards 360◦
void rotateWheels(){
  //make sure motor is stopped
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, 0);
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, 0);

  //create encoder objects
  Encoder enc1(M1_ENC_A, M1_ENC_B);
  Encoder enc2(M2_ENC_A, M2_ENC_B);
  
  //rotate wheels forwards
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, 100);
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, 100);

  //check that both wheels rotated 360 degree forwards
  while(enc1.read()<360 || enc2.read()>-360){
    Serial.print(enc1.read());
    Serial.print("//");
    Serial.println(enc2.read());
    delay(10);
  }

  //stop motors
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, 0);
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, 0);

  //wait 5 seconds
  delay(5000);

  //rotate wheels backwards
  ledcWrite(M1_IN_1_CHANNEL, 100);
  ledcWrite(M1_IN_2_CHANNEL, 0);
  ledcWrite(M2_IN_1_CHANNEL, 100);
  ledcWrite(M2_IN_2_CHANNEL, 0);

  //check that both wheels rotated 360 degree forwards
  while(enc1.read()>0 || enc2.read()<0){
    delay(10);
  }

  //stop motors
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, 0);
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, 0);
}

void loop() {
  // Create the encoder objects after the motor has
  // stopped, else some sort exception is triggered
  // Encoder enc1(M1_ENC_A, M1_ENC_B);
  // Encoder enc2(M2_ENC_A, M2_ENC_B);

  // while(true) {
  //   delay(500);
  //   Serial.print("Encoder 1 Value: ");
  //   Serial.print(enc1.read());
  //   Serial.print("  Encoder 2 Value: ");
  //   Serial.println(enc2.read());
  // }
  //printEnc();
  rotateWheels();
}
