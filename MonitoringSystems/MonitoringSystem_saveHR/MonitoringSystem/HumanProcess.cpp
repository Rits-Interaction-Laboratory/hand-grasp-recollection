#/*include "stdafx.h"
#include "HumanProcess.h"
//#include "Labeling.h"

#include <vector>
#include <iostream>
using namespace std;

#include <CommonDef.h>
#include <ModeCT.h>
#include <ModeDB.h>
#include <ModeDT.h>
#include <ModeFR.h>
#include <ModePT.h>
#include <OkaoAPI.h>
#include <OkaoCtAPI.h>
#include <OkaoDbAPI.h>
#include <OkaoDef.h>
#include <OkaoDtAPI.h>
#include <OkaoFrAPI.h>
#include <OkaoPtAPI.h>

#define _USE_MATH_DEFINES
#include <math.h>
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define pi M_PI



#define BB(IMG, X,Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)*3]
#define GG(IMG, X,Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)*3+1]
#define RR(IMG, X,Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)*3+2]
#define I(IMG, X, Y) ((uchar*)((IMG)->imageData + (IMG)->widthStep*(Y)))[(X)]

int RR[10]={128, 255,   0,   0,   0, 255, 255, 128,   0, 128};
int GG[10]={128, 255, 255, 128,   0,   0,   0,   0, 255, 128};
int BB[10]={128,   0,   0, 128, 255, 255,   0, 128, 255,   0};

const int ID_DIGIT = 16;
const int MAX_IMAGES_PER_PERSON = 50;
const int MAX_DATADIGIT = 9;//データベースのサイズの最大桁数


static HALBUMDB hDB;
static HDETECTION hDT;
static HPOINTER hPT;
static HFACERECOG hFR;
static HCONTOUR hCT ;
static HDTRESULT hDtResult;
static HPTRESULT hPtResult;
static HFRRESULT hFrResult;
static HCTRESULT hCtResult;


HUMAN_PROCESS::HUMAN_PROCESS(){

}


HUMAN_PROCESS::~HUMAN_PROCESS()
{

}


int HUMAN_PROCESS::HumanRecognition(IplImage *img_original, IplImage *img_resize, IplImage *img_diff){


	// 膨張（dilations回 膨張処理を行う）
	cvDilate( img_diff,img_diff,0,2 );
	// 収縮（erosions回 収縮処理を行う）
	cvErode( img_diff,img_diff,0,3 );

	IplImage* img_label = cvCreateImage(cvSize(img_diff->width, img_diff->height), IPL_DEPTH_8U, 1);
	Labeling(img_diff, img_label);


	HumanPositionSet(img_label, img_diff);
	MakeClothHist(img_resize, img_diff);
	//SaveClothHist();
	DetectFace(img_original, img_diff);
	FaceArea(img_original, img_diff);

	for(int i=0; i<10; i++){
		if(NowHumanData[i].face_renew_flag==1){
			FaceRecognition(i);
		}
	}

	test(img_diff);

	for(int i=0; i<10; i++){
		if(NowHumanData[i].body_renew_flag==1)
			cvRectangle(img_diff, cvPoint(NowHumanData[i].ptBodyLeftTop1.x, NowHumanData[i].ptBodyLeftTop1.y), cvPoint(NowHumanData[i].ptBodyRightBottom1.x, NowHumanData[i].ptBodyRightBottom1.y), CV_RGB(RR[i], GG[i], BB[i]), 1, CV_AA, 0);
		if(NowHumanData[i].face_renew_flag==1)
			cvRectangle(img_diff, cvPoint(NowHumanData[i].ptFaceLeftTop1.x, NowHumanData[i].ptFaceLeftTop1.y), cvPoint(NowHumanData[i].ptFaceRightBottom1.x, NowHumanData[i].ptFaceRightBottom1.y), CV_RGB(RR[i], GG[i], BB[i]), 1, CV_AA, 0);
	}

	for(int i=0; i<10; i++){
		NowHumanData[i].init();
	}

	return 0;
}



int HUMAN_PROCESS::Labeling(IplImage *img_diff, IplImage *img_label)
{  
	short *dst;
	//LabelingBS labeling;//kawa

	IplImage *image = cvCreateImage( cvGetSize( img_diff), 8, 1);
	cvCvtColor( img_diff, image, CV_RGB2GRAY); // RGBをグレースケール化
    cvThreshold(image, image, 128, 255, CV_THRESH_BINARY);
    //ラベリング処理の結果保存用配列
    dst = new short[ image->width * image->height ];

    //ラベリング実行
    //labeling.Exec((uchar *)image->imageData, dst, image->width, image->height, true, 2000);//kawa
    //------------------------------------------------------------------------------
    //int Exec( SrcT *target, DstT *result, int target_width, int target_height,
    //          const bool is_sort_region, const int region_size_min )
    //引数:
    //  SrcT *target 入力バッファ
    //  DstT *result 出力バッファ
    //  int target_width バッファの横サイズ
    //  int target_height バッファの縦サイズ
    //  bool is_sort_region 連続領域を大きさの降順にソートするか
    //  int region_size_min 最小の領域サイズ(これ未満は無視する)
    //------------------------------------------------------------------------------

	for(int j=0; j<image->height; j++){
		for(int i=0; i<image->width; i++){
			I(img_label,i,j) = dst[j*image->width+i];
		}
	}

	//for(int j=0; j<img_label->height; j++){
	//	for(int i=0; i<img_label->width; i++){
	//		RR(img_diff,i,j) = RR[I(img_label,i,j)];
	//		GG(img_diff,i,j) = GG[I(img_label,i,j)];
	//		BB(img_diff,i,j) = BB[I(img_label,i,j)];
	//	}
	//}

    //終了処理
    delete dst;
    cvReleaseImage(&image);

    return 0;
}


int HUMAN_PROCESS::HumanPositionSet(IplImage *img_label, IplImage *img_diff){

	vector<int> array[10];

	POINT LeftTop[10];
	POINT LeftBottom[10];
	POINT RightTop[10];
	POINT RightBottom[10];

	for(int i=0; i<10; i++){
		LeftTop[i].x = img_label->width;
		LeftTop[i].y = img_label->height;
		LeftBottom[i].x = img_label->width;
		LeftBottom[i].y = 0;
		RightTop[i].x = 0;
		RightTop[i].y = img_label->height;
		RightBottom[i].x = 0;
		RightBottom[i].y = 0;
	}

	for(int j=0; j<img_label->height; j++){
		for(int i=0; i<img_label->width; i++){
			if(LeftTop[I(img_label,i,j)].x>i){
				LeftTop[I(img_label,i,j)].x = i;
				LeftBottom[I(img_label,i,j)].x = i;
			}
			if(LeftTop[I(img_label,i,j)].y>j){
				LeftTop[I(img_label,i,j)].y = j;
				RightTop[I(img_label,i,j)].y = j;
			}
			if(RightBottom[I(img_label,i,j)].x<i){
				RightTop[I(img_label,i,j)].x = i;
				RightBottom[I(img_label,i,j)].x = i;
			}
			if(RightBottom[I(img_label,i,j)].y<j){
				LeftBottom[I(img_label,i,j)].y = j;
				RightBottom[I(img_label,i,j)].y = j;
			}
		}
	}


	for(int i=1; i<10; i++){
		double repeated_percent = 0.0;
		double repeated_percent_max = 0.0;
		int match_body_num = -1;
		if(RightBottom[i].x!=0){
			for(int j=0; j<10; j++){
				if(NowHumanData[j].data_set_flag>=1){
					repeated_percent = ComparePosition(LeftTop[i], RightBottom[i], NowHumanData[j].ptBodyLeftTop1, NowHumanData[j].ptBodyRightBottom1);
					if(repeated_percent >= 0.5){
						array[i].push_back( j );
						match_body_num = j;
					}
				}
			}

			if((int)array[i].size()>1){
				for(int j=0; j<10; j++){
					if(NowHumanData[j].data_set_flag==0){
						NowHumanData[j].ptBodyLeftTop1 = LeftTop[i];
						NowHumanData[j].ptBodyLeftBottom1 = LeftBottom[i];
						NowHumanData[j].ptBodyRightTop1 = RightTop[i];
						NowHumanData[j].ptBodyRightBottom1 = RightBottom[i];
						NowHumanData[j].data_set_flag = (int)array[i].size();
						NowHumanData[j].body_renew_flag = 1;

						for(int k=0; k<(int)array[i].size(); k++){
							NowHumanData[array[i][k]].data_set_flag = -1;
							NowHumanData[array[i][k]].master = j;
						}
						break;
					}
				}
			}
			if((int)array[i].size()==1){
				NowHumanData[match_body_num].ptBodyLeftTop1 = LeftTop[i];
				NowHumanData[match_body_num].ptBodyLeftBottom1 = LeftBottom[i];
				NowHumanData[match_body_num].ptBodyRightTop1 = RightTop[i];
				NowHumanData[match_body_num].ptBodyRightBottom1 = RightBottom[i];
				NowHumanData[match_body_num].data_set_flag = 1;
				NowHumanData[match_body_num].body_renew_flag = 1;
			}
			if((int)array[i].size()==0){
				for(int j=0; j<10; j++){
					if(NowHumanData[j].data_set_flag==0){
						NowHumanData[j].ptBodyLeftTop1 = LeftTop[i];
						NowHumanData[j].ptBodyLeftBottom1 = LeftBottom[i];
						NowHumanData[j].ptBodyRightTop1 = RightTop[i];
						NowHumanData[j].ptBodyRightBottom1 = RightBottom[i];
						NowHumanData[j].data_set_flag = 1;
						NowHumanData[j].body_renew_flag = 1;

						break;
					}
				}
			}

		}
	}

	for(int i=0; i<10; i++){
		if(NowHumanData[i].body_renew_flag==1){
			NowHumanData[i].update_interval = 0;
			for(int j=0; j<10; j++){
				if(NowHumanData[j].master==i)
					NowHumanData[j].update_interval = 0;
			}
		}
		else{
			NowHumanData[i].update_interval++;
			if(NowHumanData[i].update_interval>10){
				NowHumanData[i].reset();
			}
		}
		//NowHumanData[i].init();
	}

	return 0;
}




int HUMAN_PROCESS::DetectFace(IplImage *img_original, IplImage *img_diff)
{
	double img_size_ratio = img_original->width / img_diff->width; //原画像とリサイズ画像のサイズ比

	POINT LeftTop[10], LeftBottom[10], RightTop[10], RightBottom[10];

	for(int i=0; i<10; i++){
		LeftTop[i].x = -1;
		LeftTop[i].y = -1;
		LeftBottom[i].x = -1;
		LeftBottom[i].y = -1;
		RightTop[i].x = -1;
		RightTop[i].y = -1;
		RightBottom[i].x = -1;
		RightBottom[i].y = -1;
	}

	int nFaceCount=0;
	int nFaceConf=0;
	int nI;

	OKAO_Initialize( OKAO_ALL );

	//各ハンドルの作成とＤＢファイルの読み込み
	if(hDT==NULL){
		// 顔検出モジュールのハンドル作成
		hDT = OKAO_CreateDetection( DT_MODE_DEFAULT );
		// 顔検出結果格納ハンドル作成
		hDtResult = OKAO_CreateDtResult();
	}

	//顔の検出範囲の指定(DETECT_HALF_PROFILEは+-60度  DETECT_FRONTは+-30度)
	int nPose;
	nPose = DETECT_HALF_PROFILE;
	//nPose = DETECT_FRONT;
	OKAO_SetDtPose(hDT, nPose);
	OKAO_GetDtPose(hDT, &nPose);

	// 顔検出実行
	OKAO_Detection( hDT, (RAWIMAGE*)img_original->imageData, img_original->width, img_original->height, img_original->nChannels, hDtResult );
	OKAO_GetDtFaceCount( hDtResult, &nFaceCount );

	for( nI = 0; nI < nFaceCount; nI++ ) {
		OKAO_GetDtCorner( hDtResult, nI, &LeftTop[nI], &RightTop[nI], 
			&LeftBottom[nI], &RightBottom[nI], &nFaceConf );
	}

	for(int i=0; i<10; i++){
		double repeated_percent = 0.0;
		double repeated_percent_max = 0.0;
		int match_body_num = -1;
		if(LeftTop[i].x!=-1){
			LeftTop[i].x = LeftTop[i].x/img_size_ratio;
			LeftTop[i].y = LeftTop[i].y/img_size_ratio;
			RightBottom[i].x = RightBottom[i].x/img_size_ratio;
			RightBottom[i].y = RightBottom[i].y/img_size_ratio;

			for(int j=0; j<10; j++){
				if(NowHumanData[j].body_renew_flag==1){
					repeated_percent = ComparePosition(LeftTop[i], RightBottom[i], NowHumanData[j].ptBodyLeftTop1, NowHumanData[j].ptBodyRightBottom1);
					if(repeated_percent_max < repeated_percent){
						repeated_percent_max = repeated_percent;
						match_body_num = j;
					}
				}
			}
			if(match_body_num!=-1){
				NowHumanData[match_body_num].ptFaceLeftTop1 = LeftTop[i];
				NowHumanData[match_body_num].ptFaceLeftBottom1 = LeftBottom[i];
				NowHumanData[match_body_num].ptFaceRightTop1 = RightTop[i];
				NowHumanData[match_body_num].ptFaceRightBottom1 = RightBottom[i];
				NowHumanData[match_body_num].face_renew_flag = 1;
			}	
		}
	}

	OKAO_Terminate( OKAO_ALL );

	return nFaceCount;
}



int HUMAN_PROCESS::FaceArea(IplImage *img_original, IplImage *img_diff){


	double img_size_ratio = img_original->width / img_diff->width; //原画像とリサイズ画像のサイズ比

	POINT LeftTop, RightBottom;

	for(int i=0; i<10; i++){
		if(NowHumanData[i].face_renew_flag==1){

			NowHumanData[i].FaceImage = cvCloneImage(img_original);

			LeftTop.x = NowHumanData[i].ptFaceLeftTop1.x * img_size_ratio;
			LeftTop.y = NowHumanData[i].ptFaceLeftTop1.y * img_size_ratio;
			RightBottom.x = NowHumanData[i].ptFaceRightBottom1.x * img_size_ratio;
			RightBottom.y = NowHumanData[i].ptFaceRightBottom1.y * img_size_ratio;

			int diff = (int)(RightBottom.x - LeftTop.x) * 0.5;

			LeftTop.x = LeftTop.x - diff;
			LeftTop.y = LeftTop.y - diff;
			RightBottom.x = RightBottom.x + diff;
			RightBottom.y = RightBottom.y + diff;

			for(int j=0; j<img_original->height; j++){
				for(int k=0; k<img_original->width; k++){
					if(!(LeftTop.x<k && LeftTop.y<j && RightBottom.x>k && RightBottom.y>j)){
						RR(NowHumanData[i].FaceImage,k,j) = 0;
						GG(NowHumanData[i].FaceImage,k,j) = 0;
						BB(NowHumanData[i].FaceImage,k,j) = 0;
					}
				}
			}

			//cvShowImage("test2", NowHumanData[i].FaceImage);
			//cvWaitKey(6);
		}
	}

	return 0;
}





void HUMAN_PROCESS::MakeClothHist(IplImage *img_resize, IplImage *img_diff)
{
	IplImage* outImg = 0;

	IplImage *iplImage_HSV = 0;
	IplImage *Hue = 0;
	IplImage *Saturation = 0;
	IplImage *Value = 0;

	for(int j=0; j<img_diff->height; j++){
		for(int i=0; i<img_diff->width; i++){
			if(RR(img_diff,i,j)==255 && GG(img_diff,i,j)==255 && BB(img_diff,i,j)==255){
				RR(img_diff,i,j) = RR(img_resize,i,j);
				GG(img_diff,i,j) = GG(img_resize,i,j);
				BB(img_diff,i,j) = BB(img_resize,i,j);
			}
		}
	}

	for(int i=0; i<10; i++){
		if(NowHumanData[i].body_renew_flag==1){
			//img_ROI = cvCreateImage(cvSize(img_diff->width,srcImg->height),IPL_DEPTH_8U,1);
			outImg = cvCreateImage(cvSize((NowHumanData[i].ptBodyRightBottom1.x - NowHumanData[i].ptBodyLeftTop1.x + 1),
				(NowHumanData[i].ptBodyRightBottom1.y - NowHumanData[i].ptBodyLeftTop1.y + 1)), 8, 3);
			cvSetImageROI(img_diff,cvRect(NowHumanData[i].ptBodyLeftTop1.x,NowHumanData[i].ptBodyLeftTop1.y,
				outImg->width,outImg->height));
			cvCopyImage(img_diff, outImg);
			cvResetImageROI(img_diff);

			char windowname[16];
			sprintf_s(windowname, "test%02d", i);
			cvShowImage(windowname, outImg);
			cvWaitKey(6);

			iplImage_HSV = cvCreateImage(cvSize(outImg->width,outImg->height),IPL_DEPTH_8U,3);
			Hue = cvCreateImage(cvSize(outImg->width,outImg->height),IPL_DEPTH_8U,1);
			Saturation = cvCreateImage(cvSize(outImg->width,outImg->height),IPL_DEPTH_8U,1);
			Value = cvCreateImage(cvSize(outImg->width,outImg->height),IPL_DEPTH_8U,1);

			//HSV化したものを、image_HSVに格納
			cvCvtColor(outImg, iplImage_HSV, CV_BGR2HSV);
			//image_HSVをばらばらに格納
			cvSplit(outImg,Hue,Saturation,Value,0);

			NowHumanData[i].ManArea=0;

			for(int x=0; x<outImg->width; x++){
				for(int y=0; y<outImg->height; y++){
					if(!(outImg->imageData[(y*outImg->width+x)*3+2]==0)){
						//三次元ヒストグラムに格納//
						int H,S,V;

						H=(int)cvGetReal2D(Hue,y,x);
						S=(int)cvGetReal2D(Saturation,y,x);
						V=(int)cvGetReal2D(Value,y,x);

						//hsv空間へプロット//
						int h,s,v;
						double H_;

						//②円柱型
						H_ = H*2*pi/255;
						h = (S*cos(H_)+255)/25;
						s = (S*sin(H_)+255)/25;
						v = V/13;

						if(h>19)h = 19;
						if(s>19)s = 19;
						if(v>19)v = 19;

						int bin = NowHumanData[i].Hist_3D[h][s][v];
						NowHumanData[i].Hist_3D[h][s][v] = bin+1;

						////面積に１追加
						NowHumanData[i].ManArea++;
					}
				}
			}
		}
	}

	cvReleaseImage( &outImg );
	cvReleaseImage(&iplImage_HSV);
	cvReleaseImage(&Hue);
	cvReleaseImage(&Saturation);
	cvReleaseImage(&Value);
	
}


void HUMAN_PROCESS::SaveClothHist(){

	FILE *fp;

	for(int m=0; m<10; m++){
		if(NowHumanData[m].body_renew_flag==1){

			char directryname[128];
			sprintf(directryname, "c:/cloth-DB/Man%03d-hist", m);
			CreateDirectory( directryname, NULL );

			int hist_num=0;
			for(hist_num=0; ; hist_num++){
				sprintf(directryname, "c:/cloth-DB/Man%03d-hist/%03d", m, hist_num);
				if(CreateDirectory( directryname, NULL )==1){
					break;
				}
			}

			char filename[128];
			for(int k=0; k<20; k++){
				sprintf(filename, "c:/cloth-DB/Man%03d-hist/%03d/hist3d_image%02d.txt", m, hist_num, k);
				if ((fp = fopen(filename, "a+")) == NULL) {
						exit(EXIT_FAILURE);
				}
				for(int j=0; j<20; j++){
					for(int i=0; i<20; i++){
						fprintf(fp, "%d\n", NowHumanData[m].Hist_3D[i][j][k]);
					}
				}
				fclose(fp);
			}
		}
	}
}


double HUMAN_PROCESS::CompareClothHist(int ***Hist1_3D, int ***Hist2_3D){

	int HumanArea1 = 0;
	int HumanArea2 = 0;

	//ノルムの計算//
	int Norm = 0;
	int dimension = 20;
	for(int ii=0; ii<dimension; ii++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){
				HumanArea1 += Hist1_3D[ii][j][k];
				HumanArea2 += Hist2_3D[ii][j][k];
				Norm+=(Hist1_3D[ii][j][k]-Hist2_3D[ii][j][k])*(Hist1_3D[ii][j][k]-Hist2_3D[ii][j][k]);
			}
		}
	}

	//KL情報量の計算//
	double KL=0;
	double s=HumanArea1;
	double S=HumanArea2;
	for(int ii=0; ii<dimension; ii++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){

				double p=Hist1_3D[ii][j][k];
				double P=Hist2_3D[ii][j][k];

				if(p*P!=0){
					KL+=p/s*log(p/s*(S/P));
				}

			}
		}
	}

	//JS情報量の計算//
	double JS=0;
	for(int ii=0; ii<dimension; ii++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){

				double p=Hist1_3D[ii][j][k];
				double P=Hist2_3D[ii][j][k];

				if(p*P!=0){
					double n=(p/s / (0.5*p/s+0.5*P/S));
					double m=(P/S / (0.5*p/s+0.5*P/S));
					JS+=0.5*p/s*log(n);
					JS+=0.5*P/S*log(m);
					}

					if((p==0)&&(P!=0)){
					double m=(P/S / (0.5*p/s+0.5*P/S));							
					JS+=0.5*P/S*log(m);
					}

					if((P==0)&&(p!=0)){
					double n=(p/s / (0.5*p/s+0.5*P/S));					
					JS+=0.5*P/S*log(n);
				}

			}
		}
	}

	return JS;
}




double HUMAN_PROCESS::ComparePosition(POINT LeftTop1, POINT RightBottom1, POINT LeftTop2, POINT RightBottom2)
{

	int Area1 = (RightBottom1.x - LeftTop1.x + 1) * (RightBottom1.y - LeftTop1.y + 1);
	int Area2 = (RightBottom2.x - LeftTop2.x + 1) * (RightBottom2.y - LeftTop2.y + 1);
	int SmallArea;
	int LargeArea;
	if(Area1>=Area2){
		LargeArea = Area1;
		SmallArea = Area2;
	}
	else{
		SmallArea = Area1;
		LargeArea = Area2;
	}

	IplImage *img = cvCreateImage(cvSize(320,240), 8, 1);

	int pix_count = 0;
	for(int j=0;j<240;j++){
		for(int i=0;i<320;i++){
			if(LeftTop1.x<=i && i<=RightBottom1.x && LeftTop1.y<=j && j<=RightBottom1.y &&
				LeftTop2.x<=i && i<=RightBottom2.x && LeftTop2.y<=j && j<=RightBottom2.y){
				img->imageData[j * 320 + i] = 255;
				pix_count++;
			}
			else{
				img->imageData[j * 320 + i] = 0;
			}
		}
	}

	double repeated_percent = (double)pix_count/SmallArea;

	cvReleaseImage(&img);

	return repeated_percent;
}




void HUMAN_PROCESS::test(IplImage *img_diff)
{

	for(int i=0; i<10; i++){
		if(NowHumanData[i].data_set_flag>=1){
			CvFont dfont;
			cvInitFont (&dfont, CV_FONT_HERSHEY_COMPLEX, 0.3, 0.3, 0, 0.20, CV_AA); 
			cvPutText(img_diff, "ManData", cvPoint(18, 10*(i+1)), &dfont, CV_RGB(RR[i], GG[i], BB[i]));
		}
	}

	for(int i=0; i<10; i++){
		if(NowHumanData[i].candidate.size()>0 && NowHumanData[i].data_set_flag>=1){

			char fn[5][32];
			for(int j=0; j<5; j++){
				sprintf_s(fn[j], "%02d", NowHumanData[i].candidate[j].id);
			}

			int pt_x = NowHumanData[i].ptBodyLeftTop1.x;
			int pt_y = (NowHumanData[i].ptBodyLeftTop1.y + NowHumanData[i].ptBodyRightBottom1.y) / 2;

			CvFont dfont;
			for(int j=0; j<5; j++){
				if(NowHumanData[i].candidate[0].conf!=0 && (double)NowHumanData[i].candidate[j].conf/NowHumanData[i].candidate[0].conf>0.8){
					cvInitFont (&dfont, CV_FONT_HERSHEY_COMPLEX, 0.3, 0.3, 0, 0.20, CV_AA); 
					cvPutText(img_diff, fn[j], cvPoint(pt_x+5, pt_y+10*(j+1)), &dfont, CV_RGB(255, 0, 0));

					int conf_level = NowHumanData[i].candidate[j].conf/200;
					if(conf_level>50){
						conf_level=50;
					}
					cvRectangle(img_diff, cvPoint(pt_x+20, pt_y+10*(j+1)), cvPoint(pt_x+20+conf_level, pt_y+10*(j+1)-5), CV_RGB(0, 255, 0), -1, CV_AA, 0);
				}
			}
		}
	}

}




int HUMAN_PROCESS::FaceRecognition(int p_num)
{

	FILE *fp;
	int nWidth, nHeight, nDepth;
	RAWIMAGE *pImage;
	int nFaceCount=0;
	long lRet;
	int nI;
	int cConf, nConf;
	POINT ptRightTop, ptLeftBottom, ptLeftTop, ptRightBottom;  //高解像度の顔を囲む上下左右の座標
	//POINT ptLeftTop3, ptRightBottom3;  //顔抽出画像作成時の左上・右下の座標
	FR_USER_ID uID[USER_ID_LENGTH * 100], rID[USER_ID_LENGTH];
	int nIDConf[100];
	POINT aptFeatures[ FEATURE_KIND_MAX ] ;
	int aConf[ FEATURE_KIND_MAX ] ;
	POINT aptContour[ CONTOUR_KIND_MAX ] ;
	int pnCount;
	int UpDown, LeftRight, Roll;

	OKAO_Initialize( OKAO_ALL );

	//各ハンドルの作成とＤＢファイルの読み込み
	if(hPT==NULL){
		// 顔器官検出モジュールハンドルの作成
		hPT = OKAO_CreatePointer( PT_MODE_DEFAULT );
		// 顔器官検出結果格納ハンドル作成
		hPtResult = OKAO_CreatePtResult();
		// 顔認証データハンドル作成 
		hFR = OKAO_CreateFaceRecognition( FR_MODE_DEFAULT );
		// 顔認証結果格納ハンドル作成 
		hFrResult = OKAO_CreateIdentifyResult( 50 );
		// 顔器官輪郭検出モジュールハンドルの作成
		hCT = OKAO_CreateContour( CT_MODE_DEFAULT ) ;
		// 顔器官輪郭検出結果格納ハンドル作成
		hCtResult = OKAO_CreateCtResult();
		// データベースハンドル作成 
		hDB = OKAO_CreateDatabase( DB_MODE_DEFAULT, 100, MAX_IMAGES_PER_PERSON );

		int nUserNum,nVectorNum;
		FILE * fp0 = fopen("database10.txt","rb");
		int dbSize;
		fseek(fp0,0,SEEK_END);//ファイル末尾にポインタを移動
		dbSize = ftell(fp0);//バイト数計算
		char *data = new char[dbSize];
		fseek(fp0, 0L, SEEK_SET);//ファイルの先頭にポインタを移動
		fread(data,sizeof(BYTE),dbSize,fp0);
		fclose(fp0);
		OKAO_SetDatabaseData(hDB,(unsigned char*)data,&nUserNum,&nVectorNum);
		delete []data;
	}

////////////////////////////////////////////////////(高解像度での顔検出)

	nHeight = NowHumanData[p_num].FaceImage->height;
	nWidth = NowHumanData[p_num].FaceImage->width;
	nDepth = NowHumanData[p_num].FaceImage->nChannels;
	pImage = (RAWIMAGE*)NowHumanData[p_num].FaceImage->imageData;

	// 顔検出実行
	OKAO_Detection( hDT, pImage, nWidth, nHeight, nDepth, hDtResult );

	// 顔検出結果取得
	OKAO_GetDtFaceCount( hDtResult, &nFaceCount );
	OKAO_GetDtCorner( hDtResult, 0, &ptLeftTop, &ptRightTop, &ptLeftBottom, &ptRightBottom, &cConf);

	int angle=-1;
	OKAO_GetDtFacePose(hDtResult, 0, &angle);

	//顔が検出できない場合はこのフレームでの識別は終了(顔角度が+-30度以外もアウト)
	//if(nFaceCount!=1)
	if(nFaceCount!=1 || angle!=0)
	{
		OKAO_Terminate( OKAO_ALL );
		return nFaceCount;
	}

//////////////////以下から顔認証////////////////////////

	// 顔器官検出実行(最初の顔だけ)
	OKAO_SetPtPosition( hPT, hDtResult, 0 );
	OKAO_Pointer( hPT, pImage, nWidth, nHeight, nDepth, hPtResult, &nConf );
	OKAO_GetPtDirection(hPtResult, &UpDown, &LeftRight, &Roll, &nConf);
	//MData->FacePointConf = nConf;

	////// 顔器官位置検出結果取得
	////OKAO_GetPtPoint( hPtResult , aptFeatures, aConf ) ;

	// 顔器官輪郭検出実行
	OKAO_SetCtPosition( hCT, hPtResult ) ;
	OKAO_Contour( hCT, pImage, nWidth, nHeight, nDepth, hCtResult ) ;

	// 顔器官輪郭検出結果取得
	OKAO_GetCtPoint( hCtResult, aptContour ) ;
	OKAO_GetCtPointNumber(hCtResult, &pnCount);

	double leftx = aptContour[68].x;
	double rightx = aptContour[84].x;
	double AB = rightx - leftx;
	double AB_tyukanx = AB / 2 + leftx;
	double tmp3= 2 * (aptContour[34].x - AB_tyukanx)/ AB;
	double degree1 = asin(tmp3)  * 180.0 / M_PI;

	int face_large=0;
	if(aptContour[2].x<ptLeftTop.x || ptRightBottom.x<aptContour[2].x || aptContour[2].y<ptLeftTop.y || ptRightBottom.y<aptContour[2].y){
		face_large=1;
	}
	if(aptContour[18].x<ptLeftTop.x || ptRightBottom.x<aptContour[18].x || aptContour[18].y<ptLeftTop.y || ptRightBottom.y<aptContour[18].y){
		face_large=1;
	}
	if(aptContour[49].x<ptLeftTop.x || ptRightBottom.x<aptContour[49].x || aptContour[49].y<ptLeftTop.y || ptRightBottom.y<aptContour[49].y){
		face_large=1;
	}

	int ninsyou_flag=1;
	if(angle!=0 || nConf<900 || face_large==1 || abs((int)degree1-LeftRight)>15){
		ninsyou_flag=0;
	}
	//顔角度を厳選  傾きが深い顔はアウト
	if( ninsyou_flag==0 || 20<UpDown || UpDown<-20 || 20<LeftRight || LeftRight<-20 || 10<Roll || Roll<-10){
	//if( ninsyou_flag==0 || !(10>UpDown && UpDown>=-10) || !(10>LeftRight && LeftRight>=-10) || !(10>Roll && Roll>=-10)){
		return 0;
	}

	// 顔認証データの作成
	OKAO_SetFrDataFromPtResult( hFR, pImage, nWidth, nHeight, nDepth, hPtResult );
	// 識別
	OKAO_FrIdentify( hFR, pImage, nWidth, nHeight, nDepth, hDB, 0, hFrResult );
	// 識別結果取得
	OKAO_GetFrResult( hFrResult, uID, nIDConf, &nI );


	char *conf_data = new char[17];
	for(int i=0; i<nI; i++){
		memcpy(conf_data, &uID[16*i], 16);
		conf_data[17]='\0';
		if((int)(NowHumanData[p_num].candidate.size())<nI){
			CANDIDATE1 candid;
			candid.conf = nIDConf[i];
			candid.id = atoi(conf_data);
			NowHumanData[p_num].candidate.push_back(candid);
		}
		else{
			for(int j=0; j<nI; j++){
				if(NowHumanData[p_num].candidate[j].id==atoi(conf_data)){
					NowHumanData[p_num].candidate[j].conf+=nIDConf[i];
					break;
				}
			}
		}
	}

	int hold;
	for(int j=0; j<(int)NowHumanData[p_num].candidate.size()-1; j++){
		for(int i=j+1; i<(int)NowHumanData[p_num].candidate.size(); i++){
			if(NowHumanData[p_num].candidate[j].conf < NowHumanData[p_num].candidate[i].conf){
				hold = NowHumanData[p_num].candidate[j].conf;
				NowHumanData[p_num].candidate[j].conf = NowHumanData[p_num].candidate[i].conf;
				NowHumanData[p_num].candidate[i].conf = hold;

				hold = NowHumanData[p_num].candidate[j].id;
				NowHumanData[p_num].candidate[j].id = NowHumanData[p_num].candidate[i].id;
				NowHumanData[p_num].candidate[i].id = hold;
			}
		}
	}

	NowHumanData[p_num].Recog_success_flag = 1;

	////FILE *fp;
	//fp = fopen("result.txt", "a+");
	//fprintf(fp, "frame:%04d   MD_num:%02d\n", 1, p_num);
	//for(int i=0; i<nI; i++){
	//	fprintf(fp, "id:%02d  conf:%04d\n", NowHumanData[p_num].candidate[i].id, NowHumanData[p_num].candidate[i].conf);
	//}
	//fprintf(fp, "///////////////////////////////////\n\n");
	//fclose(fp);

	//////// メモリ領域の開放
	OKAO_Terminate( OKAO_ALL ) ;

	return 0;
}
*/