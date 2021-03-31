#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <string.h>
#include <limits.h>
#include <vector>
#include <iostream>
#include "geometory.h"
#include <opencv2/opencv.hpp>
//#define PI (3.14159265358979323846)
typedef std::pair<int, int> PII;
#include "image.h"


typedef struct HumanDATA{
	int x1, x2, y1, y2;    //上下左右の領域
	int xcenter, ycenter;  //重心
	long int size;    //ラベル面積
	int frame;  //自分物のID　何フレーム目にいるか
	int ID;
} HumanDATA;

class human //:public image
{
private:
	int human_num;
	HumanDATA *human_table;


public:

	
	bool HumanSearch(unsigned long frame,image &input);	//人物認識


	//人物データ初期化
	void Hset(){/*delete [] human_table;*/human_table=new HumanDATA [256];human_num=0;}
	void Hreleace(){delete [] human_table;human_table=NULL;}

	//人物整理
	//void Hset();

	//人物設定
	void setArea(int num,image &input,int i,unsigned long frame)
	{
		human_table[num].x1=input.get_STable_s_w(i);
		human_table[num].x2=input.get_STable_e_w(i);
		human_table[num].y1=input.get_STable_s_h(i);
		human_table[num].y2=input.get_STable_e_h(i);
		human_table[num].xcenter=input.get_STable_cx(i);
		human_table[num].ycenter=input.get_STable_cy(i);
		human_table[num].size=input.get_STable_size(i);
		human_table[num].frame=(int)frame;
		human_table[num].ID=(int)i;

	}
	int &HTable_s_w(int num){ return human_table[num].x1; }
	int &HTable_e_w(int num){ return human_table[num].x2; }
	int &HTable_s_h(int num){ return human_table[num].y1; }
	int &HTable_e_h(int num){ return human_table[num].y2; }
	int &HTable_cx(int num){ return human_table[num].xcenter; }
	int &HTable_cy(int num){ return human_table[num].ycenter; }
	long int &HTable_size(int num){ return human_table[num].size;}
	int &HTable_ID(int num){ return human_table[num].ID; }
	int &HTable_frame(int num){ return human_table[num].frame; }
	void HtabMAX(int num){human_num=num;}
	int &HData(){return human_num;}

	//void d_human_maching(image &input,int frame);

	void BinaryImgLabeling(image &input);
};