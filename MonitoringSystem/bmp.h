//-------------------------------------------------------------------
// FileName : bmp.h
// Context : bitmap(24bit only)
// Description : bmp class
// Author : Noriaki Katayama(CV-lab.)
// Update : 2005.6.27
//-------------------------------------------------------------------
#pragma once

//-----------------
// Header File
//-----------------
#include "image.h"
#include <iostream>
#include <fstream>

//BMPファイルヘッダ
struct MY_BITMAPFILEHEADER {
	unsigned short bfType;       //ファイルタイプ('BM'を対象)
	unsigned long  bfSize;       //ファイルサイズ[byte]
	unsigned short bfReserved1;  //予約領域（常に０）
	unsigned short bfReserved2;  //予約領域（常に０）
	unsigned long  bfOffBits;    //ファイル先頭から画像データまでのオフセット[byte](54)
};

//BMP情報ヘッダ
struct MY_BITMAPINFOHEADER{
	unsigned long  biSize;          //情報ヘッダのサイズ[byte](40)
	long           biWidth;         //画像の幅[ピクセル]
	long           biHeight;        //画像の高さ[ピクセル]
	unsigned short biPlanes;        //プレーン数（常に１）
	unsigned short biBitCount;      //１画素あたりのデータサイズ[bit](24を対象)
	unsigned long  biCompression;   //圧縮形式
	unsigned long  biSizeImage;     //画像データ部のサイズ[byte]
	long           biXPixPerMeter;  //横方向解像度[１ｍあたりの画素数]
	long           biYPixPerMeter;  //縦方向解像度[１ｍあたりの画素数]
	unsigned long  biClrUsed;       //格納されているパレット数[使用色数]
	unsigned long  biClrImporant;   //重要なパレットのインデックス
};

//-----------------
// class
//-----------------
class bmp : public image 
{
	MY_BITMAPFILEHEADER file;// file header
	MY_BITMAPINFOHEADER info;// info header
	int line;
	int dust;

public:

	// constructor
	bmp(){}
	// destructor
	~bmp(){}

	//画像データ読み込み
	bool load(const char *filename);

	//画像データ書き出し
	bool save(const char *filename);

	//上下反転
	void inverse();

	//// operator+
	//bmp& operator-(const int val)
	//{
	//	bmp hoge;
	//	for(int j=0; j < this->getheight(); j++){
	//		for(int i=0; i < this->getwidth(); i++){
	//			hoge.R(i,j) = this->R(i,j) - val;
	//			hoge.G(i,j) = this->G(i,j) - val;
	//			hoge.B(i,j) = this->B(i,j) - val;
	//		}
	//	}
	//	return hoge;
	//}
};