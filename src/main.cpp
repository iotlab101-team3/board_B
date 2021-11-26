//메카솔루션 메트로놈 프로젝트//
#include <LiquidCrystal.h>
LiquidCrystal lcd(9,8,4,5,6,7);
const int sw1 = 13;//박자수 변동
const int sw2 = 12;//박자속도 감소
const int sw3 = 11;//박자속도 증가
const int buzzer = 10;
//핀번호를 바꿀 때, 다른 핀들과 번호가 겹치지 않도록 주의바랍니다!(윗부분만 수정)//

byte userFont1[8] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B00100,
  B01100,
  B01100,
  B00000 };//4분음표
byte userFont2[8] = {
  B00000,
  B00110,
  B00100,
  B00100,
  B00100,
  B01100,
  B01100,
  B00000 };//8분음표
byte userFont3[8] = {
  B00000,
  B01111,
  B01001,
  B01001,
  B01001,
  B11011,
  B11011,
  B00000 };//8분음표x2 (한박)
byte userFont4[8] = {
  B01000,
  B01100,
  B01110,
  B01111,
  B01110,
  B01100,
  B01000,
  B00000 };//▶

int cnt = 0;
float tempo_time;
unsigned int tempo_speed = 80; //LCD에 표기되는 박자표기
unsigned int tempo_cnt = 0;
unsigned long timer=750;
unsigned long timer1;
void setup()
{
  pinMode(sw1,INPUT);
  pinMode(sw2,INPUT);
  pinMode(sw3,INPUT);
  pinMode(buzzer,OUTPUT);
  
  lcd.createChar(1,userFont1);
  lcd.createChar(2,userFont2);
  lcd.createChar(3,userFont3);
  lcd.createChar(4,userFont4);
  lcd.begin(16,2);
  
  Serial.begin(9600);
}
void loop()
{
  int beat = digitalRead(sw1);         //박자수 변동
  int temp_up = digitalRead(sw3);     //박자속도 증가
  int temp_down = digitalRead(sw2);  //박자속도 감소
  
  lcd.setCursor(0,0);
  lcd.print("Tempo:");
  int tempo_speed100 = tempo_speed / 100;
  int tempo_speed10 = tempo_speed % 100 / 10;
  int tempo_speed1 = tempo_speed % 10;
  
  lcd.setCursor(6,0);
  lcd.print(tempo_speed100);
  lcd.setCursor(7,0);
  lcd.print(tempo_speed10);
  lcd.setCursor(8,0);
  lcd.print(tempo_speed1);
  
  lcd.setCursor(12,0);  
  quickness();     //빠르기말 표시
  
  if(cnt == 9)
    cnt = 0;
  if(cnt % 3 == 2)
    tempo_time = 60.00 / tempo_speed * 500;//tempo_timequick //LCD표기를 초속도로 나타낸 공식(8분음표 한 박)
  else
    tempo_time = 60.00 / tempo_speed * 1000;//LCD표기를 초속도로 나타낸 공식(4분음표)
  
  if(millis() - timer1 >= 220)
  {
   if(beat == HIGH) // 박자증감표시(음표)
     cnt = cnt + 1;
   if(temp_down == HIGH)
    tempo_speed = tempo_speed - 2; // -2씩 감소
  if(temp_up == HIGH)
    tempo_speed = tempo_speed + 3; // +3씩 증가
    timer1 = millis();
    
    beatcheck();   //음표박자표시
  }//스위치 전용 딜레이
  int state;  //패턴 박자수
  if(cnt < 3)
  {
    pattern4_4();   //4분의 4박자 패턴표시
    state = 4;
    if(cnt == 2)
      state = 8;
  }
  else if(cnt < 6)
  {
    pattern4_3();   //4분의 3박자 패턴표시
    state = 3;
  }
  else
  {
    pattern4_2();   //4분의 2박자 패턴표시
    state = 2;
  }
  
  if(millis() - timer >= tempo_time)
  {
    timer = millis();
    
    if(tempo_cnt%state == state - 1)
     tone(buzzer,523,100); // 4옥타브 도
    else
     tone(buzzer,261,100); // 3옥타브 도

       tempo_cnt = tempo_cnt + 1;
  }//부저와 박자카운트 전용 딜레이
  
}
//////////박자와 음표표시//////////
void beatcheck()
{
  switch(cnt) {
/*
  case 0: {
    lcd.setCursor(12,0);
    lcd.print("    "); 
  } break;
*/
  case 0: {
  lcd.setCursor(14,0);
  lcd.print("4");
  lcd.write(1);
  } break;
  case 1: {
  lcd.setCursor(14,0);
  lcd.print("4");
  lcd.write(2);
  } break;
  case 2: {
  lcd.setCursor(14,0);
  lcd.print("4");
  lcd.write(3);
  } break;
  case 3: {
  lcd.setCursor(14,0);
  lcd.print("3");
  lcd.write(1);
  } break;
  case 4: {
  lcd.setCursor(14,0);
  lcd.print("3");
  lcd.write(2);
  } break;
  case 5: {
  lcd.setCursor(14,0);
  lcd.print("3");
  lcd.write(3);
  } break;
  case 6: {
  lcd.setCursor(14,0);
  lcd.print("2");
  lcd.write(1);
  } break;
  case 7: {
  lcd.setCursor(14,0);
  lcd.print("2");
  lcd.write(2);
  } break;
  case 8: {
  lcd.setCursor(14,0);
  lcd.print("2");
  lcd.write(3);
  } break;
  }
}//음표박자표시
//////////빠르기말 표시//////////
void quickness()
{
  if(tempo_speed >= 160)
  {
    lcd.setCursor(6,1);
    lcd.print("    Vivace");
  }
  else if(tempo_speed >= 132)
  {
    lcd.setCursor(6,1);
    lcd.print("   Allegro");
  }
  else if(tempo_speed >= 108)
  {
    lcd.setCursor(6,1);
    lcd.print("Allegretto");
  }
  else if(tempo_speed >= 88)
  {
    lcd.setCursor(6,1);
    lcd.print("  Moderato");
  }
  else if(tempo_speed >= 69)
  {
    lcd.setCursor(6,1);
    lcd.print(" Andantino");
  }
  else if(tempo_speed >= 66)
  {
    lcd.setCursor(6,1);
    lcd.print("   Andante");
  }
  else if(tempo_speed >= 56)
  {
    lcd.setCursor(6,1);
    lcd.print("    Adagio");
  }
  else if(tempo_speed >= 52)
  {
    lcd.setCursor(6,1);
    lcd.print("     Lento");
  }
  else
  {
    lcd.setCursor(6,1);
    lcd.print("     Largo");
  }
}//빠르기말 표시
//////////박자패턴표시//////////
void pattern4_4()
{
  switch(tempo_cnt%4) {
   case 0: {
  lcd.setCursor(0,1);
  lcd.write(4); //"▶"
  lcd.write(62);
  lcd.write(62); 
  lcd.write(62); //">"
   } break;
   case 1: {
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.write(4); //"▶"
  lcd.write(62);
  lcd.write(62); //">"
   } break;
   case 2: {
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);  //"▶"
  lcd.write(62); //">"
  } break;
  case 3: {
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);  //"▶"
  }
 }
}//4분의 4박자 패턴표시
void pattern4_3()
{
  switch(tempo_cnt%3) {
   case 0: {
  lcd.setCursor(0,1);
  lcd.write(4); //"▶"
  lcd.write(62);
  lcd.write(62); //">"
  lcd.print(" ");
   } break;
   case 1: {
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.write(4);  //"▶"
  lcd.write(62); //">"
  lcd.print(" ");
  } break;
  case 2: {
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.write(4);
  lcd.write(4);  //"▶"
  lcd.print(" ");
  } break;
 }
}//4분의 3박자 패턴표시
void pattern4_2()
{
  switch(tempo_cnt%2) {
   case 0: {
  lcd.setCursor(0,1);
  lcd.write(4); //"▶"
  lcd.write(62); //">"
  lcd.print("  ");
   } break;
   case 1: {
  lcd.setCursor(0,1);
  lcd.write(4);
  lcd.write(4);  //"▶"
  lcd.print("  ");
  } break;
 }
}//4분의 2박자 패턴표시