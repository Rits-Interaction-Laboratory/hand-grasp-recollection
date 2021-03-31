#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <string.h>
#include <limits.h>
#include <vector>
#include <iostream>
#include "geometory.h"
#include <opencv2/opencv.hpp>
#include "image.h"

//#include "network.h"

#include "bmp.h"
class Object// : public image
{
private:
	//CMonitoringSystemDlg* pParent3;  //親ウィンドウのポインタ

public:
	Object(void){}
	~Object(void){}
	cv::Mat label;//=cv::Mat(cv::Size(imageWidth, imageHeight), CV_8UC1);
	//static layer lay; 
	int max_img;
	std::vector<BMP>obj_img;//kawa static
	static std::vector<int>object_Slabel;//過去のラベルと照合

	bool ObjectSearch(unsigned long frame ,image &input);

	BMP imagedata;

	void push_img(image &input,int x)
	{ 	
		//CvFont dfont;
		
		//obj_img.resize(max_img);
		 for(int j=0;j<240;j++)
			{
			for(int i=0;i<320;i++)
			{
				if(x==input.get_Sub_label(i,j))
				{imagedata.SetPixColor(i,j,255);input.set_mask(i,j,125);/*lavel.at<uchar>(j,i)=255;*/}
				else
				{imagedata. SetPixColor(i,j,0);}

			}
		}
		obj_img.push_back(imagedata);
	    set_obj_area(input,max_img,x);
		obj_img[max_img].flag=false;

		//stringstream ss;
		//ss<<max_img;
		//cv::putText(lavel, ss.str(),cv::Point(obj_img[x].x1,obj_img[x].y1) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(100), 2, CV_AA);
		//cv::rectangle(lavel,cv::Rect(obj_img[x].x1,obj_img[x].y1,obj_img[x].x2-obj_img[x].x1,obj_img[x].y2-obj_img[x].y1),cv::Scalar(255));

	max_img++;
	}

	void set_obj_area(image &input,int x,int i)
	{
		obj_img[x].x1=input.get_STable_s_w(i);
        obj_img[x].x2=input.get_STable_e_w(i);
        obj_img[x].y1=input.get_STable_s_h(i);
        obj_img[x].y2=input.get_STable_e_h(i);
		obj_img[x].cx=input.get_STable_cx(i);
		obj_img[x].cy=input.get_STable_cy(i);
        obj_img[x].size=input.get_STable_size(i);
        
        
	}

	void sort_objectdata(image &input,int i)
	{
		input.set_ObjectTab_s_w(i,input.get_ObjectTab_s_w(i+1));
		input.set_ObjectTab_e_w(i,input.get_ObjectTab_e_w(i+1));
		input.set_ObjectTab_s_h(i,input.get_ObjectTab_s_h(i+1));
		input.set_ObjectTab_e_h(i,input.get_ObjectTab_e_h(i+1));
		input.set_ObjectTab_cx(i,input.get_ObjectTab_cx(i+1));
		input.set_ObjectTab_cy(i,input.get_ObjectTab_cy(i+1));
		input.set_ObjectTab_size(i,input.get_ObjectTab_size(i+1));
		input.set_ObjectTab_human(i,input.get_ObjectTab_human(i+1));
		input.eraceObjimage(i);
		input.ObjImage(i)=input.ObjImage(i+1);
		input.set_ObjectTab_count(i,cv::max(0,input.get_ObjectTab_count(i+1)));
		//cout<<input.ObjectTab_count(i)<<endl;
	}

	void push_objectdata(image &input,int i)
	{
		int max=input.ObjectData();
		input.set_ObjectTab_s_w(max,obj_img[i].x1);
        input.set_ObjectTab_e_w(max,obj_img[i].x2);
        input.set_ObjectTab_s_h(max,obj_img[i].y1);
        input.set_ObjectTab_e_h(max,obj_img[i].y2);
		input.set_ObjectTab_cx(max,obj_img[i].cx);
		input.set_ObjectTab_cy(max,obj_img[i].cy);
        input.set_ObjectTab_size(max,obj_img[i].size);
		input.set_ObjectTab_count(max,0);

	}

	void ObjectMatch(image &input,cv::Mat img);
	void eraseImage(image &input,int t);
	void pushImage(image &input,int t);
};

