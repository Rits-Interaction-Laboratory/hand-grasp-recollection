///////////////////////////////////////////////////////////
// FileName : scene.cpp
// Context  : �V�[���̉��(�ߋ��̗����C���݂̏��)
//
// Author�@ :  Noriaki Katayama(katayama@i.ci.ritsumei.ac.jp)
//             Computer Vision Lab.
//             Department of Human and Computer Intelligence
//             College of Information Science & Engineering
//             Ritsumeikan University
//
// Update   : 2008.04.21
///////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1 /*VisualC++�ł�localtime()�̌x���}��*/
#include "stdafx.h"

#include ".\image.h"
#include ".\scene.h"
#include <sstream>
#include <ctime>
#include <deque>
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 4244)
//'=' : 'double' ���� 'int' �ɕϊ����܂����B�f�[�^�������Ă��邩������܂���B

//-------------------------------------------------------
// Name     : get_time()
// Author�@ : Katayama Noriaki (CV-lab.)
// function : ���ݎ����𓾂�
// Argument : �Ȃ�
// return   : std::string �^�̕�����
// Update   : 2005.12.3
//--------------------------------------------------------
std::string Scene::get_time()
{
	time_t nt;
	struct tm *t_st;
	char c[30];

	std::ostringstream oss;
	//time(&nt);             //���ݎ����̎擾
	//t_st = localtime(&nt); //���ݎ������\���̂ɕϊ� 
	localtime_s(t_st,&nt);
    //strftime(c,sizeof(c),"%Y-%m-%d-%H��%M��%S�b",t_st);
	strftime(c,sizeof(c),"%Y-%m-%d-%H-%M-%S",t_st);

	std::string f_time = c;

	return f_time;
}

//-------------------------------------------------------
// Name     : set_image()
// Author�@ : Katayama Noriaki (CV-lab.)
// function : 
// Argument : 
// return   : �Ȃ�
// Update   : 2005.7
//--------------------------------------------------------
void Scene::set_image(bmp &img)
{
	std::map<std::string, bmp>::iterator p; //�C�e���[�^

	//�摜�����ݎ������L�[�ɂ��đ}������
	std::string now_t = get_time();

	//���ݎ������L�[�ɂ��ĉ摜���i�[����
	bmp_map.insert(std::map<std::string,bmp>::value_type(now_t,img));

	std::cout << get_time() << std::endl;

	//std::string fliename = now_t + ".bmp";//���t�̃t�@�C����
	
	//�t�@�C�������o��
	//bmp_map[now_t].save(fliename.c_str());


	// ����
	//p = bmp_map.find("02"); 

	std::cout << "-----���݂̊i�[����Ă���t�@�C��------" << std::endl;
	for(p = bmp_map.begin() ;p != bmp_map.end();p++){
		std::string name = p->first + ".png";
		std::cout << name  << std::endl;
		p->second.save(name.c_str());
	}


/*
	//�}�b�v������
	//if(p != bmp_map.end())
	//{
	//	std::string name = p->first;
	//	std::cout << name << "�Ƃ����摜������܂���" << std::endl;
	//	p->second.save("aaaaaaaaaa.bmp");
	//}
	//else{
	//	std::cout << "������܂���" << std::endl;
	//}

	*/
}

//------------------------------------------------------------------------
// Name     : CheckDiffNum()
// Author�@ : Katayama Noriaki (CV-lab.)
// function : ���̂��ǂ����̃`�F�b�N
//            �����������Ԃœ����ꏊ�ɂ���Ε��̂Ƃ���
// Argument : image &img     �����𒲂ׂ�摜
//            image &start   �ߋ��̉摜
//            Area *area     ���ׂ�͈͂̍��W�̍\����
//
// return   : bool           �������c���Ă���Ε���
// Update   : 2005.12.3
//------------------------------------------------------------------------
bool Scene::CheckDiffNum(image &img,image &start,Area *area)
{
	int pre_num = 0; //�ߋ��̍����̐�
	int now_num = 0; //���݂̍����̐�
	double hold;        //�����̐���臒l

	//�������ꂼ�꒲�ׂ�
	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(start.isDiff(i,j) == BACK) pre_num++;
			if(img.isDiff(i,j)   == BACK) now_num++;
		}
	}

	std::cout  << "�O" << pre_num << std::endl;
	std::cout  << "��" << now_num << std::endl;

	//�܂������������ɂ͂Ȃ�Ȃ��̂Œ���
	hold = pre_num * 0.7;

	//�ߋ��̉摜�����ꏊ�ɍ���������Ε��� true��Ԃ�
	if(now_num >= hold) return true;

	//���̌��m�͂���Ȃ������̂�false
	else return false;
}


//------------------------------------------------------------------------
// Name     : AndImage()
// Author�@ : Katayama Noriaki (CV-lab.)
// function : �����摜���m�̘_���a���Ƃ�
//
// Argument : image &start  �J�n�t���[���̉摜
//            image &end,�@ �I���t���[���摜
//            image &result �_���a���Ƃ����摜(�����Ɍ��ʂ�����)
//            Area *area    �`�F�b�N����̈�̍��W(�\����)
//
// return   : �Ȃ�
// Update   : 2005.12.3
//------------------------------------------------------------------------
void Scene::AndImage(bmp &start,bmp &end,bmp &result,Area *area)
{
	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(start.isDiff(i,j) == BACK && end.isDiff(i,j) == BACK){
				//�Q�̉摜�œ������W�Ŕw�i�����������畨�̂̍���
				result.isDiff(i,j) = BACK;
			}
			else if(start.isDiff(i,j) == BACK && end.isDiff(i,j) != BACK){
				result.isDiff(i,j) = NON;
			}
			else if(start.isDiff(i,j) != BACK && end.isDiff(i,j) == BACK){
				result.isDiff(i,j) = NON;
			}
			else if(start.isDiff(i,j) != BACK && end.isDiff(i,j) != BACK){
				result.isDiff(i,j) = NON;
			}
		}
	}
}

double Scene::SimilarLevel(bmp &pre,bmp &now,Area *area)
{
	int pre_num = 0; //�ߋ��̍����̐�
	int now_num = 0; //���݂̍����̐�

	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(pre.isDiff(i,j) == BACK) pre_num++;
			if(pre.isDiff(i,j) == BACK  && now.isDiff(i,j) == BACK) now_num++;
		}
	}

	std::cout << pre_num << std::endl;
	std::cout << now_num << std::endl;


	return ((float)now_num / (float)pre_num);
}

void Scene::AndDisp(bmp &pre,bmp &now,Area *area)
{
		for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(pre.isDiff(i,j) == BACK && now.isDiff(i,j) == BACK) now.SetPixColor(i,j,255,255,255);
		}
	}

}