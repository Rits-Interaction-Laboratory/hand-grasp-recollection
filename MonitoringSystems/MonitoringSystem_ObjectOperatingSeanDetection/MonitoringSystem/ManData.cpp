#include "stdafx.h"
#include "ManData.h"

#define _USE_MATH_DEFINES
#include <math.h>
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define pi M_PI


void CANDIDATE1::init(){
		id = 0;
		conf = 0;
}


MAN_DATA::MAN_DATA(){

	master=-1;
	body_renew_flag=0;
	face_renew_flag=0;
	dimension=20;
	//”z—ñƒƒ‚ƒŠ‚ğ“®“I‚ÉŠm•Û//
	Hist_3D = new int**[dimension];
	for(int i=0; i<dimension; i++){
		Hist_3D[i]= new int *[dimension];
		for(int j=0; j<dimension; j++){
			Hist_3D[i][j]= new int [dimension];
		}
	}

	//‰Šú‰»//
	for(int i=0; i<dimension; i++){
		for(int j=0; j<dimension; j++){
			for(int k=0; k<dimension; k++){
				Hist_3D[i][j][k]=0;
			}
		}
	}

}


MAN_DATA::~MAN_DATA()
{
	for(int i=dimension-1; i>=0; i--){
		for(int j=dimension-1; j>=0; j--){
			delete [] Hist_3D[i][j];
		}
		delete [] Hist_3D[i];
	}
	delete [] Hist_3D;
}


void MAN_DATA::init()
{

	body_renew_flag=0;
	face_renew_flag=0;
	Recog_success_flag = 0;

	////‰Šú‰»//
	//for(int i=0; i<dimension; i++){
	//	for(int j=0; j<dimension; j++){
	//		for(int k=0; k<dimension; k++){
	//			Hist_3D[i][j][k]=0;
	//		}
	//	}
	//}

}


int MAN_DATA::reset(){

	data_set_flag=0;
	init();
	

	return 0;
}


