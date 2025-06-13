#include "RemoteControl.h"

#include "ESP8266.h"
#include "Arduino.h"

RemoteControl::RemoteControl(ESP8266* esp): m_esp(esp), m_movementsLength(-1), m_currentMovement(0), m_movements(NULL), m_clientState(IDLE)
{
}

RemoteControl::~RemoteControl()
{
	if (m_movements != NULL)
		delete m_movements;
}

void RemoteControl::Tick()
{
	m_esp->Tick();
	switch(m_clientState)
	{
	case IDLE:
		if (m_esp->getCurrentState() == STATE_RECIEVING)
			m_clientState = SENDING;
		break;
		
	case SENDING:
		//if (m_esp->getCurrentState() == STATE_OK)
		{
			if (m_esp->getClientState(m_esp->getCurrentId()) == CLIENT_WAITING_FOR_RESPONSE)
			{
				char* request;
				int requestLength = m_esp->getRecievedMessage(request);
				int endPos = 0;
				for (; endPos < requestLength; ++endPos)
				{
					if (request[endPos] == ':')
						break;
				}
				char* tmp = new char[endPos];
				memcpy(tmp, request, endPos);
				int movements = atoi(tmp);
				delete tmp;
				if (m_movementsLength > 0 && m_currentMovement < m_movementsLength)
				{
					int remainingLength = m_movementsLength - m_currentMovement;
					Movement* tmpMov = new Movement[remainingLength+movements];
					memcpy(tmpMov, &m_movements[m_currentMovement], remainingLength * sizeof(Movement));
					delete m_movements;
					m_movements = tmpMov;
					m_movementsLength = remainingLength;
					movements += m_movementsLength;
				}
				else
				{
					if (m_movements != NULL)
						delete m_movements;
					m_movements = new Movement[movements];
					m_movementsLength = 0;
				}
				for (; m_movementsLength < movements; ++m_movementsLength)
				{
					int startPos = endPos+1;
					endPos = -1;
					for (int i = startPos; i < requestLength; ++i)
					{
						if (request[i] == ',')
						{
							endPos = i;
							i = requestLength;
						}
					}
					if (endPos == -1)
						endPos = requestLength;
					switch(request[startPos])
					{
						case 'F':
							m_movements[m_movementsLength].mode = FORWARD;
							break;
						case 'B':
							m_movements[m_movementsLength].mode = BACKWARD;
							break;
						case 'L':
							m_movements[m_movementsLength].mode = LEFT;
							break;
						case 'R':
							m_movements[m_movementsLength].mode = RIGHT;
							break;
						case 'S':
							m_movements[m_movementsLength].mode = SPEED;
					}
					int length = (endPos - startPos) - 1;
					tmp = new char[length];
					memcpy(tmp, &request[startPos+1], endPos);
					m_movements[m_movementsLength].distance = atoi(tmp);
					delete tmp;
				}
				m_currentMovement = 0;
				m_esp->prepareSendData(m_esp->getCurrentId(), 2);
				//m_esp->prepareSendData(0, 2);
				m_clientState = WAITING_RESPONSE;
			}
		}
		break;
		
	case WAITING_RESPONSE:
		if (m_esp->getClientState(m_esp->getCurrentId()) == CLIENT_READY_TO_RECIEVE)
		{
            m_esp->sendData("OK");
			m_clientState = IDLE;
		}
		break;
	}
}
	
Movement RemoteControl::getNextMovement()
{
	if (m_movementsLength > m_currentMovement)
	{
		Movement tmp = m_movements[m_currentMovement];
		++m_currentMovement;
		return tmp;
	}
	else
	{
		if (m_movements != NULL)
		{
			delete m_movements;
			m_movements = NULL;
			m_movementsLength = -1;
			m_currentMovement = 0;
		}
		Movement tmp;
		tmp.distance = 0;
		tmp.mode = FORWARD;
		return tmp;
	}
}

Movement RemoteControl::peakNextMovement()
{
	if (m_movementsLength > m_currentMovement)
	{
		return m_movements[m_currentMovement];
	}
	else
	{
		if (m_movements != NULL)
		{
			delete m_movements;
			m_movements = NULL;
			m_movementsLength = -1;
			m_currentMovement = 0;
		}
		Movement tmp;
		tmp.distance = 0;
		tmp.mode = FORWARD;
		return tmp;
	}
}

int RemoteControl::getNextMovements(Movement* &movements)
{
	movements = m_movements;
	return m_movementsLength;
}

int RemoteControl::getNextMovementsCount()
{
	return m_movementsLength - m_currentMovement;
}