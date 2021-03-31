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
#include <vector>
#include <iostream>
#include "geometory.h"

#include <opencv2/opencv.hpp>
//#include <GL/glut.h>

#include "Pair.h"
//#define PI (3.14159265358979323846)
#include<afxwin.h>

#define Object_count 6
#define DEPTH_RANGE_MAX 8000//4500//3500
#define DEPTH_RANGE_MIN 500//800

typedef std::pair<int, int> PII;
typedef std::pair< std::pair<int, float>, PII>PIII;

class Matv2 : public cv::Mat{
private:
	Geometory::Area area;
public:

public:
	Matv2(){}
	Matv2(const Matv2 &o){
		*this = o;
		area = o.area;
	}
	Matv2(cv::Mat mat):cv::Mat(mat){

	}
	Matv2(cv::Size size, int type, Geometory::Area area = Geometory::Area(10,10,300,220)):cv::Mat(size, type){

	}
	void setArea(Geometory::Area _area){
		area = _area;
	}
	Geometory::Area getArea(){
		return area;
	}
	cv::Mat mat(){
		return this->clone();
	}
};

class BMP
{
private:
    unsigned char image[320*240];
    //cv::Mat image;
public:
	int x1,x2,y1,y2,cx,cy;
	long int size;
	int flag;
	BMP()
	{    
		for(int i=0;i<320*240;i++)image[i]=0;
		//image=cv::Mat(240,320,CV_8UC1, cv::Scalar(0));
	}
	inline void SetPixColor(const int i, const int j,const unsigned char x)
	{
		image[j*320+i] = x;
		//image.data[j*image.cols+i]=x;
	}

	inline unsigned char &objectT(int i,int j){return image[j*320+i];}
	//cv::Mat write_image(){return image;}
	inline void SetData(const int sx, const int ex,const int sy,const int ey,const int c_x,const int c_y,const int e_flag)
	{
		x1=sx;
		x2=ex;
		y1=sy;
		y2=ey;
		cx=c_x;
		cy=c_y;
		flag=e_flag;
	}
};


//-------------------------------------
// Read me
//-------------------------------------
// pixels : �摜�̃s�N�Z���f�[�^
//			���ォ��E���Ɍ������ăf�[�^�i�[
//			RGB��
//-------------------------------------



//�������Ƃ�͈͂̎w��
//�n�_���W(s_h,s_w) �I�_���W(e_h,e_w)
typedef struct {
	int s_h;
	int e_h;
	int s_w;
	int e_w;
}Area;

//���x���̏����i�[����\����
typedef struct tagLABELdataTABLE {
	int x1, x2, y1, y2;		//�㉺���E�̗̈�
	int xcenter, ycenter;	//�d�S
	long int size;			//���x���ʐ�
} SLABELDATATABLE;


//���̌��̘A���̈�̏����i�[����\����
typedef struct tagOBJECT {
	int x1, x2, y1, y2;    //�㉺���E�̗̈�
	int xcenter, ycenter;  //�d�S
	long int size;    //���x���ʐ�
	int count;   //���t���[���ԉ摜���ɂ�����
	int human_id; //�u�����Ƃ��ɋ߂��ɂ����l��
	std::vector<cv::Mat>Oimage;
} OBJDATA;//OBJCT

typedef struct tagEventDATA{
	int object_id;//����ID
	int event_id; //�C�x���gID
	int model_id; //���f��ID
	int key_point_parent_id;   //����ID
	int frame;    //���t���[���ڂŌ���������
	int human_id; //�l�����f��ID
} DEVENT;

typedef struct tagLabel{
	int x1, x2, y1, y2,cy,cx;    //�㉺���E�̗̈�
	int frame;
}labelD;

typedef struct tagHumanDataT{
	int x1, x2, y1, y2;    //�㉺���E�̗̈�
	int xcenter, ycenter;  //�d�S
	long int size;    //���x���ʐ�
	int id;  //��������ID�@
	int skelton;    //���i�f�[�^
	int frame;      //���t���[���ڂɂ��邩
	int kinect_id;
} HUMANDATA;

typedef struct tag_Human_id
{
	int x1, x2, y1, y2;    //�㉺���E�̗̈�
	int xcenter, ycenter;  //�d�S
	int map_id;
	int id;  //�����̌ŗLID(���t���[��ID)
	int kinect_id;
	//int skelton;    //���i�f�[�^;
	long long track_skelton;
	int frame;      //���t���[���ڑO���炢�邩
	int n_frame;    //���݂̃t���[����
	vector<int> model_ID;  // �ŐV-1�̃��f��ID
	vector<int> object_ID;   // ����ID
	std::vector<PII>pose;//����̎p���̌��m
	std::vector<PII>have_obj;//�ǂ̕��̂������Ă��邩(Model_ID,�ʐ�)
	std::vector<PII> skeletons;
	//int touch[2];//���ƐڐG���Ă��邩
}T_HUMAN_ID;

typedef struct tag_object_id
{
	int x1, x2, y1, y2;    //�㉺���E�̗̈�
	int xcenter, ycenter;  //�d�S
	int size;//�ʐ�
	int model_id;  //�����̃��f��ID(���t���[��ID)
	int object_id;
	int map_id;
	int frame;      //���t���[���ڂɓ�������
	int human_id;//�N�������Ă�����
	int touch;//�Ō�ɐڐG�����l����ID
	int t_frame;//�Ō�ɐڐG�����t���[����
	cv::Mat denta;//���݂̕��̂̌`��
	float denta_size;//�E�݂̑傫��
	std::vector<PII>hide_obj;//�B��������(���f��ID)
}T_OBJECT_ID;

typedef struct EVENT_STATE
{
    std::vector<PII>L3_h;//�l���̏o����(����1�o��-1,�ŗL�l��ID)
	std::vector<PII>L4;//�p�����m(�C�x���g�B��1,����2)�C�l��ID)
	std::vector<PII>L5_t;//�ڐG���m(�ŗL�l��ID,����ID)
	std::vector<PII>L6;//�ړ��B�����ޓ��쌟�m(�B��1����2�ړ�3�A�l��ID)
	std::vector<PII>L6_o;//����ID������
}event_state;

//�����̉�f�̑���
enum DiffType{NON,BACK,KURO,SKIN,HAIR,FACE,HAND,NEITHER,OBJECT,DETECT,B_OBJECT,SHADOW,HITO};
enum Mode{ MODE_ONLINE, MODE_OFFLINE, MODE_NONE };


//-----------------
//image class
//�摜�f�[�^�@���̌��E�l���E�C�x���g���Ȃ�
//-----------------

class image {

private:
//---------------------------------------------
//�I�����C��or�I�t���C��
//---------------------------------------------
	int mode;  
	

//---------------------------------------------
//���͉摜
//---------------------------------------------
	int imageWidth, imageHeight; //����, �c��
	cv::Mat pixels;       //RGB 
	cv::Mat pixels_Y;     //�w�i�摜�f�[�^
    cv::Mat pixels_HLS;   //HSL�f�[�^
	cv::Mat pixels_HLS_Y; //�w�iHSL�f�[�^

//---------------------------------------------	
//�w�i�����f�[�^
//---------------------------------------------

	cv::Mat mask_back;      //�w�i�}�X�N
	cv::Mat label_S;        //���x���Ƃ��Č��m���ꂽ�̈�
	SLABELDATATABLE *STable;      //���o�������x���f�[�^
	int numlabel;                 //���o���ꂽ���x���̐�

//---------------------------------------------
//�l�����f�[�^
//---------------------------------------------
	HUMANDATA *human_id;
	cv::Mat Human_img;  //�摜���̐l���̈�
	cv::Mat Human_label;//�摜���̐l���̈�ɒ�`���ꂽ���x��
	Area HArea;//�l�����m�̈�
	int numhuman;

//---------------------------------------------
//�l���ǐՁE�p���E�ێ��f�[�^
//---------------------------------------------
	cv::Mat human_map_data;//�O�t���[���̐l���̏ꏊ
	//std::map<int,int> human_ID_data;//
	//std::map<int,int> skelton_ID_data;//���X�P���g���̏ꏊ
	//erase:insert
	T_HUMAN_ID *t_human_id;//��ԓ��ɂ���l��
	int t_numhuman;

//---------------------------------------------
//���̌��f�[�^
//---------------------------------------------
	//���̂�A���ŕ�����
	OBJDATA *OTable;//���̌��̈�
	unsigned long save_frame;
	Area area;//�������Ƃ�͈͂̎w��
	
	//cv::Mat Object_lavel;//�摜���̕��̗̈�ɒ�`���ꂽ���x��

	std::vector<BMP>Object_label;

	Area OArea;//���̌��m�̈�
	int numobject;

	cv::Mat object_map_data;//���݂̕��̏��
	cv::Mat object_mask_area;//���̂����m�\�ȗ̈�

//---------------------------------------------
//���̃f�[�^
//---------------------------------------------
	//std::map<int,int> object_ID_data;//�����������t���[����
	std::vector<T_OBJECT_ID>t_object_id;//��ԓ��ɂ��镨��
	cv::Mat bring_object_data;

//---------------------------------------------
//�������ݎ�������ړ����m�f�[�^
//---------------------------------------------
	DEVENT *Event_ID;
	labelD *EventDATA;
	int Event_max;
	std::vector<int>event_log;


//---------------------------------------------
//�o�͗v�ۑ��f�[�^
//---------------------------------------------
	cv::Mat output_img;   //capture�p�o�̓f�[�^
	cv::Mat diff_pix;     //�����o�͉摜
	cv::Mat Object_img; //���C���o�͗p�摜���̕��̗̈�

//---------------------------------------------
//�L�l�N�g�p�f�[�^
//---------------------------------------------
	
	cv::Mat depth_data;   //�����f�[�^�p�o�̓f�[�^
	cv::Mat depth_is_diff;//�������
	cv::Mat depth_mask;   //�w�i���X�V���ꂽ�ꏊ�@0?1

	cv::Mat depth;        //���t���[���̋������
	cv::Mat depth_human_id;//�L�l�N�g�����@�\��蓾���l��ID������(BodyIndex)
	cv::Mat depth_skelton;//�X�P���g�����
	std::vector<std::vector<PII>> skeletons;
	cv::Mat pointCloud_3D;//3������ԃf�[�^

	std::vector<std::vector<std::vector<PIII>>> frame_skeletons; 
	std::vector<std::vector<int>> back_frame_pertinent_skelton;
	//skelton�����f�[�^�d�l(�㔼�g�̂�)[3�F���A2:��A1:���A0:���A4:�E���A5:�E�r�֐�6:�E���7:�E��
	//8:�����A9:���r�֐߁A10:�����A11:�����]

//�֐��֌W
protected:
	//�e����
	void _free_pix()
	{
		pixels.release();
		pixels_Y.release();
		pixels_HLS.release();
		pixels_HLS_Y.release();
		Human_img.release();
		Human_label.release();

		Object_label.clear();

		label_S.release();
		output_img.release();
		diff_pix.release();
		Object_img.release();
		mask_back.release();
	}

	//������
	void _init()
	{
		try{

			pixels=cv::Mat(imageHeight, imageWidth,  CV_8UC3);//, cv::Scalar(0,0,0));
			pixels_Y=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC3);//, cv::Scalar(0,0,0));
			pixels_HLS=cv::Mat(cv::Size(imageWidth, imageHeight), CV_32FC1 );//, cv::Scalar(0,0,0));  
			pixels_HLS_Y=cv::Mat(cv::Size(imageWidth, imageHeight), CV_32FC1);//, cv::Scalar(0,0,0));
			
			Human_img=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC3);//, cv::Scalar(0,0,0));//�摜���̐l���̈�
			Human_label=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1);   //�摜���̐l���̈�ɒ�`���ꂽ���x��


	        //Object_lavel=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1); //�摜���̐l���̈�ɒ�`���ꂽ���x��
			Object_label.reserve(50);
			BMP a;
			Object_label.push_back(a);

			label_S=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1); //�摜���̐l���E���̌��̈�  
			numobject=0;

			output_img=cv::Mat(cv::Size(imageWidth, imageHeight),  CV_8UC3);//, cv::Scalar(0,0,0));//capture�p�o�͉摜;
			diff_pix=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC3);//, cv::Scalar(0,0,0));//CV_32FC3�o�͗p�w�i�摜
			Object_img =cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC3, cv::Scalar(0,0,0)); //�摜���̐l���̈�
			
			depth_data=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC3, cv::Scalar(0,0,0)); //�摜���̐l���̈�;
			
			depth_is_diff=cv::Mat(cv::Size(imageWidth, imageHeight), CV_32FC1, cv::Scalar(0));
			depth_mask=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1, cv::Scalar(0));

			//arimoto
			sd_map = cv::Mat( cv::Size( imageWidth, imageHeight ), CV_8UC1, cv::Scalar(0) );
			MaxSDValue = 3;
			minSDValue = 8000;
	
			depth=cv::Mat(cv::Size(imageWidth, imageHeight),CV_32FC1, cv::Scalar(0));
			depth_human_id=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1, cv::Scalar(0));
			depth_skelton=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC3, cv::Scalar(0,0,0));
	
			human_map_data=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1, cv::Scalar(0));
            object_map_data=cv::Mat(cv::Size(imageWidth, imageHeight), CV_32FC1, cv::Scalar(0));
			bring_object_data=cv::Mat(cv::Size(imageWidth, imageHeight), CV_32FC1, cv::Scalar(DEPTH_RANGE_MAX));


			STable      =  new SLABELDATATABLE [1000];

			human_id	=  new HUMANDATA [10];

			OTable      =  new OBJDATA   [1000];

			t_human_id  =  new T_HUMAN_ID[10];
			t_numhuman=0;

			mask_back  =cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1);//, cv::Scalar(0,0,0));//kawa
			event_log.reserve(10);
			numobject=0;//���̐��̏�����


			//�ڑ����Ă���L�l�N�g���g���ꍇ��true�ɐݒ�
			kinect_on=false;

		}
		catch(std::bad_alloc e){
			std::cout<<"Memory alloc error at image::init()."<<std::endl;
			AfxMessageBox("Memory alloc error at image::init().");
		}
	}


public://--------------------------------------------------------------------
	
	void init()
	{
		_free_pix();
		_init();
	}
	//�f�[�^���
	void release(void)
	{
		_free_pix();
	}
	void pix_reset(void)
	{
		pixels = 0;
	}
	//���[�h�̐ݒ�
	void set_mode(int m)
	{
		mode = m;
	}
    cv::Mat img_HLS;//���݂�HSV�̃t���[���摜
	cv::Mat img_resize;//���݂�RGB�̃t���[���摜
	cv::Mat img_original;
	cv::Mat img_depth;

	//arimoto
	cv::Mat sd_map;

	std::map<int,long long> track_skelton;
	event_state n_event_state;
	
	double sizeHeigth,sizeWidth;
	
	std::vector<Pair*> tracker;
	Pair* tracker_data;
	std::vector<cv::Mat> tracker_img;
	
	//�������p
	bool kinect_on;

	enum COLOR
	{
		BLACK,WHITE,RED,BLUE,GREEN
	};//��f�̐F

	//arimoto
	int MaxSDValue;	//depth�W���΍��̍ō��L�^�l
	int minSDValue; //depth�W���΍��̍Œ�L�^�l

	image()
	{
		imageWidth  = 320;//��
		imageHeight = 240;//����
		_init();//������
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
		imageWidth = width;//��
		imageHeight = height;//����
		_init();//������
	}

	//destructor
	~image()
	{
		_free_pix();
	}



	// operator=
	image& operator=(const image& rhs)
	{
		if(this == &rhs) return *this;
		set(rhs.getwidth(), rhs.getheight());
		//pix_copy(rhs.get_pixels());
		//pix_copy(rhs.get_pixels(),rhs.get_pixels_Y());
		return *this;
	}

	int Height(){return imageHeight;}//�w�肵��������Ԃ�
	int Width() {return imageWidth; }//�w�肵��������Ԃ�

	//���͉摜�q�f�a Mat��BGR
	inline uchar get_input_R(const int col,const int row){return pixels.data[row*pixels.step+col*pixels.elemSize()+2];}
	inline uchar get_input_G(const int col,const int row){return pixels.data[row*pixels.step+col*pixels.elemSize()+1];}
	inline uchar get_input_B(const int col,const int row){return pixels.data[row*pixels.step+col*pixels.elemSize()+0];}
	inline void set_input_R(const int col,const int row,uchar d){pixels.data[row*pixels.step+col*pixels.elemSize()+2]=d;}
	inline void set_input_G(const int col,const int row,uchar d){pixels.data[row*pixels.step+col*pixels.elemSize()+1]=d;}
	inline void set_input_B(const int col,const int row,uchar d){pixels.data[row*pixels.step+col*pixels.elemSize()+0]=d;}
	cv::Mat &pixelsO(){return pixels;}
	void pixelsM(cv::Mat data){pixels=cv::Mat(data);}

	//�w�i�摜RGB
	inline uchar get_background_R(const int i,const int j){return pixels_Y.data[j*pixels_Y.step+i*pixels_Y.elemSize()+2];}
	inline uchar get_background_G(const int i,const int j){return pixels_Y.data[j*pixels_Y.step+i*pixels_Y.elemSize()+1];}
	inline uchar get_background_B(const int i,const int j){return pixels_Y.data[j*pixels_Y.step+i*pixels_Y.elemSize()+0];}
	inline void set_background_R(const int i,const int j,uchar d){pixels_Y.data[j*pixels_Y.step+i*pixels_Y.elemSize()+2]=d;}
	inline void set_background_G(const int i,const int j,uchar d){pixels_Y.data[j*pixels_Y.step+i*pixels_Y.elemSize()+1]=d;}
	inline void set_background_B(const int i,const int j,uchar d){pixels_Y.data[j*pixels_Y.step+i*pixels_Y.elemSize()+0]=d;}
	cv::Mat &pixelsY(){return pixels_Y;}


	//capture�o�͗p�f�[�^
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

	//�����摜�o�͗p
	inline uchar get_subtraction_R(const int x,const int y){return diff_pix.data[diff_pix.step*y+diff_pix.elemSize()*x+2];}
	inline uchar get_subtraction_G(const int x,const int y){return diff_pix.data[diff_pix.step*y+diff_pix.elemSize()*x+1];}
	inline uchar get_subtraction_B(const int x,const int y){return diff_pix.data[diff_pix.step*y+diff_pix.elemSize()*x+0];}
	inline void set_subtraction_R(const int x,const int y,uchar d){diff_pix.data[diff_pix.step*y+diff_pix.elemSize()*x+2]=d;}
	inline void set_subtraction_G(const int x,const int y,uchar d){diff_pix.data[diff_pix.step*y+diff_pix.elemSize()*x+1]=d;}
	inline void set_subtraction_B(const int x,const int y,uchar d){diff_pix.data[diff_pix.step*y+diff_pix.elemSize()*x+0]=d;}
	cv::Mat &dOut(){return diff_pix;}

	//���͉摜HLS
	inline double H(const int i,const int j)
	{
		unsigned char rgbmax, rgbmin;
		double h;
		rgbmax = max((get_input_R(i,j)), max(get_input_G(i,j),get_input_B(i,j)));
		rgbmin = min((get_input_R(i,j)), min(get_input_G(i,j),get_input_B(i,j)));

		if( rgbmax == rgbmin ){
			return 0;
		} else {
			double max_min = (double)rgbmax - (double)rgbmin;
			double cr = ( (double)rgbmax - (double)get_input_R(i,j) ) / max_min;
			double cg = ( (double)rgbmax - (double)get_input_G(i,j)) / max_min;
			double cb = ( (double)rgbmax - (double)get_input_B(i,j) ) / max_min;
			//double h;
			if( get_input_R(i,j) == rgbmax ){
				h = cb - cg;
			} else if( get_input_G(i,j) == rgbmax ){
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
	inline double S(const int i,const int j)
	{
		unsigned char rgbmax, rgbmin;

		rgbmax = max((get_input_R(i,j)), max(get_input_G(i,j),get_input_B(i,j)));
		rgbmin = min((get_input_R(i,j)), min(get_input_G(i,j),get_input_B(i,j)));

		double s = ( (double)rgbmax - (double)rgbmin ) / 255.0;
		return s;
	}
														    
	inline float get_input_L(const int x,const int y){return pixels_HLS.at<float>(y,x);}
	inline void set_input_L(const int x,const int y,float d){pixels_HLS.at<float>(y,x)=d;}
	
	cv::Mat pixels_HSLO(){return pixels_HLS;}
	void pixels_HSLM(cv::Mat data){pixels_HLS=cv::Mat(data);}


	//HSL�w�i�摜
	inline double background_H(const int i,const int j)
	{
		unsigned char rgbmax, rgbmin;
		float h;
		rgbmax = max((get_background_R(i,j)), max(get_background_G(i,j),get_background_B(i,j)));
		rgbmin = min((get_background_R(i,j)), min(get_background_G(i,j),get_background_B(i,j)));

		if( rgbmax == rgbmin ){
			return 0;
		} else {
			double max_min = (double)rgbmax - (double)rgbmin;
			double cr = ( (double)rgbmax - (double)get_background_R(i,j) ) / max_min;
			double cg = ( (double)rgbmax - (double)get_background_G(i,j)) / max_min;
			double cb = ( (double)rgbmax - (double)get_background_B(i,j) ) / max_min;
			//double h;
			if( get_background_R(i,j) == rgbmax ){
				h = (float)(cb - cg);
			} else if( get_background_G(i,j) == rgbmax ){
				h = (float)(2.0 + cr - cb);
			} else {
				h = (float)(4.0 + cg - cr);
			}
			h *= 60.0;
			if( h < 0 )
				h += 360.0;

			return h;
		}
	}
	inline double background_S(const int i,const int j)
	{
		unsigned char rgbmax, rgbmin;

		rgbmax = max((get_background_R(i,j)), max(get_background_G(i,j),get_background_B(i,j)));
		rgbmin = min((get_background_R(i,j)), min(get_background_G(i,j),get_background_B(i,j)));

		double s = ( (double)rgbmax - (double)rgbmin ) / 255.0;

		return s;
		//return ((cv::Point3f*)( pixels_HLS_Y.data+ pixels_HLS_Y.step.p[0]*y))[x].y;
	}
	inline float get_background_L(const int x,const int y){return pixels_HLS_Y.at<float>(y,x);}
	inline void set_background_L(const int x,const int y,float d){pixels_HLS_Y.at<float>(y,x)=d;}
	
	cv::Mat &pixels_HSLY(){return pixels_HLS_Y;}


	//�l���摜
	inline uchar get_Human_R(const int x,const int y){return Human_img.data[Human_img.step*y+Human_img.elemSize()*x+2];}
	inline uchar get_Human_G(const int x,const int y){return Human_img.data[Human_img.step*y+Human_img.elemSize()*x+1];}
	inline uchar get_Human_B(const int x,const int y){return Human_img.data[Human_img.step*y+Human_img.elemSize()*x+0];}
	inline void set_Human_R(const int x,const int y,uchar d){Human_img.data[Human_img.step*y+Human_img.elemSize()*x+2]=d;}
	inline void set_Human_G(const int x,const int y,uchar d){Human_img.data[Human_img.step*y+Human_img.elemSize()*x+1]=d;}
	inline void set_Human_B(const int x,const int y,uchar d){Human_img.data[Human_img.step*y+Human_img.elemSize()*x+0]=d;}

	
	//�l������
	inline uchar get_Human_label(const int x,const int y){ return Human_label.data[Human_label.step*y+Human_label.elemSize()*x];}
	inline void set_Human_label(const int x,const int y,uchar d){Human_label.data[Human_label.step*y+Human_label.elemSize()*x]=d;}
	inline void Human_label_clear(){Human_label=cv::Mat::zeros(Human_label.cols,Human_label.rows, CV_8U);}
	

	//���C���o�͗p���̏�ԉ摜
	inline uchar get_Object_R(const int x,const int y){return Object_img.data[Object_img.step*y+Object_img.elemSize()*x+2];}
	inline uchar get_Object_G(const int x,const int y){return Object_img.data[Object_img.step*y+Object_img.elemSize()*x+1];}
	inline uchar get_Object_B(const int x,const int y){return Object_img.data[Object_img.step*y+Object_img.elemSize()*x+0];}
	inline void set_Object_R(const int x,const int y,uchar d){Object_img.data[Object_img.step*y+Object_img.elemSize()*x+2]=d;}
	inline void set_Object_G(const int x,const int y,uchar d){Object_img.data[Object_img.step*y+Object_img.elemSize()*x+1]=d;}
	inline void set_Object_B(const int x,const int y,uchar d){Object_img.data[Object_img.step*y+Object_img.elemSize()*x+0]=d;}
	cv::Mat &pixels_O(){return Object_img;}

	inline void OSetPixColor(const int x,const int y,const unsigned char r,const unsigned char g,const unsigned char b)
	{ 
		
		Object_img.data[Object_img.step*y+Object_img.elemSize()*x+2]=r;
		Object_img.data[Object_img.step*y+Object_img.elemSize()*x+1]=g;
		Object_img.data[Object_img.step*y+Object_img.elemSize()*x+0]=b;
	}
	inline void OSet_input_PixColor(const int x,const int y)
	{ 
		
		Object_img.data[Object_img.step*y+Object_img.elemSize()*x+2]=pixels.data[y*pixels.step+x*pixels.elemSize()+2];
		Object_img.data[Object_img.step*y+Object_img.elemSize()*x+1]=pixels.data[y*pixels.step+x*pixels.elemSize()+1];
		Object_img.data[Object_img.step*y+Object_img.elemSize()*x+0]=pixels.data[y*pixels.step+x*pixels.elemSize()+0];
	}

	//kinect�摜�o�͗p
	//input.depth_data��Ԃ�
	cv::Mat &depth_Out(){return depth_data;}

	//Depth�w�i
	inline void set_depth_is_diff_size(int w,int h)
	{
		depth_is_diff=cv::Mat(cv::Size(w, h), CV_32FC1, cv::Scalar(0));
		depth_mask=cv::Mat(cv::Size(w, h), CV_8UC1, cv::Scalar(0));
		
	}

	inline void set_depth_size(int w,int h)
	{
		depth=cv::Mat(cv::Size(w, h),CV_32FC1, cv::Scalar(0));
        depth_human_id=cv::Mat(cv::Size(Width(),Height()), CV_8UC1, cv::Scalar(0));
		depth_skelton=cv::Mat(cv::Size(w, h), CV_8UC3, cv::Scalar(0,0,0));
		pointCloud_3D=cv::Mat(cv::Size(320, 240), CV_32FC3, cv::Scalar(0,0,0));
	}
	inline float get_depth_is_diff(int x,int y)
	{
		float* data = &depth_is_diff.at<float>(y,x);
		return  data[0];
	}
	inline uchar get_depth_is_diff_d(int x,int y)
	{
		return  depth_mask.data[depth_mask.step*y+depth_mask.elemSize()*x];	
	}
	inline void set_depth_is_diff(int x,int y,float d)
	{
		float* data =&depth_is_diff.at<float>(y,x);
        data[0]=d;
	}
	inline void set_depth_is_diff_d(int x,int y,int d)
	{
		depth_mask.data[depth_mask.step*y+depth_mask.elemSize()*x]=d;

	}

	//arimoto
	inline void set_sd_map( int x, int y, int d )
	{
		sd_map.data[ sd_map.step * y + sd_map.elemSize() * x ] = d;
	}

	//�[�x���o�͂Ɠ���
	inline float get_depth(int x,int y)
	{
		if(x>=320)x=319;//�͈͑ΏۊO�Q�ƏC��
		if(y>=240)y=239;
		if(x<0)x=0;
		if(y<0)y=0;
		if(depth.at<float>(y,x)==0)return DEPTH_RANGE_MAX;
		float* data = &depth.at<float>(y,x);
		return  data[0];
	}
	inline void set_depth(int x,int y,unsigned short d)
	{
		float* data =&depth.at<float>(y,x);
        data[0]=d;
	}

	inline cv::Mat depth_O()
	{
		return depth;
	}
	inline void depth_data_copyTo(cv::Mat &a)
	{
		depth_data.copyTo(a);
	}

	//arimoto
	inline void sd_map_copyTo( cv::Mat &a )
	{
		sd_map.copyTo(a);
	}

	inline void resize_depth()
	{
		cv::resize(depth,depth,cv::Size(Width(),Height())/*, cv::INTER_CUBIC*/);
		cv::resize(depth_skelton,depth_skelton,cv::Size(Width(),Height()));
	}
	inline void depth_copyTo(cv::Mat &a)
	{   
		depth.copyTo(a);
	}

	inline int get_human_id(int x,int y)
	{
		return  depth_human_id.data[depth_human_id.step*y+depth_human_id.elemSize()*x];
	}
	inline void set_human_id(int x,int y,int d)
	{
		depth_human_id.data[depth_human_id.step*y+depth_human_id.elemSize()*x]=d;
	}

	inline void expand_human_id()
	{
		        cv::dilate(depth_human_id,depth_human_id, cv::Mat(), cv::Point(-1,-1), 2);//BodyIndex ��c��
	}

	inline UCHAR* get_depth_skelton(int x,int y)
	{
		return  &depth_skelton.data[depth_skelton.step*y+depth_skelton.elemSize()*x];
	}
	inline void set_depth_skelton_R(const int x,const int y,uchar d){depth_skelton.data[depth_skelton.step*y+depth_skelton.elemSize()*x+2]=d;}
	inline void set_depth_skelton_G(const int x,const int y,uchar d){depth_skelton.data[depth_skelton.step*y+depth_skelton.elemSize()*x+1]=d;}
	inline void set_depth_skelton_B(const int x,const int y,uchar d){depth_skelton.data[depth_skelton.step*y+depth_skelton.elemSize()*x+0]=d;}
	inline void depth_skelton_copyTo(cv::Mat &a)
	{   
		a.copyTo(depth_skelton);
	}
	inline cv::Mat depth_skelton_output()
	{
		return depth_skelton;
	}

    PII get_skeletons(int t,int n)
	{
		return skeletons[t][n] ;
	}

	std::vector<std::vector<PII>> &set_skeltons()
	{
		return skeletons;
	}

	int get_frame_skeltons(int t,int num)
	{
		return back_frame_pertinent_skelton[t][num];
	}
	std::vector<std::vector<int>> &set_back_frame_skeltons()
	{
		return back_frame_pertinent_skelton;
	}

	void set_log_skelton_data(int frame)
	{
		if(skeletons.size()==0)
		{
			return ;
		}
		int total=0;
		static int back_frame;
		PIII data;
		std::vector<PIII> skeleton_data;
		std::vector<std::vector<PIII>>skeleton_data2;
		std::vector<int> pertinent_data;
		

		for(int i = 0 ; i < (int)skeletons.size(); i++)
		{
			for(int j = 0; j < (int)skeletons[i].size() ; j++)
			{
				int X=skeletons[i][j].first;
				int Y=skeletons[i][j].second;
				data.first.first  =frame;
				data.first.second =get_depth(X,Y);
				data.second.first =X;
				data.second.second=Y;	
				skeleton_data.push_back(data);
			}
			pertinent_data.push_back(-1);
			skeleton_data2.push_back(skeleton_data);
			skeleton_data.clear();

		}
		frame_skeletons.push_back(skeleton_data2);
		back_frame_pertinent_skelton.push_back(pertinent_data);


		if((int)frame_skeletons.size()>1 && ((frame_skeletons[frame_skeletons.size()-1][0][0].first.first-frame_skeletons[frame_skeletons.size()-2][0][0].first.first)<5))
		{
			for(int i = 0; i < frame_skeletons[(int)frame_skeletons.size()-1].size() ; i++)
			{
				for(int j = 0; j < frame_skeletons[(int)frame_skeletons.size()-2].size() ; j++)
				{
					for(int t = 0; t < frame_skeletons[(int)frame_skeletons.size()-1][i].size() ; t++)
					{
					
						total+=fabs((double)frame_skeletons[(int)frame_skeletons.size()-1][i][t].first.second -frame_skeletons[(int)frame_skeletons.size()-2][j][t].first.second );
						total+=fabs((double)frame_skeletons[(int)frame_skeletons.size()-1][i][t].second.first -frame_skeletons[(int)frame_skeletons.size()-2][j][t].second.first);
						total+=fabs((double)frame_skeletons[(int)frame_skeletons.size()-1][i][t].second.second -frame_skeletons[(int)frame_skeletons.size()-2][j][t].second.second );
						
					}
				if(total/(((int)frame_skeletons[(int)frame_skeletons.size()-1][i].size())*3)<100)
				{
					back_frame_pertinent_skelton[(int)back_frame_pertinent_skelton.size()-1][i]=j;

				}

					total=0;
				}
			}
		}

		if(frame_skeletons.size()>2000)
		{
			frame_skeletons.erase(frame_skeletons.begin());
			back_frame_pertinent_skelton.erase(back_frame_pertinent_skelton.begin());
		}
	}

	//3�����|�C���g�N���E�h�̂��߂̍��W�ϊ�
	cv::Mat &pointCloud_XYZ_O(){return pointCloud_3D;}

	//���̑���
    inline unsigned char get_Object_label(const int t,const int x,const int y){return Object_label[t].objectT(x,y);}
	inline void set_Object_label(const int t,const int x,const int y,unsigned char d){Object_label[t].objectT(x,y)=d;}
	inline void Olabel_erace(int t){Object_label.erase(Object_label.begin()+t);}
	inline void push_Olabel(BMP a){Object_label.push_back(a);}


	//�w�i�X�V
	inline unsigned char get_mask(const int x,const int y){return mask_back.data[y*mask_back.cols+x]; }
	inline void set_mask(const int x,const int y,unsigned char d){mask_back.data[y*mask_back.cols+x]=d;}
	void out_mask(){cv::imshow("mask",mask_back);}


	//���́E�l�����̈�
	inline uchar get_Sub_label(const int x,const int y){ return label_S.data[y*label_S.cols+x];}
	void set_Sub_label(const int x,const int y,uchar d){label_S.data[y*label_S.cols+x]=d;}

	//���̌��m�\�̈�̐ݒ�
	void copy_obj_mask_area(cv::Mat a){a.copyTo(object_mask_area);/*cv::imshow("obj_mask",object_mask_area);*/}
	void obj_mask_areaOrig(cv::Mat &a){object_mask_area.copyTo(a);}
	inline unsigned char get_obj_mask(const int x,const int y){return object_mask_area.data[object_mask_area.step*y+object_mask_area.elemSize()*x+0]; }
	inline void set_obj_mask(const int x,const int y,unsigned char d){object_mask_area.data[object_mask_area.step*y+object_mask_area.elemSize()*x+0]=d;}

	//�摜�̍���,���̐ݒ�
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

	//���m������͈͂̐ݒ�
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

	//���̌��m������͈͂̐ݒ�
	void OsetArea(int sw,int sh,int ew,int eh)
	{
		if(eh > imageHeight) eh = imageHeight - 2;
		if(ew > imageWidth)  ew = imageWidth - 2;
		OArea.s_w = sw;
		OArea.s_h = sh;
		OArea.e_w = ew;
		OArea.e_h = eh;
	}

	int OArea_s_w(){ return OArea.s_w; }
	int OArea_s_h(){ return OArea.s_h; }
	int OArea_e_w(){ return OArea.e_w; }
	int OArea_e_h(){ return OArea.e_h; }

	//�l�����m����͈͂̐ݒ�
	void HsetArea(int sw,int sh,int ew,int eh)
	{
		if(eh > imageHeight) eh = imageHeight - 2;
		if(ew > imageWidth)  ew = imageWidth - 2;
		HArea.s_w = sw;
		HArea.s_h = sh;
		HArea.e_w = ew;
		HArea.e_h = eh;
	}

	int HArea_s_w(){ return HArea.s_w; }
	int HArea_s_h(){ return HArea.s_h; }
	int HArea_e_w(){ return HArea.e_w; }
	int HArea_e_h(){ return HArea.e_h; }

	//�w�i�����ɂ�郉�x��
	int get_STable_s_w(int num){ return STable[num].x1; }
	int get_STable_e_w(int num){ return STable[num].x2; }
	int get_STable_s_h(int num){ return STable[num].y1; }
	int get_STable_e_h(int num){ return STable[num].y2; }
	int get_STable_cx(int num){ return STable[num].xcenter; }
	int get_STable_cy(int num){ return STable[num].ycenter; }
	long int get_STable_size(int num){ return STable[num].size;}
	void set_STable_s_w(int num,int d){ STable[num].x1=d; }
	void set_STable_e_w(int num,int d){ STable[num].x2=d; }
	void set_STable_s_h(int num,int d){ STable[num].y1=d; }
	void set_STable_e_h(int num,int d){ STable[num].y2=d; }
	void set_STable_cx(int num,int d){  STable[num].xcenter=d; }
	void set_STable_cy(int num,int d){  STable[num].ycenter=d; }
	void set_STable_size(int num,long int d){STable[num].size=d;}
	void LabelMAX(int num){numlabel=num;}
	int LabelData(){return numlabel;}
	//STable:���x���̏�����
	void STableReset(){delete [] STable; STable=NULL;STable = new SLABELDATATABLE [1000];}





	//�l�����̈���
	int get_HumanTab_s_w(int num){ return human_id[num].x1; }
	int get_HumanTab_e_w(int num){ return human_id[num].x2; }
	int get_HumanTab_s_h(int num){ return human_id[num].y1; }
	int get_HumanTab_e_h(int num){ return human_id[num].y2; }
	int get_HumanTab_cx(int num){ return human_id[num].xcenter; }
	int get_HumanTab_cy(int num){ return human_id[num].ycenter; }
	long int get_HumanTab_size(int num){ return human_id[num].size;}
	int get_HumanTab_ID(int num){ return human_id[num].id; }
	int get_HumanTab_frame(int num){ return human_id[num].frame; }
	int get_HumanTab_skelton(int num){ return human_id[num].skelton; }
	int get_HumanTab_kinect(int num){ return human_id[num].kinect_id; }
	void set_HumanTab_s_w(int num,int d){human_id[num].x1=d;}
	void set_HumanTab_e_w(int num,int d){human_id[num].x2=d;}
	void set_HumanTab_s_h(int num,int d){human_id[num].y1=d;}
	void set_HumanTab_e_h(int num,int d){human_id[num].y2=d;}
	void set_HumanTab_cx(int num,int d){human_id[num].xcenter=d;}
	void set_HumanTab_cy(int num,int d){human_id[num].ycenter=d;}
	void set_HumanTab_size(int num,long int d){human_id[num].size=d;}
	void set_HumanTab_ID(int num,int d){human_id[num].id=d;}
	void set_HumanTab_frame(int num, int d){ human_id[num].frame = d; }
	void set_HumanTab_skelton( int indexHuman, int t, int ske_id )//ske_id�l��ID //num+kinect_num,MAX,input.get_human_id(i,j)
	{
		for(int indexSkeleton = 0;indexSkeleton < set_skeltons().size(); indexSkeleton++)
		{
			int x,y;
			int indexBody = 0;
			for(int indexJoint=0; indexJoint<set_skeltons()[indexSkeleton].size();indexJoint++)
			{
				x=set_skeltons()[indexSkeleton][indexJoint].first;
				y=set_skeltons()[indexSkeleton][indexJoint].second;
				if(get_human_id(x,y)>0)
				{
					indexBody = get_human_id( x, y );
					break;
				}
			}
			if( indexBody > 0 && indexBody == ske_id )
			{
				human_id[ indexHuman ].skelton = indexSkeleton;
				break;
			}
			else 
			{
				human_id[ indexHuman ].skelton = indexSkeleton;
			}
		}
		if(ske_id == -1)
		{
			human_id[ indexHuman ].skelton = -1;
		}
	}
	void set_HumanTab_kinect(int num,int i){human_id[num].kinect_id=i;}
	void HumantabMAX(int num){numhuman=num;}
	int HumanData(){return numhuman;}
	//human_id�l���f�[�^������
	void HumanTabReset(){delete [] human_id; human_id=NULL;human_id = new HUMANDATA [100];}
	
	inline uchar get_human_map_data(const int x,const int y){return human_map_data.data[human_map_data.step*y+human_map_data.elemSize()*x];}
	inline void set_human_map_data(const int x,const int y,unsigned char d){human_map_data.data[y*human_map_data.cols+human_map_data.elemSize()*x]=d;}	
	inline void copy_human_map_data(){Human_label.copyTo(human_map_data);}


//�l���ǐՏ��
	void set_t_human_id(int num)
	{   
		T_HUMAN_ID TAB=
		{
		get_HumanTab_s_w(num),get_HumanTab_e_w(num),get_HumanTab_s_h(num),get_HumanTab_e_h(num),//x1x2y1y2
		get_HumanTab_cx(num),get_HumanTab_cy(num), //xcyc
		num,//�}�b�v�ɏ�����Ă���l
		NULL,
		get_HumanTab_kinect(num),
		//get_HumanTab_skelton(num),//���i�f�[�^
		NULL,
		get_HumanTab_frame(num),//���t���[���O�ɂ��邩
		get_HumanTab_frame(num),//���t���[���O�ɂ��邩
		//�ǂ̕��̂������Ă��邩
		//NULL,NULL,//���ƐڐG���Ă��邩
		};
		//if(get_HumanTab_skelton(num)!=-1)TAB.track_skelton=track_skelton[get_HumanTab_skelton(num)];
		t_human_id[size_t_human_id()]=(TAB);
		if(get_HumanTab_skelton(num)!=-1)
		{
			t_human_id[size_t_human_id()].skeletons=skeletons[get_HumanTab_skelton(num)];
		}
		cout << size_t_human_id() << endl; 
		size_t_human_id_MAX(size_t_human_id()+1);
	}
	T_HUMAN_ID get_t_human_id(int num){return t_human_id[num];}
	void set_t_human_id_id(int num,int t){t_human_id[num].id=t;}
	void set_t_human_id_frame(int num,int frame){t_human_id[num].n_frame=frame;}
	void set_t_human_area(int t,int num)//�X�V
	{
		t_human_id[t].x1=get_HumanTab_s_w(num);
		t_human_id[t].x2=get_HumanTab_e_w(num);
		t_human_id[t].y1=get_HumanTab_s_h(num);
		t_human_id[t].y2=get_HumanTab_e_h(num);
		t_human_id[t].xcenter=get_HumanTab_cx(num);//xcyc;
		t_human_id[t].ycenter=get_HumanTab_cy(num);
		t_human_id[t].map_id=num;
		t_human_id[t].n_frame=get_HumanTab_frame(num);
		t_human_id[t].kinect_id=get_HumanTab_kinect(num);
		if(get_HumanTab_skelton(num)!=-1){
			t_human_id[t].skeletons=skeletons[get_HumanTab_skelton(num)];
		}//���i�f�[�^
	}	
	void erace_t_human_id(int key)
	{
		T_HUMAN_ID TAB=
		{
		NULL,NULL,NULL,NULL,//x1x2y1y2
		NULL,NULL, //xcyc
		NULL,//�}�b�v�ɏ�����Ă���l
		NULL,
		NULL,
		//-1,//���i�f�[�^
		NULL,
		NULL,//���t���[���O�ɂ��邩
		NULL,//���t���[���O�ɂ��邩
		//�p���̗���
		//�ǂ̕��̂������Ă��邩
		//NULL,NULL,//���ƐڐG���Ă��邩
		};
		t_human_id[key].skeletons.clear();
		for(int i=key;i<size_t_human_id()-1;i++ )
		{
			t_human_id[i]=t_human_id[i+1];
		}
		for(int i=size_t_human_id()-1;i<10;i++)
		{
			t_human_id[i]=TAB;
		}
		size_t_human_id_MAX(size_t_human_id()-1);
	}
	int size_t_human_id(){return t_numhuman;}
	void size_t_human_id_MAX(int num){t_numhuman=num;}

//�l���p�����m
	void set_t_human_id_pose(int num,int t,int frame){return t_human_id[num].pose.push_back(PII(t,frame));}
	int get_t_human_id_pose(int num,int t){return t_human_id[num].pose[t].first;}
	int get_t_human_id_pose_match(int num,int t){
		for(int i=0;i<get_t_human_id_pose_size(num);i++)
		{
			if(get_t_human_id_pose(num,i)==t)return i; 
		
		}
		return -1;
	}
	int get_t_human_id_pose_frame(int num,int t){return t_human_id[num].pose[t].second;}
	int get_t_human_id_pose_size(int num){return t_human_id[num].pose.size();}
	void erase_t_human_id_pose(int num,int t){t_human_id[num].pose.erase(t_human_id[num].pose.begin()+t);}
	void erase_t_human_id_pose_clear(int num){t_human_id[num].pose.clear();}

//�l���c�����	
	int set_t_human_id_have_obj(int num,int c)//num�ڐG�����l�� ���̃��f��ID
	{
		for(int t=0;t<size_t_human_id();t++)
		{
			if(num==t_human_id[t].id)
			{
				// t_human_id[t].have_obj.push_back(PII(get_t_object_id(c).object_id, get_t_object_id(c).size));
				t_human_id[t].have_obj.push_back(PII(get_t_object_id(c).model_id,get_t_object_id(c).size));
				t_human_id[t].object_ID.push_back(get_t_object_id(c).object_id);
				t_human_id[t].model_ID.push_back(get_t_object_id(c).model_id);
				for(int i=0;i<get_t_object_id(c).hide_obj.size();i++)//���̕��̂�����Ă��镨���ǉ��œ����
				{
					t_human_id[t].have_obj.push_back(get_t_object_id(c).hide_obj[i]);
				}
				return 1;
			}
		}
		return 0;
	}
	int get_t_human_id_have_obj(int num,int t){return t_human_id[num].have_obj[t].first;}
	int get_t_human_id_have_obj_data(int num,int t){return t_human_id[num].have_obj[t].second;}
	int get_t_human_id_have_obj_size(int num){return t_human_id[num].have_obj.size();}
	void erase_t_human_id_have_obj(int num,int t){t_human_id[num].have_obj.erase(t_human_id[num].have_obj.begin()+t);}
	void erase_t_human_id_have_obj_clear(int num){t_human_id[num].have_obj.clear();t_human_id[num].pose.clear();}
	

	//���̌��̈���
	int get_ObjectTab_s_w(int num){ return OTable[num].x1; }
	int get_ObjectTab_e_w(int num){ return OTable[num].x2; }
	int get_ObjectTab_s_h(int num){ return OTable[num].y1; }
	int get_ObjectTab_e_h(int num){ return OTable[num].y2; }
	int get_ObjectTab_cx(int num){ return OTable[num].xcenter; }
	int get_ObjectTab_cy(int num){ return OTable[num].ycenter; }
	long int get_ObjectTab_size(int num){ return OTable[num].size;}
	int get_ObjectTab_count(int num){ return OTable[num].count; }
	int get_ObjectTab_human(int num){ return OTable[num].human_id; }
	void set_ObjectTab_s_w(int num,int d){OTable[num].x1=d;}
	void set_ObjectTab_e_w(int num,int d){OTable[num].x2=d;}
	void set_ObjectTab_s_h(int num,int d){OTable[num].y1=d;}
	void set_ObjectTab_e_h(int num,int d){OTable[num].y2=d;}
	void set_ObjectTab_cx(int num,int d){OTable[num].xcenter=d;}
	void set_ObjectTab_cy(int num,int d){OTable[num].ycenter=d;}
	void set_ObjectTab_size(int num,long int d){OTable[num].size=d;}
	void set_ObjectTab_count(int num,int d){OTable[num].count=d;}
	void set_ObjectTab_human(int num,int d){OTable[num].human_id=d;}
	void ObjecttabMAX(int num){numobject=num;}
	int ObjectData(){return numobject;}
	//void pushObjimage(int num,cv::Mat a){OTable[num].Oimage.push_back(a);}
	std::vector<cv::Mat> &ObjImage(int num){return OTable[num].Oimage;}
	void eraceObjimage(int a){OTable[a].Oimage.clear();}
	cv::Mat &Obj_Img(int a,int b){return OTable[a].Oimage[b];}
	

	//���̏��
	inline float get_object_map_data(const int x,const int y){return object_map_data.at<float>(y,x);}
	inline void set_object_map_data(const int x,const int y,float d){object_map_data.at<float>(y,x)=d;}	
	inline void t_set_object_map_data(cv::Mat a){a.copyTo(object_map_data);}
	void set_t_object_id(int num)
	{
		T_OBJECT_ID TAB={
		get_Event_s_w(num),get_Event_e_w(num),get_Event_s_h(num),get_Event_e_h(num),//x1x2y1y2
		get_Event_cx(num),get_Event_cy(num),//xcyc
		get_ObjectTab_size(Event_Log( num)),
		get_Event_model(num),//�����̃��f��ID(���t���[��ID)
		get_Event_obj(num),//����ID
		num,//map�ɏ�����Ă���ID
		get_Event_frame(num),//���t���[���ڂɓ�������
		NULL,
		NULL,//�Ō�ɐڐG�����l��
		NULL,//�Ō�ɐڐG�����t���[����
		cv::Mat(cv::Size(imageWidth, imageHeight), CV_32FC1, cv::Scalar(DEPTH_RANGE_MAX)),//�E�ݗ̈�
		0//�E�݂̖ʐ�
		};
		t_object_id.push_back(TAB);
	}
	void set_object_data(int x,int y,float d)
	{
		bring_object_data.at<float>(y,x)=d;
	}
	inline float get_object_data(int x,int y)
	{
		float* data = &bring_object_data.at<float>(y,x);
		return  data[0];
	}

	T_OBJECT_ID get_t_object_id(int num){return t_object_id[num];}
	void set_t_object_id_object_id(int num,int t){t_object_id[num].object_id=t;}
	void set_t_object_id_touch(int num,int t,int n){t_object_id[num].touch=t;t_object_id[num].t_frame=n;}
	void set_t_object_id_human_id(int num,int a,int frame){ t_object_id[num].human_id=a;t_object_id[num].frame=frame;}
	void erace_t_object_id(int key)
	{
		t_object_id.erase(t_object_id.begin()+key);
	}
	int size_t_object_id(){return t_object_id.size();}


	cv::Mat &T_object_denta_map(int num){return t_object_id[num].denta;}
	inline float get_T_object_denta_map(int num,const int x,const int y){return t_object_id[num].denta.at<float>(y,x);}
	inline void set_T_object_denta_map(int num,const int x,const int y,float d){t_object_id[num].denta.at<float>(y,x)=d;}	
	inline float get_T_object_denta_size(int num){return t_object_id[num].denta_size;}
	inline void set_T_object_denta_size(int num,float d){t_object_id[num].denta_size=d;}	
	inline void set_T_object_hide_data(int num,int d)
	{
		for(int i=0;i<get_t_human_id_have_obj_size(d);i++)
		{
			t_object_id[num].hide_obj.push_back(PII(get_t_human_id_have_obj(d,i),get_t_human_id_have_obj_data(d,i)));
		}
	}
	
	//�B�������̂̏���ǉ�
	void init_bring_object_data(){bring_object_data=cv::Mat(cv::Size(getwidth(), getheight()), CV_32FC1, cv::Scalar(DEPTH_RANGE_MAX));}
	cv::Mat get_bring_object_data(){return bring_object_data;}

	//�C�x���g���m���
	void Eventset(int num)
	{
	 delete [] Event_ID; Event_ID=NULL; Event_ID = new  DEVENT[num];
	 delete [] EventDATA;EventDATA=NULL;EventDATA=new labelD[num];
	 Event_max=num;
	}

	int EventMAX(){ return Event_max;}
	void Eventreset(int e){Event_max=e;}

	int get_Event_key(int num){ return Event_ID[num].key_point_parent_id; }
	int get_Event_obj(int num){ return Event_ID[num].object_id; }
	int get_Event_Eve(int num){ return Event_ID[num].event_id; }
	int get_Event_model(int num){ return Event_ID[num].model_id; }
	int get_Event_frame(int num){ return Event_ID[num].frame; }
	int get_Event_human(int num){ return Event_ID[num].human_id; }
	void set_Event_key(int num,int d){Event_ID[num].key_point_parent_id=d;}
	void set_Event_obj(int num,int d){Event_ID[num].object_id=d;}
	void set_Event_Eve(int num,int d){Event_ID[num].event_id=d;}
	void set_Event_model(int num,int d){Event_ID[num].model_id=d;}
	void set_Event_frame(int num,int d){Event_ID[num].frame=d;}
	void set_Event_human(int num,int d){Event_ID[num].human_id=d;}


	int get_Event_s_w(int num){ return EventDATA[num].x1; }
	int get_Event_e_w(int num){ return EventDATA[num].x2; }
	int get_Event_s_h(int num){ return EventDATA[num].y1; }
	int get_Event_e_h(int num){ return EventDATA[num].y2; }
	int get_Event_cy(int num){return EventDATA[num].cy;}
	int get_Event_cx(int num){return EventDATA[num].cx;}
	int get_Event_Aframe(int num){ return EventDATA[num].frame; }
	void set_Event_s_w(int num,int d){EventDATA[num].x1=d;}
	void set_Event_e_w(int num,int d){EventDATA[num].x2=d;}
	void set_Event_s_h(int num,int d){EventDATA[num].y1=d;}
    void set_Event_e_h(int num,int d){EventDATA[num].y2=d;}
	void set_Event_cy(int num,int d){EventDATA[num].cy=d;}
	void set_Event_cx(int num,int d){EventDATA[num].cx=d;}
	void set_Event_Aframe(int num,int d){EventDATA[num].frame=d;}


	void Event_push(int num){event_log.push_back(num);}
	void Event_releace(){event_log.clear();}
	int Event_Log(int num){return event_log[num];}
	int Event_size(){return event_log.size();}

	event_state get_n_event_state(){return n_event_state;}
	void set_n_event_state_L5_t(PII a){n_event_state.L5_t.push_back(a);}
	void set_n_event_state_L2_h(PII a){n_event_state.L3_h.push_back(a);}
	void set_n_event_state_L3(PII a){n_event_state.L4.push_back(a);}
	void set_n_event_state_L5(PII a,PII n){n_event_state.L6.push_back(a);n_event_state.L6_o.push_back(n);}

	void erase_n_event_state()
	{
		n_event_state.L3_h.clear();
		n_event_state.L4.clear();
		n_event_state.L5_t.clear();
		n_event_state.L6.clear();
		n_event_state.L6_o.clear();
	}

};