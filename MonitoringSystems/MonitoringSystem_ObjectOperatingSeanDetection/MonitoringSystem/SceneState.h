#pragma once
#include "bmp.h"
#include <stack>

struct IMG_RECORD{
	bmp mask;
	bmp back;
	bmp result;
};

//enum Mode1{ONLINE,OFFLINE};
//----------------------------
//シーンの状態
//----------------------------
class Scene
{
private:
	//int layer;
	//IMG_RECORD *rec;
	bmp *top,*tail;
	bmp *img_t1,*img_t2,*img_t3,*img_t4,*img_t5,*img_t6,*img_t7,*img_t8;
	std::stack<bmp> obj_data;

	int mode;  //オンラインorオフライン

public:
	//constructor
	Scene();

	//destructor
	~Scene();

	//初期化
	void init(){top = NULL;}

	//一定フレームに物体ラベルの領域を発見したか？
	int check(bmp &in,unsigned long frame);

	//２枚の画像を比較して物体を検知したか？
	void detection(bmp &now,bmp &pre,int *count);

	void set_obj(bmp &in);
	bmp get_obj();

	//相関を計算する関数
	bool BlockCorr(bmp &pre,bmp &now);//ブロックごとの合計の平均
	bool AllCorr(bmp &pre,bmp &now);//全ての合計の平均

	//モードの設定
	void set_mode(int m){mode=m;}
};
