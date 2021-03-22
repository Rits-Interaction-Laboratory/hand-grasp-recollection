#include "stdafx.h"

#include ".\scenestate.h"
#include "bmp.h"
#include <stack>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include "FileOperator.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable: 4244)

//-------------------------
//constructor
//-------------------------
Scene::Scene(void)
{

}

//-------------------------
//destructor
//-------------------------
Scene::~Scene(void)
{
}

//------------------------------------------------------------------------
// Name     : set_obj(bmp &in)
// Author�@ : Katayama Noriaki (CV-lab.)
// function : ���݂̃V�[���ɕ��̗̈�̃}�X�N��ǉ�
// Argument : bmp ���͕��̗̈�摜
// return   : �Ȃ�
// Update   : 2006.7.12
//------------------------------------------------------------------------
void Scene::set_obj(bmp &in)
{
	const int max_size = 100;

	//�Ƃ肠����100�܂ŉ摜���X�^�b�N�ɓ����
	if(obj_data.size() < max_size){
		bmp* input= new bmp;
		input->pix_copy(in.get_pixels(),in.get_pixels_Y());
		obj_data.push(*input);
	}
}


//------------------------------------------------------------------------
// Name     : get_obj()
// Author�@ : Katayama Noriaki (CV-lab.)
// function : ���݃V�[�����̉摜�̃}�X�N�摜��Ԃ�
// Argument : �Ȃ�
// return   : 
// Update   : 2006.7.12
//------------------------------------------------------------------------
bmp Scene::get_obj()
{

	bmp top;

	if(obj_data.empty() != false) top = obj_data.top();
	else{
		bmp temp;
		temp.set_same_val(128);
		top = temp;
	}

	return top;
}


//------------------------------------------------------------------------
// Name     : detection(bmp &now,bmp &pre,Area &area)
// Author�@ : Katayama Noriaki (CV-lab.)
// function : ���̗̈�̌`���ߋ��Ɣ�ׂ�
// Argument : bmp  ���͉摜
//            bmp  �ߋ��̓��͉摜
// return   : ���̌��m��1��Ԃ�
// Update   : 2006.6.18
//------------------------------------------------------------------------
void Scene::detection(bmp &now,bmp &pre,int *count)
{
	//int same_pix = 0;//�����ꏊ�̉�f
	//int now_num  = 0; //���݂̍����̐�
	int *same_pix;
	int *now_pix;
	int imageHeight = now.getheight();
	int imageWidth = now.getwidth();

	same_pix = new int[now.get_objnum()];
	now_pix  = new int[now.get_objnum()];

	for(int i=0;i<now.get_objnum();i++){
		same_pix[i] = 0;
		now_pix[i]  = 0;
	}

	Area d_area = now.get_detect_area();

	//############################################
	//���ڂ������x���̗̈�ɂ���
	//���̗̈�ɉߋ��ɂ������傫�����炢�̓������x����
	//�������̂����������̂����ׂ�
	//����΁@�{�P�@������΁@�|�P
#if 0
	for(int j = d_area.s_h; j <= d_area.e_h; j++){
		for(int i = d_area.s_w; i <= d_area.e_w; i++){
			if(j == d_area.s_h || i == d_area.s_w){
				now.R(i,j) = 0;
				now.G(i,j) = 0;
				now.B(i,j) = 255;
			}
			if(j == d_area.e_h || i == d_area.e_w){
				now.R(i,j) = 0;
				now.G(i,j) = 0;
				now.B(i,j) = 255;
			}
		}
	}
#endif

	//�P�O�̃t���[���ƕ��̗̈�ɂ��ĉ�f�̐��𒲂ׂ�
	for(int j = d_area.s_h; j < d_area.e_h; j++){
		for(int i = d_area.s_w; i < d_area.e_w; i++){
			int num=-1;
			if(((num = now.isDiff(i,j) -(OBJECT*10))) >= 0) {
				now_pix[num]++;
			}
			if(num>=0/*now.isDiff(i,j) == OBJECT*/ &&  (pre.isDiff(i,j) < OBJECT*10) ){
				same_pix[num]--;
			}
			if(num>=0/*now.isDiff(i,j) == OBJECT*/ &&  (pre.isDiff(i,j) >= OBJECT*10) ){
				same_pix[num]++;
				//now.SetPixColor(i,j,0,0,255);
				now.G(i,j) = 255; //�΂Ŕ��������ۂ��i�������̕����m�F���₷���H�j
			}
		}
	}

	/*int tmp=0;
	for(int i=0;i<now.obj_cand;i++){
		if(now_pix[i] < 15 || same_pix[i] < 0)
			tmp++;
	}

	if(tmp == now.obj_cand+1)
		return 0;*/

	for(int i=0;i<now.get_objnum();i++){
		if(now_pix[i]<15 || same_pix[i] < 0)	continue;
		double hold = (double)now_pix[i] * 0.85;

		//臒l�ȓ��Ȃ畨�̌��m�������Ƃɂ���
		if(same_pix[i] > hold)	count[i]++;
	}

	//������Ȃ�������
	//return 0;
}

//------------------------------------------------------------------------
// Name     : check(bmp &in)
// Author�@ : Katayama Noriaki (CV-lab.)
// function : ���̂����m���邽�߂Ɉ��t���[���Ď�
// Argument : bmp ���̉摜
// return   : bool ���t���[���̓����ʒu�ɕ��̗̈悪�����true
// Update   : 2006.7.12
//------------------------------------------------------------------------
int Scene::check(bmp &in,unsigned long frame)
{
	const int interval = 10;
	bmp* input= new bmp;//���͉摜�̃R�s�[
	static bool obj_flag;
	//int count = 0; //���̂��ϑ������V�[���̐�
	static unsigned long detect_num;

	int obj_label=-1;
	int *count;
	count = new int[in.get_objnum()];
	for(int i=0;i<in.get_objnum();i++)
		count[i] = 0;

	//���͉摜���R�s�[
	input->pix_copy(in.get_pixels(),in.get_pixels_Y());
	input->copy_area(in.get_detect_area());
	//input->obj_num = in.get_objnum();
	input->set_objnum(in.get_objnum());

	//�Ō���ɐV�����摜�f�[�^��ǉ�
	tail =  input;

	//���݂̓��͉摜���珇�Ԃɉߋ��̗��������Ă���
	if(frame > 30){

		/*count += */detection(*tail,*img_t8,count);
		/*count += */detection(*tail,*img_t7,count);
		/*count += */detection(*tail,*img_t6,count);
		/*count += */detection(*tail,*img_t5,count);
		/*count += */detection(*tail,*img_t4,count);
		/*count += */detection(*tail,*img_t3,count);
		/*count += */detection(*tail,*img_t2,count);
		/*count += */detection(*tail,*img_t1,count);
		/*count += */detection(*tail,*top,count);

		//for(int i=0;i<in.obj_num;i++){

		//���͉摜�Ɖߋ������Ă���
		//�����ȏ�̃t���[���ŕ��̂��m�F���ꂽ�畨�̌��m�Ƃ���
		obj_flag = false;
		for(int z=0;z<in.get_objnum();z++){
			if(count[z] > 6){

			obj_flag = true;
			obj_label = z;

			////���ݎ������t�@�C�����ɂ���
			//std::string img_fname;
			//time_t nt;
			//struct tm *t_st;
			//char c[35];
			//char now_time[40];
			//time(&nt);             //���ݎ����̎擾
			//t_st = localtime(&nt); //���ݎ������\���̂ɕϊ� 
			//strftime(c,sizeof(c),"%Y-%m-%d-%H-%M-%S",t_st);
			////strftime(now_time,sizeof(now_time),"%Y�N%m��%d��%H��%M��%S�b",t_st);
			//strftime(now_time,sizeof(now_time),"%H��%M��%S�b",t_st);
			//img_fname = img_fname + "hoge/" + c + ".bmp";//���ݎ����̕�����

			//�ȉ��ύX///////////////////////////
			//Area obj_area = in.get_area();

			int x1,x2,y1,y2;
			x1 = in.get_obj_xmin(z);  //obj_area.s_w;
			y1 = in.get_obj_ymin(z);  //obj_area.s_h;
			x2 = in.get_obj_xmax(z);  //obj_area.e_w;
			y2 = in.get_obj_ymax(z);  //obj_area.e_h;
			//�ύX�����܂�////////////////////////

			/*std::cout << "x1 = " << x1;
			std::cout << ",x2 = " << x2 << std::endl;
			std::cout << "y1 = " << y1;
			std::cout << ",y2 = " << y2 << std::endl;*/

			in.set_object_area(x1,y1,x2,y2);

			//Log�t�@�C�������o���p
			std::ofstream file_out("../DB/object.log",std::ios::app);
			file_out << frame << std::endl;
			file_out << x1 << ",";
			file_out << x2 << ",";
			file_out << y1 << ",";
			file_out << y2 << std::endl;

#if 1//���m�������̂̊O�ڒ����`�𑾂����ň͂�
			int w_size = x2 - x1;
			int h_size = y2 - y1;
			if(w_size < 200 || h_size < 200 ){

				for (int x = x1; x <= x2; x++){
					tail->SetPixColor(x,y1  ,0,255,255);
					tail->SetPixColor(x,y1-1,0,255,255);
					tail->SetPixColor(x,y1+1,0,255,255);
					tail->SetPixColor(x,y2  ,0,255,255);
					tail->SetPixColor(x,y2-1,0,255,255);
					tail->SetPixColor(x,y2+1,0,255,255);
					//*tail->SetPixColor(x,yc,0,255,255);
				}
				for (int y = y1; y <= y2; y++){
					tail->SetPixColor(x1  ,y,0,255,255);
					tail->SetPixColor(x1-1,y,0,255,255);
					tail->SetPixColor(x1+1,y,0,255,255);
					tail->SetPixColor(x2  ,y,0,255,255);
					tail->SetPixColor(x2-1,y,0,255,255);
					tail->SetPixColor(x2+1,y,0,255,255);
					//*tail->SetPixColor(xc,y,0,255,255);
				}
			}

			//for (int y = y1; y <= y2; y++){
			//	for (int x = x1; x <= x2; x++){
			//		if(in.isDiff(x,y) == OBJECT){
			//			in.isDiff(x,y) = DETECT;
			//		}
			//	}
			//}

#endif
			//CHECK:���m�������̂�ۑ�����
			std::ostringstream dir,frame_name;
			std::string dir_name;
			FileOperator path;

			/*if(mode==ONLINE){
				dir  << "\\\\Proliant2/okamoto/Detected-Object/" << frame << "/";
				frame_name << frame;
				dir_name = "\\\\Proliant2/okamoto/Detected-Object/" + frame_name.str();
			}else if(mode==OFFLINE){*/
				dir  << "../DB/Detected-Object/" << frame << "/";
				frame_name << frame;
				dir_name = "../DB/Detected-Object/" + frame_name.str();
			//}
			
			path.MakeDir(dir_name.c_str());

			std::string img_fname1;
			std::string img_fname2;
			std::string img_fname3;
			std::string img_fname4;
			std::string img_fname5;
			std::string img_fname6;
			std::string img_fname7;
			std::string img_fname8;
			std::string img_fname9;
			std::string img_fname10;

			img_fname1  = img_fname1  + dir.str() + "result-" + frame_name.str() + ".png";
			img_fname2  = img_fname2  + dir.str() + "t-08-" + frame_name.str() + ".png";
			img_fname3  = img_fname3  + dir.str() + "t-07-" + frame_name.str() + ".png";
			img_fname4  = img_fname4  + dir.str() + "t-06-" + frame_name.str() + ".png";
			img_fname5  = img_fname5  + dir.str() + "t-05-" + frame_name.str() + ".png";
			img_fname6  = img_fname6  + dir.str() + "t-04-" + frame_name.str() + ".png";
			img_fname7  = img_fname7  + dir.str() + "t-03-" + frame_name.str() + ".png";
			img_fname8  = img_fname8  + dir.str() + "t-02-" + frame_name.str() + ".png";
			img_fname9  = img_fname9  + dir.str() + "t-01-" + frame_name.str() + ".png";
			img_fname10 = img_fname10 + dir.str() + "t-09-" + frame_name.str() + ".png";

			top->save(img_fname10.c_str());
			img_t1->save(img_fname2.c_str());
			img_t2->save(img_fname3.c_str());
			img_t3->save(img_fname4.c_str());
			img_t4->save(img_fname5.c_str());
			img_t5->save(img_fname6.c_str());
			img_t6->save(img_fname7.c_str());
			img_t7->save(img_fname8.c_str());
			img_t8->save(img_fname9.c_str());
			tail->save(img_fname1.c_str());

			////���̍��������Z�[�u
			//IplImage *obj = cvCreateImage(cvSize(tail->getwidth(),top->getheight()),IPL_DEPTH_8U,1);

			//int num=0;
			//for(int j=0;j<tail->getheight();j++){
			//	for(int i=0;i<tail->getwidth();i++){
			//		if((tail->isDiff(i,j) - (OBJECT*10))>=0){
			//			obj->imageData[obj->widthStep*(tail->getheight()-j-1)+i] = (unsigned char)(in.R(i,j)*0.299+in.G(i,j)*0.587+in.B(i,j)*0.114);
			//		}else{
			//			obj->imageData[obj->widthStep*(tail->getheight()-j-1)+i] = 0;
			//		}
			//	}
			//}
			//img_fname1 = dir.str() + "object.bmp";
			//cvSaveImage(img_fname1.c_str(),obj);
			//cvReleaseImage(&obj);

		}

		else{
			//���̂𔭌��ł��Ȃ�������
			//obj_flag = false;

		}
		}
	}

	//std::string hoge = "test-name.bmp";
	//g_dlgval.set_img_name(hoge);

	//===============================================
	//�P��̏�ԂɑS�Ẵ|�C���^��1�Â��炷
	if(top && frame >15 )top->release();	//�Ō���̃f�[�^�͉��
	top    = img_t1;
	img_t1 = img_t2;
	img_t2 = img_t3;
	img_t3 = img_t4;
	img_t4 = img_t5;
	img_t5 = img_t6;
	img_t6 = img_t7;
	img_t7 = img_t8;
	img_t8 = tail;
	//tail   = NULL;

	if(count)	delete count;

	return obj_label;

	//������t���[���ŕ��̂𔭌��������H
	if(obj_flag == true ){
		//�Ō���̃f�[�^�͉��
		//if(top)top->release();
		return true;
	}
	else {
		return false;
	}
}

//�̈���u���b�N���Ƃɑ��ւ����߂�
bool Scene::BlockCorr(bmp &pre,bmp &now)
{
	const int block_size = 5; //���ւ����߂�u���b�N�T�C�Y
	Area d_area = now.get_detect_area();//���߂�G���A
	int Height = now.getheight();//�摜�̍���
	int Width = now.getwidth();//�摜�̕�
	unsigned long int M = Height * Width;  //��f��M

	double sigma_x,sigma_y;                //�W���΍���x,��y
	double sigma_xy;
	int  ave_pre,ave_now;                 //����摜�̋P�x�l�̕��ϒl
	long  sum_Cov = 0;                //x��y���ꂼ��̕��ςƍ��̍��v
	long  sum_pre = 0,sum_now = 0;    //����摜�̋P�x�l���v
	long sum_x = 0,sum_y = 0;        //�����̕��ςƂ̍��̍��v�l
	int diff_x,diff_y;//���ςƂ̍�
	double Cov_xy;//�����U
	unsigned long m = 0;
	int block_num = block_size * block_size;

	static long int counter;//�t���[����
	++counter;//�t���[����

	for(int j = 0; j < Height; j++){
		for(int i = 0 ; i < Width; i++){
			now.Y(i,j) = now.R(i,j);
			pre.Y(i,j) = pre.R(i,j);
			m++;
		}
	}


	//std::cout << "h--" << Height << std::endl;
	//std::cout << "w-- " << Width << std::endl;
	//std::cout << "s_h" << d_area.s_h << std::endl;
	//std::cout << "e_h" << d_area.e_h << std::endl;
	//std::cout << "s_w" << d_area.s_w << std::endl;
	//std::cout << "e_w" << d_area.e_w << std::endl;

	//�u���b�N���Ƃɒl�����߂邽���̂ő傫���𒲐�
	int d_hight = d_area.e_h - d_area.s_h;
	int d_width = d_area.e_w - d_area.s_w;
	int h_surplus = d_hight % block_size;
	int w_surplus = d_width % block_size;

	if(h_surplus != 0) d_hight += (block_size - h_surplus);
	if(w_surplus != 0) d_width += (block_size - w_surplus);

	//std::cout << d_hight << std::endl;
	//std::cout << d_width << std::endl;

	int h_block = d_hight / block_size;
	int w_block = d_width / block_size;

	//���֌W�����u���b�N���Ƃɕۑ�
	double *r_rob = new double[h_block * w_block * 10]; //���Ƃŏ���11/20/2006
	int r_index = 0;

	//�̈悷�ׂĂ����郋�[�v
	for(int j = d_area.s_h; j < (d_area.s_h + d_hight); j += block_size){
		for(int i = d_area.s_w; i < (d_area.s_w + d_width); i += block_size){
			sum_pre = sum_now = 0.0;
			sum_x = sum_y = sum_Cov = 0;
			ave_pre = ave_now = 0;
			sigma_x = sigma_y = sigma_xy = Cov_xy = 0.0;
			// ���ς����߂�
			for(int l = j; l < (j + block_size); l++){
				for(int k = i; k < (i + block_size); k++){
					//�w�i�摜�̋P�x�l�̍��v
					sum_pre += pre.Y(k,l);
					//���͉摜�̋P�x�l�̍��v
					sum_now += now.Y(k,l);
				}
			}
			//�w�i�摜�̕��ϒl
			ave_pre = sum_pre / block_num;
			//���͉摜�̕��ϒl
			ave_now = sum_now / block_num;

			for(int n = j; n < (j + block_size); n++){
				for(int m = i; m < (i + block_size); m++){
					diff_x = pre.Y(m,n) - ave_pre;
					diff_y = now.Y(m,n) - ave_now;

					//�w�i�摜�̂���l�̕��ςƂ̍��̓��a
					sum_x   += diff_x * diff_x;

					//���͉摜�̂���l�̕��ςƂ̍��̓��a
					sum_y   += diff_y * diff_y;

					//�w�i�Ɠ��͉摜�̋P�x�l�ƕ��ς̍����|�������̘̂a
					sum_Cov += diff_x * diff_y;
				}
			}

			//�w�i�摜�W���΍�  ��x
			sigma_x = sqrt(((double)sum_x / (double)(block_num - 1)));
			//���͉摜�̕W���΍���y
			sigma_y = sqrt(((double)sum_y / (double)(block_num - 1))); 
			//x��y�̋����U
			Cov_xy  = ((double)sum_Cov / (double)(block_num -1 ));  

			sigma_xy = sigma_x * sigma_y;

			if(sigma_xy == 0) sigma_xy = 1000000;

			//���֌W��r_rob�����߂�
			r_rob[r_index]   = Cov_xy / sigma_xy;

			if(r_rob[r_index] > 1.0)  r_rob[r_index] =  1.0;
			if(r_rob[r_index] < -1.0) r_rob[r_index] = -1.0;

			//�t�@�C���ɏ����o��
			//std::cout << Cov_xy << std::endl;
			//fprintf(fo,"[%d frame]\n",counter);
			//fprintf(fo,"sum_pre->%d sum_now->%d\n",sum_pre,sum_now);
			//fprintf(fo,"pre->%d now->%d\n",ave_pre,ave_now);	
			//fprintf(fo,"��x=%f ",sigma_x);
			//fprintf(fo,"��y=%f\n",sigma_y);
			//fprintf(fo,"�����U%f\n",Cov_xy);
			//fprintf(fo,"���֌W���F%lf\n",r_rob[r_index]);
			//fprintf(fo,"---------------------------------\n");

			r_index++;
		}
	}//�S�̃��[�v�I���

	double r_sum = 0.0;
	int i=0;
	for(i = 0 ; i < r_index ; i++){
		r_sum += r_rob[i];
		//fprintf(fo,"r,%lf\n",r_rob[i]);
	}
	//fprintf(fo,"---------------------------------\n");
	r_sum /= i;

	//fprintf(fo,"block,%lf,",r_sum);
	//fprintf(fo,"�u���b�N���֕���,%lf\n",r_sum);

	delete r_rob;

	return true;
}

//�̈�S�̂̑��ւ����߂�
bool Scene::AllCorr(bmp &pre,bmp &now)
{
	Area d_area = now.get_object_area();
	int Height = now.getheight();
	int Width = now.getwidth();
	unsigned long int M = Height * Width;  //��f��M
	double sigma_x,sigma_y;                //�W���΍���x,��y
	int  ave_pre,ave_now;                 //����摜�̋P�x�l�̕��ϒl
	long  sum_Cov = 0;                //x��y���ꂼ��̕��ςƍ��̍��v
	long  sum_pre = 0,sum_now = 0;    //����摜�̋P�x�l���v
	long sum_x = 0,sum_y = 0;        //�����̕��ςƂ̍��̍��v�l
	int diff_x,diff_y;//���ςƂ̍�
	double Cov_xy;//�����U
	double r_rob;//���o�X�g���֌W��r_rob
	unsigned long m = 0;
	static long int counter;//�t���[����
	++counter;//�t���[����

	//std::cout << d_area.s_h << std::endl;
	//std::cout << d_area.e_h << std::endl;
	//std::cout << d_area.s_w << std::endl;
	//std::cout << d_area.e_w << std::endl;

	//					for(int j=0; j < Height; j++){
	//				for(int i=0; i < Width; i++){
	////						for(int j = d_area.s_h; j < d_area.e_h; j++){
	////		for(int i = d_area.s_w; i < d_area.e_w; i++){
	//			if(now.isDiff(i,j) == OBJECT){
	//					now.R(i,j) = now.R(i,j) * 0.299 + now.G(i,j) * 0.587 + now.B(i,j) * 0.114;
	//					now.G(i,j) = now.R(i,j);
	//					now.B(i,j) = now.R(i,j);
	//
	//					pre.G(i,j) = pre.R(i,j);
	//					pre.B(i,j) = pre.R(i,j);
	//			}
	//				}
	//				}
	//
	//						now.save("now1.bmp");
	//						pre.save("pre1.bmp");

	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//for(int j = d_area.s_h; j < d_area.e_h; j++){
			//	for(int i = d_area.s_w; i < d_area.e_w; i++){
			if(now.isDiff(i,j) == OBJECT){
				now.Y(i,j) = now.R(i,j) * 0.299 + now.G(i,j) * 0.587 + now.B(i,j) * 0.114;
				pre.Y(i,j) = pre.R(i,j);
				m++;
			}
		}
	}

	// ���ς����߂�
	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//	for(int j = d_area.s_h; j < d_area.e_h; j++){
			//		for(int i = d_area.s_w; i < d_area.e_w; i++){
			if(now.isDiff(i,j) == OBJECT){
				//for(int j=0; j < Height; j++){
				//	for(int i=0; i < Width; i++){
				sum_pre += pre.Y(i,j);//�w�i�摜�̋P�x�l�̍��v
				sum_now += now.Y(i,j);//���͉摜�̋P�x�l�̍��v
			}
		}
	}	
	ave_pre = sum_pre / m; //�w�i�摜�̕��ϒl
	ave_now = sum_now / m; //���͉摜�̕��ϒl

	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//	for(int j = d_area.s_h; j < d_area.e_h; j++){
			//		for(int i = d_area.s_w; i < d_area.e_w; i++){
			if(now.isDiff(i,j) == OBJECT){
				//for(int j=0; j < Height; j++){
				//	for(int i=0; i < Width; i++){
				diff_x = pre.Y(i,j) - ave_pre;
				diff_y = now.Y(i,j) - ave_now;

				//�w�i�摜�̂���l�̕��ςƂ̍��̓��a
				sum_x   += diff_x * diff_x;

				//���͉摜�̂���l�̕��ςƂ̍��̓��a
				sum_y   += diff_y * diff_y;

				//�w�i�Ɠ��͉摜�̋P�x�l�ƕ��ς̍����|�������̘̂a
				sum_Cov += diff_x * diff_y;
			}
		}
	}
	//�w�i�摜�W���΍�  ��x
	sigma_x = sqrt((double)(sum_x / m ));
	//���͉摜�̕W���΍���y
	sigma_y = sqrt((double)(sum_y / m )); 
	//x��y�̋����U
	Cov_xy  = (double)(sum_Cov / m );  
	//���֌W��r_rob�����߂�
	r_rob   = Cov_xy / (sigma_x * sigma_y);

	//�l�����������̔�����
	if(sigma_x == 0 || sigma_y == 0){
		r_rob   = Cov_xy / 1000000;
	}
	if(r_rob > 1.0)  r_rob =  1.0;
	if(r_rob < -1.0) r_rob = -1.0;

	//fprintf(fo,"[%d frame]----------------------\n",counter);
	//fprintf(fo,"sum_pre->%d sum_now->%d\n",sum_pre,sum_now);
	//fprintf(fo,"pre->%d now->%d\n",ave_pre,ave_now);	
	//fprintf(fo,"��x=%f ",sigma_x);
	//fprintf(fo,"��y=%f\n",sigma_y);
	//fprintf(fo,"�����U%f\n",Cov_xy);
	//fprintf(fo,"���֌W���F%lf\n",r_rob);
	//fprintf(fo,"---------------------------------\n");
	//fprintf(fo,"�̈�S�Ă̑���,%lf\n",r_rob);
	//fprintf(fo,"All,%lf\n",r_rob);

	printf("���֒lr = %lf  \n",r_rob);

	return true;


	if(r_rob < 0.6 && r_rob > -0.6) return true;
	else if(r_rob == 1.0 || r_rob == -1.0) return true;
	else return false;

}
