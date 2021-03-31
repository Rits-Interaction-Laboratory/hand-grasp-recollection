//-----------------------------------------
// FileName : network.h
// Context  : CCamera�N���X�̐錾
//
// Author   : Kazuhiro MAKI (CV-lab.)
// Updata   : 2007.05.22
//-----------------------------------------

#pragma once

//#include "bmp.h"
#include "FileOperator.h"

#include "MonitoringSystemDlg.h"

#include "Surf.h"
#include "Pair.h"
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <vector>
#include<iostream>

#include "HumanProcess.h"


#include "imageEdit.h"
#include "Connectioni.h"
#include "Humansearch.h"
#include "ObjectSearch.h"
#include "EventSearch.h"
#include "image.h"
#include<map>
#include "MultiData.h"
//#include"Kinectsensor.h"

//enum ModeType{ONLINE,OFFLINE}
class CCamera : public image{
private:
	int preservation;   //�摜�ۑ��t���O
	int direction;      //�ύX����J�����̌���

	int mode;  //�I�����C��or�I�t���C��
	CString strPathDir;
	long long frame;

	bool wflag;
	
	int c;
	bool image_is_diff;//�摜���̍������m
	bool human_detection;//�l�������m
	bool object_detection;//���̂����m
	bool event_flag;//�C�x���g�̌��m

	//IplImage *img_HLS;
	//IplImage *img_resize;

	/* ���̌��m�p---�ǉ� */
	image input;

	Edit   edit;//Lv1�摜�ҏW
	human  Human;//Lv2�l�����m
    Object object; //Lv2���̌��m
    Event  _event;//LV3-5�C�x���g���m
	//KinectSensor kinect;

    DetectionLog detectonlog;//���̏�񃍃O

	Matv2 imgInput;


    CMonitoringSystemDlg* pParent;  //�e�E�B���h�E�̃|�C���^

	//���߃N���X
	//Cinterpretation intpre;

	SurfPoint surf;

	std::map<string, MultiData> resultPack;
private:
	void initDir();
	void initImagePack();
public:

	int SendCommand(int cameraid,int task);  //������𑗂�
	int OffLine();  //�I�t���C������
	void set_preservation(){preservation=1;}
	void set_preservation_stop(){preservation=0;}
	void ChangeDirection(int course){ direction = course; }  //�ύX����J�����̌������Z�b�g

	void Resetflag(){image_is_diff=false;human_detection=false;object_detection=false;event_flag=false;};//���ꂼ��̃t���b�O�̏�����
	
	//�C�x���g���m(�ЎR����)
	int DetectionOffline(unsigned long frame);
	int Detection(unsigned long frame, Connection connection);
	int Detection(unsigned long frame, Connection connection, bool* touch_flag);	// 2019/1/9�ǉ�(�L�{)
	int Detection2(unsigned long frame, Connection connection, vector<map<string, string>> &detectedObj);
	int Detection3(unsigned long frame, Connection connection, vector<map<string, string>> &detectedObj, std::vector<float> layVec);

	double calc_SSD(cv::Mat in, cv::Mat tmp);

	void switch_window(IplImage* png);

	void set_pParent(CMonitoringSystemDlg* p){pParent = p;} //�e�E�B���h�E�ւ̃|�C���^�ݒ�

	//���߃X���b�h�p
	//�V�[���󋵂̃Z�b�g
	//void SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene);
	//IplImage *JpegBufToIplImage(char* jpegBuf, unsigned long len);
	//int HumanSearch(unsigned long frame,image &input);	//�l���F��	
	//int ObjectSearch(unsigned long frame,image &input);	//���̔F��
    //int EventSearch(unsigned long frame, image &input);	//�C�x���g���m(����@�������݁A�ړ��A��������)

	//void BackDiff(image &back,image &input,int b_threshold=20);//�w�i����
	//void OutputImage(unsigned long frame,image &input);
	//void create_window(unsigned long frame);
	void create_Sign(unsigned long frame,image &input);
	//bool loadObjectId();
	//void M_estimator(); //�l����

	void saveDATA(unsigned long frame,image &input,int human_flag,int object_flag,int event_flag);		//�摜�f�[�^�ۑ�

	void OutputImage(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag, vector<map<string, string>> recoResult);//�A�v���P�[�V�����ɉ摜�o��
	void OutputImage2(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag);//�A�v���P�[�V�����ɉ摜�o��
	void OutputImageOffline(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag);//�A�v���P�[�V�����ɉ摜�o��
	void setSkeleton(cv::Mat depth_img);
	void DBConnect();
};