#include "stdafx.h"
#include "MonitoringSystemDlg.h"
#include"EventSearch.h"
#include <queue>
#include <map>
#include <vector>
#include <fstream>
#include "FileOperator.h"
#include <sstream>
#include <algorithm>
#include <time.h>

int hide_count=1;
int drink_count=4;
Event::Event(void)
{
}

Event::~Event(void)
{
}
//
//--------------------------------------------------
// Name     :EventSearch()
// Author�@ : ��{
// Update   : 2013.8.20
// Function : object�Ō��m�������̂Ɛl�̃C�x���g����
// Argument : input image�N���X
// return   : ���m�����C�x���g�̐���Ԃ��B
//--------------------------------------------------
bool Event::EventSearch(unsigned long frame, image &input, bool object_detection, bool human_detection, bool *touch_flag)
{
	key_idname.reserve(50);//�ގ�id
	obj_idname.reserve(50);//����[��id 
	input.tracker.reserve(50);


	bool id_flag=loadObjectId();//�o�^����Ă���ID��ǂݍ���	
	int leave_id=-1;
	bool track_flag=false;
	int surf_id=-1;
	static int id_num=0;    // ��������ID���d�����Ďg�������Ȃ��ꍇ�͂����ύX�I�I�I
	static std::vector<int>cou;
	input.Eventset(input.Event_size());
	
	input.tracker_img.clear();
	input.tracker_img.reserve(50);

	d_human_maching(input,frame);//LV3�l���ǐ�

for(int num=0;num<input.Event_size();num++)
{
	//cout << "leave_id "<< endl;
//---�������蔻��------------------------------------------
	if(id_flag)leave_id=increment(frame,input,input.Event_Log(num));//model_id��Ԃ�
//---------------------------------------------------------

	if(leave_id==-1)
	{	//���̌��m���ꂽ���̂��ׂĂɑ΂��ăC�x���g���߂������Ȃ��B
		
		surf_id=surf.Pointing(frame,input, leave_id,key_idname,obj_idname,input.Event_Log(num));
		//�����ʂ̏ƍ����m���ꂽ���̂̏ƍ����s���B
		//���̒ǐ�
		

		for(int t=input.tracker.size()-1;t>=0;t--)
		{
			//cout << "track_flag "<< endl;
			if(input.tracker.size()>0)
			{
				//track_flag = in_obj_tracking(input,frame,t,num,input.tracker[t]);//�Ή����鎝������͈͂�traker���L�^����
			}
			if(track_flag)
			{
				surf_id=key_idname[input.tracker[t]->get_id()];
				input.tracker.erase(input.tracker.begin()+t);
				input.set_Event_Eve(num,2);//�ړ�
				t=-1;
			}

			else if(frame-input.tracker[0]->get_frame()>1000)
			{
				input.tracker.erase(input.tracker.begin());
				t--;
			}
			///cout << "track_flagrnd "<< endl;

		}


		if(!track_flag)
		{
			input.set_Event_Eve(num,1);//��������
		}

		cou.push_back(1);
		if(surf_id>=(int)key_idname.size())
		{
			//key_idname.push_back(surf_id);//���̂̌���Id
			//obj_idname.push_back(surf_id);//����ID	
            key_idname.push_back(surf_id);//���̂̌���Id
			obj_idname.push_back(id_num);//����ID
			id_num++;
		}
		else
		{
			key_idname.push_back(surf_id);
			obj_idname.push_back((obj_idname[surf_id]));
			cou[obj_idname[surf_id]]++;
		}
		
		//std::cout<<surf_id<<endl; 

	    input.set_Event_key(num,key_idname.back()) ;//�Ō�̗v�f��Ԃ�
		input.set_Event_obj(num,obj_idname.back()) ;
	


		std::ofstream ofs( "../DB/output-img/map.txt", std::ios::app );
		ofs << input.get_Event_key(num) << "\t";
		ofs << input.get_Event_obj(num)<< "\t";
		ofs << cou[obj_idname[surf_id]]<< "\t";
		ofs << endl;
		ofs.close();
		//�󂾂����ꍇ�͒ǉ�
		//�C�x���g�f�[�^��ۑ��@

		//id_lay.push_back(obj_idname.size());//�������莞�Ɏg�p

		//input.tracker.clear();
		input.set_Event_model(num,(obj_idname.size()-1));//input.Event_Log(num);
	}
	else
	{
	//�Ō�Ɏ������肪���ꂽ�t���[���ԍ���͈͂Ȃǂ̏���ۑ��i�ǐՂȂǂŎg�p���邽�߁j
		Pair* pf = new Pair(input.get_ObjectTab_s_w(input.Event_Log(num)),input.get_ObjectTab_s_h(input.Event_Log(num)),input.get_ObjectTab_e_w(input.Event_Log(num)),input.get_ObjectTab_e_h(input.Event_Log(num)));
		input.tracker.push_back(pf);
		input.tracker[input.tracker.size()-1]->set_frame(frame);
		input.tracker[input.tracker.size()-1]->set_id(leave_id);
		input.set_Event_Eve(num,0);
		input.set_Event_key(num,key_idname[leave_id]);
		input.set_Event_obj(num,obj_idname[leave_id]);
		input.set_Event_model(num,leave_id);
		surf_id=leave_id;
		
		//leave_id���ǂ̂Ԃ�����ID����\���l�ɕς���
	}
	
	//cout << " interpretation"<< endl;
    input.set_Event_frame(num,frame);		
	input.set_Event_s_w(num,input.get_ObjectTab_s_w(input.Event_Log(num)));
    input.set_Event_e_w(num,input.get_ObjectTab_e_w(input.Event_Log(num)));
	input.set_Event_s_h(num,input.get_ObjectTab_s_h(input.Event_Log(num)));
	input.set_Event_e_h(num,input.get_ObjectTab_e_h(input.Event_Log(num)));
	input.set_Event_Aframe(num,frame);
	input.set_Event_cx(num,input.get_ObjectTab_cx(input.Event_Log(num)));
	input.set_Event_cy(num,input.get_ObjectTab_cy(input.Event_Log(num)));

	cout << " Makefile"<< endl;

	//SaveObjData(input,frame,input.Event_Log(num),num,leave_id,input.get_Event_model(num));//���̏��ۑ��t�H���_�쐬
	std::ostringstream dir,frame_name,model_name;
	std::string dir_name;
	//std::string dir_name2;
	FileOperator path;
	dir  << "../DB/Detected-Object/" << frame<<"-"<<input.get_Event_model(num)<< "/";
	frame_name << frame<<"-"<<input.get_Event_model(num);
	dir_name = "../DB/Detected-Object/" + frame_name.str();
	path.MakeDir(dir_name.c_str());
	dir_name.clear();
	model_name <<input.get_Event_model(num);
	//TODO:write Object image(output image)
	dir_name = "../Object/" + model_name.str();
	path.MakeDir(dir_name.c_str());
	dir_name.clear();

	interpretation(input,frame,input.Event_Log(num),num,leave_id);
	std::cout<<"interpretationend"<<endl;
	//���̂̏����폜

	//if(human_detection)
}
	//�������񂾕��̂̍X�V
    object_reload(input,frame);      
	//cout<<"object_reload_end"<<endl;

	if(input.HumanData()>0)
	{
		//cout << " touch"<< endl;
		touch_event(input,frame, touch_flag);					// �ڐG���m�����I�I
	    //cout << "style_detect"<< endl;
		style_detect(input,frame);
		//cout << "style_detect_end"<< endl;
	}
	for(int num=0;num<input.Event_size();num++)
	{
		if(input.get_Event_Eve(num)!=0){leave_id=-1;}else{leave_id=1;}
		SaveObjData(input,frame,input.Event_Log(num),num,leave_id,input.get_Event_model(num));//���̏��ۑ��t�H���_�쐬
	}

	key_idname.clear();
	obj_idname.clear();
	//input.Event_releace();
	if (input.EventMAX()>0)return true;

	return false;
}

//--------------------------------------------------
// Name     :loadObjectId()
// Author�@ : ��{
// Update   : 2013.8.20
// Function : �o�^�������f���@�h�c�@obj_idname�Akey_idname������t����
// Argument : input image�N���X
// return   : key_idname��id������t�����Ȃ������ꍇfalse
//--------------------------------------------------
bool Event::loadObjectId() {

	// ����ID�Ǝ��ʂ��i�[�����t�@�C�����J��
	std::ifstream ofs( "../DB/output-img/object.txt");
	// 1�s���ǂݍ��݁A����ID->���̖���map���쐬
	string line;
	while (getline(ofs, line, '\n')) {
		// �^�u�ŕ��������������ldata�֊i�[
		istringstream ss(line);
		string s;
		for (int j = 0; getline(ss, s, '\t'); j++) {
			// ����ID�ƃL�[�|�C���g���𒊏o���Ċi�[
			if(j == 2){}
			else if(j == 1) obj_idname.push_back(atoi(s.c_str()));
			else if(j == 0) key_idname.push_back(atoi(s.c_str()));
		}
		//int KeypointId = atol(ldata[0].c_str());
		//idname[KeypointId] = ObjId;
		//for(int j = 0; j < keypoint ; j++)labels.push_back(objId);
		//id2name.insert(map<int, int>::value_type(objId, keypoint));
	}

	return (key_idname.size() > 0);

}

//bool Object::loadmap(std::vector<int> &Obj, std::vector<int> &Num) {
//	// ����ID�ƕ��̖����i�[�����t�@�C�����J��
//	std::ifstream ifs( "../DB/output-img/map.txt");
//
//	// 1�s���ǂݍ��݁A����ID->���̖���map���쐬
//	string line;
//	while (getline(ifs, line, '\n')) {
//		// �^�u�ŕ��������������ldata�֊i�[
//		stringstream ss(line);
//		string s;
//		for (int j = 0; getline(ss, s, '\t'); j++) {
//			if(j == 1) Obj.push_back(atoi(s.c_str()));
//			else if(j == 2)Num.push_back(atoi(s.c_str()));
//		}
//		// ����ID�ƃL�[�|�C���g���𒊏o���Ċi�[
//	}
//	return (Obj.size() > 0);
//}

//TODO:increment
/*	increment
 *	function	:
 *	arg			:image			:input	:
 *				:unsigned long	:frame	:
 *	return		:bmp			:det	:
 */
int Event::increment(unsigned long frame,image &input,int num)
{
	int Height = 240;//240
	int Width  = 320;//320
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
	for(int index = 1; index < (int)object_img.size(); index++){
		same_flag = check(input,num,index);
		if(same_flag == true){//�e�}�X�N�Ƃ̃}�b�`���O���^�ɂȂ�����
			//�������C����ID
			delete_key = l_map[index];
			del_objct_num = index;
			delete_flag = true;
			//for(int j=0; j < Height; j++){
			//	for(int i=0; i < Width; i++){
			//		//end() ����begin()�܂ŒH�� = �ォ�猩���悤�ɂȂ�
			//		std::vector<int>::iterator it = lay.m_val(i,j).end();

			//		while( it != lay.m_val(i,j).begin()){
			//			--it;
			//			if(*it != -1){

			//				//�I�u�W�F�N�gID�������C����get
			//				int index2 = lay.get_layer(*it);

			//			}
			//		}
			//	}
			//}
		}
	}
	//cout <<"delete_key:"<<delete_key<<endl;	
	return delete_key;
}

//TODO:���̌��m��臒l
//--------------------------------------------------
// Name     :check()
// Author�@ : ��{
// Update   : 2013.8.20
// Function : �f�����ɂ��镨�̂Ƒ傫���̔�r
// Argument : input image�N���X�@int num,int index
// return   : �Ȃ�
//--------------------------------------------------
bool  Event::check(image &input,int num,int index)//input object_img
{
//	int x1,y1,x2,y2;
	long same_pix = 0;//�����ꏊ�̉�f
	long now_num  = 0; //���݂̍����̐�
	long pre_num =0; //pre���̖̂ʐ�
	long total_num = 0; //OR
	int imageHeight = input.getheight();
	int imageWidth = input.getwidth();
	
	//############################################
	//���ڂ������x���̗̈�ɂ���
	//���̗̈�ɉߋ��ɂ������傫�����炢�̓������x����
	//�������̂����������̂����ׂ�
	//����΁@�{�P�@������΁@�|�P
	//�P�O�̃t���[���ƕ��̗̈�ɂ��ĉ�f�̐��𒲂ׂ�


	//cout << "[" << __FUNCTION__ << "]"  << "pre.new_objnum: " << pre.getNewObjNum() << endl;
	//TODO:"AND / OR ����芄���ȏ�Ȃ�"�ɏ����ς���


		//���C�������̂̑Ή��t�@�C���쐬


	//pre.isDiff(i,j) == OBJECT�͂Ȃ�
	for(int j=0; j < imageHeight; j++){
		for(int i=0; i < imageWidth; i++){
			if(input.get_ObjectTab_s_w(num)<i&&i<input.get_ObjectTab_e_w(num)&&input.get_ObjectTab_s_h(num)<j&&j<input.get_ObjectTab_e_h(num)){}
			else{input.set_Object_label(num,i,j,0) ;}
			if(input.get_Object_label(num,i,j) ==num||  object_img[index].objectT(i,j) == OBJECT ){
				
				total_num++;
			}
			if(input.get_Object_label(num,i,j) ==num &&  object_img[index].objectT(i,j) == OBJECT ){
				same_pix++;	
			}
			if(input.get_Object_label(num,i,j) == num ) now_num++;
			if(object_img[index].objectT(i,j) == OBJECT ) pre_num++;
			
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
		if(ratio >= 0.4){
			return true;
		}
	}
	//������Ȃ�������
	return false;
}

unsigned long Event::readLogFile(string filename){
	std::ifstream ifs(filename);
	char buff[82];
	int result = 0;
	while(ifs.getline(buff,80)>0){
		result = atoi(buff);
	}
	ifs.close();
	return(unsigned long)result;
}

unsigned long Event::searchFolder(unsigned long frame)
{
	return readLogFile("../DB/Human-DB/human.log");
}

unsigned long Event::searchFile(unsigned long frame)
{
	return readLogFile("../DB/Human-DB/human2.log");
}

//--------------------------------------------------
// Name     : objectTracking()
// Author�@ : camshift�ɂ��ړ����m
// Update   : 2013.8.20
//			  2014/04/08	kawakita
// Function : �f�����ɂ��镨�̂Ƒ傫���̔�r
// Argument : input image�N���X�@unsigned long nowFrame���t���[����,int t�@traker�ɓo�^����Ă��鎝�����蕨�́@,int num�@,Pair* traker��������͈͂Ȃ�
// return   : �������ݕ��̂��������蕨�̂ɓ����(true)
//			  �ړ����m�����������Ƃ��Ɏ�������ꂽ���̂�ID��Ԃ��悤�ɕύX
//--------------------------------------------------
int Event::objectTracking( image &input, unsigned long nowFrame, int t, int objectNum, int num, Pair* traker )//, std::vector<Pair*> tracker)
{
	cv::Mat input2;
	char fileName[ 128 ];
	char tmp[ 10 ];	//frame�ԍ�,�t�H���_���ȂǂŎg�p

	unsigned long i = searchFile( nowFrame );//�Ō�̃t���[��

	cout << i << endl;
	if( i == 0 )
	{
		cerr << "frame human break" << endl;
		return false;
	}

	bool eflag = false;
	int trackObjectID = -1;

	int trackerLeftTopX, trackerLeftTopY, trackerRightBottomX, trackerRightBottomY;
	int leaveFrame;
	int imageHeight, imageWidth;

	if( input.tracker.size( ) == 0 || !( input.tracker[ t ]->get_x( ) > 0 && input.tracker[ t ]->get_y( ) > 0 && input.tracker[ t ]->get_vx( ) > 0 && input.tracker[ t ]->get_vy( ) > 0 ) )
	{
		cout << "��������̃f�[�^������܂���" << endl;
		return false;
	}
	//CamShift�֌W+++++++++++++++++++++++++++++++++++++++++++++++++++
	cv::Mat image, hsv, hue, mask, back, backproject;
	cv::Mat backOriginal;

	cv::Mat firstHue, firstMask;

	int trackObject = 0;
	cv::Rect selection;//�̈�x,y,width,height
	cv::Rect trackWindow;
	cv::Mat objectMask;
	CvConnectedComp trackComp;//�A������ /* �Z�O�����g�����ꂽ�A�������̖ʐ� */ /* �Z�O�����g�����ꂽ�A�������̃O���[�X�P�[���l *//* �Z�O�����g�����ꂽ�A��������ROI */

	int vmin = 10, vmax = 256, smin = 30;
	cv::RotatedRect res;//��]���l�����ꂽ2�����̔�
	cv::RotatedRect resObjectArea;
	cv::RotatedRect trackArea;

	//�ǉ��i�Ώە��̂̎�����l���j
	cv::Rect selectionAround;
	cv::Mat hsvAround, hueAround, maskAround;
	cv::RotatedRect resAround;

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	trackerLeftTopX = input.tracker[ t ]->get_x( );
	trackerLeftTopY = input.tracker[ t ]->get_y( );
	trackerRightBottomX = input.tracker[ t ]->get_vx( ) - input.tracker[ t ]->get_x( );
	trackerRightBottomY = input.tracker[ t ]->get_vy( ) - input.tracker[ t ]->get_y( );
	leaveFrame = input.tracker[ t ]->get_frame( );

	//�ߋ��ɑk���Ēǐ�
	while( 1 )
	{
		//�����l�̐ݒ�
		unsigned long frameHuman = searchFile( nowFrame );
		if( frameHuman == 0 )
		{
			cerr << "frame human break" << endl;
			break;
		}
		if( i == frameHuman )
		{
			selection.x = input.get_ObjectTab_s_w( input.Event_Log( num ) );//o_area.s_w;//�������ݗ̈�
			selection.y = input.get_ObjectTab_s_h( input.Event_Log( num ) );// o_area.e_h;
			selection.width = input.get_ObjectTab_e_w( input.Event_Log( num ) ) - input.get_ObjectTab_s_w( input.Event_Log( num ) );//o_area.e_w-o_area.s_w;
			selection.height = input.get_ObjectTab_e_h( input.Event_Log( num ) ) - input.get_ObjectTab_s_h( input.Event_Log( num ) );//o_area.e_h-o_area.s_h;
			trackObject = -1;
		}

		sprintf_s( fileName, "../DB/Human-DB/Original/%lu/%08lu.%s", searchFolder( nowFrame ), i, "png" );
		input2 = cv::imread( fileName );

		//�摜���Ȃ��ꍇ��i�����炵�Ă��邩�ǂ����T��
		if( input2.data == NULL )
		{
			if( eflag == true && i < searchFolder( nowFrame ) )
			{ //�I��
				cout << "no image" << endl;
				eflag = false;
				break;
			}
			else
			{
				eflag = true;
				i--;
				sprintf_s( fileName, "../DB/Human-DB/Original/%lu/%08lu.%s", searchFolder( nowFrame ), i, "png" );
				input2 = cv::imread( fileName );
				continue;
			}
		}

		//�ŏ��̗̈�m��
		if( image.data == NULL )
		{
			image = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC3, cv::Scalar( 0, 0, 0 ) );
			hsv = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC3, cv::Scalar( 0, 0, 0 ) );
			hue = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			mask = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			back = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC3, cv::Scalar( 0, 0, 0 ) );
			firstHue = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			firstMask = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			objectMask = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			hueAround = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			maskAround = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
			trackWindow = selection;

			imageHeight = input.getheight( );
			imageWidth = input.getwidth( );
			backOriginal = back.clone( );
		}

		//�ǂݍ��񂾉摜��image�ɃR�s�[���AHSV�\�F�n�ɕϊ�����hsv�Ɋi�[
		image = input2.clone( );
		sprintf_s( fileName, "../DB/track/%08lu.%s", i, "png" );
		cv::imwrite( fileName, image );

		//hsv�摜�̂����AH�`�����l����hue�Ƃ��ĕ���
		cv::cvtColor( image, hsv, CV_BGR2HSV );
		cv::extractChannel( hsv, hue, 0 );

		//�����摜�̓ǂݍ���
		sprintf_s( fileName, "../DB/Human-DB/Process/%lu/%08lu.%s", searchFolder( nowFrame ), i, "png" );
		back = cv::imread( fileName );

		//�}�X�N����
		for( int height = 0; height < imageHeight; height++ )
		{
			for( int width = 0; width < imageWidth; width++ )
			{
				if( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) ] ) == ( uchar ) 128 )
				{ //�e����
					mask.data[ height*mask.cols + width ] = ( uchar ) 0;
				}
				else if( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) ] ) == ( uchar ) 255 )
				{ //B
					mask.data[ height*mask.cols + width ] = ( uchar ) 255;
				}
				else if( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) + 2 ] ) == ( uchar ) 255 )
				{ //R
					mask.data[ height*mask.cols + width ] = ( uchar ) 255;
				}
				else if( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) + 1 ] ) == ( uchar ) 255 )
				{ //G
					mask.data[ height*mask.cols + width ] = ( uchar ) 0;
				}
				else
				{
					mask.data[ height*mask.cols + width ] = ( uchar ) 0;
				}
			}
		}

		objectMask = mask.clone( );

		for( int height = 0; height < imageHeight; height++ )
		{
			for( int width = 0; width < imageWidth; width++ )
			{
				if( ( trackWindow.y <= height && height <= trackWindow.y + trackWindow.height ) && ( trackWindow.x <= width && width <= trackWindow.x + trackWindow.width ) )
				{
					if( objectMask.data[ ( height*objectMask.cols + width ) ] != 0 )
					{
						objectMask.data[ ( height*objectMask.cols + width ) ] = ( uchar ) 255;
					}
				}
				else
				{
					objectMask.data[ ( height*objectMask.cols + width ) ] = 0;
				}
			}
		}
		/*		//-------------------------------�ǐՏI�����߂��Ƃ� <���g�p>-----------------------------
		CString oframe, idName;
		//		string imageData,objFname;
		oframe.Format( "%lu", leaveFrame );

		char objname[ 128 ];
		cv::Mat obj;

		int x;
		for( x = 0; x < 10; x++ )
		{
		if( input2.data == NULL )
		{
		sprintf_s( objname, "../DB/Detect-Object/%lu-%lu/%s.%s", oframe, x, "object", "png" );
		//			objFname = "../DB/Detected-Object/" + oframe +"-"+idName+ "/object.png";
		//			imageData = "../DB/Detected-Object/" + oframe +"-"+idName+ "/input.png";
		obj = cv::imread( objname );
		if( obj.data != NULL )
		break;
		}
		}
		if( x == 10 ) { break; }

		//���̂̍������Ȃ��Ȃ邽�߁A�ǐՈʒu�Ǝ�������ʒu���߂��ꍇ�������쐬
		for( int k = 0; k < 240; k++ )
		{
		for( int l = 0; l < 320; l++ )
		{
		if( ( unsigned char ) ( obj.data[ ( obj.step*k + l * 3 ) ] ) == ( uchar ) 255
		&& ( unsigned char ) ( obj.data[ ( obj.step*k + l * 3 ) + 1 ] ) == ( uchar ) 255
		&& ( unsigned char ) ( obj.data[ ( obj.step*k + l * 3 ) + 2 ] ) == ( uchar ) 255
		)
		{
		mask.data[ k*mask.cols + l ] = 255;
		}
		}
		}
		//---------------------------------------------------------------------------------------*/

		if( trackObject < 0 )
		{  //�ǐՊJ�n��
			//�Ώە��̂̍ŏ���mask,hue�摜���L��
			firstHue = hue.clone( );
			firstMask = objectMask.clone( );

			//���ӗ̈�̏��
			hueAround = hue.clone( );
			hsvAround = hsv.clone( );
			maskAround = objectMask.clone( );

			selectionAround.x = max( 0, trackWindow.x - ( trackWindow.width / 2 ) );
			selectionAround.y = max( 0, trackWindow.y - ( trackWindow.height / 2 ) );
			selectionAround.width = min( 320, 2 * trackWindow.width );
			selectionAround.height = min( 240, 2 * trackWindow.height );

			//���ӗ̈�̒���
			for( int height = 0; height < imageHeight; height++ )
			{
				for( int width = 0; width < imageWidth; width++ )
				{
					if( width >= selectionAround.x && width <= selectionAround.x + selectionAround.width &&
						height >= selectionAround.y && height <= selectionAround.y + selectionAround.height )
					{
						if( width >= trackWindow.x && width <= trackWindow.x + trackWindow.width &&
							height >= trackWindow.y && height <= trackWindow.y + trackWindow.height )
						{
							maskAround.data[ ( height*maskAround.cols + width ) ] = ( uchar ) 0;
						}
						else
						{
							maskAround.data[ ( height*maskAround.cols + width ) ] = ( uchar ) 255;
						}
					}
					else
					{
						maskAround.data[ ( height*maskAround.cols + width ) ] = ( uchar ) 0;
					}
				}
			}
			trackWindow = selection;//
			trackObject = 1;
		}
		else
		{
			hueAround = hue.clone( );
			maskAround = mask.clone( );

			for( int height = 0; height < imageHeight; height++ )
			{
				for( int width = 0; width < imageWidth; width++ )
				{
					maskAround.data[ ( height*maskAround.cols + width ) ] = ( uchar ) 0;
				}
			}
			int aroundLineWidth;
			if( resAround.size.height > resAround.size.width )
			{
				aroundLineWidth = resAround.size.height / 10;
			}
			else
			{
				aroundLineWidth = resAround.size.width / 10;
			}
			cv::ellipse( maskAround, resAround, cv::Scalar( 255, 255, 255 ), aroundLineWidth - 1, CV_AA );
		}

		sprintf_s( fileName, "../DB/Human-DB/Process/%lu/object/%08lu.%s", searchFolder( nowFrame ), i, "png" );
		cv::imwrite( fileName, objectMask );

		sprintf_s( fileName, "../DB/Human-DB/Process/%lu/around/%08lu.%s", searchFolder( nowFrame ), i, "png" );
		cv::imwrite( fileName, maskAround );

		//		cv::imshow( "diffArea", back );
		//		cv::waitKey( 1 );

		//�l���̈�̔r��
		for( int height = 0; height < imageHeight; height++ )
		{
			for( int width = 0; width < imageWidth; width++ )
			{
				if( ( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) ] ) == ( uchar ) 0 //�c��F�F��
					&& ( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 + 1 ) ] ) >= ( uchar ) 200
					|| ( unsigned char ) ( back.data[ ( back.step*height + width * 3 + 2 ) ] ) >= ( uchar ) 200 ) )
					||
					( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 + 1 ) ] ) == ( uchar ) 0 //�c��F�F��
					&& ( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) ] ) >= ( uchar ) 200
					|| ( unsigned char ) ( back.data[ ( back.step*height + width * 3 + 2 ) ] ) >= ( uchar ) 200 ) )
					||
					( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 + 2 ) ] ) == ( uchar ) 0 //�c��F�F��
					&& ( ( unsigned char ) ( back.data[ ( back.step*height + width * 3 + 1 ) ] ) >= ( uchar ) 200
					|| ( unsigned char ) ( back.data[ ( back.step*height + width * 3 ) ] ) >= ( uchar ) 200 ) ) )
				{
					back.data[ ( back.step*height + width * 3 ) ] = ( uchar ) 0;
					back.data[ ( back.step*height + width * 3 + 1 ) ] = ( uchar ) 0;
					back.data[ ( back.step*height + width * 3 + 2 ) ] = ( uchar ) 0;
					mask.data[ height*mask.cols + width ] = ( uchar ) 0;
				}
			}
		}

		//�Oframe�̒ǐ՗̈�̕`��
		cv::ellipse( mask, resObjectArea, cv::Scalar( 255, 255, 255 ), -1, CV_AA );
		cv::ellipse( back, resObjectArea, cv::Scalar( 255, 255, 255 ), -1, CV_AA );
		cv::ellipse( objectMask, trackArea, cv::Scalar( 255, 255, 255 ), -1, CV_AA );

		cv::multiply( mask, objectMask, mask, 255 );

		//�q�X�g�O�����A�o�b�N�v���W�F�N�g�̌v�Z
		calcBack( input, firstHue, firstMask, hue, objectMask, hueAround, maskAround, mask, nowFrame );

		sprintf_s( fileName, "../DB/track_img/%08lu.%s", nowFrame, "png" );
		backproject = cv::imread( fileName, 0 ); //grayScale�`���ł̉摜�ǂݍ���

		/*		cv::imshow( "image", image );
		cv::waitKey( 1 );
		cv::imshow( "trackExpansion", objectMask );
		cv::waitKey( 1 );
		cv::imshow( "mask", mask );
		cv::waitKey( 1 );
		cv::imshow( "maskAround", maskAround );
		cv::waitKey( 1 );
		*/
		//ONLINE�ł͖��Ȃ����A�������̂�OFFLINE�ŗ�����error���������A�V�X�e����������ꍇ������
		res = cv::CamShift( backproject, trackWindow,
							cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 )
							);


		// size.width,height�ɂ���ė̈�̒�����
		trackWindow.x = res.center.x;
		trackWindow.y = res.center.y;
		trackWindow.width = res.size.width*1.15;
		trackWindow.height = res.size.height*1.15;

		trackArea.angle = res.angle;
		trackArea.center.x = res.center.x;
		trackArea.center.y = res.center.y;
		trackArea.size.height = res.size.height*1.5;
		trackArea.size.width = res.size.width*1.5;
		if( trackArea.size.width < trackArea.size.height )
		{
			trackArea.size.width = trackArea.size.height;
		}
		else
		{
			trackArea.size.height = trackArea.size.width;
		}

		resObjectArea.angle = res.angle;
		resObjectArea.center.x = res.center.x;
		resObjectArea.center.y = res.center.y;
		resObjectArea.size.height = res.size.height*1.15;
		resObjectArea.size.width = res.size.width*1.15;

		resAround.angle = resObjectArea.angle;
		resAround.center.x = resObjectArea.center.x;
		resAround.center.y = resObjectArea.center.y;
		resAround.size.height = resObjectArea.size.height*1.3 + 5;
		resAround.size.width = resObjectArea.size.width*1.3 + 5;

		cv::ellipse( image, res, cv::Scalar( 255, 255, 255 ), -1, CV_AA );
		//		cv::imshow( "trackArea", image );
		//		cv::waitKey( 1 );

		//camshift�ǐՂɎ��s�����ꍇ�I��
		/*
		if( &res == NULL )
		{
		cout << "camshift dont track" << endl;
		break;
		}
		else */
		if( res.center.x == 0 || res.size.height == 0 )
		{
			cout << "no object. ( res.center.x, res.center.y ) = ( " << res.center.x << ", " << res.center.y << " )" << endl;
			cout << "camshift dont track" << endl;
			break;
		}
		else
		{
		}

		//�ɏ��̏ꍇ�������ɃX���[������ res�����Ă��Ȃ�����`����Ȃ�
		if( res.center.x > 10 && res.center.y > 10 &&
			res.size.height > 10 && res.size.width > 10 )
		{
			cv::ellipse( image, res, cv::Scalar( 255, 0, 0 ), 3, CV_AA );
		}
		int margin = 25;
		if( res.center.x >= trackerLeftTopX - margin && res.center.x <= trackerLeftTopX + trackerRightBottomX + margin
			&& res.center.y >= trackerLeftTopY - margin && res.center.y <= trackerLeftTopY + trackerRightBottomY + margin )
		{
			//���̂�ID��Ԃ��悤�ɕύX
			//			trackObjectID = true;
			trackObjectID = input.tracker[ t ]->get_id( );
			cout << "trackObjectID:" << trackObjectID << endl;
		}
		else
			trackObjectID = -1;

		for( int t = 0; t < ( int ) input.tracker.size( ); t++ )
		{
			int	objectLeftTopX = input.tracker[ t ]->get_x( );
			int	objectLeftTopY = input.tracker[ t ]->get_y( );
			int	objectRightBottomX = input.tracker[ t ]->get_vx( );
			int	objectRightBottomY = input.tracker[ t ]->get_vy( );
			cv::rectangle( image, cvPoint( objectLeftTopX, objectLeftTopY ), cvPoint( objectRightBottomX, objectRightBottomY ), cv::Scalar( 0, 255, 255 ), 1, 8, 0 );
		}

		int xSum = trackerLeftTopX + trackerRightBottomX, ySum = trackerLeftTopY + trackerRightBottomY;
		//���W�̗̈�O�Q��
		if( xSum >= 320 )
		{
			xSum = 320;
		}
		if( ySum >= 240 )
		{
			ySum = 240;
		}

		if( !input.tracker.empty( ) )
		{
			if( trackObjectID == -1 )
			{
				//cv::rectangle( image, cvPoint( trackerLeftTopX, trackerLeftTopY ), cvPoint( trackerLeftTopX + trackerRightBottomX, trackerLeftTopY + trackerRightBottomY ), cv::Scalar( 0, 255, 255 ), 1, 8, 0 );
			}
			else
			{
				cv::rectangle( image, cvPoint( trackerLeftTopX, trackerLeftTopY ), cvPoint( trackerLeftTopX + trackerRightBottomX, trackerLeftTopY + trackerRightBottomY ), cv::Scalar( 0, 255, 255 ), 1, 8, 0 );
			}
		}

		sprintf_s( fileName, "../DB/trackImg/%08lu.%s", i, "png" );
		imwrite( fileName, image );

		//�摜�\��//�ǐՏ��o�͗p
		cv::Mat imgobjtracking = image.clone( );
		bmp det;

		cout << "input.tracker_img.push_back(cv::Mat(image));" << endl;
		if( i == searchFile( nowFrame ) )
		{
			imwrite( "result.png", image );
		}

		i--;
		eflag = false;

		//�I������(�������݂Ɣ��肳���ΏI��)
		cout << "leaveFrame:" << leaveFrame << endl;
		cout << "i:" << leaveFrame << endl;
		cout << "searchFolder(nowFrame)" << searchFolder( nowFrame ) << endl;

		//�������́A���S�ɂ��߂��ƕ���������I��
		if( i == searchFolder( nowFrame ) - 25 )
		{
			cerr << "search folder break" << endl;
			break;
		}

		//�Ō�̎�������̃t���[���n�_�܂ők��ΏI��//kawa(���l�����͐��m�Ȏ������莞�̌덷)
		if( i == ( leaveFrame - 39 ) )
		{
			cerr << "Last leave frame " << endl;
			cout << "�Ō�̎�������n�_�܂Ŗ߂�܂����B" << endl;
			break;
		}
		//�������݂Ǝ�������̂������Ől�����r���œr�؂�Ă���ꍇ�ǐՕs��//kawa
		if( leaveFrame < ( int ) searchFolder( nowFrame ) )
		{
			trackObjectID = -1;
			cv::destroyWindow( "object-tracking" );
			cerr << "Last human frame " << endl;
			cout << "�ŋ߂̎������݂Ǝ�������ɓ���̐l�����֘A���Ă��Ȃ��\��������܂��B" << endl;
			break;
		}
	}//while(1){
	cout << "end" << endl;

	//Debug�p�E�B���h�E�̏���
	cv::destroyAllWindows( );

	if( trackObjectID != -1 )cout << " move object." << endl;
	return trackObjectID;
}

//--------------------------------------------------
// Name     :interpretation()
// Author�@ : ���C���[�\���ɂ�镨�̏��Ǘ�
// Update   : 2013.8.20
// Function : �f�����ɂ��镨�̂����C���[�Ǘ�
// Argument : input image�N���X�@unsigned long now_frame���t���[����,int obj_num�@�C�x���g���O���ɓ����Ă��镨�̂h�c�@,int num�@���̌����ɓ����Ă���h�c,delete_key�@�������蔻��
// return   : �������ݕ��̂��������蕨�̂ɓ����(true)
//--------------------------------------------------
void Event::interpretation(image &input, unsigned long frame, int obj_num,int num,int delete_key)
{
	static layer lay;//kawa�@�ێ����郌�C���[�\���̕����������݂�������Ηe�ʂ������Ă����B

	int Height = 240;
	int Width  = 320;
	//static bmp output,output_val;//kawa    output���C���[�ʂɐF�������ꂽ�l   output_val�ォ�猩���l


//////////////////////////////////////////////////////////////////////////////
	//static bmp mask_img[10];//���I�z��ɕύX//kawa
	//static std::vector<bmp>  mask_img;
	//mask_img.reserve(30);
	//object_img.reserve(30);
	static unsigned int laydata=1;
//////////////////////////////////////////////////////////////////////////////


	//bmp outputmask[10];//kawa�@�e�X�g�p
	//std::string filename;

	const int b_threshold = 20;//������臒l

	static int del_objct_num = 0; //static�ɕύX
	bool no_exist = true;
	bool objId = false;

	static int del_num=0;    //static�ɕύX

	int count[320*240]={-1};
	int table[10]={-1};//���I�z��ɕύX//kawa
	//static std::vector<int>table;

	//int out_objId = -1;

	FileOperator file;
	FILE *fl;

	int LayNum=0;

	fopen_s(&fl,"../DB/Detected-Object/object.log","a");

	//std::cout <<  "key_id" <<key_id<<"obj__id"<<obj_id<<"delete_key"<<delete_key<< std::endl;

#if 1 //������������������������������������������������������������������������������
	// �����̌��m�����������摜�ɂ��ăV�[���̉��߂��s��
	// �e�ƂȂ�Ȃ��������̂����ɂ��āI�I

	Geometory::Point2D posCenter(input.get_ObjectTab_cx(obj_num),input.get_ObjectTab_cy(obj_num));//���̂̏d�S
	//���̂̊O�ڒ����`
	Area o_area = input.get_area();

	//���̂̊O�ڒ����`�̒��_���W
	Geometory::Point2D posStart(0 + input.get_ObjectTab_s_w(obj_num), 0 + input.get_ObjectTab_s_h(obj_num));//(0,0)����_�ɕ��̂̊O�ڒ����`�̍�����W
	Geometory::Point2D posEnd(0 + input.get_ObjectTab_e_w(obj_num), 0 + input.get_ObjectTab_e_h(obj_num));//(0,0)����_�ɕ��̂̊O�ڒ����`�̉E�����W

	//���̖̂ʐ�
	int objArea = input.get_ObjectTab_size(obj_num);

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
//	char c[35];


	
	//���̍��������Z�[�u
	//TODO:error
	//cout << "cvCreateImage(cvSize(" << input.getwidth() << "," << input.getheight() << ")," << IPL_DEPTH_8U << "," << 3 << ")" << endl; 

	cv::Mat obj = cv::Mat(cvSize(input.getwidth(),input.getheight()),CV_8UC3,cv::Scalar(0,0,0));


	//�}�b�`���O��̏������ł���΂��Ƃ͂ł���I�I�I�I�I�I
	//std::cout <<  delete_key << std::endl;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		

	//cout << obj_num << endl;

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
		for(int row=0; row < Height; row++){
			for(int col=0; col < Width; col++){
				if(input.get_Object_label(obj_num,col,row) == obj_num){
					//double diff = sqrt((double)((output_val.R(i,j) - input.R(i,j)) * (output_val.R(i,j) - input.R(i,j))));//kawa�Z�W�l�ɂ��w�i�̂����r?��O�����̕����H
					if(input.get_ObjectTab_s_w(obj_num)<col&&col<input.get_ObjectTab_e_w(obj_num)&&input.get_ObjectTab_s_h(obj_num)<row&&row<input.get_ObjectTab_e_h(obj_num))
					{
						obj.at<cv::Vec3b>(row,col)[0]=255;//input.get_input_L(col,row);
						obj.at<cv::Vec3b>(row,col)[1]=0;//obj.at<cv::Vec3b>(col,row)[0];
						obj.at<cv::Vec3b>(row,col)[2]=0;//obj.at<cv::Vec3b>(col,row)[0];
						input.set_background_L(col,row,(float)lay.get_val(col,row,pos));//input.L(i,j);

						input.set_background_R(col,row,input.get_input_R(col,row));
						input.set_background_G(col,row,input.get_input_G(col,row));
						input.set_background_B(col,row,input.get_input_B(col,row));
						if(input.kinect_on){
							if(lay.get_depth(col,row,pos)>0){
							input.set_depth_is_diff_d(col,row,1);//�[����␳
							input.set_depth_is_diff(col,row,lay.get_depth(col,row,pos));

							}
						}
					}
					
				}else{
					obj.at<cv::Vec3b>(row,col)[0]=0;
					obj.at<cv::Vec3b>(row,col)[1]=0;
					obj.at<cv::Vec3b>(row,col)[2]=0;
					if(lay.get_depth(col,row,pos)!=-1)
						{
							if(lay.get_depth(col,row,pos)>0){
						   input.set_depth_is_diff_d(col,row,1);//�[����␳
						   input.set_depth_is_diff(col,row,input.get_depth(col,row));
							}
						}

					//input.set_depth_is_diff_d(col,row,0);
				}
			}
		}
		//std::cout <<  "������" << pos << "�Ԗڂ�����" << std::endl;
		for(int row=0; row < Height; row++){
			for(int col=0; col < Width; col++){
				lay.val(col,row).erase(lay.val(col,row).begin() + pos);//���C���[���̍폜�����@
				lay.m_val(col,row).erase(lay.m_val(col,row).begin() + pos);//�}�X�N���̍폜
				if(input.kinect_on)
				{
					lay.depth_val(col,row).erase(lay.depth_val(col,row).begin()+pos);//�������̍폜
				}
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
		//std::cout <<  "���C���ǉ�" << std::endl;
		for(int row=0; row < Height; row++){
			for(int col=0; col < Width; col++){
				//�����f���I�u�W�F�N�g�������炻�̗v�f�̒l������
				if(input.get_Object_label(obj_num,col,row) == obj_num){
					//�e�N�X�`���̏���ǉ�
					
					//�}�X�N�̏���ǉ�
					//lay.add_mask(i,j,obj_id);//kawa     ���̌ŗL�Ȃ�keyid�ق��ɓ���̏ꍇ��obj_id���g���Ă�?
					if(input.get_ObjectTab_s_w(obj_num)<col&&col<input.get_ObjectTab_e_w(obj_num)&&input.get_ObjectTab_s_h(obj_num)<row&&row<input.get_ObjectTab_e_h(obj_num))
					{
						int val = (int)(input.get_background_L(col,row));//�Z�W�l
						lay.add(col,row,val);//val->(key_idname.size()-1)
						lay.add_mask(col,row,(key_idname.size()-1));
						input.set_background_L(col,row,input.get_input_L(col,row));
						input.set_background_R(col,row,input.get_input_R(col,row));
						input.set_background_G(col,row,input.get_input_G(col,row));
						input.set_background_B(col,row,input.get_input_B(col,row));
						if(input.kinect_on)
						{
							if(input.get_depth(col,row)>0){
							lay.add_depth(col,row,input.get_depth_is_diff(col,row));
							input.set_depth_is_diff_d(col,row,1);
							input.set_depth_is_diff(col,row,input.get_depth(col,row));
							}
							else
							{
							lay.add_depth(col,row,-1);
							//input.set_depth_is_diff_d(col,row,1);
							//input.set_depth_is_diff(col,row,input.get_depth(col,row));
							}
						}
						/*obj.at<cv::Vec3b>(row,col)[0]=255;
						obj.at<cv::Vec3b>(row,col)[1]=obj.at<cv::Vec3b>(row,col)[0];
						obj.at<cv::Vec3b>(row,col)[2]=obj.at<cv::Vec3b>(row,col)[0];*/
						
					}
					else
					{	
						lay.add(col,row,-1);
						lay.add_mask(col,row,-1);
						if(input.kinect_on)
						{
							lay.add_depth(col,row,-1);
							//input.set_depth_is_diff_d(col,row,0);
						}

					}

				}
				//����ȊO�͉������� -1 ������
				else{
						lay.add(col,row,-1);
						lay.add_mask(col,row,-1);
						/*obj.at<cv::Vec3b>(row,col)[0]=0;
						obj.at<cv::Vec3b>(row,col)[1]=obj.at<cv::Vec3b>(row,col)[0];
						obj.at<cv::Vec3b>(row,col)[2]=obj.at<cv::Vec3b>(row,col)[0];*/
						if(input.kinect_on)
						{
							lay.add_depth(col,row,-1);
							//input.set_depth_is_diff_d(col,row,0);
						}
				    }
			}
		}
		//�I�u�W�F�N�g��ID��ǉ�
	lay.set_layer((key_idname.size()-1));//l_arrey��ID��push����
	laydata++;
	}


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if(delete_flag){
		fprintf(fl,"%d,", EVENT_TAKING_OUT);
		fprintf(fl,"%d,",delete_key);
	}else{
		if(input.get_Event_Eve(num)==2) {fprintf(fl,"%d,",(int)EVENT_MOVE);}
		else {fprintf(fl,"%d,",(int)EVENT_BRINGING_IN);}

		fprintf(fl,"%d,", key_idname.back());
	}

	CString oframe,id_name;
	string image_data,obj_fname;
	oframe.Format("%lu",frame);
	id_name.Format("%lu",input.get_Event_model(num));

	obj_fname = "../DB/Detected-Object/" + oframe +"-"+id_name+ "/object.png";
	cv::imwrite(obj_fname,obj);
	image_data = "../DB/Detected-Object/" + oframe +"-"+id_name+ "/input.png";

	//bmp *bmpInput = (bmp*)(&input);
	//bmpInput->save(obj_fname);
	cv::imwrite(image_data,input.img_resize);
	

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//��ԏォ�猩���I�u�W�F�N�g�̏d�Ȃ��\��
	//��ɕ����d�Ȃ邱�Ƃɂ���Č`���ς���Ă��܂����̏���
	//
	//������肠�Ƃɒǉ����ꂽ�I�u�W�F�N�g�ɒ��ڂ���

//	int num = lay.get_layer(lay.get_end())+1;//���C���̍ő吔
//	int l_max = lay.get_layer(lay.get_end())+1;
//	if(l_max==-1){l_max=0;}//kawa
	int num1 = laydata;//���C���̍ő吔
	int l_max = laydata;



	//LayNum = num;


	//�o�͉摜�̏����� //kawa
//	for(int i = 1;i <= 9;i++){//kawa
//		mask_img[i].init();
//		object_img[i].init();
//	}

	//mask_img object_img vecte������Ȃ炱���ő��₷

	//clearobject_img();
	object_img.clear();
    //mask_img.clear();
	
	//resizeobject_img(l_max);
	object_img.resize(l_max);
    //mask_img.resize(l_max);
    //std::cout <<  "object����" <<object_img.size()<< std::endl;
	//std::cout <<  "mask����" <<mask_img.size()<< std::endl;
	


	
	//output.init();
	//output_val.init();

	//���C������map���X�V�O�ɏ�����
	l_map.clear();
	//l_map2.clear();

	if(num1!=-1){//���C���[������ԈȊO�̎�
		no_exist = false;//���̃I�u�W�F�N�g�����݂���
		std::vector<int> all = lay.get_layer_all();
		std::vector<int>::reverse_iterator itr = all.rbegin();


		//map���ŐV��ԂɍX�V update
		//����ID�����C��
		//���C��������ID
		//�̑o�����ɃA�N�Z�X�ł���map������Ă���(�Ӗ������邩�͕s��)
		if(num1 > 0){
			while(num1 != 1){
				num1--;
				//cout<<num1<<";"<< lay.returnl_array(num1-1)<<endl;
				l_map.insert(std::pair<int,int>(num1,lay.returnl_array(num1-1)));
				//l_map2.insert(std::pair<int,int>(num,lay.returnl_array(num-1)));
			}
		}
	}
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

  //for(int j=0; j <lay.get_layer(lay.get_end()); j++){cout<<"laynaiID:"<<j<<"����"<<lay.returnl_array(j)<<endl;}

//�d�Ȃ肪������悤�ɂ�����
	for(int j=0; j < Height; j++)
	{
		for(int i=0; i < Width; i++)
		{
			//���̏��ԂŌ���Əォ�猩���悤�ɂȂ�
			for(std::vector<int>::reverse_iterator it = lay.m_val(i,j).rbegin();it != lay.m_val(i,j).rend();it++)
			{		
				    index=0;
					/*if(input.ObjectTab_s_w(obj_num)<i&&i<input.ObjectTab_e_w(obj_num)&&input.ObjectTab_s_h(obj_num)<j&&j<input.ObjectTab_e_h(obj_num))
					{*/
						if(lay.get_layer(*it)!=-1)
						{
							if(*it != -1)
							{
								laynum++;
						
								//�I�u�W�F�N�gID�������C����get

								if(laynum==1)
								{//kawa?
							
									index = lay.get_layer(*it);//model_id��Ԃ�
									count[j*320+i]=index%10;
									
									object_img[index].SetPixColor(i,j,OBJECT);


								}
								if(table[index%10]<laynum)
								{
										table[index%10]=laynum%10;
								}
								
							}
						}
					/*}*/
			}
			if(laynum==0)	count[j*320+i]=-1;
			laynum=0;
		}
	}

	for(int j=0;j<240;j++){
		for(int i=0;i<320;i++){
				input.OSetPixColor(i,j,0,0,0);
			//input.OSet_input_PixColor(i,j);
			if(count[j*320+i]>=0){
				switch(table[count[j*320+i]]){
				case 1: input.OSetPixColor(i,j,0,255,0);
					break;
				case 2: input.OSetPixColor(i,j,255,0,0);
					break;
				case 3: input.OSetPixColor(i,j,0,0,255);
					break;
				case 4: input.OSetPixColor(i,j,255,255,0);
					break;
				case 5: input.OSetPixColor(i,j,255,0,255);
					break;
				case 6: input.OSetPixColor(i,j,0,255,255);
					break;
				default: input.OSetPixColor(i,j,255%(count[j*320+i]+1),255%(count[j*320+i]+1),255%(count[j*320+i]+1));//kawa
					break;
				}
			}
			if(no_exist==true)
				input.OSetPixColor(i,j,0,0,0);
		}
	}


#if 0 //���C���摜�̕ۑ�
	char fname2[30];
	sprintf(fname2,"%08lu-out2.png",frame);
	det.save(fname2);
#endif
#endif
	//cout << __FUNCTION__ << " l_max:" << l_max << endl;
	//=============================================================
	//���C�����Ƃ̃e�N�X�`���Əォ�猩���}�X�N�Ƃ�AND���Ƃ�
	//��
	//����Ԃł̃}�b�`���O�Ɏg�p���镨�̗̂\�z���ꂽ�`���e�N�X�`���ɂȂ�
	//
	
	


	//=============================================================


//	for(int k=0;k<=9;k++){
	for(int k=1;k<(int)l_map.size();k++){
		//���C�������̂̑Ή��t�@�C���쐬
		FILE *fp;
		CString file,file2,tmp1,tmp2,tmp3;
		tmp1.Format("%lu",frame);
		tmp2.Format("%d",k);
		tmp3.Format("%d",input.get_Event_model(num));
		file = "../DB/Detected-Object/" + tmp1 + "-"+tmp3+"/" + "layer.log";
		//cv::imwrite(string(file),object_img[k].write_image());
		fopen_s(&fp,file,"a");
		fprintf(fp,"%d,",k);
		fprintf(fp,"%d\n",l_map[k]);
		fclose(fp);
		cv::Mat image=cv::Mat(240,320,CV_8UC1, cv::Scalar(0));
		for(int j=0;j<240;j++)
		{
			for(int i=0;i<320;i++)
			{
				if(object_img[k].objectT(i,j)==OBJECT)
				{image.data[image.cols*j+i]=255;}
				else{image.data[image.cols*j+i]=0;}
			}
		}

		file2="../DB/Detected-Object/" + tmp1 + "-"+tmp3+"/" + "result"+tmp2+".png";
		
		cv::imwrite(string(file2),image);
	}
	/*
	* �e��ID��ǉ�
	*/	
	//if(interpre_flag == true)
	fprintf(fl,"%d\n",0);
	fclose(fl);
	

#endif
	//return 0;
}

//--------------------------------------------------
// Name     :SaveObjData()
// Author�@ : ���̏����c�a�ɕۑ�
// Update   : 2013.8.20
// Function : ���̏���ۑ�����
// Argument : input image�N���X�@unsigned long now_frame���t���[����,int obj_num�@�C�x���g���O���ɓ����Ă��镨�̂h�c�@,int num�@���̌����ɓ����Ă���h�c,leave_id�@��������ꂽ���̂̂h�c�Cmodel_id �������܂ꂽ���̂̂h�c
// return   : �������ݕ��̂��������蕨�̂ɓ����(true)
//--------------------------------------------------
void Event::SaveObjData(image &input,unsigned long frame,int obj_num,int num,int leave_id,int model_id)
{
	std::ostringstream dir,frame_name,model_name;
	std::string dir_name;
	//std::string dir_name2;
	//FileOperator path;
	dir  << "../DB/Detected-Object/" << frame<<"-"<<model_id<< "/";
	frame_name << frame<<"-"<<model_id;
	dir_name = "../DB/Detected-Object/" + frame_name.str();
	//path.MakeDir(dir_name.c_str());
	//dir_name.clear();
	//model_name <<model_id;
	//TODO:write Object image(output image)
	/*dir_name = "../Object/" + model_name.str();
	path.MakeDir(dir_name.c_str());*/

	if(leave_id==-1)
	{
	std::ofstream ofs( "../DB/output-img/object.txt", std::ios::app );
	ofs << input.get_Event_key(num) << "\t" <<input.get_Event_obj(num)  << "\t" << endl;
	ofs.close();
	}
	
	
	char extention[10] = "png";

	char obj_fname[128];
	sprintf_s(obj_fname,"../Object/%lu/%lu.%s", input.get_Event_model(num), input.get_Event_model(num), extention);
	
	char obj_fname1[128];
	sprintf_s(obj_fname1,"../Object/%lu/%lu.%s", input.get_Event_key(num), input.get_Event_model(num), extention);
	
	std::string img_fname1;
	img_fname1  = img_fname1  + dir.str() + "result-" + frame_name.str() + ".png";
	cv::imwrite(img_fname1,input.Obj_Img(obj_num,input.ObjImage(obj_num).size()-1));
	

	int max_num=min(6,(int)input.ObjImage(obj_num).size());
	for(int i=0 ;i<max_num;i++)
	{
		std::string img_fname2;
		std::ostringstream stream;
		stream<<max_num-i;
		std::string nume= stream.str();
        img_fname2  = img_fname2  + dir.str() + "t-" + nume  +"-"+ frame_name.str() + ".png";
		cv::imwrite(img_fname2,input.Obj_Img(obj_num,max(0,max_num-i-1)));
		img_fname2.clear();
		stream.clear();
	}

}

float ave_object(int x,int y,int x1,int y1,int x2,int y2,image &input)
{
	int areX,areY;//N*N�������������s��
	areX=max(1,(x2-x1)/15);
	areY=max(1,(y2-y1)/15);
	float data=0.0;
	for(int j=y-areY/2;j<y+areY/2;j++)
	{
		for(int i=y-areX/2;i<x+areX/2;i++)
		{
			data+=input.get_depth(x,y);
		}
	}
	data=data/(areX*areY);
	return data; 

};

int denta_object(image &input,int i)
{
	cv::Mat object_img=input.img_resize.clone();
	cv::Mat object_img1=input.img_resize.clone();
	cv::Mat object_img2=input.img_resize.clone();

	int object_max=input.size_t_object_id();
	int size_t=0;
	
		
	int x1=input.get_t_object_id(i).x1;
	int x2=input.get_t_object_id(i).x2-input.get_t_object_id(i).x1;
	int y1=input.get_t_object_id(i).y1;
	int y2=input.get_t_object_id(i).y2-input.get_t_object_id(i).y1;
	int areX=1;//max(1,x2/10);
	int areY=1;//max(1,y2/10);
	std::vector<std::pair<PII,PII>>line_x;
	std::vector<std::pair<PII,PII>>line_y;
	
	std::vector<cv::Point> point_area;
	//cout<<areX<<areY<<endl;
	//std::vector<PII> data_num ;
	//std::vector<cv::Point> hull;
	////cv::rectangle(object_img,cv::Rect(input.get_t_object_id(i).x1,input.get_t_object_id(i).y1,input.get_t_object_id(i).x2-input.get_t_object_id(i).x1,input.get_t_object_id(i).y2-input.get_t_object_id(i).y1),cv::Scalar(0,0,255));
	for(int y=y1; y< y1+y2; y=y+areY)
	{
		std::vector<int>denta_x;
		std::vector<int>ledge_x;
		for(int x=x1;x< x1+x2; x=x+areX)
		{
			float num=input.get_bring_object_data().at<float>(y,x);
			float num_f=input.get_bring_object_data().at<float>(y-areY,x);
			float num_b=input.get_bring_object_data().at<float>(y+areY,x);
			float num_u=input.get_bring_object_data().at<float>(y,x-areX);
			float num_d=input.get_bring_object_data().at<float>(y,x+areX);
			float non=DEPTH_RANGE_MAX;
			if(num_f!=non&&num_b!=non&&num_d!=non&&num_u!=non)
			{
				if(num_d>num&&num<num_u)//�o������
				{
					// �摜�C�~�̒��S���W�C���a�C�F�C�������C���
					//cv::circle(object_img, cv::Point(x,y),2, cv::Scalar(255,0,0), 2, 4);
					ledge_x.push_back(x);
				}
				if(num_d<num&&num>num_u)//�E��
				{
					// �摜�C�~�̒��S���W�C���a�C�F�C�������C���
					cv::circle(object_img1, cv::Point(x,y),2, cv::Scalar(0,0,255), 1, 4);
					denta_x.push_back(x);
				}
			}
		}
		if(ledge_x.size()>1&&denta_x.size()>0)
		{
			for(int t=0;t<ledge_x.size()-1;t++)
			{
				for(int j=0;j<denta_x.size();j++)
				{
					if(ledge_x[t]<denta_x[j]&&denta_x[j]<ledge_x[t+1])
					{
						/*for(int c=ledge_x[t];c<ledge_x[t+1];c++)
						{
							input.set_T_object_denta_map(i, c,y,input.get_depth_is_diff(c,y));
							cv::circle(object_img, cv::Point(c,y),1, cv::Scalar(0,0,200), 2, 4);
						}*/
						PII a(ledge_x[t],y),b(ledge_x[t+1],y);
						std::pair<PII,PII> c(a,b);
						line_x.push_back(c);
					}
				}
			}
		}
	}
	for(int x=x1;x< x1+x2; x=x+areX)
	{
		std::vector<int>denta_y;
		std::vector<int>ledge_y;
		for(int y=y1; y< y1+y2;  y=y+areY)
		{
			float num=input.get_bring_object_data().at<float>(y,x);
			float num_f=input.get_bring_object_data().at<float>(y-areY,x);
			float num_b=input.get_bring_object_data().at<float>(y+areY,x);
			float num_u=input.get_bring_object_data().at<float>(y,x-areX);
			float num_d=input.get_bring_object_data().at<float>(y,x+areX);
			float non=DEPTH_RANGE_MAX;
			if(num_f!=non&&num_b!=non&&num_d!=non&&num_u!=non)
			{	
				//if(num_f<num&&num>num_b&&num_d<num&&num>num_u)
				if(num_f<num&&num>num_b)
				{
					// �摜�C�~�̒��S���W�C���a�C�F�C�������C���
					cv::circle(object_img1, cv::Point(x,y),2, cv::Scalar(0,0,255), 1, 4);
					denta_y.push_back(y);
				}
				if(num_f>num&&num<num_b)
				{
					// �摜�C�~�̒��S���W�C���a�C�F�C�������C���				
					//cv::circle(object_img, cv::Point(x,y),2, cv::Scalar(255,0,0), 2, 4);
					ledge_y.push_back(y);
				}
			}
			
		}
		if(ledge_y.size()>1&&denta_y.size()>0)
		{
			for(int t=0;t<ledge_y.size()-1;t++)
			{
				for(int j=0;j<denta_y.size();j++)
				{
					if(ledge_y[t]<denta_y[j]&&denta_y[j]<ledge_y[t+1])
					{
						PII a(x,ledge_y[t]),b(x,ledge_y[t+1]);
						std::pair<PII,PII> c(a,b);
						line_y.push_back(c);
					}
				}
			}
		}
	}


	float size=0;
	for(int y=0; y< line_y.size(); y++)
	{
		for(int x=0;x< line_x.size(); x++)
		{
			int x_x1=line_x[x].first.first;
			int x_x2=line_x[x].second.first;
			int x_y1=line_x[x].first.second;
			int x_y2=line_x[x].second.second;
			int y_x1=line_y[y].first.first;
			int y_x2=line_y[y].second.first;
			int y_y1=line_y[y].first.second;
			int y_y2=line_y[y].second.second;
			
			if(x_x1<y_x1&&y_x1<x_x2)
			{
				if(y_y1<x_y1&&x_y1<y_y2)
				{
					for(int j=x_x1;j<=x_x2;j++)
					{
						input.set_T_object_denta_map(i, j,x_y1,input.get_depth_is_diff(j,x_y1));
						cv::circle(object_img, cv::Point(j,x_y1),2, cv::Scalar(255,0,0), 1, 4);
					}
					for(int j=y_y1;j<=y_y2;j++)
					{
						input.set_T_object_denta_map(i, y_x1,j,input.get_depth_is_diff(y_x1,j));
						cv::circle(object_img, cv::Point(y_x1,j),2, cv::Scalar(255,0,0), 1, 4);
					}
					cv::circle(object_img2, cv::Point(x_x1,x_y1),2, cv::Scalar(255,0,0), 1, 4);
					cv::circle(object_img2, cv::Point(x_x2,x_y2),2, cv::Scalar(255,0,0), 1, 4);
					cv::circle(object_img2, cv::Point(y_x1,y_y1),2, cv::Scalar(255,0,0), 1, 4);
					cv::circle(object_img2, cv::Point(y_x2,y_y2),2, cv::Scalar(255,0,0), 1, 4);
					
					point_area.push_back(cv::Point(y_x2,y_y2));			
				}
			}	
		}
	}
		
		
	for(int y=y1; y< y1+y2; y++)
	{
		for(int x=x1;x< x1+x2; x++)
		{
			if(input.get_T_object_denta_map(i,x,y)!=DEPTH_RANGE_MAX)size_t++;//input.get_T_object_denta_map(i,x,y);
		}
	}
	if(input.get_T_object_denta_size(i)<size_t)input.set_T_object_denta_size(i,size_t);

	return input.get_T_object_denta_size(i);
};

void Event::object_reload(image &input,unsigned long frame)//�C�x���g�ɂ�镨�̏��̍X�V
{
//----------------------------------------------------------------------���̏��̍X�V
	//static cv::Mat object_img=input.img_resize.clone();
	for(int t=0;t<input.EventMAX();t++)
	{	
		Geometory::Point2D posCenter(input.get_Event_cx(t),input.get_Event_cy(t));//���̂̏d�S
		//input.pixels_O().convertTo(lay,CV_8UC3);
		char* hour = Util::string2Char(Util::Time().getCurrentTimeString("%H").c_str());
		char* minute = Util::string2Char(Util::Time().getCurrentTimeString("%M").c_str());

		//�����̕��̃f�[�^��\��

		int e=input.Event_Log(t);
		if(input.get_Event_Eve(t)==0)//�������茟�m
		{
			cout<<"take"<<endl;
			int object_max=input.size_t_object_id();
			input.set_Event_human(t,input.get_ObjectTab_human(e));
			for(int i=0;i<object_max;i++)
			{
				cout << "input.get_t_object_id(i).model_id: " << input.get_t_object_id(i).model_id << ",input.get_t_object_id(i).object_id: " << input.get_t_object_id(i).object_id << endl;
				if(input.get_t_object_id(i).model_id==input.get_Event_model(t))
				{
					if(input.get_t_object_id(i).touch != NULL)//�ێ����̂̍X�V
					{
						input.set_Event_human(t,input.get_t_object_id(i).touch);//�ŗL�l��ID��o�^
						if(!input.set_t_human_id_have_obj(input.get_t_object_id(i).touch,i))//�������Ă���l�̐l��ID)
						{//0�̎����łɐl�������������Ă���Ƃ��čX�V��ύX
							int a=0;
							FILE *fl;
							fopen_s(&fl,"../DB/Object-in-out-DB/Object.log","a");
							fprintf(fl,"-1 %d %d %d\n",input.get_Event_model(t),input.get_t_object_id(i).touch,frame);
							fclose(fl);
							for(int obj=0;obj<input.get_t_object_id(i).hide_obj.size();obj++)
							{
								FILE *fl1;
								fopen_s(&fl1,"../DB/Object-in-out-DB/Object.log","a");
								fprintf(fl1,"-1 %d %d %d\n",input.get_t_object_id(i).hide_obj[obj].first,input.get_t_object_id(i).touch,frame);
								fclose(fl1);
							}
						}
					}
					else if(input.get_ObjectTab_human(e)!=-1)//���̏��ɐڐG��񂪂Ȃ������̌����ɂ�������Q�Ƃ���
					{	
						if(!input.set_t_human_id_have_obj(input.get_ObjectTab_human(e),i))//�������Ă���l�̐l��ID)
						{//0�̎����łɐl�������������Ă���Ƃ��čX�V��ύX
							int a=0;
							FILE *fl;
							fopen_s(&fl,"../DB/Object-in-out-DB/Object.log","a");
							fprintf(fl,"-1 %d %d %d\n",input.get_Event_model(t),input.get_ObjectTab_human(e),frame);
							fclose(fl);
							for(int obj=0;obj<input.get_t_object_id(i).hide_obj.size();obj++)
							{
								FILE *fl1;
								fopen_s(&fl1,"../DB/Object-in-out-DB/Object.log","a");
								fprintf(fl1,"-1 %d %d %d\n",input.get_t_object_id(i).hide_obj[obj].first,input.get_ObjectTab_human(e),frame);
								fclose(fl1);
							}
						}
					}

					input.erace_t_object_id(i);
					break;
				}
			}
		}
		else//��������.�ړ����m
		{ 
			if(input.get_ObjectTab_human(e)>-1){
			input.set_Event_human(t,input.get_ObjectTab_human(e)) ;}
			else{input.set_Event_human(t,-1);}
			input.set_t_object_id(t);//���̏��̓o�^
			if (input.get_Event_Eve(t) == 2)input.set_n_event_state_L5(PII(3, input.get_ObjectTab_human(input.Event_Log(t))), PII(input.get_Event_key(t), input.get_Event_obj(t)));
			// if(input.get_Event_Eve(t)==2)input.set_n_event_state_L5(PII(3,input.get_ObjectTab_human(input.Event_Log(t))),PII(input.get_Event_key(t),input.get_Event_model(t)));
			int size=input.size_t_object_id()-1;
			cout << "input.get_Event_Eve(t): " << input.get_Event_Eve(t) << ", t: " << t << ", size: " << size << endl;
			bool track_flag=false;
			if(input.get_ObjectTab_human(input.Event_Log(t))!=-1)//�ړ��̌��m
			{
				for(int m=0;m<input.size_t_human_id();m++)//���m�l���Əƍ�
				{
					if(input.get_t_human_id(m).id==input.get_ObjectTab_human(input.Event_Log(t)))//���̌����ɂ���ڐG�����l������ID���ǐՐl��ID�̏�񂪓����ꍇ�ړ����m����
					{
						if(input.get_t_human_id(m).have_obj.size()>0)//�l���ǐՂ�p�����ړ��̌��m
						{
							cout<<"tracking"<<endl;
							//�������񂾕��̂Ɏ����Ă�����S������̔w����
							cout << "first: " << input.get_t_human_id(m).have_obj[0].first << ", second: " << input.get_t_human_id(m).have_obj[0].second << endl;
							// input.set_t_object_id_object_id(size, input.get_t_human_id(m).have_obj[0].first);
							input.set_t_object_id_object_id(size, input.get_t_human_id(m).object_ID[input.get_t_human_id(m).object_ID.size() - 1]);
							
							// input.set_n_event_state_L5(PII(3, input.get_t_human_id(m).id), PII(input.get_t_human_id(m).have_obj[0].first, input.get_Event_obj(t)));
							// input.set_n_event_state_L5(PII(3, input.get_t_human_id(m).id), PII(input.get_t_human_id(m).have_obj[0].first, input.get_Event_model(t)));
							input.set_n_event_state_L5(PII(3, input.get_t_human_id(m).id), PII(input.get_t_human_id(m).object_ID[input.get_t_human_id(m).object_ID.size() - 1], input.get_Event_model(t)));
							input.set_Event_Eve(t,2);//�������݂��ړ��ɕύX

							// input.set_Event_key(t, input.get_t_human_id(m).have_obj[0].first);
							input.set_Event_key(t, input.get_t_human_id(m).object_ID[input.get_t_human_id(m).object_ID.size() - 1]);
							// input.set_Event_obj(t, obj_idname[input.get_t_human_id(m).have_obj[0].first]);
							input.set_Event_obj(t, obj_idname[input.get_t_human_id(m).object_ID[input.get_t_human_id(m).object_ID.size() - 1]]);

							input.erase_t_human_id_have_obj(m,0);

							FILE *fl;//���̂�����
							fopen_s(&fl,"../DB/HI-Event-DB/Event.log","a");
							fprintf(fl,"%d t %d %d",frame,input.get_t_human_id(m).id,input.get_t_object_id(size).model_id);

						    char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
							sprintf_s(filenameRGB,"../DB/HI-Event-DB/%d-t-%08lu.png",input.get_t_human_id(size).id,frame);
							cv::imwrite(string(filenameRGB), input.img_resize);

							for(int io=0;io<input.get_t_human_id_have_obj_size(m);io++)
							{
								fprintf(fl," %d",input.get_t_human_id_have_obj(m,io));
							}
							fprintf(fl,"\n");
							fclose(fl);

							if(input.get_t_human_id(m).have_obj.size()>0)input.set_T_object_hide_data(size,m);
							input.erase_t_human_id_have_obj_clear(m);

							track_flag=true;
							
						}
						//input.set_n_event_state_L5_t(PII(input.get_t_human_id(m).id,input.get_t_object_id(size).model_id));
					}
				}
			}
			if(!track_flag&&input.get_Event_Eve(t)==1)
			{cout<<"bring"<<endl;
				FILE *fl;//���̂�����
				fopen_s(&fl,"../DB/Object-in-out-DB/Object.log","a");
				fprintf(fl,"1 %d %d %d\n",input.get_t_object_id(size).model_id,input.get_ObjectTab_human(input.Event_Log(t)),frame);
				fclose(fl);
			}
		}


		input.init_bring_object_data();
		int object_max = input.size_t_object_id();
		cout << "object_max: " << object_max << endl;
		//cout<<object_max<<endl;
		for(int i=1;i<=object_max;i++)
		{	
			int x1=input.get_t_object_id(i-1).x1;
			int x2=input.get_t_object_id(i-1).x2-input.get_t_object_id(i-1).x1;
			int y1=input.get_t_object_id(i-1).y1;
			int y2=input.get_t_object_id(i-1).y2-input.get_t_object_id(i-1).y1;
			int areX,areY;//N*N�������������s��
			
			for(int y=y1;y<y1+y2;y++)
			{
				for(int x=x1;x<x1+x2;x++)
				{
					if(object_img[i].objectT(x,y)==OBJECT)//input.get_Object_label(e,i,j)!=0)
					{
						input.get_bring_object_data().at<float>(y,x)=input.get_depth_is_diff(x,y);
					}
				}
			}//�������������s���悤�ɂ���
			areX=std::max(1,x2/10);
			areY=std::max(1,y2/10);
			int are=std::max(1,(areX+areY)/2);
			cv::Rect roi_rect(x1+are,y1+are,x2-are,y2-are);
			//cout<<roi_rect.x<<":"<<roi_rect.y<<":"<<roi_rect.width<<":"<<roi_rect.height<<endl;
			//�f�[�^�X�V
			//���f�B�A���t�B���^�ɂ���ĕ�����
			cv::Mat src_roi =input.get_bring_object_data()(roi_rect);
			cv::Mat dst_roi =input.get_bring_object_data()(roi_rect);
			cv::blur(dst_roi, src_roi,cv::Size(areX,areY));
			denta_object(input,i-1);
		}
	}

	cv::Mat object_img=input.img_resize.clone();
	cv::Vec3b pixel_value;
		
	for(int j=0; j< input.Height(); j++)
	{
			
		for(int i=0; i<input.Width(); i++)
		{
			pixel_value.val[0]=255*(max(0,((int)input.get_bring_object_data().at<float>(j,i)-DEPTH_RANGE_MIN)))/DEPTH_RANGE_MAX;
			pixel_value.val[1]=pixel_value.val[0];
			pixel_value.val[2]=pixel_value.val[0]; 
			object_img.at<cv::Vec3b>(j,i)=pixel_value;
		}
	}
		
	int object_max=input.size_t_object_id();
	cv::Mat object_depth=input.depth_O().clone();
	for(int i=0;i<object_max;i++)
	{ 
		int x1=input.get_t_object_id(i).x1;
		int x2=input.get_t_object_id(i).x2-input.get_t_object_id(i).x1;
		int y1=input.get_t_object_id(i).y1;
		int y2=input.get_t_object_id(i).y2-input.get_t_object_id(i).y1;
//		int areX,areY;
		if(1/*size_d>5000*/)
		{
			int human_max=input.size_t_human_id();
			for(int c=0;c<human_max;c++)
			{
				//cout<<size_d<<endl;
				if(input.get_t_human_id(c).skeletons.size()>0&&input.get_t_human_id(c).kinect_id>0&&input.get_t_human_id(c).have_obj.size()>0&&input.get_t_object_id(i).touch==input.get_t_human_id(c).id)
				{//���i������ێ����L�����̂ɐڐG�����l���Ɛl����������
					//int t=input.get_t_human_id(c).skelton;
					int hx1=input.get_t_human_id(c).skeletons[22].first;
					int hy1=input.get_t_human_id(c).skeletons[22].second;
					int hx2=input.get_t_human_id(c).skeletons[24].first;
					int hy2=input.get_t_human_id(c).skeletons[24].second;
				    int area=5;
					if((x1-area<hx1&&hx1<x2+x1+area&&y1-area<hy1&&hy1<y2+y1+area)||(x1-area<hx2&&hx2<x2+x1+area&&y1-area<hy2&&hy2<y1+y2+area))//�B�����m
					{
						
						float diff_r=fabs(input.get_depth(hx1,hy1)-input.get_depth_is_diff(hx1,hy1));
						float diff_l=fabs(input.get_depth(hx2,hy2)-input.get_depth_is_diff(hx2,hy2));
						if(diff_r<700||diff_l<700)
						{
							int size_denta=denta_object(input,i);//�E�݂̍ő�l���o��
							int pose=input.get_t_human_id_pose_match(c,5);
							if(pose==-1){input.erase_t_human_id_pose_clear(c);}
							input.set_t_human_id_pose(c,5,frame);
							if(input.get_t_human_id_pose_size(c)>=hide_count&&size_denta>20/*(input.get_t_human_id_have_obj_data(c,0)/2)*/)
							{
								
								cv::Mat touch_img=input.img_resize.clone();
								cv::rectangle(touch_img,cv::Rect(x1,y1,x2,y2),cv::Scalar(0,0,255));
								stringstream cc;

								cc<<"Hide_object:"<<input.get_t_human_id(c).have_obj[0].first;
								cv::putText(touch_img, cc.str(),cv::Point(10,210) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
								//imshow("Lv_5:Event",touch_img);

								char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
								sprintf_s(filenameRGB,"../DB/HI-Event-DB/%d-h-%08lu.png",input.get_t_human_id(c).id,frame);
								cv::imwrite(string(filenameRGB), touch_img);

								input.set_T_object_hide_data(i,c);//input.get_t_human_id_have_obj(c,0));�����Ă��镨��S���B��
						
								FILE *fl;//���̂�����
								fopen_s(&fl,"../DB/HI-Event-DB/Event.log","a");
								fprintf(fl,"%d h %d %d",frame,input.get_t_human_id(c).id,input.get_t_object_id(i).model_id);
						
								for(int io=0;io<input.get_t_human_id_have_obj_size(c);io++)
								{
									fprintf(fl," %d",input.get_t_human_id_have_obj(c,io));
								}
								fprintf(fl,"\n");
								fclose(fl);

								input.set_n_event_state_L5(PII(1, input.get_t_human_id(c).id), PII(input.get_t_object_id(i).model_id, input.get_t_human_id_have_obj(c, 0)));
								// input.set_n_event_state_L5(PII(1, input.get_t_human_id(c).id), PII(input.get_t_object_id(i).object_id, input.get_t_human_id_have_obj(c, 0)));
							
								input.erase_t_human_id_have_obj_clear(c);
							}
						}
					}
				}
			}
		}
	}
}

void Event::touch_event(image &input,unsigned long frame, bool *touch_flag)//�������񂾕��̂ɑ΂���ڐG����
{
	int touch_count = 0;	// 2019/1/11�ǉ�(�L�{)
	for(int num=0;num<input.size_t_object_id();num++)		// object_id��size_t(int)�Ƃ������Ƃ͌��m���Ă��镨�̂�ID�������[�v�H
	{
		int ox1,ox2,oy1,oy2,cx,cy,dep=15;//���̗̂��[		// o��object, c��center, dep�͋�����񂩁H���~�@�ˊO�g�̕�
		ox1=max(0,input.get_t_object_id(num).x1-dep);		// num�Ԗڂ�ID�̕��̂�input����get���āA���̃����o�[x1����dep(15)�����������̂�0�Ƃ��r�A�傫������ox1�ɑ��
		ox2=min(320,input.get_t_object_id(num).x2+dep);		//										���̃����o�[x2��dep(15)�𑫂������̂�320�Ƃ��r�A����������ox2�ɑ��
		oy1=max(0,input.get_t_object_id(num).y1-dep);		//										���̃����o�[y1����dep(15)�����������̂�0�Ƃ��r�A�傫������oy1�ɑ��
		oy2=min(240,input.get_t_object_id(num).y2+dep);		//										���̃����o�[y2��dep(15)�𑫂������̂�320�Ƃ��r�A����������oy2�ɑ��
		cx=input.get_t_object_id(num).xcenter;				//										���̃����o�[xcenter��cx�ɑ��
		cy=input.get_t_object_id(num).ycenter;				// 										���̃����o�[ycenter��cy�ɑ��
		int human_max=input.size_t_human_id();				// human_max�Ɍ��m���Ă�l����ID�����R�s�[
		int ox_range = ox2 - ox1;
		int oy_range = oy2 - oy1;

		for(int i=0;i<human_max;i++)	//���m�����l�������[�v
		{
			if(input.get_t_human_id(i).skeletons.size()>0&&input.get_t_human_id(i).kinect_id>0)		// i�Ԗڂ�ID�̐l������skeletons.size()��0�ȏ�(���݂���)�A����
			{																						// i�Ԗڂ�ID�̐l������kinect_id��0�ȏ�(ID:0�Ԃ�Kinect�ɂ͂Ȃ��H)
				//int t=input.get_t_human_id(i).skelton;
				int x1=input.get_t_human_id(i).skeletons[22].first;									// x1,y1��ID:i�̐l����skeleton��22�Ԋ֐߂̍��W
				int y1=input.get_t_human_id(i).skeletons[22].second;								
				int x2=input.get_t_human_id(i).skeletons[24].first;									// x2,y2��ID:i�̐l����skeleton��24�Ԋ֐߂̍��W
				int y2=input.get_t_human_id(i).skeletons[24].second;
				float depth_hand_r=input.get_depth(x1,y1);											// ���W(x1, y1)�̐[�x����get
				float depth_hand_l=input.get_depth(x2,y2);											// ���W(x2, y2)�̐[�x����get
				float depth_flont_r=input.get_depth_is_diff(x1,y1);									// ���W(x1, y1)��depth_is_diff(?)��get
				float depth_flont_l=input.get_depth_is_diff(x2,y2);									// ���W(x2, y2)��depth_is_diff(?)��get

				if((ox1+ox_range/4<x1&&x1<ox2-ox_range/4&&oy1+oy_range/4<y1&&y1<oy2-oy_range/4)
					||(ox1+ox_range/4<x2&&x2<ox2-ox_range/4&&oy1+oy_range/4<y2&&y2<oy2-oy_range/4))	// 22�Ԋ֐߂�24�Ԋ֐߂����̈ʒuBOX�̒��S�G���A�ɔ���Ă���
				{
					float Ddiff_r=std::fabs(input.get_depth_is_diff(cx,cy)-depth_hand_r);			// ���̒��S(cx, cy)��depth_is_diff(?) �� 22�Ԋ֐ߍ��W(x1, y1)��depth �̍���diff_1�ɑ��
					float Ddiff_l=std::fabs(input.get_depth_is_diff(cx,cy)-depth_hand_l);			// ���̒��S(cx, cy)��depth_is_diff(?) �� 24�Ԋ֐ߍ��W(x2, y2)��depth �̍���diff_2�ɑ��
					//float Pdiff2_r=pow(cx-x1, 2.)+pow(cy-y1, 2.);									// ���ʏ�̕��̒��S�Ǝ����W�̍�(�E��)
					//float Pdiff2_l=pow(cx-x2, 2.)+pow(cy-y2, 2.);									// ���ʏ�̕��̒��S�Ǝ����W�̍�(����)
					//float diff_r=sqrt(Pdiff2_r + pow((double)Ddiff_r, 2.));							// ���̒��S�Ǝ��̎O�����������v�Z(�E��)
					//float diff_l=sqrt(Pdiff2_l + pow((double)Ddiff_l, 2.));							// ���̒��S�Ǝ��̎O�����������v�Z(����)

					if(Ddiff_l<100||Ddiff_r<100)														// �e������450�����A�܂�e�֐߂ƕ��̂̐[�x���\���߂�
					{
					    //cout<<"object_touch"<<endl;
						input.set_t_object_id_touch(num,input.get_t_human_id(i).id,frame);			// ID:num�̕��́AID:i�̐l��ID�A���݃t���[���ԍ���p����
																									// t_object_id[num].touch�ɐl��ID(i)���A
																									// t_object_id[num].t_frame�Ɍ��݃t���[���ԍ���set�yset_t_object_id_touch()�z
						//�ڐG���̉摜��p��
						cv::Mat touch_img=input.img_resize.clone();
						
						cv::rectangle(touch_img,cv::Rect(ox1+dep,oy1+dep,ox2-ox1-2*dep,oy2-oy1-2*dep),cv::Scalar(0,0,255));
						stringstream cc;
						cc<<"human_ID:"<<input.get_t_human_id(i).id;
						cv::putText(touch_img, cc.str(),cv::Point(10,20) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
						//imshow("lv_2_touch_obj",touch_img);
						
						char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
						sprintf_s(filenameRGB,"../DB/Human-DB/touch/%d-t-%08lu.png",input.get_t_human_id(i).id,frame);
						cv::imwrite(string(filenameRGB), touch_img);
						FILE *fl3;//���̂�����
						fopen_s(&fl3,"../DB/Human-DB/touch/touch.log","a");
						fprintf(fl3,"%d t %d %d\n",input.get_t_human_id(i).id,input.get_t_object_id(num).model_id,frame);
						fclose(fl3);


						PII a(input.get_t_human_id(i).id,input.get_t_object_id(num).model_id);
						input.set_n_event_state_L5_t(a);

						touch_count++;		// 2019/1/11�ǉ�(�L�{)
					}
				}
				/*else if(((fabs(depth_hand_l-depth_flont_l)<100&&input.get_obj_mask(x2,y2)==255)||(fabs(depth_hand_r-depth_flont_r)<100)&&input.get_obj_mask(x1,y1)==255)&&input.get_t_human_id_have_obj_size(i)>0)//��ԓ��ɕ����B�����ꍇ������*/
				//()�Ԃ��Ȃ񂩂��������Ȃ��H����������Ȃ��H��

				else if( ( ( fabs( depth_hand_l-depth_flont_l ) < 100 &&			// 24�Ԋ֐ߍ��W(x2, y2)�̐[�x��depth_is_diff�̍���100�����A����
									input.get_obj_mask( x2, y2 ) == 255 ) ||		// 24�Ԋ֐ߍ��W(x2, y2)�̃}�X�N��255(���A���m�͈͊O)			�܂���
						   ( fabs( depth_hand_r-depth_flont_r ) < 100 &&			// 22�Ԋ֐ߍ��W(x1, y1)�̐[�x��depth_is_diff�̍���100�����A����
									input.get_obj_mask( x1, y1 ) == 255 ) ) &&		// 22�Ԋ֐ߍ��W(x1, y1)�̃}�X�N��255(���A���m�͈͊O)					����
									input.get_t_human_id_have_obj_size( i ) > 0 )	// human_id_have_obj_size(i)(?)��0���߂Ȃ�E�E�E			
																					// ��ԓ��ɕ����B�����ꍇ������
				{//�ǂɐڐG
					//cout<<"wall_touch"<<endl;
					cv::Mat touch_img=input.img_resize.clone();
					//cv::rectangle(touch_img,cv::Rect(x1,y1,x2,y2),cv::Scalar(0,0,255));
					cv::circle(touch_img, cv::Point(x1,y1),1, cv::Scalar(255,0,255), 5, 4);
					cv::circle(touch_img, cv::Point(x2,y2),1, cv::Scalar(255,0,255), 5, 4);
					stringstream cc;
					cc<<"Hide_object:"<<input.get_t_human_id(i).have_obj[0].first;
					cv::putText(touch_img, cc.str(),cv::Point(10,20) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
					//imshow("Lv_5:Event",touch_img);

					char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
					sprintf_s(filenameRGB,"../DB/HI-Event-DB/%d-tw-%08lu.png",input.get_t_human_id(i).id,frame);
					cv::imwrite(string(filenameRGB), touch_img);

					//input.set_n_event_state_L5(PII(1,input.get_t_human_id(i).id),input.get_t_object_id(i).hide_obj[0]);

					FILE *fl;//���̂�����
					fopen_s(&fl,"../DB/HI-Event-DB/Event.log","a");
					fprintf(fl,"%d h %d %d\n",frame,input.get_t_human_id(i).id,input.get_t_human_id(i).have_obj[0].first);
					fclose(fl);
				}
			}
		}	
	}
	if( touch_count > 0 ){																// 2019/1/11�ǉ�(�L�{)
		*touch_flag = true;																// ��
		cout << "touching occerd in this frame: " << touch_count << "(times)" << endl;	// ��
	}																					// ��
	else{ *touch_flag = false; }														// �����܂�
}

void Event::style_detect(image &input,unsigned long frame)
{
	int human_max=input.size_t_human_id();
	for(int i=0;i<human_max;i++)	
	{
		if(input.get_t_human_id(i).skeletons.size()>0&&input.get_t_human_id(i).have_obj.size()/*&&input.get_t_human_id_pose_size(i)==0*/)
		{
			//int t=input.get_t_human_id(i).skeletons;
			int x1=input.get_t_human_id(i).skeletons[21].first;//�E��̈ʒu
			int y1=input.get_t_human_id(i).skeletons[21].second;
			float d1=input.get_depth(x1,y1);
			int x2=input.get_t_human_id(i).skeletons[23].first;//����̈ʒu
			int y2=input.get_t_human_id(i).skeletons[23].second;
			float d2=input.get_depth(x2,y2);
			int xh=input.get_t_human_id(i).skeletons[3].first;//���̈ʒu
			int yh=input.get_t_human_id(i).skeletons[3].second;
			float dh=input.get_depth(xh,yh);
			
			//���ɉB���p���̌��m
			int xw=input.get_t_human_id(i).skeletons[0].first;//���̈ʒu
			int yw=input.get_t_human_id(i).skeletons[0].second;
			float dw=input.get_depth(xw,yw);
			int xl=input.get_t_human_id(i).skeletons[20].first;//�񉺂̈ʒu
			int yl=input.get_t_human_id(i).skeletons[20].second;
			//float dl=input.get_depth(xw,yw);
			//float diff_d_wait=dw-dl;
			int diff_wait_x=xw-xl;
			int diff_wait_y=yw-yl;  

			int xw_r=input.get_t_human_id(i).skeletons[4].first-diff_wait_x;//�E���̈ʒu
			int yw_r=input.get_t_human_id(i).skeletons[4].second-diff_wait_y;
			float dw_r=input.get_depth(xw_r,yw_r);
			int xw_l=input.get_t_human_id(i).skeletons[8].first-diff_wait_x;//�����̈ʒu
			int yw_l=input.get_t_human_id(i).skeletons[8].second-diff_wait_y;
			float dw_l=input.get_depth(xw_l,yw_l);
			
			
			//if(input.get_t_human_id_pose_size(i)==0)//�p�����ɉ����Ȃ���
			//{
				int dex1=std::abs(x1-xh)+std::abs(y1-yh);//���ޓ��쌟�m
				int dex2=std::abs(x2-xh)+std::abs(y2-yh);

				if((dex1<25||dex2<25)&&(std::fabs(dh-d1)<700||fabs(dh-d2)<700))
				{
					int pose=input.get_t_human_id_pose_match(i,1);
					if(pose==-1){input.erase_t_human_id_pose_clear(i);}
					input.set_t_human_id_pose(i,1,frame);//���m�����t���[���ԍ����L�����Ă����C���ȏ㓯���ꍇ�͂���\������
					if(input.get_t_human_id_pose_size(i)==drink_count)
					{
						cv::Mat	drink_img=input.img_resize.clone();
						stringstream aa;
						aa<<"Drink_humanID:"<<input.get_t_human_id(i).id;
						cv::putText(drink_img, aa.str(),cv::Point(10,210) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);

						//imshow("Lv_5:Event",drink_img);
						int xt1=input.get_t_human_id(i).x1;
						int xt2=input.get_t_human_id(i).x2-input.get_t_human_id(i).x1;
						int yt1=input.get_t_human_id(i).y1;
						int yt2=input.get_t_human_id(i).y2-input.get_t_human_id(i).y1;
						cv::rectangle(drink_img,cv::Rect(xt1,yt1,xt2,yt2),cv::Scalar(0,0,255));
						char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
						sprintf_s(filenameRGB,"../DB/HI-Event-DB/%d-d-%08lu.png",input.get_t_human_id(i).id,frame);
						cv::imwrite(string(filenameRGB), drink_img);

						FILE *fl;//���̂�����
						fopen_s(&fl,"../DB/HI-Event-DB/Event.log","a");
						fprintf(fl,"%d d %d %d\n",frame,input.get_t_human_id(i).id,input.get_t_human_id(i).have_obj[0].first);
						fclose(fl);

						FILE *fl2;//���̂�����
						fopen_s(&fl2,"../DB/Human-DB/Motion/Motion.log","a");
						fprintf(fl2,"%d d %d %d\n",frame,input.get_t_human_id(i).id,input.get_t_human_id(i).have_obj[0].first);
						fclose(fl2);

						input.set_n_event_state_L3(PII(input.get_t_human_id(i).id,2));
						input.set_n_event_state_L5(PII(2,input.get_t_human_id(i).id),PII(input.get_t_human_id(i).have_obj[0].first,-1));
						
					}
				}
				else
				{
					if(input.get_t_human_id_pose_size(i)>(drink_count+2)){input.erase_t_human_id_pose_clear(i);}
				}

				dex1=std::abs(x1-xw_r)+std::abs(y1-yw_r);//���|�P�b�g�ɉB������̌��m
				dex2=std::abs(x2-xw_l)+std::abs(y2-yw_l);
				int dex3=std::abs(x1-xw_l)+std::abs(y1-yw_l);
				int dex4=std::abs(x2-xw_r)+std::abs(y2-yw_r);
				if((dex1<10||dex2<10||dex3<10||dex4<10)/*&&input.get_t_human_id_pose_size(i)==0*/)
				{
					input.set_t_human_id_pose(i,2,frame);//���m�����t���[���ԍ����L�����Ă����C���ȏ㓯���ꍇ�͂���\������
					cv::Mat	hide_img=input.img_resize.clone();
					stringstream aa;
					aa<<"poket_hide_humanID:"<<input.get_t_human_id(i).id;
					cv::putText(hide_img, aa.str(),cv::Point(10,210) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
					//imshow("Lv_5:Event",hide_img);
					
					char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
					sprintf_s(filenameRGB,"../DB/HI-Event-DB/%d-hp-%08lu.png",input.get_t_human_id(i).id,frame);
					cv::imwrite(string(filenameRGB), hide_img);
					
					FILE *fl;//���̂��B��
					fopen_s(&fl,"../DB/HI-Event-DB/Event.log","a");
					fprintf(fl,"%d h %d %d",frame,input.get_t_human_id(i).id,input.get_t_human_id(i).have_obj[0].first);
					for(int io=1;io<input.get_t_human_id_have_obj_size(i);io++)
					{
						fprintf(fl," %d",input.get_t_human_id_have_obj(i,io));
					}
					fprintf(fl,"\n");
					fclose(fl);
					input.set_n_event_state_L3(PII(input.get_t_human_id(i).id,1));
					//input.set_n_event_state_L5(PII(1,input.get_t_human_id(i).id),PII(input.get_t_human_id(i).have_obj[0],-1));

				}
			//}	
		}	
		else if(input.get_t_human_id_pose_size(i)>0){
			if(frame-input.get_t_human_id_pose_frame(i,0)>15){
				input.erase_t_human_id_pose_clear(i);
			}
		}
	}
}

void Event::d_human_maching(image &input,int frame)
{
	int human_max=10;
	static int human_id=1;
	int human_size=input.size_t_human_id();
	int si=80;
	bool human_num[10];for(int i=0;i<10;i++){human_num[i]=false;}
	bool sub_human_num[11];for(int i=1;i<11;i++){sub_human_num[i]=false;}
	int ans,sub_ans;
	for(int t=1;t<=input.HumanData();t++)//�����L�l�N�gID�̕����ɍ��킹��
	{
		for(int i=0;i<human_size;i++)
		{
			if(human_num[i]==false&&(input.get_t_human_id(i).kinect_id!=-1&&input.get_t_human_id(i).kinect_id==input.get_HumanTab_kinect(t)))
			{

					input.set_t_human_area(i,t);//�X�V
					ans=i;
					sub_human_num[t]=true;
					human_num[i]=true;
					i=human_size;
			}
		}
	}
	for(int t=1;t<=input.HumanData();t++)
	{
		if(input.get_HumanTab_size(t)>20)
		{
			if(sub_human_num[t]==false)
			{
				int hx1,hx2,hy1,hy2,cx,cy;//�l���̗��[
					hx1=input.get_HumanTab_s_w(t); 
					hx2=input.get_HumanTab_e_w(t);
					hy1=input.get_HumanTab_s_h(t);
					hy2=input.get_HumanTab_e_h(t);
					cx=input.get_HumanTab_cx(t);
					cy=input.get_HumanTab_cy(t);
					int Diff=40000;
					ans=-1;
				//bool flag=false;
				for(int i=0;i<human_size;i++)
				{
					if(human_num[i]==false){
						int thx1,thx2,thy1,thy2,tcx,tcy;//�l���̗��[
						thx1=input.get_t_human_id(i).x1;
						thx2=input.get_t_human_id(i).x2;
						thy1=input.get_t_human_id(i).y1;
						thy2=input.get_t_human_id(i).y2;
						tcx=input.get_t_human_id(i).xcenter;
						tcy=input.get_t_human_id(i).ycenter;
						//cout<<tcx<<tcy<<cx<<cy<<endl;
						if((cx-si<tcx&&tcx<cx+si)&&(cy-si<tcy&&tcy<cy+si))/*&&(hx1<300&&hx2>20&&hy1<220&&hy2>20)*///�قڗގ�
						{
							if((abs(cx-tcx)+abs(cy-tcy))<Diff){
							//i=human_size;
							Diff=abs(cx-tcx)+abs(cy-tcy);
							ans=i;
							sub_ans=t;
							//flag=true;
							}
					}
					}
				}
				if(ans!=-1){human_num[ans]=true;sub_human_num[sub_ans]=true;input.set_t_human_area(ans,sub_ans);}//�X�V}
				else if(sub_human_num[t]==false)//if(sub_human_num[t]==false)//if(flag==false/*&&(hx1<300&&hx2>20&&hy1<220&&hy2>20)*/)//�Ȃ����V�K�ǉ�
				{
					cv::Mat human_img=input.img_resize.clone();
					cv::rectangle(human_img, cv::Point(hx1,hy1), cv::Point(hx2, hy2), cv::Scalar(0, 152, 243), 2, 4);
					stringstream aa;
					aa<<"ID:"<<human_id;
					cv::putText(human_img, aa.str(),cv::Point(hx1,hy1-15) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     

					FileOperator path;std::string dir;std::ostringstream frame_name;
					char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
					frame_name << human_id;
					dir="../DB/Track-Human-DB/"+frame_name.str();
					path.MakeDir(dir.c_str());
					dir.clear();
					sprintf_s(filenameRGB,"../DB/Track-Human-DB/%d/%08lu.png",human_id,frame);
					cv::imwrite(string(filenameRGB),human_img );

					input.set_n_event_state_L2_h(PII(1,human_id));
			
					FILE *fl;
					fopen_s(&fl,"../DB/Track-Human-DB/Human.log","a");
					fprintf(fl,"%d %d %d\n",human_id,1,frame);
					fclose(fl);
					input.set_t_human_id(t);
					input.set_t_human_id_id(input.size_t_human_id()-1,human_id);
					human_id++;
					human_num[input.size_t_human_id()-1]=true;
					sub_human_num[t]=true;
				}
			}
		}
	}
	for(int i=0;i<input.size_t_human_id();i++)
	{
		
		if(human_num[i]==false)//�l������ԓ��ɂ��Ȃ��Ȃ������폜
		{
			int xx1=input.get_t_human_id(i).x1;
			int xx2=input.get_t_human_id(i).x2;
			int yy1=input.get_t_human_id(i).y1;
			int yy2=input.get_t_human_id(i).y2;
			cv::Mat human_img=input.img_resize.clone();
			
			cv::rectangle(human_img, cv::Point(xx1,yy1), cv::Point(xx2, yy2), cv::Scalar(0, 152, 243), 2, 4);
			stringstream aa;
			aa<<"ID:"<<input.get_t_human_id(i).id;
			cv::putText(human_img, aa.str(),cv::Point(xx1,yy1-15) /*cv::Point(50,50)*/, cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     

			char filenameRGB[256];  //TODO: �����������ԁ@���悻0.08�b�@�Ԃꂪ�Ђǂ�
			sprintf_s(filenameRGB,"../DB/Track-Human-DB/%d/%08lu.png",input.get_t_human_id(i).id,frame);
			cv::imwrite(string(filenameRGB), human_img);
			FILE *fl;
			fopen_s(&fl,"../DB/Track-Human-DB/Human.log","a");
			fprintf(fl,"%d %d %d\n",input.get_t_human_id(i).id,-1,frame);
			fclose(fl);
			//���̐l�������̂������Ă��������̂̃A�E�g��������
			if(input.get_t_human_id(i).have_obj.size()>0)
			{
				for(int obj=0;obj<input.get_t_human_id(i).have_obj.size();obj++)
				{
					FILE *fl;
					fopen_s(&fl,"../DB/Object-in-out-DB/Object.log","a");
					fprintf(fl,"%d %d %d %d\n",-1,input.get_t_human_id(i).have_obj[obj].first,input.get_t_human_id(i).id,frame);
					fclose(fl);
				}
			}
			input.set_n_event_state_L2_h(PII(-1,input.get_t_human_id(i).id));
			input.erace_t_human_id(i);
			for(int num=i;num<human_max-1;num++){human_num[num]=human_num[num+1];}
			i--;
		}
		
	}
	for(int i=0;i<input.size_t_human_id();i++)
	{
		if(frame-input.get_t_human_id(i).frame>20000&&input.get_t_human_id(i).kinect_id==-1)
		{
			int x1=input.get_t_human_id(i).x1;
			int x2=input.get_t_human_id(i).x2;
			int y1=input.get_t_human_id(i).y1;
			int y2=input.get_t_human_id(i).y2;
			for(int y=y1;y<y2;y++){
				for(int x=x1;x<x2;x++){
					input.set_mask(x,y,255);
				}
			}
		}
	}
}

void Event::calcBack( image &input, cv::Mat firstHue, cv::Mat firstMask, cv::Mat objectHue, cv::Mat objectMask, cv::Mat aroundHue, cv::Mat aroundMask, cv::Mat trackArea, int nowFrame )
{
	/*	cv::imshow( "hueMap", objectHue );
	cv::waitKey( 1 );
	cv::imshow( "objectMask", objectMask );
	cv::waitKey( 1 );
	*/
	char fileName[ 128 ];

	cv::Mat backproject;
	cv::Mat firstHist, hist;

	int hdims = 180;
	int histSize[ ] = { hdims }; // 0, 180
	float hrangesArray[ ] = { 0, 180 };
	const float *hranges = hrangesArray;
	int histChannel[ ] = { 0, 1 };
	int backChannel = 0;
	float sranges[ ] = { 0, 256 };
	const float* ranges[ ] = { hrangesArray, sranges };

	//Histogram�m�F�p
	cv::Mat objectColorR, objectColorG, objectColorB; //���̂�Ώ�
	cv::Mat aroundColorR, aroundColorG, aroundColorB; //���ӂ�Ώ�
	cv::Mat backhist;
	cv::Mat histDiff;
	cv::Mat histVisual, histAroundVisual;

	//臒l
	cv::Mat firstHistThresh;
	cv::Mat objectHistThresh;
	cv::Mat aroundHistThresh;
	cv::Mat histDiffThresh;
	cv::Mat histAround, histOriginal;

	//�q�X�g�O�����`��
	cv::Mat histImg, histAroundImg, histDiffImg;

	objectColorR = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
	objectColorG = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
	objectColorB = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) ); //���̂�Ώ�
	aroundColorR = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
	aroundColorG = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );
	aroundColorB = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) ); //���ӂ�Ώ�
	backhist = cv::Mat( cvSize( input.getwidth( ), input.getheight( ) ), CV_8UC1, cv::Scalar( 0, 0, 0 ) );

	histImg = cv::Mat::zeros( 180, 256, CV_8UC3 );
	histAroundImg = cv::Mat::zeros( 180, 256, CV_8UC3 );
	histDiffImg = cv::Mat::zeros( 180, 256, CV_8UC3 );

	//�ǐՊJ�n���̃q�X�g�O�����̌v�Z
	cv::calcHist( &firstHue, 1, histChannel, firstMask, histOriginal, 1, histSize, ranges, true, false );

	//�Ώە��̂̃q�X�g�O�����v�Z
	cv::calcHist( &objectHue, 1, histChannel, objectMask, hist, 1, histSize, ranges, true, false );
	cv::calcHist( &objectHue, 1, histChannel, objectMask, histDiff, 1, histSize, ranges, true, false );

	//�Ώە��̂̎��ӂ̃q�X�g�O�����̌v�Z
	cv::calcHist( &aroundHue, 1, histChannel, aroundMask, histAround, 1, histSize, ranges, true, false );

	cv::normalize( histOriginal, histOriginal, 0, 255, cv::NORM_MINMAX, -1, cv::Mat( ) );
	cv::normalize( hist, hist, 0, 255, cv::NORM_MINMAX, -1, cv::Mat( ) );
	cv::normalize( histAround, histAround, 0, 255, cv::NORM_MINMAX, -1, cv::Mat( ) );

	firstHistThresh = histOriginal.clone( );
	objectHistThresh = hist.clone( );
	aroundHistThresh = histAround.clone( );

	//------�����q�X�g�O�����̎擾�Ƃ��ꂼ��̕`��---------------------------------------
	// �q�X�g�O������`��
	int binW = histImg.cols / hdims;
	cv::Mat buf( 1, hdims, CV_8UC3 );

	int objectBinVal, aroundBinVal, diffBinVal;
	for( int i = 0; i < hdims; i++ )
	{
		objectBinVal = cv::saturate_cast< int >( histOriginal.at<float>( i )*histImg.rows / 255 );
		cv::rectangle( histImg, cv::Point( i*binW, histImg.rows ), cv::Point( ( i + 1 )*binW, histImg.rows - objectBinVal ), cv::Scalar( buf.at<cv::Vec3b>( i ) ), -1, 8 );

		aroundBinVal = cv::saturate_cast< int >( histAround.at<float>( i )*histAroundImg.rows / 255 );
		cv::rectangle( histAroundImg, cv::Point( i*binW, histAroundImg.rows ), cv::Point( ( i + 1 )*binW, histAroundImg.rows - aroundBinVal ), cv::Scalar( buf.at<cv::Vec3b>( i ) ), -1, 8 );

		diffBinVal = cv::saturate_cast< int >( ( histOriginal.at<float>( i ) -histAround.at<float>( i ) )*histDiffImg.rows / 255 );
		histDiff.at<float>( i ) = diffBinVal;
		cv::rectangle( histDiffImg, cv::Point( i*binW, histDiffImg.rows ), cv::Point( ( i + 1 )*binW, histDiffImg.rows - diffBinVal ), cv::Scalar( buf.at<cv::Vec3b>( i ) ), -1, 8 );
	}

	//�o�b�N�v���W�F�N�V�������v�Z����
	cv::calcBackProject( &objectHue, 1, &backChannel, histDiff, backproject, &hranges, 1, true );

	cv::multiply( backproject, trackArea, backproject, 255 );

	sprintf_s( fileName, "../DB/track_img/%08lu.%s", nowFrame, "png" );
	cv::imwrite( fileName, backproject );

	/*	cv::imshow( "backproject", backproject );
	cv::waitKey( 1 );
	cv::imshow( "objectHist", histImg );
	cv::waitKey( 1 );
	cv::imshow( "aroundHist", histAroundImg );
	cv::waitKey( 1 );
	cv::imshow( "histDiff", histDiffImg );
	cv::waitKey( 100 );
	*/
}