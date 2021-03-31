//-----------------------------------------
// FileName : network.h
// Context  : CCameraクラスの宣言
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
	int preservation;   //画像保存フラグ
	int direction;      //変更するカメラの向き

	int mode;  //オンラインorオフライン
	CString strPathDir;
	long long frame;

	bool wflag;
	
	int c;
	bool image_is_diff;//画像内の差分検知
	bool human_detection;//人物を検知
	bool object_detection;//物体を検知
	bool event_flag;//イベントの検知

	//IplImage *img_HLS;
	//IplImage *img_resize;

	/* 物体検知用---追加 */
	image input;

	Edit   edit;//Lv1画像編集
	human  Human;//Lv2人物検知
    Object object; //Lv2物体検知
    Event  _event;//LV3-5イベント検知
	//KinectSensor kinect;

    DetectionLog detectonlog;//物体情報ログ

	Matv2 imgInput;


    CMonitoringSystemDlg* pParent;  //親ウィンドウのポインタ

	//解釈クラス
	//Cinterpretation intpre;

	SurfPoint surf;

	std::map<string, MultiData> resultPack;
private:
	void initDir();
	void initImagePack();
public:

	int SendCommand(int cameraid,int task);  //文字列を送る
	int OffLine();  //オフライン処理
	void set_preservation(){preservation=1;}
	void set_preservation_stop(){preservation=0;}
	void ChangeDirection(int course){ direction = course; }  //変更するカメラの向きをセット

	void Resetflag(){image_is_diff=false;human_detection=false;object_detection=false;event_flag=false;};//それぞれのフラッグの初期化
	
	//イベント検知(片山さん)
	int DetectionOffline(unsigned long frame);
	int Detection(unsigned long frame, Connection connection);
	int Detection(unsigned long frame, Connection connection, bool* touch_flag);	// 2019/1/9追加(有本)
	int Detection2(unsigned long frame, Connection connection, vector<map<string, string>> &detectedObj);
	int Detection3(unsigned long frame, Connection connection, vector<map<string, string>> &detectedObj, std::vector<float> layVec);

	double calc_SSD(cv::Mat in, cv::Mat tmp);

	void switch_window(IplImage* png);

	void set_pParent(CMonitoringSystemDlg* p){pParent = p;} //親ウィンドウへのポインタ設定

	//解釈スレッド用
	//シーン状況のセット
	//void SetScene(int id,int frame,int cx,int cy,char *hour,char *minute,bool scene);
	//IplImage *JpegBufToIplImage(char* jpegBuf, unsigned long len);
	//int HumanSearch(unsigned long frame,image &input);	//人物認識	
	//int ObjectSearch(unsigned long frame,image &input);	//物体認識
    //int EventSearch(unsigned long frame, image &input);	//イベント検知(現状　持ち込み、移動、持ち去り)

	//void BackDiff(image &back,image &input,int b_threshold=20);//背景差分
	//void OutputImage(unsigned long frame,image &input);
	//void create_window(unsigned long frame);
	void create_Sign(unsigned long frame,image &input);
	//bool loadObjectId();
	//void M_estimator(); //Ｍ推定

	void saveDATA(unsigned long frame,image &input,int human_flag,int object_flag,int event_flag);		//画像データ保存

	void OutputImage(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag, vector<map<string, string>> recoResult);//アプリケーションに画像出力
	void OutputImage2(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag);//アプリケーションに画像出力
	void OutputImageOffline(unsigned long frame, image &input, int image_flag, int human_flag, int event_flag);//アプリケーションに画像出力
	void setSkeleton(cv::Mat depth_img);
	void DBConnect();
};