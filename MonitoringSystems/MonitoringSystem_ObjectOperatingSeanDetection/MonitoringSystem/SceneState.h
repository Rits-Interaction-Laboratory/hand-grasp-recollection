#pragma once
#include "bmp.h"
#include <stack>

struct IMG_RECORD{
	bmp mask;
	bmp back;
	bmp result;
};

//enum Mode1{ONLINE,OFFLINE};
//----------------------------
//�V�[���̏��
//----------------------------
class Scene
{
private:
	//int layer;
	//IMG_RECORD *rec;
	bmp *top,*tail;
	bmp *img_t1,*img_t2,*img_t3,*img_t4,*img_t5,*img_t6,*img_t7,*img_t8;
	std::stack<bmp> obj_data;

	int mode;  //�I�����C��or�I�t���C��

public:
	//constructor
	Scene();

	//destructor
	~Scene();

	//������
	void init(){top = NULL;}

	//���t���[���ɕ��̃��x���̗̈�𔭌��������H
	int check(bmp &in,unsigned long frame);

	//�Q���̉摜���r���ĕ��̂����m�������H
	void detection(bmp &now,bmp &pre,int *count);

	void set_obj(bmp &in);
	bmp get_obj();

	//���ւ��v�Z����֐�
	bool BlockCorr(bmp &pre,bmp &now);//�u���b�N���Ƃ̍��v�̕���
	bool AllCorr(bmp &pre,bmp &now);//�S�Ă̍��v�̕���

	//���[�h�̐ݒ�
	void set_mode(int m){mode=m;}
};
