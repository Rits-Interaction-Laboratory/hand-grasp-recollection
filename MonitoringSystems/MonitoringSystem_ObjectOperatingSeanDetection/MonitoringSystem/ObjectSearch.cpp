#include "stdafx.h"
#include "image.h"
//#include "scenestate.h"
#include "network.h"
#include "Objectsearch.h"

#include <queue>
#include <map>
#include <vector>
#include <fstream>
#include "FileOperator.h"
#include <sstream>
#include <algorithm>
#include <time.h>

int Object_min=50;//���̂̃T�C�Y
int Object_max=1500;

//
//--------------------------------------------------
// Name     :�@ObjectSearch()
// Author�@ : ��{
// Update   : 2013.8.20
// Function : ���ȏ㕨�̂������ꏊ�ɂ����ꍇ���m����
// Argument : image�N���X�Gunsigned long frame�@�t���[����
// return   : Objectflag���̌��m�����Βl��Ԃ�
//--------------------------------------------------
bool Object::ObjectSearch(unsigned long frame ,image &input)
{
	bool Objectflag=false;
	//object_img.clear();
	obj_img.reserve(50);//�f�[�^�̈�m��
	cv::Mat img;
	img=cv::Mat( cvSize(320,240),CV_8UC3);
	//�o�͉摜��img�ɑ��
	for(int j=0;j<input.Height();j++)
	{
		for(int i=0;i<input.Width();i++)
		{
		img.data[j*img.step+i*img.elemSize()+ 0]=input.get_Out_B(i,j);
		img.data[j*img.step+i*img.elemSize()+ 1]=input.get_Out_G(i,j);
		img.data[j*img.step+i*img.elemSize()+ 2]=input.get_Out_R(i,j);
		}
	}

	//�O��C�x���g���m���ꂽ���̂���
	//int num_max=input.ObjectData();
	for(int num=1;num<=input.ObjectData();num++)
	{
		if(input.get_ObjectTab_count(num)>Object_count)
		{
			eraseImage(input,num);//���̌����ɑO�t���[���̕��̂��Ȃ��ꍇ�폜
			num--;
		}
	}


	max_img=0;

    for(int i=0;i<input.LabelData();i++)	//checkpoint
	{//�O������蓾�����x�����畨�̌��𒊏o
		//cv::rectangle(input.dOut(),cv::Rect(input.get_STable_s_w(i),input.get_STable_s_h(i),input.get_STable_e_w(i)-input.get_STable_s_w(i),input.get_STable_e_h(i)-input.get_STable_s_h(i)),cv::Scalar(0,0,255));  
		if(Object_min<input.get_STable_size(i) &&Object_max>input.get_STable_size(i) )
		{
			//if(input.get_STable_s_w(i)>20&&input.get_STable_s_h(i)>50 &&input.get_STable_e_w(i)<260 &&input.get_STable_e_h(i)<240)
			int cx=input.get_STable_cx(i);
			int cy=input.get_STable_cy(i);
			if(input.get_obj_mask(cx,cy)!=255)
			{
				push_img(input,i);
			}
		}
	}
	

///----------------------�O�t���[���ɓ������̂����邩�ǂ���

	ObjectMatch(input,img);

///-------------------------------------------------------

	obj_img.clear();

	for(int num=1;num<=input.ObjectData();num++)
	{
		if(input.get_ObjectTab_count(num)>Object_count)
		{
			input.Event_push(num);
			Objectflag=true;
		}
	}




	return Objectflag;
}

void Object::ObjectMatch(image &input,cv::Mat img)
{
double Objectsize=0;
int Obj_flag=false;
double totalObjectsize=0;
double ratio; 

cv::Mat objectimg;

//cv::Mat test3=cv::Mat(cvSize(320,240),CV_8UC1,cv::Scalar(0));
img.copyTo(objectimg);
//���m���ꂽ���̂��ȑO�����������̔�r

//cout<<input.ObjectData()<<endl;
//cout<<max_img<<endl;




for(int t=1;t<=input.ObjectData();t++)
{ 
	for(int x=0;x<max_img;x++)
	{
		Objectsize=0;//������
		totalObjectsize=0;
		ratio=0;
		//bool areaflag=false;

		//test3=cv::Mat(cvSize(320,240),CV_8UC1,cv::Scalar(0));

		for(int j=std::min(input.get_ObjectTab_s_h(t),obj_img[x].y1);j<=std::max(input.get_ObjectTab_e_h(t),obj_img[x].y2);j++)
		{
			for(int i=std::min(input.get_ObjectTab_s_w(t),obj_img[x].x1);i<=std::max(input.get_ObjectTab_e_w(t),obj_img[x].x2);i++)
			{
				if( input.get_Object_label(t,i,j) == t && obj_img[x].objectT(i,j) == 255)//���x����`����Ă���ꏊ��\��
				{
					Objectsize++;
				}
				if( input.get_Object_label(t,i,j) == t || obj_img[x].objectT(i,j) == 255)
				{
					totalObjectsize++;
				}
				/*if(input.Olabel(i,j) == t){test3.at<uchar>(j,i)=255;}*/
				if(obj_img[x].objectT(i,j) == (uchar)255){objectimg.data[j*objectimg.step+i*objectimg.elemSize()+1]=(char)255;}
			}
		}

		ratio=Objectsize/totalObjectsize;

		cv::waitKey(1);
		

		if(0.5<ratio)
		{

			cv::Rect objrect;
			objrect.x = cvRound(input.get_ObjectTab_s_w(t));
			objrect.width = cvRound(input.get_ObjectTab_e_w(t)-input.get_ObjectTab_s_w(t));
			objrect.y = cvRound(input.get_ObjectTab_s_h(t));
			objrect.height = cvRound(input.get_ObjectTab_e_h(t)-input.get_ObjectTab_s_h(t));

			
			cv::rectangle(objectimg, objrect.tl(), objrect.br(), cv::Scalar(0,0,255),3);//

			//input.ObjImage(t).push_back(cv::Mat(objectimg));//�摜�f�[�^��ǉ�
			input.ObjImage(t).resize(input.get_ObjectTab_count(t)+1);
			//�摜�f�[�^��ǉ�
			cv::Mat(objectimg).copyTo(input.Obj_Img(t,(input.get_ObjectTab_count(t))));

			for(int j=std::min(input.get_ObjectTab_s_h(t),obj_img[x].y1);j<=std::max(input.get_ObjectTab_e_h(t),obj_img[x].y2);j++)
			{
				for(int i=std::min(input.get_ObjectTab_s_w(t),obj_img[x].x1);i<=std::max(input.get_ObjectTab_e_w(t),obj_img[x].x2);i++)
				{
					if( input.get_Object_label(t,i,j) != t && obj_img[x].objectT(i,j) == 255)
					{
						input.set_Object_label(t,i,j,t);	
					}
					else if( input.get_Object_label(t,i,j) == t && obj_img[x].objectT(i,j) != 255)
					{
						input.set_Object_label(t,i,j,0);
					}

				}
			}


			img.copyTo(objectimg);
			input.set_ObjectTab_s_w(t,obj_img[x].x1);
			input.set_ObjectTab_e_w(t,obj_img[x].x2);
			input.set_ObjectTab_s_h(t,obj_img[x].y1);
			input.set_ObjectTab_e_h(t,obj_img[x].y2);
			input.set_ObjectTab_cx(t,obj_img[x].cx);
			input.set_ObjectTab_cy(t,obj_img[x].cy);
			input.set_ObjectTab_size(t,obj_img[x].size);

			input.set_ObjectTab_count(t,input.get_ObjectTab_count(t)+1);		
			

			obj_img[x].flag=true;
			Obj_flag=true;

			x=max_img;//�T���I��
		}
		
	}


	if(Obj_flag==false)
	{
		eraseImage(input,t);//���̌����ɑO�t���[���̕��̂��Ȃ��ꍇ�폜
		t--;
	}

	Obj_flag=false;
}


//�V�K���̌��o�^
for(int x=0;x<(int)obj_img.size();x++)
{
	if(obj_img[x].flag==false)
	{

		cv::Rect objrect;
		objrect.x = cvRound(obj_img[x].x1);
		objrect.width = cvRound(obj_img[x].x2-obj_img[x].x1);
		objrect.y = cvRound(obj_img[x].y1);
		objrect.height = cvRound(obj_img[x].y2-obj_img[x].y1);


		cv::rectangle(objectimg, objrect.tl(), objrect.br(), cv::Scalar(0,0,255),3);

		
	    pushImage(input,x);//�V�������̂𓱓�
		input.ObjImage(input.ObjectData()).resize(1);
			//�摜�f�[�^��ǉ�
		cv::Mat(objectimg).copyTo(input.Obj_Img(input.ObjectData(),0));
		img.copyTo(objectimg);

		//�ڐG���
		int min_x=320,min_y=240,data=-1;
		int obj_max=input.ObjectData();
		for(int m=0;m<input.size_t_human_id();m++)//�߂��̎�̏������
		{
			int hx1,hx2,hy1,hy2,hcx;//�l���̗��[
			hx1=input.get_t_human_id(m).x1; 
			hx2=input.get_t_human_id(m).x2;
			hy1=input.get_t_human_id(m).y1;
			hy2=input.get_t_human_id(m).y2;
			hcx=input.get_t_human_id(m).xcenter;

			int min_diff=60000;
			int ox1,ox2,oy1,oy2,ocx,ocy;//���̗��[
			int area=70;
			ox1=max(0,obj_img[x].x1-area);
			ox2=min(320,obj_img[x].x2+area);
			oy1=max(0,obj_img[x].y1-area);
			oy2=min(240,obj_img[x].y2+area);
			ocx=obj_img[x].cx;
			ocy=obj_img[x].cy;
			//cout<<hx1<<":"<<hy1<<":"<<hx2<<":"<<hy2<<endl;
			//cout<<ox1<<":"<<oy1<<":"<<ox2<<":"<<oy2<<endl;
			if(input.get_t_human_id(m).skeletons.size()>0)
			{
				//int t=input.get_t_human_id(m).skelton;
				hx1=input.get_t_human_id(m).skeletons[22].first;
				hy1=input.get_t_human_id(m).skeletons[22].second;
				hx2=input.get_t_human_id(m).skeletons[24].first;
				hy2=input.get_t_human_id(m).skeletons[24].second;

				if((ox1<hx1&&hx1<ox2&&oy1<hy1&&hy1<oy2)||(ox1<hx2&&hx2<ox2&&oy1<hy2&&hy2<oy2))//���̂ɋ߂���̌��m
				{
					if(min(std::abs((abs(hx1-ocx)+abs(hy1-ocy))/2),std::abs((abs(hx2-ocx)+abs(hy2-ocy))/2))<min_diff)
					{input.set_ObjectTab_human(obj_max,input.get_t_human_id(m).id);data=m;
					min_diff=min(std::abs((abs(hx1-ocx)+abs(hy1-ocy))/2),std::abs((abs(hx2-ocx)+abs(hy2-ocy))/2));}
				}
			}
			else if(hcx<ocx)
			{
				area=50;
				ox1=max(0,obj_img[x].x1-area);
				ox2=min(320,obj_img[x].x2+area);
				oy1=max(0,obj_img[x].y1-area);
				oy2=min(240,obj_img[x].y2+area);
				if(ox1<hx2&&oy1<hy2&&hy1<oy2&&(std::abs(hcx-ocx)<min_diff))
				{
					input.set_ObjectTab_human(obj_max,input.get_t_human_id(m).id);data=m;
					min_diff=std::abs(hcx-ocx);
				}
			}
			else
			{
				area=50;
				ox1=max(0,obj_img[x].x1-area);
				ox2=min(320,obj_img[x].x2+area);
				oy1=max(0,obj_img[x].y1-area);
				oy2=min(240,obj_img[x].y2+area);
				if(hx1<ox2&&oy1<hy2&&hy1<oy2&&(std::abs(hcx-ocx)<min_diff))
				{
					input.set_ObjectTab_human(obj_max,input.get_t_human_id(m).id);data=m;
					min_diff=std::abs(hcx-ocx);
				}
			}
				
		}

		if(data!=-1)
		{
			/*cv::Rect human;
			human.x = cvRound(input.get_HumanTab_s_w(data));
			human.width = cvRound(input.get_HumanTab_e_w(data)-input.get_HumanTab_s_w(data));
			human.y = cvRound(input.get_HumanTab_s_h(data));
			human.height = cvRound(input.get_HumanTab_e_h(data)-input.get_HumanTab_s_h(data));
			cv::rectangle(objectimg, human.tl(), human.br(), cv::Scalar(0,0,255),3);	*/	
		}
		else
		{
			//eraseImage(input,input.ObjectData());//���̌����ɑO�t���[���̕��̂��Ȃ��ꍇ�폜
			input.set_ObjectTab_human(obj_max,-1);
		}
	}
}

}

void Object::eraseImage(image &input,int t)
{
	input.Olabel_erace(t);
	input.ObjecttabMAX(input.ObjectData()-1);
	for(int x=t;x<=input.ObjectData();x++)
	{
		sort_objectdata(input,x);
	

		for(int j=input.get_ObjectTab_s_h(x/*t */);j<=input.get_ObjectTab_e_h(x);j++)
		{
			for(int i=input.get_ObjectTab_s_w(x);i<=input.get_ObjectTab_e_w(x);i++)
			{
				if( input.get_Object_label(x,i,j)== x+1)
				{
					input.set_Object_label(x,i,j,x);	
				}
				else 
				{
					input.set_Object_label(x,i,j,0);
				}
			}
		}
	}
}

void Object::pushImage(image &input,int t)
{
	input.ObjecttabMAX(input.ObjectData()+1);

	BMP Olabel;

	for(int i=obj_img[t].x1;i<obj_img[t].x2;i++)
	{
		for(int j=obj_img[t].y1;j<obj_img[t].y2;j++)
		{
			if(obj_img[t].objectT(i,j)==255)
			{
				Olabel.SetPixColor(i,j,input.ObjectData());	
			}
			else
			{
				Olabel.SetPixColor(i,j,0);
			}
		}
	}

	input.push_Olabel(Olabel);
	
	push_objectdata(input,t);
}

