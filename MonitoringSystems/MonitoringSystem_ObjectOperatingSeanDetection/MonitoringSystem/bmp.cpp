#include "stdafx.h"
#include "bmp.h"
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

//-------------------------------------------------
//�f�[�^�ǂݍ��݊֐�
//
//
//-------------------------------------------------
bool bmp::load(const char *filename)
{
	ifstream fin(filename,ios::in | ios::binary);
	if(fin.fail()) return false;

	//�t�@�C���w�b�_�ǂݍ���
	fin.read((char *)&file.bfType,2);          //�t�@�C���^�C�v(BM)
	fin.read((char *)&file.bfSize,4);          //�t�@�C���T�C�Y[byte]
	fin.read((char *)&file.bfReserved1,2);     //�\��̈�(0)
	fin.read((char *)&file.bfReserved2,2);     //�\��̈�(0)
	fin.read((char *)&file.bfOffBits,4);       //�t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g[byte]
	//���w�b�_�ǂݍ���
	fin.read((char *)&info.biSize,  4);        //���w�b�_�̃T�C�Y[byte]
	fin.read((char *)&info.biWidth, 4);        //�摜�̕�[pixel]
	fin.read((char *)&info.biHeight,4);        //�摜�̍���[pixel]
	fin.read((char *)&info.biPlanes,2);        //�v���[����(1)
	fin.read((char *)&info.biBitCount,2);      //�P��f������̃f�[�^�T�C�Y(24)
	fin.read((char *)&info.biCompression,4);   //���k�`��
	fin.read((char *)&info.biSizeImage,4);     //�摜�f�[�^���̃T�C�Y[byte]
	fin.read((char *)&info.biXPixPerMeter,4);  //�������𑜓x(�P��������̉�f��)
	fin.read((char *)&info.biYPixPerMeter,4);  //�c�����𑜓x(�P��������̉�f��)
	fin.read((char *)&info.biClrUsed,4);       //�i�[����Ă���p���b�g���i�g�p�F���j
	fin.read((char *)&info.biClrImporant,4);   //�d�v�ȃp���b�g�̃C���f�b�N�X

	//1���C���̃f�[�^���� 4 byte ���E�ɂ��킹��
	line = (info.biWidth * info.biBitCount) / 8;
	if ((line % 4) != 0) {
		line = ((line / 4) + 1) * 4;
	}
	line = line / info.biBitCount * 8;	//pixel�ɕϊ�

	//�摜�f�[�^�ǂݍ���
	set(info.biWidth,info.biHeight);
	image(getwidth(),getheight());
	int r,g,b;
#if 1
	for(int row=0; row<=getheight()-1; row++){
		for(int col=0; col<line; col++){
			if(col < getwidth()){
				r=get_input_R(col,row);
				g=get_input_G(col,row);
				b=get_input_B(col,row);
				fin.read((char*)&b,1);
				fin.read((char*)&g,1);
				fin.read((char*)&r,1);
			}
			else fin.read((char *)&dust,3);
		}
	} //kawa
#endif

#if 0 //�t�����̏ꍇ
	for(int j=getheight()-1; j>=0; j--){
		for(int i=0; i<line; i++){
			if(i < getwidth()){
				fin.read((char *)&B(i,j),1);
				fin.read((char *)&G(i,j),1);
				fin.read((char *)&R(i,j),1);
			}
			else{
				fin.read((char *)&dust,3);
			}
		}
	}
#endif

	return true;
}


//------------------------------------------------------------------------
// Name     : save()
// Author�@ : Katayama Noriaki (CV-lab.)
// Update   : 2005.9.19
// function : �摜�̃f�[�^���t�@�C���ɏ�������
// Argument : char �t�@�C����
// return   : bool�ϐ�
//------------------------------------------------------------------------
bool bmp::save(const char *filename)
{
	int line = getwidth();
	ofstream fout(filename, ios::out | ios::binary);
	if(fout.fail()) return false;

	// init file header
	memset(&file,0,sizeof(file));
	file.bfType    = 0x4D42;
	file.bfSize	  = 14+40+line*getheight();
	file.bfOffBits = 54;

	// init info header
	memset(&info,0,sizeof(info));
	info.biSize = 40;
	info.biWidth = getwidth();
	info.biHeight = getheight();
	info.biPlanes = 1;
	info.biBitCount = 24;
	info.biXPixPerMeter = 3780;//2834;
	info.biYPixPerMeter = 3780;//2834;

	//�t�@�C���w�b�_��������
	fout.write((char *)&file.bfType,2);          //�t�@�C���^�C�v(BM)
	fout.write((char *)&file.bfSize,4);          //�t�@�C���T�C�Y[byte]
	fout.write((char *)&file.bfReserved1,2);     //�\��̈�(0)
	fout.write((char *)&file.bfReserved2,2);     //�\��̈�(0)
	fout.write((char *)&file.bfOffBits,4);       //�t�@�C���擪����摜�f�[�^�܂ł̃I�t�Z�b�g[byte]
	//���w�b�_��������
	fout.write((char *)&info.biSize,  4);        //���w�b�_�̃T�C�Y[byte]
	fout.write((char *)&info.biWidth, 4);        //�摜�̕�[pixel]
	fout.write((char *)&info.biHeight,4);        //�摜�̍���[pixel]
	fout.write((char *)&info.biPlanes,2);        //�v���[����(1)
	fout.write((char *)&info.biBitCount,2);      //�P��f������̃f�[�^�T�C�Y(24)
	fout.write((char *)&info.biCompression,4);   //���k�`��
	fout.write((char *)&info.biSizeImage,4);     //�摜�f�[�^���̃T�C�Y[byte]
	fout.write((char *)&info.biXPixPerMeter,4);  //�������𑜓x(�P��������̉�f��)
	fout.write((char *)&info.biYPixPerMeter,4);  //�c�����𑜓x(�P��������̉�f��)
	fout.write((char *)&info.biClrUsed,4);       //�i�[����Ă���p���b�g���i�g�p�F���j
	fout.write((char *)&info.biClrImporant,4);   //�d�v�ȃp���b�g�̃C���f�b�N�X

	/*
	unsigned char r,g,b;
	for(int j=0; j<getheight()/2; j++){
	for(int i=0; i<getwidth(); i++){
	r=R(i,j); g=G(i,j); b=B(i,j);
	R(i,j)=R(i,getheight()-j-1); G(i,j)=G(i,getheight()-j-1); B(i,j)=B(i,getheight()-j-1);
	R(i,getheight()-j-1)=r; G(i,getheight()-j-1)=g; B(i,getheight()-j-1)=b;
	}
	}
*/

	//#####################################
	//�摜�f�[�^��������
	//#####################################
	//	for(int j=getheight()-1; j>=0; j--){
	//		for(int i=0; i<line; i++){
	int r,g,b;
	for(int j=0; j<=getheight()-1; j++){
		for(int i=0; i<line; i++){
			if(j < getwidth()){
				r=get_input_R(i,j);
				g=get_input_G(i,j);
				b=get_input_B(i,j);
				fout.write((char*)&b,1);
				fout.write((char*)&g,1);
				fout.write((char*)&r,1);
			}
			else fout.write((char *)&dust,3);
		}
	}//kawa

	/*
	for(int j=0; j<getheight()/2; j++){
	for(int i=0; i<getwidth(); i++){
	r=R(i,j); g=G(i,j); b=B(i,j);
	R(i,j)=R(i,getheight()-j-1); G(i,j)=G(i,getheight()-j-1); B(i,j)=B(i,getheight()-j-1);
	R(i,getheight()-j-1)=r; G(i,getheight()-j-1)=g; B(i,getheight()-j-1)=b;
	}
	}
	*/
	


	return true;
}

//-------------------------------------------------
//�㉺���]
//-------------------------------------------------
void bmp::inverse()
{
	unsigned char r,g,b;
	for(int j=0; j<getheight()/2; j++){
		for(int i=0; i<getwidth(); i++){
			r=get_input_R(i,j); g=get_input_G(i,j); b=get_input_B(i,j);
			set_input_R(i,j,get_input_R(i,getheight()-j-1)); set_input_G(i,j,get_input_G(i,getheight()-j-1)); set_input_B(i,j,get_input_B(i,getheight()-j-1));
			set_input_R(i,getheight()-j-1,r); set_input_G(i,getheight()-j-1,g); set_input_B(i,getheight()-j-1,b);
		}
	}
} //kawa