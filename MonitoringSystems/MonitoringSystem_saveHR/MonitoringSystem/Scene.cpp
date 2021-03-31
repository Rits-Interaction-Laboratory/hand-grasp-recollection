///////////////////////////////////////////////////////////
// FileName : scene.cpp
// Context  : シーンの解析(過去の履歴，現在の状態)
//
// Author　 :  Noriaki Katayama(katayama@i.ci.ritsumei.ac.jp)
//             Computer Vision Lab.
//             Department of Human and Computer Intelligence
//             College of Information Science & Engineering
//             Ritsumeikan University
//
// Update   : 2008.04.21
///////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE 1 /*VisualC++でのlocaltime()の警告抑制*/
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
//'=' : 'double' から 'int' に変換しました。データが失われているかもしれません。

//-------------------------------------------------------
// Name     : get_time()
// Author　 : Katayama Noriaki (CV-lab.)
// function : 現在時刻を得る
// Argument : なし
// return   : std::string 型の文字列
// Update   : 2005.12.3
//--------------------------------------------------------
std::string Scene::get_time()
{
	time_t nt;
	struct tm *t_st;
	char c[30];

	std::ostringstream oss;
	//time(&nt);             //現在時刻の取得
	//t_st = localtime(&nt); //現在時刻を構造体に変換 
	localtime_s(t_st,&nt);
    //strftime(c,sizeof(c),"%Y-%m-%d-%H時%M分%S秒",t_st);
	strftime(c,sizeof(c),"%Y-%m-%d-%H-%M-%S",t_st);

	std::string f_time = c;

	return f_time;
}

//-------------------------------------------------------
// Name     : set_image()
// Author　 : Katayama Noriaki (CV-lab.)
// function : 
// Argument : 
// return   : なし
// Update   : 2005.7
//--------------------------------------------------------
void Scene::set_image(bmp &img)
{
	std::map<std::string, bmp>::iterator p; //イテレータ

	//画像を現在時刻をキーにして挿入する
	std::string now_t = get_time();

	//現在時刻をキーにして画像を格納する
	bmp_map.insert(std::map<std::string,bmp>::value_type(now_t,img));

	std::cout << get_time() << std::endl;

	//std::string fliename = now_t + ".bmp";//日付のファイル名
	
	//ファイル書き出し
	//bmp_map[now_t].save(fliename.c_str());


	// 検索
	//p = bmp_map.find("02"); 

	std::cout << "-----現在の格納されているファイル------" << std::endl;
	for(p = bmp_map.begin() ;p != bmp_map.end();p++){
		std::string name = p->first + ".png";
		std::cout << name  << std::endl;
		p->second.save(name.c_str());
	}


/*
	//マップを検索
	//if(p != bmp_map.end())
	//{
	//	std::string name = p->first;
	//	std::cout << name << "という画像がありました" << std::endl;
	//	p->second.save("aaaaaaaaaa.bmp");
	//}
	//else{
	//	std::cout << "見つかりません" << std::endl;
	//}

	*/
}

//------------------------------------------------------------------------
// Name     : CheckDiffNum()
// Author　 : Katayama Noriaki (CV-lab.)
// function : 物体かどうかのチェック
//            差分がある区間で同じ場所にあれば物体とした
// Argument : image &img     差分を調べる画像
//            image &start   過去の画像
//            Area *area     調べる範囲の座標の構造体
//
// return   : bool           差分が残っていれば物体
// Update   : 2005.12.3
//------------------------------------------------------------------------
bool Scene::CheckDiffNum(image &img,image &start,Area *area)
{
	int pre_num = 0; //過去の差分の数
	int now_num = 0; //現在の差分の数
	double hold;        //差分の数の閾値

	//差をそれぞれ調べる
	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(start.isDiff(i,j) == BACK) pre_num++;
			if(img.isDiff(i,j)   == BACK) now_num++;
		}
	}

	std::cout  << "前" << pre_num << std::endl;
	std::cout  << "後" << now_num << std::endl;

	//まったく同じ数にはならないので調整
	hold = pre_num * 0.7;

	//過去の画像同じ場所に差分があれば物体 trueを返す
	if(now_num >= hold) return true;

	//物体検知はされなかったのでfalse
	else return false;
}


//------------------------------------------------------------------------
// Name     : AndImage()
// Author　 : Katayama Noriaki (CV-lab.)
// function : 差分画像同士の論理和をとる
//
// Argument : image &start  開始フレームの画像
//            image &end,　 終了フレーム画像
//            image &result 論理和をとった画像(ここに結果が入る)
//            Area *area    チェックする領域の座標(構造体)
//
// return   : なし
// Update   : 2005.12.3
//------------------------------------------------------------------------
void Scene::AndImage(bmp &start,bmp &end,bmp &result,Area *area)
{
	for(int j=area->s_h; j<=area->e_h; j++){
		for(int i=area->s_w; i<=area->e_w; i++){
			if(start.isDiff(i,j) == BACK && end.isDiff(i,j) == BACK){
				//２つの画像で同じ座標で背景差分だったら物体の差分
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
	int pre_num = 0; //過去の差分の数
	int now_num = 0; //現在の差分の数

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