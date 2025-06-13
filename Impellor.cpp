#include "Impellor.h"

#include <Arduino.h>
#include <Servo.h>

Impellor::Impellor(int escPin, int enAPin, int in1Pin, int in2Pin, int in3Pin, int in4Pin, int enBPin): m_escPin(escPin), m_enAPin(enAPin), m_enBPin(enBPin), m_in1Pin(in1Pin), m_in2Pin(in2Pin), m_in3Pin(in3Pin), m_in4Pin(in4Pin), m_currentPos(0), m_destPos(0), m_impellorSpeed(0), m_esc(NULL), m_state(IMPELLOR_STATE_UNKNOWN), m_prevMillis(0)
{
}

Impellor::~Impellor()
{
	if (m_esc != NULL)
		delete m_esc;
}

void Impellor::Init()
{
	m_esc = new Servo();
	m_esc -> attach(m_escPin);
	m_impellorSpeed = DEF_MINSPEED;
	m_esc -> writeMicroseconds(DEF_MINSPEED);
	pinMode(m_enAPin, OUTPUT);
	pinMode(m_enBPin, OUTPUT);
	pinMode(m_in1Pin, OUTPUT);
	pinMode(m_in2Pin, OUTPUT);
	pinMode(m_in3Pin, OUTPUT);
	pinMode(m_in4Pin, OUTPUT);
	analogWrite(m_enAPin, 0);
	analogWrite(m_enBPin, 0);  
	m_state = IMPELLOR_STATE_INIT;
	m_prevMillis = millis();
}

void Impellor::Tick()
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	long currentMillis = millis();
	int dTime = currentMillis - m_prevMillis;
	m_currentPos += dTime;
	CheckState();
	m_prevMillis = currentMillis;
}

void Impellor::StartImpellor(int speedPercent)
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	switch (m_state)
	{
		case IMPELLOR_STATE_STOPPED:
			IMPELLOR_STATE_INIT;
			m_destPos = 1000;
			break;
		default:
			break;
	}
	m_impellorSpeed = map(speedPercent, 0, 100, DEF_MINSPEED, DEF_MAXSPEED);
	if (speedPercent < 10)
		StopImpellor();
}

void Impellor::StopImpellor()
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	m_state = IMPELLOR_STATE_STOPPED;
	m_currentPos = 0;
}

void Impellor::MoveForward(int distance)
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	m_state = IMPELLOR_STATE_FORWARD;
	m_currentPos = 0;
	m_destPos = distance;
	digitalWrite(m_in1Pin, 0);
	digitalWrite(m_in2Pin, 1);
	digitalWrite(m_in3Pin, 0);
	digitalWrite(m_in4Pin, 1);  
	analogWrite(m_enAPin, 255);
	analogWrite(m_enBPin, 255);
	m_prevMillis = millis();
}

void Impellor::MoveBackward(int distance)
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	m_state = IMPELLOR_STATE_BACKWARD;
	m_currentPos = 0;
	m_destPos = distance;
	digitalWrite(m_in1Pin, 1);
	digitalWrite(m_in2Pin, 0);
	digitalWrite(m_in3Pin, 1);
	digitalWrite(m_in4Pin, 0);  
	analogWrite(m_enAPin, 255);
	analogWrite(m_enBPin, 255);
	m_prevMillis = millis();
}

void Impellor::TurnLeft(int distance)
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	m_state = IMPELLOR_STATE_LEFT;
	m_currentPos = 0;
	m_destPos = distance;
	digitalWrite(m_in1Pin, 1);
	digitalWrite(m_in2Pin, 0);
	digitalWrite(m_in3Pin, 0);
	digitalWrite(m_in4Pin, 1);  
	analogWrite(m_enAPin, 255);
	analogWrite(m_enBPin, 255);
	m_prevMillis = millis();
}

void Impellor::TurnRight(int distance)
{
	if (IMPELLOR_STATE_UNKNOWN)
		return;
	m_state = IMPELLOR_STATE_RIGHT;
	m_currentPos = 0;
	m_destPos = distance;
	digitalWrite(m_in1Pin, 0);
	digitalWrite(m_in2Pin, 1);
	digitalWrite(m_in3Pin, 1);
	digitalWrite(m_in4Pin, 0);  
	analogWrite(m_enAPin, 255);
	analogWrite(m_enBPin, 255);
	m_prevMillis = millis();
}

bool Impellor::IsDoneMoving()
{
	return (m_state == IMPELLOR_STATE_DONE);
}

void Impellor::CheckState()
{
	if (m_currentPos >= m_destPos)
	{
		switch (m_state)
		{
			case IMPELLOR_STATE_INIT:
				m_esc -> writeMicroseconds(m_impellorSpeed);
				m_state = IMPELLOR_STATE_DONE;
				break;
				
			case IMPELLOR_STATE_FORWARD:
			case IMPELLOR_STATE_BACKWARD:
			case IMPELLOR_STATE_LEFT:
			case IMPELLOR_STATE_RIGHT:
				analogWrite(m_enBPin, 0);
				analogWrite(m_enAPin, 0);  
				m_state = IMPELLOR_STATE_DONE;
				break;
	
			default:
				break;
		}
	}
	else if (m_state == IMPELLOR_STATE_INIT || m_state == IMPELLOR_STATE_STOPPED)
	{
		m_esc -> writeMicroseconds(DEF_MINSPEED);
	}
	else
	{
		m_esc -> writeMicroseconds(m_impellorSpeed);
	}	
}