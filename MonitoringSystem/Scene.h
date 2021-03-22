//#pragma once
//#include ".\image.h"
//#include ".\bmp.h"
//#include <map>
//#include <deque>
//#include <vector>
//
//
////必要
////
//// 今何層目まで重なってるか？
//// シーンにマスク画像追加
//// 今どんなマスクが乗っているのか辿る？
//// マスクの上下関係を更新
//// 現在の状態
//// ある座標周りに履歴からどんな物体が検知されたか調べる
//// ある時の画像出力
//// 物体画像の追加，更新，削除，履歴
////
//// ある座標(i,j)に重なる一番上の物体画像出力
//
////
//// pop push　delete clear disp update
//// search
////	void Set_img(bmp &obj){ img = obj; }
////	bmp &Get_img(){ return img;}
//	
//
////=====================
////物体画像を表す構造体
////=====================
//typedef struct obj{
//	bmp img;  //画像(物体検知された)
//	int layer;//何層目か?
//	int id;   //画像ID
//}Object_img;
//
////-------------------
////Class
////-------------------
//class Scene
//{
//	//===============================================================================
//private:
//	std::map<std::string, bmp> bmp_map; //物体画像の履歴 key＜時刻，画像＞      
//	std::vector<Object_img>    obj;  //物体画像を表す構造体を格納する
//	//===============================================================================
//
//	//-------------------------------------------------------------------------------
//public:
//	// constructor
//	Scene(void){}
//	// destructor
//	~Scene(void){}
//
//	//初期化
//	void init(){
//	}
//
//	//現在時刻を文字列で返す
//	std::string get_time();
//
//	//物体の履歴
//	void set_image(bmp &img);
//
//	//差分のチェック
//	bool CheckDiffNum(image &img,image &start,Area *area);
//
//	double SimilarLevel(bmp &pre,bmp &now,Area *area);
//	
//	//２つの画像の論理和画像生成
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
//	//現在のシーンにある画像全て表示
//	void disp(){
//		for(std::vector<Object_img>::iterator i = obj.begin();i!=obj.end();i++){
//			std::cout << i[0].id << std::endl;
// 		}
//	}
//
//	//-------------------------------------------------------------------------------
//};
