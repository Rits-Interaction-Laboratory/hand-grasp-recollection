//#pragma once
//#include ".\image.h"
//#include ".\bmp.h"
//#include <map>
//#include <deque>
//#include <vector>
//
//
////�K�v
////
//// �����w�ڂ܂ŏd�Ȃ��Ă邩�H
//// �V�[���Ƀ}�X�N�摜�ǉ�
//// ���ǂ�ȃ}�X�N������Ă���̂��H��H
//// �}�X�N�̏㉺�֌W���X�V
//// ���݂̏��
//// ������W����ɗ�������ǂ�ȕ��̂����m���ꂽ�����ׂ�
//// ���鎞�̉摜�o��
//// ���̉摜�̒ǉ��C�X�V�C�폜�C����
////
//// ������W(i,j)�ɏd�Ȃ��ԏ�̕��̉摜�o��
//
////
//// pop push�@delete clear disp update
//// search
////	void Set_img(bmp &obj){ img = obj; }
////	bmp &Get_img(){ return img;}
//	
//
////=====================
////���̉摜��\���\����
////=====================
//typedef struct obj{
//	bmp img;  //�摜(���̌��m���ꂽ)
//	int layer;//���w�ڂ�?
//	int id;   //�摜ID
//}Object_img;
//
////-------------------
////Class
////-------------------
//class Scene
//{
//	//===============================================================================
//private:
//	std::map<std::string, bmp> bmp_map; //���̉摜�̗��� key�������C�摜��      
//	std::vector<Object_img>    obj;  //���̉摜��\���\���̂��i�[����
//	//===============================================================================
//
//	//-------------------------------------------------------------------------------
//public:
//	// constructor
//	Scene(void){}
//	// destructor
//	~Scene(void){}
//
//	//������
//	void init(){
//	}
//
//	//���ݎ����𕶎���ŕԂ�
//	std::string get_time();
//
//	//���̗̂���
//	void set_image(bmp &img);
//
//	//�����̃`�F�b�N
//	bool CheckDiffNum(image &img,image &start,Area *area);
//
//	double SimilarLevel(bmp &pre,bmp &now,Area *area);
//	
//	//�Q�̉摜�̘_���a�摜����
//	void AndImage(bmp &start,bmp &end,bmp &result,Area *area);
//
//	void AndDisp(bmp &start,bmp &end,Area *area);
//
//	void update(bmp &img){
//		Object_img a;
//		a.img = img;
//		a.layer = 3;
//		a.id = 2;
//		obj.push_back(a);
//	}
//
//	//���݂̃V�[���ɂ���摜�S�ĕ\��
//	void disp(){
//		for(std::vector<Object_img>::iterator i = obj.begin();i!=obj.end();i++){
//			std::cout << i[0].id << std::endl;
// 		}
//	}
//
//	//-------------------------------------------------------------------------------
//};
