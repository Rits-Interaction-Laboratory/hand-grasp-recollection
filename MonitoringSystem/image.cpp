///////////////////////////////////////////////////////////
// FileName : image.cpp
// Context  : image(RGB)
// Author　 : Katayama Noriaki (CV-lab.)
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

// 差分のしきい値調整のための変数
int diff_threshold = 15;
//
int diff_cncc=95,diff_hs=5;


//スレッド(人検知画像保存)
static UINT AFX_CDECL ThreadSave(LPVOID pParam)
{
	p_human=(bmp*)pParam;
	p_human->save(savename);
	hsave=false;
	return 0;
}

//スレッド(人検知画像保存)
static UINT AFX_CDECL ThreadSave2(LPVOID pParam)
{
	p_human=(bmp*)pParam;
	p_human->save(savename2);
	hsave=false;
	return 0;
}

//--------------------------------------------------
// Name     : watch(int frame)
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2006.6.27
// Function : シーンの監視(研究のメイン部分)
// Argument : int型のフレーム数
// return   : bool型の変数
//--------------------------------------------------
bool image::watch(unsigned long frame)
{        
	clock_t t1=clock();

	static Scene state; //シーン状態
	static bmp pre;     //推定した背景画像
	bmp now;            //現在フレームの画像

	static bmp pre_bg;         //影除去のための更新前のフレーム

	int re=0;
	r_flag = false;

	static unsigned long f=-1;	//フラグ
	++f;						//フラグインクリメント

	save_frame = ff = f;

	//各種フラグ初期化
	//static bool save_flag = false;
	static int save_flag = -1;
	state.set_mode(mode);

	//物体検知の範囲設定
	//setArea(30,40,320,120);
	setArea(10,10,330,230);

	//現在の入力画像の輝度値のみ使用(輝度値をM推定)
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
	//背景をM推定を用いて推定(毎フレーム更新)
	M_estimator(pre,now,pre_bg);
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();
#if 0 //画像の保存
	char fname1[40];
	sprintf(fname1,"pre/%07d.png",frame);
	pre.save(fname1);
	//char fname2[40];
	//sprintf(fname2,"now/%07d.bmp",frame);
	//now.save(fname2);
#endif

	//推定背景画像と入力画像の差分
	BackDiff(pre,now,diff_threshold);
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//影除去(CNCC)
	Contract(BACK);
	//膨張
	Expand(BACK);
	Contract(BACK);
	//収縮
	//Contract(BACK);//kawa

	if(f>30 && r_flag ==true)	ElimWithCNCC(pre,now);
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

#if 0 //画像の保存
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
	//ラベリング
	Labeling();
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//ラベルに色を付ける
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

	//物体検知
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

	//とりあえず、ここで解放
	if(new_objnum>0){
		if(new_label)	delete new_label;
		if(OTable)		delete OTable;
		end.free_obj_memory();
	}
	if(f<30)	return false;
	t2=clock();	
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//物体を検知したらそのラベルの背景に取り込む
	if(save_flag >= 0){
		re++;
		
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
//		for(int j =d_area.s_h; j < d_area.e_h; j++){
//			for(int i = d_area.s_w; i < d_area.e_w; i++){
				//物体として検知したラベルの画素を背景にする
				if(end.isDiff(i,j) == OBJECT*10+save_flag){
					Y(i,j) = pre.Y_d(i,j); // 間違い？ modified by shimada 2012.12.14 //kawa
					pre.Y_d(i,j) = now.Y_d(i,j);
					pre.r_d(i,j) = now.r_d(i,j);
					pre.b_d(i,j) = now.b_d(i,j);
					pre.g_d(i,j) = now.g_d(i,j);
					end.isDiff(i,j) = OBJECT;
					
				}else if(end.isDiff(i,j) == SHADOW){ //影
					Y(i,j) = pre_bg.Y_d(i,j);
					pre.Y_d(i,j) = pre_bg.Y_d(i,j);
					pre.R(i,j)   = pre_bg.R(i,j);
					pre.G(i,j)   = pre_bg.G(i,j);
					pre.B(i,j)   = pre_bg.B(i,j);
					//end.isDiff(i,j) = OBJECT;
					
				}else if(end.isDiff(i,j)==HITO ||
					end.isDiff(i,j)==SKIN ||
					end.isDiff(i,j)==KURO){   //人領域
						Y(i,j) = pre_bg.Y_d(i,j);
						pre.Y_d(i,j) = pre_bg.Y_d(i,j);
						pre.R(i,j)   = pre_bg.R(i,j);
						pre.G(i,j)   = pre_bg.G(i,j);
						pre.B(i,j)   = pre_bg.B(i,j);
                   
				}
				   
			}
		}
		//物体検知待ち状態にする
		save_flag = -1;
	}
#endif//############################################################################
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	//検知する範囲を線で囲む	
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.23
// function : 背景差分
// Argument : image 背景画像
//            image 入力画像
// return   : なし
//Background subtraction 
//------------------------------------------------------------------------
void image::BackDiff(image &back,image &input,int b_threshold /* = 20*/)
{
	static double diff;//輝度値の差
	//const int b_threshold = 20;//差分の閾値

	//==============================================================
#if 1
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff = sqrt((back.Y_d(i,j) - input.Y_d(i,j)) * (back.Y_d(i,j) - input.Y_d(i,j)));
			//差分を調べる指定範囲について計算する
			if((j >= area.s_h  && j <= area.e_h) && (i >= area.s_w && i<= area.e_w)){
				if(diff < b_threshold){//差が閾値以下なら背景
					isDiff(i,j) = NON;
					dB(i,j) = dG(i,j) = dR(i,j) = 0;
				}
				else{//背景と差分がでたら
					isDiff(i,j) = BACK;
					dB(i,j) = dG(i,j) = dR(i,j) = 255;
				}
			}
			else {//差分を調べない部分はそのまま背景画像にする
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
				if( cncc == CNCC_ERROR ){ //エラー（基本的に起きないと思うけど）
					isDiff(w,h) = NON;  //これでいいのか？？
					dB(w,h) = dG(w,h) = dR(w,h) = 0;
					dB(w,h) = 255;
				}else if(cncc == CNCC_ERROR2){  //分散が平均値より小さく、明るくなった(Not 影)
					isDiff(w,h) = BACK;
					dB(w,h) = dG(w,h) = dR(w,h) = 0;
					//dG(w,h) = 255;
				}else if(cncc == 10){           //分散が平均値より小さく、hとsの相関高(影)
					isDiff(w,h) = SHADOW;
					dB(w,h) = dG(w,h) = dR(w,h) = 128;
					/*dB(w,h) = dG(w,h) = dR(w,h) = 0;
					dR(w,h) = 255;*/
				}else if(cncc == 11){           //分散が平均値より小さく、hとsの相関低(Not 影)
					isDiff(w,h) = BACK;
					dB(w,h) = dG(w,h) = dR(w,h) = 255;
					//dG(w,h) = dB(w,h) = 255;
				}else if(cncc==12){     //h=0 計算不可
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

			//追加(h,sの相関を求めるため)
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

/*	//分散が平均値より小さすぎる場合
	if((var_a < mean_a/10.0) || (var_b < mean_b/10.0)){
		if(mean_bL > mean_fL){ //暗くなった場合(明度が小さくなる)
			//hとsの相関を求める
			if(cip_ab == 0 || sqrt(cip_aa * cip_bb)==0) //h,s=0(計算不可)
				return 12;
			double chs = (cip_ab) / sqrt(cip_aa * cip_bb);
			if(chs < 0.1 && chs > -0.1)   //hとsの相関が小さい（Not 影）
				return 11;
			return 10;      //hとsの相関が高い（影）
		}
		return CNCC_ERROR2; //明るくなった場合(明度が大きくなる)
	}*/

	if((var_a < mean_a/10.0) && (var_b < mean_b/10.0)){
		if(mean_bL > mean_fL){ //暗くなった場合(明度が小さくなる)
			//hとsの相関を求める
			//if(cip_ab == 0 || sqrt(cip_aa * cip_bb)==0) //h,s=0(計算不可)
				//return 12;
			//double chs = (cip_ab) / sqrt(cip_aa * cip_bb);

			bfx=sqrt(bfx);
			bfy=sqrt(bfy);
			double chs=(bfx+bfy)/(windowArea*2);

			if(chs < ((double)diff_hs/100) && chs > 0.0)   
				return 10;  //前景と背景のHとSの差が低い（影）
			    
			    return 11;  //hとsの差が高い（Not 影）
		}
		return CNCC_ERROR2; //明るくなった場合(明度が大きくなる)
	}
	else if((var_a < mean_a/20.0) || (var_b < mean_b/20.0)){//どちらかの分散が十分大きい場合物体検知
		//if(mean_bL > mean_fL){ //暗くなった場合(明度が小さくなる)
		//return 10;//影
	    // }
		
		return 11;}//物体



	cncc = ( ip_ab - mean_fL*mean_bL*(double)windowArea ) / sqrt( var_a * var_b );

	//cnccが-1～1でない
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.9.19
// function : 画像のデータをコピー
// Argument : image  コピーされる画像
//            image　コピーする画像データ
// return   : bool変数
//------------------------------------------------------------------------
void image::CopyImg(image &img,const char type)
{
	switch(type){
		case 'Y'://輝度値で出力する場合
			for(int j=0; j<imageHeight; j++){
				for(int i=0; i<imageWidth; i++){
					R(i,j) = img.Y(i,j); 	
					G(i,j) = img.Y(i,j); 
					B(i,j) = img.Y(i,j); 
				}
			}
			break;
		case 'R'://ＲＧＢで出力する場合
			for(int j=0; j<imageHeight; j++){
				for(int i=0; i<imageWidth; i++){
					R(i,j) = img.R(i,j); 	
					G(i,j) = img.G(i,j); 
					B(i,j) = img.B(i,j); 
				}
			}
			break;
		default://それ以外
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.9.19
// function : 画像をある値で初期化する
// Argument : image  値を入れる画像
//            int　  初期化の値
// return   : bool変数
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
// ORをとる
// 重なっていることろは色を変える
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.9
// function : 画像の色を得る
// Argument : int x座標
//            int y座標
// return   : 色番号(enumで定義されている)
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.8.22
// function : 画像に線を引く
// Argument : image型の画像データ，始点高さ，始点幅，終点高さ，終点幅
// return   : なし
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
 *	function:	Areaの内側でかつ一番外側の画素を青(R,G,B)=(0,0,255)で塗る
 *	□:画像の各画素
 *	○:Areaの範囲画素
 *	●:Area内の一番外側の画素(塗る場所)
 *
 *	□□□□□□      □□□□□□      □□□□□□
 *	□□□□□□      □□○○○□      □□●●●□
 *	□□□□□□  →  □□○○○□  →  □□●○●□
 *	□□□□□□      □□○○○□      □□●●●□
 *	□□□□□□      □□□□□□      □□□□□□
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
	const double a = 0.8;                //累計誤差の記憶率α 現在0.8で固定
	double eta;                          //環境の変化に対する適応率
	const double eta_min = 0.001;        //適応率の最小値
	long int M = Height*Width;			//画素数M
	double Eps_x;                        //ある画素における入力画像と背景画像の輝度値の差
	double psi_x;                        //ρの微分ψ(εx)
	static bool flag = false;            //初期化フラグ
	static long int counter;             //フレーム数カウント
	double *dist = new double[M];       //輝度値の差の絶対値の二乗
	static int tem_counter=0;			//一時フレーム数
	static int *something=new int[Width*Height];	//物体or人の領域
	static int *n_something=new int[Width*Height];	//背景
	static bmp before;

	//static int *check=new int[Width*Height];	//背景

	static bool toriaezu=false;//test

	++counter;//フレーム数
	++tem_counter; //一時フレーム
	clock_t t1=clock();
	//初期化(１回のみ行う)背景画像の累計誤差Et=0,画素の輝度値全て128にする
	//この初期値を適切な値にする必要アリ
	if(flag == false){
		for(int j=0; j<Height; j++){
			for(int i=0; i<Width; i++){
				pre.Y_d(i,j) = 128.0;
				pre.Y(i,j)   = 128;
				pre.E(i,j)   = 0.0;
				something[j*Width+i]=0;
				n_something[j*Width+i]=0;
				//とりあえず
				pre.r_d(i,j) = now.r_d(i,j);
				pre.g_d(i,j) = now.g_d(i,j);
				pre.b_d(i,j) = now.b_d(i,j);
				pre.R(i,j) = pre.G(i,j) = pre.B(i,j) = 0;
				pre.mask(i,j)=255;
			}
		}
		flag = true;//フラグをオン
	}
	clock_t t2=clock();		
	double rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

	if(flag == true){//初期画像があれば
		//輝度値の差の二乗を求める
		for(int j=0; j<Height; j++){
			for(int i=0; i<Width; i++){
				//差の絶対値の二乗を格納
				dist[j*Width+i] = fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))) 
					* fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j)));

			}
		}

		//fprintf(fo,"差分の画素数 = %d \n",diff_count);
	t2=clock();
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;

	t1=clock();

#if 1//←１にしたらηの自動調整をする
	clock_t _t1=clock();

	double sigma_x,sigma_y;              //標準偏差σx,σy
	int  ave_pre,ave_now;                //ある画像の輝度値の平均値
	long int sum_Cov = 0;                //xとyそれぞれの平均と差の合計
	long int sum_pre = 0,sum_now = 0;    //ある画像の輝度値合計
	long int sum_x = 0,sum_y = 0;        //自分の平均との差の合計値


	//#################################
	//例外値を求める
	double med;//メジアン(中央値)
	double sigma_ber;//標準偏差の推定値
	double Exception_val;//例外値
	double di;//輝度値の差の絶対値の二乗の平方根
	long int Excep_num = 0;//例外値ではない画素数

	//qsort O(nlogn)
	//M:76800
	//O(76800*log(76800)=371950)
	std::sort(dist,dist+M-1);//クイックソートを行う
	//画素数が奇数のときはそのまま中央値
	if(M%2==1){
		med = dist[(M-1)/2];
	}
	//画素数が偶数のときは中央の２値の平均
	if(M%2==0){
		med = (dist[M/2]+dist[M/2-1])/2;
	}

	//標準偏差の推定値を求める計算
	sigma_ber = 1.4826 * (double)(1 + (5 / (M-1))) * sqrt(med);
	//2.5*σを例外値とする
	Exception_val = sigma_ber * 2.5;

	//fprintf(fo,"例外値 = %f \n",Exception_val);

	//Heght:240,Width:320
	//76800 loop
	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			di =sqrt( fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))) 
				* fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))));
			//例外値ではないならば
			if(Exception_val > di){
				sum_pre += (long)pre.Y_d(i,j);//背景画像の輝度値の合計
				sum_now += (long)now.Y_d(i,j);//入力画像の輝度値の合計
				++Excep_num;
			}
		}
	}
	clock_t _t2=clock();		
	double _rtime=(double)(_t2-_t1)/CLOCKS_PER_SEC;
	_t1=clock();


	//fprintf(fo,"sum_pre->%d sum_now->%d\n",sum_pre,sum_now);

	//例外値でないときの平均値
	ave_pre = sum_pre / Excep_num; //背景画像の平均値
	ave_now = sum_now / Excep_num; //入力画像の平均値
	//fprintf(fo,"pre->%d now->%d\n",ave_pre,ave_now);	



	int diff_x,diff_y;//平均との差
	double Cov_xy;//共分散
	double r_rob;//ロバスト相関係数r_rob

	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			di = sqrt(fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))) 
				* fabs((double)(now.Y_d(i,j) - pre.Y_d(i,j))));
			if(Exception_val > di){
				diff_x = (int)(pre.Y_d(i,j) - ave_pre);
				diff_y = (int)(now.Y_d(i,j) - ave_now);

				//背景画像のある値の平均との差の二乗和
				sum_x   += diff_x * diff_x;
				//入力画像のある値の平均との差の二乗和
				sum_y   += diff_y * diff_y;
				//背景と入力画像の輝度値と平均の差を掛けたものの和
				sum_Cov += diff_x * diff_y;
			}
		}
	}
	_t2=clock();		
	_rtime=(double)(_t2-_t1)/CLOCKS_PER_SEC;
	_t1=clock();

	//背景と入力画像それぞれの標準偏差と共分散を求め
	//ロバストテンプレートマッチングを行い相関係数を求める
	sigma_x = sqrt((double)(sum_x / Excep_num));  //背景画像標準偏差  σx
	sigma_y = sqrt((double)(sum_y / Excep_num));  //入力画像の標準偏差σy
	Cov_xy  = (double)(sum_Cov / Excep_num);       //xとyの共分散
	r_rob   = Cov_xy / (sigma_x * sigma_y);  //相関係数r_robを求める

	//fprintf(fo,"σx=%f ",sigma_x);
	//fprintf(fo,"σy=%f\n",sigma_y);
	//fprintf(fo,"共分散%f\n",Cov_xy);
	//fprintf(fo,"%f\n",r_rob);


	//相関値rを -1 ≦ r ≦ 1 に抑える
	if(sigma_x == 0.0 || sigma_y == 0.0) r_rob = 0.0;


	//相関が低すぎるときの調整
	if(r_rob<=0.0){
		r_rob=eta_min;
	}

	//相関が低いときに急激な背景更新の準備
	//（閾値によりかなり高速で更新したければ使う）
	if(r_rob<=0.30){
		tem_counter=0;
	}

	//適応率η = f(r_rob) の自動調節
	//現在はこの式を最適としてる
	//0.995あたりで程よく小さくなるから
	if(r_rob<0.95){
		eta=1.0;
	}
	else{
		r_flag = true;
		eta=-400.0*(r_rob-0.95)*(r_rob-0.95)+1.0;
	}

	//適応が全く進まないのを避けるために微小な値を乗せる
	eta  = eta + eta_min;	

	//適応率を1.0以内に抑える
	if(eta>1.0){
		eta=1.0;
	}
	_t2=clock();		
	_rtime=(double)(_t2-_t1)/CLOCKS_PER_SEC;

#endif//ηの自動調節 END
	t2=clock();		
	rtime=(double)(t2-t1)/CLOCKS_PER_SEC;
	t1=clock();

		//ηをはじめの数フレームは高くする
		if(tem_counter < 30){
			eta = 0.9999;
			r_flag = false;
		}

		double tem;		//Eps_xの正の値
	
		
		//##############################################################
		//ロバスト統計に基づき背景の輝度値を推定
		//輝度値θ(theta)を推定し背景画像の値に代入

		for(int j=0; j<Height; j++){
			for(int i=0; i<Width; i++){

				//投票により物体領域を保持したければ投票数を指定(30)
				if(something[j*Width+i]<30){
			
					//輝度値の差ε(x) = 背景画像θ - 入力画像xt
					Eps_x = (pre.Y_d(i,j) - now.Y_d(i,j));

					//輝度差の正の値を出す
					tem=fabs(Eps_x);
			
					double re_val=0.0;	//更新を行う輝度差の閾値
					
					

					//現在こちらを使う(式に数値的な根拠なし)
					re_val=45.0/(pow(r_rob,26.0));

					//0.998のときに２０ぐらい（ほぼ更新しない）になるように設定
					/*re_val=(double)1000.0*(1.0-r_rob)*10.0;
					if(re_val>255){
						re_val=255;
					}*/
					
					int back=false;
					//どっちの関数を使うかの判断
					//・最初のフレーム
					//・全体相関が低い＆明白な輝度差がないとき
					//・完全な背景変化
					/////////////////////////////////////////////////////////
					//重みを決める相関の閾値の設定
					///////////////////////////////////////////////////////
					//if((tem_counter<30) || (r_rob<0.998&&re_val>tem)){//default
					if((tem_counter<30) || (r_rob<0.990&&re_val>tem)){
					//if((tem_counter<30) || (r_rob<0.9920&&re_val>tem)){
					
						psi_x = tanh(Eps_x / 50.0);
						back=true;
					}
					
					//・全体の相関が高い（通常状態）
					//・全体の相関が低くても輝度差が大きいとき
					else{
						///////////////////////////////////////////
						//重みを決める輝度差の閾値の設定
						///////////////////////////////////////////
						//Eps_x=Eps_x/(double)5.0;//default
						Eps_x=Eps_x/(double)1.0;	//閾値きつめ
						
						psi_x = 2*Eps_x/((1.0+Eps_x*Eps_x)*(1.0+Eps_x*Eps_x));
						psi_x = psi_x*2;
						if(psi_x>(double)1.0){
							psi_x=(double)1.0;
						}
					}


					//影を考慮した背景推定のため
					pre_bg.E(i,j)   = pre.E(i,j);
					pre_bg.Y_d(i,j) = pre.Y_d(i,j);
					pre_bg.R(i,j) = pre.R(i,j);
					pre_bg.G(i,j) = pre.G(i,j);
					pre_bg.B(i,j) = pre.B(i,j);
			
					if(pre.mask(i,j)=255||back==true){
					//記憶率をかける
					pre.E(i,j) = psi_x + (a * pre.E(i,j));
					//(R(i,j) * 0.299 + G(i,j) * 0.587 + B(i,j) * 0.114);
					//背景画像の輝度値更新
					//Etを最小とする背景の輝度値θtを最急降下法で求める
					//[数式]θt = θt-1 -η(dEt / dθ)
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
					//unsigned char 型に値を戻す
					//pre.Y(i,j) = pre.Y_d(i,j);

					//pre.R(i,j) = pre.G(i,j) = pre.B(i,j) = (unsigned char)pre.Y_d(i,j);

					//影除去用
					//背景画像のRGB
//					pre.R(i,j) = pre.R(i,j) - (eta * pre.E(i,j));
//					pre.G(i,j) = pre.G(i,j) - (eta * pre.E(i,j));
//					pre.B(i,j) = pre.B(i,j) - (eta * pre.E(i,j));

					
					//値の初期化
					psi_x = Eps_x = 0.0;

				}
				
				//投票
				//背景ではないと推定
				if(tem>20.0&&r_rob>0.9950){
					something[j*Width+i]+=1;
					n_something[j*Width+i]=0;
				}
				//背景部分と推定
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

		//			/////この段階のＲＧＢ値を記憶//////
		//			//背景画素＆フラグが立っていないとき
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.7
// function : クイックソート
// Argument : double型配列，int型Index番号 int型Index番号
// return   : なし
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.5.29
// function : フレーム間差分による動体検出
// Argument : なし
// return   : なし
//--------------------------------------------------
void image::FrameDiff(int frame)
{
	static double diff;//輝度値の差
	const int b_threshold = 20;//差分の閾値

	static image back;

	setArea(5,40,310,230);

	//現在の入力画像の輝度値のみ使用(輝度値をM推定)
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
			//差分を調べる指定範囲について計算する
			if((j >= area.s_h  && j <= area.e_h) && (i >= area.s_w && i<= area.e_w)){
				if(diff < b_threshold){//差が閾値以下なら背景
					isDiff(i,j) = NON;
					dB(i,j) = dG(i,j) = dR(i,j) = 0;
				}
				else{//背景と差分がでたら
					isDiff(i,j) = BACK;
					dB(i,j) = dG(i,j) = dR(i,j) = 255;
				}
			}
			else {//差分を調べない部分はそのまま背景画像にする
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

	
#if 0 //画像の保存
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.10.19
// function : 肌色検出
// Argument : image 差分マスク画像
// return   : なし
//---------------------------------------------------------------------
void image::skinExtraction(image &mask)
{
	int Yi, Cb, Cr;


	//:::::::::::::::::::::::::
	// RGB⇒YCbCr
	//:::::::::::::::::::::::::
	for(int j = 0; j < imageHeight; j++){
		for(int i = 0;i < imageWidth; i++){
			//差分となった画素だったら
			if(mask.isDiff(i,j) == BACK){
				Yi = (int)( 0.299  * mask.R(i,j) + 0.587  * mask.G(i,j) + 0.114  * mask.B(i,j)); // Y
				Cb = (int)(-0.1687 * mask.R(i,j) - 0.3312 * mask.G(i,j) + 0.5000 * mask.B(i,j)); // Cb
				Cr = (int)( 0.5000 * mask.R(i,j) - 0.4183 * mask.G(i,j) - 0.0816 * mask.B(i,j)); // Cr

				//肌色の閾値に以内なら
				if( ((Yi > 20) && (Yi < 240))
					&& ((Cb > -30) && (Cb < 20))
					&& ((Cr > 10) && (Cr < 30)) ){
						mask.B(i,j) = 0;
						mask.G(i,j) = 0;
						mask.R(i,j) = 255;
					}
				else {//肌色じゃなかったら0にしておく
					mask.R(i,j) = 0;
					mask.G(i,j) = 0;
					mask.B(i,j) = 0;
				}
			}
			//差分じゃない画素は
			if(mask.isDiff(i,j) == NON){
				mask.R(i,j) = mask.G(i,j) = mask.B(i,j) = 0;
			}
		}
	}
	//RGB⇒YCbCrここまで
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
}

//---------------------------------------------------------------------
// Name     : void image::skinExtraction()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.17
// function : 肌色検出
// Argument : なし
// return   : なし
//---------------------------------------------------------------------
//●差分から人の手を検出するために色ごとに属性を付ける
//  
//■今のところどの色空間を使用するか決めていない(2005.10.19)
//  ①RGB⇒YCbCr
//  ②RGB⇒
//
//■髪の毛の色は？
////①if(R(i,j) < 48 && G(i,j) < 55 && B(i,j) <50){}
////②Y(輝度値50以下)
//---------------------------------------------------------------------
void image::skinExtraction()
{
	int Yi, Cb, Cr;

	//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	//:RGB⇒YCbCr
	for(int j = 0; j < imageHeight; j++){
		for(int i = 0;i < imageWidth; i++){
			//差分となった画素についてラベルを付ける
			if(isDiff(i,j) == BACK){
				Yi = (int)( 0.299  * R(i,j) + 0.587  * G(i,j) + 0.114  * B(i,j)); // Y
				Cb = (int)(-0.1687 * R(i,j) - 0.3312 * G(i,j) + 0.5000 * B(i,j)); // Cb
				Cr = (int)( 0.5000 * R(i,j) - 0.4183 * G(i,j) - 0.0816 * B(i,j)); // Cr

				//肌色の閾値に以内なら肌色のラベルを付ける
				if( ((Yi > 20) && (Yi < 240))
					&& ((Cb > -30) && (Cb < 20))
					&& ((Cr > 10) && (Cr < 30)) ){
						isDiff(i,j) = SKIN;
						//SetPixColor(i,j,0,255,0);
					}
				else if(Yi < 50){//髪色の閾値以内なら髪色のラベルを付ける
					isDiff(i,j) = HAIR;
				}
				else {//肌色でも髪色でもなかったら「それ以外」のラベル
					isDiff(i,j) = NEITHER;
				}
			}
			//差分じゃない画素は「属性なし」でそのままにしておく
			if(isDiff(i,j) == NON){
			}
		}
	}
	//RGB⇒YCbCrここまで
	//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#if 0//###############################################################
	//それぞれの画素の属性ごとに色を付ける
	//すべて差分となった画素について
	for(int j=0;j<imageHeight;j++){
		for(int i=0;i<imageWidth;i++){
			if(isDiff(i,j) == HAIR){//髪色だったら
				SetPixColor(i,j,255,0,0);
			}
			else if(isDiff(i,j) == SKIN){//肌色だったら
				SetPixColor(i,j,0,255,0);
			}
			else if(isDiff(i,j) == NEITHER){//髪色でも肌色でもなかったら
				SetPixColor(i,j,0,0,255);
			}
			else{//それ以外
				SetPixColor(i,j,0,0,0);
			}
		}
	}
#endif//###############################################################
}

//
//基本的な画像処理関数　ここまで
//==============================================================================================

//--------------------------------------------------
// Name     : void image::PointDistance()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.20
// function : 2点間の距離を出すA(x1,y1),B(x2,y2)
// Argument : int x1
//            int y1
//            int x2
//            int y2
// return   : int 差の二乗和のルートをとった値
// まあ特にたいしたことないんで
//--------------------------------------------------
int image::PointDistance(const int a,const int b,const int c,const int d)
{
	int dist;

	dist = sqrt((double)(c - a) * (c - a) + (d - b) * ( d - b));

	return dist;
}


//--------------------------------------------------
// Name     : void image::Labeling()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2006.3.17
// function : ２値画像のラベリング処理
// Argument : なし
// return   : なし
//--------------------------------------------------
void image::Labeling()
{

	label Label;

	//ラベル設定
	Label.setArea(imageWidth,imageHeight);

	//初期化
	Label.init();








	//差分画素のみラベリングするため座標をセット//探索領域
	//for(int j=0; j<imageHeight; j++){
	//	for(int i=0; i<imageWidth; i++){
	for(int j=area.s_h; j<area.e_h; j++){
		for(int i=area.s_w; i<area.e_w; i++){
			if(isDiff(i,j) == BACK) Label.setPoint(i,j);	
		}
	}

	//ラベリング(再帰処理)
	Label.Labeling();

	//#########################################################
	//ここからラベル表示部分

	//各ラベル番号をゲット
	std::vector<int> min_label   = Label.getminLabel();//小さいラベル
	std::vector<int> large_label = Label.getlargeLabel();//人物領域
	std::vector<int> obj_label   = Label.getobjLabel();//物体候補領域
	int obj_num = Label.getLargestObjLabel(obj_label);//物体候補領域のなかで最も大きい領域
	int h_num = Label.getLargestObjLabel(large_label);//人物領域のなかで最も大きい領域
	int object_sh = d_area.s_w;//物体検知領域
	int object_sw = d_area.s_h;
	int object_eh = d_area.e_w;
	int object_ew = d_area.e_h;

	//同一物体（とみなす）でラベルを付ける
	if(obj_label.size() != 0){
		new_label = new int[obj_label.size()];
		OTable = new OBJDATA[obj_label.size()];
	}
	new_objnum = 0; //新しい物体のラベル数
	int tmp_num =0;
	bool check = false;

	for(std::vector<int>::iterator it = obj_label.begin();it != obj_label.end();++it){
		if(obj_label.size() == 0)	break;
		for(int i=0;i<new_objnum;i++){
			//重心の距離求める
			int nx,ny,ox,oy;
			//Label.getGravityCenter(new_label[i],nx,ny);
			nx = OTable[i].xcenter;	ny = OTable[i].ycenter;
			Label.getGravityCenter(*it,ox,oy);
			int dist = sqrt((double)((nx-ox)*(nx-ox)+(ny-oy)*(ny-oy)));
			//if(dist<50){  //同一物体とする
			if(dist<35){  //同一物体とする
				//if(OTable[i].size >= (Label.getLabelSize(*it))){
					new_label[tmp_num] = i;  /*new_label[i];*/
					//面積、重心、外接長方形を更新する
					int tmp_size = OTable[i].size + Label.getLabelSize(*it);
					nx = ((nx*OTable[i].size)+(ox*Label.getLabelSize(*it))) / (tmp_size); //重心
					ny = ((ny*OTable[i].size)+(oy*Label.getLabelSize(*it))) / (tmp_size);
					OTable[i].size += tmp_size;  //面積
					int x1,x2,y1,y2;
					Label.getLabelArea(*it,x1,x2,y1,y2);
					if(OTable[i].x1 > x1)	OTable[i].x1 = x1;
					if(OTable[i].x2 < x2)	OTable[i].x2 = x2;
					if(OTable[i].y1 > y1)	OTable[i].y1 = y1;
					if(OTable[i].y2 < y2)	OTable[i].y2 = y2;
					check = true; //ラベル情報を格納したか
				//}
			}
		}

		if(check == false){
			new_label[tmp_num] = new_objnum;  /**it;*/  //新しいラベルと古いラベルの対応
			OTable[new_objnum].size = Label.getLabelSize(*it); //ラベル情報更新（面積）
			int x1,x2,y1,y2;
			Label.getLabelArea(*it,x1,x2,y1,y2);
			OTable[new_objnum].x1 = x1;	OTable[new_objnum].x2 = x2;  //ラベル情報更新（外接長方形）
			OTable[new_objnum].y1 = y1;	OTable[new_objnum].y2 = y2;
			Label.getGravityCenter(*it,x1,y1);
			OTable[new_objnum].xcenter = x1;             //ラベル情報更新（重心）
			OTable[new_objnum].ycenter = y1;
			new_objnum++;      //ラベル数をカウント
		}
		check = false;
		tmp_num++;

	}//for(std::vector<int>::iterator it = obj_label.begin();it != obj_label.end();++it){


	//領域の面積が大きすぎるものと小さすぎるものは見ないようにする
	//本当は「人」の領域を判断できればいいが
	for(int j = area.s_h; j < area.e_h; j++){
		for(int i = area.s_w; i < area.e_w; i++){

			int sel_num = Label.getPoint(i,j);
			bool min_flag = false;
			bool l_flag   = false;
			bool o_flag   = false;


			/*
			//============================================
			//面積の小さいラベル
			for(int m = 0; m < (int)min_label.size() ;m++){
			if(min_label.size() == 0) break;
			if(sel_num == min_label[m]){
			min_flag = true;
			//SetPixColor(i,j,255,0,0);//赤色
			}
			}
			//============================================
			*/

			//============================================
			//物として定義した大きさくらいのラベル
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

						//以下変更///////////////////////////////////
						//obj_num = obj_label[n];

						//物体ラベルを画素にセット
						//isDiff(i,j) = OBJECT;

						isDiff(i,j) = OBJECT*10 + new_label[n];//kawa
						//変更ここまで////////////////////////////////
					}
				}
			}
			//============================================

			/*
			//============================================
			//面積の多きいラベル
			for(int n = 0; n < (int)large_label.size() ;n++){
			if(large_label.size() == 0) break;
			if(sel_num == large_label[n]){
			l_flag = true;
			//SetPixColor(i,j,0,255,255);//青色	
			}
			}
			//============================================
			*/

			//大きい面積のラベルについて人の要素を調べる
			if(sel_num == h_num  && h_num != 0){
				int Yi, Cb, Cr;
				Yi = (int)( 0.299  * R(i,j) + 0.587  * G(i,j) + 0.114  * B(i,j));
				Cb = (int)(-0.1687 * R(i,j) - 0.3312 * G(i,j) + 0.5000 * B(i,j));
				Cr = (int)( 0.5000 * R(i,j) - 0.4183 * G(i,j) - 0.0816 * B(i,j));

				//肌色の閾値に以内なら肌色のラベルを付ける
				if( ((Yi > 20) && (Yi < 240))
						&& ((Cb > -20) && (Cb < 20))
						&& ((Cr > 10) && (Cr < 30)) ){
						isDiff(i,j) = SKIN;
						dB(i,j) = dG(i,j) = dR(i,j) = 0;
						dG(i,j) = 255;
						//SetPixColor(i,j,0,255,0);
					}
					//髪色の閾値以内なら髪色のラベルを付ける
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
#if 1//人間を見つける  

	int human_num = Label.getLargestObjLabel(large_label);
	int h_x1,h_y1,h_x2,h_y2,h_xc,h_yc;
	int g_count = 0;//ラベルの面積(重心計算用)
	int xx = 0;//x座標の合計(重心計算用)
	int yy = 0;//y座標の合計(重心計算用)
	int face_x,face_y;
	static bool FaceFlag = false;//フラグ[顔が見つかったか]

	Label.getLabelArea(human_num, h_x1,h_x2,h_y1,h_y2);
	Label.getGravityCenter(human_num, h_xc,h_yc);

	if(h_x1 > 0 && h_x2 >0 && h_y1 > 0 && h_y2 > 0 &&
		h_x1 < 320 && h_x2 < 320 && h_y1 < 240 && h_y2 < 240){
			//物体領域を枠で囲み、重心を通る十字線を描く
			for (int x = h_x1; x <= h_x2; x++) {
				SetPixColor(x,h_y1,255,255,0);
				SetPixColor(x,h_y2,255,255,0);
				//SetPixColor(x,h_yc,255,0,255);//TODO:remove comment out 重心
			}
			for (int y = h_y1; y <= h_y2; y++) {
				SetPixColor(h_x1,y,255,255,0);
				SetPixColor(h_x2,y,255,255,0);
				//SetPixColor(h_xc,y,255,0,255);//TODO:remove comment out 重心
			}

			//とりあえず、画像保存
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

				//連続してなければ、新しいフォルダを作る
				if((save_frame-latest_frame)>10){
					//Human-DB結果画像保存フォルダ
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
					//Human-DB差分画像保存フォルダ
					/*if(mode==ONLINE){
						dir  << "\\\\Proliant2/okamoto/Human-DB/Process/" << save_frame << "/";
						dir_name = "\\\\Proliant2/okamoto/Human-DB/Process/" + frame_name.str();
					}else if(mode==OFFLINE){*/
						dir  << "../DB/Human-DB/Process/" << save_frame << "/";
						dir_name = "../DB/Human-DB/Process/" + frame_name.str();
					//}
					path.MakeDir(dir_name.c_str());
					folder_frame = save_frame;
					
					//logファイル
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

			//背景差分の最大領域の上半分から顔を探索する
			for(int j = h_yc * 1.15; j< h_y2; j++){
				for(int i = h_x1; i< h_x2; i++){
					if(isDiff(i,j) == SKIN){//差分の重心より上の肌色は顔ラベル
						isDiff(i,j) = FACE;
						//SetPixColor(i,j,0,0,255);
						xx += i;
						yy += j;
						++g_count;
					}
					if(isDiff(i,j) == KURO){//差分の重心より上の黒色は髪ラベル
						isDiff(i,j) = HAIR;
						//SetPixColor(i,j,255,0,0);
					}
				}
			}

			if(g_count > 20){//顔ラベルが無かった時を防ぐ
				face_x = xx / g_count; //顔の重心の x座標(合計/面積)
				face_y = yy / g_count; //顔の重心の y座標(合計/面積)

				//顔の重心座標に色を付ける
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						//SetPixColor(face_x+m,face_y+n,255,255,255);
					}
				}

				FaceFlag = true;

				//差分の上下に分ける線を引く
				//for(int x = h_x1; x <= h_x2; x++){
				//SetPixColor(x,h_yc,255,255,255);
				//}
			}//顔探すLoop -END-

		}


		if(FaceFlag == true){
			//下半分の領域で肌色は手とする
			for(int j = h_y1; j< h_yc; j++){
				for(int i = h_x1; i< h_x2; i++){
					if(isDiff(i,j) == SKIN){
						isDiff(i,j) = HAND;
					}
				}
			}

			int max_dist = INT_MIN;//最大距離
			int hand_x1,hand_y1;//指先の座標(x1,y1)
			int hand_x2,hand_y2;//指先の座標(x2,y2)
			static bool h_flag1 = false;
			static bool h_flag2 = false;

			//重心より左下の範囲を走査して肌色ラベルの１番遠い座標を求める
			for(int j = h_y1; j < face_y; j++){
				for(int i = h_x1; i < face_x; i++){
					if(isDiff(i,j) == HAND){
						if(max_dist < PointDistance(i,j,face_x,face_y)){
							max_dist = PointDistance(i,j,face_x,face_y);
							hand_x1 = i;
							hand_y1 = j;
							h_flag1 = true;
						}
						//手ラベルに色をつける
						SetPixColor(i,j,0,255,255);
					}
				}
			}

			//もう１回最大値を求めるため初期化
			max_dist = INT_MIN;

			//重心より右下の走査で肌色の一番遠い座標を求める
			for(int j = h_y1; j < face_y; j++){
				for(int i = face_x; i < h_x2; i++){
					if(isDiff(i,j) == HAND){
						if(max_dist < PointDistance(i,j,face_x,face_y)){
							max_dist = PointDistance(i,j,face_x,face_y);
							hand_x2 = i;
							hand_y2 = j;
							h_flag2 = true;
						}
						//手ラベルに色付ける
						SetPixColor(i,j,0,255,255);
					}
				}
			}

			//####################################################＊使っていない
			//指先の点が求まっていれば色を付ける
			//重心より右側の手
			if(h_flag1 == true){
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						//SetPixColor(hand_x1+m,hand_y1+n,0,255,255);
					}
				}
				h_flag1 = false;
			}
			//もう片方の重心より左側の手
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
#if 1 //物体の外接長方形と重心を表示させる  
		for(int n = 0; n < (int)obj_label.size() ;n++){
			if(obj_label.size() == 0) break;
			int x1,x2, y1, y2;
			int xc, yc;
			bool o_flag1,o_flag2;

			x1 = x2 = y1 = y2 = xc = yc = 0;
			o_flag1 = false;
			o_flag2 = false;

			//物体領域の外接長方形の座標を得る
			Label.getLabelArea(obj_label[n], x1,x2,y1,y2);
			//物体領域の重心座標を得る
			Label.getGravityCenter(obj_label[n], xc,yc);

			//物体領域らしき重心が検知範囲内であれば
			if(xc > object_sh && xc < object_eh )  o_flag1 = true;
			if(yc > object_sw && yc < object_ew )  o_flag2 = true;

			if(o_flag1 == true && o_flag2 == true){
				//ちょっとだけ探索範囲を広げる(微調整)
				if(y1 > 10 && x1 > 10){
					x1 = x1 - 10;
					x2 = x2 + 10;
					y1 = y1 - 10;
					y2 = y2 + 10; 

					//物体の外接長方形の座標を覚えておく
					//setArea(y1,x1,y2,x2);
					setArea(x1,y1,x2,y2);
					set_center(xc,yc);

					int area_num = Label.getLabelSize(obj_label[n]);
					set_square(area_num);
				}


			}

		}
		//}//全ての座標を得るときのループ

#endif//#########################################################

}


//--------------------------------------------------
// Name     : void image::Contract()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : 収縮処理
// Argument : image 入力画像
//            int   差分のタイプ
// return   : なし
//--------------------------------------------------
void image::Contract(image &mask,const int type)
{

	int *diff = new int[imageWidth*imageHeight];

	//「ラベルなし」で初期化
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}

	//引数で与えられたラベルに対して収縮処理
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : 収縮処理
// Argument : int   差分のタイプ
// return   : なし
//--------------------------------------------------
void image::Contract(const int type)//type=1
{
	int *diff = new int[imageWidth*imageHeight];

	//「ラベルなし」で初期化
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}

	//引数で与えられたラベルに対して収縮処理
	for(int z=0;z<1;z++){//収縮回数
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : 膨張処理(局所的な処理)
// Argument : 
// return   : なし
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : 膨張処理(局所的な処理)
// Argument : なし
// return   : なし
//--------------------------------------------------
void image::Expand(const int type)//type=1
{
	int *diff = new int[imageWidth*imageHeight];

	//初期化
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			diff[imageWidth*j+i] = NON;
		}
	}
	//あるNONやSHADOWの周りにBACKがある場合BACKさせる。
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.16
// function : 属性ごとに色を分ける
// Argument : なし
// return   : なし
//---------------------------------------------------------------------
void image::DispResult()
{
	//画素に付けられたラベルごとに色を付ける(表示用)
	for(int j=0;j<imageHeight;j++){
		for(int i=0;i<imageWidth;i++){
			if(isDiff(i,j) == HAIR){//髪色だったら
				SetPixColor(i,j,255,0,0);//レッド
			}
			else if(isDiff(i,j) == FACE){//肌色でかつ顔ラベル
				SetPixColor(i,j,0,255,0);//グリーン
			}
			//else if(isDiff(i,j) == SKIN){//肌色だったら
			//	SetPixColor(i,j,0,255,0);
			//}
			//else if(isDiff(i,j) == NEITHER){//髪色でも肌色でもなかったら
			//	SetPixColor(i,j,0,0,255);
			//}
			//else if(isDiff(i,j) == BACK){//差分だったら
			//	SetPixColor(i,j,255,0,255);
			//}
			//else{//それ以外
			//	SetPixColor(i,j,0,0,0);
			//}
		}
	}
}
//--------------------------------------------------
// Name     : void image::Labeling()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : ２値画像のラベリング処理
// Argument : なし
// return   : なし
//------------------------------------------------------------------
//■まず差分についてラベリングして最大領域を求めるそしてその最大面積の重心より
//  上から顔を捜し肌色の最大を顔とする顔を求めてからその重心から一番距離の離れた肌色ラベルを
//  指先とし左右一点ずつ求めておく(ユーザの入力の座標に使用するため)
//-------------------------------------------------------------------
void image::BinaryImgLabeling()
{
	label Label;
	int face_x,face_y;//顔の重心のx,y座標
	static bool FaceFlag = false;//フラグ[顔が見つかったか]
	int x1, x2, y1, y2;//外接長方形の座標
	int xc, yc;//重心の座標(xc,yc)
	int g_count = 0;//ラベルの面積(重心計算用)
	int xx = 0;//x座標の合計(重心計算用)
	int yy = 0;//y座標の合計(重心計算用)

	//ラベル付けの範囲設定
	Label.setArea(imageWidth,imageHeight);

	//差分としてラベルが付けられた領域に対してラベリング
	for(int j=0; j<imageHeight; j++){
		for(int i=0; i<imageWidth; i++){
			if(isDiff(i,j) >= NEITHER){Label.setPoint(i,j);}
		}
	}

	//差分(肌，髪，それ以外)に対してラベリング
	Label.Labeling();

	int largest = Label.getLargestLabel();//最大面積のラベル番号

	//最大面積のラベルの外接長方形(の座標)と重心の座標を求める
	Label.getLabelArea(largest, x1,x2,y1,y2);
	Label.getGravityCenter(largest, xc,yc);


	//探索範囲を少し広げる微調整(あとで消すかも)
	//yc = yc * 0.8;
	x1 = x1 * 0.7;


	//背景差分の最大領域の上半分から顔を探索する
	for(int j = yc; j< y2; j++){
		for(int i = x1; i< x2; i++){
			//if(isDiff(i,j) == HAIR){}
			if(isDiff(i,j) == SKIN){//差分の重心より上の肌色は顔ラベル
				isDiff(i,j) = FACE;
				xx += i;
				yy += j;
				++g_count;
			}
		}
	}

	if(g_count > 20){//顔ラベルが無かった時を防ぐ
		face_x = xx / g_count; //顔の重心の x座標(合計/面積)
		face_y = yy / g_count; //顔の重心の y座標(合計/面積)
		//顔の重心座標に色を付ける

		for(int m = -1;m < 2 ;m++){
			for(int n = -1;n < 2 ;n++){
				SetPixColor(face_x+m,face_y+n,0,255,255);
			}
		}

		FaceFlag = true;
		//差分の上下に分ける線を引く
		for(int x = x1; x <= x2; x++){
			SetPixColor(x,yc,255,0,255);
		}
	}


	//顔が見つかっていれば手を捜す処理をする
	if(FaceFlag == true){

		//下半分の領域で肌色は手とする
		for(int j = y1; j< yc; j++){
			for(int i = x1; i< x2; i++){
				if(isDiff(i,j) == SKIN){
					isDiff(i,j) = HAND;
				}
			}
		}

		int max_dist = INT_MIN;//最大距離
		int hand_x1,hand_y1;//指先の座標(x1,y1)
		int hand_x2,hand_y2;//指先の座標(x2,y2)
		static bool h_flag1 = false;
		static bool h_flag2 = false;

		//重心より左下の範囲を走査して肌色ラベルの１番遠い座標を求める
		for(int j = y1; j < face_y; j++){
			for(int i = x1; i < face_x; i++){
				if(isDiff(i,j) == HAND){
					if(max_dist < PointDistance(i,j,face_x,face_y)){
						max_dist = PointDistance(i,j,face_x,face_y);
						hand_x1 = i;
						hand_y1 = j;
						h_flag1 = true;
					}
					//手ラベルに色をつける
					//SetPixColor(i,j,0,255,255);
				}
			}
		}

		//もう１回最大値を求めるため初期化
		max_dist = INT_MIN;

		//重心より右下の走査で肌色の一番遠い座標を求める
		for(int j = y1; j < face_y; j++){
			for(int i = face_x; i < x2; i++){
				if(isDiff(i,j) == HAND){
					if(max_dist < PointDistance(i,j,face_x,face_y)){
						max_dist = PointDistance(i,j,face_x,face_y);
						hand_x2 = i;
						hand_y2 = j;
						h_flag2 = true;
					}
					//手ラベルに色付ける
					//SetPixColor(i,j,0,255,255);
				}
			}
		}

		//####################################################
		//指先の点が求まっていれば色を付ける
		//一つ目に見つかった手
		if(h_flag1 == true){

			for(int m = -1;m < 2 ;m++){
				for(int n = -1;n < 2 ;n++){
					SetPixColor(hand_x1+m,hand_y1+n,0,255,255);
				}
			}
			h_flag1 = false;		
		}
		//二つ目の手
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

	}//手を捜すLoop -END-

}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//基本的な画像処理郡
//
//基本的な画像処理なので研究には直接は関係ないよ(^^)
//

//-------------------------------------
//Name:grayscale()
//濃淡化
//返り値：なし
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
//閾値による二値化
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
//sobelフィルタを用いたエッジ検出
//------------------------------------------
void image::EdgeDetection()
{
	double min,max;
	double pixel_value;
	double div_const = 1.0;
	const unsigned char max_brightness = 255;

	min = (double)INT_MAX;
	max = (double)INT_MIN;

	//濃淡画像に変換
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

