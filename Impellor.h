/*
	Impellor
	
	A class to easily control the Impellor Gecko robot. 
	
	functions:
	* Init : initializes the object.
	* Tick : needs to be called every frame to use the object.
	* StartImpellor : Sets the speed of the impellor (in percent). 
					  Note: if a value less than 10% is given, the impellor will stop.
	* StopImpellor: Stops the impellor.
	* MoveForward : move the robot forward.
	* MoveBackward : move the robot backwards.
	* TurnLeft : turn the robot to the left.
	* TurnRight : turn the robot to the right.
	* IsDoneMoving : check if the current movement is done.
	
	Created by
	Kefke
*/

#ifndef IMPELLOR_H
#define IMPELLOR_H

class Servo;

enum ImpellorState
{
	IMPELLOR_STATE_UNKNOWN,
	IMPELLOR_STATE_INIT,
	IMPELLOR_STATE_STOPPED,
	IMPELLOR_STATE_STARTED,
	IMPELLOR_STATE_FORWARD,
	IMPELLOR_STATE_BACKWARD,
	IMPELLOR_STATE_LEFT,
	IMPELLOR_STATE_RIGHT,
	IMPELLOR_STATE_DONE
};

class Impellor
{
public:
	Impellor(int escPin, int enAPin, int in1Pin, int in2Pin, int in3Pin, int in4Pin, int enBPin);
	~Impellor();
	
	void Init();
	void Tick();
	
	void StartImpellor(int speedPercent = 60);
	void StopImpellor();
	
	void MoveForward(int distance);
	void MoveBackward(int distance);
	void TurnLeft(int distance);
	void TurnRight(int distance);
	
	bool IsDoneMoving();
	
	static const int DEF_MINSPEED = 701;
	static const int DEF_MAXSPEED = 2100;
	
private:
	void CheckState();
	
	int m_escPin, m_enAPin, m_enBPin, m_in1Pin, m_in2Pin, m_in3Pin, m_in4Pin, m_currentPos, m_destPos, m_impellorSpeed;
	long m_prevMillis;
	Servo* m_esc;
	ImpellorState m_state;
};
#endif