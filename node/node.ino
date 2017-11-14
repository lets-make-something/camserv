#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(14, 15); // CE, CSN
const byte address[6] = "00001";

struct Wheel{
  int pattern_step;
  int forward;
  int wait;
  int wait_reset;
  int assign[4];

  void setup(int a1,int a2,int a3,int a4){
    wait=10;
    pattern_step=0;
    forward=1;
    wait_reset=10;
    this->assign[0]=a1;
    this->assign[1]=a2;
    this->assign[2]=a3;
    this->assign[3]=a4;
    pinMode(this->assign[0],OUTPUT);
    pinMode(this->assign[1],OUTPUT);
    pinMode(this->assign[2],OUTPUT);
    pinMode(this->assign[3],OUTPUT);
  }
  void front(){
    if(0<forward){
      wait_reset--;
      if(wait_reset<1){
        wait_reset=1;
      }
    }else{
      wait_reset++;
      if(wait_reset>10){
        forward++;
        wait_reset=10;
      }
    }
  }
  
  void back(){
    if(0>forward){
      wait_reset--;
      if(wait_reset<1){
        wait_reset=1;
      }
    }else{
      wait_reset++;
      if(wait_reset>10){
        forward--;
        wait_reset=10;
      }
    }
  }

  void loop(){
    wait--;
    if(wait<=0){
      const char pattern[8][4]={
        {1,0,0,1},
        {1,0,0,0},
        {1,1,0,0},
        {0,1,0,0},
        {0,1,1,0},
        {0,0,1,0},
        {0,0,1,1},
        {0,0,0,1},
      };
      digitalWrite(this->assign[0],pattern[pattern_step][0]?HIGH:LOW);
      digitalWrite(this->assign[1],pattern[pattern_step][1]?HIGH:LOW);
      digitalWrite(this->assign[2],pattern[pattern_step][2]?HIGH:LOW);
      digitalWrite(this->assign[3],pattern[pattern_step][3]?HIGH:LOW);
      wait=wait_reset;
      if(0<forward){
        pattern_step++;
      }
      if(0>forward){
        pattern_step--;
      }
      if(pattern_step>=8){
        pattern_step=0;
      }else if(pattern_step<0){
        pattern_step=7;
      }
    }  
  }
};

Wheel wr,wl;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  wr.setup(2,3,4,5);
  wl.setup(6,7,8,9);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {

  wr.loop();
  wl.loop();
  delay(1);
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    int inputchar = text[0];
    if('o'==inputchar){
     wr.front();
    }
    if('p'==inputchar){
      wr.back();
    }
    if('k'==inputchar){
      wl.front();
    }
    if('l'==inputchar){
      wl.back();
    }
  }  
}
