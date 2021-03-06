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
	bool scene;  //true:持ち込み false:持ち去り
	bool track;  //true:移動     false:持ち込み
public:
	RecordObjectData();
	~RecordObjectData();
public:
	bool writeData();
	bool readData();
};