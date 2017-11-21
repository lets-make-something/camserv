#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(14, 15); // CE, CSN
const byte address[6] = "00001";
int sleep=0;
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

  void throttle(int speed){
    if(speed>10){
      speed=10;
    }else if(speed<-10){
      speed=-10;
    }
    if(0==speed){
      wait_reset=10;
      forward=0;
    }else if(0<speed){
      wait_reset=10-speed;
      forward=1;
    }else{
      wait_reset=10+speed;
      forward=-1;
    }
  }

  void loop(){
    wait--;
    if(wait<=0){
#if 1
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
#else
      const char pattern[8][4]={
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1},
        {1,0,0,0},
        {0,1,0,0},
        {0,0,1,0},
        {0,0,0,1},
      };
#endif
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
  sleep=0;
  // put your setup code here, to run once:
  Serial.begin(9600);
  wr.setup(2,3,4,5);
  wl.setup(6,7,8,9);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void command(char *cmd){
  int inputchar = cmd[0];
  int bs=sleep;
  sleep=0;
  if('q'==inputchar){
    wr.throttle(10);
  }else if('w'==inputchar){
    wl.throttle(-10);
  }else if('a'==inputchar){
    wr.throttle(0);
  }else if('s'==inputchar){
    wl.throttle(0);
  }else if('z'==inputchar){
    wr.throttle(-10);
  }else if('x'==inputchar){
    wl.throttle(10);
  }else if('o'==inputchar){
    wr.front();
  }else if('p'==inputchar){
    wr.back();
  }else if('k'==inputchar){
    wl.front();
  }else if('l'==inputchar){
    wl.back();
  }else{
    sleep=bs;
  }
}

void loop() {
  sleep++;
  if(1000<sleep){
    wr.throttle(0);
    wl.throttle(0);
  }

  wr.loop();
  wl.loop();
  delay(1);
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
    command(text);
  }
  int input;
  input = Serial.read();
  if(input != -1 ){
    char text[] = " \n";
    text[0]=input;
    command(text);
  }
}
