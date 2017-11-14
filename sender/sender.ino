#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  Serial.begin(9600);
}
void loop() {
  int input;
  input = Serial.read();
  if(input != -1 ){
    char text[] = " \n";
    text[0]=input;
    radio.write(&text, sizeof(text));
  }
}
