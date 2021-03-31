#pragma once
#include <cmath>
#include <string.h>
#include <limits.h>
#include <vector>
#include <math.h>
#include <iostream>
#include "geometory.h"
#include <opencv2/opencv.hpp>
#include "image.h"
#include"layer.h"
#include"ObjectSearch.h"
#include"objectInfo.h"
#include"Time.h"
#include"Surf.h"
#include "Util.h"
#include "image.h"
#include "detectionlog.h"
class CMonitoringSystemDlg;

class State{
private:
	PII pos;
	int id;
public:
	State(){
		pos = PII(0,0);
		id = -1;
	}
	State(PII pos, int id):pos(pos), id(id){
	}
	~State(){
	}
	void setPos(PII pos){
		this->pos = pos;
	}
	void setId(const int id){
		this->id = id;
	}
	PII getPos(){
		return pos;
	}
	int getId(){
		return id;
	}
	bool operator < (const State &o)const{
		return id > o.id;
	}
	bool operator == (const int id)const{
		return (this->id == id);
	}
};

typedef struct {
	int s_h;
	int e_h;
	int s_w;
	int e_w;
	int id;
}Object_Area;

class Event //: public image
{
private:
	
	
	std::vector<int>key_idname;//モデルID
	std::vector<int>obj_idname;
	//std::vector<int>cou;
	std::vector<BMP>object_img;//見え方が変化した画像を保存
	BMP output_val;
	std::vector<Object_Area> object_area;//画面内の物体データ
	std::vector<int> touched_human;//接触した人物情報
	std::map<int,int> l_map;//持ち去りデータ用
	//Object hobj;

public:
	Event(void);
	~Event(void);
	CMonitoringSystemDlg *pParent;
	//std::vector<int>id_lay;//モデルID
	//CMonitoringSystemDlg* pParent2; 
	 //int leaveframe;
	//static layer lay;
	SurfPoint surf;
	//bool EventSearch(unsigned long frame ,image &input,bool object_detection,bool human_detection); //2019/1/10(有本)
	bool EventSearch(unsigned long frame ,image &input,bool object_detection,bool human_detection, bool* touch_flag);		// 2019/1/9追加(有本)
	bool loadObjectId();
	int increment(unsigned long frame,image &input,int num);
	bool check(image &input,int num,int index);

	//int SurfMatching(CvSeq *imageKeypoints,CvSeq *imageDescriptors, IplImage *png,unsigned long frame, CvRect region, bmp &det, image &input, int delete_key, std::vector<Pair*> tracker, std::vector<int> outId,std::vector<int> idname);
	
	unsigned long readLogFile(string filename);
	unsigned long searchFolder(unsigned long frame);
	unsigned long searchFile(unsigned long frame);
	int objectTracking( image &input, unsigned long nowFrame, int t, int objectNum, int num, Pair* traker );//,std::vector<Pair*> tracker);//物体追跡判定
	//void set_pParent(CMonitoringSystemDlg* p){pParent2 = p;} //親ウィンドウへのポインタ設定
	void SaveObjData(image &input,unsigned long frame,int obj_num,int num,int leave_id,int surf_id);
	void interpretation(image &input, unsigned long frame,  int obj_num,int num,int delete_key);
	void motion_analays(image &input, unsigned long frame,  int obj_num,int num);
	void object_reload(image &input,unsigned long frame);
	void style_detect(image &input,unsigned long frame);
	//void near_object(image &input,int num,unsigned long frame);
	void touch_event(image &input,unsigned long frame);
	void touch_event(image &input,unsigned long frame, bool* touch_flag);		// 2019/1/9追加(有本)
	void d_human_maching(image &input,int frame);

	void Event::calcBack( image &input, cv::Mat firstHue, cv::Mat firstMask, cv::Mat objectHue, cv::Mat objectMask, cv::Mat aroundHue, cv::Mat aroundMask, cv::Mat trackArea, int nowFrame );
};