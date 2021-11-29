#include <Arduino.h>
#include <SSD1306.h>
#define speakerpin 5
#define bpmControl_MSB 12
#define bpmControl_LSB 13
#define buttonSW 2

SSD1306    display(0x3c, 4, 5, GEOMETRY_128_32);

int first = 1319; //메트로놈 첫박 소리 높이
int other = 1047; //메트로놈 다른 소리 높이
float bpm;
// float bpm1;
// float bpm2; //가변저항에 따른 박자 속도 계산
int count = 1;
int rhythm = 4; //박자 계산(ex 4분의 4박자, 4분의 3박자...)
int buttonstate = 0;
int prevstate = 1;
int rhythmcount = 1; //박자 조절 스위치

volatile int lastEncoder = 0;
volatile long encoderValue = 0;

IRAM_ATTR void handleRotary() {
  int MSB = digitalRead(bpmControl_MSB);
  int LSB = digitalRead(bpmControl_LSB);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoder << 2) | encoded;
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;
  lastEncoder = encoded;
  if(encoderValue > 255){
    encoderValue = 255;
  }else if(encoderValue < 0){
    encoderValue = 0;
  }
}

IRAM_ATTR void buttonClicked() {
  //Serial.println("pushed");
  /// 버튼이 눌릴 때 리듬 카운트 값을 바꿔주면 즉시 리듬의 변화를 줄 수 있음.
  rhythmcount++;
  if (rhythmcount > 3)
  {
    rhythmcount = 1;
  }
}


void setup() {
    Serial.begin(115200);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.drawString(10, 10, "TEAM3");
    display.display();

    pinMode(buttonSW, INPUT_PULLUP);
    pinMode(bpmControl_MSB, INPUT_PULLUP);
    pinMode(bpmControl_LSB, INPUT_PULLUP);
    attachInterrupt(buttonSW, buttonClicked, FALLING);
    attachInterrupt(bpmControl_MSB, handleRotary, CHANGE);
    attachInterrupt(bpmControl_LSB, handleRotary, CHANGE);
}

void loop() {
  if((encoderValue >= 60) && (encoderValue <= 180))
  {
    bpm = (60 / (float)encoderValue) * 850;
    
    /* bpm1 = 60 / (float)encoderValue;
    bpm2 = bpm1 * 850; //가변저항 값에 따라 메트로놈 속도 변환
    bpm2 = bpm1 * 1000; */

    /// 버튼 인터럽트에서 리듬 카운트를 함으로써 버튼 값 저장을 할 필요가 없어짐. 
    //buttonstate = digitalRead(buttonSW); //버튼 값 저장
    // Serial.println(buttonstate);

    if (count%rhythm == 1) { //첫 박마다 소리 출력
      tone(speakerpin, first, 150);
      delay(150);
      noTone(speakerpin);
      delay(bpm);
      count++; //count 값 증가
    }
    else { //첫박을 제외한 다른 박 소리 출력
        tone(speakerpin, other, 150);
        delay(150);
        noTone(speakerpin);
        delay(bpm);
        count++; //count 값 증가
    }

    /// 그래서 버튼 state 관련된 코드를 모두 지우고 switch문 만 남겨뒀음. 
    switch(rhythmcount) { //버튼 누른 횟수에 따른 박자 계산(ex 4분의 4박자, 4분의 3박자...)
        case 1 :
            rhythm = 4;
            break;
        case 2 :
            rhythm = 3;
            break;
        case 3 :
            rhythm = 2;
            break;
    }

    Serial.print("\nrhythm : "); //시리얼 모니터 출력
    Serial.print(rhythm); 
    Serial.println("/4");
    Serial.print("bpm : ");
    Serial.println(bpm); 
    delay(50); 

    display.init();
    display.flipScreenVertically();
    display.drawString(10, 10, String(rhythm) + "/4 BPM: " + String(bpm));
    display.display();
    delay(50); 
  }
  else
  {
    Serial.println("\nLoading..  NOW - " + String(encoderValue));

    display.init();
    display.flipScreenVertically();
    display.drawString(10, 10, "Loading..");
    display.display();  
    delay(1000);
  }
} 
