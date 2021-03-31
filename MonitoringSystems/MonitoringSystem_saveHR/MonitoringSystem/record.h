#pragma once
#include "geometory.h"
#include <iostream>
#include "date.h"
class Record{
private:

public:
	Record();
public:
	virtual bool writeData();
	virtual bool readData();


};

class RecordObjectData : public Record{
private:
	long currentframe;
	int id;
	int key_point_parent_id;
	Geometory::Point2D position;
	Date date;
	bool scene;  //true:�������� false:��������
	bool track;  //true:�ړ�     false:��������
public:
	RecordObjectData();
	~RecordObjectData();
public:
	bool writeData();
	bool readData();
};