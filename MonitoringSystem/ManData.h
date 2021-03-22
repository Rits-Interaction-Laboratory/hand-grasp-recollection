#include <opencv/cv.h>
#include <opencv/highgui.h>


//#pragma onece

class CANDIDATE1{

private:

public:
	CANDIDATE1(){
		init();
	}
	~CANDIDATE1(){
	}
	int id;
	int conf;

	void init();
};


class MAN_DATA{

private:

public:
	MAN_DATA();
	~MAN_DATA();

	int data_set_flag;
	int body_renew_flag;
	int face_renew_flag;
	int Recog_success_flag;
	int master;

	IplImage *FaceImage;
	POINT ptFaceRightTop1, ptFaceLeftBottom1, ptFaceLeftTop1, ptFaceRightBottom1; 
	POINT ptBodyRightTop1, ptBodyLeftBottom1, ptBodyLeftTop1, ptBodyRightBottom1;
	std::vector<CANDIDATE1 > candidate;

	int update_interval;
	int dimension;
	int ManArea;
	int ***Hist_3D;   //ƒqƒXƒg“Š•[—p


	int reset();
	void init();
};



