/*
	RemoteControl
	
	A class to manage messages send to the Gecko robot. 
	
	functions:
	* Tick : needs to be called every frame to use the object.
	* getNextMovement : Gets the next movement in the buffer and 
						forwards the current index int the buffer.
	* peakNextMovement : Gets the next movement in the buffer.
	* getNextMovements : Get all movements. Warning: returns a pointer to the internal structure.
						 Returns the size of the structure.
	* getNextMovementsCount : Get the number of remaining movements.
	
	Created by
	Kefke
*/

#ifndef REMOTECONTROL_H
#define REMOTECONTROL_H

enum MovementMode
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	SPEED
};

enum ClientState
{
	SENDING,
	WAITING_RESPONSE,
	IDLE
};

struct Movement
{
	int distance;
	MovementMode mode;
};

class ESP8266;

class RemoteControl
{
public:
	RemoteControl(ESP8266* esp);
	~RemoteControl();
	
	void Tick();
	
	Movement getNextMovement();
	Movement peakNextMovement();
	int getNextMovements(Movement* &movements);
	int getNextMovementsCount();
	
private:
	ESP8266* m_esp;
	int m_movementsLength;
	int m_currentMovement;
	Movement* m_movements;
	ClientState m_clientState;
};

#endif