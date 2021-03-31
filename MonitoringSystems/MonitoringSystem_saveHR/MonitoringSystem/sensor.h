#pragma once 
#include <iostream>

class Sensor{
private:
	std::string name;

public:
	Sensor();
	~Sensor();

public:
	virtual void recieve();
	virtual void send(std::string msg);


};
