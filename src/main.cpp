#include <Arduino.h>
#include <SSD1306.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define speakerpin 14
#define bpmControl_MSB 12
#define bpmControl_LSB 13
#define buttonSW 2

SSD1306    display(0x3c, 4, 5, GEOMETRY_128_32);

int first = 1319; //메트로놈 첫박 소리 높이
int other = 1047; //메트로놈 다른 소리 높이
float bpm;
float bpm_delay;
int count = 1;
int rhythm = 4; //박자 계산(ex 4분의 4박자, 4분의 3박자...)
int buttonstate = 0;
int prevstate = 1;
int rhythmcount = 1; //박자 조절 스위치

volatile int lastEncoder = 0;
volatile long encoderValue = 0;

const char*         ssid ="KT_GiGA_4C6F";
const char*         password = "0ebe01ge28";
const char*         mqttServer = "3.84.34.84";
const int           mqttPort = 1883;
const char* topic = "deviceid/team3_b/cmd/angle_b";

unsigned long       pubInterval = 5000;
unsigned long       lastPublished = - pubInterval;

WiFiClient          espClient;
PubSubClient        client(espClient);

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

void callback(char* topic, byte* payload, unsigned int length) {
    
    int i;
    String Message = "";
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    while (i < length){
      Message += (char)payload [i++];
    } 
    Serial.println();
    Serial.println(Message);
}

void setup() {
    Serial.begin(115200);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.drawString(10, 10, "TEAM3");
    display.display();

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
  if((encoderValue >= 60) && (encoderValue <= 180))
  {
    bpm = encoderValue;
    bpm_delay = (60 / (float)encoderValue) * 850;

    if (count%rhythm == 1) { //첫 박마다 소리 출력
      tone(speakerpin, first, 30);
      delay(150);
      noTone(speakerpin);
      delay((int)bpm_delay);
      count++; //count 값 증가
    }
    else { //첫박을 제외한 다른 박 소리 출력
        tone(speakerpin, other, 30);
        delay(150);
        noTone(speakerpin);
        delay((int)bpm_delay);
        count++; //count 값 증가
    }


    switch(rhythmcount) { 
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
