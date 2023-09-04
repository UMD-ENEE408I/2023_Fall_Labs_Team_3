#include <Arduino.h>
#include <Adafruit_MCP3008.h>

Adafruit_MCP3008 adc1;
Adafruit_MCP3008 adc2;

const unsigned int ADC_1_CS = 2;
const unsigned int ADC_2_CS = 17;

int adc1_buf[8];
int adc2_buf[8];

int lines[13];

void readADC() {
  for (int i = 0; i < 8; i++) {
    adc1_buf[i] = adc1.readADC(i);
    adc2_buf[i] = adc2.readADC(i);

    if (i<7) {
      //Serial.print((adc1_buf[i]>700)?"0":"1"); Serial.print("\t");
      lines[i*2] = (adc1_buf[i]>700)?0:1;
    }

    if (i<6) {
      //Serial.print((adc2_buf[i]>700)?"0":"1"); Serial.print("\t");
      lines[i*2+1] = (adc2_buf[i]>700)?0:1;
    }
  }
}

void detectLinePosition() {
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
        Serial.print("Line occurs at "); Serial.print(upi); Serial.print(" to "); Serial.print(i); Serial.print("\t");
      }
    }
    prev = curr;
  }
  if(curr > 0){
    Serial.print("Line occurs at "); Serial.print(upi); Serial.print(" to 13"); Serial.print("\t");
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

  adc1.begin(ADC_1_CS);  
  adc2.begin(ADC_2_CS);

}

void loop() {

  int t_start = micros();
  readADC();
  detectLinePosition();
  int t_end = micros();

  Serial.print("time: \t"); Serial.print(t_end - t_start); Serial.print("\t");
  Serial.println();

  delay(500);

}
