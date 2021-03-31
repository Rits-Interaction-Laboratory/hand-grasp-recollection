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

//BMP�t�@�C���w�b�_
struct MY_BITMAPFILEHEADER {
	unsigned short bfType;       //�t�@�C���^�C�v('BM'��Ώ�)
	unsigned long  bfSize;       //�t�@�C���T�C�Y[byte]
	unsigned short bfReserved1;  //�\��̈�i��ɂO�j
	unsigned short bfReserved2;  //�\��̈�i��ɂO�j
	unsigned long  bfOffBits;    //�t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g[byte](54)
};

//BMP���w�b�_
struct MY_BITMAPINFOHEADER{
	unsigned long  biSize;          //���w�b�_�̃T�C�Y[byte](40)
	long           biWidth;         //�摜�̕�[�s�N�Z��]
	long           biHeight;        //�摜�̍���[�s�N�Z��]
	unsigned short biPlanes;        //�v���[�����i��ɂP�j
	unsigned short biBitCount;      //�P��f������̃f�[�^�T�C�Y[bit](24��Ώ�)
	unsigned long  biCompression;   //���k�`��
	unsigned long  biSizeImage;     //�摜�f�[�^���̃T�C�Y[byte]
	long           biXPixPerMeter;  //�������𑜓x[�P��������̉�f��]
	long           biYPixPerMeter;  //�c�����𑜓x[�P��������̉�f��]
	unsigned long  biClrUsed;       //�i�[����Ă���p���b�g��[�g�p�F��]
	unsigned long  biClrImporant;   //�d�v�ȃp���b�g�̃C���f�b�N�X
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

	//�摜�f�[�^�ǂݍ���
	bool load(const char *filename);

	//�摜�f�[�^�����o��
	bool save(const char *filename);

	//�㉺���]
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