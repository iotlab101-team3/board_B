#include <Arduino.h>
#include <SSD1306.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define RELAY  15
#define speakerpin 14
#define bpmControl_MSB 12
#define bpmControl_LSB 13
#define buttonSW 2

SSD1306    display(0x3c, 4, 5, GEOMETRY_128_32);

WiFiClient      espClient;
PubSubClient    client(espClient);

int first = 1319; //메트로놈 첫박 소리 높이
int other = 1047; //메트로놈 다른 소리 높이
float bpm;
float bpm_delay;
int count = 1;
int rhythm = 4; //박자 계산
int buttonstate = 0;
int prevstate = 1;
int rhythmcount = 1; //박자 조절 스위치
int checkMode = 0; // 체크모드 진입 카운트 스위치
int check_flag = 0;
int bitCount = 0;

int j = 0;

volatile int lastEncoder = 0;
volatile long encoderValue = 0;

const char*         ssid = "Gogle"; //"SK_WiFiGIGA4AB4";  희정 : KT_GiGA_2G_1F1E  연빈: SK_WiFiGIGA4AB4
const char*         password = "20010228"; // "2009024098"; 희정 : dcgb2ed245       연빈: 2009024098
const char*         mqttServer = "3.84.34.84";
const int           mqttPort = 1883;
const char* topic = "deviceid/team3/evt/angle";

unsigned long       pubInterval = 5000;
unsigned long       lastPublished = - pubInterval;

char basicMode4[8] = {'1','1','3','1','1','1','3','1'}; // 0x49 = 1 
char basicMode3[6] = {'1','1','3','1','1','3'}; // closeHH closeHH snare
char basicMode2[4] = {'1','3','1','3'}; // closeHH snare

char modifyMode4[8] = {'1','1','3','3','1','3','1','3'};
char modifyMode3[6] = {'1','1','3','3','1','3'};
char modifyMode2[4] = {'1','3','3','1'};

char dudududu4[8] = {'3','3','4','4','5','5','6','6'};
char dudududu3[6] = {'3','3','4','5','6','6'};
char dudududu2[4] = {'3','4','5','6'};

char currentBit[8] = {};
char currentDrum;


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
  }
  else if(encoderValue < 0){
    encoderValue = 0;
  }
}

IRAM_ATTR void buttonClicked() {
  rhythmcount++;
  bitCount = 0;
  if (rhythmcount > 3)
  {
    rhythmcount = 1;
    checkMode++;
    Serial.printf("현재 모드: "); Serial.println(checkMode);
    if(checkMode == 1) check_flag = 1;
    else if(checkMode == 4) {
      checkMode = 0;
      check_flag = 0;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
    
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for(int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        currentDrum = (char)payload[0];
    }
   
    Serial.println();
    Serial.print("현재 드럼 소리: "); Serial.println(currentDrum); 
}

void copyarray(char from[], char to[], int n)
{
  for(int i = 0; i < n; i++) to[i] = from[i];
}

void setup() {
  Serial.begin(115200);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 10, "TEAM3");
  display.display();

  pinMode(RELAY, OUTPUT);
  pinMode(speakerpin, OUTPUT);
  pinMode(buttonSW, INPUT_PULLUP);
  pinMode(bpmControl_MSB, INPUT_PULLUP);
  pinMode(bpmControl_LSB, INPUT_PULLUP);
  attachInterrupt(buttonSW, buttonClicked, FALLING);
  attachInterrupt(bpmControl_MSB, handleRotary, CHANGE);
  attachInterrupt(bpmControl_LSB, handleRotary, CHANGE);

  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("team3_b")) {
      Serial.println("connected");  
    }
    else {
      Serial.print("failed with state "); Serial.println(client.state());
      delay(2000);
    }
  }
  client.subscribe(topic);
  client.setCallback(callback);
}

void loop() {
  client.loop();
  if((encoderValue >= 30) && (encoderValue <= 180))
  {
    switch(rhythmcount) { 
      case 1 :
        rhythm = 4;
        if(check_flag == 1){
          memset(currentBit, 0, sizeof(currentBit));
          if(checkMode == 1) copyarray(basicMode4, currentBit, rhythm * 2);
          else if(checkMode == 2) copyarray(modifyMode4, currentBit, rhythm * 2);
          else if(checkMode == 3) copyarray(dudududu4, currentBit, rhythm * 2);
        }
        break;
      case 2 :
        rhythm = 3;
        if(check_flag == 1){
          memset(currentBit, 0, sizeof(currentBit));
          if(checkMode == 1) copyarray(basicMode3, currentBit, rhythm * 2);
          else if(checkMode == 2) copyarray(modifyMode3, currentBit, rhythm * 2);
          else if(checkMode == 3) copyarray(dudududu3, currentBit, rhythm * 2);
        }
        break;
      case 3 :
        rhythm = 2;
        if(check_flag == 1){
          memset(currentBit, 0, sizeof(currentBit));
          if(checkMode == 1) copyarray(basicMode2, currentBit, rhythm * 2);
          else if(checkMode == 2) copyarray(modifyMode2, currentBit,  rhythm * 2);
          else if(checkMode == 3) copyarray(dudududu2, currentBit, rhythm * 2);
        }
        break;
    }
    bpm = encoderValue;
    bpm_delay = (60 / (float)encoderValue) * 1000 - 50;

    for(j = 30; j < 181; j++)
    {
      bpm_delay -= 1;
    }

    if (count%rhythm == 1) { //첫 박마다 소리 출력
      tone(speakerpin, first);
      delay(50);
      if(check_flag == 1){
        if(currentBit[bitCount] != currentDrum) digitalWrite(RELAY, 1);
        else digitalWrite(RELAY, 0);
        currentDrum = 0;
        Serial.println(currentBit[bitCount]);
        bitCount++; 
      }
      noTone(speakerpin);
      delay(bpm_delay);
      digitalWrite(RELAY, 0);
      count++; //count 값 증가
    }
    else { //첫박을 제외한 다른 박 소리 출력
      tone(speakerpin, other);
      delay(50);
      if(check_flag == 1){
        if(currentBit[bitCount] != currentDrum) digitalWrite(RELAY, 1);
        else digitalWrite(RELAY, 0);
        currentDrum = 0;
        Serial.println(currentBit[bitCount]);
        bitCount++; 
        if(bitCount >= rhythm*2) bitCount = 0;
      }
      noTone(speakerpin);
      delay(bpm_delay);
      digitalWrite(RELAY, 0);
      count++; //count 값 증가
    } 

    Serial.print("\nrhythm : "); 
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
    delay(800);
  }
} 
