#include "ESP8266.h"

#include "Arduino.h"

const char* ESP8266::CMD_AT = "AT";
const char* ESP8266::CMD_ECHO = "E";
const char* ESP8266::CMD_SETAPPARAMS = "+CWSAP=";
const char* ESP8266::CMD_GETIP = "+CIFSR";
const char* ESP8266::CMD_SETMUX = "+CIPMUX=";
const char* ESP8266::CMD_SETSERVER = "+CIPSERVER=";
const char* ESP8266::CMD_SEND = "+CIPSENDBUF=";

const char* ESP8266::MSG_READY = "ready";
const char* ESP8266::MSG_OK = "OK";
const char* ESP8266::MSG_ERROR = "ERROR";
const char* ESP8266::MSG_BUSY = "BUSY";
const char* ESP8266::MSG_RECIEVE = "+IPD";
const char* ESP8266::MSG_CONNECTED = "CONNECT";
const char* ESP8266::MSG_DISCONNECTED = "CLOSED";

ESP8266::ESP8266(HardwareSerial *serial): m_serial(serial), m_state(STATE_UNKNOWN), m_currentMessageLength(0), m_currentId(-1), m_recievedData(NULL), m_recievedDataLength(0), m_overFlow(false), m_eol(false), m_maxDataLength(0)
{
	m_clientStates = new clientState[MAX_CONNECTIONS];
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
		m_clientStates[i] = CLIENT_UNKNOWN;
	m_currentMessage = new char[MAX_MESSAGESIZE];
	m_currentMessage[0] = '\0';
	m_lastInstruction = "";
}

ESP8266::~ESP8266()
{
	delete m_currentMessage;
	if (m_recievedData != NULL)
		delete m_recievedData;
}

void ESP8266::checkAT()
{
	m_state = STATE_SENDING;
	m_serial->println(CMD_AT);
	m_lastInstruction = CMD_AT;
	m_state = STATE_WAITING_FOR_RESPONSE;
}

espState ESP8266::getCurrentState()
{
	return m_state;
}

void ESP8266::setLocalEcho(bool on)
{
	m_state = STATE_SENDING;
	m_serial->print(CMD_AT);
	m_serial->print(CMD_ECHO);
	if (on)
		m_serial->println(F("1"));
	else
		m_serial->println(F("0"));
	m_lastInstruction = CMD_ECHO;
	m_state = STATE_WAITING_FOR_RESPONSE;
}

void ESP8266::configureAP(const char *ssid, const char *password, unsigned int channel, wifiMode mode)
{
	m_state = STATE_SENDING;
	m_serial->print(CMD_AT);
	m_serial->print(CMD_SETAPPARAMS);
	m_serial->print(F("\""));
	m_serial->print(ssid);
	m_serial->print(F("\",\""));
	m_serial->print(password);
	m_serial->print(F("\","));
	m_serial->print(channel);
	m_serial->print(F(","));
	m_serial->println(mode);
	m_lastInstruction = CMD_SETAPPARAMS;
	m_state = STATE_WAITING_FOR_RESPONSE;
}

void ESP8266::setMux(bool on)
{
	m_state = STATE_SENDING;
	m_serial->print(CMD_AT);
	m_serial->print(CMD_SETMUX);
	if (on)
		m_serial->println(F("1"));
	else
		m_serial->println(F("0"));
	m_lastInstruction = CMD_SETMUX;
	m_state = STATE_WAITING_FOR_RESPONSE;
}

void ESP8266::setServer(unsigned int port, bool enable)
{
	m_state = STATE_SENDING;
	m_serial->print(CMD_AT);
	m_serial->print(CMD_SETSERVER);
	if (enable)
		m_serial->print(F("1"));
	else
		m_serial->print(F("0"));
	m_serial->print(F(","));
	m_serial->println(port);
	m_lastInstruction = CMD_SETSERVER;
	m_state = STATE_WAITING_FOR_RESPONSE;
}

void ESP8266::prepareSendData(unsigned int id, unsigned int size)
{
	if (id < MAX_CONNECTIONS)
	{
		if (m_clientStates[id] == CLIENT_WAITING_FOR_RESPONSE || m_clientStates[id] == CLIENT_CONNECTED)
		{
			m_state = STATE_SENDING;
			m_serial->print(CMD_AT);
			m_serial->print(CMD_SEND);
			m_serial->print(id);
			m_serial->print(F(","));
			m_serial->println(size);
			m_currentId = id;
			m_lastInstruction = CMD_SEND;
			m_state = STATE_WAITING_FOR_RESPONSE;
		}
	}
}

void ESP8266::sendData(const char* data)
{
	if (m_currentId >= 0)
	{
		if (m_clientStates[m_currentId] == CLIENT_READY_TO_RECIEVE)
		{
			m_clientStates[m_currentId] = CLIENT_RECIEVING;
			m_state = STATE_SENDING;
			m_serial->println(data);
			m_state = STATE_WAITING_FOR_RESPONSE;
		}
	}
}

int ESP8266::getRecievedMessage(char* &result)
{
	if (m_recievedData == NULL)
		return 0;
	//m_recievedData[m_recievedDataLength] = 0;
	result = m_recievedData;
	return m_recievedDataLength;
}

void ESP8266::deleteRecievedMessage()
{
	if (m_recievedData != NULL)
		delete m_recievedData;
	m_recievedData = NULL;
	m_recievedDataLength = 0;
}

clientState ESP8266::getClientState(unsigned int id)
{
	if (id < MAX_CONNECTIONS)
		return m_clientStates[id];
	return CLIENT_UNKNOWN;
}

unsigned int ESP8266::getCurrentId()
{
	return m_currentId;
}

void ESP8266::Tick()
{
	int tempChar = m_serial->read();
	if (tempChar != 1 && m_eol)	
		m_currentMessageLength = 0;
	m_eol = false;
	while (tempChar != -1)
	{
		if (tempChar == '\n')
		{
			if (m_currentMessageLength > 0)
			{
				--m_currentMessageLength;
				m_currentMessage[m_currentMessageLength] = 0;
				processCurrentMessage();
				m_eol = true;
				return;
			}
			m_overFlow = false;
		}
		m_currentMessage[m_currentMessageLength] = (char)tempChar;
		++m_currentMessageLength;
		if (m_currentMessageLength == MAX_MESSAGESIZE)
		{
			processCurrentMessage();
			if (OVERFLOW_KEEP > 0)
			{
				memcpy(m_currentMessage, &m_currentMessage + MAX_MESSAGESIZE - OVERFLOW_KEEP - 1, MAX_MESSAGESIZE - OVERFLOW_KEEP);
			}
			m_overFlow = true;
			m_currentMessageLength = OVERFLOW_KEEP;
		}
		tempChar = m_serial->read();
	}
}

void ESP8266::processCurrentMessage()
{
	if (strcmp(m_currentMessage, MSG_OK) == 0)
	{
		m_state = STATE_OK;
		if (m_lastInstruction == CMD_SEND)
		{
			if (m_currentId >= 0)
				m_clientStates[m_currentId] = CLIENT_READY_TO_RECIEVE;
		}
		/*else if (m_lastInstruction == MSG_RECIEVE)
		{
			if (m_currentId >= 0)
				m_clientStates[m_currentId] = CLIENT_WAITING_FOR_RESPONSE;
		}*/
		return;
	}
	if (strcmp(m_currentMessage, MSG_ERROR) == 0)
	{
		m_state = STATE_ERROR;
		return;
	}
	if (strcmp(m_currentMessage, MSG_BUSY) == 0)
	{
		m_state = STATE_BUSY;
		return;
	}
	if (strcmp(m_currentMessage, MSG_READY) == 0)
	{
		m_state = STATE_READY;
		return;
	}
	int len = strlen(MSG_RECIEVE);
	if (m_currentMessageLength > len)
	{
		if (findString(MSG_RECIEVE))
		{
			m_state = STATE_RECIEVING;
			// Get id
			int startPos = len + 1;
			int endPos = startPos;
			for (; endPos < m_currentMessageLength; ++endPos)
			{
				if (m_currentMessage[endPos] == ',')
					break;
			}
			int size = endPos-startPos;
			char* tmp = new char[++size];
			memcpy(tmp, m_currentMessage+startPos, size);
			int id = atoi(tmp);
			// Get size
			delete tmp;
			if (id >= 0 && id < MAX_CONNECTIONS)
				m_clientStates[id] = CLIENT_SENDING;
			m_currentId = id;
			startPos = endPos + 1;
			endPos = startPos;
			for (; endPos < m_currentMessageLength; ++endPos)
			{
				if (m_currentMessage[endPos] == ':')
					break;
			}
			size = endPos-startPos;
			tmp = new char[size];
			memcpy(tmp, m_currentMessage+startPos, size);
			int len = atoi(tmp);
			delete tmp;
			// Get data
			// Todo: add support for fragmented data
			if (len >= 0)
			{
				if (m_recievedData != NULL)
					delete m_recievedData;
				m_recievedData = new char[++len];
				m_maxDataLength = --len;
				startPos = endPos + 1;
				/*len = m_currentMessageLength - startPos;
				if (len > m_maxDataLength)
					len = m_maxDataLength;*/
				memcpy(m_recievedData, m_currentMessage + startPos, len);
				m_recievedDataLength = len;
				//if (m_recievedDataLength == m_maxDataLength)
					m_clientStates[m_currentId] = CLIENT_WAITING_FOR_RESPONSE;
				m_recievedData[m_maxDataLength] = '\0';
				//m_serial->println(m_recievedData);
			}
			m_lastInstruction = MSG_RECIEVE;
			return;
		}
	}
	len = strlen(MSG_CONNECTED);
	if (m_currentMessageLength > len)
	{
		int startPos = (m_currentMessageLength - len)-1;
		if (findString(MSG_CONNECTED, startPos))
		{
			char* tmp = new char[--startPos];
			memcpy(tmp, m_currentMessage, startPos);
			int id = atoi(tmp);
			delete tmp;
			if (id >= 0 && id < MAX_CONNECTIONS)
				m_clientStates[id] = CLIENT_CONNECTED;
			return;
		}
	}
	len = strlen(MSG_DISCONNECTED);
	if (m_currentMessageLength > len)
	{
		int startPos = (m_currentMessageLength - len)-1;
		if (findString(MSG_DISCONNECTED, startPos))
		{
			char* tmp = new char[--startPos];
			memcpy(tmp, m_currentMessage, startPos);
			int id = atoi(tmp);
			delete tmp;
			if (id >= 0 && id < MAX_CONNECTIONS)
				m_clientStates[id] = CLIENT_DISCONNECTED;
			return;
		}
	}
	if (m_lastInstruction == MSG_RECIEVE)
	{
		if (m_currentId >= 0)
		{
			if (m_clientStates[m_currentId] == CLIENT_SENDING)
			{
				memcpy(m_recievedData, m_currentMessage + m_recievedDataLength, m_currentMessageLength);
				m_recievedDataLength += m_currentMessageLength;
				if (m_recievedDataLength == m_maxDataLength)
					m_clientStates[m_currentId] = CLIENT_WAITING_FOR_RESPONSE;
			}
		}
	}
}

bool ESP8266::findString(const char *stringToFind, int startPos)
{
	int len = strlen(stringToFind);
	int i = 0;
	for (; i < len; ++i)
	{
		if (m_currentMessage[startPos+i] != MSG_RECIEVE[i])
		{
			return false;
		}
	}
	return true;
}