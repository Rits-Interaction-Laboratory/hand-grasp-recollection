#pragma once

#include "bmp.h"
#include "Pair.h"
#include <iostream>
#include <vector>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>    //BruteForceMatcheに必要。opencv2.4で移動した？
#include <map>
#include <string>
#include <fstream>
#include <set>
using namespace std;

class SurfPoint
{
	//解釈クラス
	//Cinterpretation intpre;
public:
	vector<int> ptpairs;  // keypointsのインデックスが2つずつ組になるように格納
	vector<Pair*> pair;
	//vector<Pair*> tracker;
	vector<int> ptpairs2;
	vector<int> entry_pairs;  // keypointsのインデックスを格納
	std::set<int> nums;

	int Registered;

	bool wflag;
	
	bool new_obj;

	SurfPoint(void);
	~SurfPoint(void);
	int Pointing(unsigned long frame,image &input, int delete_key, std::vector<int> key_idname,std::vector<int>obj_idname,int num);
	int SurfMatching(std::vector<cv::KeyPoint> imageKeypoints, std::vector<float> imageDescriptors,unsigned long frame,image &input,CvRect region ,int delete_key,std::vector<int> key_idname,std::vector<int> obj_idname,int num);
	void saveFile(int objId, std::vector<cv::KeyPoint> imageKeypoints1, std::vector<float> imageDescriptors1, IplImage* output, vector<int> entry_pairs, CvRect region, map<int, int> id2name, unsigned long frame, bool new_obj, bool file,std::vector<int> key_idname,std::vector<int> obj_idname);
	int searchNN(float *vec, int lap, std::vector<int> &labels, std::vector<int> &laplacians, CvMat* objMat, CvSURFPoint *p);
	bool loadKeypointId(map<int, int>& id2name/*, std::vector<int>& labels*/);
	bool loadDescription(CvMemStorage* storage, CvSeq* point_seq, vector<int> &labels, std::vector<int> &laplacians, CvMat* &objMat, map<int, int>& id2name);
	double affine(std::vector<Pair*> pair/*, IplImage* compari*/, cv::Mat output, /*IplImage* matchingImage,*/ CvScalar color,int object,int vote
							 , std::vector<int>& box_votes, std::vector<cv::KeyPoint> keypoints1/*, std::vector<Pair*>& entry*/, int flag);
	double Ransac(const std::vector<Pair*>& pair,/*const std::vector<Pair*>& affine_pair ,*/ CvMat *affineMatrix/*, int count*//*, IplImage *output*/,int Id);
	double euclidDistance(float* vec1, float* vec2, int length);
	int nearestNeighbor(float *mvec, int lap,const std::vector<int>& laplacians, CvMat* objMat);
	void findPairs(std::vector<cv::KeyPoint> keypoints1, std::vector<float> descriptors1, std::vector<int> &labels, vector<int> &laplacians, CvMat* objMat, std::vector<int>& ptpairs, int maxVal);
	int Votes_Registered();
	};
