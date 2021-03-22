#pragma once

#include "bmp.h"
#include "FileOperator.h"
#include "Pair.h"

#include <vector>
#include <ctime>
#include <map>
#include <limits.h>
#include <cmath>
//#include "scene.h"
#include "label.h"
#include "layer.h"
#include <algorithm>
#include <string>

#include "opencv/cv.h"
#include "opencv/highgui.h"

//class CMonitoringSystemDlg;
//class CCamera;
//class State{
//private:
//	PII pos;
//	int id;
//public:
//	State(){
//		pos = PII(0,0);
//		id = -1;
//	}
//	State(PII pos, int id):pos(pos), id(id){
//	}
//	~State(){
//	}
//	void setPos(PII pos){
//		this->pos = pos;
//	}
//	void setId(const int id){
//		this->id = id;
//	}
//	PII getPos(){
//		return pos;
//	}
//	int getId(){
//		return id;
//	}
//	bool operator < (const State &o)const{
//		return id > o.id;
//	}
//	bool operator == (const int id)const{
//		return (this->id == id);
//	}
//};
//class Cinterpretation
//{
//	int mode;
//	//CMonitoringSystemDlg* pParent2;  //親ウィンドウのポインタ
//
//	//bmp object_img[10];//kawa
//	std::vector<bmp>object_img;
//	
//
//
//	std::map<int,int> l_map,l_map2;
//
//public:
//	Cinterpretation(void);
//	~Cinterpretation(void);
//
//	/*std::vector<Pair*> pair;	
//	std::vector<int> outId;*/
//
//	//イベント解釈(レイヤ構造)
//	int increment(image &input, unsigned long frame, bmp &det);
//	int interpretation(image &input, unsigned long frame, bmp &det, int delete_key, int obj_id, int key_id, bool track_flag);
//	bool check(image &now,bmp &pre);
//
//	//イベント解釈(物体追跡)
//	bool in_obj_tracking(image &first, unsigned long now_frame,std::vector<Pair*> pair,unsigned long leaveframe);    //持込み時//kawa
//	bool out_obj_tracking(image &first, unsigned long now_frame);   //持去り時
//	unsigned long readLogFile(string filename);
//	unsigned long search_folder(unsigned long frame); //フォルダの検索
//	unsigned long search_file(unsigned long frame); //ファイルの検索
//
//	//モードの設定
//	void set_mode(int m){ mode = m; }
////	void set_pParent(CMonitoringSystemDlg* p){pParent2 = p;} //親ウィンドウへのポインタ設定
//
//
//
//	//再配列//kawa
//	void resizeobject_img(int l_max){object_img.resize(l_max);}
//	//解放//kawa
//	void clearobject_img(){object_img.clear();}
//
//
//};