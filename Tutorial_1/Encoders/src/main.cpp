#include <Arduino.h>
#include <Encoder.h>

const unsigned int M1_ENC_A = 39;
const unsigned int M1_ENC_B = 38;
const unsigned int M2_ENC_A = 37;
const unsigned int M2_ENC_B = 36;


void setup() {
  // Stop the right motor by setting pin 14 low
  // this pin floats high or is pulled
  // high during the bootloader phase for some reason
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  delay(100);


  Serial.begin(115200);
}

//print encoder counts
void printEnc(Encoder &enc1, Encoder &enc2){
  while(true) {
    delay(500);
    Serial.print("Encoder 1 Value: ");
    Serial.print(enc1.read());
    Serial.print("  Encoder 2 Value: ");
    Serial.println(enc2.read());
  }
}

void loop() {
  Encoder enc1(M1_ENC_A, M1_ENC_B);
  Encoder enc2(M2_ENC_A, M2_ENC_B);
  printEnc(enc1, enc2);
}
