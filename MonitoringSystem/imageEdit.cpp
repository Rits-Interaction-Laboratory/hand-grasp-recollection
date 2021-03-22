#include "stdafx.h"
#include "image.h"
//#include "bmp.h"
#include "label.h"
//#include "scenestate.h"
#include "network.h"
#include "imageEdit.h"


#include <queue>
#include <map>
#include <vector>
#include <fstream>
#include "FileOperator.h"
#include <sstream>
#include <algorithm>
#include <time.h>
#include <cmath>

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
//bmp d_human;
//bmp d_human2;
char savename[50];
char savename2[50];
bool hsave = false;

// 差分のしきい値調整のための変数
int diff_threshold = 0.1;
//
int diff_cncc=95,diff_hs=5;

#define EXTRAPIXEL 40
#define FACETHRESHOLD 300

#define KINECT_HUMANS 10
#define OKAOSERVER_HUMANS 10
#define DISTANCE_HUMAN_COUNTVALUE 100
#define FLAME_NUM 20//100//200
#define VALUE 100//50
#define LABEL_MAX 50
#define FLAME_PAST_DEPTH 100//200
#define REGISTER_OBJECT_IMAGE 50

#define OBJECT_DETECT_THREASHOLD 50
#define OBJECT_DETECTED_NUM 1000




M_estimates::M_estimates(){
	param_alpha = 0.8;
	param_eta = 10;
}
M_estimates::~M_estimates(){
	
}




//スレッド(人検知画像保存)
static UINT AFX_CDECL 
	Save(LPVOID pParam)
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
// Name     : imageEdit
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2006.6.27
// Function : シーンの監視(研究のメイン部分)
// Argument : int型のフレーム数,imageクラス
// return   : int型の変数
//--------------------------------------------------
bool Edit::imageEdit(unsigned long frame,image &input)
{        
	//Edit::Diff と Edit::depth_Diff を CV_8UC1(8bit符号無し1チャネル)、inputの画像サイズ、画素値ALL0で初期化 
	Diff_init(input);
	
	int re = 0;

	static unsigned long f=0;	//フラグ
	++f;						//フラグインクリメント


	//背景をM推定を用いて推定(毎フレーム更新)
	//M_estimator(input);

	
#if 0 //画像の保存
	char fname1[40];
	sprintf(fname1,"pre/%07d.png",frame);
	pre.save(fname1);
	//char fname2[40];
	//sprintf(fname2,"now/%07d.bmp",frame);
	//now.save(fname2);
#endif


	//推定背景画像と入力画像の差分
	//BackDiff(input);

	

	//Contract(BACK);
	////膨張
	//Expand(BACK);
	//Contract(BACK);
	//収縮
	//Contract(BACK);//kawa

////影除去(CNCC)
	//if(f>50)	ElimWithCNCC(input);



//距離情報と画像情報との重ね合わせによるマッチング

	if( input.kinect_on )
	{   
		//depth_Diffを CV_8UC1、inputの画像サイズ、画素値ALL0で 初期化
		depth_Diff_init( input );

		//if(f > FLAME_NUM ){}
	    //推定depth背景とdepthの差分
	    //depth_diff(input);


		//背景推定
		depth_M_estimator( input );
		//[FLAME_NUM]フレーム分のdepth背景を利用する
		if( f > FLAME_NUM )
		{
			//Edit::imageEdit()が[FLAME_NUM]回呼ばれたら

			//depth背景(input.depth_Diff)の膨張収縮処理
		    depth_isDiff_Expand_Contract();

			//画像の差分を付け加える
			//input.depth_Diffとinput.depth_human_idの内容をinput.Diffとinput.diff_pixに反映
			depth_image_Matching( input );
		}
		//cout<<"depth"<<endl;
	}


	
	
	//
	//ラベリング
	Labeling(input);

	//input.Diffを解放
	Diff_releace();

	if(input.LabelData()>0){re=1;}
	if(re==1)	return true;
	return false;
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
void Edit::BackDiff(image &input,int b_threshold /* = 20*/)
{
	//cv::Mat cap=cv::Mat(cv::Size(320, 240), CV_8UC1,cv::Scalar(0));
	//static
	double diff;
#if 1
	for(int j=0; j<input.Height(); j++){
		for(int i=0; i<input.Width(); i++){

		    diff=sqrt((double)(input.get_background_L(i,j) - input.get_input_L(i,j)) * (double)(input.get_background_L(i,j) - input.get_input_L(i,j)));
			
			
			if(diff < b_threshold-5){//差が閾値以下なら背景
				set_isDiff(i,j,NON) ;
				input.set_subtraction_B(i,j,0);
				input.set_subtraction_G(i,j,0);
				input.set_subtraction_R(i,j,0);
			}
			else{//背景と差分がでたら
			
				set_isDiff(i,j, BACK) ;
				input.set_subtraction_B(i,j,255);
				input.set_subtraction_G(i,j,255);
				input.set_subtraction_R(i,j,255);
			}

			if((int)input.get_human_id(i,j)!=0)
			{
				for(int k=-2 ; k<3 ; k++)
				{
					if(i>2)
					{
						set_isDiff(i+k,j,NON) ;
						input.set_subtraction_B(i+k,j,0);
						input.set_subtraction_G(i+k,j,0);
						input.set_subtraction_R(i+k,j,255);
					}
				}

			}
			if(input.get_background_L(i,j)==0|| input.get_input_L(i,j)==0)
			{
				set_isDiff(i,j,NON) ;
				input.set_subtraction_B(i,j,0);
				input.set_subtraction_G(i,j,0);
				input.set_subtraction_R(i,j,0);
			}
		}
	}

#endif
	//==============================================================
}

void Edit::ElimWithCNCC(image &input)
{
	double cncc=0;
	double threshold = 0.9;

	for( int w=0; w<input.Width(); w++ ){
		for( int h=0; h<input.Height(); h++ ){
			if(get_isDiff(w,h)==BACK){
				cncc = CalcCNCC5(input,w,h);
				if( cncc == CNCC_ERROR ){ //エラー（基本的に起きないと思うけど）
					set_isDiff(w,h,BACK);  //これでいいのか？？
					input.set_subtraction_B(w,h,255);
					input.set_subtraction_G(w,h,255);
					input.set_subtraction_R(w,h,255);
					//dB(w,h) = 255;
				}else if(cncc == CNCC_ERROR2){  //分散が平均値より小さく、明るくなった(Not 影)
					set_isDiff(w,h,BACK) ;
					input.set_subtraction_B(w,h,255);
					input.set_subtraction_G(w,h,255);
					input.set_subtraction_R(w,h,255);
					//dG(w,h) = 255;
				}else if(cncc == 10){           //分散が平均値より小さく、hとsの相関高(影)
					set_isDiff(w,h,NON) ;
					input.set_subtraction_B(w,h,128);
					input.set_subtraction_G(w,h,128);
					input.set_subtraction_R(w,h,128);
					/*dB(w,h) = dG(w,h) = dR(w,h) = 0;
					dR(w,h) = 255;*/
				}else if(cncc == 11){           //分散が平均値より小さく、hとsの相関低(Not 影)
					set_isDiff(w,h,BACK);
					input.set_subtraction_B(w,h,255);
					input.set_subtraction_G(w,h,255);
					input.set_subtraction_R(w,h,255);
					//dG(w,h) = dB(w,h) = 255;
				}else if(cncc==12){     //h=0 計算不可
					set_isDiff(w,h,BACK) ;
					input.set_subtraction_B(w,h,255);
					input.set_subtraction_G(w,h,255);
					input.set_subtraction_R(w,h,255);
					//dR(w,h) = dB(w,h) = 255;
				}else{
					if( cncc > threshold){
						set_isDiff(w,h,NON) ;
						input.set_subtraction_B(w,h,128);
						input.set_subtraction_G(w,h,128);
						input.set_subtraction_R(w,h,128);
					}else{
						set_isDiff(w,h,BACK);
						input.set_subtraction_B(w,h,255);
						input.set_subtraction_G(w,h,255);
						input.set_subtraction_R(w,h,255);
					}
				}
			} 
			/*else
			{
				input.dB(w,h) = input.dG(w,h) = input.dR(w,h) = 0;
			}*/
			
		}
	}
}

double Edit::CalcCNCC5(image &input, int x, int y)
{

	int windowSizeX = 5, windowSizeY = 5;
	int windowArea = windowSizeX * windowSizeY;

	if(	   x < windowSizeX / 2 || x >= input.Width() - windowSizeX / 2
		|| y < windowSizeY / 2 || y >= input.Height() - windowSizeY / 2 )
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
			bH = input.background_H(i,j);
			bS = input.background_S(i,j);
			bL = (double)input.get_background_L(i,j);
			fH = input.H(i,j) ;
			fS = input.S(i,j) ;
			fL = (double)input.get_input_L(i,j);


			mean_bL += bL;
			mean_fL += fL;
			
			bchx = bS * sin( RADIAN(bH) );
			bchy = bS * cos( RADIAN(bH) );
			fchx = fS  * sin( RADIAN(input.H(i,j)) );
			fchy = fS  * cos( RADIAN(input.H(i,j)) );
			
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


	if((var_a < mean_a/100.0) || (var_b < mean_b/100.0))
	{
		
		if(mean_bL > mean_fL)
		{ 

			bfx=sqrt(bfx);
			bfy=sqrt(bfy);
			double chs=(bfx+bfy)/(windowArea*2);

			if(chs < ((double)diff_hs/100) && chs > 0.0)   
			    return 10;  //前景と背景のHとSの差が低い（影）
			    
			    return 11;  //hとsの差が高い（NOT 影）

		}
		return 11; //暗くなった場合(明度が大きくなる)
	}
	



	cncc = ( ip_ab - mean_fL*mean_bL*(double)windowArea ) / sqrt( var_a * var_b );

	//cnccが-1～1でない
	if( cncc < -1.000001 || cncc > 1.000001 ){
		return CNCC_ERROR;
	}

	return cncc;
}



/*  M_estimator
 *  function:
 *  arg		:	image	: pre	:
			:	image	: now	:
			:	image	: pre_bg:
 *	return	:	none
 */
void Edit::M_estimator(image &input)
{	
	long int M = input.Height()*input.Width();			//画素数M
	int rows = input.img_resize.rows;
	int cols = input.img_resize.cols;
	//cout<<rows<<":"<<cols<<endl;
	static int frame=0;
	static double *Y_d = new double[M]; 
	float *dist_Y=new float[M];
	float *dist_Y_sort =new float[M];
	
	static bool flag=false;
	cv::Mat cap;
	//cv::Mat mask=cv::Mat(cv::Size(input.Width(), input.Height()), CV_8UC1,cv::Scalar(0,0,0));
	input.pixelsO().convertTo(cap,CV_8UC3);

	if(!flag)
	{
		for(int row = 0 ; row < rows; row++)
		{
			for(int col = 0; col < cols; col++)
			{
				int pos = row*cols+ col;
			    Y_d[pos]=0;
				input.set_background_L(col,row,128);

				input.set_background_R(col,row,cap.data[row*cap.step+col*cap.elemSize()+2]);
				input.set_background_G(col,row,cap.data[row*cap.step+col*cap.elemSize()+1]);
				input.set_background_B(col,row,cap.data[row*cap.step+col*cap.elemSize()+0]);
				input.set_mask(col,row,255);
			}
		}
	flag=true;
	}
	int size = cols * rows;
	for(int row = 0 ; row < rows; row++){
		for(int col = 0 ; col < cols; col++ ){
			//dist_Y.push_back(fabs(input.L(col,row)-input.YL(col,row)));
			int pos = row*cols+ col;
			dist_Y[pos]=fabs((float)(input.get_input_L(col,row)-input.get_background_L(col,row)));
			dist_Y_sort[pos]=fabs((float)(input.get_input_L(col,row)-input.get_background_L(col,row)));
		}
	}
	

	//std::sort(dist_Y_sort.begin(), dist_Y_sort.end());
	std::sort(dist_Y_sort,dist_Y_sort+M-1);
	

	float d_median_Y = dist_Y_sort[size/2];

	float sigma_Y = 1.4826*(1 + 5./(size - 1)) * fabs(d_median_Y);
	float sum_x_Y = 0;
	float sum_y_Y = 0;
	int cnt_Y = 0;

	for(int row = 0 ; row < rows; row++){
		for(int col = 0; col < cols; col++){
			int pos = row*cols+ col;
			if(dist_Y[pos] < 2.6*sigma_Y){ sum_x_Y += dist_Y[pos]; sum_y_Y += dist_Y[pos]; cnt_Y++; }

			
		}
	}
	float ave_x_Y = sum_x_Y / (float)cnt_Y;
	float ave_y_Y = sum_y_Y / (float)cnt_Y;
	float standard_deviation_x_Y = 0;
	float standard_deviation_y_Y = 0; 
	float r_sum_Y = 0;

	for(int row = 0 ; row < rows; row++){
		for(int col = 0; col < cols; col++){
			int pos = row*cols+ col;
			if(dist_Y[pos] < 2.5*sigma_Y){
				r_sum_Y += (input.get_background_L(col, row) - ave_x_Y)*(input.get_input_L(col, row) - ave_y_Y);
				standard_deviation_x_Y += (dist_Y[pos] - ave_x_Y) * (dist_Y[pos] - ave_x_Y); 
				standard_deviation_y_Y += (dist_Y[pos] - ave_y_Y) * (dist_Y[pos] - ave_y_Y);
			}
			
		}
	}
	standard_deviation_x_Y = sqrt(standard_deviation_x_Y); 
	standard_deviation_y_Y = sqrt(standard_deviation_y_Y);

	//相関係数
	float r_rob_Y = 1./(float)(size - cnt_Y) * r_sum_Y / (standard_deviation_x_Y * standard_deviation_y_Y);
	//m_estimate.setAlpha();
	//m_setimate.setEta();
	float alpha = m_estimate.getAlpha();
	double eta_eps =1e-8;
	float eta = m_estimate.getEta();
	float eta_Y = frame < 30?10:(1 - r_rob_Y)*(1 - r_rob_Y)*0.2 + eta_eps;
	frame++;


	if(eta_Y>1.0)
	{
		eta_Y=1.0;	
	}

	for(int row = 0; row < input.img_resize.rows; row++){
		for(int col = 0; col < input.img_resize.cols; col++){
			int pos = row*cols+ col;

			m_estimate.setEpsilon((input.get_input_L(col, row) - input.get_background_L(col, row)));
			float epsilon_Y = m_estimate.getEpsilon();
			float psi_Y = m_estimate.calcPsi(epsilon_Y);
			 Y_d[pos] = (float)((-psi_Y) + alpha*Y_d[pos]);
			

			 if(input.get_mask(col,row)==(uchar)255||frame<50){
				// mask.at<uchar>(row,col)=0;
			input.set_background_L(col,row,(input.get_background_L(col,row)- (double)eta_Y * Y_d[pos]));
			if(input.get_background_L(col,row)<0||input.get_background_L(col,row)>255){input.set_background_L(col,row,input.get_background_L(col,row)<0?0:255);}
			
			/*input.set_background_R(col,row,(input.get_background_R(col,row)-  0.299*eta_Y * Y_d[pos]/eta));
			input.set_background_G(col,row,(input.get_background_G(col,row)- 0.587*eta_Y * Y_d[pos]/eta));
			input.set_background_B(col,row,(input.get_background_B(col,row)- 0.114*eta_Y * Y_d[pos]/eta));*/
			//double rho = m_estimate.calcRho(epsilon);
			//multiImageLatest->E(col,row) = (float)( rho + alpha * multiImagePre->E(col, row));
			 }
			 else
			 {
				 //mask.at<uchar>(row,col)=255;
				 if(input.get_human_id(col,row)!=0)
					{input.set_background_L(col,row,(input.get_background_L(col,row)- (eta_Y * Y_d[pos]/eta)*0.1));
				 }

				if(input.get_background_L(col,row)<0||input.get_background_L(col,row)>255){input.set_background_L(col,row,input.get_background_L(col,row)<0?0:255);}
				input.set_background_R(col,row,(input.get_input_R(col,row)));

				input.set_background_G(col,row,(input.get_input_G(col,row)));

				input.set_background_B(col,row,(input.get_input_B(col,row)));
			 }
			 input.set_mask(col,row,255);
		}
	}

	//cv::imshow("test7",mask);
    //cv::waitKey(1);
	//cv::imwrite("a.bmp",lavel);
	delete [] dist_Y_sort;
	delete [] dist_Y;
	//dist_Y_sort.clear();
	//dist_Y.clear();
	
}


//--------------------------------------------------
// Name     : void image::Labeling()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2006.3.17
// function : ２値画像のラベリング処理
// Argument : なし
// return   : なし
//--------------------------------------------------


void Edit::Labeling(image &input)
{
	input.STableReset();
	label Label;

	cv::Mat test=cv::Mat(cv::Size(320, 240), CV_32FC3,cv::Scalar(0,0,0));
	//cv::Mat test1=cv::Mat(cv::Size(320, 240), CV_8UC1,cv::Scalar(0));
	//ラベル設定
	Label.setArea(input.getwidth(),input.getheight());

	//初期化
	Label.init();
	int *new_label;

	//差分画素のみラベリングするため座標をセット//探索領域
	//for(int j=0; j<imageHeight; j++){
	//	for(int i=0; i<imageWidth; i++){
	for(int j=input.area_s_h(); j<input.area_e_h(); j++)
	{
		for(int i=input.area_s_w(); i<input.area_e_w(); i++)
		{
			//input.Diff[i, j]がBACK(1)なら
			if(get_isDiff(i,j) == BACK) 
			{
				//label.Pixels[i, j] = -1
				Label.setPoint(i,j);
				//input.diff_pix[i, j] = [255, 255, 255](白)
				input.set_subtraction_B(i,j,255);
				input.set_subtraction_G(i,j,255); 
				input.set_subtraction_R(i,j,255);
			}
			//input.Diff[i, j]がBACK(1)でないなら
			else{
				//label.Pixels[i, j] = 0
				Label.getPoint(i,j)=0;
			}
		}
	}

	//↑これBACK(1)のところをラベリングしてないか？
	//ラベリング(再帰処理)
	Label.Labeling();

	//#########################################################
	//ここからラベル表示部分

	//各ラベル番号をゲット
	//std::vector<int> min_label   = Label.getminLabel();//小さいラベル
	//std::vector<int> large_label = Label.getlargeLabel();//人物領域


	//int obj_num = Label.getLargestObjLabel(obj_label);//物体候補領域のなかで最も大きい領域
	//int h_num = Label.getLargestObjLabel(large_label);//人物領域のなかで最も大きい領域
	//int object_sh = d_area.s_w;//物体検知領域
	//int object_sw = d_area.s_h;
	//int object_eh = d_area.e_w;
	//int object_ew = d_area.e_h;

	std::vector<int> obj_label;
	//obj_label.reserve(100);
	obj_label= Label.getobjLabel();//人物・物体候補領域(obj_label内には処理で使われた番号が入っている)

	//同一領域（とみなす）でラベルを付ける
	/*if(obj_label.size() != 0){*/
	new_label = new int[obj_label.size()];
	//OTable = new OBJDATA[obj_label.size()];
	/*}*/

	int new_objnum = 0; //新しい物体のラベル数
	int tmp_num =0;
	bool check = false;

	for(std::vector<int>::iterator it = obj_label.begin();it != obj_label.end();++it)
	{
		if(obj_label.size() == 0)	break;//ラベルがない場合は終了

		for(int i=0;i<new_objnum;i++)
		{//新しいオブジェクトと同一のものかを調べる
			//重心の距離求める
			int nx,ny,ox,oy;
			//Label.getGravityCenter(new_label[i],nx,ny);
			nx = input.get_STable_cx(new_label[i]);	 
			ny = input.get_STable_cy(new_label[i]);
			Label.getGravityCenter(*it,ox,oy);
			int dist = sqrt((double)((nx-ox)*(nx-ox)+(ny-oy)*(ny-oy)));
			// if(dist<100){  //同一物体とする
			if (dist<30){  //同一物体とする
				//if(OTable[i].size >= (Label.getLabelSize(*it))){
				new_label[tmp_num] =new_label[i]; //i->new_label[i];
				//面積、重心、外接長方形を更新する
				int tmp_size = input.get_STable_size(new_label[i]) + Label.getLabelSize(*it);

				input.set_STable_cx(new_label[i],((ox*Label.getLabelSize(*it))+(nx*input.get_STable_size(new_label[i]))) / (tmp_size)) ; //重心
				input.set_STable_cy(new_label[i],((oy*Label.getLabelSize(*it))+(ny*input.get_STable_size(new_label[i]))) / (tmp_size)) ;

				input.set_STable_size(new_label[i],tmp_size);  //面積

				int x1,x2,y1,y2;
				Label.getLabelArea(*it,x1,x2,y1,y2);
				if(input.get_STable_s_w(new_label[i]) > x1)	input.set_STable_s_w(new_label[i],x1) ;
				if(input.get_STable_e_w(new_label[i]) < x2)	input.set_STable_e_w(new_label[i],x2) ;
				if(input.get_STable_s_h(new_label[i]) > y1)	input.set_STable_s_h(new_label[i],y1) ;
				if(input.get_STable_e_h(new_label[i]) < y2)	input.set_STable_e_h(new_label[i],y2) ;

				check = true; //ラベル情報を格納したか
				i=new_objnum; //合体させたら終わらす。
				//}


			}


		}

		if(check == false)
		{
			new_label[tmp_num] = new_objnum;  /**it;*/							//新しいラベルと古いラベルの対応
			input.set_STable_size(new_objnum, Label.getLabelSize(*it));			  //ラベル情報更新（面積）
			int x1,x2,y1,y2,cy,cx;
			Label.getLabelArea(*it,x1,x2,y1,y2);
			input.set_STable_s_w(new_objnum,x1);	input.set_STable_e_w(new_objnum,x2);//ラベル情報更新（外接長方形）
			input.set_STable_s_h(new_objnum,y1);	input.set_STable_e_h(new_objnum,y2);
			Label.getGravityCenter(*it,cx,cy);
			input.set_STable_cx(new_objnum,cx);										  //ラベル情報更新（重心）
			input.set_STable_cy(new_objnum,cy);

			new_objnum++;		//ラベル数をカウント
		}
		check = false;
		tmp_num++;

	}//for(std::vector<int>::iterator it = obj_label.begin();it != obj_label.end();++it){




	input.LabelMAX(new_objnum);

	bool no_label=false;

	//物体・人物候補領域ラベル付
	for(int j=0; j<input.Height(); j++)
	{
		for(int i=0; i<input.Width(); i++)
		{
			int sel_num = Label.getPoint(i,j);//ラベルは1以上　0は何もない場所

			input.set_Sub_label(i,j,-1);

			if(sel_num==0){input.set_Sub_label(i,j,-1);}//差分のない領域には-1を代入
			else
			{
				for(int t=0;t<(int)obj_label.size();t++)
				{
					if(obj_label[t]==sel_num)
					{
						input.set_Sub_label(i,j,new_label[t]);
						no_label=true;
					}
				}
				if(!no_label){input.set_Sub_label(i,j,-1);}
			}
			no_label=false;
		}
	}
	/*for(int t=0; t<new_objnum; t++)
	{
	CvFont dfont;
	stringstream ss;
	ss<<t;
	cv::putText(test, ss.str(),cv::Point(input.STable_s_w(t),input.STable_s_h(t)), 0, 1.2, cv::Scalar(0,255,0), 2, CV_AA);
	cv::rectangle(test,cv::Rect(input.STable_s_w(t),input.STable_s_h(t),input.STable_e_w(t)-input.STable_s_w(t),input.STable_e_h(t)-input.STable_s_h(t)),cv::Scalar(0,255,255));
	for(int j=0; j<input.Height(); j++)
	{
	for(int i=0; i<input.Width(); i++)
	{
	if(input.Slabel(i,j)==t){((cv::Point3f*)(test.data+  test.step.p[0]*j))[i].x=255;}	
	}
	}
	cv::imshow("label",test);
	cv::waitKey(1);
	}*/
	obj_label.clear();
	delete [] new_label;//kawa
}

//--------------------------------------------------
// Name     : void image::Contract()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : 収縮処理
// Argument : int   差分のタイプ
// return   : なし
//--------------------------------------------------
void Edit::Contract(const int type)//type=1
{
	int *diffs =new int [Width*Height];


	//「ラベルなし」で初期化
	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			diffs[Width*j+i] = NON;
		}
	}

	//引数で与えられたラベルに対して収縮処理
	for(int z=0;z<1;z++){//収縮回数
	for(int j=1; j<Height-2; j++){
		for(int i=1; i<Width-2; i++){			
			if(get_isDiff(i,j) == type){				
				int count = 0;
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						if(get_isDiff(i+n,j+m) == NON) count++;
					}
				}
				if(count > 0) diffs[Width*j+i] = NON;
				else {diffs[Width*j+i] = type; }
			}
		}
	}
	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			set_isDiff(i,j,diffs[Width*j+i]) ;
		}
	}

}
	delete [] diffs;
}


//--------------------------------------------------
// Name     : void image::Expand()
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : 膨張処理(局所的な処理)
// Argument : なし
// return   : なし
//--------------------------------------------------
void Edit::Expand(const int type)//type=1
{
	int *diffs=new int[Width*Height];

	//初期化
	for(int j=0; j<Height; j++){
		for(int i=0; i<Width; i++){
			diffs[Width*j+i] = NON;
		}
	}
	//あるNONやSHADOWの周りにBACKがある場合BACKさせる。
//	for(int j = 2; j < imageHeight - 4; j++){//kawa
//		for(int i = 2; i < imageWidth - 4; i++){
	for(int z=0;z<3;z++){
	for(int j = 1; j < Height - 2; j++){
		for(int i = 1; i < Width - 2; i++){
			if(get_isDiff(i,j) == NON || get_isDiff(i,j) == SHADOW){
				int count = 0;
//				for(int m = -1;m < 3 ;m++){
//					for(int n = -1;n < 3 ;n++){
				for(int m = -1;m < 2 ;m++){
					for(int n = -1;n < 2 ;n++){
						if(get_isDiff(i+n,j+m) == type)count++;
					}
				}
				if(count > 0) {diffs[Width*j+i] = type;}
			}
			else {diffs[Width*j+i] = type;}
		}
	}
	for(int j=0; j<Height; j++)
	{
		for(int i=0; i<Width; i++)
		{
		    set_isDiff(i,j,diffs[Width*j+i]) ;
		}
	}
}
	delete[] diffs;
}


//背景推定
//--------------------------------------------------
// Name     :　depth_M_estimator
// Author　 : 川本
// Update   : 2013.8.20
// Function : Depth画像の背景推定
// Argument : imageクラス
// return   : なし
//--------------------------------------------------
void Edit::depth_M_estimator(image &input)
{
	static bool bInitialized = false;	
	/*static GLuint depthTexID;
	static GLuint depthTexIDCurrent;*/
	static unsigned char* pDepthTexBuf;
	static unsigned char* pDepthTexBufCurrent;
	static int texWidth, texHeight;
	static cv::Mat pastdepth[FLAME_PAST_DEPTH];
	static int frame=0;
	//static cv::vector<cv::Mat> pastImages;
	static double  sumDepth[320*240]={0};
	static int sumAveCount[320*240] = {0};
	static int sumSdev[320*240]={0};
	static int pixelFlameCount[320*240]={0};
	static int depthCount[320*240]={0};
	static double sigmaDepth[320*240]={0};
	static double sigmaDepthSquare[320*240]={0};
	static double averageDepth[320*240]={0};
	static double variance[320*240]={0};
	//static /*XnLabel*/int sceneData[320*240]={0};
	static vector<double> pastDepth[FLAME_NUM];
	//static cv::Mat flameCountImage(240,320,CV_8UC1,cv::Scalar::all(0));
	/*static cv::Mat deviationImage(240,320,CV_8UC3);*/
	static cv::Mat binaryImage(240,320,CV_8UC1,cv::Scalar::all(0));
	/*static cv::Mat diffImage(240,320,CV_8UC1,cv::Scalar::all(0));
	static cv::Mat rgbImage(240, 320, CV_8UC3);*/
	static double g_pCountHist[FLAME_NUM];
	double topLeftX;
	double topLeftY;
	double bottomRightY;
	double bottomRightX;
	double texXpos;	
	double texYpos;
	int personNum;

	char personStr[3] = "";
	char detectData[64*4] = "";

	int cols=input.getwidth();
	int rows=input.getheight();

	if(!bInitialized)
	{
		input.set_depth_is_diff_size(320,240);
		bInitialized = true;
		for(int i=0;i<FLAME_PAST_DEPTH;i++)pastdepth[i] = cvCreateMat(240, 320, CV_64FC3);
		/*for(int j=0; j< input.Height(); j++){
			for(int i=0; i<input.Width(); i++){
				input.get_bring_object_data().at<float>(j,i)=input.get_depth(i,j);
			}
		}*/
	}

	int currentImd = frame % FLAME_PAST_DEPTH;

	input.depth_copyTo(pastdepth[currentImd]);


	//pastImages.push_back(pastdepth[currentImd].clone());
	// pixelFlameCount[]各ピクセルに使用分のフレームのdepth情報を保存
	//指定フレーム数のdepth情報を保存
	if (frame < FLAME_NUM) 
	{
		int num=0;


		for (int y=0; y<rows; y++) {
			for (int x=0; x<cols; x++) {
				num = pixelFlameCount[y*input.Width()+x] % FLAME_NUM;
				double currentAverageDepth=0.0,currentVariance=0.0;

				int currentPoint = y*input.Width()+x;
				//sceneData[currentPoint] = smd.Data()[currentPoint];
				//人物領域内の場合
				if (input.get_human_id(x,y)>0) {//SceneMetaDataー＞人物データ
					pastDepth[num].push_back(0);
				}
				//人物領域外の場合
				else {
					/*if( input.get_depth(x, y) == 4500 ){
						pastDepth[num].push_back((double)2300);
					}else{*/
						pastDepth[num].push_back((double)input.get_depth(x,y));
					//}
				}


				//累計の平均値と分散を求める為の値を取得
				if (input.get_depth(x,y) >= 500 && input.get_depth(x, y) <= 4500) {//DepthMetaData->実距離データ		//change2019/10/8 depth_range -> 500～4000

					sigmaDepth[currentPoint] += (double)input.get_depth(x,y);
					sigmaDepthSquare[currentPoint] += pow((double)input.get_depth(x,y),2.0);

					sumDepth[currentPoint] += (double)input.get_depth(x,y);
					sumAveCount[currentPoint] +=1;

					sumSdev[currentPoint] += ((double)input.get_depth(x,y)-sumDepth[currentPoint]/(double)sumAveCount[currentPoint] )*((double)input.get_depth(x,y)-sumDepth[currentPoint]/(double)sumAveCount[currentPoint] );

				}
				else{												//change2019/10/8 add else{...}
					sigmaDepth[currentPoint] += (double)0.;
					sigmaDepthSquare[currentPoint] += (double)0.;
					sumDepth[currentPoint] += (double)0.;
					sumAveCount[currentPoint] += 1;
					sumSdev[currentPoint] += (double)0.;
				}
				if( sumAveCount != 0 ){								//change2019/10/8 add if(sumAveCount != 0){ }
					currentAverageDepth = (double)sigmaDepth[currentPoint] / (double)sumAveCount[currentPoint] ;
					currentVariance = (double)sigmaDepthSquare[currentPoint]/(double)sumAveCount[currentPoint] - pow((double)(sigmaDepth[currentPoint]/(double)sumAveCount[currentPoint]),2.0);

					averageDepth[currentPoint] =currentAverageDepth;

					input.set_depth_is_diff(x,y,(float)currentAverageDepth);

					double currentSD = sqrt(currentVariance);
					if(currentSD < VALUE)
						currentSD = VALUE;
					variance[currentPoint] = currentSD*10;
				}
				/*
				variance[currentPoint] = sqrt(pow(((double)sigmaDepth[currentPoint]/(double)sumAveCount[currentPoint]),2.0));
				*/
				//flameCountImage.data[currentPoint] = g_pCountHist[sumAveCount[currentPoint]];

				pixelFlameCount[y*input.Width()+x] +=1;
			}
		}        
	}      
	//最新一定フレーム全てのデータを持っている場合
	else {

		int num = 0;
		bool is_depth_outrange = false;
		//cv::Mat sd_map = cv::Mat( cv::Size(cols, rows), CV_8UC1, cv::Scalar(0) );
		//int MaxSDValue = 100;

		/*
		num = flameCount % FLAME_NUM;
		*/
		for (int row=0; row<rows; row++) {
			for (int col=0; col<cols; col++) {

				int currentPoint = row*cols+col;//配列
				double currentDepth = (double)input.get_depth(col,row);
				if(currentDepth >= 500 && currentDepth <= 4500 ){			//change2019/10/8 change if( currentDepth == 0 ) -> if( currentDepth >= 500 && currentDepth <= 4000 )
					is_depth_outrange = false;								//change2019/10/8 del currentDepth = 0; add is_depth_outrange = false;
				}
				else{
					is_depth_outrange = true;
				}
				/*else if(currentDepth>=4000)
				{
				currentDepth=4000;
				}*/
				//sceneData[currentPoint] = smd.Data()[currentPoint];

				double currentAverageDepth = 0.0;//平均深度
				double currentVariance = 0.0;//分散
				num = pixelFlameCount[currentPoint] % FLAME_NUM;
				pixelFlameCount[currentPoint] +=1;
				double pastAverageDepth = sigmaDepth[currentPoint] / (double)sumAveCount[currentPoint] ;
				double pastVariance = sigmaDepthSquare[currentPoint]/sumAveCount[currentPoint] - pow(((double)sigmaDepth[currentPoint]/(double)sumAveCount[currentPoint]),2.0);
				double pastSD = sqrt(pastVariance);


				//イベント検知で距離が更新された部分は更新された時の距離データを挿入
				if((int)input.get_depth_is_diff_d(col,row)==1)//持ち去り更新
				{
					double back_pastAverageDepth=pastAverageDepth;

					sumAveCount[currentPoint]=0;//=FLAME_NUM;
					sigmaDepthSquare[currentPoint]=0;//sigmaDepthSquare[currentPoint]-sumAveCount[currentPoint]*pow(((double)sigmaDepth[currentPoint]/(double)sumAveCount[currentPoint]),2.0)+sumAveCount[currentPoint]*pow(((double)input.get_depth_is_diff(col,row)),2.0);
					sigmaDepth[currentPoint]=0;//(double)input.get_depth_is_diff(col,row)*sumAveCount[currentPoint];

					pastAverageDepth = ((double)input.get_depth_is_diff(col,row));//*sumAveCount[currentPoint]) / (double)sumAveCount[currentPoint] ;

					for(int i=0;i<FLAME_NUM;i++)
					{
						if( pastDepth[i][currentPoint] != 0 || pastDepth[i][currentPoint] != 8000 ) // 2019/11/22 -> || pastDepth != 8000
						{
							if( pastDepth[i][currentPoint] > 4500/*= DEPTH_RANGE_MAX*/ ) // 2019/11/22
							{
								is_depth_outrange = true;
								cout << "DEPTH_RANGE_MAX 1111111111" << endl;
								pastDepth[i][currentPoint] = 4500; // 2019/11/22

								continue; 
							}	//change2019/10/9 add if( pastDepth[i][currentPoint] == 4500 ){ continue; }else{}
							else{
								is_depth_outrange = false;
								pastDepth[i][currentPoint]=pastDepth[i][currentPoint]+(pastAverageDepth-back_pastAverageDepth);//sqrt((double)sigmaDepthSquare[currentPoint]/FLAME_NUM);
								sigmaDepthSquare[currentPoint]+=pow((double)pastDepth[i][currentPoint],2.0) ;
								sigmaDepth[currentPoint]+=pastDepth[i][currentPoint];
								sumAveCount[currentPoint]++;
							}

						}
					}
					if(sumAveCount[currentPoint]>0){
						pastVariance = sigmaDepthSquare[currentPoint]/sumAveCount[currentPoint] -(double)( pow(((double)sigmaDepth[currentPoint]/(double)sumAveCount[currentPoint]),2.0));
					}
				}

				//背景更新

				if((int)input.get_human_id(col,row)==0&&(currentDepth>=500 && currentDepth<=4500)&&input.get_mask(col,row)!=0&&input.get_mask(col,row)!=125)
				{
					//if (currentDepth>0) {

					//最古のフレームのデプスが500以上
					if (pastDepth[num][currentPoint] >= 500 && sumAveCount[currentPoint] > 0) // 2019/11/22 >= 0 -> >= 500 
					{
						if( pastDepth[num][currentPoint] > 4500/*=DEPTH_RANGE_MAX*/ )//DEPTH_RANGE_MAX)	//change2019/10/8 DEPTH_RANGE_MAX -> 4000 // 2019/11/22 -> > 4500
						{
							is_depth_outrange = true;
							pastDepth[num][currentPoint] = 4500;
							//cout << "DEPTH_RANGE_MAX 2222222222" << endl;
						}
						//else{ // 2019//11/22
						is_depth_outrange = false;
						double past_sub=pastDepth[num][currentPoint]-0.3*(pastDepth[num][currentPoint]-currentDepth);
						sigmaDepth[currentPoint] = sigmaDepth[currentPoint] + (double)past_sub- (double)pastDepth[num][currentPoint];
						sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +((double)( pow(past_sub,2.0)) -(double) ( pow((double)pastDepth[num][currentPoint],2.0)));
						pastDepth[num][currentPoint]=past_sub;
						//} // 2019/11/22
					}
					else if(sumAveCount[currentPoint] <FLAME_NUM) 
					{
						if(pastDepth[num][currentPoint] <= 500) // 2019/11/22 <= 0 -> <= 500
							pastDepth[num][currentPoint] = 500; // 2019/11/22 = 0 -> = 500


						if( sumAveCount[currentPoint] > 0/*&&sumAveCount[currentPoint]<10*/)
						{
							if( pastDepth[num][currentPoint] >= 500)//微小更新を行う // 2019/11/22 -> >= 500
							{
								double past_sub;
								if( pastDepth[num][currentPoint] > 4500 /*=DEPTH_RANGE_MAX*/ ) // 2019/11/22 -> > 4500
								{
									is_depth_outrange = true;
									pastDepth[num][currentPoint] = 4500;
									cout << "DEPTH_RANGE_MAX 3333333333" << endl;
								}
								//else{ // 2019/11/22
								is_depth_outrange = false;
								past_sub = pastDepth[num][currentPoint];
								pastDepth[num][currentPoint] = pastDepth[num][currentPoint]-0.08*(pastDepth[num][currentPoint]-currentDepth);
								sigmaDepth[currentPoint] = sigmaDepth[currentPoint] + (double)pastDepth[num][currentPoint]-past_sub;
								sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +(double)( pow(pastDepth[num][currentPoint],2.0)) -(double) ( pow(past_sub,2.0));
								//} // 2019/11/22
							}
							else//超微小更新を行う
							{

								sigmaDepth[currentPoint] = sigmaDepth[currentPoint] +0.008*(double)currentDepth;
								sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +(double) pow(0.008*currentDepth,2.0);
								sumAveCount[currentPoint]+=1;
								pastDepth[num][currentPoint]=0.008*currentDepth;
							}
						}
						else
						{
							sigmaDepth[currentPoint] = sigmaDepth[currentPoint] +0.1*(double)currentDepth;
							sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +(double) pow(0.1*currentDepth,2.0);
							sumAveCount[currentPoint]+=1;
							pastDepth[num][currentPoint]=0.1*currentDepth;	
						}
					}

					//}
				}
				else if((input.get_mask(col,row)==0||input.get_mask(col,row)==125||(int)input.get_human_id(col,row)>0)&&(currentDepth>=500&&currentDepth<=4500))//マスク0の時物体か人物がいる可能性がある場所
				{  
					if((int)input.get_human_id(col,row)>0)
					{

					}
					else if(input.get_mask(col,row)==125)//物体候補領域
					{
					}
					else if(input.get_mask(col,row)==0)//人物候補領域
					{
						if( pastDepth[num][currentPoint] < 500 )		//
						{												//
							is_depth_outrange = true;					//
							pastDepth[num][currentPoint] = 500;			//
						}												//
						else if( pastDepth[num][currentPoint] > 4500 )	// 2019/11/22 add
						{												//
							is_depth_outrange = true;					//
							pastDepth[num][currentPoint] = 4500;		//
						}												//
						double past_sub=pastDepth[num][currentPoint]-0.001*(pastDepth[num][currentPoint]-currentDepth);
						sigmaDepth[currentPoint] = sigmaDepth[currentPoint] + (double)past_sub- (double)pastDepth[num][currentPoint];
						sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +((double)( pow(past_sub,2.0)) -(double) ( pow((double)pastDepth[num][currentPoint],2.0)));
						pastDepth[num][currentPoint]=past_sub;
					}
					else if ( pastDepth[num][currentPoint] >= 500 ) // 2019/11/22 > 0 -> >= 500 
					{
						double past_sub=pastDepth[num][currentPoint];
						pastDepth[num][currentPoint]=pastDepth[num][currentPoint]-0.001*(pastDepth[num][currentPoint]-currentDepth);
						sigmaDepth[currentPoint] = sigmaDepth[currentPoint] + (double)pastDepth[num][currentPoint]-past_sub;
						sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +(double)( pow(pastDepth[num][currentPoint],2.0)) -(double) ( pow(past_sub,2.0));
					}
					else if(pastDepth[num][currentPoint] == 500 ) // 2019/11/22 == 0 -> == 500
					{

						double past_sub=pastDepth[num][currentPoint];
						pastDepth[num][currentPoint]=pastDepth[num][currentPoint]-0.001*(pastDepth[num][currentPoint]-currentDepth);
						sigmaDepth[currentPoint] = sigmaDepth[currentPoint] + (double)pastDepth[num][currentPoint]-past_sub;
						sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] +(double)( pow(pastDepth[num][currentPoint],2.0)) -(double) ( pow(past_sub,2.0));
						sumAveCount[currentPoint]+=1;
					}
				}
				else
				{

					if( currentDepth == DEPTH_RANGE_MAX && pastDepth[num][currentPoint] > 500 && sumAveCount[currentPoint] > 0 )	//change2019/10/8 currentDepth == 0 -> currentDepth == 4500
					{
						if( pastDepth[num][currentPoint] > 4500/*= DEPTH_RANGE_MAX*/ ) // 2019/11/22 -> > 4500
						{
							is_depth_outrange = true;
							pastDepth[num][currentPoint] = 4500;
							//cout << "DEPTH_RANGE_MAX 4444444444" << endl;
						}
						else{
							is_depth_outrange = false;
							sigmaDepth[currentPoint] = sigmaDepth[currentPoint] - 0.01*pastDepth[num][currentPoint];
							sigmaDepthSquare[currentPoint] = sigmaDepthSquare[currentPoint] - pow(pastDepth[num][currentPoint],2.0)+pow(0.99*pastDepth[num][currentPoint],2.0);                            
							//sumAveCount[currentPoint]-=1;
							pastDepth[num][currentPoint]=0.99*pastDepth[num][currentPoint];
						}
					}
				}

				//人物領域外の場合

				if (sumAveCount[currentPoint] == 0) {
					currentAverageDepth = 0.0;
					currentVariance = 0.0;
				}
				else{
					currentAverageDepth = sigmaDepth[currentPoint] / sumAveCount[currentPoint] ;
					currentVariance = fabs((double)(sigmaDepthSquare[currentPoint]/sumAveCount[currentPoint] - (double)pow((double)(sigmaDepth[currentPoint]/sumAveCount[currentPoint]),2.0)));
				}
				//if(120*input.Width()+180){ cout<<"自乗深度"<< currentVariance<<endl;}
				//変数名の統一
				averageDepth[currentPoint] = currentAverageDepth;

				input.set_depth_is_diff(col,row,(float)currentAverageDepth);
				//input.set_depth_is_diff(col,row,(float)currentDepth);
				double currentSD = sqrt(currentVariance);
				if(currentSD < 10){//VALUE
					currentSD = 10;//VALUE;
				}
				else if(currentSD >=100)//100)
				{
					//currentSD=50;//100;
				}

				//variance[currentPoint] = currentSD*10;                                
				cv::Vec3b pixel_DevValue;

				if(sumAveCount[currentPoint]>FLAME_NUM)
				{
					cout<<sumAveCount[currentPoint];
				}

				if(currentSD>4500||currentSD<0||currentAverageDepth<0||currentVariance<0 )
				{
					//cout<<"偏差"<<currentSD<<endl;
					//cout<<"深度"<<currentAverageDepth<<endl;
					//cout<<"自乗深度"<< currentVariance<<endl;
				}

				//指定の差分以上の場合と、人体が入っていない場合と、指定の分散以内の場合にピクセルを白色に変更
				if ( sumAveCount[row*input.Width()+col]>10 && 
					/*fabs(*/currentAverageDepth - (double)input.get_depth(col,row)/*)*/ > 3 &&
					(int)input.get_human_id(col,row) == 0 &&
					(/*(double)input.get_depth(col,row) > //2.0// 1* currentSD + currentAverageDepth ||*/(double) input.get_depth(col,row) < currentAverageDepth -/*2.0*/1*currentSD ) &&
					((double)input.get_depth(col,row)>=500 && (double)input.get_depth(col, row)<=4500) &&
					currentSD < 20 //&&
					//input.sd_map.at<UCHAR>(row, col) < 5)
					)
				{
					//cout << sumAveCount[currentPoint] << "(sumAveCount[currentPoint])" << endl;
					//cout << currentAverageDepth - (double)input.get_depth(col,row) << "(Average - depth)" << endl;
					//cout << input.get_human_id(col,row) << "(human_id)" << endl;
					//cout << (double) input.get_depth(col,row) << "(depth)" << endl;
					//cout << currentAverageDepth - 1*currentSD << "(Average - currentSD)" << endl;
					//cout << currentSD << "(sd_map at(row, col))" << endl;

					cv::Vec3b pixel_value;  
					pixel_value.val[0]=255;
					pixel_value.val[1]=255;
					pixel_value.val[2]=255;   
					input.depth_Out().at<cv::Vec3b>(row,col)=pixel_value;
					//binaryImage.data[currentPoint] = 255;
					set_depth_isDiff(col,row,BACK); //= BACK;

				}
				else if( (double)input.get_depth(col, row) == DEPTH_RANGE_MAX ){
					cv::Vec3b pixel_value;  
					pixel_value.val[0]=55;
					pixel_value.val[1]=55;
					if( (int)input.get_human_id( col, row ) != 0 )
						pixel_value.val[2]=255;
					else
						pixel_value.val[2]=55;
					input.depth_Out().at<cv::Vec3b>(row,col)=pixel_value;
					set_depth_isDiff(col,row,55); //= OUT OF RANGE(仮);
				}
				else if( input.sd_map.at<UCHAR>(row, col) >= 5 ){
					cv::Vec3b pixel_value;  
					pixel_value.val[0]=55;
					pixel_value.val[1]=55;
					if( (int)input.get_human_id( col, row ) != 0 )
						pixel_value.val[2] = 255;
					else
						pixel_value.val[2] = 55;
					input.depth_Out().at<cv::Vec3b>(row,col)=pixel_value;
					set_depth_isDiff(col,row,55); //= OUT OF RANGE(仮);
				}
				else {
					cv::Vec3b pixel_value;
					pixel_value.val[0]=0;
					pixel_value.val[1]=0;
					if((int)input.get_human_id(col,row)!=0)
						pixel_value.val[2]=255;
					else
						pixel_value.val[2]=0;


					/*
					Image.at<cv::Vec3b>(y,x)=pixel_value;     */ 
					input.depth_Out().at<cv::Vec3b>(row,col)=pixel_value;
					//binaryImage.data[currentPoint] = 0;
					set_depth_isDiff(col,row,NON);// = NON;
				}

				input.set_depth_is_diff_d(col,row,0);
				input.set_mask(col,row,255);
				input.sd_map.at<UCHAR>(row, col) = (int)( currentSD/4500*255 - 3.3/4500 );
				if( currentSD > input.MaxSDValue ){
					//cout << currentSD << "(currentSD_Max), " << currentAverageDepth << "(currentAveDepth)" << endl;
					input.MaxSDValue = currentSD;
				}
				if( currentSD < input.minSDValue ){
					//cout << currentSD << "(currentSD_min)" << currentAverageDepth << "(currentAveDepth)" << endl;
					input.minSDValue = currentSD;
				}
			}
		}
		cv::Mat v_depth_data;
		cv::Mat v_depth_Diff;
		//cv::Mat v_sd_map;
		input.depth_data_copyTo( v_depth_data );
		depth_Diff_O( v_depth_Diff );
		//input.sd_map_copyTo( v_sd_map );
		//v_sd_map.convertTo( v_sd_map, CV_8UC1, 255. / 8000., -500. * 255. / 8000. );
		
		imshow( "view depth_data", v_depth_data );
		imshow( "view depth_Diff", v_depth_Diff );
		imshow( "view SD_MAP", input.sd_map );
		cv::waitKey(1);
	}

	frame++;
}


//--------------------------------------------------
// Name     :　depth_diff
// Author　 : 川本
// Update   : 2013.8.20
// Function : 背景depthと入力depthの比較
// Argument : imageクラス
// return   : なし
//--------------------------------------------------
void Edit::depth_diff(image &input) 
{
	//static
	int rows = input.img_resize.rows;
	int cols = input.img_resize.cols;
	double diff;
	//double distance;

	cv::Mat cap=cv::Mat(cv::Size(cols, rows), CV_8UC1,cv::Scalar(0));
	cv::Mat cap_Y=cv::Mat(cv::Size(cols, rows), CV_8UC1,cv::Scalar(0));
#if 1
	for(int j=0; j<rows; j++){
		for(int i=0; i<cols; i++){
				cap.at<unsigned char>(j,i)=255*(max(0,((int)input.get_depth(i,j)-DEPTH_RANGE_MIN)))/DEPTH_RANGE_MAX;
				cap_Y.at<unsigned char>(j,i)=255*(max(0,((int)input.get_depth_is_diff(i,j)-DEPTH_RANGE_MIN)))/DEPTH_RANGE_MAX;
		}
	}
#endif
	//==============================================================
	/*cv::imshow("depth_input",cap);
	cv::waitKey(1);
	cv::imshow("depth_Y",cap_Y);
	cv::waitKey(1);*/
}

//--------------------------------------------------
// Name     :　depth_image_Matching
// Author　 : 川本
// Update   : 2013.8.20
// Function :ＲＧＢ画像とＤｅｐth画像のマッチング
// Argument : imageクラス
// return   : なし
//--------------------------------------------------
void Edit::depth_image_Matching(image &input)
{
	int rows = input.img_resize.rows;
	int cols = input.img_resize.cols;
	//double diff;
	//double distance;

	//BodyIndexの膨張処理
	input.expand_human_id();

	//rows * cols, 8bit, 1channel, All(0)のcapを作成
	cv::Mat cap=cv::Mat(cv::Size(cols, rows), CV_8UC1,cv::Scalar(0));

	for(int j=0; j<rows; j++){
		for(int i=0; i<cols; i++){
			//input.depth_Diff[i, j]がBACK(1)の場合 
			if(get_depth_isDiff(i,j)==BACK )
			{
				//input.Diff[i, j]にBACK(1)をセット
				set_isDiff(i,j,BACK);
				//差分出力画像 input.diff_pix[i, j]のRGB各値をセット -> [255, 255, 255] => 白
				input.set_subtraction_B(i,j,255);
				input.set_subtraction_G(i,j,255);
				input.set_subtraction_R(i,j,255);
			}
			//input.depth_Diff[i, j]がBACK(1)ではない場合
			else
			{
				//input.Diff[i, j]にNON(0)をセット
				set_isDiff(i,j,NON);
				//差分出力画像 input.diff_pix[i, j]のRGB各値をセット -> [0, 0, 0] => 黒
				input.set_subtraction_B(i,j,0);
				input.set_subtraction_G(i,j,0);
				input.set_subtraction_R(i,j,0);
			}

			//KinectのBodyIndex画像 input.depth_human_id[i, j]が0でない場合(0は人物領域外)
			if(input.get_human_id(i,j)!=0)
			{
				//input.Diff[i, j]にNON(0)をセット
				set_isDiff(i,j,NON);
				//差分出力画像 input.diff_pix[i, j]のRGB各値をセット -> [0, 0, 0] => 黒
				input.set_subtraction_B(i,j,0);
				input.set_subtraction_G(i,j,0);
			    input.set_subtraction_R(i,j,0);
				//input.set_depth_skelton_R(i,j,255);
				
				//cap[i, j]に255(白)をセット
				cap.at<unsigned char>(j,i)=255;
			}
		}
	}
	//まとめ
	//input.Diff	: [i, j]が人物領域 => NON(0) else input.depth_Diff[i, j]がBACK(1)でない => NON(0) それ以外 => BACK(1)
	//input.diff_pix: [i, j]が人物領域 => [0, 0, 0](黒) else input.depth_Diff[i, j]がBACK(1)でない => [0, 0, 0](黒) else => [255, 255, 255](白)
	//cap			: [i, j]が人物領域 => 255(白)

	cv::imshow("kinect_diff",cap); //結局BodyIndex画像
	cv::waitKey(1);
}