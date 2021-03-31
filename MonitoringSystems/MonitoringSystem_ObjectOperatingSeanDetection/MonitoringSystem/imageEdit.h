//-------------------------------------------------------------------
// FileName : image.h
// Context : image(RGB)
// Description : image class
// Author : Katayama Noriaki (CV-lab.)
// Update : 2006.7.12
//-------------------------------------------------------------------
#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <string.h>
#include <limits.h>

#include <iostream>
#include <sstream>
#include <Windows.h>

#include <vector>
#include <iostream>
#include "geometory.h"
#include <opencv2/opencv.hpp>
//#define PI (3.14159265358979323846)
typedef std::pair<int, int> PII;
#include "image.h"
#include <math.h>
class M_estimates{
private:
	double param_alpha;
	double param_eta;
	double param_epsilon;
	double param_psi;
	double param_rho;
public:
	M_estimates();
	~M_estimates();
	void setAlpha(double alpha=0.8){ param_alpha = alpha; }
	double getAlpha(){ return param_alpha; }

	void setEta(double eta=10){ param_eta = eta; }
	double getEta(){ return param_eta; }

	void setEpsilon(double epsilon){ param_epsilon = epsilon; }
	double getEpsilon(){ return param_epsilon; }

	void setPsi(double psi){ param_psi = psi; }
	double getPsi(){ return param_psi; }

	void setRho(double rho){ param_rho = rho; }
	double getRho(){ return param_rho; }

	double calcPsi(double x){ return (param_psi = tanh(x /10.)); } 
	double calcRho(double x){ return (param_rho = log(cosh(x /10.))); } 
};



class Edit//: public image
{
private:
	//unsigned char *Ed;
	int Width,Height;
	cv::Mat Diff;
	cv::Mat depth_Diff;
	M_estimates m_estimate;
	M_estimates depth_m_estimate;
public:
	//Edit();
	//~Edit();
		//------------------------------------------------------------------------------
	//imageEdit process
	//------------------------------------------------------------------------------

	//void E_set(unsigned char *Ed){ Ed = new unsigned char[Width()*Height()*3];}
	
	void Diff_init(image &input){Diff =depth_Diff = cv::Mat(cv::Size(input.Width(),input.Height()), CV_8UC1, cv::Scalar(0));} /*new int[input.Width()*input.Height()];Width=input.Width();Height=input.Height();}*/
	void Diff_releace(){Diff.release();}
	int get_isDiff(const int x,const int y){return Diff.data[y*Diff.step+Diff.elemSize()*x];}
	void set_isDiff(int x,int y,unsigned char d){Diff.data[y*Diff.step+Diff.elemSize()*x]=d;}
	//void Diff_release(){	delete[]Diff;	Diff     = NULL;}

	bool imageEdit(unsigned long frame,image &input);
	void M_estimator(image &input); //�l����

	//���́E�l�`�F�b�N
	bool CheckDiffNum(image &result,image &start,Area *area);//���̌��m�̃`�F�b�N
	bool CheckObj(image &pre,Area *area);
	void ORImage(image &img1,image &img2,image &result);//�Q�̉摜�̘_���a���Ƃ�
	void SubUnderImg(image &img1,image &img2,image &result);// result = img1 - img2
	void BackDiff(image &input,int b_threshold=30);//�w�i����
	void FrameDiff(int frame);//�t���[���ԍ���

	//��f�̒l��ς���
	//int  PixColor(const int i, const int j);//��f�̐F�̒l�𒲂ׂ�
	//void draw(int x, int y, int R, int G, int B);
	//void DrawLineImg(image &img,Area *area);//�摜�ɐ�������
	//void DrawLineImg(Area *area);
	//void CopyImg(image &img,const char type);//�摜�̃R�s�[(�o�͗p)
	//void CopyImg(image &img1,int val);//�摜�R�s�[(�������p)
	//void DispResult();//��f�ɕt����ꂽ�������ƂɐF��������

	//void skinExtraction(image &mask);//���F���o
	//void skinExtraction();//���F���o
	void BinaryImgLabeling();//�Q�l�摜�̃��x�����O
	void Labeling(image &input);

	//2�l�摜�̎��k
	void Contract(const int type);

	//2�l�摜�̖c��
	void Expand(const int type);

	//�����ɒ��ڂ͊֌W�Ȃ����ǎg���֐�

	//void qsort(double dat[], int first, int last);//�N�C�b�N�\�[�g
	//int  PointDistance(const int a,const int b,const int c,const int d);//��_�Ԃ̋��������߂�
//	void grayscale();//�Z�W��
//	void blackwhite(const unsigned char threshold);//��l��
//	void Edge();//�G�b�W���o(sobel)
	//--------------------------------------------------------------------------------


	//�e����
	void ElimWithCNCC(image &input);
	double CalcCNCC5(image &input, int x, int y );

//	double getL(unsigned char r,unsigned char g,unsigned char b);  //L for HSL
//	double getH2(unsigned char r,unsigned char g,unsigned char b); //H for HSL 2
//	double getS3(unsigned char r,unsigned char g,unsigned char b); //S for HSL 3

	unsigned char MAXC(unsigned char a,unsigned char b){
		if(a >= b)	return a;
		else	return b;
	}
	unsigned char MINC(unsigned char a,unsigned char b){
		if(a <= b)	return a;
		else	return b;
	}
	double RADIAN(double x){return ((x) / 180.0) * PI;}


	void depth_M_estimator(image &input);
	void depth_diff(image &input); 
	void depth_image_Matching(image &input); 
	//void depth_Diff_init(image &input){delete[]depth_Diff;depth_Diff = new int[input.Width()*input.Height()*input.sizeHeigth*input.sizeWidth];Width=input.Width();Height=input.Height();}
	void depth_Diff_init(image &input){depth_Diff = cv::Mat(cv::Size(input.Width(),input.Height()), CV_8UC1, cv::Scalar(0));}
	//void depth_Diff_resize(image &input){cv::resize(depth_Diff,depth_Diff,cv::Size(input.Width(),input.Height()));}
    int get_depth_isDiff(const int x,const int y){return depth_Diff.data[y*depth_Diff.cols+x];}//+  depth_Diff.step.p[0]*y))[x].x;}
	//edit.depth_Diff[x, y]�ɒl���Z�b�g����
	void set_depth_isDiff(int x,int y,unsigned char d){depth_Diff.data[y*depth_Diff.cols+x]=d;}
	void depth_isDiff_Expand_Contract()
	{
		cv::erode(depth_Diff,depth_Diff ,  cv::Mat(), cv::Point(-1,-1), 1);//���k
		cv::Mat erode1 = depth_Diff.clone();
		//imshow( "depth_Diff 1.erode", erode1);
        cv::dilate(depth_Diff,depth_Diff, cv::Mat(), cv::Point(-1,-1), 1);//�c��
		cv::Mat dilate2 = depth_Diff.clone();
		//imshow( "depth_Diff 2.dilate", dilate2);
		cv::erode(depth_Diff,depth_Diff ,  cv::Mat(), cv::Point(-1,-1), 1);//���k
		cv::Mat erode3 = depth_Diff.clone();
		//imshow( "depth_Diff 3.erode", erode3);
		cv::dilate(depth_Diff,depth_Diff, cv::Mat(), cv::Point(-1,-1), 1);//�c��
		cv::Mat dilate4 = depth_Diff.clone();
		//imshow( "depth_Diff 4.dilate", dilate4);
		//cv::erode(depth_Diff,depth_Diff ,  cv::Mat(), cv::Point(-1,-1), 1);//���k

	}
	void depth_Diff_O( cv::Mat &a ){ depth_Diff.copyTo(a); }
};

