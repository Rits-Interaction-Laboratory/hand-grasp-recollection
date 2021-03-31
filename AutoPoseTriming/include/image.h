#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <string.h>
#include <limits.h>
#include <vector>
#include <iostream>
//#include "geometory.h"

#include <opencv2/opencv.hpp>
//#include <GL/glut.h>

//#include "Pair.h"
//#define PI (3.14159265358979323846)
//#include<afxwin.h>

#define Object_count 6
#define DEPTH_RANGE_MAX 4500//3500
#define DEPTH_RANGE_MIN 500//800

using namespace std;
using namespace cv;

typedef std::pair<int, int> PII;
typedef std::pair< PII, int > PIII;
typedef std::pair< std::pair<int, float>, PII>PIFII;

typedef struct{
	int x;
	int y;
	double accuracy;
}point2D;

typedef struct{
	double x;
	double y;
	double confidence;
}Point2D;

typedef struct{
	int x;
	int y;
	int z;
	double accuracy;
}point3D;

typedef struct{
	double x;
	double y;
	double z;
	double confidense;
	double orig_x;
	double orig_y;
	double orig_z;
}Point3D;

typedef struct{
	double dx;
	double dy;
	double dz;
}Vector3D;

//差分をとる範囲の指定
//始点座標(s_h,s_w) 終点座標(e_h,e_w)
typedef struct {
	int s_h;
	int e_h;
	int s_w;
	int e_w;
}Area;

class image {

private:
//---------------------------------------------
//入力画像
//---------------------------------------------
	int imageWidth, imageHeight; //横幅, 縦幅
	cv::Mat pixels;       //RGB 
//---------------------------------------------
//出力要保存データ
//---------------------------------------------
	cv::Mat output_img;   //capture用出力データ

protected:
//各種解放
	void _free_pix()
	{
		pixels.release();
		output_img.release();
	}
//初期化
	void _init()
	{
		try{

			pixels=cv::Mat(imageHeight, imageWidth,  CV_8UC3);//, cv::Scalar(0,0,0));
			output_img=cv::Mat(cv::Size(imageWidth, imageHeight),  CV_8UC3);//, cv::Scalar(0,0,0));//capture用出力画像;
		}
		catch(std::bad_alloc e){
			std::cout<<"Memory alloc error at image::init()."<<std::endl;
		}
	}
public:

	void init()
	{
		_free_pix();
		_init();
	}
	//データ解放
	void release(void)
	{
		_free_pix();
	}
	void pix_reset(void)
	{
		pixels = 0;
	}

	point2D& make_point2D( int x, int y, double a ){
		point2D temp;
		temp.x = x;
		temp.y = y;
		temp.accuracy = a;

		return temp;
	}
	point3D& make_point3D( int x, int y, int z, double a ){
		point3D temp;
		temp.x = x;
		temp.y = y;
		temp.z = z;
		temp.accuracy = a;

		return temp;
	}
	
	cv::Mat img_resize;//現在のRGBのフレーム画像
	cv::Mat img_original;
	cv::Mat img_coverSkeletonColor;
	cv::Mat img_original_depth;
	cv::Mat img_coverSkeletonDepth;
	cv::Mat img_original_depth8U;
	cv::Mat img_coverSkeletonDepth8U;
	cv::Mat img_original_bodyindex;
	cv::Mat img_coverSkeletonBodyIndex;
	Area area;//差分をとる範囲の指定

		//現在フレーム番号
	std::string saveFrame;

	//出力先アドレス
	std::string eventID_dirPath;
	std::string eventID_color_dirPath;
	std::string eventID_depth_dirPath;
	std::string eventID_bodyindex_dirPath;
	std::string eventID_touch_info_dirPath;
	std::string eventID_logs_dirPath;
	std::string eventID_logs_json_dirPath;
	std::string saveOpenPose_dirPath;
	std::string saveOpenPoseImg_dir;
	std::string saveOpenPoseSkeleton_dir;
	std::string saveHandPoints_dir;
	std::string saveHandPoints_pxmm_dir;
	std::string saveHandPoints_mm_dir;

	std::string depth8UDirPath;
	std::string handDirPath;
	std::string handColorDirPath;
	std::string handDepthDirPath;
	std::string handDepth8UDirPath;
	std::string coverSkeletonDirPath;
	std::string coverSkeletonColorDirPath;
	std::string coverSkeletonDepthDirPath;
	std::string coverSkeletonDepth8UDirPath;
	std::string hmnDirPath;
	std::string hmnColorDirPath;
	std::string hmnDepthDirPath;
	std::string hmnDepth8UDirPath;
	std::string objDirPath;
	std::string objColorDirPath;
	std::string objDepthDirPath;
	std::string objDepth8UDirPath;
	std::string touchHandDirPath;
	std::string touchHandColorDirPath;
	std::string touchHandDepthDirPath;
	std::string touchHandDepth8UDirPath;
	std::string touchHumanDirPath;
	std::string touchHumanColorDirPath;
	std::string touchHumanDepthDirPath;
	std::string touchHumanDepth8UDirPath;

//---------------------------------------------
//スケルトン解析用データ
//---------------------------------------------
	std::vector<std::vector<PII>> skeletons;								//Kinect用		[人1[関節1[x,y],...],人2[関節1[x,y],...],...]
	std::vector<std::map<std::string, std::vector<point2D>>> skeletonSets2D;	//OpenPose2D用	[人1["pose2D"[関節1[x,y],...],"face2D"[関節1[x,y],...],"LHand2D"[関節1[x,y],...],"RHand2D"[関節1[x,y],...]],人2[],...]
	std::vector<std::map<std::string, std::vector<point3D>>> skeletonSets3D;	//OpenPose3D用	[人1["pose3D"[関節1[x,y,z],...],"face3D"[関節1[x,y,z],...],"LHand3D"[関節1[x,y,z],...],"RHand3D"[関節1[x,y,z],...]],人2[],...]

	int obj_minX;  //接触物体の初期位置座標
	int obj_maxX;  //
	int obj_minY;  //
	int obj_maxY;  //

	//11フレーム目物体接触時の全人物の手位置情報
	//手位置情報の構造体
	typedef struct{
		int OPID;
		int L0R1;
		int x;
		int y;
		double z;
		int kinectID;
	}Hand11F;

	typedef struct{
		int OPID;
		int x;
		int y;
		double z;
		int kinectID;
	}Hand;

	//フレームに映る人物BOXの座標情報
	typedef struct{
		int minX;
		int minY;
		int maxX;
		int maxY;
		int centerX;
		int centerY;
	}HumanBox;

	//フレームに映る人物左手指BOXの座標情報
	typedef struct{
		int minX;
		int minY;
		int maxX;
		int maxY;
		int centerX;
		int centerY;
		int numFoundPoint;
	}LHandBox;

	//フレームに映る人物右手指BOXの座標情報
	typedef struct{
		int minX;
		int minY;
		int maxX;
		int maxY;
		int centerX;
		int centerY;
		int numFoundPoint;
	}RHandBox;

	//物体接触時(11フレーム)の人物情報
	std::vector<Hand11F> Lhands;
	std::vector<Hand11F> Rhands;
	std::vector<HumanBox> humanBoxes;
	std::vector<int> HumanBoxKinectIDs;

	//他フレームの人物情報
	std::vector<Hand> frameHands;
	std::vector<HumanBox> frameHumanBoxes;
	std::vector<int> frameHumanBoxKinectIDs;
	std::vector<RHandBox> frameRHandBoxes;
	std::vector<LHandBox> frameLHandBoxes;

	int touched_humanNum; //物体に接触した人物のID

	std::map<int, long long> track_skeleton;
	double sizeHeight, sizeWidth;
	image()
	{
		imageWidth  = 1920;//幅
		imageHeight = 1080;//高さ
		_init();//初期化
	}
	// copy constructor
	image(const image& Img)
		: imageWidth(Img.imageWidth), imageHeight(Img.imageHeight)
	{
		_init();
		//pix_copy(Img.get_pixels(),Img.get_pixels_Y());
	}
	image(int width,int height)
	{
		imageWidth = width;//幅
		imageHeight = height;//高さ
		_init();//初期化
	}
	//destructor
	~image()
	{
		_free_pix();
	}
	image& operator=(const image& rhs)
	{
		if(this == &rhs) return *this;
		set(rhs.getwidth(), rhs.getheight());
		//pix_copy(rhs.get_pixels());
		//pix_copy(rhs.get_pixels(),rhs.get_pixels_Y());
		return *this;
	}
	int Height(){return imageHeight;}//指定した高さを返す
	int Width() {return imageWidth; }//指定した横幅を返す
	//入力画像ＲＧＢ Mat内BGR
	inline uchar get_input_R(const int col,const int row){return pixels.data[row*pixels.step+col*pixels.elemSize()+2];}
	inline uchar get_input_G(const int col,const int row){return pixels.data[row*pixels.step+col*pixels.elemSize()+1];}
	inline uchar get_input_B(const int col,const int row){return pixels.data[row*pixels.step+col*pixels.elemSize()+0];}
	inline void set_input_R(const int col,const int row,uchar d){pixels.data[row*pixels.step+col*pixels.elemSize()+2]=d;}
	inline void set_input_G(const int col,const int row,uchar d){pixels.data[row*pixels.step+col*pixels.elemSize()+1]=d;}
	inline void set_input_B(const int col,const int row,uchar d){pixels.data[row*pixels.step+col*pixels.elemSize()+0]=d;}
	cv::Mat &pixelsO(){return pixels;}
	void pixelsM(cv::Mat data){pixels=cv::Mat(data);}

	//capture出力用データ
	inline uchar get_Out_R(const int x,const int y){return output_img.data[output_img.step*y+output_img.elemSize()*x+2];}
	inline uchar get_Out_G(const int x,const int y){return output_img.data[output_img.step*y+output_img.elemSize()*x+1];}
	inline uchar get_Out_B(const int x,const int y){return output_img.data[output_img.step*y+output_img.elemSize()*x+0];}
	inline void set_Out_R(const int x,const int y,uchar d){output_img.data[output_img.step*y+output_img.elemSize()*x+2]=d;}
	inline void set_Out_G(const int x,const int y,uchar d){output_img.data[output_img.step*y+output_img.elemSize()*x+1]=d;}
	inline void set_Out_B(const int x,const int y,uchar d){output_img.data[output_img.step*y+output_img.elemSize()*x+0]=d;}
	cv::Mat &Out(){return output_img;}
	inline void output_img_copyTo(cv::Mat a)
	{   
		a.copyTo(output_img);
	}

	//double型のvector配列を値の昇順で整列した上でその中央値を求める関数
	double median( vector<double> v ) {
		int v_size = v.size();
		vector<double> _v/*( v.size() )*/;
		copy( v.begin(), v.end(), back_inserter( _v ) );
		double tmp;
		for( int i = 0; i < v_size - 1; i++){
			for (int j = i + 1; j < v_size; j++) {
				if ( _v[i] > _v[j] ){
					tmp = _v[i];
					_v[i] = _v[j];
					_v[j] = tmp;
				}
			}
		}
		if ( v_size % 2 == 1) {
			return _v[( v_size - 1) / 2];
		} else {
			return ( _v[ ( v_size / 2 ) - 1 ] + _v[ v_size / 2 ] ) / 2;
		}
	}

	//外れ値を除外してDepthを取得する関数
	double get_fixedDepth( int areaX1, int areaY1, int areaX2, int areaY2, cv::Mat& imgDepth )
	{
		//引数に与えられた領域の全画素値を取得(ただし、0～500の値と8000以上の値を除外)
		std::vector<double> depthValues;
		//引数に与えられた領域が画像範囲内でなければ-1を返して終了
		if( areaX1 >= imgDepth.cols || areaX1 < 0 ){
			std::cout << "the value(" << areaX1 << ") given to argument1(areaX1) indicates out of the image. (image.h image::get_fixedDepth())" << endl;
			return -1;
		}
		if( areaY1 >= imgDepth.rows || areaY1 < 0 ){
			std::cout << "the value(" << areaY1 << ") given to argument2(areaY1) indicates out of the image. (image.h image::get_fixedDepth())" << endl;
			return -1;
		}
		if( areaX2 >= imgDepth.cols || areaX2 < 0 ){
			std::cout << "the value(" << areaX2 << ") given to argument1(areaX2) indicates out of the image. (image.h image::get_fixedDepth())" << endl;
			return -1;
		}
		if( areaY2 >= imgDepth.rows || areaY2 < 0 ){
			std::cout << "the value(" << areaY2 << ") given to argument1(areaY2) indicates out of the image. (image.h image::get_fixedDepth())" << endl;
			return -1;
		}

		for( int row = areaY1; row <= areaY2; row++ ){
			for( int col = areaX1; col <= areaX2; col++ ){
				if( imgDepth.at<ushort>( row, col ) < 500 || imgDepth.at<ushort>( row, col ) >= 8000 ){
					continue;
				}
				depthValues.push_back( imgDepth.at<ushort>( row, col ) );
			}
		}
		//有効値が一つも見つからなかった場合-2を返して終了
		if( depthValues.size() == 0 ){
			cout << "the area-blook defined by argument1-4 has no valid value. (image.h image::get_fixedDepth())" << endl;
			return -2;
		}
		//取得したDepth値の中央値を求める
		double median = image::median( depthValues );
		return median;
	}

	//kinectのスケルトン解析用関数
	PII get_skeletons(int p,int k)	//p:person, k:keypoint
	{
		return skeletons[p][k];
	}
	std::vector<std::vector<PII>> &set_skeletons()
	{
		return skeletons;
	}

	//OpenPoseのスケルトン解析用関数
	std::vector<point2D> get_skeletonType2D( int p, std::string t )
	{
		return skeletonSets2D[p][t.c_str()];
	}
	std::vector<point3D> get_skeletonType3D( int p, std::string t )
	{
		return skeletonSets3D[p][t.c_str()];
	}
	/*PII get_skeletons(int p,int k)
	{
		return skeletons2D[p][k];
	}
	PIII get_skeletons(int a, int b, int c)
	{
		return skeletons3D[a][b][c];
	}*/
	std::vector<std::map<std::string, std::vector<point2D>>> &set_skeletons2D()
	{
		return skeletonSets2D;
	}
	std::vector<std::map<std::string, std::vector<point3D>>> &set_skeletons3D()
	{
		return skeletonSets3D;
	}

	//画像の高さ,幅の設定
	int getwidth() const
	{ 
		return imageWidth;
	}
	int getheight() const
	{ 
		return imageHeight;
	}
	size_t size() const
	{
		return imageWidth * imageHeight * 3;
	}
	size_t size_Y() const
	{
		return imageWidth * imageHeight * 2;
	}
	Area get_area()const
	{
		return area;
	}
	void set(const int w, const int h)
	{
		imageWidth = w;
		imageHeight = h; 
	}
	//検知をする範囲の設定
	void setArea(int sw,int sh,int ew,int eh)
	{
		if(eh > imageHeight) eh = imageHeight - 2;
		if(ew > imageWidth)  ew = imageWidth - 2;
		area.s_w = sw;
		area.s_h = sh;
		area.e_w = ew;
		area.e_h = eh;
	}
	
	int area_s_w(){ return area.s_w; }
	int area_s_h(){ return area.s_h; }
	int area_e_w(){ return area.e_w; }
	int area_e_h(){ return area.e_h; }
};