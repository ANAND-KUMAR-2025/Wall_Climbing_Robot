#include "ESP8266.h"
#include "RemoteControl.h"
#include "Impellor.h"

#define ESP_ENABLE 2

#define ESC_PIN 10
#define EN_A_PIN 3
#define IN_1_PIN 7
#define IN_2_PIN 8
#define IN_3_PIN 13
#define IN_4_PIN 12
#define EN_B_PIN   11

#define BAUDRATE 115200

const char* ssid = "wall climbing";
const char* pwd = "12345678";
const int channel = 1;
const int enc = WIFI_WPA2;

ESP8266* esp;
RemoteControl* remote;
Impellor* impellor;

void waitTillState(espState state)
{
  do
  {
    esp->Tick();
  }
  while(esp->getCurrentState() != state);
}

void setup() 
{
  // Reset ESP
  pinMode(ESP_ENABLE, OUTPUT);
  digitalWrite(ESP_ENABLE, LOW);
  // Create impellor
  impellor = new Impellor(ESC_PIN, EN_A_PIN, IN_1_PIN, IN_2_PIN, IN_3_PIN, IN_4_PIN, EN_B_PIN);
  impellor -> Init();
  Serial.begin(BAUDRATE);
  // Create module and wait till ready
  esp = new ESP8266(&Serial); 
  digitalWrite(ESP_ENABLE, HIGH);
  waitTillState(STATE_READY);
  // Disable local echo
  esp->setLocalEcho(false);
  waitTillState(STATE_OK);
  // Configure AP
  esp->configureAP(ssid, pwd, channel, enc);
  waitTillState(STATE_OK);
  // Set mux
  esp->setMux(true);
  waitTillState(STATE_OK);
  // Set server
  esp->setServer(1234);
  waitTillState(STATE_OK);
  remote = new RemoteControl(esp);
}

void loop() 
{
  remote->Tick();
  impellor->Tick();
  if (impellor->IsDoneMoving())
  {
    Movement move = remote->getNextMovement();
    if (move.distance > 0)
    {
      switch (move.mode)
      {
        case FORWARD:
          //Serial.println("F");
          impellor->MoveForward(move.distance);
          break;
        case RIGHT:
          //Serial.println("R");
          impellor->TurnRight(move.distance);
          break;
        case BACKWARD:
          //Serial.println("B");
          impellor->MoveBackward(move.distance);
          break;
        case LEFT:
          //Serial.println("L");
          impellor->TurnLeft(move.distance);
          break;
        case SPEED:
          //Serial.println("S");
          impellor->StartImpellor(move.distance);
          break;
      }
    }
  } 
  else
  {
    Movement move = remote->peakNextMovement();
    if (move.mode == SPEED)
    {
      //Serial.println("S");
      impellor->StartImpellor(move.distance);
    }
  }
}

