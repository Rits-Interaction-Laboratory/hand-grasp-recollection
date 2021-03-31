#include "stdafx.h"
//#include "scenestate.h"
#include "network.h"
#include "imageEdit.h"
#include "Humanprocess.h"
#include "Humansearch.h"

#include <queue>
#include <map>
#include <vector>
#include <fstream>
#include "FileOperator.h"
#include <sstream>
#include <algorithm>
#include <time.h>

int humansize_min=1600;//�l���̈�
int humansize_max=30000;
int human_max=6;//�ő匟�m��
//--------------------------------------------------
// Name     : HumanSearch
// Author�@ : kawamoto(cv_)
// Update   : 2013.3.30
// Function : �l���F��
// Argument : int�^�̃t���[����,image�N���X
// return   : int�^�̕ϐ�
//--------------------------------------------------
bool human::HumanSearch(unsigned long frame,image &input)
{

	Hset();//�l��������


	static bool Humanflag=false;//�O�̃t���[��
	//std::map<int,int>human_id_data;
	int kinect_num=0;
	int num=0;

	input.HumanTabReset();//������
	input.Human_label_clear();

	/*
	// �X�P���g�����݂̂�M�p���邽�߃R�����g�A�E�g(2016/11/21 ����)
	for(int i=0;i<input.LabelData();i++)
	{
		if(humansize_min<input.get_STable_size(i)&&input.get_STable_size(i)<humansize_max&&input.HumanData()<human_max)
		{   
			num++;
			setArea(num,input,i,frame);//�l���F���p

			input.set_HumanTab_s_w(num,input.get_STable_s_w(i));//�l���̈���f�[�^�ŕۑ�����B
			input.set_HumanTab_e_w(num,input.get_STable_e_w(i));
			input.set_HumanTab_s_h(num,input.get_STable_s_h(i));
			input.set_HumanTab_e_h(num,input.get_STable_e_h(i));
			input.set_HumanTab_size(num,input.get_STable_size(i));
			input.set_HumanTab_cx(num,input.get_STable_cx(i));
			input.set_HumanTab_cy(num,input.get_STable_cy(i));
			input.set_HumanTab_ID(num,i);//����l���F��   *���̂����쐬����\��
			input.set_HumanTab_frame(num,(int)frame);//
			input.set_HumanTab_skelton(num,-1,-1);
			input.set_HumanTab_kinect(num, -1);
			//human_id_data.insert(std::pair<int,int>(num,i));
			Humanflag=true;
		}
	}
	for(int j=0;j<input.Height();j++)
	{
		for(int i=0;i<input.Width();i++)
		{
			for(int t=1;t<=num;t++)
			{
				if(HTable_ID(t) == input.get_Sub_label(i,j)) 
				{
					input.set_Human_label(i,j,t);//
					input.set_mask(i,j,0);
				}
				else{input.set_Human_label(i,j,0);}
			}

		}
	}
	*/


	if(input.set_skeltons().size())
	{

		int MAX=0;
		bool human_flag=false;
		std::map<int,int>id_data;
		id_data.clear();
		for(int j=0;j<input.Height();j++){
			for(int i=0;i<input.Width();i++){
				if(input.get_human_id(i,j)>0)
				{
					human_flag=false;
					for(int t=0;t<MAX;t++)
					{
						if(input.get_human_id(i,j)==id_data[t])//�o�^���ꂽ���̂̈ꕔ
						{
							if(input.get_HumanTab_e_w(t+num+1)<i)input.set_HumanTab_e_w(t+num+1,i);
							if(input.get_HumanTab_s_w(t+num+1)>i)input.set_HumanTab_s_w(t+num+1,i);
							if(input.get_HumanTab_e_h(t+num+1)<j)input.set_HumanTab_e_h(t+num+1,j);
							//if(input.get_HumanTab_s_h(t+num+1)<j)input.set_HumanTab_s_h(t+num+1,j);
							input.set_HumanTab_size(t+num+1,(input.get_HumanTab_size(t+num+1))+1);
							input.set_HumanTab_cx(t+num+1,input.get_HumanTab_cx(t+num+1)+i);
							input.set_HumanTab_cy(t+num+1,input.get_HumanTab_cy(t+num+1)+j);
							input.set_Human_label(i,j,t+num+1);//
							input.set_mask(i,j,0);
							t=MAX+1;
							human_flag=true;
						}
					}
					if(!human_flag&&(num+MAX)<human_max)//�V�K����
					{	
						kinect_num++;
						input.set_HumanTab_s_w(num+kinect_num,i);
						input.set_HumanTab_s_h(num+kinect_num,j);
						input.set_HumanTab_e_w(num+kinect_num,i);
						input.set_HumanTab_e_h(num+kinect_num,j);
						input.set_HumanTab_size(num+kinect_num,1);
						input.set_HumanTab_cx(num+kinect_num,i);
						input.set_HumanTab_cy(num+kinect_num,j);
						input.set_HumanTab_ID(num+kinect_num,num+kinect_num);//����l���F��   *���̂����쐬����\��
						input.set_HumanTab_frame(num+kinect_num,(int)frame);//
						id_data.insert(std::pair<int,int>(MAX,input.get_human_id(i,j)));
						input.set_Human_label(i,j,num+kinect_num);//
						input.set_HumanTab_skelton(num+kinect_num,MAX,input.get_human_id(i,j));//
						input.set_HumanTab_kinect(num+kinect_num,input.get_human_id(i,j));
						input.set_mask(i,j,0);
						MAX++;
					}
				}
			}
		}

		for(int t=0;t<MAX;t++)
		{

			input.set_HumanTab_cx(t+num+1,input.get_HumanTab_cx(t+num+1)/input.get_HumanTab_size(t+num+1));
			input.set_HumanTab_cy(t+num+1,input.get_HumanTab_cy(t+num+1)/input.get_HumanTab_size(t+num+1));

			//cout<<"cx,cy:"<<input.get_HumanTab_cx(t+num+1)<<","<<input.get_HumanTab_cy(t+num+1)<<endl;
			human_table[t+num+1].x1=input.get_HumanTab_s_w(t+num+1);
			human_table[t+num+1].x2=input.get_HumanTab_e_w(t+num+1);
			human_table[t+num+1].y1=input.get_HumanTab_s_h(t+num+1);
			human_table[t+num+1].y2=input.get_HumanTab_e_h(t+num+1);
			human_table[t+num+1].xcenter=input.get_HumanTab_cx(t+num+1);
			human_table[t+num+1].ycenter=input.get_HumanTab_cy(t+num+1);
			human_table[t+num+1].size=input.get_HumanTab_size(t+num+1);
			human_table[t+num+1].frame=(int)frame;
			human_table[t+num+1].ID=(int)0;//input.get_HumanTab_ID(id_data[t]);
			Humanflag=true;
			/*if(humansize_min>input.get_HumanTab_size(t+num+1))
			{
			humansize_min=input.get_HumanTab_size(t+num+1);
			}*/
		}
		input.set_log_skelton_data(frame);
	}

	if( kinect_num > 0 ){
		//cout << kinect_num << "(kinect_num)!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
	}
	HtabMAX(num+kinect_num);//���m���ꂽ�l��
	input.HumantabMAX(num+kinect_num);//���m���ꂽ�l����␔

	//d_human_maching(input,frame);//LV3�l���ǐ�

	if(Humanflag)BinaryImgLabeling(input);//�o�͉摜�f�[�^�ɕ`��

	Hreleace();

	if(num==0&&kinect_num==0){Humanflag=false;}//�l�������m���ꂽ
	return Humanflag;
}

//--------------------------------------------------
// Name     : human::BinaryImgLabeling()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.11.9
// function : �o�͉摜�ɐl���̈�`������
// Argument : �Ȃ�
// return   : �Ȃ�
//--------------------------------------------------
void human::BinaryImgLabeling(image &input)
{
	//�����Ƃ��ă��x�����t����ꂽ�̈�ɑ΂��ĐF�Â�
	for(int t=1;t<=input.HumanData();t++){
		for (int x = input.get_HumanTab_s_w(t); x <= input.get_HumanTab_e_w(t); x++) {
			input.set_Out_R(x,input.get_HumanTab_s_h(t),255);
			input.set_Out_G(x,input.get_HumanTab_s_h(t),255);
			input.set_Out_B(x,input.get_HumanTab_s_h(t),0);

			input.set_Out_R(x,input.get_HumanTab_e_h(t),255);
			input.set_Out_G(x,input.get_HumanTab_e_h(t),255);
			input.set_Out_B(x,input.get_HumanTab_e_h(t),0);

			input.set_Out_R(x,input.get_HumanTab_cy(t),255);//TODO:remove comment out �d�S
			input.set_Out_G(x,input.get_HumanTab_cy(t),255);
			input.set_Out_B(x,input.get_HumanTab_cy(t),0);
		}
		for (int y = input.get_HumanTab_s_h(t); y <= input.get_HumanTab_e_h(t); y++) {
			input.set_Out_R(input.get_HumanTab_s_w(t),y,255);
			input.set_Out_G(input.get_HumanTab_s_w(t),y,255);
			input.set_Out_B(input.get_HumanTab_s_w(t),y,0);

			input.set_Out_R(input.get_HumanTab_e_w(t),y,255);
			input.set_Out_G(input.get_HumanTab_e_w(t),y,255);
			input.set_Out_B(input.get_HumanTab_e_w(t),y,0);

			input.set_Out_R(input.get_HumanTab_cx(t),y,255);//TODO:remove comment out �d�S
			input.set_Out_G(input.get_HumanTab_cx(t),y,255);
			input.set_Out_B(input.get_HumanTab_cx(t),y,0);
		}

		for(int j=input.get_HumanTab_s_h(t); j<input.get_HumanTab_e_h(t); j++){
			for(int i=input.get_HumanTab_s_w(t); i<input.get_HumanTab_e_w(t); i++){
				int Yi, Cb, Cr;
				Yi = (int)( 0.299  * input.get_input_R(i,j) + 0.587  * input.get_input_G(i,j) + 0.114  * input.get_input_B(i,j));
				Cb = (int)(-0.1687 * input.get_input_R(i,j) - 0.3312 * input.get_input_G(i,j) + 0.5000 * input.get_input_B(i,j));
				Cr = (int)( 0.5000 * input.get_input_R(i,j) - 0.4183 * input.get_input_G(i,j) - 0.0816 * input.get_input_B(i,j));

				//���F��臒l�Ɉȓ��Ȃ甧�F�̃��x����t����
				if(input.get_subtraction_G(i,j)==255)
				{
				if( ((Yi > 20) && (Yi < 240))
						&& ((Cb > -20) && (Cb < 20))
						&& ((Cr > 10) && (Cr < 30)) ){
					input.set_subtraction_B(i,j,0); 
					input.set_subtraction_R(i,j,0);
					//input.set_subtraction_G(i,j,255);
					}
					//���F��臒l�ȓ��Ȃ甯�F�̃��x����t����
				else if(Yi < 50){
					
					input.set_subtraction_B(i,j,0); 
					//input.set_subtraction_R(i,j,255);
					input.set_subtraction_G(i,j,0);

				}
				else{//�l���̈�
					//input.set_subtraction_B(i,j,255); 
					input.set_subtraction_R(i,j,0);
					input.set_subtraction_G(i,j,0);
				}

				}

			}
		}
	}
	//����(���C���C����ȊO)�ɑ΂��ă��x�����O
}
