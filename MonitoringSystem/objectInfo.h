#ifndef __OBJECTINFO_H__
#define __OBJECTINFO_H__
#include "geometory.h"
#include <vector>

enum EVENT{EVENT_BRINGING_IN,  EVENT_TAKING_OUT, EVENT_MOVE, EVENT_NONE};

// class名: ObjectInfo
// 各検出されたオブジェクトの情報を管理するクラス
// 作成者: 池上貴之
class ObjectInfo{
private:
	int detectedFrame;
	Geometory::Point2D posCenter;
	Geometory::Point2D posTopLeft;
	Geometory::Point2D posBottomRight;
	double area;
	string strDetectedDate;
	int eventId;
	int objectId;
public:
	ObjectInfo(){}
	ObjectInfo(int detectedFrame
		, Geometory::Point2D posCenter
		, Geometory::Point2D posTopLeft, Geometory::Point2D posBottomRight
		, double area, string strDetectedDate, int eventId, int objectId)
		:detectedFrame(detectedFrame), posCenter(posCenter), posTopLeft(posTopLeft), posBottomRight(posBottomRight), area(area), strDetectedDate(strDetectedDate), eventId(eventId), objectId(objectId){}

	//ObjectInfo(){}

	~ObjectInfo(){
	}

	// getter/setter
	int getDetectedFrame(){ return detectedFrame; }	
	void setDetectedFrame(int _detectedFrame){ detectedFrame = _detectedFrame; }

	Geometory::Point2D getPosCenter(){ return posCenter;}
	void setPosCenter(Geometory::Point2D _posCenter){ posCenter = _posCenter; }

	Geometory::Point2D getPosTopLeft(){ return posTopLeft; }
	void setPosTopLeft(Geometory::Point2D _posTopLeft){ posTopLeft = _posTopLeft; }

	Geometory::Point2D getPosBottomRight(){ return posBottomRight; }
	void setBottomRight(Geometory::Point2D _posBottomRight){ posBottomRight = _posBottomRight; }

	double getArea(){ return area; }
	void setArea(double _area){ area = _area; }

	string getStrDetectedDate(){ return strDetectedDate; }
	void setStrDetectedDate(string _strDetectedDate){ strDetectedDate = _strDetectedDate; }

	int getEventID(){ return eventId; }
	void setEventID(int _eventId){ eventId = _eventId; }

	int getID(){ return objectId; }
	void setID(int _objectId){ objectId = _objectId; }

	// compare (same)
	bool isSameEVENT(const ObjectInfo &o){
		return (this->eventId == o.eventId);
	}
	bool isSameID(const ObjectInfo &o){
		return (this->objectId == o.objectId);
	}

	string getDetectedEventKind(){
		if(eventId == EVENT_BRINGING_IN) return "bring in";
		else if(eventId == EVENT_TAKING_OUT) return "taking out";
		else if(eventId == EVENT_TAKING_OUT) return "move";
		else return "none";
	}
};

// class名: EventInfo
// イベントが起きて検出されたオブジェクト情報を管理するクラス
// 作成者: 池上貴之
// 
class DetectedObjectsInfo{
protected:
	std::vector<ObjectInfo> detectedObjects;
public:
	DetectedObjectsInfo(){
		detectedObjects.clear();
	}
	DetectedObjectsInfo(std::vector<ObjectInfo> detectedObjects):detectedObjects(detectedObjects){
	}
	//コピーコンストラクタ
	DetectedObjectsInfo(const DetectedObjectsInfo &o){
		detectedObjects = o.detectedObjects;
	}
	~DetectedObjectsInfo(){
	}
	virtual int getObjectsNum(){
		return (int)detectedObjects.size();
	}
	virtual void setInfomations(std::vector<ObjectInfo> _detectedObjects){
		detectedObjects = _detectedObjects;
	}
	virtual std::vector<ObjectInfo> getInfomations(){
		return detectedObjects;
	}
	virtual std::vector<ObjectInfo>::iterator begin(){
		return detectedObjects.begin();
	}
	virtual std::vector<ObjectInfo>::iterator end(){
		return detectedObjects.end();
	}
	virtual void push_back(ObjectInfo objectInfo){
		detectedObjects.push_back(objectInfo);
	}
	virtual int size(){
		return detectedObjects.size();
	}
	virtual ObjectInfo operator[](int i){
		return detectedObjects[i];
	}
	std::vector<ObjectInfo>::iterator findId(int id){
		for(vector<ObjectInfo>::iterator itr = detectedObjects.begin(); itr != detectedObjects.end(); itr++){
			if(itr->getID() == id)return itr;
		}
		return detectedObjects.end();
	}
	void erase(int id){
		for(vector<ObjectInfo>::iterator itr = detectedObjects.begin(); itr != detectedObjects.end(); itr++){
			if(itr->getID() == id){
				detectedObjects.erase(itr);
				return;
			}
		}
	}
};
class TakingOutObjectList : public DetectedObjectsInfo{
private:

public:

};
class BringInObjectList : public DetectedObjectsInfo{
private:
	int latestDetectedobjId;
public:
	int getLatestDetectedObjId(){
		return (detectedObjects.size() - 1);
	}

};
class MoveObjectList : public DetectedObjectsInfo{
private:

public:

};

class EventInfo{
private:
public:
	TakingOutObjectList takingOutObjList;
	BringInObjectList bringInObjList;
public:
	EventInfo(TakingOutObjectList takingOutObjList, BringInObjectList bringInObjList)
		:takingOutObjList(takingOutObjList), bringInObjList(bringInObjList){
	}
	EventInfo(){

	}
	~EventInfo(){
	}
};
#endif