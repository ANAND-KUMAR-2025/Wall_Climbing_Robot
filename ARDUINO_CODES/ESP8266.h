/*
	ESP8266
	
	A class to easily manage a ESP8266 module. 
	
	functions:
	* getCurrentState : Returns the current state of the module.
	* checkAT : Check if AT commands are enabled on the module.
	* setLocalEcho : Sets if commands should be echoed back.
	* configureAP: Configure the module to be used as a AP.
	* setMux : Set mux mode.
	* setServer : Enable / disable a server on a specific port.
	* prepareSendData : Prepare to send data to a connected client.
	* sendData : Send data directly to the module.
	* getRecievedMessage : Get the last recieved message.
	* deleteRecievedMessage : Deletes the last recieved message.
	* getClientState : Gets the clientState of a client.
	* getCurrentId : Gets the last used client id.
	
	Created by
	Kefke
*/

#ifndef ESP8266_H
#define ESP8266_H

#define MAX_CONNECTIONS 5
#define MAX_MESSAGESIZE 50
#define OVERFLOW_KEEP 10

enum espState
{
	STATE_ERROR = -1,
	STATE_UNKNOWN = 0,
	STATE_READY = 1,
	STATE_WAITING_FOR_RESPONSE = 2,
	STATE_BUSY = 3,
	STATE_OK = 4,
	STATE_RECIEVING = 5,
	STATE_SENDING = 6
};

enum wifiMode
{
	WIFI_OPEN = 0,
	WIFI_WPA = 2,
	WIFI_WPA2 = 3,
	WIFI_WPA_WPA2 = 4
};

enum clientState
{
	CLIENT_UNKNOWN = -1,
	CLIENT_DISCONNECTED = 0,
	CLIENT_CONNECTED = 1,
	CLIENT_WAITING_FOR_RESPONSE = 2,
	CLIENT_READY_TO_RECIEVE = 3,
	CLIENT_RECIEVING = 4,
	CLIENT_SENDING = 5
};

class HardwareSerial;

class ESP8266
{
public:
	ESP8266(HardwareSerial *serial);
	~ESP8266();
	
	espState getCurrentState();
	
	void checkAT();
	void setLocalEcho(bool on);
	void configureAP(const char *ssid, const char *password, unsigned int channel, wifiMode mode);
	void setMux(bool on);
	void setServer(unsigned int port, bool enable = true);
	void prepareSendData(unsigned int id, unsigned int size);
	void sendData(const char* data);
	int getRecievedMessage(char* &result);
	void deleteRecievedMessage();
	
	clientState getClientState(unsigned int id);
	unsigned int getCurrentId();
	
	void Tick();
	
	static const char* CMD_AT;
	static const char* CMD_ECHO;
	static const char* CMD_SETAPPARAMS;
	static const char* CMD_GETIP;
	static const char* CMD_SETMUX;
	static const char* CMD_SETSERVER;
	static const char* CMD_SEND;

	static const char* MSG_READY;
	static const char* MSG_OK;
	static const char* MSG_ERROR;
	static const char* MSG_BUSY;
	static const char* MSG_RECIEVE;
	static const char* MSG_CONNECTED;
	static const char* MSG_DISCONNECTED;
	
private:
	void processCurrentMessage();
	bool findString(const char *stringToFind, int startPos = 0);

	HardwareSerial *m_serial;
	espState m_state;
	clientState* m_clientStates;
	char* m_currentMessage;
	int m_currentMessageLength;
	int m_currentId;
	char* m_lastInstruction;
	char* m_recievedData;
	int m_recievedDataLength;
	int m_maxDataLength;
	bool m_overFlow;
	bool m_eol;
};

#endif