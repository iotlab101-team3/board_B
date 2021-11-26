#include <Arduino.h>
#include <SSD1306.h>

SSD1306    display(0x3c, 4, 5, GEOMETRY_128_32);

 #define speakerpin 5
 #define bpmControl A2
 #define button1 2
 int first = 1319; //메트로놈 첫박 소리 높이
 int other = 1047; //메트로놈 다른 소리 높이
 float bpm; //가변저항 입력값 저장
 float bpm1;
 float bpm2; //가변저항에 따른 박자 속도 계산
 int count = 1;
 int rhythm = 4; //박자 계산(ex 4분의 4박자, 4분의 3박자...)
 int buttonstate1 = 0;
 int prevstate1 = 0;
 int rhythmcount = 1; //박자 조절 스위치

 void setup() {
   pinMode(speakerpin, OUTPUT);
   pinMode(bpmControl, INPUT);
   pinMode(button1, INPUT); //입력, 출력 설정
   
   Serial.begin(115200);
   display.init();
   display.flipScreenVertically();
   display.setFont(ArialMT_Plain_16);
   display.drawString(10, 10, "Hello World");
   display.display();
 }

 void loop() {
   bpm = analogRead(bpmControl); //가변저항 값 저장
   bpm = map(bpm, 0, 1023, 60, 180); //0부터 1023까지의 가변저항 값을 60부터 180으로 조절
   bpm1 = 60/bpm;
   bpm2 = bpm1*850; //가변저항 값에 따라 메트로놈 속도 변환
   buttonstate1 = digitalRead(button1); //버튼 값 저장
 
   if (count%rhythm == 1) { //첫 박마다 소리 출력
     tone(speakerpin, first, 150);
     delay(150);
     noTone(speakerpin);
     delay(bpm2);
     count++; //count 값 증가
   }
   else { //첫박을 제외한 다른 박 소리 출력
     tone(speakerpin, other, 150);
     delay(150);
     noTone(speakerpin);
     delay(bpm2);
     count++; //count 값 증가
   }

   if (buttonstate1 != prevstate1) { //버튼 디바이싱
     if (buttonstate1 == 1) {
       rhythmcount++;
       if (rhythmcount > 3) {
         rhythmcount = 1;
       }
     }
     else {
     }
     prevstate1 = buttonstate1;
   }

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

   Serial.print("rhythm : "); //시리얼 모니터 출력
   Serial.print(rhythm);
   Serial.print("/4,  bpm : ");
   Serial.println(bpm);
   delay(100);

 } 
