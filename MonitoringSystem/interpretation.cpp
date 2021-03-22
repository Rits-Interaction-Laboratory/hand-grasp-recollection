#include "stdAfx.h"
#include "interpretation.h"

#include "MonitoringSystemDlg.h"
//common
#include "common.h"




#include "Util.h"
#include "geometory.h"
#include "objectInfo.h"
#include "Time.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
Cinterpretation::Cinterpretation(void)
{
}

Cinterpretation::~Cinterpretation(void)
{
}

//TODO:increment
/*	increment
 *	function	:
 *	arg			:image			:input	:
 *				:unsigned long	:frame	:
 *	return		:bmp			:det	:
 */
int Cinterpretation::increment(image &input, unsigned long frame, bmp &det)
{
	static layer lay;
	int Height = 240;//240
	int Width  = 320;//320
	static bmp output,output_val;
	std::string filename;

	const int b_threshold = 20;//������臒l

	static int del_objct_num = 0; //static�ɕύX
	bool no_exist = true;

	static int del_num=0;    //static�ɕύX

	

	


	//������������������������������������������������������������������������������
	// �����̌��m�����������摜�ɂ��ăV�[���̉��߂��s��
	// �e�ƂȂ�Ȃ��������̂����ɂ��āI�I

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//�ǂݍ��񂾃}�X�N�̕����ɂ��ĉ��̃I�u�W�F�N�g�Ƃ̏d�Ȃ蔻��
	//[check]��O�̗\�z��ԂƂ̔�r
	int delete_key = -1;
	bool same_flag = false;
	bool delete_flag = false;


	//object_img[i]�Ƃ��ׂĂɂ��ă}�b�`���O�I�I�_����(AND���Ƃ�)
	for(int index = 1; index < object_img.size(); index++){
		same_flag = check(input, object_img[index]);
		if(same_flag == true){//�e�}�X�N�Ƃ̃}�b�`���O���^�ɂȂ�����
			//�������C����ID
			delete_key = l_map2[index];
			del_objct_num = index;
			delete_flag = true;
			for(int j=0; j < Height; j++){
				for(int i=0; i < Width; i++){
					//end() ����begin()�܂ŒH�� = �ォ�猩���悤�ɂȂ�
					std::vector<int>::iterator it = lay.m_val(i,j).end();

					while( it != lay.m_val(i,j).begin()){
						--it;
						if(*it != -1){

							//�I�u�W�F�N�gID�������C����get
							int index2 = lay.get_layer(*it);

						}
					}
				}
			}
		}
	}
	cout <<"delete_key:"<<delete_key<<endl;	
	return delete_key;
}




double getDistanceFromAtoB(PII a, PII b){
	double dx = a.first - b.first;
	double dy = a.second - b.second;
	return sqrt(dx * dx + dy * dy);
}
PII getBestPosForOutputDetectedObjectInfo(PII _posDetectedObj){
	Point posDetectedObj(_posDetectedObj.first, _posDetectedObj.second);
	vector<Point> posNConnect;
	Point pos(1.0, 0);
	double angleRotate = M_PI / 180.;
	for(int i = 0 ; i < 8; i++){
		posNConnect.push_back(Geometory::rotate(pos, ((double)(i) * 45. * angleRotate)) + posDetectedObj);
		cout << "    (" << imag(posNConnect[i])<< "," << real(posNConnect[i]) << ")" << endl;
	}
	return PII();
}


//EventInfo getDetectedObjectInfomation(){
//	ifstream ifs("../DB/Detected-Object/object.log");
//	
//	if(ifs == NULL)cout <<"no"<<endl;
//
//	map<int,State> mapEventInfo;
//	int latestId;
//	BringInObjectList bringInObjList;
//	TakingOutObjectList takingOutObjList;
//
//	string line;
//	for(int i = 0 ; std::getline(ifs, line); i++){
//		Util::replaceAll(line,","," ");
//		stringstream ss(line);
//		string s;
//		int x1,y1,x2,y2,x3,y3,eventId,objId,frame;
//		double area;
//		string date;
//		for(int j = 0 ;(ss >> s); j++){
//			if(j == 0) frame = atoi(s.c_str());
//			else if(j == 1) x1 = atoi(s.c_str());
//			else if(j == 2) y1 = atoi(s.c_str());
//			else if(j == 3) x2 = atoi(s.c_str());
//			else if(j == 4) y2 = atoi(s.c_str());
//			else if(j == 5) x3 = atoi(s.c_str());
//			else if(j == 6) y3 = atoi(s.c_str());
//			else if(j == 7) area = (double)atof(s.c_str());
//			else if(j == 8) date = s;
//			//eventId
//			else if(j == 9){
//				eventId = atoi(s.c_str());//kawa 
//			}
//			else if(j == 10){
//				objId = atoi(s.c_str());
//				latestId = objId;
//			}
//		}
//		ObjectInfo objInfo(frame,Geometory::Point2D(x1,y1),Geometory::Point2D(x2,y2),Geometory::Point2D(x3,y3),area,date,eventId,objId);
//
//		cout << "(" << objInfo.getDetectedFrame() << "frame) " << "c(" << objInfo.getPosCenter().getX() << "," << objInfo.getPosCenter().getY() << ")"
//			<< "[" << objInfo.getDetectedEventKind() << "]" << objInfo.getID() << endl; 
//		//��������Ȃ�
//		if(eventId == EVENT_TAKING_OUT){
//			takingOutObjList.push_back(objInfo);
//			bringInObjList.erase(objId);
//		}
//		else if(eventId == EVENT_BRINGING_IN || eventId == EVENT_MOVE){
//			bringInObjList.push_back(objInfo);
//		}
//		/*det.SetPixColor(x,y,255,0,255);*/
//	}
//	return EventInfo(takingOutObjList, bringInObjList);
//}




//-----���C���[�\����p�������̂̎������ݥ�������襈ړ����m��������


int Cinterpretation::interpretation(image &input, unsigned long frame, bmp &det, int delete_key, int obj_id, int key_id, bool track_flag)
{
	static layer lay;//kawa�@�ێ����郌�C���[�\���̕����������݂�������Ηe�ʂ������Ă����B
	

	int Height = 240;
	int Width  = 320;
	static bmp output,output_val;//kawa    output���C���[�ʂɐF�������ꂽ�l   output_val�ォ�猩���l


//////////////////////////////////////////////////////////////////////////////
	//static bmp mask_img[10];//���I�z��ɕύX//kawa
	static std::vector<bmp>  mask_img;
	mask_img.reserve(30);
	object_img.reserve(30);
	static unsigned int laydata=1;
//////////////////////////////////////////////////////////////////////////////


	//bmp outputmask[10];//kawa�@�e�X�g�p
	std::string filename;

	const int b_threshold = 20;//������臒l

	static int del_objct_num = 0; //static�ɕύX
	bool no_exist = true;
	bool objId = false;

	static int del_num=0;    //static�ɕύX

	int count[320*240]={-1};
	int table[10]={-1};//���I�z��ɕύX//kawa

	//int out_objId = -1;

	FileOperator file;
	FILE *fl;

	int LayNum=0;

	fopen_s(&fl,"../DB/Detected-Object/object.log","a");

	std::cout <<  "key_id" <<key_id<<"obj__id"<<obj_id<<"delete_key"<<delete_key<< std::endl;

#if 1 //������������������������������������������������������������������������������
	// �����̌��m�����������摜�ɂ��ăV�[���̉��߂��s��
	// �e�ƂȂ�Ȃ��������̂����ɂ��āI�I

	Geometory::Point2D posCenter(input.get_xc(),input.get_yc());//���̂̏d�S
	//���̂̊O�ڒ����`
	Area o_area = input.get_area();

	//���̂̊O�ڒ����`�̒��_���W
	Geometory::Point2D posStart(0 + o_area.s_w, 0 + o_area.s_h);//(0,0)����_�ɕ��̂̊O�ڒ����`�̍�����W
	Geometory::Point2D posEnd(0 + o_area.e_w, 0 + o_area.e_h);//(0,0)����_�ɕ��̂̊O�ڒ����`�̉E�����W

	//���̖̂ʐ�
	int objArea = input.get_square();

	/*
	 * ���O�ɏ��������o��
	 * :�t�@�C����
	 * :���̂̏d�S���W(x,y)
	 * :���̂̊O�ڒ����`��1�̒��_���W(����)
	 * :���̂̊O�ڒ����`��1�̒��_���W(����)�̔��Α��̒��_���W(�E��)
	 * :���̖̂ʐ�
	 */
	string strCurTime = Util::Time().getCurrentTimeString();
	fprintf(fl,"%ld,%d,%d,%d,%d,%d,%d,%d,%s,",
		frame,
		posCenter.getX(), posCenter.getY(),
		posStart.getX(), posStart.getY(),
		posEnd.getX(), posEnd.getY(),
		objArea,
		strCurTime.c_str());
	std::string img_fname;
	char c[35];


	
	//���̍��������Z�[�u
	//TODO:error
	cout << "cvCreateImage(cvSize(" << input.getwidth() << "," << input.getheight() << ")," << IPL_DEPTH_8U << "," << 3 << ")" << endl; 

	IplImage *obj = cvCreateImage(cvSize(input.getwidth(),input.getheight()),IPL_DEPTH_8U,3);

	if(obj == NULL){
		cout << "error" << endl;
	}
	//�}�b�`���O��̏������ł���΂��Ƃ͂ł���I�I�I�I�I�I
	//std::cout <<  delete_key << std::endl;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		



	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//��苎������A�w�i���ς���Ă����Ƃ��̏���(�ЎR���̗�O�����̕���)
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//�������Ƃ����Ƃ������s�N�Z���ɂȂ��������ɂ��Ă͍ēx���x�����O


	//���͂�������I�I�I�I�I�I�I�I�I



	//�������C���̈���̃��C���̃I�u�W�F�N�g�Ƃ̍������Ƃ��Ď����ɑ������


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//�����v�f������f�ɂ��ă��C������
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//�w�肵���l�����郌�C�����ׂ�delete����
	//    ��
	//�S�Ẵ��C���[��������
	bool delete_flag = false;

	//�������C���ԍ����w��
	int pos = lay.get_layer(delete_key);



//�������莞�̗�O����
	if(delete_key != -1){
		int ex_pixel = 0;
		input.grayscale();//���͉摜��Z�W�摜 input.RGB�ɔZ�W�l������B
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				if(input.isDiff(i,j) == OBJECT){
					double diff = sqrt((double)((output_val.R(i,j) - input.R(i,j)) * (output_val.R(i,j) - input.R(i,j))));//kawa�Z�W�l�ɂ��w�i�̂����r?��O�����̕����H

					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = output_val.R(i,j);//�lval���͂����Ă���val=object_img[pos].R(i,j)
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
					if(diff < b_threshold){//����臒l�ȉ��Ȃ�w�i
						//hoge.SetPixColor(i,j,255,255,255);
					}
					else{//�w�i�ƍ������ł���(臒l:20)
						//input.isDiff(i,j) = B_OBJECT;
						//object_img[del_objct_num].isDiff(i,j) = B_OBJECT;
						object_img[pos].isDiff(i,j) = B_OBJECT;//
						ex_pixel++;//�B��Ă����s�N�Z���̗�
					}
				}else{
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = 0;
				}
			}
		}

		//���k�E�c��
		//object_img[del_objct_num].Contract(B_OBJECT);
		//object_img[del_objct_num].Expand(B_OBJECT);


		//std::cout <<  "������" << pos << "�Ԗڂ�����" << std::endl;
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				lay.val(i,j).erase(lay.val(i,j).begin() + pos);//���C���[���̍폜�����@
				lay.m_val(i,j).erase(lay.m_val(i,j).begin() + pos);//�}�X�N���̍폜
			}
		}

#if 0     //�������莞�̗�O����(�ЎR���C�_4�͂��)�ۑ������w�i�̏ꍇ �s�N�Z���̗ʂ�100�ȏ�̂Ƃ��̓��C���[�ɏ���ǉ�
		  //�����p������邽��тɂ��̏��������邱�Ƃ����Ȃ��Ȃ��Ă��܂����\��������̂Œ����K�v������(��{)

		if(ex_pixel > 100){
			//std::cout <<  "��O���������܂�" << std::endl;
			for(int j=0; j < Height; j++){
				for(int i=0; i < Width; i++){
					//if(object_img[del_objct_num].isDiff(i,j) == B_OBJECT){
					if(object_img[pos].isDiff(i,j) == B_OBJECT){
						//�e�N�X�`���̏���ǉ�
						int val = object_img[pos].R(i,j);
						lay.val(i,j).insert(lay.val(i,j).begin() + pos,val);
						int mask_id = l_map2[pos+1];
						lay.m_val(i,j).insert(lay.m_val(i,j).begin() + pos,mask_id);
					}
				}
			}

		}//��O����end
#endif
		lay.erase_layer(delete_key);
		delete_flag = true;
	}

	//�����I�u�W�F�N�g��ID
	//fprintf(fl,"%d,",delete_key);

	//
	////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	////���̗̈�ǐՂɂ�����
	////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//if(delete_flag != true){
	//	objId = in_obj_tracking(input,frame);
	//	if(objId == true){
	//		for(int i = 0; i < outId.size(); i++){
	//			out_objId = outId[i];
	//		}
	//	}
	//	outId.clear();
	//}
	//else{
	//	Pair* pf = new Pointair(x1,y1,x2,y2);
	//	pair.push_back(pf);
	//	outId.push_back(delete_key);
	//}

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//�}�X�N�摜���V�[���ɒǉ�
	//�����l������vector�^��push_back�Œǉ� insert
	//
	//�v�f�͎�����t���[�����ɂ��Ă����΂��ƂŌ����Ɏg����H����
	if(delete_flag == true){
		//fprintf(fl,"-1,");
        laydata--;//kawa
	}
	else {//�������݂̏ꍇ
		std::cout <<  "���C���ǉ�" << std::endl;
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				//�����f���I�u�W�F�N�g�������炻�̗v�f�̒l������
				if(input.isDiff(i,j) == OBJECT){
					//�e�N�X�`���̏���ǉ�
					int val = input.Y(i,j);//�Z�W�l
					lay.add(i,j,val);

					//�}�X�N�̏���ǉ�
					//lay.add_mask(i,j,obj_id);//kawa     ���̌ŗL�Ȃ�keyid�ق��ɓ���̏ꍇ��obj_id���g���Ă�?
					lay.add_mask(i,j,key_id);


					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = input.Y(i,j);//RGB�ɔZ�W�f�[�^����
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
				}
				//����ȊO�͉������� -1 ������
				else{
					lay.add(i,j,-1);
					lay.add_mask(i,j,-1);

					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = 0;
				}
			}
		}
		//�I�u�W�F�N�g��ID��ǉ�
	lay.set_layer(key_id);//l_arrey��ID��push����
	laydata++;
	}


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if(delete_flag){
		fprintf(fl,"%d,", EVENT_TAKING_OUT);
		fprintf(fl,"%d,",delete_key);
	}else{
		if(track_flag) {fprintf(fl,"%d,",(int)EVENT_MOVE);}
		else {fprintf(fl,"%d,",(int)EVENT_BRINGING_IN);}

		fprintf(fl,"%d,", key_id);
	}

	CString obj_fname,oframe;

	oframe.Format("%lu",frame);

	obj_fname = "../DB/Detected-Object/" + oframe + "/object.png";
	cvSaveImage(obj_fname,obj);
	obj_fname = "../DB/Detected-Object/" + oframe + "/input.png";
	bmp *bmpInput = (bmp*)(&input);
	bmpInput->save(obj_fname);
	cvReleaseImage(&obj);

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//��ԏォ�猩���I�u�W�F�N�g�̏d�Ȃ��\��
	//��ɕ����d�Ȃ邱�Ƃɂ���Č`���ς���Ă��܂����̏���
	//
	//������肠�Ƃɒǉ����ꂽ�I�u�W�F�N�g�ɒ��ڂ���

//	int num = lay.get_layer(lay.get_end())+1;//���C���̍ő吔
//	int l_max = lay.get_layer(lay.get_end())+1;
//	if(l_max==-1){l_max=0;}//kawa
	int num = laydata;//���C���̍ő吔
	int l_max = laydata;



	//LayNum = num;


	//�o�͉摜�̏����� //kawa
//	for(int i = 1;i <= 9;i++){//kawa
//		mask_img[i].init();
//		object_img[i].init();
//	}

	//mask_img object_img vecte������Ȃ炱���ő��₷

	clearobject_img();
    mask_img.clear();
	
	resizeobject_img(l_max);
    mask_img.resize(l_max);
    std::cout <<  "object����" <<object_img.size()<< std::endl;
	std::cout <<  "mask����" <<mask_img.size()<< std::endl;
	


	
	output.init();
	output_val.init();

	//���C������map���X�V�O�ɏ�����
	l_map.clear();
	l_map2.clear();

	if(num!=-1){//���C���[������ԈȊO�̎�
		no_exist = false;//���̃I�u�W�F�N�g�����݂���
		std::vector<int> all = lay.get_layer_all();
		std::vector<int>::reverse_iterator itr = all.rbegin();


		//map���ŐV��ԂɍX�V update
		//����ID�����C��
		//���C��������ID
		//�̑o�����ɃA�N�Z�X�ł���map������Ă���(�Ӗ������邩�͕s��)
		if(num > 0){
			while(num != 1){
				num--;
				
				cout<<num<<";"<< lay.returnl_array(num-1)<<endl;
				l_map.insert(std::pair<int,int>(num,lay.returnl_array(num-1)));
				l_map2.insert(std::pair<int,int>(num,lay.returnl_array(num-1)));

				
			}
		}
	}


//-----------------------------------------------------------------------
//�����ŉ摜�ʂ���̌��������̉������ォ�猩���l�Ƃ��ꂼ��̎������ݎ��̃I�u�W�F�N�g�̍�����ϐ��ɑ��

	//�P��f�̒l�����ł͉��w�ڂ��͕�����Ȃ�
	//�Q�l�摜�̎��̓e�N�X�`���̒l���S�ăI�u�W�F�N�gID�ɂȂ��Ă����̂ŊȒP
	//��
	//�������A�����ύX���Ȃ��Ƃ����Ȃ� 1/25/2007

	//�ꉞtop�r���[�̉摜�̂͂�!! output
	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//end() ����begin()�܂ŒH�� = �ォ�猩���悤�ɂȂ�
			int val = lay.end_val(i,j);//kawa�@-1�@���߂��Ă���\��������B?
			output_val.SetPixColor(i,j,val,val,val);
		}
	}

	//=================================================================//kawa??
	int t=0;
//	for(int k=9; k > 0 ; k--){//kawa
	for(int k=l_max-1; k > 0 ; k--){
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				det.B(i,j)=0;
				det.G(i,j)=0;//kawa
				det.R(i,j)=0;//kawa?

				int v = lay.get_val(i,j,k);
				object_img[k].SetPixColor(i,j,v,v,v);
			}
		}
	}
	//=================================================================


//-------------------------------------------------------------------------

#if 1//�ォ�猩�����̂̏d�Ȃ�����C�����ƂɐF�𕪂��Đ�������

	//�ő�̒l�A�ő僌�C������1��f���Ƃɉ��̑w��k��
	//������艺�̃I�u�W�F�N�g�ɑ΂���update
	//��ԍŉ��w�܂ō����h��Ԃ�
	//�������扽�w���m���Ă���Έ�ԉ�(��0�w)�܂œh��Ԃ���
	int laynum=0;
	int colorPattern[10][3]={	
							{128,128,128},//0
							{255,255,255},//1
							{  0,255,255},//2
							{255,  0,255},//3
							{255,255,  0},//4
							{  0,  0,255},//5
							{138,  0,255},//6
							{  0,128,255},//7
							{  0,128,128},//8
							{128,128,  0} //9
							};
  int index=0;

  for(int j=0; j <lay.get_layer(lay.get_end()); j++){cout<<"laynaiID:"<<j<<"����"<<lay.returnl_array(j)<<endl;}


	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//���̏��ԂŌ���Əォ�猩���悤�ɂȂ�
			for(std::vector<int>::reverse_iterator it = lay.m_val(i,j).rbegin();it != lay.m_val(i,j).rend();it++)
			{		
				    index=0;
					if(lay.get_layer(*it)!=-1){
					if(*it != -1){
						laynum++;
						
						//�I�u�W�F�N�gID�������C����get

						if(laynum==1){//kawa?
							
							index = lay.get_layer(*it);//key_id��Ԃ�
							//index=index%100;
							count[j*320+i]=index%10;								
							// shimada modified 2012.12.13
							output.SetPixColor(i,j,colorPattern[index%10][0],colorPattern[index%10][1],colorPattern[index%10][2]);
							mask_img[index].isDiff(i,j) = OBJECT;
							mask_img[index].SetPixColor(i,j,colorPattern[index%10][0],colorPattern[index%10][1],colorPattern[index%10][2]);
							//for(int c=index-1;c>0;c--){// shimada modified 2012.12.13 //kawa
							//	mask_img[c].SetPixColor(i,j,0,0,0);
							//    mask_img[c].isDiff(i,j)=0;
							//}

						}
						if(table[index]<laynum)
							table[index]=laynum;
					}
			}
			}
			if(laynum==0)	count[j*320+i]=-1;
			laynum=0;
		}
	}

	for(int j=0;j<240;j++){
		for(int i=0;i<320;i++){
			.SetPixColor(i,j,0,0,0);
			if(count[j*320+i]>=0){
				switch(table[count[j*320+i]]){
				case 1: det.SetPixColor(i,j,0,255,0);
					break;
				case 2: det.SetPixColor(i,j,255,0,0);
					break;
				case 3: det.SetPixColor(i,j,0,0,255);
					break;
				case 4: det.SetPixColor(i,j,255,255,0);
					break;
				case 5: det.SetPixColor(i,j,255,0,255);
					break;
				case 6: det.SetPixColor(i,j,0,255,255);
					break;
				default: det.SetPixColor(i,j,128,128,128);//kawa
					break;
				}
			}
			if(no_exist==true)
				det.SetPixColor(i,j,0,0,0);
		}
	}
#if 0 //���C���摜�̕ۑ�
	char fname2[30];
	sprintf(fname2,"%08lu-out2.png",frame);
	det.save(fname2);
#endif
#endif
	cout << __FUNCTION__ << " l_max:" << l_max << endl;
	//=============================================================
	//���C�����Ƃ̃e�N�X�`���Əォ�猩���}�X�N�Ƃ�AND���Ƃ�
	//��
	//����Ԃł̃}�b�`���O�Ɏg�p���镨�̗̂\�z���ꂽ�`���e�N�X�`���ɂȂ�
	//
	for(int k = 1;k < l_max ;++k){
//	for(int k=1; k<=9; ++k ) {
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				object_img[k].isDiff(i,j) = mask_img[k].isDiff(i,j);
				int r = lay.get_val(i,j,k);
				object_img[k].SetPixColor(i,j,r);
					
			}
		}
	}
	//=============================================================


//	for(int k=0;k<=9;k++){
	for(int k=1;k<l_max;k++){
		//���C�������̂̑Ή��t�@�C���쐬
		FILE *fp;
		CString file,tmp1,tmp2,tmp3;
		tmp1.Format("%lu",frame);
		tmp2.Format("%d",k);

		file = "../DB/Detected-Object/" + tmp1 + "/" + "layer.log";

		fopen_s(&fp,file,"a");
		fprintf(fp,"%d,",k);
		fprintf(fp,"%d\n",l_map2[k]);
		fclose(fp);

		file = "../DB/Detected-Object/" + tmp1 + "/" + tmp2 + ".png";
		object_img[k].save(file);
		file = "../DB/Detected-Object/" + tmp1 + "/" + "output_val.png";
		output_val.save(file);
	}

#if 0  //���ʉ摜�o��----------------------------//
	//���Ƃň�ʉ�����
	//
	char fname1[30];
	sprintf(fname1,"%08lu-out.png",frame);
	output.save(fname1);
	char filenameMask[30];
	for(int i=1;i<10;i++){
		sprintf(filenameMask,"mask-%30d.png",i);
		mask_img[i].save(filenameMask);
	}
	char filenameObject[30];
	//���Ԃ��֌W�Ȃ��Ȃ�(mask_img1�`9��������Ƃ�object1�`9�Ƃ��K�v���Ȃ��Ȃ�)��L�ɑg�ݍ���ł��悢(������)
	for(int i=1;i<10;i++){
		sprintf(filenameObject,"obj-%30d.png",i);
		object_img[i].save(filenameObject);
	}
#endif


	/*
	* �e��ID��ǉ�
	*/
	
	//if(interpre_flag == true)
	fprintf(fl,"%d\n",0);
	fclose(fl);
	//else
	//	fprintf(fl,"%d",1);


	//�S�Ẵf�[�^���������̂ŉ��s���Ď��̕��̉摜�̓��͂�҂�

	//�_�C�A���O��LOG�Ɍ��m�󋵂��o��+++++++++++++++++++++++++++++++++++++++++++++++

	/*
	* ���Ԏ擾
	*�u���m����:�v�̕����ɕ\����������
	*/

	char* hour = Util::string2Char(Util::Time().getCurrentTimeString("%H").c_str());
	char* minute = Util::string2Char(Util::Time().getCurrentTimeString("%M").c_str());

	//�V�[���E����ID�o��
	if(delete_flag==true){
		//��������
		//pParent2->SetScene(delete_key,frame,posCenter.getX(),posCenter.getY(),hour,minute,!delete_flag,track_flag);//kawa
		pParent2->SetScene(obj_id,frame,posCenter.getX(),posCenter.getY(),hour,minute,!delete_flag,track_flag);//�������ݏo�̖͂��
	}else{
		//��������
		pParent2->SetScene(obj_id,frame,posCenter.getX(),posCenter.getY(),hour,minute,!delete_flag,track_flag);
	}




	for(int i=o_area.s_w;i<o_area.e_w;i++){
		for(int j=0;j<3;j++){
			input.R(i,o_area.s_h-j) = input.R(i,o_area.e_h+j) = 0;
			input.G(i,o_area.s_h-j) = input.G(i,o_area.e_h+j) = 0;
			input.B(i,o_area.s_h-j) = input.B(i,o_area.e_h+j) = 0;
		}
	}
	for(int i=o_area.s_h;i<o_area.e_h;i++){
		for(int j=0;j<3;j++){
			input.R(o_area.s_w-j,i) = input.R(o_area.e_w+j,i) = 0;
			input.G(o_area.s_w-j,i) = input.G(o_area.e_w+j,i) = 0;
			input.B(o_area.s_w-j,i) = input.B(o_area.e_w+j,i) = 0;
		}
	}



	//char*����Mat��
	cv::Mat imgDet = Util::convertBmpChar2Mat(det);
	//int dx[] = {0,1,0,-1};
	//int dy[] = {1,0,-1,0};
	ObjectInfo objectInfo;
	PII posIdOutput(20,imgDet.rows - 20);

	EventInfo eventInfo = getDetectedObjectInfomation();//���̂�݂���
	cout << "bringIn:";
	for(int i = 0; i < eventInfo.bringInObjList.size(); i++){
		cout << "[" << eventInfo.bringInObjList.getInfomations()[i].getID() << "]";
	}
	cout << endl;
	cout << "takingOut:";
	for(int i = 0; i < eventInfo.takingOutObjList.size(); i++){
		cout << "[" << eventInfo.takingOutObjList.getInfomations()[i].getID() << "]";
	}
	cout << endl;
	std::vector<ObjectInfo>::iterator itrEnd = eventInfo.bringInObjList.end();
	//�o�^���Ă���C�x���g�������Ƀ��C����ʂ�id�ƃC�x���g���m���ʂ���̃��C����\������

	for(std::vector<ObjectInfo>::iterator itr = eventInfo.bringInObjList.begin();itr != itrEnd; itr++){
		Geometory::Point2D posObj = itr->getPosCenter();
		int id = itr->getID();
		cv::circle(imgDet, cv::Point(posObj.getX(), imgDet.rows - posObj.getY()), 3, cv::Scalar(255,0,255));
		//�ŐV��id�Ȃ物�F(0,255,255)�A������id�Ȃ率�F(255,0,255)�œh��
		cv::Scalar paintColor = (id == eventInfo.bringInObjList.getLatestDetectedObjId() ? cv::Scalar(0,255,255) : cv::Scalar(255,0,255));
		cv::line(imgDet, cv::Point(posObj.getX(), imgDet.rows - posObj.getY()), cv::Point(posIdOutput.first, imgDet.rows - posIdOutput.second), paintColor);
		cv::putText(imgDet, ("id:" + Util::toString(id)).c_str(), cv::Point(posIdOutput.first, imgDet.rows - posIdOutput.second), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,0), 1.5, CV_AA);
		posIdOutput.first += 35;//�\���ʒu��20(�K��)�����炷
	}
	//Mat����char*��
	Util::convertMat2BmpChar(imgDet, det);
	/*int numn = lay.layer_size(200,90);
	char msg[10];
	itoa(numn,msg,10);
	pParent->Disp_Log(msg);*/
	// TODO: ���C���[���o��
	pParent2->DetectOut();
	pParent2->LayerOut(&det.B(0,0));
	//�K�w���o�͂��ׂ��H�H�H

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
	return 0;
}

//TODO:���̌��m��臒l
bool Cinterpretation::check(image &now, bmp &pre)//input object_img
{
	int x1,y1,x2,y2;
	long same_pix = 0;//�����ꏊ�̉�f
	long now_num  = 0; //���݂̍����̐�
	long pre_num =0; //pre���̖̂ʐ�
	long total_num = 0; //OR
	int imageHeight = now.getheight();
	int imageWidth = now.getwidth();
	
	//############################################
	//���ڂ������x���̗̈�ɂ���
	//���̗̈�ɉߋ��ɂ������傫�����炢�̓������x����
	//�������̂����������̂����ׂ�
	//����΁@�{�P�@������΁@�|�P
	//�P�O�̃t���[���ƕ��̗̈�ɂ��ĉ�f�̐��𒲂ׂ�


	//cout << "[" << __FUNCTION__ << "]"  << "pre.new_objnum: " << pre.getNewObjNum() << endl;
	//TODO:"AND / OR ����芄���ȏ�Ȃ�"�ɏ����ς���


		//���C�������̂̑Ή��t�@�C���쐬


#if 0
	for(int j=0; j < imageHeight; j++){
		for(int i=0; i < imageWidth; i++){
			if(now.isDiff(i,j) == OBJECT){
				now_num++;
			}
			if(now.isDiff(i,j) != OBJECT &&  pre.isDiff(i,j) == OBJECT ){
				same_pix--;
			}
			if(now.isDiff(i,j) == OBJECT &&  pre.isDiff(i,j) == OBJECT ){
				same_pix++;	
			}
		}
	}

	//if(now_num < 15 || same_pix < 0) return 0;
	double hold = (double)(now_num * 0.05);
	cout << "diff: " << now_num << ":" << same_pix << endl;
	//臒l�ȓ��Ȃ畨�̌��m�������Ƃɂ���
	if(same_pix > hold){ return true; }
#else 
	//pre.isDiff(i,j) == OBJECT�͂Ȃ�
	for(int j=0; j < imageHeight; j++){
		for(int i=0; i < imageWidth; i++){
			if(now.isDiff(i,j) == OBJECT ||  pre.isDiff(i,j) == OBJECT ){
				total_num++;
			}
			if(now.isDiff(i,j) == OBJECT &&  pre.isDiff(i,j) == OBJECT ){
				same_pix++;	
			}
			if(now.isDiff(i,j) == OBJECT ) now_num++;
			if(pre.isDiff(i,j) == OBJECT ) pre_num++;
			
		}
	}

	if(now_num == 0){
		cout << "[" << __FUNCTION__ << "]" << "0 divide error" << endl;
		return false;	
	}
	else{
		float ratio = (float)same_pix / (float)total_num;
		cout << "[" << __FUNCTION__ <<"]"<< "same_pix:" << same_pix 
			<< " total_num:" << total_num << " now_num:"
			<< now_num << " pre_num:" << pre_num << endl;
		cout << "[" << __FUNCTION__ <<"]"<< "ratio:" << ratio*100 << "%" << endl;
		if(ratio > 0.6){
			return true;
		}
	}
#endif
	//������Ȃ�������
	return false;
}


unsigned long Cinterpretation::readLogFile(string filename){
	std::ifstream ifs(filename);
	char buff[82];
	int result = 0;
	while(ifs.getline(buff,80)>0){
		result = atoi(buff);
	}
	ifs.close();
	return(unsigned long)result;
}
unsigned long Cinterpretation::search_folder(unsigned long frame)
{
	return readLogFile("../DB/Human-DB/human.log");
}

unsigned long Cinterpretation::search_file(unsigned long frame)
{
	return readLogFile("../DB/Human-DB/human2.log");
}

//�������݃`�F�b�N
//TODO:�������݃`�F�b�N
bool Cinterpretation::in_obj_tracking(image &first, unsigned long now_frame,std::vector<Pair*> pair,unsigned long leaveframe)
{
	IplImage *input2;
	char file[128];
	char tmp[10];

	//unsigned long i = now_frame-1;
	unsigned long i = search_file(now_frame);//�Ō�̃t���[��
	bool eflag = false;
	bool mflag = false;

	int x1, y1, x2, y2;
	
	int xs,ys,xl,yl;

	//CamShift�֌W+++++++++++++++++++++++++++++++++++++++++++++++++++
	IplImage *image=0,*hsv=0,*hue=0,*mask=0,*backproject=0, *back;
	CvHistogram *hist=0;

	int track_object=0;
	CvRect selection;//�̈�x,y,width,height
	CvRect track_window;
	CvConnectedComp track_comp;//�A������ /* �Z�O�����g�����ꂽ�A�������̖ʐ� */ /* �Z�O�����g�����ꂽ�A�������̃O���[�X�P�[���l *//* �Z�O�����g�����ꂽ�A��������ROI */
	int hdims=16;
	float hranges_arr[]={0,180};
	float *hranges = hranges_arr;
	int vmin=10,vmax=256,smin=30;
	CvBox2D track_box;//��]���l�����ꂽ2�����̔�

	//�ǉ��i�Ώە��̂̎�����l���j
	CvRect selection2;
	IplImage *hue2=0, *mask2=0;
	CvHistogram *hist2=0, *hist_original=0;
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if(pair.size()==0)
	{cout<<"��������̃f�[�^������܂���"<<endl;
		return false;}
	for(int c = 0; c < (int)pair.size(); c++){//��������̈�
		x1 = pair[c]->get_x();
		y1 = 240 - pair[c]->get_y();
		x2 = pair[c]->get_vx();
		y2 = 240 - pair[c]->get_vy();
	}


	//�ߋ��ɑk���Ēǐ�
	while(1){
		//�����l�̐ݒ�
		unsigned long frame_human = search_file(now_frame);
		cout << "    " << i << ":" << frame_human << endl;
		if(frame_human == 0) {
			cerr <<"frame human break" << endl;
			break;
		}
		if(i == frame_human){
			Area o_area = first.get_area();
			selection.x = o_area.s_w;//�������ݗ̈�
			selection.y = 240 - o_area.e_h;
			selection.width=o_area.e_w-o_area.s_w;
			selection.height=o_area.e_h-o_area.s_h;
			track_object=-1;
		
		}

		//�摜�ǂݍ���
		/*if(mode==ONLINE){
			sprintf_s(file,"\\\\Proliant2/okamoto/Human-DB/Original/");
		}else if(mode==OFFLINE){*/
//			sprintf_s(file,"../DB/Human-DB/Original/");
		//}
		cout << "now_frame:" << i << endl;
		sprintf_s(file, "../DB/Human-DB/Original/%lu/%08lu.%s", search_folder(now_frame), i, "png");

		//�摜���Ȃ�
		if((input2 = cvLoadImage(file,CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR)) == NULL){
			if(eflag==true && i < search_folder(now_frame)){ //�I��
				cout << "no image" << endl;
				eflag = false;
				break;
			}
			else{
				//break;
				eflag = true;
				i--;
				continue;
			}
		}

		//�ŏ��̗̈�m��
		if( !image ){
			image = cvCreateImage( cvSize(320,240), 8, 3 );
			cvZero(image);
			image->origin = input2->origin;
			hsv = cvCreateImage( cvGetSize(image), 8, 3 );
			hue = cvCreateImage( cvGetSize(image), 8, 1 );
			mask = cvCreateImage( cvGetSize(image), 8, 1 );
			backproject = cvCreateImage( cvGetSize(image), 8, 1 );
			hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );  //�q�X�g�O�����̍쐬

			hue2 = cvCreateImage( cvGetSize(image), 8, 1 );
			mask2 = cvCreateImage( cvGetSize(image), 8, 1 );
			hist2 = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
			hist_original = cvCreateHist(1,&hdims,CV_HIST_ARRAY,&hranges,1);
			track_window = selection;
		}

		//�ǂݍ��񂾉摜��image�ɃR�s�[���AHSV�\�F�n�ɕϊ�����hsv�Ɋi�[
		cvCopy(input2,image,NULL);
		cvCvtColor(image,hsv,CV_BGR2HSV);

		////hsv�摜�̊e��f���l�͈͓̔��ɓ����Ă��邩�`�F�b�N���A�}�X�N�摜���쐬
		/*cvInRangeS(hsv,cvScalar(0,50,50,0),
					cvScalar(180,256,200,0),mask);*/

		//hsv�摜�̂����AH�`�����l����hue�Ƃ��ĕ���
		cvSplit(hsv,hue,0,0,0);

		//�����摜�̓ǂݍ���
		/*if(mode==ONLINE){
			sprintf_s(file,"\\\\Proliant2/okamoto/Human-DB/Original/");
		}else if(mode==OFFLINE){*/
			sprintf_s(file,"../DB/Human-DB/Process/");
		//}
		sprintf_s(tmp,"%lu",search_folder(now_frame));
		strcat_s(file,tmp);
		strcat_s(file,"/");
		sprintf_s(tmp,"%08lu",i);
		strcat_s(file,tmp);
		strcat_s(file,".png");
		back = cvLoadImage(file,CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);

		//�}�X�N����
		/*int search_xmin = track_window.x-20;
		if(search_xmin<0)	search_xmin = 0;
		int search_xmax = track_window.x+track_window.width+20;
		if(search_xmin>320)	search_xmin = 255;
		int search_ymin = track_window.y-20;
		if(search_ymin<0)	search_ymin = 0;
		int search_ymax = track_window.y+track_window.height+20;
		if(search_ymax>240)	search_ymax = 240;*/
		for(int k=0;k<240;k++){
			for(int l=0;l<320;l++){
				/*if(l>search_xmin && l<=search_xmax &&
					k>=search_ymin && k<=search_ymax){*/
					//if(l>selection.x && l<=selection.x+selection.width &&
					//	k>=selection.y && k<=selection.y+selection.height)
					//	//cvSetReal1D(&mask->imageData,k*mask->width+l,255);
					//	mask->imageData[k*mask->width+l] = 255;
					if((unsigned char)(back->imageData[(back->widthStep*k+l*3)])==255)
						//cvSetReal1D(&mask->imageData,k*mask->width+l,255);
						mask->imageData[k*mask->width+l] = 255;
					else if((unsigned char)(back->imageData[(back->widthStep*k+l*3)+1])==255)
						//cvSetReal1D(&mask->imageData,k*mask->width+l,255);
						mask->imageData[k*mask->width+l] = 255;
					else
						//cvSetReal1D(&mask->imageData,k*mask->width+l,0);
						mask->imageData[k*mask->width+l] = 0;
				/*}
				else
					mask->imageData[k*mask->width+l] = 0;*/
			}
		}

		//�ǐ՗̈�̃q�X�g�O�����v�Z
		if(track_object<0){  //�ǐՊJ�n
			float max_val; //�q�X�g�O�����̍ő�p�x

			int around = (sqrt((double)(selection.width+selection.height)*(selection.width+selection.height)/4 
					+ selection.width*selection.height) - (selection.width+selection.height)/2) / 2;


			int dx=selection.x,dy=selection.y,dh=selection.height,dw=selection.width;

			xs = max(0,dx-around);
		    ys = max(0,dy-around);
		    xl = min(320,dw+around*2);
		    yl = min(240,dh+around*2);
		    selection2.x = xs;
		    selection2.y = ys;
			selection2.width = xl;
		    selection2.height = yl;



			//�ߖT�̑ΏۊO�̃q�X�g�O�����v�Z
			//selection2.x = selection.x-around;
			//selection2.y = selection.y-around;
			//selection2.width = selection.width+around*2;
			//selection2.height = selection.height+around*2;
			cvCopy(hue,hue2,0);//hue->original��h�̒l
			cvCopy(mask,mask2,0);//mask1->process��0�A255�̒l
			//�w�i�v�Z�̂��߁A�Ώۗ̈�̎���̂݌v�Z����悤�Ƀ}�X�N����
			for(int k=selection2.y;k<selection2.y+selection2.height;k++){	if(k==240){break;}//kawa
				for(int l=selection2.x;l<selection2.x+selection2.width;l++){if(l==320){break;}//kawa
					if(l>=selection.x && l<=selection.x+selection.width && 
						k>=selection.y && k<=selection.y+selection.height)
							mask2->imageData[(k*mask2->width+l)] = 0;
					else
						mask2->imageData[(k*mask2->width+l)] = 255;
				}
			}
			//�Ώە��̂̎��ӂ̃q�X�g�O�����̌v�Z
			cvSetImageROI(hue2,selection2);//(�摜, ��`���)
			cvSetImageROI(mask2,selection2);
			cvCalcHist(&hue2,hist2,0,mask2);//�摜�i�Q�j�̃q�X�g�O�������v�Z����,(���͉摜�Q�i CvMat** �`���ł��\��Ȃ��j�D�S�ē����T�C�Y�E�^�C�v�D,�q�X�g�O�����ւ̃|�C���^�D ,�����}�X�N�D���͉摜���̂ǂ̃s�N�Z�����J�E���g���邩�����肷��D)

			cvResetImageROI(hue2);
			cvResetImageROI(mask2);

			//�Ώە��̂̃q�X�g�O�����v�Z
			cvSetImageROI(hue,selection);
			cvSetImageROI(mask,selection);
			cvCalcHist(&hue,hist,0,mask);

			//�Ώە��̂̍ŏ��̃q�X�g�O�������L��
			cvCopyHist(hist,&hist_original);

			cvGetMinMaxHistValue(hist,0,&max_val,0,0);//�Ώە��̂̍ő�l�C�ŏ��l�����r�������߂�
			cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );//�z��ɑ΂��ĔC�ӂ̐��`�ϊ����s���܂�.�ő�l���ɒ����H
			cvGetMinMaxHistValue( hist2, 0, &max_val, 0, 0 );//���ӗ̈�
			cvConvertScale( hist2->bins, hist2->bins, max_val ? 255. / max_val : 0., 0 );

//-----------------------------------------------------------------------------------
			for(int k=0;k<hdims;k++){//hdims=16
				double re = cvGetReal1D(hist->bins,k)-(cvGetReal1D(hist2->bins,k));
				if(re<0)	re=0;
				cvSetReal1D(hist->bins,k,re);
			}
//-----------------------------------------------------------------------------------

			cvGetMinMaxHistValue(hist,0,&max_val,0,0);
			//�q�X�g�O�����̏c��(�p�x)��0-255�̃_�C�i�~�b�N�����W�ɐ��K��
			if(max_val==0.0)
				cvConvertScale(hist->bins,hist->bins,0.0,0);
			else
				cvConvertScale(hist->bins,hist->bins,255.0 / max_val,0);
			//hue,mask�摜�ɐݒ肳�ꂽROI�����Z�b�g
			cvResetImageROI(hue);
			cvResetImageROI(mask);
			track_window = selection;//
			track_object=1;
		}//if(track_object<0){  //�ǐՊJ�n



		//�ߖT�̑ΏۊO�̃q�X�g�O�����v�Z
		int around = (sqrt((double)(selection.width+selection.height)*(selection.width+selection.height)/4 
				+ selection.width*selection.height) - (selection.width+selection.height)/2) / 2;

		int dx=selection.x,dy=selection.y,dh=selection.height,dw=selection.width;
		xs = max(0,dx-around);
	    ys = max(0,dy-around);
		xl = min(320,dw+around*2);
		yl = min(240,dh+around*2);
		selection2.x = xs;
		selection2.y = ys;
		selection2.width = xl;
		selection2.height = yl;

		cout << "selection2" <<selection2.x<<","<<selection2.y<<","<<selection2.width<<","<<selection2.height<< endl;

		//selection2.x = track_window.x-around;
		//selection2.y = track_window.y-around;
		//selection2.width = track_window.width+around*2;
		//selection2.height = track_window.height+around*2;
		cvCopy(hue,hue2,0);
		cvCopy(mask,mask2,0);
		//�w�i�v�Z�̂��߁A�Ώۗ̈�̎���̂݌v�Z����悤�Ƀ}�X�N����
		for(int k=selection2.y;k<selection2.y+selection2.height;k++){if(k==240){break;}//kawa
			for(int l=selection2.x;l<selection2.x+selection2.width;l++){if(l==320){break;}//kawa
				if(l>=track_window.x && l<=track_window.x+track_window.width && 
					k>=track_window.y && k<=track_window.y+track_window.height)
						mask2->imageData[(k*mask2->width+l)] = 0;
				else
					mask2->imageData[(k*mask2->width+l)] = 255;
			}
		}
		cvSetImageROI(hue2,selection2);
		cvSetImageROI(mask2,selection2);

		cvCalcHist(&hue2,hist2,0,mask2);
		cvResetImageROI(hue2);
		cvResetImageROI(mask2);

		for(int k=0;k<hdims;k++){
			double re = cvGetReal1D(hist_original->bins,k)-(cvGetReal1D(hist2->bins,k));
			if(re<0)	re=0;
			cvSetReal1D(hist->bins,k,re);
		}

		float max_val = 0.f;
		cvGetMinMaxHistValue(hist,0,&max_val,0,0);
		//�q�X�g�O�����̏c��(�p�x)��0-255�̃_�C�i�~�b�N�����W�ɐ��K��
		if(max_val==0.0)
			cvConvertScale(hist->bins,hist->bins,0.0,0);
		else
			cvConvertScale(hist->bins,hist->bins,255.0 / max_val,0);

		//�o�b�N�v���W�F�N�V�������v�Z����
		cvCalcBackProject(&hue,backproject,hist);//(���͉摜�Q�i CvMat** �`���ł��\��Ȃ��j�D���ׂē���T�C�Y�C����^�C�v�A�o�͂̃o�b�N�v���W�F�N�V�����摜�D���͉摜�Q�Ɠ����^�C�v,�q�X�g�O�����D)
		cvAnd(backproject,mask,backproject,0);
		sprintf_s(file,"../backproject/");
		sprintf_s(tmp,"%08lu",i);
		strcat_s(file,tmp);
		strcat_s(file,".png");
		cvSaveImage(file,backproject);
		
		//CamShift�ɂ��ǐ�(�I�u�W�F�N�g�q�X�g�O�����̃o�b�N�v���W�F�N�V���� �icvCalcBackProject���Q�Ɓj�D�����T���E�B���h�E�D���ʂƂ��ē�����\���́D���������T���E�B���h�E�̍��W�icomp->rect �t�B�[���h�j�C����уE�B���h�E���̑S�s�N�Z���̍��v�l�icomp->area �t�B�[���h�j���܂܂��D �I�u�W�F�N�g�̊O�ڋ�`�DNULL�łȂ��ꍇ�C�I�u�W�F�N�g�̃T�C�Y�Ǝp�����܂܂��D   )
		int res = cvCamShift( backproject, track_window,
				 cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
				 &track_comp, &track_box );
		cout << "res:" << res <<";track_box:"<<track_box.angle<<","<<track_box.center.x<<","<<track_box.center.y<<","<<track_box.size.height<<","<<track_box.size.width<< endl;
		//camshift�ǐՂɎ��s�����ꍇ�I��
		if(res < 0) {
			cout << "camshift dont track" << endl;
			break;
		}
		if(&track_box == NULL){
			cout << "no object(" << track_box.center.x << "," << track_box.center.y << ")"  << endl;
		}else{
			cout << "find object(" << track_box.center.x << "," << track_box.center.y << ")" << endl;
		}


		track_window = track_comp.rect;

		//if( backproject_mode )
		//	 cvCvtColor( backproject, image, CV_GRAY2BGR );
		 if( image->origin )
			 track_box.angle = -track_box.angle;



		 if( track_box.center.x>10&& 
		    track_box.center.y>10&& 
			track_box.size.height>10&&
			track_box.size.width>10)
		{cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );}//�ɏ��̏ꍇ�������ɃX���[������


		 int margin=25;
		 if(track_box.center.x >= x1-margin && track_box.center.x <= x2+margin
			 && track_box.center.y >= y2-margin && track_box.center.y <= y1+margin)
			 mflag = true;
		 else
			 mflag = false;

		if(mflag){
			cout << "tracking" << endl;
			for(int i = 0; i < (int)pair.size(); i++){
				x1 = pair[i]->get_x();
				y1 = 240 - pair[i]->get_y();
				x2 = pair[i]->get_vx();
				y2 = 240 - pair[i]->get_vy();
				cout << x1 << "," << x2 << " " << y1 << "," << y2 << endl;
				cout << track_box.center.x << "," << track_box.center.y << endl;
			}
		}
		if(!pair.empty()){
			if(mflag == false){
				cvRectangle (image, cvPoint (x1, y2),
					cvPoint (x2, y1), CV_RGB (0, 255, 255), 1);
			}
			else{
				cvRectangle (image, cvPoint (x1, y2),
					cvPoint (x2, y1), CV_RGB (255, 0, 0), 1);
			}
		}

		 /*if( select_object && selection.width > 0 && selection.height > 0 )
		 {
			cvSetImageROI( image, selection );
			cvXorS( image, cvScalarAll(255), image, 0 );
			cvResetImageROI( image );
		 }*/

		//�E�B���h�E�̐���
		cvNamedWindow("object-tracking",1);
		cvMoveWindow("object-tracking",600,480);

		//���ʕۑ�
		/*CvPoint pt1,pt2;
		pt1.x = selection.x;	pt1.y = selection.y;
		pt2.x = selection.x + selection.width;
		pt2.y = selection.y + selection.height;
		cvRectangle(image,pt1,pt2,CV_RGB(0,255,0),2,8,0);
		pt1.x = selection2.x;	pt1.y = selection2.y;
		 pt2.x = selection2.x + selection2.width;
		 pt2.y = selection2.y + selection2.height;
		 cvRectangle(image,pt1,pt2,CV_RGB(0,0,255),2,8,0);*/
		sprintf_s(file,"../result/");
		sprintf_s(tmp,"%08lu",i);
		strcat_s(file,tmp);
		strcat_s(file,".png");
		cvSaveImage(file,image);

		//�摜�\��
		cvShowImage("object-tracking",image);
		cvWaitKey(1);

		if(i==search_file(now_frame)){
			cvSaveImage("result.png",image);
		}

		i--;
		eflag = false;
		//�I������(�������݂Ɣ��肳���ΏI��)

		//�������́A���S�ɂ��߂��ƕ���������I��
		if(i == search_folder(now_frame) + 2){
			cerr << "search folder break" << endl;
			break;
		}

		//�Ō�̎�������̃t���[���n�_�܂ők��ΏI��//kawa(���l�����͐��m�Ȏ������莞�̌덷)
		if(i==(leaveframe-19))
		{   
			cerr<<"Last leave frame "<<endl;
		    cout<<"�Ō�̎�������n�_�܂Ŗ߂�܂����B"<<endl;
			break;
		}
		//�������݂Ǝ�������̂������Ől�����r���œr�؂�Ă���ꍇ�ǐՕs��//kawa
		if(leaveframe<search_folder(now_frame))
		{ mflag = false;
		cvDestroyWindow("object-tracking");
		  cerr<<"Last human frame "<<endl;
		  cout<<"�ŋ߂̎������݂Ǝ�������ɓ���̐l�����֘A���Ă��Ȃ��\��������܂��B"<<endl;
		  return false;
		 }


	}//while(1){
	//�E�B���h�E�̔j��
	cvDestroyWindow("object-tracking");
	if(mflag)cout << "end" << endl;
	return mflag;
}