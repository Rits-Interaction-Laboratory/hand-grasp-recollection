///////////////////////////////////////////////////////////
// FileName : image.cpp
// Context  : image(RGB)
// Author�@ : Katayama Noriaki (CV-lab.)
// Author2  : Maki Kazuhiro (CV-lab.)
///////////////////////////////////////////////////////////
#include "stdafx.h"
#include "image.h"
#include "bmp.h"
#include "label.h"
#include "scenestate.h"
#include <queue>
#include <map>
#include <vector>
#include <fstream>
#include "FileOperator.h"
#include <sstream>
#include <algorithm>
#include <time.h>

//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define CNCC_ERROR -100.0
#define CNCC_ERROR2 -1000.0

#pragma warning(disable: 4244)
#pragma warning(disable: 4101)

bmp *p_human;
bmp d_human;
bmp d_human2;
char savename[50];
char savename2[50];
bool hsave = false;

// �����̂������l�����̂��߂̕ϐ�
int diff_threshold = 15;
//
int diff_cncc=95,diff_hs=5;


//�X���b�h(�l���m�摜�ۑ�)
static UINT AFX_CDECL ThreadSave(LPVOID pParam)
{
	p_human=(bmp*)pParam;
	p_human->save(savename);
	hsave=false;
	return 0;
}

//�X���b�h(�l���m�摜�ۑ�)
static UINT AFX_CDECL ThreadSave2(LPVOID pParam)
{
	p_human=(bmp*)pParam;
	p_human->save(savename2);
	hsave=false;
	return 0;
}

//--------------------------------------------------
// Name     : watch(int frame)
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2006.6.27
// Function : �V�[���̊Ď�(�����̃��C������)
// Argument : int�^�̃t���[����
// return   : bool�^�̕ϐ�
//--------------------------------------------------
bool image::watch(unsigned long frame)
{        
	clock_t t1=clock();

	static Scene state; //�V�[�����
	static bmp pre;     //���肵���w�i�摜
	bmp now;            //���݃t���[���̉摜

	static bmp pre_bg;         //�e�����̂��߂̍X�V�O�̃t���[��

	int re=0;
	r_flag = false;

	static unsigned long f=-1;	//�t���O
	++f;						//�t���O�C���N�������g

	save_frame = ff = f;

	//�e��t���O������
	//static bool save_flag = false;
	static int save_flag = -1;
	state.set_mode(mode);

	//���̌��m�͈̔͐ݒ�
	//setArea(30,40,320,120);
	setArea(10,10,330,230);

	//���݂̓��͉摜�̋P�x�l�̂ݎg�p(�P�x�l��M����)
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			now.Y_d(i,j) = Y_d(i,j) = (double)(R(i,j) * 0.299 + G(i,j) * 0.587 + B(i,j) * 0.114);
			now.Y(i,j) = Y(i,j) = (unsigned char)Y_d(i,j);
			now.r_d(i,j) = now.R(i,j) = R(i,j);
			now.g_d(i,j) = now.G(i,j) = G(i,j);
			now.b_d(i,j) = now.B(i,j) = B(i,j);
		}
	}
	clock_t t2=clock();
	double rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();
	//�w�i��M�����p���Đ���(���t���[���X�V)
	M_estimator(pre,now,pre_bg);
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();
#if 0 //�摜�̕ۑ�
	char fname1[40];
	sprintf(fname1,"pre/%07d.png",frame);
	pre.save(fname1);
	//char fname2[40];
	//sprintf(fname2,"now/%07d.bmp",frame);
	//now.save(fname2);
#endif

	//����w�i�摜�Ɠ��͉摜�̍���
	BackDiff(pre,now,diff_threshold);
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//�e����(CNCC)
	Contract(BACK);
	//�c��
	Expand(BACK);
	Contract(BACK);
	//���k
	//Contract(BACK);//kawa

	if(f>30 && r_flag ==true)	ElimWithCNCC(pre,now);
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

#if 0 //�摜�̕ۑ�
	char fname1[40];
	sprintf(fname1,"pre/%07d.png",frame);
	pre.save(fname1);
	//if(frame>=220 && frame<=280){
	/*for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			if(isDiff(i,j)==NON){
				now.R(i,j) = 0;
				now.G(i,j) = 0;
				now.B(i,j) = 0;
			}else{
				now.R(i,j) = 255;
				now.G(i,j) = 255;
				now.B(i,j) = 255;
			}
		}
	}
		char fname2[40];
		sprintf(fname2,"diff/%07d.png",frame);
		now.save(fname2);*/
	//}
#endif
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();
	//
	//���x�����O
	Labeling();
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//���x���ɐF��t����
	DispResult();

	bmp end;

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			end.R(i,j) = R(i,j);
			end.G(i,j) = G(i,j);
			end.B(i,j) = B(i,j);
			end.isDiff(i,j) = isDiff(i,j);
			end.area = area;
			end.d_area = d_area;
			//end.Y(i,j) = pre.Y(i,j);
		}
	}

#if 0//teat
	/// display debug
	
	IplImage *thres_check = cvCreateImage(cvSize(imageWidth,imageHeight),8,3);
	// IplImage *image2 = cvCreateImage(cvSize(imageWidth,imageHeight), IPL_DEPTH_8U, 1);
	IplImage *image = cvCreateImage(cvSize(imageWidth,imageHeight),8,3);
	if( thres_check != NULL ) {
		for(int i=2; i<imageHeight; i++ ) {
			for( int j=0; j<imageWidth; j++ ) {
				int ii = imageHeight - i + 1; // vertical flip
				cvSet2D(thres_check,i,j,cvScalar(pre.b_d(j,ii),pre.g_d(j,ii),pre.r_d(j,ii)));
				//cvSet2D(image2,i,j,cvScalar(pre.Y(j,ii)));
				cvSet2D(image,i,j,cvScalar(B(j,ii),G(j,ii),R(j,ii)));
			}
		}
	}
	
	cvNamedWindow("thres_check");
	cvCreateTrackbar("threshold", "thres_check", &diff_threshold, 100, NULL);

	cvShowImage("thres_check", thres_check);
	//cvShowImage("image2", image2);
	cvShowImage("image", image);
	cvWaitKey(1);
	cvReleaseImage(&thres_check);
	cvReleaseImage(&image);
	//cvReleaseImage(&image2);
#endif


#if 1 //#######################################################################
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//���̌��m
	end.new_objnum = new_objnum;
	end.fix_obj_memory(new_objnum);
	for(int z=0;z<new_objnum;z++){
		end.OTable[z].size    = OTable[z].size;
		end.OTable[z].x1      = OTable[z].x1;
		end.OTable[z].x2      = OTable[z].x2;
		end.OTable[z].xcenter = OTable[z].xcenter;
		end.OTable[z].y1      = OTable[z].y1;
		end.OTable[z].y2      = OTable[z].y2;
		end.OTable[z].ycenter = OTable[z].ycenter;
	}

	save_flag = state.check(end,frame);

	bool corr_flag = false;

	//�Ƃ肠�����A�����ŉ��
	if(new_objnum>0){
		if(new_label)	delete new_label;
		if(OTable)		delete OTable;
		end.free_obj_memory();
	}
	if(f<30)	return false;
	t2=clock();	
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//���̂����m�����炻�̃��x���̔w�i�Ɏ�荞��
	if(save_flag >= 0){
		re++;
		
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
//		for(int j =d_area.s_h; j < d_area.e_h; j++){
//			for(int i = d_area.s_w; i < d_area.e_w; i++){
				//���̂Ƃ��Č��m�������x���̉�f��w�i�ɂ���
				if(end.isDiff(i,j) == OBJECT*10+save_flag){
					Y(i,j) = pre.Y_d(i,j); // �ԈႢ�H modified by shimada 2012.12.14 //kawa
					pre.Y_d(i,j) = now.Y_d(i,j);
					pre.r_d(i,j) = now.r_d(i,j);
					pre.b_d(i,j) = now.b_d(i,j);
					pre.g_d(i,j) = now.g_d(i,j);
					end.isDiff(i,j) = OBJECT;
					
				}else if(end.isDiff(i,j) == SHADOW){ //�e
					Y(i,j) = pre_bg.Y_d(i,j);
					pre.Y_d(i,j) = pre_bg.Y_d(i,j);
					pre.R(i,j)   = pre_bg.R(i,j);
					pre.G(i,j)   = pre_bg.G(i,j);
					pre.B(i,j)   = pre_bg.B(i,j);
					//end.isDiff(i,j) = OBJECT;
					
				}else if(end.isDiff(i,j)==HITO ||
					end.isDiff(i,j)==SKIN ||
					end.isDiff(i,j)==KURO){   //�l�̈�
						Y(i,j) = pre_bg.Y_d(i,j);
						pre.Y_d(i,j) = pre_bg.Y_d(i,j);
						pre.R(i,j)   = pre_bg.R(i,j);
						pre.G(i,j)   = pre_bg.G(i,j);
						pre.B(i,j)   = pre_bg.B(i,j);
                   
				}
				   
			}
		}
		//���̌��m�҂���Ԃɂ���
		save_flag = -1;
	}
#endif//############################################################################
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//���m����͈͂���ň͂�	
	setArea(d_area.s_w,d_area.s_h,d_area.e_w,d_area.e_h);
	DrawLineImg(&area);


	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			R(i,j) = end.R(i,j);
			G(i,j) = end.G(i,j);
			B(i,j) = end.B(i,j);
			isDiff(i,j) = end.isDiff(i,j);
		}
	}
	area = end.area;
	d_area = end.d_area;
	o_area = end.o_area;

///////////////////////////////////////////////////////////////////////////////////
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
		if(end.isDiff(i,j) == OBJECT||end.isDiff(i,j) == SHADOW||end.isDiff(i,j)==HITO ||
					end.isDiff(i,j)==SKIN ||
					end.isDiff(i,j)==KURO)
		{  pre.mask(i,j)=0;}
		else{pre.mask(i,j)=255;}

		}}

///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
#if 0//test
	/// display debug
	


	IplImage *image2 = cvCreateImage(cvSize(imageWidth,imageHeight), IPL_DEPTH_8U, 1);
	
	if( image2 != NULL ) {
		for(int i=2; i<imageHeight; i++ ) {
			for( int j=0; j<imageWidth; j++ ) {
				int ii = imageHeight - i + 1; // vertical flip
				cvSet2D(image2,i,j,cvScalar(pre.mask(j,ii)));
			}
		}
	}
	
	cvNamedWindow("thres_check");
	cvCreateTrackbar("threshold", "thres_check", &diff_threshold, 100, NULL);

	
	cvShowImage("image2", image2);
	cvWaitKey(1);
	cvReleaseImage(&image2);
#endif

///////////////////////////////////////////////////////////////////////////////////


	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	r_flag = false;

	if(re==1)	return true;
	return false;
	//return corr_flag;
}

//------------------------------------------------------------------------
// Name     : BackDiff(image &back,image &input)
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.23
// function : �w�i����
// Argument : image �w�i�摜
//            image ���͉摜
// return   : �Ȃ�
//Background subtraction 
//------------------------------------------------------------------------
void image::BackDiff(image &back,image &input,int b_threshold /* = 20*/)
{
	static double diff;//�P�x�l�̍�
	//const int b_threshold = 20;//������臒l

	//==============================================================
#if 1
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff = sqrt((back.Y_d(i,j) - input.Y_d(i,j)) * (back.Y_d(i,j) - input.Y_d(i,j)));
			//�����𒲂ׂ�w��͈͂ɂ��Čv�Z����
			if((j >= area.s_h  && j <= area.e_h) && (i >= area.s_w && i<= area.e_w)){
				if(diff < b_threshold){//����臒l�ȉ��Ȃ�w�i
					isDiff(i,j) = NON;
					dB(i,j) = dG(i,j) = dR(i,j) = 0;
				}
				else{//�w�i�ƍ������ł���
					isDiff(i,j) = BACK;
					dB(i,j) = dG(i,j) = dR(i,j) = 255;
				}
			}
			else {//�����𒲂ׂȂ������͂��̂܂ܔw�i�摜�ɂ���
				isDiff(i,j) = NON;
				if(diff < b_threshold)	dB(i,j) = dG(i,j) = dR(i,j) = 0;
				else	dB(i,j) = dG(i,j) = dR(i,j) = 255;
			}
		}
	}
#endif
	//==============================================================
}

void image::ElimWithCNCC(image &back, image &input)
{
	double cncc=0;
	double threshold = 0.9;
	threshold=((double)diff_cncc/100);
	//----------------------------------------------------------------test
	cvNamedWindow("thres_check");
	cvCreateTrackbar("diff_cncc", "thres_check", &diff_cncc, 100, NULL);
	cvWaitKey(1);
	cvCreateTrackbar("diff_hs", "thres_check", &diff_hs, 100, NULL);
    cvWaitKey(1);
	//-----------------------------------------------------------------test

	clock_t t1,t2;
	static double runningtime=0;
	static int cnt=0;
	t1=clock();
	static double timemax=0;
	static double timemin=1000.0;
	for( int w=0; w<imageWidth; w++ ){
		for( int h=0; h<imageHeight; h++ ){
			if(isDiff(w,h)==BACK){
				cncc = CalcCNCC5(back,input,w,h);
				if( cncc == CNCC_ERROR ){ //�G���[�i��{�I�ɋN���Ȃ��Ǝv�����ǁj
					isDiff(w,h) = NON;  //����ł����̂��H�H
					dB(w,h) = dG(w,h) = dR(w,h) = 0;
					dB(w,h) = 255;
				}else if(cncc == CNCC_ERROR2){  //���U�����ϒl��菬�����A���邭�Ȃ���(Not �e)
					isDiff(w,h) = BACK;
					dB(w,h) = dG(w,h) = dR(w,h) = 0;
					//dG(w,h) = 255;
				}else if(cncc == 10){           //���U�����ϒl��菬�����Ah��s�̑��֍�(�e)
					isDiff(w,h) = SHADOW;
					dB(w,h) = dG(w,h) = dR(w,h) = 128;
					/*dB(w,h) = dG(w,h) = dR(w,h) = 0;
					dR(w,h) = 255;*/
				}else if(cncc == 11){           //���U�����ϒl��菬�����Ah��s�̑��֒�(Not �e)
					isDiff(w,h) = BACK;
					dB(w,h) = dG(w,h) = dR(w,h) = 255;
					//dG(w,h) = dB(w,h) = 255;
				}else if(cncc==12){     //h=0 �v�Z�s��
					isDiff(w,h) = BACK;
					dB(w,h) = dG(w,h) = dR(w,h) = 255;
					//dR(w,h) = dB(w,h) = 255;
				}else{
					if( cncc > threshold){
						isDiff(w,h) = SHADOW;
						dB(w,h) = dG(w,h) = dR(w,h) = 128;
					}else{
						isDiff(w,h) = BACK;
					}
				}
			} else{
				dB(w,h) = dG(w,h) = dR(w,h) = 0;
			}
		}
	}
	t2=clock();
	double rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	runningtime+=(double)(t2-t1)/CLOCKS_PER_SEC;
	cnt++;
	timemax=rtime>timemax?rtime:timemax;
	timemin=rtime<timemin?rtime:timemin;
}

double image::CalcCNCC5(image &back, image &input, int x, int y)
{

	int windowSizeX = 5, windowSizeY = 5;
	int windowArea = windowSizeX * windowSizeY;

	if(	   x < windowSizeX / 2 || x >= imageWidth - windowSizeX / 2
		|| y < windowSizeY / 2 || y >= imageHeight - windowSizeY / 2 )
		return 1.0;

	double bH, bS, bL, fH, fS, fL;
	double bchx, bchy, fchx, fchy;
	double mean_bchx = 0.0, mean_bchy = 0.0;
	double mean_fchx = 0.0, mean_fchy = 0.0;
	double mean_bH = 0.0, mean_bS = 0.0, mean_bL = 0.0;
	double mean_fH = 0.0, mean_fS = 0.0, mean_fL = 0.0;
	double ip_aa = 0.0, ip_bb = 0.0;
	double ip_ab = 0.0;

	double cip_aa = 0.0, cip_bb = 0.0;
	double cip_ab = 0.0;

    double bfx=0.0,bfy=0.0;//kawa

	for( int i=x-(windowSizeX/2); i<=x+(windowSizeX/2); i++ ){
		for( int j=y-(windowSizeY/2); j<=y+(windowSizeY/2); j++ ){
			bH = this->getH2(back.R(i,j),back.G(i,j),back.B(i,j));
			bS = this->getS3(back.R(i,j),back.G(i,j),back.B(i,j));
			bL = this->getL(back.R(i,j),back.G(i,j),back.B(i,j));
			fH = this->getH2(input.R(i,j),input.G(i,j),input.B(i,j));
			fS = this->getS3(input.R(i,j),input.G(i,j),input.B(i,j));
			fL = this->getL(input.R(i,j),input.G(i,j),input.B(i,j));

			mean_bL += bL;
			mean_fL += fL;

			bchx = bS * sin( RADIAN(bH) );
			bchy = bS * cos( RADIAN(bH) );
			fchx = fS * sin( RADIAN(fH) );
			fchy = fS * cos( RADIAN(fH) );

			//�ǉ�(h,s�̑��ւ����߂邽��)
			cip_aa += (bchx * bchx + bchy * bchy);
			cip_bb += (fchx * fchx + fchy * fchy);
			cip_ab += (bchx * fchx + bchy * fchy);

			ip_aa += ( bchx * bchx + bchy * bchy + bL * bL );
			ip_bb += ( fchx * fchx + fchy * fchy + fL * fL );
			ip_ab += ( bchx * fchx + bchy * fchy + bL * fL );

			bfx+=pow(bchx-fchx,2.0);//kawa
			bfy+=pow(bchy-fchy,2.0);
		}
	}

	mean_bL /= (double)windowArea;
	mean_fL /= (double)windowArea;

	double mean_a = ip_aa / (double)windowArea;
	double mean_b = ip_bb / (double)windowArea;

	double var_a  = ip_aa - (double)windowArea * pow(mean_bL, 2.0 );
	double var_b  = ip_bb - (double)windowArea * pow(mean_fL, 2.0 );

	double cncc=0;

/*	//���U�����ϒl��菬��������ꍇ
	if((var_a < mean_a/10.0) || (var_b < mean_b/10.0)){
		if(mean_bL > mean_fL){ //�Â��Ȃ����ꍇ(���x���������Ȃ�)
			//h��s�̑��ւ����߂�
			if(cip_ab == 0 || sqrt(cip_aa * cip_bb)==0) //h,s=0(�v�Z�s��)
				return 12;
			double chs = (cip_ab) / sqrt(cip_aa * cip_bb);
			if(chs < 0.1 && chs > -0.1)   //h��s�̑��ւ��������iNot �e�j
				return 11;
			return 10;      //h��s�̑��ւ������i�e�j
		}
		return CNCC_ERROR2; //���邭�Ȃ����ꍇ(���x���傫���Ȃ�)
	}*/

	if((var_a < mean_a/10.0) && (var_b < mean_b/10.0)){
		if(mean_bL > mean_fL){ //�Â��Ȃ����ꍇ(���x���������Ȃ�)
			//h��s�̑��ւ����߂�
			//if(cip_ab == 0 || sqrt(cip_aa * cip_bb)==0) //h,s=0(�v�Z�s��)
				//return 12;
			//double chs = (cip_ab) / sqrt(cip_aa * cip_bb);

			bfx=sqrt(bfx);
			bfy=sqrt(bfy);
			double chs=(bfx+bfy)/(windowArea*2);

			if(chs < ((double)diff_hs/100) && chs > 0.0)   
				return 10;  //�O�i�Ɣw�i��H��S�̍����Ⴂ�i�e�j
			    
			    return 11;  //h��s�̍��������iNot �e�j
		}
		return CNCC_ERROR2; //���邭�Ȃ����ꍇ(���x���傫���Ȃ�)
	}
	else if((var_a < mean_a/20.0) || (var_b < mean_b/20.0)){//�ǂ��炩�̕��U���\���傫���ꍇ���̌��m
		//if(mean_bL > mean_fL){ //�Â��Ȃ����ꍇ(���x���������Ȃ�)
		//return 10;//�e
	    // }
		
		return 11;}//����



	cncc = ( ip_ab - mean_fL*mean_bL*(double)windowArea ) / sqrt( var_a * var_b );

	//cncc��-1�`1�łȂ�
	if( cncc < -1.000001 || cncc > 1.000001 ){
		return CNCC_ERROR;
	}

	return cncc;
}

double image::getS3(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char rgbmax, rgbmin;

	rgbmax = MAXC(r, MAXC(g,b));
	rgbmin = MINC(r, MINC(g,b));

	double s = ( (double)rgbmax - (double)rgbmin ) / 255.0;

	return s;
}

double image::getH2(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char rgbmax, rgbmin;

	rgbmax = MAXC(r, MAXC(g,b));
	rgbmin = MINC(r, MINC(g,b));

	if( rgbmax == rgbmin ){
		return 0.0;
	} else {
		double max_min = (double)rgbmax - (double)rgbmin;
		double cr = ( (double)rgbmax - (double)r ) / max_min;
		double cg = ( (double)rgbmax - (double)g ) / max_min;
		double cb = ( (double)rgbmax - (double)b ) / max_min;
		double h;
		if( r == rgbmax ){
			h = cb - cg;
		} else if( g == rgbmax ){
			h = 2.0 + cr - cb;
		} else {
			h = 4.0 + cg - cr;
		}
		h *= 60.0;
		if( h < 0 )
			h += 360.0;

		return h;
	}
}

//L for HSL
double image::getL(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char rgbmax,rgbmin;

	rgbmax = MAXC(r,MAXC(g,b));
	rgbmin = MINC(r,MINC(g,b));

	double l = ((double)rgbmax + (double)rgbmin) / 2.0;

	return l / 255.0;
}

//------------------------------------------------------------------------
// Name     : CopyImg()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.9.19
// function : �摜�̃f�[�^���R�s�[
// Argument : image  �R�s�[�����摜
//            image�@�R�s�[����摜�f�[�^
// return   : bool�ϐ�
//------------------------------------------------------------------------
void image::CopyImg(image &img,const char type)
{
	switch(type){
		case 'Y'://�P�x�l�ŏo�͂���ꍇ
			for(int j=0; j<imageHeight; j++){
				for(int i=0; i<imageWidth; i++){
					R(i,j) = img.Y(i,j); 	
					G(i,j) = img.Y(i,j); 
					B(i,j) = img.Y(i,j); 
				}
			}
			break;
		case 'R'://�q�f�a�ŏo�͂���ꍇ
			for(int j=0; j<imageHeight; j++){
				for(int i=0; i<imageWidth; i++){
					R(i,j) = img.R(i,j); 	
					G(i,j) = img.G(i,j); 
					B(i,j) = img.B(i,j); 
				}
			}
			break;
		default://����ȊO
			for(int j=0; j<imageHeight; j++){
				for(int i=0; i<imageWidth; i++){
					R(i,j) = img.R(i,j); 	
					G(i,j) = img.G(i,j); 
					B(i,j) = img.B(i,j); 
				}
			}
			break;
	}
}

//------------------------------------------------------------------------
// Name     : CopyImg()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.9.19
// function : �摜������l�ŏ���������
// Argument : image  �l������摜
//            int�@  �������̒l
// return   : bool�ϐ�
//------------------------------------------------------------------------
void image::CopyImg(image &img1,int val)
{
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			img1.R(i,j) = val;
			img1.G(i,j) = val;
			img1.B(i,j) = val;
		}
	}
}


//---------------------------------------------------
// OR���Ƃ�
// �d�Ȃ��Ă��邱�Ƃ�͐F��ς���
//---------------------------------------------------
void image::ORImage(image &img1,image &img2,image &result)
{
	for(int j=0; j< imageHeight; j++){
		for(int i=0; i< imageWidth; i++){
			if(img1.PixColor(i,j) == image::GREEN && img2.PixColor(i,j) != image::GREEN){ 
				result.R(i,j) = 0;
				result.G(i,j) = 255;
				result.B(i,j) = 0;
			}
			else if(img1.PixColor(i,j) != image::GREEN && img2.PixColor(i,j) == image::GREEN){ 
				result.R(i,j) = 0;
				result.G(i,j) = 255;
				result.B(i,j) = 0;
			}
			else if(img1.PixColor(i,j) == image::GREEN && img2.PixColor(i,j) == image::GREEN){ 
				result.R(i,j) = 0;
				result.G(i,j) = 255;
				result.B(i,j) = 0;
			}
			else{
				result.R(i,j) = 0;
				result.G(i,j) = 0;
				result.B(i,j) = 0;

			}
		}
	}
}

void image::SubUnderImg(image &img1,image &img2,image &result)
{

	for(int j=0; j< imageHeight; j++){
		for(int i=0; i< imageWidth; i++){
			if(img1.PixColor(i,j) == image::GREEN && img2.PixColor(i,j) != image::GREEN){ 
				result.R(i,j) = 0;
				result.G(i,j) = 255;
				result.B(i,j) = 0;
			}
			else if(img1.PixColor(i,j) != image::GREEN && img2.PixColor(i,j) == image::GREEN){ 
				result.R(i,j) = result.G(i,j) = result.B(i,j) = 0;
			}
			else if(img1.PixColor(i,j) == image::GREEN && img2.PixColor(i,j) == image::GREEN){ 
				result.R(i,j) = result.G(i,j) = result.B(i,j) = 0;
			}
			else{
				result.R(i,j) = 0;
				result.G(i,j) = 0;
				result.B(i,j) = 0;
			}
		}
	}
}

//-------------------------------------------------------
// Name     : PixColor()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.9
// function : �摜�̐F�𓾂�
// Argument : int x���W
//            int y���W
// return   : �F�ԍ�(enum�Œ�`����Ă���)
//--------------------------------------------------------
int image::PixColor(const int i, const int j)
{
	int color_num = 0;

	if(R(i,j) == 255 && G(i,j) == 0 && B(i,j) == 0)     color_num = RED;
	if(R(i,j) == 0   && G(i,j) == 255 && B(i,j) == 0)   color_num = GREEN;
	if(R(i,j) == 0   && G(i,j) == 0   && B(i,j) == 255) color_num = BLUE;
	if(R(i,j) == 255 && G(i,j) == 255 && B(i,j) == 255) color_num = WHITE;
	if(R(i,j) == 0   && G(i,j) == 0   && B(i,j) == 0)   color_num = BLACK;

	return color_num;
}

void image::draw(int x, int y, int r, int g, int b){
	R(x,y) = r;
	G(x,y) = g;
	B(x,y) = b;
}


//------------------------------------------------------------------------
// Name     : DrawLineImg()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.8.22
// function : �摜�ɐ�������
// Argument : image�^�̉摜�f�[�^�C�n�_�����C�n�_���C�I�_�����C�I�_��
// return   : �Ȃ�
//------------------------------------------------------------------------
void image::DrawLineImg(image &img,Area *area)
{
	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(j==area->s_h || i==area->s_w	
				|| j==area->e_h|| i==area->e_w){
					img.draw(i,j,0,0,255);
			}
		}
	}
}

/*	DrawLineImg
 *	function:	Area�̓����ł���ԊO���̉�f���(R,G,B)=(0,0,255)�œh��
 *	��:�摜�̊e��f
 *	��:Area�͈͉̔�f
 *	��:Area���̈�ԊO���̉�f(�h��ꏊ)
 *
 *	������������      ������������      ������������
 *	������������      ������������      ������������
 *	������������  ��  ������������  ��  ������������
 *	������������      ������������      ������������
 *	������������      ������������      ������������
 *	arg		:	Area	: area	:	
 *	return	:	none
 */
void image::DrawLineImg(Area *area)
{
	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(j == area->s_h || i == area->s_w || 
				j == area->e_h || i == area->e_w){
					draw(i,j,0,0,255);
			}
		}
	}
}
/*  M_estimator
 *  function:
 *  arg		:	image	: pre	:
			:	image	: now	:
			:	image	: pre_bg:
 *	return	:	none
 */


void image::M_estimator(image &pre,image &now, image &pre_bg)
{
	int Height=imageHeight;
	int Width=imageWidth;
	const double a = 0.8;                //�݌v�덷�̋L������ ����0.8�ŌŒ�
	double eta;                          //���̕ω��ɑ΂���K����
	const double eta_min = 0.001;        //�K�����̍ŏ��l
	long int M = Height*Width;			//��f��M
	double Eps_x;                        //�����f�ɂ�������͉摜�Ɣw�i�摜�̋P�x�l�̍�
	double psi_x;                        //�ς̔�����(��x)
	static bool flag = false;            //�������t���O
	static long int counter;             //�t���[�����J�E���g
	double *dist = new double[M];       //�P�x�l�̍��̐�Βl�̓��
	static int tem_counter=0;			//�ꎞ�t���[����
	static int *something=new int[Width*Height];	//����or�l�̗̈�
	static int *n_something=new int[Width*Height];	//�w�i
	static bmp before;

	//static int *check=new int[Width*Height];	//�w�i

	static bool toriaezu=false;//test

	++counter;//�t���[����
	++tem_counter; //�ꎞ�t���[��
	clock_t t1=clock();
	//������(�P��̂ݍs��)�w�i�摜�̗݌v�덷Et=0,��f�̋P�x�l�S��128�ɂ���
	//���̏����l��K�؂Ȓl�ɂ���K�v�A��
	if(flag == false){
		for(int j=0; j<Height; j++){
			for(int i=0; i<Width; i++){
				pre.Y_d(i,j) = 128.0;
				pre.Y(i,j)   = 128;
				pre.E(i,j)   = 0.0;
				something[j*Width+i]=0;
				n_something[j*Width+i]=0;
				//�Ƃ肠����
				pre.r_d(i,j) = now.r_d(i,j);
				pre.g_d(i,j) = now.g_d(i,j);
				pre.b_d(i,j) = now.b_d(i,j);
				pre.R(i,j) = pre.G(i,j) = pre.B(i,j) = 0;
				pre.mask(i,j)=255;
			}
		}
		flag = true;//�t���O���I��
	}
	clock_t t2=clock();		
	double rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	if(flag == true){//�����摜�������
		//�P�x�l�̍��̓������߂�
		for(int j=0; j<Height; j++){
			for(int i=0; i<Width; i++){
				//���̐�Βl�̓����i�[
				dist[j*Width+i] = fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))) 
					* fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j)));

			}
		}

		//fprintf(fo,"�����̉�f�� = %d \n",diff_count);
	t2=clock();
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;

	t1=clock();

#if 1//���P�ɂ�����ł̎�������������
	clock_t _t1=clock();

	double sigma_x,sigma_y;              //�W���΍���x,��y
	int  ave_pre,ave_now;                //����摜�̋P�x�l�̕��ϒl
	long int sum_Cov = 0;                //x��y���ꂼ��̕��ςƍ��̍��v
	long int sum_pre = 0,sum_now = 0;    //����摜�̋P�x�l���v
	long int sum_x = 0,sum_y = 0;        //�����̕��ςƂ̍��̍��v�l


	//#################################
	//��O�l�����߂�
	double med;//���W�A��(�����l)
	double sigma_ber;//�W���΍��̐���l
	double Exception_val;//��O�l
	double di;//�P�x�l�̍��̐�Βl�̓��̕�����
	long int Excep_num = 0;//��O�l�ł͂Ȃ���f��

	//qsort O(nlogn)
	//M:76800
	//O(76800*log(76800)=371950)
	std::sort(dist,dist+M-1);//�N�C�b�N�\�[�g���s��
	//��f������̂Ƃ��͂��̂܂ܒ����l
	if(M%2==1){
		med = dist[(M-1)/2];
	}
	//��f���������̂Ƃ��͒����̂Q�l�̕���
	if(M%2==0){
		med = (dist[M/2]+dist[M/2-1])/2;
	}

	//�W���΍��̐���l�����߂�v�Z
	sigma_ber = 1.4826 * (double)(1 + (5 / (M-1))) * sqrt(med);
	//2.5*�Ђ��O�l�Ƃ���
	Exception_val = sigma_ber * 2.5;

	//fprintf(fo,"��O�l = %f \n",Exception_val);

	//Heght:240,Width:320
	//76800 loop
	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			di =sqrt( fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))) 
				* fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))));
			//��O�l�ł͂Ȃ��Ȃ��
			if(Exception_val > di){
				sum_pre += (long)pre.Y_d(i,j);//�w�i�摜�̋P�x�l�̍��v
				sum_now += (long)now.Y_d(i,j);//���͉摜�̋P�x�l�̍��v
				++Excep_num;
			}
		}
	}
	clock_t _t2=clock();		
	double _rtime=(double)(_t2-_t1)/CLOCKS_PER_SEC;
	_t1=clock();


	//fprintf(fo,"sum_pre->%d sum_now->%d\n",sum_pre,sum_now);

	//��O�l�łȂ��Ƃ��̕��ϒl
	ave_pre = sum_pre / Excep_num; //�w�i�摜�̕��ϒl
	ave_now = sum_now / Excep_num; //���͉摜�̕��ϒl
	//fprintf(fo,"pre->%d now->%d\n",ave_pre,ave_now);	



	int diff_x,diff_y;//���ςƂ̍�
	double Cov_xy;//�����U
	double r_rob;//���o�X�g���֌W��r_rob

	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			di = sqrt(fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))) 
				* fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))));
			if(Exception_val > di){
				diff_x = (int)(pre.Y_d(i,j) - ave_pre);
				diff_y = (int)(now.Y_d(i,j) - ave_now);

				//�w�i�摜�̂���l�̕��ςƂ̍��̓��a
				sum_x   += diff_x * diff_x;
				//���͉摜�̂���l�̕��ςƂ̍��̓��a
				sum_y   += diff_y * diff_y;
				//�w�i�Ɠ��͉摜�̋P�x�l�ƕ��ς̍����|�������̘̂a
				sum_Cov += diff_x * diff_y;
			}
		}
	}
	_t2=clock();		
	_rtime=(double)(_t2-_t1)/CLOCKS_PER_SEC;
	_t1=clock();

	//�w�i�Ɠ��͉摜���ꂼ��̕W���΍��Ƌ����U������
	//���o�X�g�e���v���[�g�}�b�`���O���s�����֌W�������߂�
	sigma_x = sqrt((double)(sum_x / Excep_num));  //�w�i�摜�W���΍�  ��x
	sigma_y = sqrt((double)(sum_y / Excep_num));  //���͉摜�̕W���΍���y
	Cov_xy  = (double)(sum_Cov / Excep_num);       //x��y�̋����U
	r_rob   = Cov_xy / (sigma_x * sigma_y);  //���֌W��r_rob�����߂�

	//fprintf(fo,"��x=%f ",sigma_x);
	//fprintf(fo,"��y=%f\n",sigma_y);
	//fprintf(fo,"�����U%f\n",Cov_xy);
	//fprintf(fo,"%f\n",r_rob);


	//���֒lr�� -1 �� r �� 1 �ɗ}����
	if(sigma_x == 0.0 || sigma_y == 0.0) r_rob = 0.0;


	//���ւ��Ⴗ����Ƃ��̒���
	if(r_rob<=0.0){
		r_rob=eta_min;
	}

	//���ւ��Ⴂ�Ƃ��ɋ}���Ȕw�i�X�V�̏���
	//�i臒l�ɂ�肩�Ȃ荂���ōX�V��������Ύg���j
	if(r_rob<=0.30){
		tem_counter=0;
	}

	//�K������ = f(r_rob) �̎�������
	//���݂͂��̎����œK�Ƃ��Ă�
	//0.995������Œ��悭�������Ȃ邩��
	if(r_rob<0.95){
		eta=1.0;
	}
	else{
		r_flag = true;
		eta=-400.0*(r_rob-0.95)*(r_rob-0.95)+1.0;
	}

	//�K�����S���i�܂Ȃ��̂�����邽�߂ɔ����Ȓl���悹��
	eta  = eta + eta_min;	

	//�K������1.0�ȓ��ɗ}����
	if(eta>1.0){
		eta=1.0;
	}
	_t2=clock();		
	_rtime=(double)(_t2-_t1)/CLOCKS_PER_SEC;

#endif//�ł̎������� END
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

		//�ł��͂��߂̐��t���[���͍�������
		if(tem_counter < 30){
			eta = 0.9999;
			r_flag = false;
		}

		double tem;		//Eps_x�̐��̒l
	
		
		//##############################################################
		//���o�X�g���v�Ɋ�Â��w�i�̋P�x�l�𐄒�
		//�P�x�l��(theta)�𐄒肵�w�i�摜�̒l�ɑ��

		for(int j=0; j<Height; j++){
			for(int i=0; i<Width; i++){

				//���[�ɂ�蕨�̗̈��ێ���������Γ��[�����w��(30)
				if(something[j*Width+i]<30){
			
					//�P�x�l�̍���(x) = �w�i�摜�� - ���͉摜xt
					Eps_x = (pre.Y_d(i,j) - now.Y_d(i,j));

					//�P�x���̐��̒l���o��
					tem=fabs(Eps_x);
			
					double re_val=0.0;	//�X�V���s���P�x����臒l
					
					

					//���݂�������g��(���ɐ��l�I�ȍ����Ȃ�)
					re_val=45.0/(pow(r_rob,26.0));

					//0.998�̂Ƃ��ɂQ�O���炢�i�قڍX�V���Ȃ��j�ɂȂ�悤�ɐݒ�
					/*re_val=(double)1000.0*(1.0-r_rob)*10.0;
					if(re_val>255){
						re_val=255;
					}*/
					
					int back=false;
					//�ǂ����̊֐����g�����̔��f
					//�E�ŏ��̃t���[��
					//�E�S�̑��ւ��Ⴂ�������ȋP�x�����Ȃ��Ƃ�
					//�E���S�Ȕw�i�ω�
					/////////////////////////////////////////////////////////
					//�d�݂����߂鑊�ւ�臒l�̐ݒ�
					///////////////////////////////////////////////////////
					//if((tem_counter<30) || (r_rob<0.998&&re_val>tem)){//default
					if((tem_counter<30) || (r_rob<0.990&&re_val>tem)){
					//if((tem_counter<30) || (r_rob<0.9920&&re_val>tem)){
					
						psi_x = tanh(Eps_x / 50.0);
						back=true;
					}
					
					//�E�S�̂̑��ւ������i�ʏ��ԁj
					//�E�S�̂̑��ւ��Ⴍ�Ă��P�x�����傫���Ƃ�
					else{
						///////////////////////////////////////////
						//�d�݂����߂�P�x����臒l�̐ݒ�
						///////////////////////////////////////////
						//Eps_x=Eps_x/(double)5.0;//default
						Eps_x=Eps_x/(double)1.0;	//臒l����
						
						psi_x = 2*Eps_x/((1.0+Eps_x*Eps_x)*(1.0+Eps_x*Eps_x));
						psi_x = psi_x*2;
						if(psi_x>(double)1.0){
							psi_x=(double)1.0;
						}
					}


					//�e���l�������w�i����̂���
					pre_bg.E(i,j)   = pre.E(i,j);
					pre_bg.Y_d(i,j) = pre.Y_d(i,j);
					pre_bg.R(i,j) = pre.R(i,j);
					pre_bg.G(i,j) = pre.G(i,j);
					pre_bg.B(i,j) = pre.B(i,j);
			
					if(pre.mask(i,j)=255||back==true){
					//�L������������
					pre.E(i,j) = psi_x + (a * pre.E(i,j));
					//(R(i,j) * 0.299 + G(i,j) * 0.587 + B(i,j) * 0.114);
					//�w�i�摜�̋P�x�l�X�V
					//Et���ŏ��Ƃ���w�i�̋P�x�l��t���ŋ}�~���@�ŋ��߂�
					//[����]��t = ��t-1 -��(dEt / d��)
					pre.Y_d(i,j) = pre.Y_d(i,j) - (eta * pre.E(i,j));
					pre.Y(i,j) = (unsigned char)pre.Y_d(i,j);
//					pre.R(i,j) = pre.r_d(i,j) - (eta * pre.E(i,j));
//					pre.G(i,j) = pre.g_d(i,j) - (eta * pre.E(i,j));
//					pre.B(i,j) = pre.b_d(i,j) - (eta * pre.E(i,j));
					pre.r_d(i,j) = pre.r_d(i,j) - (eta * pre.E(i,j));
					pre.g_d(i,j) = pre.g_d(i,j) - (eta * pre.E(i,j));
					pre.b_d(i,j) = pre.b_d(i,j) - (eta * pre.E(i,j));
//					pre.r_d(i,j) = pre.r_d(i,j) - (eta * 0.299*pre.E(i,j));
//					pre.g_d(i,j) = pre.g_d(i,j) - (eta * 0.587*pre.E(i,j));
//					pre.b_d(i,j) = pre.b_d(i,j) - (eta * 0.114*pre.E(i,j));
					}
					else{
                    pre.Y_d(i,j) = pre.Y_d(i,j);
					pre.Y(i,j) = (unsigned char)pre.Y_d(i,j);
					pre.r_d(i,j) = pre.r_d(i,j);
					pre.g_d(i,j) = pre.g_d(i,j);
					pre.b_d(i,j) = pre.b_d(i,j);
					}



					pre.R(i,j) = (unsigned char)pre.r_d(i,j);
					pre.G(i,j) = (unsigned char)pre.g_d(i,j);
					pre.B(i,j) = (unsigned char)pre.b_d(i,j);
					//unsigned char �^�ɒl��߂�
					//pre.Y(i,j) = pre.Y_d(i,j);

					//pre.R(i,j) = pre.G(i,j) = pre.B(i,j) = (unsigned char)pre.Y_d(i,j);

					//�e�����p
					//�w�i�摜��RGB
//					pre.R(i,j) = pre.R(i,j) - (eta * pre.E(i,j));
//					pre.G(i,j) = pre.G(i,j) - (eta * pre.E(i,j));
//					pre.B(i,j) = pre.B(i,j) - (eta * pre.E(i,j));

					
					//�l�̏�����
					psi_x = Eps_x = 0.0;

				}
				
				//���[
				//�w�i�ł͂Ȃ��Ɛ���
				if(tem>20.0&&r_rob>0.9950){
					something[j*Width+i]+=1;
					n_something[j*Width+i]=0;
				}
				//�w�i�����Ɛ���
				if(tem<30.0&&r_rob>0.9920){
					n_something[j*Width+i]+=1;
					
					if(n_something[j*Width+i]==3){
						something[j*Width+i]=0;
						n_something[j*Width+i]=0;
					}
				}	
			}
		}

	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;



		//for(int i=0;i<Width;i++){
		//	for(int j=0;j<Height;j++){
		//		if(something[j*Width+i]!=0){
		//			check[j*Width+i]=1;
		//		}
		//		else{
		//			check[j*Width+i]=0;

		//			/////���̒i�K�̂q�f�a�l���L��//////
		//			//�w�i��f���t���O�������Ă��Ȃ��Ƃ�
		//		/*	if(toriaezu==false){
		//				before.R(i,j)=R(i,j);
		//				before.G(i,j)=G(i,j);
		//				before.B(i,j)=B(i,j);
		//			}*/

		//		}

		//		if(toriaezu==false){
		//		if(counter==49){
		//			before.R(i,j)=R(i,j);
		//			before.G(i,j)=G(i,j);
		//			before.B(i,j)=B(i,j);
		//		}
		//		}
		//	}
		//}
		

		//##############################################################
	}
	delete dist;
}

//----------------------------------------------------------
// Name     : qsort()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.7
// function : �N�C�b�N�\�[�g
// Argument : double�^�z��Cint�^Index�ԍ� int�^Index�ԍ�
// return   : �Ȃ�
//----------------------------------------------------------
void image::qsort(double dat[], int first, int last)
{
	int i, j;
	double x, t;

	x = dat[(first + last) / 2];
	i = first;
	j = last;
	for(;;i++,j--) {
		while (dat[i] < x) i++;
		while (x < dat[j]) j--;
		if (i >= j) break;
		t = dat[i];
		dat[i] = dat[j];
		dat[j] = t;
	}
	if (first  < i - 1) qsort(dat, first , i - 1);
	if (j + 1 < last) qsort(dat, j + 1, last);
}

//--------------------------------------------------
// Name     : void image::FrameDiff()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.5.29
// function : �t���[���ԍ����ɂ�铮�̌��o
// Argument : �Ȃ�
// return   : �Ȃ�
//--------------------------------------------------
void image::FrameDiff(int frame)
{
	static double diff;//�P�x�l�̍�
	const int b_threshold = 20;//������臒l

	static image back;

	setArea(5,40,310,230);

	//���݂̓��͉摜�̋P�x�l�̂ݎg�p(�P�x�l��M����)
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			Y_d(i,j) = (double)(R(i,j) * 0.299 + G(i,j) * 0.587 + B(i,j) * 0.114);
			if(frame==0){
				back.Y_d(i,j) = Y_d(i,j);
			}
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff = sqrt((back.Y_d(i,j) - Y_d(i,j)) * (back.Y_d(i,j) - Y_d(i,j)));
			//�����𒲂ׂ�w��͈͂ɂ��Čv�Z����
			if((j >= area.s_h  && j <= area.e_h) && (i >= area.s_w && i<= area.e_w)){
				if(diff < b_threshold){//����臒l�ȉ��Ȃ�w�i
					isDiff(i,j) = NON;
					dB(i,j) = dG(i,j) = dR(i,j) = 0;
				}
				else{//�w�i�ƍ������ł���
					isDiff(i,j) = BACK;
					dB(i,j) = dG(i,j) = dR(i,j) = 255;
				}
			}
			else {//�����𒲂ׂȂ������͂��̂܂ܔw�i�摜�ɂ���
				isDiff(i,j) = NON;
				if(diff < b_threshold)	dB(i,j) = dG(i,j) = dR(i,j) = 0;
				else	dB(i,j) = dG(i,j) = dR(i,j) = 255;
			}
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
				back.Y_d(i,j) = Y_d(i,j);
		}
	}

	
#if 0 //�摜�̕ۑ�
	//char fname1[40];
	//sprintf(fname1,"pre/%07d.png",frame);
	//pre.save(fname1);
	bmp now;
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			now.R(i,j) = dR(i,j);
			now.B(i,j) = dB(i,j);
			now.G(i,j) = dG(i,j);
		}
		char fname2[40];
		sprintf(fname2,"result/%07d.png",frame);
		now.save(fname2);
	}
#endif

	/*unsigned char r1,r2;
	unsigned char g1,g2;
	unsigned char b1,b2;
	static image rev1(imageWidth,imageHeight),rev2(imageWidth,imageHeight),
		move(imageWidth,imageHeight),move_temp(imageWidth,imageHeight);
	const int diff = 20;

	rev2 = rev1;
	move_temp = move;

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			r2=(unsigned char)sqrt((double)((rev2.R(i,j) - rev1.R(i,j))*(rev2.R(i,j) - rev1.R(i,j))));
			g2=(unsigned char)sqrt((double)((rev2.G(i,j) - rev1.G(i,j))*(rev2.G(i,j) - rev1.G(i,j))));
			b2=(unsigned char)sqrt((double)((rev2.B(i,j) - rev1.B(i,j))*(rev2.B(i,j) - rev1.B(i,j))));

			if(r2 < diff && g2 < diff && b2 < diff){
				move.R(i,j) = move.G(i,j) = move.B(i,j) = 0;
			}
			else move.R(i,j) = move.G(i,j) = move.B(i,j) = 255;
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			r1=(unsigned char)sqrt((double)((R(i,j) - rev1.R(i,j))*(R(i,j) - rev1.R(i,j))));
			g1=(unsigned char)sqrt((double)((G(i,j) - rev1.G(i,j))*(G(i,j) - rev1.G(i,j))));
			b1=(unsigned char)sqrt((double)((B(i,j) - rev1.B(i,j))*(B(i,j) - rev1.B(i,j))));

			if(r1 < diff && g1 < diff && b1 < diff){
				move.R(i,j) = move.G(i,j) = move.B(i,j) = 0;
			}
			else move.R(i,j) = move.G(i,j) = move.B(i,j) = 255;
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			rev1.R(i,j) = R(i,j);
			rev1.G(i,j) = G(i,j);
			rev1.B(i,j) = B(i,j);
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			if((move_temp.R(i,j) - move.R(i,j)) != 0) move.R(i,j) = 0;
			if((move_temp.G(i,j) - move.G(i,j)) != 0) move.G(i,j) = 0;
			if((move_temp.B(i,j) - move.B(i,j)) != 0) move.B(i,j) = 0;
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			R(i,j) = move.R(i,j);
			G(i,j) = move.G(i,j);
			B(i,j) = move.B(i,j);
		}
	}*/
}


//---------------------------------------------------------------------
// Name     : void image::skinExtraction()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.10.19
// function : ���F���o
// Argument : image �����}�X�N�摜
// return   : �Ȃ�
//---------------------------------------------------------------------
void image::skinExtraction(image &mask)
{
	int Yi, Cb, Cr;


	//:::::::::::::::::::::::::
	// RGB��YCbCr
	//:::::::::::::::::::::::::
	for(int j = 0; j < imageHeight; j++){
		for(int i = 0;i < imageWidth; i++){
			//�����ƂȂ�����f��������
			if(mask.isDiff(i,j) == BACK){
				Yi = (int)( 0.299  * mask.R(i,j) + 0.587  * mask.G(i,j) + 0.114  * mask.B(i,j)); // Y
				Cb = (int)(-0.1687 * mask.R(i,j) - 0.3312 * mask.G(i,j) + 0.5000 * mask.B(i,j)); // Cb
				Cr = (int)( 0.5000 * mask.R(i,j) - 0.4183 * mask.G(i,j) - 0.0816 * mask.B(i,j)); // Cr

				//���F��臒l�Ɉȓ��Ȃ�
				if( ((Yi > 20) && (Yi < 240))
					&& ((Cb > -30) && (Cb < 20))
					&& ((Cr > 10) && (Cr < 30)) ){
						mask.B(i,j) = 0;
						mask.G(i,j) = 0;
						mask.R(i,j) = 255;
					}
				else {//���F����Ȃ�������0�ɂ��Ă���
					mask.R(i,j) = 0;
					mask.G(i,j) = 0;
					mask.B(i,j) = 0;
				}
			}
			//��������Ȃ���f��
			if(mask.isDiff(i,j) == NON){
				mask.R(i,j) = mask.G(i,j) = mask.B(i,j) = 0;
			}
		}
	}
	//RGB��YCbCr�����܂�
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
}

//---------------------------------------------------------------------
// Name     : void image::skinExtraction()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.17
// function : ���F���o
// Argument : �Ȃ�
// return   : �Ȃ�
//---------------------------------------------------------------------
//����������l�̎�����o���邽�߂ɐF���Ƃɑ�����t����
//  
//�����̂Ƃ���ǂ̐F��Ԃ��g�p���邩���߂Ă��Ȃ�(2005.10.19)
//  �@RGB��YCbCr
//  �ARGB��
//
//�����̖т̐F�́H
////�@if(R(i,j) < 48 && G(i,j) < 55 && B(i,j) <50){}
////�AY(�P�x�l50�ȉ�)
//---------------------------------------------------------------------
void image::skinExtraction()
{
	int Yi, Cb, Cr;

	//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	//:RGB��YCbCr
	for(int j = 0; j < imageHeight; j++){
		for(int i = 0;i < imageWidth; i++){
			//�����ƂȂ�����f�ɂ��ă��x����t����
			if(isDiff(i,j) == BACK){
				Yi = (int)( 0.299  * R(i,j) + 0.587  * G(i,j) + 0.114  * B(i,j)); // Y
				Cb = (int)(-0.1687 * R(i,j) - 0.3312 * G(i,j) + 0.5000 * B(i,j)); // Cb
				Cr = (int)( 0.5000 * R(i,j) - 0.4183 * G(i,j) - 0.0816 * B(i,j)); // Cr

				//���F��臒l�Ɉȓ��Ȃ甧�F�̃��x����t����
				if( ((Yi > 20) && (Yi < 240))
					&& ((Cb > -30) && (Cb < 20))
					&& ((Cr > 10) && (Cr < 30)) ){
						isDiff(i,j) = SKIN;
						//SetPixColor(i,j,0,255,0);
					}
				else if(Yi < 50){//���F��臒l�ȓ��Ȃ甯�F�̃��x����t����
					isDiff(i,j) = HAIR;
				}
				else {//���F�ł����F�ł��Ȃ�������u����ȊO�v�̃��x��
					isDiff(i,j) = NEITHER;
				}
			}
			//��������Ȃ���f�́u�����Ȃ��v�ł��̂܂܂ɂ��Ă���
			if(isDiff(i,j) == NON){
			}
		}
	}
	//RGB��YCbCr�����܂�
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#if 0//###############################################################
	//���ꂼ��̉�f�̑������ƂɐF��t����
	//���ׂč����ƂȂ�����f�ɂ���
	for(int j=0;j<imageHeight;j++){
		for(int i=0;i<imageWidth;i++){
			if(isDiff(i,j) == HAIR){//���F��������
				SetPixColor(i,j,255,0,0);
			}
			else if(isDiff(i,j) == SKIN){//���F��������
				SetPixColor(i,j,0,255,0);
			}
			else if(isDiff(i,j) == NEITHER){//���F�ł����F�ł��Ȃ�������
				SetPixColor(i,j,0,0,255);
			}
			else{//����ȊO
				SetPixColor(i,j,0,0,0);
			}
		}
	}
#endif//###############################################################
}

//
//��{�I�ȉ摜�����֐��@�����܂�
//==============================================================================================

//--------------------------------------------------
// Name     : void image::PointDistance()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.20
// function : 2�_�Ԃ̋������o��A(x1,y1),B(x2,y2)
// Argument : int x1
//            int y1
//            int x2
//            int y2
// return   : int ���̓��a�̃��[�g���Ƃ����l
// �܂����ɂ����������ƂȂ����
//--------------------------------------------------
int image::PointDistance(const int a,const int b,const int c,const int d)
{
	int dist;

	dist = sqrt((double)(c - a) * (c - a) + (d - b) * ( d - b));

	return dist;
}


//--------------------------------------------------
// Name     : void image::Labeling()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2006.3.17
// function : �Q�l�摜�̃��x�����O����
// Argument : �Ȃ�
// return   : �Ȃ�
//--------------------------------------------------
void image::Labeling()
{

	label Label;

	//���x���ݒ�
	Label.setArea(imageWidth,imageHeight);

	//������
	Label.init();








	//������f�̂݃��x�����O���邽�ߍ��W���Z�b�g//�T���̈�
	//for(int j=0; j<imageHeight; j++){
	//	for(int i=0; i<imageWidth; i++){
	for(int j=area.s_h; j<area.e_h; j++){
		for(int i=area.s_w; i<area.e_w; i++){
			if(isDiff(i,j) == BACK) Label.setPoint(i,j);	
		}
	}

	//���x�����O(�ċA����)
	Label.Labeling();

	//#########################################################
	//�������烉�x���\������

	//�e���x���ԍ����Q�b�g
	std::vector<int> min_label   = Label.getminLabel();//���������x��
	std::vector<int> large_label = Label.getlargeLabel();//�l���̈�
	std::vector<int> obj_label   = Label.getobjLabel();//���̌��̈�
	int obj_num = Label.getLargestObjLabel(obj_label);//���̌��̈�̂Ȃ��ōł��傫���̈�
	int h_num = Label.getLargestObjLabel(large_label);//�l���̈�̂Ȃ��ōł��傫���̈�
	int object_sh = d_area.s_w;//���̌��m�̈�
	int object_sw = d_area.s_h;
	int object_eh = d_area.e_w;
	int object_ew = d_area.e_h;

	//���ꕨ�́i�Ƃ݂Ȃ��j�Ń��x����t����
	if(obj_label.size() != 0){
		new_label = new int[obj_label.size()];
		OTable = new OBJDATA[obj_label.size()];
	}
	new_objnum = 0; //�V�������̂̃��x����
	int tmp_num =0;
	bool check = false;

	for(std::vector<int>::iterator it = obj_label.begin();it != obj_label.end();++it){
		if(obj_label.size() == 0)	break;
		for(int i=0;i<new_objnum;i++){
			//�d�S�̋������߂�
			int nx,ny,ox,oy;
			//Label.getGravityCenter(new_label[i],nx,ny);
			nx = OTable[i].xcenter;	ny = OTable[i].ycenter;
			Label.getGravityCenter(*it,ox,oy);
			int dist = sqrt((double)((nx-ox)*(nx-ox)+(ny-oy)*(ny-oy)));
			//if(dist<50){  //���ꕨ�̂Ƃ���
			if(dist<35){  //���ꕨ�̂Ƃ���
				//if(OTable[i].size >= (Label.getLabelSize(*it))){
					new_label[tmp_num] = i;  /*new_label[i];*/
					//�ʐρA�d�S�A�O�ڒ����`���X�V����
					int tmp_size = OTable[i].size + Label.getLabelSize(*it);
					nx = ((nx*OTable[i].size)+(ox*Label.getLabelSize(*it))) / (tmp_size); //�d�S
					ny = ((ny*OTable[i].size)+(oy*Label.getLabelSize(*it))) / (tmp_size);
					OTable[i].size += tmp_size;  //�ʐ�
					int x1,x2,y1,y2;
					Label.getLabelArea(*it,x1,x2,y1,y2);
					if(OTable[i].x1 > x1)	OTable[i].x1 = x1;
					if(OTable[i].x2 < x2)	OTable[i].x2 = x2;
					if(OTable[i].y1 > y1)	OTable[i].y1 = y1;
					if(OTable[i].y2 < y2)	OTable[i].y2 = y2;
					check = true; //���x�������i�[������
				//}
			}
		}

		if(check == false){
			new_label[tmp_num] = new_objnum;  /**it;*/  //�V�������x���ƌÂ����x���̑Ή�
			OTable[new_objnum].size = Label.getLabelSize(*it); //���x�����X�V�i�ʐρj
			int x1,x2,y1,y2;
			Label.getLabelArea(*it,x1,x2,y1,y2);
			OTable[new_objnum].x1 = x1;	OTable[new_objnum].x2 = x2;  //���x�����X�V�i�O�ڒ����`�j
			OTable[new_objnum].y1 = y1;	OTable[new_objnum].y2 = y2;
			Label.getGravityCenter(*it,x1,y1);
			OTable[new_objnum].xcenter = x1;             //���x�����X�V�i�d�S�j
			OTable[new_objnum].ycenter = y1;
			new_objnum++;      //���x�������J�E���g
		}
		check = false;
		tmp_num++;

	}//for(std::vector<int>::iterator it = obj_label.begin();it != obj_label.end();++it){


	//�̈�̖ʐς��傫��������̂Ə�����������̂͌��Ȃ��悤�ɂ���
	//�{���́u�l�v�̗̈�𔻒f�ł���΂�����
	for(int j = area.s_h; j < area.e_h; j++){
		for(int i = area.s_w; i < area.e_w; i++){

			int sel_num = Label.getPoint(i,j);
			bool min_flag = false;
			bool l_flag   = false;
			bool o_flag   = false;


			/*
			//============================================
			//�ʐς̏��������x��
			for(int m = 0; m < (int)min_label.size() ;m++){
			if(min_label.size() == 0) break;
			if(sel_num == min_label[m]){
			min_flag = true;
			//SetPixColor(i,j,255,0,0);//�ԐF
			}
			}
			//============================================
			*/

			//============================================
			//���Ƃ��Ē�`�����傫�����炢�̃��x��
			for(int n = 0; n < (int)obj_label.size() ;n++){
				if(obj_label.size() == 0) break;
				if(sel_num == obj_label[n]){

					int x,y;
					//Label.getGravityCenter(obj_label[n], x,y);
					x = OTable[new_label[n]].xcenter;
					y = OTable[new_label[n]].ycenter;
					bool obj_flag1 = false;
					bool obj_flag2 = false;
				//	if(x > 100 && x < 330 )  obj_flag1 = true;//kawa
					if(x > 110 && x < 260 )  obj_flag1 = true;
					if(y > 10 && y < 230 )  obj_flag2 = true;
					if(obj_flag1 == true && obj_flag2 == true){

						//�ȉ��ύX///////////////////////////////////
						//obj_num = obj_label[n];

						//���̃��x������f�ɃZ�b�g
						//isDiff(i,j) = OBJECT;

						isDiff(i,j) = OBJECT*10 + new_label[n];//kawa
						//�ύX�����܂�////////////////////////////////
					}
				}
			}
			//============================================

			/*
			//============================================
			//�ʐς̑��������x��
			for(int n = 0; n < (int)large_label.size() ;n++){
			if(large_label.size() == 0) break;
			if(sel_num == large_label[n]){
			l_flag = true;
			//SetPixColor(i,j,0,255,255);//�F	
			}
			}
			//============================================
			*/

			//�傫���ʐς̃��x���ɂ��Đl�̗v�f�𒲂ׂ�
			if(sel_num == h_num  && h_num != 0){
				int Yi, Cb, Cr;
				Yi = (int)( 0.299  * R(i,j) + 0.587  * G(i,j) + 0.114  * B(i,j));
				Cb = (int)(-0.1687 * R(i,j) - 0.3312 * G(i,j) + 0.5000 * B(i,j));
				Cr = (int)( 0.5000 * R(i,j) - 0.4183 * G(i,j) - 0.0816 * B(i,j));

				//���F��臒l�Ɉȓ��Ȃ甧�F�̃��x����t����
				if( ((Yi > 20) && (Yi < 240))
						&& ((Cb > -20) && (Cb < 20))
						&& ((Cr > 10) && (Cr < 30)) ){
						isDiff(i,j) = SKIN;
						dB(i,j) = dG(i,j) = dR(i,j) = 0;
						dG(i,j) = 255;
						//SetPixColor(i,j,0,255,0);
					}
					//���F��臒l�ȓ��Ȃ甯�F�̃��x����t����
				else if(Yi < 50){
					isDiff(i,j) = KURO;
					dB(i,j) = dG(i,j) = dR(i,j) = 0;
					dG(i,j) = 255;
				}
				else{
					isDiff(i,j) = HITO;
					dB(i,j) = dG(i,j) = dR(i,j) = 0;
					dG(i,j) = 255;
					//SetPixColor(i,j,255,255,0);
				}
			}

			//============================================

		}
	}

	//###########################################################################
#if 1//�l�Ԃ�������  

	int human_num = Label.getLargestObjLabel(large_label);
	int h_x1,h_y1,h_x2,h_y2,h_xc,h_yc;
	int g_count = 0;//���x���̖ʐ�(�d�S�v�Z�p)
	int xx = 0;//x���W�̍��v(�d�S�v�Z�p)
	int yy = 0;//y���W�̍��v(�d�S�v�Z�p)
	int face_x,face_y;
	static bool FaceFlag = false;//�t���O[�炪����������]

	Label.getLabelArea(human_num, h_x1,h_x2,h_y1,h_y2);
	Label.getGravityCenter(human_num, h_xc,h_yc);

	if(h_x1 > 0 && h_x2 >0 && h_y1 > 0 && h_y2 > 0 &&
		h_x1 < 320 && h_x2 < 320 && h_y1 < 240 && h_y2 < 240){
			//���̗̈��g�ň͂݁A�d�S��ʂ�\������`��
			for (int x = h_x1; x <= h_x2; x++) {
				SetPixColor(x,h_y1,255,255,0);
				SetPixColor(x,h_y2,255,255,0);
				//SetPixColor(x,h_yc,255,0,255);//TODO:remove comment out �d�S
			}
			for (int y = h_y1; y <= h_y2; y++) {
				SetPixColor(h_x1,y,255,255,0);
				SetPixColor(h_x2,y,255,255,0);
				//SetPixColor(h_xc,y,255,0,255);//TODO:remove comment out �d�S
			}

			//�Ƃ肠�����A�摜�ۑ�
			if(hsave==false && r_flag==true){
				hsave=true;
				char tmp[10];
				d_human.set(imageWidth,imageHeight);
				memcpy(&d_human.B(0,0),pixels,imageWidth * imageHeight * 3 );
				memcpy(&d_human2.B(0,0),diff_pix,imageWidth*imageHeight*3);

				static long latest_frame = 0;
				static long folder_frame = 0;
				std::ostringstream dir,frame_name;
				std::string dir_name;
				FileOperator path;

				//�A�����ĂȂ���΁A�V�����t�H���_�����
				if((save_frame-latest_frame)>10){
					//Human-DB���ʉ摜�ۑ��t�H���_
					/*if(mode==ONLINE){
						dir  << "\\\\Proliant2/okamoto/Human-DB/Original/" << save_frame << "/";
						frame_name << save_frame;
						dir_name = "\\\\Proliant2/okamoto/Human-DB/Original/" + frame_name.str();
					}else if(mode==OFFLINE){*/
						dir  << "../DB/Human-DB/Original/" << save_frame << "/";
						frame_name << save_frame;
						dir_name = "../DB/Human-DB/Original/" + frame_name.str();
					//}
					path.MakeDir(dir_name.c_str());
					//Human-DB�����摜�ۑ��t�H���_
					/*if(mode==ONLINE){
						dir  << "\\\\Proliant2/okamoto/Human-DB/Process/" << save_frame << "/";
						dir_name = "\\\\Proliant2/okamoto/Human-DB/Process/" + frame_name.str();
					}else if(mode==OFFLINE){*/
						dir  << "../DB/Human-DB/Process/" << save_frame << "/";
						dir_name = "../DB/Human-DB/Process/" + frame_name.str();
					//}
					path.MakeDir(dir_name.c_str());
					folder_frame = save_frame;
					
					//log�t�@�C��
					FileOperator file;
					FILE *fl;
					/*if(mode==ONLINE)
						fopen_s(&fl,"\\\\Proliant2/okamoto/Human-DB/human.log","a");
					else if(mode==OFFLINE)*/
						fopen_s(&fl,"../DB/Human-DB/human.log","a");
					fprintf(fl,"%d\n",save_frame);
					fclose(fl);
				}

				/*if(mode==ONLINE){
					sprintf_s(savename,"\\\\Proliant2/okamoto/Human-DB/Original/");
					sprintf_s(savename2,"\\\\Proliant2/okamoto/Human-DB/Process/");
				}else if(mode==OFFLINE){*/
					sprintf_s(savename,"../DB/Human-DB/Original/");
					sprintf_s(savename2,"../DB/Human-DB/Process/");
				//}
				sprintf_s(tmp,"%d",folder_frame);
				strcat_s(savename,tmp);
				strcat_s(savename2,tmp);
				strcat_s(savename,"/");
				strcat_s(savename2,"/");

				sprintf_s(tmp,"%08lu",save_frame);
				strcat_s(savename,tmp);
				strcat_s(savename2,tmp);
				strcat_s(savename,".png");
				strcat_s(savename2,".png");

				latest_frame = save_frame;
				std::ofstream ofs( "../DB/Human-DB/human2.log", std::ios::out | std::ios::trunc );
				ofs << latest_frame << std::endl;
			
				AfxBeginThread(ThreadSave,&d_human,THREAD_PRIORITY_NORMAL);
				AfxBeginThread(ThreadSave2,&d_human2,THREAD_PRIORITY_NORMAL);
			}

			//�w�i�����̍ő�̈�̏㔼��������T������
			for(int j = h_yc * 1.15; j< h_y2; j++){
				for(int i = h_x1; i< h_x2; i++){
					if(isDiff(i,j) == SKIN){//�����̏d�S����̔��F�͊烉�x��
						isDiff(i,j) = FACE;
						//SetPixColor(i,j,0,0,255);
						xx += i;
						yy += j;
						++g_count;
					}
					if(isDiff(i,j) == KURO){//�����̏d�S����̍��F�͔����x��
						isDiff(i,j) = HAIR;
						//SetPixColor(i,j,255,0,0);
					}
				}
			}

			if(g_count > 20){//�烉�x����������������h��
				face_x = xx / g_count; //��̏d�S�� x���W(���v/�ʐ�)
				face_y = yy / g_count; //��̏d�S�� y���W(���v/�ʐ�)

				//��̏d�S���W�ɐF��t����
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						//SetPixColor(face_x+m,face_y+n,255,255,255);
					}
				}

				FaceFlag = true;

				//�����̏㉺�ɕ������������
				//for(int x = h_x1; x <= h_x2; x++){
				//SetPixColor(x,h_yc,255,255,255);
				//}
			}//��T��Loop -END-

		}


		if(FaceFlag == true){
			//�������̗̈�Ŕ��F�͎�Ƃ���
			for(int j = h_y1; j< h_yc; j++){
				for(int i = h_x1; i< h_x2; i++){
					if(isDiff(i,j) == SKIN){
						isDiff(i,j) = HAND;
					}
				}
			}

			int max_dist = INT_MIN;//�ő勗��
			int hand_x1,hand_y1;//�w��̍��W(x1,y1)
			int hand_x2,hand_y2;//�w��̍��W(x2,y2)
			static bool h_flag1 = false;
			static bool h_flag2 = false;

			//�d�S��荶���͈̔͂𑖍����Ĕ��F���x���̂P�ԉ������W�����߂�
			for(int j = h_y1; j < face_y; j++){
				for(int i = h_x1; i < face_x; i++){
					if(isDiff(i,j) == HAND){
						if(max_dist < PointDistance(i,j,face_x,face_y)){
							max_dist = PointDistance(i,j,face_x,face_y);
							hand_x1 = i;
							hand_y1 = j;
							h_flag1 = true;
						}
						//�胉�x���ɐF������
						SetPixColor(i,j,0,255,255);
					}
				}
			}

			//�����P��ő�l�����߂邽�ߏ�����
			max_dist = INT_MIN;

			//�d�S���E���̑����Ŕ��F�̈�ԉ������W�����߂�
			for(int j = h_y1; j < face_y; j++){
				for(int i = face_x; i < h_x2; i++){
					if(isDiff(i,j) == HAND){
						if(max_dist < PointDistance(i,j,face_x,face_y)){
							max_dist = PointDistance(i,j,face_x,face_y);
							hand_x2 = i;
							hand_y2 = j;
							h_flag2 = true;
						}
						//�胉�x���ɐF�t����
						SetPixColor(i,j,0,255,255);
					}
				}
			}

			//####################################################���g���Ă��Ȃ�
			//�w��̓_�����܂��Ă���ΐF��t����
			//�d�S���E���̎�
			if(h_flag1 == true){
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						//SetPixColor(hand_x1+m,hand_y1+n,0,255,255);
					}
				}
				h_flag1 = false;
			}
			//�����Е��̏d�S��荶���̎�
			if(h_flag2 == true){
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						//SetPixColor(hand_x2+m ,hand_y2+n,0,255,255);
					}
				}
				h_flag2 = false;

			}
			//####################################################

			FaceFlag = false;
		}

#endif
		//###################################################################################



		//###################################################################################
#if 1 //���̂̊O�ڒ����`�Əd�S��\��������  
		for(int n = 0; n < (int)obj_label.size() ;n++){
			if(obj_label.size() == 0) break;
			int x1,x2, y1, y2;
			int xc, yc;
			bool o_flag1,o_flag2;

			x1 = x2 = y1 = y2 = xc = yc = 0;
			o_flag1 = false;
			o_flag2 = false;

			//���̗̈�̊O�ڒ����`�̍��W�𓾂�
			Label.getLabelArea(obj_label[n], x1,x2,y1,y2);
			//���̗̈�̏d�S���W�𓾂�
			Label.getGravityCenter(obj_label[n], xc,yc);

			//���̗̈�炵���d�S�����m�͈͓��ł����
			if(xc > object_sh && xc < object_eh )  o_flag1 = true;
			if(yc > object_sw && yc < object_ew )  o_flag2 = true;

			if(o_flag1 == true && o_flag2 == true){
				//������Ƃ����T���͈͂��L����(������)
				if(y1 > 10 && x1 > 10){
					x1 = x1 - 10;
					x2 = x2 + 10;
					y1 = y1 - 10;
					y2 = y2 + 10; 

					//���̂̊O�ڒ����`�̍��W���o���Ă���
					//setArea(y1,x1,y2,x2);
					setArea(x1,y1,x2,y2);
					set_center(xc,yc);

					int area_num = Label.getLabelSize(obj_label[n]);
					set_square(area_num);
				}


			}

		}
		//}//�S�Ă̍��W�𓾂�Ƃ��̃��[�v

#endif//#########################################################

}


//--------------------------------------------------
// Name     : void image::Contract()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : ���k����
// Argument : image ���͉摜
//            int   �����̃^�C�v
// return   : �Ȃ�
//--------------------------------------------------
void image::Contract(image &mask,const int type)
{

	int *diff = new int[imageWidth*imageHeight];

	//�u���x���Ȃ��v�ŏ�����
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}

	//�����ŗ^����ꂽ���x���ɑ΂��Ď��k����
	for(int j=2; j<imageHeight-2; j++){
		for(int i=2; i<imageWidth-2; i++){			
			if(mask.isDiff(i,j) == type){				
				int count = 0;
				for(int m = -1;m < 3 ;m++){
					for(int n = -1;n < 3 ;n++){
						if(mask.isDiff(i+n,j+m) == NON) count++;
					}
				}
				if(count > 0) diff[imageWidth*j+i] = NON;
				else {diff[imageWidth*j+i] = type; }
			}
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			mask.isDiff(i,j) = diff[imageWidth*j+i];
		}
	}

	delete [] diff;
}

//--------------------------------------------------
// Name     : void image::Contract()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : ���k����
// Argument : int   �����̃^�C�v
// return   : �Ȃ�
//--------------------------------------------------
void image::Contract(const int type)//type=1
{
	int *diff = new int[imageWidth*imageHeight];

	//�u���x���Ȃ��v�ŏ�����
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}

	//�����ŗ^����ꂽ���x���ɑ΂��Ď��k����
	for(int z=0;z<1;z++){//���k��
	for(int j=1; j<imageHeight-2; j++){
		for(int i=1; i<imageWidth-2; i++){			
			if(isDiff(i,j) == type){				
				int count = 0;
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						if(isDiff(i+n,j+m) == NON) count++;
					}
				}
				if(count > 0) diff[imageWidth*j+i] = NON;
				else {diff[imageWidth*j+i] = type; }
			}
		}
	}
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			isDiff(i,j) = diff[imageWidth*j+i];
		}
	}

}
	delete [] diff;
}


//--------------------------------------------------
// Name     : void image::Expand()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : �c������(�Ǐ��I�ȏ���)
// Argument : 
// return   : �Ȃ�
//--------------------------------------------------
void image::Expand(image &mask,const int type)
{
	int *diff = new int[imageWidth*imageHeight];

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}

	for(int j = 2; j < imageHeight - 2; j++){
		for(int i = 2; i < imageWidth - 2; i++){
			if(mask.isDiff(i,j) == type) {diff[imageWidth*j+i] = type;}			
			if(mask.isDiff(i,j) == NON){
				int count = 0;
				for(int m = -1;m < 3 ;m++){
					for(int n = -1;n < 3 ;n++){
						if(mask.isDiff(i+n,j+m) == type)count++;
					}
				}
				if(count > 0) diff[imageWidth*j+i] = type;
			}
		}
	}

	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			mask.isDiff(i,j) = diff[imageWidth*j+i];
		}
	}

	delete[] diff;
}

//--------------------------------------------------
// Name     : void image::Expand()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : �c������(�Ǐ��I�ȏ���)
// Argument : �Ȃ�
// return   : �Ȃ�
//--------------------------------------------------
void image::Expand(const int type)//type=1
{
	int *diff = new int[imageWidth*imageHeight];

	//������
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}
	//����NON��SHADOW�̎����BACK������ꍇBACK������B
//	for(int j = 2; j < imageHeight - 4; j++){//kawa
//		for(int i = 2; i < imageWidth - 4; i++){
	for(int z=0;z<3;z++){
	for(int j = 1; j < imageHeight - 2; j++){
		for(int i = 3; i < imageWidth - 2; i++){
			if(isDiff(i,j) == NON || isDiff(i,j) == SHADOW){
				int count = 0;
//				for(int m = -1;m < 3 ;m++){
//					for(int n = -1;n < 3 ;n++){
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						if(isDiff(i+n,j+m) == type)count++;
					}
				}
				if(count > 0) {diff[imageWidth*j+i] = type;}
			}
			else {diff[imageWidth*j+i] = type;}
		}
	}
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
//			if(isDiff(i,j) == SHADOW && diff[imageWidth*j+1] == NON)	break;
		    isDiff(i,j) = diff[imageWidth*j+i];
			//if(diff[imageWidth*j+i]!=NON	dB(i,j) = dG(i,j) = dR(i,j) =255;
		}
	}
}
	delete[] diff;
}

//---------------------------------------------------------------------
// Name     : void image::DispResult()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.16
// function : �������ƂɐF�𕪂���
// Argument : �Ȃ�
// return   : �Ȃ�
//---------------------------------------------------------------------
void image::DispResult()
{
	//��f�ɕt����ꂽ���x�����ƂɐF��t����(�\���p)
	for(int j=0;j<imageHeight;j++){
		for(int i=0;i<imageWidth;i++){
			if(isDiff(i,j) == HAIR){//���F��������
				SetPixColor(i,j,255,0,0);//���b�h
			}
			else if(isDiff(i,j) == FACE){//���F�ł��烉�x��
				SetPixColor(i,j,0,255,0);//�O���[��
			}
			//else if(isDiff(i,j) == SKIN){//���F��������
			//	SetPixColor(i,j,0,255,0);
			//}
			//else if(isDiff(i,j) == NEITHER){//���F�ł����F�ł��Ȃ�������
			//	SetPixColor(i,j,0,0,255);
			//}
			//else if(isDiff(i,j) == BACK){//������������
			//	SetPixColor(i,j,255,0,255);
			//}
			//else{//����ȊO
			//	SetPixColor(i,j,0,0,0);
			//}
		}
	}
}
//--------------------------------------------------
// Name     : void image::Labeling()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : �Q�l�摜�̃��x�����O����
// Argument : �Ȃ�
// return   : �Ȃ�
//------------------------------------------------------------------
//���܂������ɂ��ă��x�����O���čő�̈�����߂邻���Ă��̍ő�ʐς̏d�S���
//  �ォ����{�����F�̍ő����Ƃ��������߂Ă��炻�̏d�S�����ԋ����̗��ꂽ���F���x����
//  �w��Ƃ����E��_�����߂Ă���(���[�U�̓��͂̍��W�Ɏg�p���邽��)
//-------------------------------------------------------------------
void image::BinaryImgLabeling()
{
	label Label;
	int face_x,face_y;//��̏d�S��x,y���W
	static bool FaceFlag = false;//�t���O[�炪����������]
	int x1, x2, y1, y2;//�O�ڒ����`�̍��W
	int xc, yc;//�d�S�̍��W(xc,yc)
	int g_count = 0;//���x���̖ʐ�(�d�S�v�Z�p)
	int xx = 0;//x���W�̍��v(�d�S�v�Z�p)
	int yy = 0;//y���W�̍��v(�d�S�v�Z�p)

	//���x���t���͈̔͐ݒ�
	Label.setArea(imageWidth,imageHeight);

	//�����Ƃ��ă��x�����t����ꂽ�̈�ɑ΂��ă��x�����O
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			if(isDiff(i,j) >= NEITHER){Label.setPoint(i,j);}
		}
	}

	//����(���C���C����ȊO)�ɑ΂��ă��x�����O
	Label.Labeling();

	int largest = Label.getLargestLabel();//�ő�ʐς̃��x���ԍ�

	//�ő�ʐς̃��x���̊O�ڒ����`(�̍��W)�Əd�S�̍��W�����߂�
	Label.getLabelArea(largest, x1,x2,y1,y2);
	Label.getGravityCenter(largest, xc,yc);


	//�T���͈͂������L���������(���Ƃŏ�������)
	//yc = yc * 0.8;
	x1 = x1 * 0.7;


	//�w�i�����̍ő�̈�̏㔼��������T������
	for(int j = yc; j< y2; j++){
		for(int i = x1; i< x2; i++){
			//if(isDiff(i,j) == HAIR){}
			if(isDiff(i,j) == SKIN){//�����̏d�S����̔��F�͊烉�x��
				isDiff(i,j) = FACE;
				xx += i;
				yy += j;
				++g_count;
			}
		}
	}

	if(g_count > 20){//�烉�x����������������h��
		face_x = xx / g_count; //��̏d�S�� x���W(���v/�ʐ�)
		face_y = yy / g_count; //��̏d�S�� y���W(���v/�ʐ�)
		//��̏d�S���W�ɐF��t����

		for(int m = -1;m < 2 ;m++){
			for(int n = -1;n < 2 ;n++){
				SetPixColor(face_x+m,face_y+n,0,255,255);
			}
		}

		FaceFlag = true;
		//�����̏㉺�ɕ������������
		for(int x = x1; x <= x2; x++){
			SetPixColor(x,yc,255,0,255);
		}
	}


	//�炪�������Ă���Ύ��{������������
	if(FaceFlag == true){

		//�������̗̈�Ŕ��F�͎�Ƃ���
		for(int j = y1; j< yc; j++){
			for(int i = x1; i< x2; i++){
				if(isDiff(i,j) == SKIN){
					isDiff(i,j) = HAND;
				}
			}
		}

		int max_dist = INT_MIN;//�ő勗��
		int hand_x1,hand_y1;//�w��̍��W(x1,y1)
		int hand_x2,hand_y2;//�w��̍��W(x2,y2)
		static bool h_flag1 = false;
		static bool h_flag2 = false;

		//�d�S��荶���͈̔͂𑖍����Ĕ��F���x���̂P�ԉ������W�����߂�
		for(int j = y1; j < face_y; j++){
			for(int i = x1; i < face_x; i++){
				if(isDiff(i,j) == HAND){
					if(max_dist < PointDistance(i,j,face_x,face_y)){
						max_dist = PointDistance(i,j,face_x,face_y);
						hand_x1 = i;
						hand_y1 = j;
						h_flag1 = true;
					}
					//�胉�x���ɐF������
					//SetPixColor(i,j,0,255,255);
				}
			}
		}

		//�����P��ő�l�����߂邽�ߏ�����
		max_dist = INT_MIN;

		//�d�S���E���̑����Ŕ��F�̈�ԉ������W�����߂�
		for(int j = y1; j < face_y; j++){
			for(int i = face_x; i < x2; i++){
				if(isDiff(i,j) == HAND){
					if(max_dist < PointDistance(i,j,face_x,face_y)){
						max_dist = PointDistance(i,j,face_x,face_y);
						hand_x2 = i;
						hand_y2 = j;
						h_flag2 = true;
					}
					//�胉�x���ɐF�t����
					//SetPixColor(i,j,0,255,255);
				}
			}
		}

		//####################################################
		//�w��̓_�����܂��Ă���ΐF��t����
		//��ڂɌ���������
		if(h_flag1 == true){

			for(int m = -1;m < 2 ;m++){
				for(int n = -1;n < 2 ;n++){
					SetPixColor(hand_x1+m,hand_y1+n,0,255,255);
				}
			}
			h_flag1 = false;		
		}
		//��ڂ̎�
		if(h_flag2 == true){

			for(int m = -1;m < 2 ;m++){
				for(int n = -1;n < 2 ;n++){
					SetPixColor(hand_x2 + m ,hand_y2 + n,0,255,255);
				}
			}
			h_flag2 = false;
		}
		//####################################################

		FaceFlag = false;

	}//���{��Loop -END-

}

//������������������������������������������������������������������������������������������
//������������������������������������������������������������������������������������������
//������������������������������������������������������������������������������������������
//��{�I�ȉ摜�����S
//
//��{�I�ȉ摜�����Ȃ̂Ō����ɂ͒��ڂ͊֌W�Ȃ���(^^)
//

//-------------------------------------
//Name:grayscale()
//�Z�W��
//�Ԃ�l�F�Ȃ�
//-------------------------------------
//////////////////////////////////////////////////////////
//YIQ (NTSC - National Television System Committee)
//
// Y =  0.299 * R + 0.587 * G + 0.114 * B
// I =  0.596 * R - 0.274 * G - 0.322 * B
// Q =  0.212 * R - 0.523 * G + 0.311 * B
// 
// R = Y + 0.956 * I + 0.621 * Q
// G = Y - 0.272 * I - 0.647 * Q
// B = Y - 1.105 * I + 1.702 * Q
///////////////////////////////////////////////////////////
void image::grayscale()
{
	for(int j=0;j<imageHeight;j++)
		for(int i=0;i<imageWidth;i++){
			R(i,j) = (unsigned char)(R(i,j)*0.299+G(i,j)*0.587+B(i,j)*0.114);
			G(i,j) = R(i,j);
			B(i,j) = R(i,j);
		}
}


//-----------------------------------
//臒l�ɂ���l��
//-----------------------------------
void image::blackwhite(const unsigned char threshold)
{
	grayscale();
	for(int j=0;j<imageHeight;j++)
		for(int i=0;i<imageWidth;i++){
			if(R(i,j)>threshold)	R(i,j)=G(i,j)=B(i,j)=255;
			else                    R(i,j)=G(i,j)=B(i,j)=0;
		}
}

//------------------------------------------
//sobel�t�B���^��p�����G�b�W���o
//------------------------------------------
void image::EdgeDetection()
{
	double min,max;
	double pixel_value;
	double div_const = 1.0;
	const unsigned char max_brightness = 255;

	min = (double)INT_MAX;
	max = (double)INT_MIN;

	//�Z�W�摜�ɕϊ�
	grayscale();

	for(int j = 1; j < imageHeight-1; j++){
		for(int i = 1;i < imageWidth-1;i++){
			pixel_value = 0.0;
			double fx = (2*R(i,j+1) - 2*R(i,j-1)) +
				(R(i-1,j+1) - R(i-1,j-1)) +
				(R(i+1,j+1) - R(i+1,j-1));
			double fy = (2*R(i+1,j) - 2*R(i-1,j)) +
				(R(i+1,j+1) - R(i-1,j+1)) +
				(R(i+1,j-1) - R(i-1,j-1));
			pixel_value = (double)sqrt(fx*fx+fy*fy);
			//pixel_value = pixel_value / div_const;
			if(pixel_value < min) min = pixel_value;
			if(pixel_value > max) max = pixel_value;			
		}
	}

	for(int j=1;j<imageHeight -1;j++){
		for(int i=1;i<imageWidth -1;i++){
			pixel_value = 0.0;
			double fx = (2*R(i,j+1) - 2*R(i,j-1)) +
				(R(i-1,j+1) - R(i-1,j-1)) +
				(R(i+1,j+1) - R(i+1,j-1));
			double fy = (2*R(i+1,j) - 2*R(i-1,j)) + 
				(R(i+1,j+1) - R(i-1,j+1)) +
				(R(i+1,j-1) - R(i-1,j-1));
			pixel_value = (double)sqrt(fx*fx+fy*fy);
			//pixel_value = pixel_value / div_const;
			pixel_value = (double)(max_brightness / 
				( max - min ) * (pixel_value - min));
			Y(i,j) = (unsigned char)pixel_value;
			//if(Y(i,j) < 50 )  Y(i,j) = 0;
			//if(Y(i,j) >= 50 ) Y(i,j) = 255;
		}
	}

	for(int j=0 ; j<imageHeight;j++){
		for(int i=0 ; i<imageWidth;i++){
			if(i == 0 || j == imageHeight || j == 0 || i == imageWidth) {
				R(i,j) = 0;
				G(i,j) = 0;
				B(i,j) = 0;
			}
			else{
				R(i,j) = Y(i,j);
				G(i,j) = Y(i,j);
				B(i,j) = Y(i,j);
			}
		}
	}

}

