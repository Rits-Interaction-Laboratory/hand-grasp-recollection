#define _USE_MATH_DEFINES
#include <cmath>
#include "opencv2\opencv.hpp"
#include "picojson.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <direct.h>
#include <sys\stat.h>
#include <Windows.h>
#include <vector>
#include "image.h"
#include "ImageEditUtility.h"
#include <stdlib.h>

using namespace std;
using namespace cv;

template
<   
    typename TYPE,
    std::size_t SIZE
>
std::size_t arrlen(const TYPE (&)[SIZE])
{   
    return SIZE;
}


//指定したフォルダ(dir_name)からサブフォルダ名を列挙する(第2、第3引数にチェックするファイル名指定)
std::vector<std::string> get_dir_path( const std::string& dir_name, const std::string& checkfile1, const std::string& checkfile2 )
{
    HANDLE hFind;
    WIN32_FIND_DATA win32fd;//defined at Windwos.h
    vector<std::string> subDir_names;

    std::string search_name = dir_name + "\\*";
	//cout << "search & list [FOLDER] from: " << search_name.c_str() << endl;

    hFind = FindFirstFile( search_name.c_str() , &win32fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        throw runtime_error("file not found");
    }
    do {
		if( strcmp( win32fd.cFileName, "." ) && strcmp( win32fd.cFileName, ".." ) ){
			if ( win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

				//case1: 必要ファイル(checkfile1)と除外ファイル(checkfile2)共に指定されている場合
				if( !checkfile1.empty() && !checkfile2.empty() ){
					std::string checkFilePath1 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile1;
					std::string checkFilePath2 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile2;
					ifstream ifs1( checkFilePath1 );
					ifstream ifs2( checkFilePath2 );
					if( ifs1.is_open() && !ifs2.is_open() ){
						subDir_names.push_back(win32fd.cFileName);
						//printf("%s\n", subDir_names.back().c_str() );
					}
				}
				//case2: 必要ファイル(checkfile1)のみが指定されている場合
				else if( !checkfile1.empty() && checkfile2.empty() ){
					std::string checkFilePath1 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile1;
					ifstream ifs1( checkFilePath1 );
					if( ifs1.is_open() ){
						subDir_names.push_back(win32fd.cFileName);
						//printf("%s\n", subDir_names.back().c_str() );
					}
				}
				//case3: 除外ファイル(checkfile2)のみが指定されている場合
				else if( checkfile1.empty() && !checkfile2.empty() ){
					std::string checkFilePath2 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile2;
					ifstream ifs2( checkFilePath2 );
					if( !ifs2.is_open() ){
						subDir_names.push_back(win32fd.cFileName);
						//printf("%s\n", subDir_names.back().c_str() );
					}
				}
				//case4: 何も指定されていない場合
				else if( checkfile1.empty() && checkfile2.empty() ){
					subDir_names.push_back(win32fd.cFileName);
					//printf("%s\n", subDir_names.back().c_str() );
				}
			}
			else {
			}
		}
    } while (FindNextFile(hFind, &win32fd));

    FindClose(hFind);

    return subDir_names;
}

//指定したフォルダ(dir_name)から指定した拡張子(extension)のファイル名を列挙する
std::vector<std::string> get_file_path(const std::string& dir_name , const std::string& extension)
{
    HANDLE hFind;
    WIN32_FIND_DATA win32fd;//defined at Windwos.h
    vector<std::string> file_names;

    //拡張子の設定
    std::string search_name = dir_name + "\\*." + extension;
	//cout << "search & list [." << extension.c_str() << "] from: " << search_name.c_str() << endl;

    hFind = FindFirstFile( search_name.c_str() , &win32fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        throw runtime_error("file not found");
    }

    do {
        if ( win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        }
        else {
            file_names.push_back(win32fd.cFileName);
            //printf("%s\n", file_names.back().c_str() );

        }
    } while (FindNextFile(hFind, &win32fd));

    FindClose(hFind);

    return file_names;
}

//入力画像が1920x1080の場合専用、KinectのDepth(512x424)を縦横約3倍に拡大している(1536x1272)ものとして計算
std::pair<double, double> mmTransform(int pointX, int pointY, int depth)
{
	//KinectDepth画像平面中心とカメラ位置の距離(これは定数、約365[pixel] )
	//X軸(幅512pixel、視野角35°)で計算しているが
	//Y軸(高212pixel、視野角30°)で計算してもほぼ同じ値になる
	double camDistPixel = (512 / 2.) / tan(35. * M_PI / 180.);

	//与えられた点の座標値(左上基準)を画像中心基準に変換
	double WfromCenter_HR = pointX - (1920 / 2.);
	double HfromCenter_HR = pointY - (1080 / 2.);

	//与えられた点の座標値(1920x1080平面での値)を拡大前(KinectDepth画像512x424平面での値)に戻す(約2.93～3.04倍)
	double WfromCenter = WfromCenter_HR / 3.;
	double HfromCenter = HfromCenter_HR / 3.;

	//KinectDepth画像平面上での点のカメラ位置との距離(pixel)を計算(三平方の定理)
	double pointDistPixel = sqrt(pow(camDistPixel, 2.) + pow(WfromCenter, 2.) + pow(HfromCenter, 2.));

	//実測値距離とpixel距離の比率計算
	double rate_pix2mm = depth / pointDistPixel;

	//画像中心からのX・Y方向の実距離(mm)を計算
	double WfromCenter_mm = WfromCenter * rate_pix2mm;
	double HfromCenter_mm = HfromCenter * rate_pix2mm;
	//depthが取れていない場合、おかしな値になるので、-10000に補正
	if(depth == -10000)
	{
		WfromCenter_mm = -10000.;
		HfromCenter_mm = -10000.;
	}
	std::pair<double, double> cood = std::make_pair(WfromCenter_mm, HfromCenter_mm);

	return cood;
}

//入力画像が1920x1080の場合専用、KinectのDepth(512x424)を縦横約3倍に拡大している(1536x1272)ものとして計算
std::pair<double, double> pxScaleUniform(double PWfromC, double PHfromC, double DReal, double DBaseC)
{
	//KinectColor画像平面中心とカメラ位置の仮想距離(これは定数、約538[pixel] )
	//X軸(幅1536pixel、視野角70°)で計算しているが
	//Y軸(高1272pixel、視野角60°)で計算してもほぼ同じ値になる
	double tangent = tan(35. * M_PI / 180.);
	double halfwidth = (512 / 2.) * 3;
	double camDistPixel = halfwidth / tangent;

	//対象点からカメラ原点への入射角を計算
	double Wtheta = atan(PWfromC / camDistPixel);
	double Htheta = atan(PHfromC / camDistPixel);

	//depthベース平面(中心の深度が3000mm, 端に行くほど遠くなる)を想定し、対象点のベース平面上距離を計算
	double DBasePW = DBaseC / cos(Wtheta);
	double DBasePH = DBaseC / cos(Htheta);

	//実測値距離とベース平面距離の比率計算
	double ratePW = DReal * cos(Htheta) / DBasePW;
	double ratePH = DReal * cos(Wtheta) / DBasePH;

	//比率をかけてスケール均一化
	double suPW = PWfromC * ratePW;
	double suPH = PHfromC * ratePH;
	//depthが取れていない場合、おかしな値になるので、-10000に補正
	if(DReal == -10000 || DReal == 0)
	{
		suPW = -10000.;
		suPH = -10000.;
	}
	std::pair<double, double> cood = std::make_pair(suPW, suPH);

	return cood;
}

double get_median_px(std::vector<point3D>& hand3Ds)
{
	//depth値が0もしくは-10000以外の手指キーポイントをdepth値順に並べ替えて中央値を返す
	//depth有効値がひとつもなかった場合は0を返す
	int size = hand3Ds.size();
    std::vector<double> _handDepths;
	for(int p = 0; p < size; p++){
		if(hand3Ds[p].z != -10000 && hand3Ds[p].z != 0){ //Depth値がとれていないところは除外
			_handDepths.push_back(hand3Ds[p].z);
		}
	}
    //copy(hand3Ds.begin(), hand3Ds.end(), back_inserter(_hand3Ds));
	size = _handDepths.size(); //改めてサイズ計算
	if(size == 0){
		return 0;
	}
    double tmp;
    for (int i = 0; i < size - 1; i++){
        for (int j = i + 1; j < size; j++) {
            if (_handDepths[i] > _handDepths[j]){
                tmp = _handDepths[i];
                _handDepths[i] = _handDepths[j];
                _handDepths[j] = tmp;
            }
        }
    }
    if (size % 2 == 1) {
        return _handDepths[(size - 1) / 2];
    } else {
        return (_handDepths[(size / 2) - 1] + _handDepths[size / 2]) / 2;
    }
}

double search_nearest_depth_px(Mat depth, int x_c, int y_c, double limin_c, double limax_c, double orig_span)
{
	//depth制限範囲条件を満たす、近傍画素候補の平均値を返す
	//候補が2点以上見つかった時点で探索終了
	//手の半サイズより検索ボックスが大きくなれば探索終了
	//候補がまったく見つからなかった場合-10000を返す
	std::vector<double> candidate;
	int boxsize = 1;
	while(1){
		int min_y = max(y_c-boxsize, 0);
		int min_x = max(x_c-boxsize, 0);
		int max_y = min(y_c+boxsize, depth.rows-1);
		int max_x = min(x_c+boxsize, depth.cols-1);
		for(int iy = min_y; iy <= max_y; iy++){
			for(int ix = min_x; ix <= max_x; ix++){
				if(depth.at<ushort>(iy, ix) > limin_c && depth.at<ushort>(iy, ix) < limax_c){
					candidate.push_back(depth.at<ushort>(iy, ix));
				}
			}
		}
		if(candidate.size() > 1){ break; }
		boxsize += 1;
		if(boxsize > orig_span / 2){ break; }
	}
	double sum = 0.;
	if(candidate.size() == 0){
		return -10000;
	}
	for(int ic = 0; ic < candidate.size(); ic++){
		sum += candidate[ic];
	}
	return sum / candidate.size();
}

void limit_hand_depth_px(std::vector<point3D>& hand3Ds, double med, double span, double orig_span, double ObjCenter, Mat depth)
{
	//depth制限範囲(-(span/2)～med～+(span/2))条件を満たす近傍画素の値平均を取得
	//medが0の場合はCenterを制限範囲の中心とする
	//XYが分からなければ何もしない。(depthは0が入っているはず)
	//近傍見つからなければ(-10000が返れば)medで埋める
	if(med == 0){med = ObjCenter;}
	double min = max(500, med - span / 2.);	//カメラ座標での
	double max = min(8000, med + span / 2.);	//Depth制限範囲

	cout << "hand depth span(camera base): " << min << " ～ " << max << endl;
					
	for(int i = 0; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //関節iのxy取れてるなら
			if(hand3Ds[i].z < min || hand3Ds[i].z > max){ //制限範囲外の値が取れている関節点　※hand3Dsは物体位置基準座標
				hand3Ds[i].z = (int)search_nearest_depth_px(depth, hand3Ds[i].x, hand3Ds[i].y, min, max, orig_span); //引数はカメラ位置基準制限範囲
				if(hand3Ds[i].z == -10000){ hand3Ds[i].z = med; }
			}
		}
	}
}

double get_median(std::vector<Point3D>& hand3Ds)
{
	int size = hand3Ds.size();
    std::vector<double> _handDepths;
	for(int p = 0; p < size; p++){
		if(hand3Ds[p].z != -10000 && hand3Ds[p].z != 0){ //Depth値がとれていないところは除外
			_handDepths.push_back(hand3Ds[p].z);
		}
	}
    //copy(hand3Ds.begin(), hand3Ds.end(), back_inserter(_hand3Ds));
	size = _handDepths.size(); //改めてサイズ計算
	if(size == 0){
		return 0;
	}
    double tmp;
    for (int i = 0; i < size - 1; i++){
        for (int j = i + 1; j < size; j++) {
            if (_handDepths[i] > _handDepths[j]){
                tmp = _handDepths[i];
                _handDepths[i] = _handDepths[j];
                _handDepths[j] = tmp;
            }
        }
    }
    if (size % 2 == 1) {
        return _handDepths[(size - 1) / 2];
    } else {
        return (_handDepths[(size / 2) - 1] + _handDepths[size / 2]) / 2;
    }
}

double search_nearest_depth(Mat depth, int x_c, int y_c, double limin_c, double limax_c, double orig_span)
{
	std::vector<double> candidate;
	int boxsize = 1;
	while(1){
		int min_y = max(y_c-boxsize, 0);
		int min_x = max(x_c-boxsize, 0);
		int max_y = min(y_c+boxsize, depth.rows-1);
		int max_x = min(x_c+boxsize, depth.cols-1);
		for(int iy = min_y; iy <= max_y; iy++){
			for(int ix = min_x; ix <= max_x; ix++){
				if(depth.at<ushort>(iy, ix) > limin_c && depth.at<ushort>(iy, ix) < limax_c){
					candidate.push_back(depth.at<ushort>(iy, ix));
				}
			}
		}
		if(candidate.size() > 1){ break; }
		boxsize += 1;
		if(boxsize > orig_span / 2){ break; }
	}
	double sum = 0.;
	if(candidate.size() == 0){
		return -10000;
	}
	for(int ic = 0; ic < candidate.size(); ic++){
		sum += candidate[ic];
	}
	return sum / candidate.size();
}

void limit_hand_depth(std::vector<Point3D>& hand3Ds, double med, double span, double orig_span, double ObjCenter, Mat depth)
{
	double min_o = max(500 - ObjCenter, med - span / 2.);	//物体位置基準座標での
	double max_o = min(8000 - ObjCenter, med + span / 2.);	//Depth制限範囲

	double min_c = min_o + ObjCenter;						//カメラ位置基準座標での
	double max_c = max_o + ObjCenter;						//Depth制限範囲(Depth画像の値)

	cout << "hand depth span(object base): " << min_o << " ～ " << max_o << endl;
	cout << "hand depth span(camera base): " << min_c << " ～ " << max_c << endl;
					
	for(int i = 0; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //関節iのxy取れてるなら
			if(hand3Ds[i].z < min_o || hand3Ds[i].z > max_o){ //制限範囲外の値が取れている関節点　※hand3Dsは物体位置基準座標
				hand3Ds[i].z = search_nearest_depth(depth, hand3Ds[i].orig_x, hand3Ds[i].orig_y, min_c, max_c, orig_span); //引数はカメラ位置基準制限範囲
				if(hand3Ds[i].z == -10000){ hand3Ds[i].z = med; }
				else{ hand3Ds[i].z -= ObjCenter; } 
			}
		}
	}
}

bool checkFileExistence(const std::string& str)
{
    std::ifstream ifs(str);
    return ifs.is_open();
}

int get_actDepth(Mat inputDepth, int y, int x, int boxsize )
{
	int min_y = max(y-boxsize, 0);
	int min_x = max(x-boxsize, 0);
	int max_y = min(y+boxsize, inputDepth.rows-1);
	int max_x = min(x+boxsize, inputDepth.cols-1);
	int min_depth = 10000;
	for(int iy = min_y; iy <= max_y; iy++){
		for(int ix = min_x; ix <= max_x; ix++){
			int idepth = (int)inputDepth.at<ushort>( iy, ix );
			if(idepth <= 500){continue;}
			if(idepth >= 8000){continue;}
			min_depth = min(min_depth, idepth);
		}
	}
	if(min_depth == 10000){
		min_depth = -10000;
	}
	return min_depth;
}

double calcDist(Point3D p1, Point3D p2)
{
	if(p1.z == -10000 || p2.z == -10000){
		return 0;
	}
	double dist = sqrt(pow(p2.x - p1.x, 2.) + pow(p2.y - p1.y, 2.) + pow(p2.z - p1.z, 2.));
	return dist;
}
void fix_hand_dist
	(
	bool *isEs,
	double *ELs,
	int *idxt,
	int *idxc,
	std::vector<Point3D>& hand3Ds,
	Vector3D dir,
	Vector3D *EVs,
	double med
	)
{
	bool is_changed = TRUE;
	int num_cycle = 0;
	//0-1エッジを基準とした各エッジの比率
	double rate[20] = {
		1.000,1.186,1.059,0.830, //親指
		2.474,1.011,0.693,0.693, //人差し指
		2.390,1.109,0.792,0.813, //中指
		2.324,1.019,0.694,0.918, //薬指
		2.353,0.792,0.572,0.678  //小指
	};
	int p1[20] = {0,1,2,3,0,5,6,7,0,9, 10,11,0, 13,14,15,0, 17,18,19}; //20本のエッジの始点
	int p2[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; //20本のエッジの終点
	double tempZs[5];

	//エッジグループ0かどうか
	int eg;
	if(idxt[0] == 0){eg = 0;}else if(idxt[0] == 1){eg =1;}else if(idxt[0] == 2){eg = 2;}else{eg = 3;}

	//最初にターゲットのZ座標をすべて覚えておく
	for(int i = 0; i < 5; i++){
		tempZs[i] = hand3Ds[p2[idxt[i]]].z;
	}

	int f=0;
	while(is_changed || num_cycle == 0){
		is_changed = FALSE;
		for(int ft = 0; ft < 5; ft++){								//補正対象指エッジ5本分ループ
			if(isEs[idxt[ft]] == TRUE){								//↑2D○なら
				for(int fc = 0; fc < 5; fc++){						//比較対象指エッジ5本分ループ
					if(idxt[ft] == idxc[fc]){continue;}				//補正対象と比較対象が同じエッジの場合スキップ
					if(ELs[idxc[fc]] != -10000 && ELs[idxc[fc]] != 0){ //比較対象エッジの長さが計算可能なら
						//他エッジとの比率(事前調査)から外れているか
						if(
							(ELs[idxt[ft]] / ELs[idxc[fc]] > rate[idxt[ft]] / rate[idxc[fc]] + 0.1)      //理想比率よりも0.1長い
							//|| (ELs[idxt[ft]] / ELs[idxc[fc]] < rate[idxt[ft]] / rate[idxc[fc]] - 0.1) //理想比率よりも0.1短い
							)
						{ 
							//外れていた場合、Depthを調整して距離を補正
							//cout << "ターゲットの長さ：" << Ls[idx[f]] << endl;
							//cout << "比較対象の長さ：" << Ls[idxc[id]] << endl;
							//cout << "L[" << idx[f] << "] ÷ L[" << idxc[id] << "] = " << Ls[idx[f]] / Ls[idxc[id]] << " (理想比率：" << rate[idx[f]] / rate[idxc[id]] << ")" << endl;
							
							//理想の距離を算出(2乗)
							double expectDist = pow(ELs[idxc[fc]] * rate[idxt[ft]] / rate[idxc[fc]], 2.);
							//cout << "理想距離(2乗)：" << expectDist << endl;
							
							//二次元平面距離(2乗)を計算
							double dist2D = pow(hand3Ds[p2[idxt[ft]]].x - hand3Ds[p1[idxt[ft]]].x, 2.) + pow(hand3Ds[p2[idxt[ft]]].y - hand3Ds[p1[idxt[ft]]].y, 2.);
							//cout << "XY平面距離(2乗)：" << dist2D << endl;
							
							//補正対象エッジの方向ベクトル
							Vector3D vect = EVs[ft];
							//比較対象エッジの方向ベクトル
							Vector3D vecc = EVs[fc];
							//手指全体の平均方向ベクトル
							Vector3D orgDir = dir;
							
							//if(hand3Ds[p2nds[idx[f]]].z - hand3Ds[p1sts[idx[f]]].z >= 0){orgDirection = 1.;}
							//else{orgDirection = -1.;}
							
							//理想距離(2乗)と二次元平面距離(2乗)からDepth距離を算出
							if(dist2D == ELs[idxt[ft]]){
								//cout << "depth計算不可(補正済み)" << endl;
								continue;
							}
							//理想距離(2乗)より二次元平面距離(2乗)の方が大きいなら破綻しているので,
							//隣接エッジのベクトルを元にZを決定(ない場合はmedianを設定)
							if(dist2D > expectDist){
								double rateK; //並行ベクトルの係数
								//cout << "depth計算不可(XY異常)" << endl;
								if(eg == 0 || EVs[idxt[ft]-1].dx == 0 || EVs[idxt[ft]-1].dy == 0){
									hand3Ds[p2[idxt[ft]]].z = med;
								}
								else{
									if(abs(EVs[idxt[ft]].dx / EVs[idxt[ft]-1].dx) > abs(EVs[idxt[ft]].dy / EVs[idxt[ft]-1].dy)){
										rateK = EVs[idxt[ft]].dx / EVs[idxt[ft]-1].dx;
									}
									else{ rateK = EVs[idxt[ft]].dy / EVs[idxt[ft]-1].dy; }
									hand3Ds[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + EVs[idxt[ft]-1].dz * rateK;
									cout << hand3Ds[p2[idxt[ft]]].z << "(fixed)" << endl;
									//rateKが大きすぎる場合は長さ調整
									if(hand3Ds[p2[idxt[ft]]].z < med - 100 || hand3Ds[p2[idxt[ft]]].z > med + 100){
										//エッジの長さを再計算
										cout << "recalculation." << endl;
										double EL = calcDist(hand3Ds[p2[idxt[ft]]], hand3Ds[p1[idxt[ft]]]);
										cout << EL << "(EL)" << endl;
										double reduction = (100 - abs(hand3Ds[p1[idxt[ft]]].z - med)) / EL;
										cout << reduction << "(reduction)" << endl;
										hand3Ds[p2[idxt[ft]]].x = hand3Ds[p1[idxt[ft]]].x + EVs[idxt[ft]-1].dx * reduction;
										hand3Ds[p2[idxt[ft]]].y = hand3Ds[p1[idxt[ft]]].y + EVs[idxt[ft]-1].dy * reduction;
										hand3Ds[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + EVs[idxt[ft]-1].dz * reduction;
									}
								}
								//補正結果を元に長さとベクトルを再計算して反映
								ELs[idxt[ft]] = calcDist(hand3Ds[p2[idxt[ft]]], hand3Ds[p1[idxt[ft]]]);
								EVs[idxt[ft]].dx = hand3Ds[p2[idxt[ft]]].x - hand3Ds[p1[idxt[ft]]].x;
								EVs[idxt[ft]].dy = hand3Ds[p2[idxt[ft]]].y - hand3Ds[p1[idxt[ft]]].y;
								EVs[idxt[ft]].dz = hand3Ds[p2[idxt[ft]]].z - hand3Ds[p1[idxt[ft]]].z;
								continue;
							}
							//理想距離と二次元平面距離から理想奥行きを算定
							double distDepth = sqrt(expectDist - dist2D);
							
							//↑平方根のため解が正負2パターン							
							/*//2パターンそれぞれの変更後指全体長さ合計から最適方向(正負)を推定
							int fingernum = idxt[ft] / 4; //指番号を計算(親指0～小指4)
							//指1本分だけのスケルトンデータコピー作成
							Point3D finger3Ds_p[5], finger3Ds_m[5];
							finger3Ds_p[0] = hand3Ds[0]; finger3Ds_m[0] = hand3Ds[0];
							for(int j = 1; j <= 4; j++ ){
								if(hand3Ds[4 * fingernum + j].x == -10000 || hand3Ds[4 * fingernum + j].y == -10000 ||
									hand3Ds[4 * fingernum + j].x == 0 || hand3Ds[4 * fingernum + j].y == 0)
								{
									if(j != 4){
										finger3Ds_p[j].x = (hand3Ds[4 * fingernum + j+1].x + hand3Ds[4 * fingernum + j-1].x) / 2.;
										finger3Ds_p[j].y = (hand3Ds[4 * fingernum + j+1].y + hand3Ds[4 * fingernum + j-1].y) / 2.;
										finger3Ds_p[j].z = med;
										finger3Ds_m[j].x = (hand3Ds[4 * fingernum + j+1].x + hand3Ds[4 * fingernum + j-1].x) / 2.;
										finger3Ds_m[j].y = (hand3Ds[4 * fingernum + j+1].y + hand3Ds[4 * fingernum + j-1].y) / 2.;
										finger3Ds_m[j].z = med;
									}else{
										finger3Ds_p[j].x = hand3Ds[4 * fingernum + j-1].x + (hand3Ds[4 * fingernum + j-1].x - hand3Ds[4 * fingernum + j-2].x)
									}
								}
								finger3Ds_p[j] = hand3Ds[4 * fingernum + j];
								finger3Ds_m[j] = hand3Ds[4 * fingernum + j];
							}
							//計算した理想奥行きを正負2パターンで足し込み
							finger3Ds_p[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + distDepth;
							finger3Ds_m[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z - distDepth;
							double sumLen_p, sumLen_m;*/
							//エッジグループの平均方向ベクトルを参照
							int ecnt = 0;
							double avedz = 0.;
							double sign;
							for(int e = 0; e < 5; e++){
								if(EVs[idxt[e]].dz != 0.){
									ecnt ++;
									avedz += EVs[idxt[e]].dz;
								}
							}
							if(ecnt > 0){avedz /= ecnt;}
							if(avedz > 0){sign = 1.;}else if(avedz < 0){sign = -1.;}
							else{
								if(dir.dz > 0){sign = 1.;}else if(dir.dz < 0){sign = -1.;}
								else{sign = 0;}
							}
							//cout << "理想奥行き距離：" << distDepth << endl;
							//計算したDepth距離を元にzを補正
							cout << "補正前depth：" << hand3Ds[p2[idxt[ft]]].z << endl;
							if(sign == 0){hand3Ds[p2[idxt[ft]]].z = med;}
							else{hand3Ds[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + distDepth * sign;}
							cout << "補正後depth：" << hand3Ds[p2[idxt[ft]]].z << endl;
							//補正結果を元に長さとベクトルを再計算して反映
							ELs[idxt[ft]] = calcDist(hand3Ds[p2[idxt[ft]]], hand3Ds[p1[idxt[ft]]]);
							EVs[idxt[ft]].dx = hand3Ds[p2[idxt[ft]]].x - hand3Ds[p1[idxt[ft]]].x;
							EVs[idxt[ft]].dy = hand3Ds[p2[idxt[ft]]].y - hand3Ds[p1[idxt[ft]]].y;
							EVs[idxt[ft]].dz = hand3Ds[p2[idxt[ft]]].z - hand3Ds[p1[idxt[ft]]].z;
							//cout << "再計算後のターゲットの長さ：" << ELs[idxt[ft]] << endl;
						}
					}
				}
			}
		}
		num_cycle++;
		for(int i = 0; i < 5; i++){
			if(tempZs[i] != hand3Ds[p2[idxt[i]]].z){
				//前回と変わってる部分があれば継続
				is_changed = TRUE;
			}
			tempZs[i] = hand3Ds[p2[idxt[i]]].z;
		}
		if(num_cycle >= 5){is_changed = FALSE;}
		//cout << is_changed << endl;
		cout << num_cycle << "(cycle)" << endl;
	}
}
/*void fixDist(bool *isLs, double *Ls, int *idx, int *idxc, std::vector<Point3D>& hand3Ds, double dir){
	bool is_changed = TRUE;
	int num_cycle = 0;
	double rate[20] = {1,1.186,1.059,0.830,2.474,1.011,0.693,0.693,2.390,1.109,0.792,0.813,2.324,1.019,0.694,0.918,2.353,0.792,0.572,0.678};
	int p1sts[20] = {0,1,2,3,0,5,6,7,0,9,10,11,0,13,14,15,0,17,18,19};
	int p2nds[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
	double tempZs[5];

	//最初にターゲットのZ座標をすべて覚えておく
	for(int i = 0; i < 5; i++){
		tempZs[i] = hand3Ds[p2nds[idx[i]]].z;
	}

	int f=0;
	while(is_changed || num_cycle == 0){
		is_changed = FALSE;
		for(int f = 0; f < 5; f++){
			if(isLs[idx[f]] == TRUE){ //2D○なら
				for(int id = 0; id < 5; id++){
					if(idx[f] == idxc[id]){continue;}
					if(Ls[idxc[id]] != -10000 && Ls[idxc[id]] != 0){
						//他エッジとの比率(事前調査)から外れているか
						if(Ls[idx[f]] / Ls[idxc[id]] > rate[idx[f]] / rate[idxc[id]] + 0.1){ //Ls[idx[f]] / Ls[idxc[id]] < rate[idx[f]] / rate[idxc[id]] - 0.1 || 
							//外れていた場合、Depthを調整して距離を補正
							//cout << "ターゲットの長さ：" << Ls[idx[f]] << endl;
							//cout << "比較対象の長さ：" << Ls[idxc[id]] << endl;
							//cout << "L[" << idx[f] << "] ÷ L[" << idxc[id] << "] = " << Ls[idx[f]] / Ls[idxc[id]] << " (理想比率：" << rate[idx[f]] / rate[idxc[id]] << ")" << endl;
							//理想の距離を算出(2乗)
							double expectDist = pow(Ls[idxc[id]] * rate[idx[f]] / rate[idxc[id]], 2.);
							//cout << "理想距離(2乗)：" << expectDist << endl;
							//二次元平面距離(2乗)を計算
							double dist2D = pow(hand3Ds[p2nds[idx[f]]].x - hand3Ds[p1sts[idx[f]]].x, 2.) + pow(hand3Ds[p2nds[idx[f]]].y - hand3Ds[p1sts[idx[f]]].y, 2.);
							//cout << "XY平面距離(2乗)：" << dist2D << endl;
							//元々の2点間のDepthの方向(正負)を取得(点番号の若い方からの方向、奥＋前-)
							double orgDirection = dir;
							//if(hand3Ds[p2nds[idx[f]]].z - hand3Ds[p1sts[idx[f]]].z >= 0){orgDirection = 1.;}
							//else{orgDirection = -1.;}
							//理想距離(2乗)と二次元平面距離(2乗)からDepth距離を算出
							//理想距離(2乗)より二次元平面距離(2乗)の方が大きいなら破綻しているので,Zを同じ値にして諦め
							if(dist2D == Ls[idx[f]]){
								//cout << "depth計算不可(補正済み)" << endl;
								continue;
							}
							if(dist2D > expectDist){
								//cout << "depth計算不可" << endl;
								hand3Ds[p2nds[idx[f]]].z = hand3Ds[p1sts[idx[f]]].z;
								Ls[idx[f]] = calcDist(hand3Ds[p2nds[idx[f]]], hand3Ds[p1sts[idx[f]]]);
								continue;
							}
							double distDepth = sqrt(expectDist - dist2D) * orgDirection;
							//cout << "理想奥行き距離：" << distDepth << endl;
							//計算したDepth距離を元にzを補正
							cout << "補正前depth：" << hand3Ds[p2nds[idx[f]]].z << endl;
							hand3Ds[p2nds[idx[f]]].z = hand3Ds[p1sts[idx[f]]].z + distDepth;
							cout << "補正後depth：" << hand3Ds[p2nds[idx[f]]].z << endl;
							//2点間の距離を再計算
							Ls[idx[f]] = calcDist(hand3Ds[p2nds[idx[f]]], hand3Ds[p1sts[idx[f]]]);
							//cout << "再計算後のターゲットの長さ：" << Ls[idx[f]] << endl;
						}
					}
				}
			}
		}
		num_cycle++;
		for(int i = 0; i < 5; i++){
			if(tempZs[i] != hand3Ds[p2nds[idx[i]]].z){
				//前回と変わってる部分があれば継続
				is_changed = TRUE;
			}
			tempZs[i] = hand3Ds[p2nds[idx[i]]].z;
		}
		if(num_cycle >= 5){is_changed = FALSE;}
		cout << is_changed << endl;
		cout << num_cycle << "(cycle)" << endl;
	}
}*/
void fix_hand_depth(std::vector<Point3D>& hand3Ds, double hmed_d)
{
	//各関節点見つかっているか
	bool isPs[21];						
	//各エッジ(XY)存在するか
	bool isEs[20];	
	//各エッジ長さ(XYZ)
	double ELs[20];
	//各エッジベクトル(XYZ)
	Vector3D EVs[20];
	int p1sts[20] = {0,1,2,3,0,5,6,7,0,9, 10,11,0, 13,14,15,0, 17,18,19}; //20本のエッジの始点
	int p2nds[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; //20本のエッジの終点
	//エッジindex
	//[  0:0-1 ,  1:1-2  ,  2:2-3  ,  3:3-4  , //親指
	//   4:0-5 ,  5:5-6  ,  6:6-7  ,  7:7-8  , //人差し指
	//   8:0-9 ,  9:9-10 , 10:10-11, 11:11-12, //中指
	//  12:0-13, 13:13-14, 14:14-15, 15:15-16, //薬指
	//  16:0-17, 17:17-18, 18:18-19, 19:19-20 ]//小指

	//std::vector<pair<int, int>> Lgroup1, Lgroup2, Lgroup3, Lgroup4, Lgroup5, Lgroup6, Lgroup7;
	//Lgroup1.push_back(make_pair(0,1)); Lgroup1.push_back(make_pair(2,3)); Lgroup1.push_back(make_pair(5,6)); Lgroup1.push_back(make_pair(13,14));
	//Lgroup2.push_back(make_pair(1,2)); Lgroup2.push_back(make_pair(9,10));
	//Lgroup3.push_back(make_pair(3,4)); Lgroup3.push_back(make_pair(10,11)); Lgroup3.push_back(make_pair(11,12)); Lgroup3.push_back(make_pair(17,18));
	//Lgroup4.push_back(make_pair(0,5)); Lgroup4.push_back(make_pair(0,9)); Lgroup4.push_back(make_pair(0,13)); Lgroup4.push_back(make_pair(0,17));
	//Lgroup5.push_back(make_pair(6,7)); Lgroup5.push_back(make_pair(7,8)); Lgroup5.push_back(make_pair(14,15)); Lgroup5.push_back(make_pair(19,20));
	//Lgroup6.push_back(make_pair(15,16));
	//Lgroup7.push_back(make_pair(18,19));

	//まずP0が-10000でないかチェック
	if(hand3Ds[0].x == -10000 && hand3Ds[0].y == -10000){return;} //原点ないなら諦め
	//すべての点について存在するかどうかチェック
	for(int i = 0; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //関節iのxy取れてるなら
			isPs[i] = TRUE;//関節i XY○
		}
		else{isPs[i] = FALSE;}//関節i XY×
	}
	//すべてのエッジについて存在するかどうかチェック&距離計算&ベクトル計算(depth取れていない場合は距離0)
	for(int e = 0; e < arrlen(EVs); e++){
		if(isPs[p1sts[e]] == TRUE && isPs[p2nds[e]] == TRUE){
			isEs[e] = TRUE;
			ELs[e] = calcDist(hand3Ds[p1sts[e]], hand3Ds[p2nds[e]]);
			EVs[e].dx = hand3Ds[p2nds[e]].x - hand3Ds[p1sts[e]].x;
			EVs[e].dy = hand3Ds[p2nds[e]].y - hand3Ds[p1sts[e]].y;
			EVs[e].dz = hand3Ds[p2nds[e]].z - hand3Ds[p1sts[e]].z;
		}
		else{
			isEs[e] = FALSE;
			ELs[e] = 0.;
			EVs[e].dx = 0.; EVs[e].dy = 0.; EVs[e].dz = 0.;
		}
	}
	//手指全体の向きベクトルを各エッジベクトルの相加平均で推定
	Vector3D direction = {0., 0., 0.};
	int ecnt = 0;
	for(int di = 0; di < arrlen(EVs); di++){
		if(isEs[di] == TRUE){
			direction.dx += EVs[di].dx;
			direction.dy += EVs[di].dy;
			direction.dz += EVs[di].dz;
			ecnt ++;
		}
	}
	if(ecnt > 0){
		direction.dx /= ecnt;
		direction.dy /= ecnt;
		direction.dz /= ecnt;
	}
	cout << "手指全体平均ベクトル\nx：" << direction.dx << "\ny：" << direction.dy << "\nz：" << direction.dz << endl;

	//ここから各点およびエッジを補正
	//まずは原点からの基準5本
	int idx[5] = {0,4,8,12,16};
	int idx2[5] = {1,5,9,13,17};
	int idx3[5] = {2,6,10,14,18};
	int idx4[5] = {3,7,11,15,19};

	//fix_hand_dist(isLs, Ls, idx, idx, hand3Ds, direction, EVs, hmed_d);
	//fix_hand_dist(isLs, Ls, idx2, idx, hand3Ds, direction, EVs, hmed_d);
	fix_hand_dist(isEs, ELs, idx3, idx, hand3Ds, direction, EVs, hmed_d);
	fix_hand_dist(isEs, ELs, idx4, idx, hand3Ds, direction, EVs, hmed_d);
}
/*void fix_depth(std::vector<Point3D>& hand3Ds)
{
	bool isPs[21];
	bool is0_1,is1_2,is2_3,is3_4;       //親指
	bool is0_5,is5_6,is6_7,is7_8;       //人差し指
	bool is0_9,is9_10,is10_11,is11_12;  //中指
	bool is0_13,is13_14,is14_15,is15_16;//薬指
	bool is0_17,is17_18,is18_19,is19_20;//小指
	bool isLs[20];
	double L0_1=0,L1_2=0,L2_3=0,L3_4=0;
	double L0_5=0,L5_6=0,L6_7=0,L7_8=0;
	double L0_9=0,L9_10=0,L10_11=0,L11_12=0;
	double L0_13=0,L13_14=0,L14_15=0,L15_16=0;
	double L0_17=0,L17_18=0,L18_19=0,L19_20=0;
	double Ls[20];

	//std::vector<pair<int, int>> Lgroup1, Lgroup2, Lgroup3, Lgroup4, Lgroup5, Lgroup6, Lgroup7;
	//Lgroup1.push_back(make_pair(0,1)); Lgroup1.push_back(make_pair(2,3)); Lgroup1.push_back(make_pair(5,6)); Lgroup1.push_back(make_pair(13,14));
	//Lgroup2.push_back(make_pair(1,2)); Lgroup2.push_back(make_pair(9,10));
	//Lgroup3.push_back(make_pair(3,4)); Lgroup3.push_back(make_pair(10,11)); Lgroup3.push_back(make_pair(11,12)); Lgroup3.push_back(make_pair(17,18));
	//Lgroup4.push_back(make_pair(0,5)); Lgroup4.push_back(make_pair(0,9)); Lgroup4.push_back(make_pair(0,13)); Lgroup4.push_back(make_pair(0,17));
	//Lgroup5.push_back(make_pair(6,7)); Lgroup5.push_back(make_pair(7,8)); Lgroup5.push_back(make_pair(14,15)); Lgroup5.push_back(make_pair(19,20));
	//Lgroup6.push_back(make_pair(15,16));
	//Lgroup7.push_back(make_pair(18,19));
	//まず0が-10000でないかチェック
	if(hand3Ds[0].x == -10000 && hand3Ds[0].y == -10000){return;} //原点ないなら諦め
	//すべての点について存在するかどうかチェック
	for(int i = 1; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //関節iのxy取れてるなら
			isPs[i] = TRUE;//関節i XY○
		}
		else{isPs[i] = FALSE;}//関節i XY×
	}
	//すべてのエッジについて存在するかどうかチェック&距離計算(depth取れていない場合は距離0)
	if(isPs[1] == TRUE){is0_1 = TRUE; L0_1 = calcDist(hand3Ds[0], hand3Ds[1]);}else{is0_1 = FALSE;}
	if(isPs[5] == TRUE){is0_5 = TRUE; L0_5 = calcDist(hand3Ds[0], hand3Ds[5]);}else{is0_5 = FALSE;}
	if(isPs[9] == TRUE){is0_9 = TRUE; L0_9 = calcDist(hand3Ds[0], hand3Ds[9]);}else{is0_9 = FALSE;}
	if(isPs[13] == TRUE){is0_13 = TRUE; L0_13 = calcDist(hand3Ds[0], hand3Ds[13]);}else{is0_13 = FALSE;}
	if(isPs[17] == TRUE){is0_17 = TRUE; L0_17 = calcDist(hand3Ds[0], hand3Ds[17]);}else{is0_17 = FALSE;}
	if(isPs[1] == TRUE && isPs[2] == TRUE){is1_2 = TRUE; L1_2 = calcDist(hand3Ds[1], hand3Ds[2]);}else{is1_2 = FALSE;}
	if(isPs[2] == TRUE && isPs[3] == TRUE){is2_3 = TRUE; L2_3 = calcDist(hand3Ds[2], hand3Ds[3]);}else{is2_3 = FALSE;}
	if(isPs[3] == TRUE && isPs[4] == TRUE){is3_4 = TRUE; L3_4 = calcDist(hand3Ds[3], hand3Ds[4]);}else{is3_4 = FALSE;}
	if(isPs[5] == TRUE && isPs[6] == TRUE){is5_6 = TRUE; L5_6 = calcDist(hand3Ds[5], hand3Ds[6]);}else{is5_6 = FALSE;}
	if(isPs[6] == TRUE && isPs[7] == TRUE){is6_7 = TRUE; L6_7 = calcDist(hand3Ds[6], hand3Ds[7]);}else{is6_7 = FALSE;}
	if(isPs[7] == TRUE && isPs[8] == TRUE){is7_8 = TRUE; L7_8 = calcDist(hand3Ds[7], hand3Ds[8]);}else{is7_8 = FALSE;}
	if(isPs[9] == TRUE && isPs[10] == TRUE){is9_10 = TRUE; L9_10 = calcDist(hand3Ds[9], hand3Ds[10]);}else{is9_10 = FALSE;}
	if(isPs[10] == TRUE && isPs[11] == TRUE){is10_11 = TRUE; L10_11 = calcDist(hand3Ds[10], hand3Ds[11]);}else{is10_11 = FALSE;}
	if(isPs[11] == TRUE && isPs[12] == TRUE){is11_12 = TRUE; L11_12 = calcDist(hand3Ds[11], hand3Ds[12]);}else{is11_12 = FALSE;}
	if(isPs[13] == TRUE && isPs[14] == TRUE){is13_14 = TRUE; L13_14 = calcDist(hand3Ds[13], hand3Ds[14]);}else{is13_14 = FALSE;}
	if(isPs[14] == TRUE && isPs[15] == TRUE){is14_15 = TRUE; L14_15 = calcDist(hand3Ds[14], hand3Ds[15]);}else{is14_15 = FALSE;}
	if(isPs[15] == TRUE && isPs[16] == TRUE){is15_16 = TRUE; L15_16 = calcDist(hand3Ds[15], hand3Ds[16]);}else{is15_16 = FALSE;}
	if(isPs[17] == TRUE && isPs[18] == TRUE){is17_18 = TRUE; L17_18 = calcDist(hand3Ds[17], hand3Ds[18]);}else{is17_18 = FALSE;}
	if(isPs[18] == TRUE && isPs[19] == TRUE){is18_19 = TRUE; L18_19 = calcDist(hand3Ds[18], hand3Ds[19]);}else{is18_19 = FALSE;}
	if(isPs[19] == TRUE && isPs[20] == TRUE){is19_20 = TRUE; L19_20 = calcDist(hand3Ds[19], hand3Ds[20]);}else{is19_20 = FALSE;}

	isLs[0] = is0_1; isLs[1] = is1_2; isLs[2] = is2_3; isLs[3] = is3_4;           //親指
	isLs[4] = is0_5; isLs[5] = is5_6; isLs[6] = is6_7; isLs[7] = is7_8;           //人差し指
	isLs[8] = is0_9; isLs[9] = is9_10; isLs[10] = is10_11; isLs[11] = is11_12;    //中指
	isLs[12] = is0_13; isLs[13] = is13_14; isLs[14] = is14_15; isLs[15] = is15_16;//薬指
	isLs[16] = is0_17; isLs[17] = is17_18; isLs[18] = is18_19; isLs[19] = is19_20;//小指
	Ls[0] = L0_1; Ls[1] = L1_2; Ls[2] = L2_3; Ls[3] = L3_4;           //親指
	Ls[4] = L0_5; Ls[5] = L5_6; Ls[6] = L6_7; Ls[7] = L7_8;           //人差し指
	Ls[8] = L0_9; Ls[9] = L9_10; Ls[10] = L10_11; Ls[11] = L11_12;    //中指
	Ls[12] = L0_13; Ls[13] = L13_14; Ls[14] = L14_15; Ls[15] = L15_16;//薬指
	Ls[16] = L0_17; Ls[17] = L17_18; Ls[18] = L18_19; Ls[19] = L19_20;//小指

	//最初の時点での指全体の向きを多数決で推定
	double dsum = 0;
	double direction = 0.;
	for(int di = 0; di < 20; di++){
		double tdir = 0;
		if(hand3Ds[di+1].z - hand3Ds[di].z > 0){tdir = 1.;}
		else{tdir = -1.;}
		dsum += tdir;
	}
	if(dsum >= 0){cout << "手の奥行き方向(+)" << endl; direction = 1.;}
	else{cout << "手の奥行き方向(-)" << endl; direction = -1.;}

	//ここから各点およびエッジを補正
	//まずは原点からの基準5本
	int idx[5] = {0,4,8,12,16};
	int idx2[5] = {1,5,9,13,17};
	int idx3[5] = {2,6,10,14,18};
	int idx4[5] = {3,7,11,15,19};

	//fixDist(isLs, Ls, idx, idx, hand3Ds, direction);
	//fixDist(isLs, Ls, idx2, idx, hand3Ds, direction);
	fixDist(isLs, Ls, idx3, idx, hand3Ds, direction);
	fixDist(isLs, Ls, idx4, idx, hand3Ds, direction);
}*/

int main()
{
	//フォルダ名の読み込み

	//ベースフォルダの読み込み(テキストファイル指定)
	ifstream ifs("FolderPath.txt");
	string base_dir;

	if(ifs.fail())
	{
		cerr << "read file error !" << endl;
		return -1;
	}
	getline(ifs, base_dir);

	//ここからメイン処理

	std::string pose2D = "pose2D";
	std::string Lhand2D = "Lhand2D";
	std::string Rhand2D = "Rhand2D";

	while(1)
	{
		//日付_時刻フォルダの探索・リストアップ
		std::vector<std::string> date_time_dirNames;
		std::string needFile;
		std::string notNeedFile = "HRchecked.txt";

		date_time_dirNames = get_dir_path(base_dir, needFile, notNeedFile);

		//日付_時刻フォルダ内の処理
		for( int i = 0; i < date_time_dirNames.size(); i++ ){

			//各日付_時刻フォルダ内、HRフォルダの読み込み
			std::string dt = date_time_dirNames[i];
			std::string date_time = base_dir + "\\" + date_time_dirNames[i];
			std::string date_time_dir = base_dir + "\\" + date_time_dirNames[i] + "\\HR";

			//HRフォルダ内の処理
			//各フォルダパス生成
			std::string HRcolor_dir = date_time_dir + "\\color";
			std::string HRdepth_dir = date_time_dir + "\\depth";
			std::string HRbodyindex_dir = date_time_dir + "\\bodyindex";

			//OpenPoseによる骨格推定

			//骨格推定結果保存用フォルダ作成
			//openposeフォルダ
			std::string saveOpenPose_dir = date_time_dir + "\\openpose";
			struct stat statBuf;
			if(stat(saveOpenPose_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveOpenPose_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveOpenPose_dir.c_str() << endl;
					return -1;
				}
			}
			//imgフォルダ
			std::string saveOpenPoseImg_dir = saveOpenPose_dir + "\\img";
			if(stat(saveOpenPoseImg_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveOpenPoseImg_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveOpenPoseImg_dir.c_str() << endl;
					return -1;
				}
			}
			//skeletonフォルダ
			std::string saveOpenPoseSkeleton_dir = saveOpenPose_dir + "\\skeleton";
			if(stat(saveOpenPoseSkeleton_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveOpenPoseSkeleton_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveOpenPoseSkeleton_dir.c_str() << endl;
					return -1;
				}
			}
			//hand_pointsフォルダ
			std::string saveHandPoints_dir = date_time_dir + "\\hand_points";
			if(stat(saveHandPoints_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveHandPoints_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveHandPoints_dir.c_str() << endl;
					return -1;
				}
			}
			//hand_points\\pxmmフォルダ
			std::string saveHandPoints_pxmm_dir = saveHandPoints_dir + "\\pxmm";
			if(stat(saveHandPoints_pxmm_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveHandPoints_pxmm_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveHandPoints_pxmm_dir.c_str() << endl;
					return -1;
				}
			}
			//hand_points\\mmフォルダ
			std::string saveHandPoints_mm_dir = saveHandPoints_dir + "\\mm";
			if(stat(saveHandPoints_mm_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveHandPoints_mm_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveHandPoints_mm_dir.c_str() << endl;
					return -1;
				}
			}
			//hand_points\\dataset2dフォルダ
			std::string saveHandPoints_mm_dir = saveHandPoints_dir + "\\dataset2d";
			if(stat(saveHandPoints_mm_dir.c_str(), &statBuf) != 0)
			{
				if(_mkdir(saveHandPoints_mm_dir.c_str()) != 0)
				{
					cout << "cannot create directory: " << saveHandPoints_mm_dir.c_str() << endl;
					return -1;
				}
			}
			//OpenPose実行コマンド生成
			std::string command_str = "cd C:\\openpose\\openpose-1.4.0-win64-gpu-binaries && bin\\OpenPoseDemo.exe --image_dir " + HRcolor_dir + " --hand --write_images " 
					+ saveOpenPoseImg_dir + " --write_json " + saveOpenPoseSkeleton_dir;
			std::string openpose_checkfile = saveOpenPose_dir + "\\done.txt";
			if(!checkFileExistence(openpose_checkfile))
			{
				//コマンド実行
				system(command_str.c_str());

				FILE *fp;
				fp = fopen( openpose_checkfile.c_str(), "w" );
				fclose( fp );
			}

			//colorフォルダ内のPNGファイルリストアップ
			vector<std::string> colorFileNames = get_file_path( HRcolor_dir, "png" );

			//depthフォルダ内のPNGファイルリストアップ
			vector<std::string> depthFileNames = get_file_path( HRdepth_dir, "png" );

			//bodyindexフォルダ内の
			vector<std::string> bodyindexFileNames = get_file_path( HRbodyindex_dir, "png" );

			//JSONテキストファイルのリストアップ
			vector<std::string> skeletonFileNames = get_file_path( saveOpenPoseSkeleton_dir, "json" );

			//画像とJSONテキストの数のチェック
			if( depthFileNames.size() != skeletonFileNames.size() ){
				cout << "ERROR：.PNGファイルと.TXTファイルの数が異なります" << endl;
				return -1;
			}

			std::string saveR_file = date_time_dir + "\\R.txt";
			std::string saveL_file = date_time_dir + "\\L.txt";
			bool saveL = FALSE;
			bool saveR = FALSE;
			if(checkFileExistence(saveR_file)){
				//Rを保存フラグON
				saveR = TRUE;
			}
			if(checkFileExistence(saveL_file)){
				//Rを保存フラグON
				saveL = TRUE;
			}

			//画像1枚ごとの処理
			for(int p = 0; p < colorFileNames.size(); p++)
			{
				cout << colorFileNames[p] << endl;
				//depth画像ファイルリストから1枚読み込み
				std::string inputDepth_file = HRdepth_dir + "\\" + depthFileNames[p];
				//cout << "open PNG depth file: " << inputDepthPath.c_str() << endl;
				Mat inputDepth = imread( inputDepth_file, -1 );
				if( inputDepth.data == NULL )
				{
					cout << "read depth image error !" << endl;
					return -1;
				}else{ /*cout << "success reading depth."<< endl;*/ }

				//16bit深度画像から8bit深度画像を生成
				Mat inputDepth8U;
				inputDepth.convertTo( inputDepth8U, CV_8U, 255. / 2990., -1625. * 255. / 2990. );
				if( inputDepth8U.data == NULL )
				{
					cout << "generate depth(CV_8U) image error !" << endl;
					return -1;
				}else{ /*cout << "success generating depth(CV_8U) image." << endl;*/ }

				//JSONテキストファイルリストから1データ分読み込み
				std::string inputSkeleton_file = saveOpenPoseSkeleton_dir + "\\" + skeletonFileNames[p];
				//cout << "open JSON_TXT file: " << inputSkeletonPath.c_str() << endl;
				ifstream ifs4( inputSkeleton_file.c_str() );
				if ( ifs4.fail() )
				{
					cerr << "read JSON_TXT file error !" << endl;
					return -1;
				}
				istreambuf_iterator<char> it( ifs4 );
				istreambuf_iterator<char> last;
				string BodyInfo(it, last);

				image input(inputDepth.cols, inputDepth.rows);

				//inputに保存先フォルダパス登録
				input.date_time_dir = date_time_dir;
				input.HRcolor_dir = HRcolor_dir;
				input.HRdepth_dir = HRdepth_dir;
				input.HRbodyindex_dir = HRbodyindex_dir;
				input.saveOpenPose_dir =  saveOpenPose_dir;
				input.saveOpenPoseImg_dir = saveOpenPoseImg_dir;
				input.saveOpenPoseSkeleton_dir = saveOpenPoseSkeleton_dir;
				input.saveHandPoints_dir = saveHandPoints_dir;
				input.saveHandPoints_pxmm_dir = saveHandPoints_pxmm_dir;
				input.saveHandPoints_mm_dir = saveHandPoints_mm_dir;

				//inputに画像登録
				input.img_original_depth = inputDepth.clone();
				input.img_original_depth8U = inputDepth8U.clone();
				
				//Skeletonの解析
				ImageEditUtil::drawOpenPoseSkeleton(input, BodyInfo);

				//保存ファイル名の生成
				std::string saveFileName( colorFileNames[p] );
				std::string saveFrame = saveFileName.substr( 0, 8 );
				//saveFileName = saveFrame + "_openpose";
				input.saveFrame = saveFrame;

				//手指と全身の各関節点を手首位置基準座標系に変換してテキスト保存
				//XYについてはpixel座標系とmm座標系2種類作成
				//フレームごとに全関節点を番号順に並べて出力

				std::vector<point2D> pose2Ds = input.get_skeletonType2D( /*touched_personIDs[p],*/0, pose2D );
				std::vector<point2D> Lhand2Ds = input.get_skeletonType2D( /*touched_personIDs[p],*/0, Lhand2D );
				std::vector<point2D> Rhand2Ds = input.get_skeletonType2D( /*touched_personIDs[p],*/0, Rhand2D );
				std::vector<point3D> Lhand3Ds(Lhand2Ds.size());
				std::vector<point3D> Rhand3Ds(Rhand2Ds.size());
				std::vector<point3D> pose3Ds(pose2Ds.size());

				std::ofstream Points_Lhand_pxmmToCSV;
				std::ofstream Points_Rhand_pxmmToCSV;
				std::ofstream Points_Lhand_mmToCSV;
				std::ofstream Points_Rhand_mmToCSV;
				
				//pix, pix, mm(x, y, z)
				if(saveL){Points_Lhand_pxmmToCSV.open( ( input.saveHandPoints_pxmm_dir + "\\LHand_pxmm_" + dt + "_" + input.saveFrame + ".csv" ).c_str(), ios::out );}
				if(saveR){Points_Rhand_pxmmToCSV.open( ( input.saveHandPoints_pxmm_dir + "\\RHand_pxmm_" + dt + "_" + input.saveFrame + ".csv" ).c_str(), ios::out );}
				
				//mm, mm, mm(x, y, z)
				if(saveL){Points_Lhand_mmToCSV.open( ( input.saveHandPoints_mm_dir + "\\LHand_" + dt + "_" + input.saveFrame + ".csv" ).c_str(), ios::out );}
				if(saveR){Points_Rhand_mmToCSV.open( ( input.saveHandPoints_mm_dir + "\\RHand_" + dt + "_" + input.saveFrame + ".csv" ).c_str(), ios::out );}


				//全関節点群XYにdepth情報を追加した3Dデータを作成(手関節のみも作成)
				/*for( int p3d = 0; p3d < pose3Ds.size(); p3d++ ){
					if( pose2Ds[p3d].accuracy != 0 ){
						pose3Ds[p3d].x = pose2Ds[p3d].x;
						pose3Ds[p3d].y = pose2Ds[p3d].y;
					}else{
						pose3Ds[p3d].x = -10000;
						pose3Ds[p3d].y = -10000;
					}
					pose3Ds[p3d].accuracy = pose2Ds[p3d].accuracy;
					pose3Ds[p3d].z = (int)inputDepth.at<ushort>( pose2Ds[p3d].y, pose2Ds[p3d].x );
					if( pose3Ds[p3d].z < 500 || pose3Ds[p3d].z >= 8000 ){
						pose3Ds[p3d].z = -10000;
					}
				}*/

				double hand_median_px;
				//左手関節点群XYにdepth情報を追加した3Dデータを作成
				double min_LhandX_px = NULL;
				double min_LhandY_px = NULL;
				double max_LhandX_px = NULL;
				double max_LhandY_px = NULL;
				double Lhand_spanX_px, Lhand_spanY_px, Lhand_span_px = 0;
				for( int lh3d = 0; lh3d < Lhand3Ds.size(); lh3d++ ){
					if( Lhand2Ds[lh3d].accuracy != 0 ){
						Lhand3Ds[lh3d].x = Lhand2Ds[lh3d].x;
						Lhand3Ds[lh3d].y = Lhand2Ds[lh3d].y;
						if(min_LhandX_px == NULL && max_LhandX_px == NULL){min_LhandX_px = Lhand3Ds[lh3d].x; max_LhandX_px = Lhand3Ds[lh3d].x;}
						if(min_LhandY_px == NULL && max_LhandY_px == NULL){min_LhandY_px = Lhand3Ds[lh3d].y; max_LhandY_px = Lhand3Ds[lh3d].y;}
						if(min_LhandX_px > Lhand3Ds[lh3d].x){min_LhandX_px = Lhand3Ds[lh3d].x;}
						if(max_LhandX_px < Lhand3Ds[lh3d].x){max_LhandX_px = Lhand3Ds[lh3d].x;}
						if(min_LhandY_px > Lhand3Ds[lh3d].y){min_LhandY_px = Lhand3Ds[lh3d].y;}
						if(max_LhandY_px < Lhand3Ds[lh3d].y){max_LhandY_px = Lhand3Ds[lh3d].y;}
					}else{
						Lhand3Ds[lh3d].x = -10000;
						Lhand3Ds[lh3d].y = -10000;
					}
					Lhand3Ds[lh3d].accuracy = Lhand2Ds[lh3d].accuracy;
					//Lhand3Ds[lh3d].z = (int)get_actDepth(inputDepth, Lhand2Ds[lh3d].y, Lhand2Ds[lh3d].x, 10);
					if(Lhand3Ds[lh3d].x < 0 || Lhand3Ds[lh3d].x > 1919 || Lhand3Ds[lh3d].y < 0 || Lhand3Ds[lh3d].y > 1079){
							Lhand3Ds[lh3d].z = 0;
					}else{
						Lhand3Ds[lh3d].z = (int)inputDepth.at<ushort>( Lhand3Ds[lh3d].y, Lhand3Ds[lh3d].x );
					}
					//Lhand3Ds[lh3d].z = (int)inputDepth.at<ushort>( Lhand2Ds[lh3d].y, Lhand2Ds[lh3d].x );
					//if( Lhand3Ds[lh3d].z < 500 || Lhand3Ds[lh3d].z >= 8000 ){
					//	Lhand3Ds[lh3d].z = -10000;
					//}
				}
				if(min_LhandX_px != max_LhandX_px && min_LhandY_px != max_LhandY_px){
					Lhand_spanX_px = max_LhandX_px - min_LhandX_px;
					Lhand_spanY_px = max_LhandY_px - min_LhandY_px;
					Lhand_span_px = max(Lhand_spanX_px, Lhand_spanY_px);
					cout << "hand span(px): " << Lhand_span_px << endl;
				}
				hand_median_px = get_median_px(Lhand3Ds);
				limit_hand_depth_px(Lhand3Ds, hand_median_px, 200, Lhand_span_px, 3000, inputDepth);

				//右手関節点群XYにdepth情報を追加した3Dデータを作成
				double min_RhandX_px = NULL;
				double min_RhandY_px = NULL;
				double max_RhandX_px = NULL;
				double max_RhandY_px = NULL;
				double Rhand_spanX_px, Rhand_spanY_px, Rhand_span_px = 0;
				for( int rh3d = 0; rh3d < Rhand3Ds.size(); rh3d++ ){
					if( Rhand2Ds[rh3d].accuracy != 0 ){
						Rhand3Ds[rh3d].x = Rhand2Ds[rh3d].x;
						Rhand3Ds[rh3d].y = Rhand2Ds[rh3d].y;
						if(min_RhandX_px == NULL && max_RhandX_px == NULL){min_RhandX_px = Rhand3Ds[rh3d].x; max_RhandX_px = Rhand3Ds[rh3d].x;}
						if(min_RhandY_px == NULL && max_RhandY_px == NULL){min_RhandY_px = Rhand3Ds[rh3d].y; max_RhandY_px = Rhand3Ds[rh3d].y;}
						if(min_RhandX_px > Rhand3Ds[rh3d].x){min_RhandX_px = Rhand3Ds[rh3d].x;}
						if(max_RhandX_px < Rhand3Ds[rh3d].x){max_RhandX_px = Rhand3Ds[rh3d].x;}
						if(min_RhandY_px > Rhand3Ds[rh3d].y){min_RhandY_px = Rhand3Ds[rh3d].y;}
						if(max_RhandY_px < Rhand3Ds[rh3d].y){max_RhandY_px = Rhand3Ds[rh3d].y;}
					}else{
						Rhand3Ds[rh3d].x = -10000;
						Rhand3Ds[rh3d].y = -10000;
					}
					Rhand3Ds[rh3d].accuracy = Rhand2Ds[rh3d].accuracy;
					//Rhand3Ds[lh3d].z = (int)get_actDepth(inputDepth, Rhand2Ds[lh3d].y, Rhand2Ds[lh3d].x, 10);
					if(Rhand3Ds[rh3d].x < 0 || Rhand3Ds[rh3d].x > 1919 || Rhand3Ds[rh3d].y < 0 || Rhand3Ds[rh3d].y > 1079){
							Rhand3Ds[rh3d].z = 0;
					}else{
						Rhand3Ds[rh3d].z = (int)inputDepth.at<ushort>( Rhand3Ds[rh3d].y, Rhand3Ds[rh3d].x );
					}
					//Rhand3Ds[lh3d].z = (int)inputDepth.at<ushort>( Rhand2Ds[lh3d].y, Rhand2Ds[lh3d].x );
					//if( Rhand3Ds[lh3d].z < 500 || Rhand3Ds[lh3d].z >= 8000 ){
					//	Rhand3Ds[lh3d].z = -10000;
					//}
				}
				if(min_RhandX_px != max_RhandX_px && min_RhandY_px != max_RhandY_px){
					Rhand_spanX_px = max_RhandX_px - min_RhandX_px;
					Rhand_spanY_px = max_RhandY_px - min_RhandY_px;
					Rhand_span_px = max(Rhand_spanX_px, Rhand_spanY_px);
					cout << "hand span(px): " << Rhand_span_px << endl;
				}
				hand_median_px = get_median_px(Rhand3Ds);
				limit_hand_depth_px(Rhand3Ds, hand_median_px, 200, Rhand_span_px, 3000, inputDepth);

				/*以前のコード(右手)
				for( int rh3d = 0; rh3d < Rhand3Ds.size(); rh3d++ ){
					if( Rhand2Ds[rh3d].accuracy != 0 ){
						Rhand3Ds[rh3d].x = Rhand2Ds[rh3d].x;
						Rhand3Ds[rh3d].y = Rhand2Ds[rh3d].y;
					}else{
						Rhand3Ds[rh3d].x = -10000;
						Rhand3Ds[rh3d].y = -10000;
					}
					Rhand3Ds[rh3d].accuracy = Rhand2Ds[rh3d].accuracy;
					Rhand3Ds[rh3d].z = (int)get_actDepth(inputDepth, Rhand2Ds[rh3d].y, Rhand2Ds[rh3d].x, 5);
					//Rhand3Ds[rh3d].z = (int)inputDepth.at<ushort>( Rhand2Ds[rh3d].y, Rhand2Ds[rh3d].x );
					//if( Rhand3Ds[rh3d].z < 500 || Rhand3Ds[rh3d].z >= 8000 ){
					//	Rhand3Ds[rh3d].z = -10000;
					//}
				}*/
				
				//2021/02/17追加
				//手首位置を原点とした二次元スケルトンを、距離によるスケール差を均一化処理して出力する
				std::vector<Point2D> scaleUniformedLH2Ds_px(Lhand3Ds.size());
				std::vector<Point2D> scaleUniformedRH2Ds_px(Rhand3Ds.size());
				double suLhandP0X, suLhandP0Y, suRhandP0X, suRhandP0Y;

				if(saveL){
					cout << "左手2D" << endl;
					//手首位置のdepthを取得
					double LH0_depth = Lhand3Ds[0].z;
					if(Lhand3Ds[0].accuracy == 0 || LH0_depth == 0 || LH0_depth == -10000){
						//手首位置XYがわからない or 手首depthが0 or 手首depthが-10000
						for(int err = 0; err < Lhand3Ds.size(); err++){
							//無効値と信頼度0を埋めて終了
							scaleUniformedLH2Ds_px[err].x = -10000;
							scaleUniformedLH2Ds_px[err].y = -10000;
							scaleUniformedLH2Ds_px[err].confidence = 0;
						}
					}else{
						for( int l2d = 0; l2d < Lhand3Ds.size(); l2d++ ){
							if( Lhand3Ds[l2d].accuracy != 0){
								//信頼度値はそのまま代入
								scaleUniformedLH2Ds_px[l2d].confidence = Lhand3Ds[l2d].accuracy;
								//まずは画像中心基準座標系に変換
								double pWfromC = Lhand3Ds[l2d].x - input.getwidth() / 2;
								double pHfromC = Lhand3Ds[l2d].y - input.getheight() / 2;
								//depth3000ラインを基準とした遠近によるピクセルスケール均一化補正
								std::pair<double, double> suLHp = pxScaleUniform(pWfromC, pHfromC, LH0_depth, 3000.);
								scaleUniformedLH2Ds_px[l2d].x = suLHp.first;
								scaleUniformedLH2Ds_px[l2d].y = suLHp.second;
								
								
								
							}else{
								scaleUniformedLH2Ds_px[l2d].x = -10000;
								scaleUniformedLH2Ds_px[l2d].y = -10000;
							}
						}
						for( int l2d = 1; l2d < Lhand3Ds.size(); l2d++ ){
							//手首位置基準座標系に変換
							scaleUniformedLH2Ds_px[l2d].x -= scaleUniformedLH2Ds_px[0].x;
							scaleUniformedLH2Ds_px[l2d].y -= scaleUniformedLH2Ds_px[0].y;
						}
						scaleUniformedLH2Ds_px[0].x -= scaleUniformedLH2Ds_px[0].x;
						scaleUniformedLH2Ds_px[0].y -= scaleUniformedLH2Ds_px[0].y;
					}
					for( int l2d = 0; l2d < Lhand3Ds.size(); l2d++ ){
						Points_Lhand_pxmmToCSV << l2d << "," << scaleUniformedLH2Ds_px[l2d].x << "," << scaleUniformedLH2Ds_px[l2d].y << ","
							<< scaleUniformedLH2Ds_px[l2d].confidence << endl;
					}
					Points_Lhand_pxmmToCSV.close();
				}
				if(saveR){
					cout << "右手2D" << endl;
					//手首位置のdepthを取得
					double RH0_depth = Rhand3Ds[0].z;
					if(Rhand3Ds[0].accuracy == 0 || RH0_depth == 0 || RH0_depth == -10000){
						//手首位置XYがわからない or 手首depthが0 or 手首depthが-10000
						for(int err = 0; err < Rhand3Ds.size(); err++){
							//無効値と信頼度0を埋めて終了
							scaleUniformedRH2Ds_px[err].x = -10000;
							scaleUniformedRH2Ds_px[err].y = -10000;
							scaleUniformedRH2Ds_px[err].confidence = 0;
						}
					}else{
						for( int r2d = 0; r2d < Rhand3Ds.size(); r2d++ ){
							if( Rhand3Ds[r2d].accuracy != 0){
								//信頼度値はそのまま代入
								scaleUniformedRH2Ds_px[r2d].confidence = Rhand3Ds[r2d].accuracy;
								//まずは画像中心基準座標系に変換
								double pWfromC = Rhand3Ds[r2d].x - input.getwidth() / 2;
								double pHfromC = Rhand3Ds[r2d].y - input.getheight() / 2;
								//depth3000ラインを基準とした遠近によるピクセルスケール均一化補正
								std::pair<double, double> suRHp = pxScaleUniform(pWfromC, pHfromC, RH0_depth, 3000.);
								scaleUniformedRH2Ds_px[r2d].x = suRHp.first;
								scaleUniformedRH2Ds_px[r2d].y = suRHp.second;
							}else{
								scaleUniformedRH2Ds_px[r2d].x = -10000;
								scaleUniformedRH2Ds_px[r2d].y = -10000;
							}
						}
						for( int r2d = 1; r2d < Rhand3Ds.size(); r2d++ ){
							//手首位置基準座標系に変換
							scaleUniformedRH2Ds_px[r2d].x -= scaleUniformedRH2Ds_px[0].x;
							scaleUniformedRH2Ds_px[r2d].y -= scaleUniformedRH2Ds_px[0].y;
						}
						scaleUniformedRH2Ds_px[0].x -= scaleUniformedRH2Ds_px[0].x;
						scaleUniformedRH2Ds_px[0].y -= scaleUniformedRH2Ds_px[0].y;
					}
					for( int r2d = 0; r2d < Rhand3Ds.size(); r2d++ ){
						Points_Rhand_pxmmToCSV << r2d << "," << scaleUniformedRH2Ds_px[r2d].x << "," << scaleUniformedRH2Ds_px[r2d].y << ","
							<< scaleUniformedRH2Ds_px[r2d].confidence << endl;
					}
					Points_Rhand_pxmmToCSV.close();
				}


				//ここから、手首を原点とした三次元座標系に変換する

				//手首位置座標の取得
				int LhandP0X, LhandP0Y, LhandP0Z, RhandP0X, RhandP0Y, RhandP0Z;
				LhandP0X = Lhand3Ds[0].x;
				LhandP0Y = Lhand3Ds[0].y;
				LhandP0Z = Lhand3Ds[0].z;
				RhandP0X = Rhand3Ds[0].x;
				RhandP0Y = Rhand3Ds[0].y;
				RhandP0Z = Rhand3Ds[0].z;

				/*//XYZ => pixel pixel mm 座標系
				std::vector<point3D> LH0based3Ds_pxmm( Lhand3Ds.size() );
				std::vector<point3D> RH0based3Ds_pxmm( Rhand3Ds.size() );

				for( int l3d = 0; l3d < Lhand3Ds.size(); l3d++ ){
					if( Lhand3Ds[l3d].accuracy != 0 ){
						LH0based3Ds_pxmm[l3d].x = (int)( Lhand3Ds[l3d].x - LhandP0X );
						LH0based3Ds_pxmm[l3d].y = (int)( Lhand3Ds[l3d].y - LhandP0Y );
						if( Lhand3Ds[l3d].z != -10000 ){
							LH0based3Ds_pxmm[l3d].z = (int)( Lhand3Ds[l3d].z - LhandP0Z );
						}
						else{
							LH0based3Ds_pxmm[l3d].z = -10000;
						}
					}else{
						LH0based3Ds_pxmm[l3d].x = -10000;
						LH0based3Ds_pxmm[l3d].y = -10000;
						LH0based3Ds_pxmm[l3d].z = -10000;
					}
					LH0based3Ds_pxmm[l3d].accuracy = Lhand3Ds[l3d].accuracy;
					Points_Lhand_pxmmToCSV << l3d << "," << LH0based3Ds_pxmm[l3d].x << "," << LH0based3Ds_pxmm[l3d].y << ","
						<< LH0based3Ds_pxmm[l3d].accuracy << "," << LH0based3Ds_pxmm[l3d].z << endl;
				}
				Points_Lhand_pxmmToCSV.close();

				for( int r3d = 0; r3d < Rhand3Ds.size(); r3d++ ){
					if( Rhand3Ds[r3d].accuracy != 0 ){
						RH0based3Ds_pxmm[r3d].x = (int)( Rhand3Ds[r3d].x - RhandP0X );
						RH0based3Ds_pxmm[r3d].y = (int)( Rhand3Ds[r3d].y - RhandP0Y );
						if( Rhand3Ds[r3d].z != -10000 ){
							RH0based3Ds_pxmm[r3d].z = (int)( Rhand3Ds[r3d].z - RhandP0Z );
						}
						else{
							RH0based3Ds_pxmm[r3d].z = -10000;
						}
					}else{
						RH0based3Ds_pxmm[r3d].x = -10000;
						RH0based3Ds_pxmm[r3d].y = -10000;
						RH0based3Ds_pxmm[r3d].z = -10000;
					}
					RH0based3Ds_pxmm[r3d].accuracy = Rhand3Ds[r3d].accuracy;
					Points_Rhand_pxmmToCSV << r3d << "," << RH0based3Ds_pxmm[r3d].x << "," << RH0based3Ds_pxmm[r3d].y << ","
						<< RH0based3Ds_pxmm[r3d].accuracy << "," << RH0based3Ds_pxmm[r3d].z << endl;
				}
				Points_Rhand_pxmmToCSV.close();*/

				//XYZ => mm mm mm 座標系
				std::vector<Point3D> LH0based3Ds_mm( Lhand3Ds.size() );
				std::vector<Point3D> RH0based3Ds_mm( Rhand3Ds.size() );
				std::pair<double, double> LH0_mm;
				std::pair<double, double> RH0_mm;

				//手首位置のmm mm mm 座標系変換
				LH0_mm = mmTransform(LhandP0X, LhandP0Y, LhandP0Z);
				RH0_mm = mmTransform(RhandP0X, RhandP0Y, RhandP0Z);
				cout << "LH0(mm) : [" << LH0_mm.first << ", " << LH0_mm.second << ", " << LhandP0Z << "]" << endl;
				cout << "RH0(mm) : [" << RH0_mm.first << ", " << RH0_mm.second << ", " << RhandP0Z << "]" << endl;
				
				double min_handX = NULL;
				double min_handY = NULL;
				double max_handX = NULL;
				double max_handY = NULL;
				double hand_spanX, hand_spanY, hand_span = 0;

				double min_orig_handX = NULL;
				double min_orig_handY = NULL;
				double max_orig_handX = NULL;
				double max_orig_handY = NULL;
				double orig_hand_spanX, orig_hand_spanY, orig_hand_span = 0;
				//左手各関節点の座標をmm mm mm 座標系変換
				if(saveL){
					cout << "左手" << endl;
					for( int l3d = 0; l3d < Lhand3Ds.size(); l3d++ ){
						if( Lhand3Ds[l3d].accuracy != 0 && Lhand3Ds[l3d].z != -10000){
							LH0based3Ds_mm[l3d].z = Lhand3Ds[l3d].z - LhandP0Z;
							std::pair<double, double> LHp_mm = mmTransform(Lhand3Ds[l3d].x, Lhand3Ds[l3d].y, Lhand3Ds[l3d].z);
							LH0based3Ds_mm[l3d].x = LHp_mm.first - LH0_mm.first;
							LH0based3Ds_mm[l3d].y = LHp_mm.second - LH0_mm.second;
							if(min_handX == NULL && max_handX == NULL){min_handX = LH0based3Ds_mm[l3d].x; max_handX = LH0based3Ds_mm[l3d].x;}
							if(min_handY == NULL && max_handY == NULL){min_handY = LH0based3Ds_mm[l3d].y; max_handY = LH0based3Ds_mm[l3d].y;}
							if(min_handX > LH0based3Ds_mm[l3d].x){min_handX = LH0based3Ds_mm[l3d].x;}
							if(max_handX < LH0based3Ds_mm[l3d].x){max_handX = LH0based3Ds_mm[l3d].x;}
							if(min_handY > LH0based3Ds_mm[l3d].y){min_handY = LH0based3Ds_mm[l3d].y;}
							if(max_handY < LH0based3Ds_mm[l3d].y){max_handY = LH0based3Ds_mm[l3d].y;}
						}else{
							LH0based3Ds_mm[l3d].x = -10000;
							LH0based3Ds_mm[l3d].y = -10000;
							LH0based3Ds_mm[l3d].z = -10000;
						}
						LH0based3Ds_mm[l3d].confidense = Lhand3Ds[l3d].accuracy;
						//物体位置基準座標系に変換する前の情報も保持しておく(XYはpixel単位)
						LH0based3Ds_mm[l3d].orig_x = Lhand3Ds[l3d].x;
						LH0based3Ds_mm[l3d].orig_y = Lhand3Ds[l3d].y;
						LH0based3Ds_mm[l3d].orig_z = Lhand3Ds[l3d].z;
						if(min_orig_handX == NULL && max_orig_handX == NULL && LH0based3Ds_mm[l3d].orig_x != -10000){min_orig_handX = LH0based3Ds_mm[l3d].orig_x; max_orig_handX = LH0based3Ds_mm[l3d].orig_x;}
						if(min_orig_handY == NULL && max_orig_handY == NULL && LH0based3Ds_mm[l3d].orig_y != -10000){min_orig_handY = LH0based3Ds_mm[l3d].orig_y; max_orig_handY = LH0based3Ds_mm[l3d].orig_y;}
						if(min_orig_handX > LH0based3Ds_mm[l3d].orig_x && LH0based3Ds_mm[l3d].orig_x != -10000){min_orig_handX = LH0based3Ds_mm[l3d].orig_x;}
						if(max_orig_handX < LH0based3Ds_mm[l3d].orig_x && LH0based3Ds_mm[l3d].orig_x != -10000){max_orig_handX = LH0based3Ds_mm[l3d].orig_x;}
						if(min_orig_handY > LH0based3Ds_mm[l3d].orig_y && LH0based3Ds_mm[l3d].orig_y != -10000){min_orig_handY = LH0based3Ds_mm[l3d].orig_y;}
						if(max_orig_handY < LH0based3Ds_mm[l3d].orig_y && LH0based3Ds_mm[l3d].orig_y != -10000){max_orig_handY = LH0based3Ds_mm[l3d].orig_y;}
					}
					//depthの補正
					hand_span = 200; //手の奥行き制限範囲を固定の20cmにする
					//pixel単位でも計算しておく
					if(min_orig_handX != max_orig_handX && min_orig_handY != max_orig_handY){
						orig_hand_spanX = max_orig_handX - min_orig_handX;
						orig_hand_spanY = max_orig_handY - min_orig_handY;
						orig_hand_span = max(orig_hand_spanX, orig_hand_spanY);
						cout << "hand span(px): " << orig_hand_span << endl;
					}
					//手のdepthの中央値を手の中心として取得
					double hand_median = get_median(LH0based3Ds_mm);
					cout << "hand median(mm): " << hand_median << endl; 
					limit_hand_depth(LH0based3Ds_mm, hand_median, hand_span, orig_hand_span, LhandP0Z, inputDepth);
					fix_hand_depth(LH0based3Ds_mm, hand_median);
					for( int ph3d = 0; ph3d < Lhand3Ds.size(); ph3d++ ){
						LH0based3Ds_mm[ph3d].confidense = Lhand3Ds[ph3d].accuracy;
						Points_Lhand_mmToCSV << ph3d << "," << LH0based3Ds_mm[ph3d].x << "," << LH0based3Ds_mm[ph3d].y << ","
							<< LH0based3Ds_mm[ph3d].confidense << "," << LH0based3Ds_mm[ph3d].z << endl;
					}
					Points_Lhand_mmToCSV.close();
				}
				

				//左手各関節点の座標をmm mm mm 座標系変換
				if(saveR){
					cout << "右手" << endl;
					for( int r3d = 0; r3d < Rhand3Ds.size(); r3d++ ){
						if( Rhand3Ds[r3d].accuracy != 0 && Rhand3Ds[r3d].z != -10000){
							RH0based3Ds_mm[r3d].z = Rhand3Ds[r3d].z - RhandP0Z;
							std::pair<double, double> RHp_mm = mmTransform(Rhand3Ds[r3d].x, Rhand3Ds[r3d].y, Rhand3Ds[r3d].z);
							RH0based3Ds_mm[r3d].x = RHp_mm.first - RH0_mm.first;
							RH0based3Ds_mm[r3d].y = RHp_mm.second - RH0_mm.second;
							if(min_handX == NULL && max_handX == NULL){min_handX = RH0based3Ds_mm[r3d].x; max_handX = RH0based3Ds_mm[r3d].x;}
							if(min_handY == NULL && max_handY == NULL){min_handY = RH0based3Ds_mm[r3d].y; max_handY = RH0based3Ds_mm[r3d].y;}
							if(min_handX > RH0based3Ds_mm[r3d].x){min_handX = RH0based3Ds_mm[r3d].x;}
							if(max_handX < RH0based3Ds_mm[r3d].x){max_handX = RH0based3Ds_mm[r3d].x;}
							if(min_handY > RH0based3Ds_mm[r3d].y){min_handY = RH0based3Ds_mm[r3d].y;}
							if(max_handY < RH0based3Ds_mm[r3d].y){max_handY = RH0based3Ds_mm[r3d].y;}
						}else{
							RH0based3Ds_mm[r3d].x = -10000;
							RH0based3Ds_mm[r3d].y = -10000;
							RH0based3Ds_mm[r3d].z = -10000;
						}
						RH0based3Ds_mm[r3d].confidense = Rhand3Ds[r3d].accuracy;
						//物体位置基準座標系に変換する前の情報も保持しておく(XYはpixel単位)
						RH0based3Ds_mm[r3d].orig_x = Rhand3Ds[r3d].x;
						RH0based3Ds_mm[r3d].orig_y = Rhand3Ds[r3d].y;
						RH0based3Ds_mm[r3d].orig_z = Rhand3Ds[r3d].z;
						if(min_orig_handX == NULL && max_orig_handX == NULL && RH0based3Ds_mm[r3d].orig_x != -10000){min_orig_handX = RH0based3Ds_mm[r3d].orig_x; max_orig_handX = RH0based3Ds_mm[r3d].orig_x;}
						if(min_orig_handY == NULL && max_orig_handY == NULL && RH0based3Ds_mm[r3d].orig_y != -10000){min_orig_handY = RH0based3Ds_mm[r3d].orig_y; max_orig_handY = RH0based3Ds_mm[r3d].orig_y;}
						if(min_orig_handX > RH0based3Ds_mm[r3d].orig_x && RH0based3Ds_mm[r3d].orig_x != -10000){min_orig_handX = RH0based3Ds_mm[r3d].orig_x;}
						if(max_orig_handX < RH0based3Ds_mm[r3d].orig_x && RH0based3Ds_mm[r3d].orig_x != -10000){max_orig_handX = RH0based3Ds_mm[r3d].orig_x;}
						if(min_orig_handY > RH0based3Ds_mm[r3d].orig_y && RH0based3Ds_mm[r3d].orig_y != -10000){min_orig_handY = RH0based3Ds_mm[r3d].orig_y;}
						if(max_orig_handY < RH0based3Ds_mm[r3d].orig_y && RH0based3Ds_mm[r3d].orig_y != -10000){max_orig_handY = RH0based3Ds_mm[r3d].orig_y;}
					}
					//depthの補正
					hand_span = 200; //手の奥行き制限範囲を固定の20cmにする
					//pixel単位でも計算しておく
					if(min_orig_handX != max_orig_handX && min_orig_handY != max_orig_handY){
						orig_hand_spanX = max_orig_handX - min_orig_handX;
						orig_hand_spanY = max_orig_handY - min_orig_handY;
						orig_hand_span = max(orig_hand_spanX, orig_hand_spanY);
						cout << "hand span(px): " << orig_hand_span << endl;
					}
					//手のdepthの中央値を手の中心として取得
					double hand_median = get_median(RH0based3Ds_mm);
					cout << "hand median(mm): " << hand_median << endl; 
					limit_hand_depth(RH0based3Ds_mm, hand_median, hand_span, orig_hand_span, RhandP0Z, inputDepth);
					fix_hand_depth(RH0based3Ds_mm, hand_median);
					for( int ph3d = 0; ph3d < Rhand3Ds.size(); ph3d++ ){
						RH0based3Ds_mm[ph3d].confidense = Rhand3Ds[ph3d].accuracy;
						Points_Rhand_mmToCSV << ph3d << "," << RH0based3Ds_mm[ph3d].x << "," << RH0based3Ds_mm[ph3d].y << ","
							<< RH0based3Ds_mm[ph3d].confidense << "," << RH0based3Ds_mm[ph3d].z << endl;
					}
					Points_Rhand_mmToCSV.close();
				}
				
			}
			std::string HRcheckedTextFile = date_time + "\\HRchecked.txt";
			FILE *fp;
			fp = fopen( HRcheckedTextFile.c_str(), "w" );
			fclose( fp );
			cout << dt << "fin." << endl;
		}
	}
}