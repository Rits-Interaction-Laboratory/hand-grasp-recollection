/*#ifndef __TEST_H__
#define __TEST_H__
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "ManData.h"
#include "person.h"


//#pragma onece


class HUMAN_PROCESS{

private:

public:
	HUMAN_PROCESS();
	~HUMAN_PROCESS();

	MAN_DATA PreHumanData[10];
	MAN_DATA NowHumanData[10];


	int HumanRecognition(IplImage *img_original, IplImage *img_resize, IplImage *img_diff);
	int Labeling(IplImage *img_diff, IplImage *img_label);
	int HumanPositionSet(IplImage *img_label, IplImage *img_diff);
	int FacePositionSet(IplImage *img_original, IplImage *img_diff);
	int DetectFace(IplImage *iplImage, IplImage *img_diff);
	double ComparePosition(POINT LeftTop1, POINT RightBottom1, POINT LeftTop2, POINT RightBottom2);
	void MakeClothHist(IplImage *img_resize, IplImage *img_diff);
	void SaveClothHist();
	double CompareClothHist(int ***NowHist_3D, int ***PreHist_3D);
	int FaceRecognition(int person_num);
	int FaceArea(IplImage *img_original, IplImage *img_diff);

	void test(IplImage *img_diff);

};

#endif __TEST_H__


*/