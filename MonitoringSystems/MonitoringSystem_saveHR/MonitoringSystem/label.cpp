#include "stdafx.h"

#include "label.h"
#include <iostream>
#include <vector>
#include <climits>
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int large_size = 76700; //�傫�����x���̖ʐ�//kawa3000
const int min_size = 100;//���������x���̖ʐ�//kawa400


//--------------------------------------------------
// Name     : std::vector<int> label::getobjLabel(void)
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2006.3.17
// function : ���̂ƒ�`�����̈�̃��x���̔ԍ����i�[
// Argument : �Ȃ�
// return   : vector<int> ���x���ԍ������Ԃɓ����Ă���
//--------------------------------------------------
std::vector<int> label::getobjLabel(void)
{
	int LabelName=0;
	std::vector<int> o_label;

	for (int i=1; i<=Num; i++) {
		if (LTable[i].size < large_size
			&& LTable[i].size > min_size){
				
			o_label.push_back(i);
		}
	}
	return o_label;
}

//---------------------------------
//���x���̑傫����Ԃ�
//---------------------------------
int label::getLabelSize(const int LabelName)
{
	return LTable[LabelName].size;
}

void label::getLabelArea(const int LabelName, int &x1, int &x2, int &y1, int &y2)
{
	x1 = LTable[LabelName].x1;
	x2 = LTable[LabelName].x2;
	y1 = LTable[LabelName].y1;
	y2 = LTable[LabelName].y2;
}

void label::getGravityCenter(const int LabelName, int &x, int &y)
{
	x = LTable[LabelName].xcenter;
	y = LTable[LabelName].ycenter;
}

//----------------------------------
//���x�����O����
//
//
//----------------------------------
void label::Labeling(void)
{

	for (int y=0; y<Height; y++) {
		for (int x=0; x<Width; x++) {
			//label.Pixels[x, y] = -1 �Ȃ�
			if (place(x,y) == -1) {
				Num++;
				//Num��int�^�̍ő�l�ɓ��B������\��
				if(INT_MAX==Num)std::cout<<"Num:"<<Num<<std::endl;

				//label.Pixels��e�s�N�Z���̉򂲂Ƃɒʂ��ԍ�Num��U���Ă���
				RunAnalysis(Num, x,y);  //�������
			}
		}
	}

	setLabelTable();
}

//----------------------------------
//���W(x,y)���܂ސ��������Ƀ��x���t�����A���̏㉺�ɗאڂ����f�ɂ����X�Ƀ��x���t�������Ă���
//
//
//----------------------------------
void label::RunAnalysis(const int LabelName, const int x, const int y)
{
	int left, right;  //���[�ƉE�[

	left = seekLeft(x,y);
	right = setLabel(LabelName, left,y);  //���x���̐ݒ�

	if (left == 0)  left++;
	if (right == Width-1)  right--;

	//�����̏㉺�̒���
	if (y>0) {
		for (int x2=left-1; x2<=right+1; x2++) {
			if (place(x2,y-1) == -1) {
				RunAnalysis(LabelName, x2,y-1);  //�ċA�I����
			}
		}
	}
	if (y<Height-1) {
		for (int x2=left-1; x2<=right+1; x2++) {
			if (place(x2,y+1) == -1) {
				RunAnalysis(LabelName, x2,y+1);  //�ċA�I����
			}
		}
	}

}

//----------------------------------
//
//���W(x,y)���牡�����ɘA�����Ă��邷�ׂĂ�
//��f�Ƀ��x���t�����A�E�[��x���W��Ԃ�
//----------------------------------
int label::setLabel(const int LabelName, const int x, const int y)
{
	int left=seekLeft(x,y);
	int right=seekRight(x,y);

	for (int x2=left; x2<=right; x2++) {
		place(x2,y) = LabelName;
		//setLabelTable(LabelName, x2,y);
	}

	return right;
}

//----------------------------------
//
//���W(x,y)���牡�����ɘA�����Ă���
//��f�̍��[��x���W��Ԃ�
//----------------------------------
int label::seekLeft(const int x, const int y)
{
	int x2;

	if (place(x,y)!=-1)  return -1;
	else {
		for (x2=x; x2>=0; x2--) {
			if (x2==0)  break;  //��ʒ[�ɓ��B
			else if (place(x2-1,y)!=-1)  break;
		}

		return x2;
	}
}

//----------------------------------
//���W(x,y)���牡�����ɘA�����Ă����f�̉E�[��x���W��Ԃ�
//----------------------------------
int label::seekRight(const int x, const int y)
{
	int x2;

	if (place(x,y)!=-1)  return -1;
	else {
		for (x2=x; x2<Width; x2++) {
			if (x2==Width-1)  break;  //��ʒ[�ɓ��B
			else if (place(x2+1,y)!=-1)  break;
		}
		return x2;
	}
}

//----------------------------------
//
//----------------------------------
void label::setLabelTable(void)
{
	for (int y = 0; y<Height; y++) {
		for (int x = 0; x<Width; x++) {
			if (place(x,y) > 0) {
				int ln = place(x,y);

				//�̈�ݒ�
				if (LTable[ln].size == 0) {
					LTable[ln].x1 = x;
					LTable[ln].x2 = x;
					LTable[ln].y1 = y;
					LTable[ln].y2 = y;
				}
				else {
					if (x < LTable[ln].x1) {
						LTable[ln].x1 = x;
					}
					else if (x > LTable[ln].x2) {
						LTable[ln].x2 = x;
					}
					if (y < LTable[ln].y1) {
						LTable[ln].y1 = y;
					}
					else if (y > LTable[ln].y2) {
						LTable[ln].y2 = y;
					}
				}

				//�d�S�v�Z�p�̗\���v�Z
				LTable[ln].xsum += x;
				LTable[ln].ysum += y;

				LTable[ln].size++;
			}
		}
	}

	//�d�S�v�Z
	for (int i=1; i<=Num; i++) {
		//if(LTable[i].size < 10) LTable[i].size = 0;
		LTable[i].xcenter = (int)(LTable[i].xsum / LTable[i].size);
		LTable[i].ycenter = (int)(LTable[i].ysum / LTable[i].size);
	}

}
