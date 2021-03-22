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
//	//CMonitoringSystemDlg* pParent2;  //�e�E�B���h�E�̃|�C���^
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
//	//�C�x���g����(���C���\��)
//	int increment(image &input, unsigned long frame, bmp &det);
//	int interpretation(image &input, unsigned long frame, bmp &det, int delete_key, int obj_id, int key_id, bool track_flag);
//	bool check(image &now,bmp &pre);
//
//	//�C�x���g����(���̒ǐ�)
//	bool in_obj_tracking(image &first, unsigned long now_frame,std::vector<Pair*> pair,unsigned long leaveframe);    //�����ݎ�//kawa
//	bool out_obj_tracking(image &first, unsigned long now_frame);   //�����莞
//	unsigned long readLogFile(string filename);
//	unsigned long search_folder(unsigned long frame); //�t�H���_�̌���
//	unsigned long search_file(unsigned long frame); //�t�@�C���̌���
//
//	//���[�h�̐ݒ�
//	void set_mode(int m){ mode = m; }
////	void set_pParent(CMonitoringSystemDlg* p){pParent2 = p;} //�e�E�B���h�E�ւ̃|�C���^�ݒ�
//
//
//
//	//�Ĕz��//kawa
//	void resizeobject_img(int l_max){object_img.resize(l_max);}
//	//���//kawa
//	void clearobject_img(){object_img.clear();}
//
//
//};