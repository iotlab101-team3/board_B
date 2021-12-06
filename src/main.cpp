#include <Arduino.h>
#include <SSD1306.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define RELAY  15
#define speakerpin 14
#define bpmControl_MSB 12
#define bpmControl_LSB 13
#define buttonSW 2
#define ModeSW 0

SSD1306    display(0x3c, 4, 5, GEOMETRY_128_32);

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
unsigned int bitCount = 0;

int j = 0;

volatile int lastEncoder = 0;
volatile long encoderValue = 0;

const char*         ssid = "SK_WiFiGIGA4AB4"; //"SK_WiFiGIGA4AB4";  희정 : KT_GiGA_2G_1F1E  연빈: SK_WiFiGIGA4AB4
const char*         password = "2009024098"; // "2009024098"; 희정 : dcgb2ed245       연빈: 2009024098
const char*         mqttServer = "3.84.34.84";
const int           mqttPort = 1883;
const char* topic = "deviceid/team3_b/cmd/angle_b";

unsigned long       pubInterval = 5000;
unsigned long       lastPublished = - pubInterval;

int basicMode4[4] = {5,5,3,5}; // closeHH closeHH snare CloseHH closeHH closeHH snare CloseHH
int basicMode3[3] = {5,5,3}; // closeHH closeHH snare
int basicMode2[2] = {5,3}; // closeHH snare

int fillInMode4[10] = {5,5,3,5,5,5,3,3,3,1};
int fillInMode3[8] = {5,5,3,5,3,3,3,1};
int fillInMode2[6] = {5,3,5,3,3,1};

int dudududu4[8] = {3,3,1,1,2,2,6,6};
int dudududu3[6] = {3,3,1,2,6,6};
int dudududu2[4] = {3,1,2,6};

int currentBit[16] = {};
int currentDrum = 0;


WiFiClient      espClient;
PubSubClient    client(espClient);

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
  rhythmcount++;
  if (rhythmcount > 3)
  {
    rhythmcount = 1;
  }
}

IRAM_ATTR void buttonClickedHard() {
  checkMode++; 
  if (checkMode > 4)
  {
    checkMode = 0;
  }
  // checkMode 0:off  1:기본비트  2:변형비트1  3: 변형비트2
}

void callback(char* topic, byte* payload, unsigned int length) {
    
    unsigned int i=0;
    String Message = "";
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    while (i < length){
      Message += (char)payload [i++];
      currentDrum += (int)payload [i++];
    } 
    Serial.println();
    Serial.println(Message);

    if(checkMode==2 && checkMode==3){
      if(currentBit[bitCount] != currentDrum) digitalWrite(RELAY, 1);
      else digitalWrite(RELAY, 0);
      Serial.println(currentBit[bitCount]);
      bitCount++; 
      if(checkMode==2){
        if(bitCount >= rhythm + 6) bitCount = 0;
      }
      else if(bitCount >= rhythm + 4) bitCount = 0;
      digitalWrite(RELAY, 0);
    }

}

void copyarray(int from[], int to[], int n)
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
    pinMode(ModeSW, INPUT_PULLUP);
    pinMode(bpmControl_MSB, INPUT_PULLUP);
    pinMode(bpmControl_LSB, INPUT_PULLUP);
    attachInterrupt(buttonSW, buttonClicked, FALLING);
    attachInterrupt(ModeSW, buttonClickedHard, FALLING);
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
  if((encoderValue >= 60) && (encoderValue <= 180))
  {
    switch(rhythmcount) { 
        case 1 :
            rhythm = 4;
            memset(currentBit, 0, sizeof(currentBit));
            if(checkMode == 1) copyarray(basicMode4, currentBit, sizeof(basicMode4));
            else if(checkMode == 2) copyarray(fillInMode4, currentBit, sizeof(fillInMode4));
            else if(checkMode == 3) copyarray(dudududu4, currentBit, sizeof(dudududu4));
            break;
        case 2 :
            rhythm = 3;
            memset(currentBit, 0, sizeof(currentBit));
            if(checkMode == 1) copyarray(basicMode3, currentBit, sizeof(basicMode3));
            else if(checkMode == 2) copyarray(fillInMode3, currentBit, sizeof(fillInMode3));
            else if(checkMode == 3) copyarray(dudududu3, currentBit, sizeof(dudududu3));
            break;
        case 3 :
            rhythm = 2;
            memset(currentBit, 0, sizeof(currentBit));
            if(checkMode == 1) copyarray(basicMode2, currentBit, sizeof(basicMode2));
            else if(checkMode == 2) copyarray(fillInMode2, currentBit,  sizeof(fillInMode2));
            else if(checkMode == 3) copyarray(dudududu2, currentBit, sizeof(dudududu2));
            break;
    }

    bpm = encoderValue;
    bpm_delay = (60 / (float)encoderValue) * 1000 - 50;

     for(j = 60; j < 181; j++)
    {
      bpm_delay -= 1;
    }

    if (count%rhythm == 1) { //첫 박마다 소리 출력
      tone(speakerpin, first);
      delay(50);
      if(checkMode == 1){
        if(currentBit[bitCount] != currentDrum) digitalWrite(RELAY, 1);
        else digitalWrite(RELAY, 0);
        Serial.println(currentBit[bitCount]);
        bitCount++; 
        if(bitCount >= rhythm) bitCount = 0;
      }
      noTone(speakerpin);
      delay(bpm_delay);
      digitalWrite(RELAY, 0);
      count++; //count 값 증가
    }
    else { //첫박을 제외한 다른 박 소리 출력
        tone(speakerpin, other);
        delay(50);
        if(checkMode == 1){
          if(currentBit[bitCount] != currentDrum) digitalWrite(RELAY, 1);
          else digitalWrite(RELAY, 0);
          Serial.println(currentBit[bitCount]);
          bitCount++; 
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
    delay(1000);
  }
} 
