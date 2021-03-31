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
//データ読み込み関数
//
//
//-------------------------------------------------
bool bmp::load(const char *filename)
{
	ifstream fin(filename,ios::in | ios::binary);
	if(fin.fail()) return false;

	//ファイルヘッダ読み込み
	fin.read((char *)&file.bfType,2);          //ファイルタイプ(BM)
	fin.read((char *)&file.bfSize,4);          //ファイルサイズ[byte]
	fin.read((char *)&file.bfReserved1,2);     //予約領域(0)
	fin.read((char *)&file.bfReserved2,2);     //予約領域(0)
	fin.read((char *)&file.bfOffBits,4);       //ファイル先頭から画像データまでのオフセット[byte]
	//情報ヘッダ読み込み
	fin.read((char *)&info.biSize,  4);        //情報ヘッダのサイズ[byte]
	fin.read((char *)&info.biWidth, 4);        //画像の幅[pixel]
	fin.read((char *)&info.biHeight,4);        //画像の高さ[pixel]
	fin.read((char *)&info.biPlanes,2);        //プレーン数(1)
	fin.read((char *)&info.biBitCount,2);      //１画素あたりのデータサイズ(24)
	fin.read((char *)&info.biCompression,4);   //圧縮形式
	fin.read((char *)&info.biSizeImage,4);     //画像データ部のサイズ[byte]
	fin.read((char *)&info.biXPixPerMeter,4);  //横方向解像度(１ｍあたりの画素数)
	fin.read((char *)&info.biYPixPerMeter,4);  //縦方向解像度(１ｍあたりの画素数)
	fin.read((char *)&info.biClrUsed,4);       //格納されているパレット数（使用色数）
	fin.read((char *)&info.biClrImporant,4);   //重要なパレットのインデックス

	//1ラインのデータ長は 4 byte 境界にあわせる
	line = (info.biWidth * info.biBitCount) / 8;
	if ((line % 4) != 0) {
		line = ((line / 4) + 1) * 4;
	}
	line = line / info.biBitCount * 8;	//pixelに変換

	//画像データ読み込み
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

#if 0 //逆向きの場合
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
// Author　 : Katayama Noriaki (CV-lab.)
// Update   : 2005.9.19
// function : 画像のデータをファイルに書き込む
// Argument : char ファイル名
// return   : bool変数
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

	//ファイルヘッダ書き込み
	fout.write((char *)&file.bfType,2);          //ファイルタイプ(BM)
	fout.write((char *)&file.bfSize,4);          //ファイルサイズ[byte]
	fout.write((char *)&file.bfReserved1,2);     //予約領域(0)
	fout.write((char *)&file.bfReserved2,2);     //予約領域(0)
	fout.write((char *)&file.bfOffBits,4);       //ファイル先頭から画像データまでのオフセット[byte]
	//情報ヘッダ書き込み
	fout.write((char *)&info.biSize,  4);        //情報ヘッダのサイズ[byte]
	fout.write((char *)&info.biWidth, 4);        //画像の幅[pixel]
	fout.write((char *)&info.biHeight,4);        //画像の高さ[pixel]
	fout.write((char *)&info.biPlanes,2);        //プレーン数(1)
	fout.write((char *)&info.biBitCount,2);      //１画素あたりのデータサイズ(24)
	fout.write((char *)&info.biCompression,4);   //圧縮形式
	fout.write((char *)&info.biSizeImage,4);     //画像データ部のサイズ[byte]
	fout.write((char *)&info.biXPixPerMeter,4);  //横方向解像度(１ｍあたりの画素数)
	fout.write((char *)&info.biYPixPerMeter,4);  //縦方向解像度(１ｍあたりの画素数)
	fout.write((char *)&info.biClrUsed,4);       //格納されているパレット数（使用色数）
	fout.write((char *)&info.biClrImporant,4);   //重要なパレットのインデックス

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
	//画像データ書き込み
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
//上下反転
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