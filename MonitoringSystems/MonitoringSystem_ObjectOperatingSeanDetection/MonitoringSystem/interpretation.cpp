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

	const int b_threshold = 20;//差分の閾値

	static int del_objct_num = 0; //staticに変更
	bool no_exist = true;

	static int del_num=0;    //staticに変更

	

	


	//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
	// ↓物体検知が成功した画像についてシーンの解釈を行う
	// 影とならなかったものだけについて！！

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//読み込んだマスクの部分について下のオブジェクトとの重なり判定
	//[check]一つ前の予想状態との比較
	int delete_key = -1;
	bool same_flag = false;
	bool delete_flag = false;


	//object_img[i]とすべてについてマッチング！！論理積(ANDをとる)
	for(int index = 1; index < object_img.size(); index++){
		same_flag = check(input, object_img[index]);
		if(same_flag == true){//各マスクとのマッチングが真になったら
			//消すレイヤのID
			delete_key = l_map2[index];
			del_objct_num = index;
			delete_flag = true;
			for(int j=0; j < Height; j++){
				for(int i=0; i < Width; i++){
					//end() からbegin()まで辿る = 上から見たようになる
					std::vector<int>::iterator it = lay.m_val(i,j).end();

					while( it != lay.m_val(i,j).begin()){
						--it;
						if(*it != -1){

							//オブジェクトIDがもつレイヤをget
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
//		//持ち去りなら
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




//-----レイヤー構造を用いた物体の持ち込み･持ち去り･移動検知処理部分


int Cinterpretation::interpretation(image &input, unsigned long frame, bmp &det, int delete_key, int obj_id, int key_id, bool track_flag)
{
	static layer lay;//kawa　保持するレイヤー構造の部分持ち込みが増えれば容量も増えていく。
	

	int Height = 240;
	int Width  = 320;
	static bmp output,output_val;//kawa    outputレイヤー別に色分けされた値   output_val上から見た値


//////////////////////////////////////////////////////////////////////////////
	//static bmp mask_img[10];//動的配列に変更//kawa
	static std::vector<bmp>  mask_img;
	mask_img.reserve(30);
	object_img.reserve(30);
	static unsigned int laydata=1;
//////////////////////////////////////////////////////////////////////////////


	//bmp outputmask[10];//kawa　テスト用
	std::string filename;

	const int b_threshold = 20;//差分の閾値

	static int del_objct_num = 0; //staticに変更
	bool no_exist = true;
	bool objId = false;

	static int del_num=0;    //staticに変更

	int count[320*240]={-1};
	int table[10]={-1};//動的配列に変更//kawa

	//int out_objId = -1;

	FileOperator file;
	FILE *fl;

	int LayNum=0;

	fopen_s(&fl,"../DB/Detected-Object/object.log","a");

	std::cout <<  "key_id" <<key_id<<"obj__id"<<obj_id<<"delete_key"<<delete_key<< std::endl;

#if 1 //■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
	// ↓物体検知が成功した画像についてシーンの解釈を行う
	// 影とならなかったものだけについて！！

	Geometory::Point2D posCenter(input.get_xc(),input.get_yc());//物体の重心
	//物体の外接長方形
	Area o_area = input.get_area();

	//物体の外接長方形の頂点座標
	Geometory::Point2D posStart(0 + o_area.s_w, 0 + o_area.s_h);//(0,0)を基点に物体の外接長方形の左上座標
	Geometory::Point2D posEnd(0 + o_area.e_w, 0 + o_area.e_h);//(0,0)を基点に物体の外接長方形の右下座標

	//物体の面積
	int objArea = input.get_square();

	/*
	 * ログに情報を書き出す
	 * :ファイル名
	 * :物体の重心座標(x,y)
	 * :物体の外接長方形の1つの頂点座標(左上)
	 * :物体の外接長方形の1つの頂点座標(左上)の反対側の頂点座標(右下)
	 * :物体の面積
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


	
	//物体差分情報をセーブ
	//TODO:error
	cout << "cvCreateImage(cvSize(" << input.getwidth() << "," << input.getheight() << ")," << IPL_DEPTH_8U << "," << 3 << ")" << endl; 

	IplImage *obj = cvCreateImage(cvSize(input.getwidth(),input.getheight()),IPL_DEPTH_8U,3);

	if(obj == NULL){
		cout << "error" << endl;
	}
	//マッチング後の処理ができればあとはできる！！！！！！
	//std::cout <<  delete_key << std::endl;
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		



	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//取り去った後、背景が変わっていたときの処理(片山氏の例外処理の部分)
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//差分をとったとき差分ピクセルになった部分については再度ラベリング


	//次はこれをやれ！！！！！！！！！



	//消すレイヤの一つ下のレイヤのオブジェクトとの差分をとって自分に代入する


	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//同じ要素を持つ画素についてレイヤ操作
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//指定した値があるレイヤすべてdeleteする
	//    ↓
	//全てのレイヤーを見直す
	bool delete_flag = false;

	//消すレイヤ番号を指定
	int pos = lay.get_layer(delete_key);



//持ち去り時の例外処理
	if(delete_key != -1){
		int ex_pixel = 0;
		input.grayscale();//入力画像を濃淡画像 input.RGBに濃淡値を入れる。
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				if(input.isDiff(i,j) == OBJECT){
					double diff = sqrt((double)((output_val.R(i,j) - input.R(i,j)) * (output_val.R(i,j) - input.R(i,j))));//kawa濃淡値により背景のさを比較?例外処理の部分？

					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = output_val.R(i,j);//値valがはいっているval=object_img[pos].R(i,j)
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
					if(diff < b_threshold){//差が閾値以下なら背景
						//hoge.SetPixColor(i,j,255,255,255);
					}
					else{//背景と差分がでたら(閾値:20)
						//input.isDiff(i,j) = B_OBJECT;
						//object_img[del_objct_num].isDiff(i,j) = B_OBJECT;
						object_img[pos].isDiff(i,j) = B_OBJECT;//
						ex_pixel++;//隠れていたピクセルの量
					}
				}else{
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = 0;
				}
			}
		}

		//収縮・膨張
		//object_img[del_objct_num].Contract(B_OBJECT);
		//object_img[del_objct_num].Expand(B_OBJECT);


		//std::cout <<  "下から" << pos << "番目を消す" << std::endl;
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				lay.val(i,j).erase(lay.val(i,j).begin() + pos);//レイヤー情報の削除処理　
				lay.m_val(i,j).erase(lay.m_val(i,j).begin() + pos);//マスク情報の削除
			}
		}

#if 0     //持ち去り時の例外処理(片山氏修論4章より)保存した背景の場合 ピクセルの量が100以上のときはレイヤーに情報を追加
		  //ただ継承されるたんびにこの処理をすることをしなくなってしまった可能性があるので直す必要がある(川本)

		if(ex_pixel > 100){
			//std::cout <<  "例外処理をします" << std::endl;
			for(int j=0; j < Height; j++){
				for(int i=0; i < Width; i++){
					//if(object_img[del_objct_num].isDiff(i,j) == B_OBJECT){
					if(object_img[pos].isDiff(i,j) == B_OBJECT){
						//テクスチャの情報を追加
						int val = object_img[pos].R(i,j);
						lay.val(i,j).insert(lay.val(i,j).begin() + pos,val);
						int mask_id = l_map2[pos+1];
						lay.m_val(i,j).insert(lay.m_val(i,j).begin() + pos,mask_id);
					}
				}
			}

		}//例外処理end
#endif
		lay.erase_layer(delete_key);
		delete_flag = true;
	}

	//消すオブジェクトのID
	//fprintf(fl,"%d,",delete_key);

	//
	////++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	////物体領域追跡による解釈
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
	//マスク画像をシーンに追加
	//何も考えずにvector型にpush_backで追加 insert
	//
	//要素は時刻やフレーム数にしておけばあとで検索に使える？かも
	if(delete_flag == true){
		//fprintf(fl,"-1,");
        laydata--;//kawa
	}
	else {//持ち込みの場合
		std::cout <<  "レイヤ追加" << std::endl;
		for(int j=0; j < Height; j++){
			for(int i=0; i < Width; i++){
				//ある画素がオブジェクトだったらその要素の値を入れる
				if(input.isDiff(i,j) == OBJECT){
					//テクスチャの情報を追加
					int val = input.Y(i,j);//濃淡値
					lay.add(i,j,val);

					//マスクの情報を追加
					//lay.add_mask(i,j,obj_id);//kawa     物体固有ならkeyidほかに同一の場合はobj_idを使ってる?
					lay.add_mask(i,j,key_id);


					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = input.Y(i,j);//RGBに濃淡データを代入
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)];
				}
				//それ以外は何も無し -1 を入れる
				else{
					lay.add(i,j,-1);
					lay.add_mask(i,j,-1);

					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+1] = 0;
					obj->imageData[(obj->widthStep*(obj->height-j-1)+i*3)+2] = 0;
				}
			}
		}
		//オブジェクトのIDを追加
	lay.set_layer(key_id);//l_arreyにIDをpushする
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
	//一番上から見たオブジェクトの重なりを表示
	//上に物が重なることによって形が変わってしまう件の処理
	//
	//自分よりあとに追加されたオブジェクトに注目する

//	int num = lay.get_layer(lay.get_end())+1;//レイヤの最大数
//	int l_max = lay.get_layer(lay.get_end())+1;
//	if(l_max==-1){l_max=0;}//kawa
	int num = laydata;//レイヤの最大数
	int l_max = laydata;



	//LayNum = num;


	//出力画像の初期化 //kawa
//	for(int i = 1;i <= 9;i++){//kawa
//		mask_img[i].init();
//		object_img[i].init();
//	}

	//mask_img object_img vecte化するならここで増やす

	clearobject_img();
    mask_img.clear();
	
	resizeobject_img(l_max);
    mask_img.resize(l_max);
    std::cout <<  "object内部" <<object_img.size()<< std::endl;
	std::cout <<  "mask内部" <<mask_img.size()<< std::endl;
	


	
	output.init();
	output_val.init();

	//レイヤ情報のmapも更新前に初期化
	l_map.clear();
	l_map2.clear();

	if(num!=-1){//レイヤー初期状態以外の時
		no_exist = false;//物体オブジェクトが存在する
		std::vector<int> all = lay.get_layer_all();
		std::vector<int>::reverse_iterator itr = all.rbegin();


		//mapを最新状態に更新 update
		//物体ID→レイヤ
		//レイヤ→物体ID
		//の双方向にアクセスできるmapを作っておく(意味があるかは不明)
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
//ここで画像面からの見え差分の下準備上から見た値とそれぞれの持ち込み時のオブジェクトの差分を変数に代入

	//１画素の値だけでは何層目かは分からない
	//２値画像の時はテクスチャの値が全てオブジェクトIDになっていたので簡単
	//↓
	//しかし、これを変更しないといけない 1/25/2007

	//一応topビューの画像のはず!! output
	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//end() からbegin()まで辿る = 上から見たようになる
			int val = lay.end_val(i,j);//kawa　-1　が戻ってくる可能性がある。?
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

#if 1//上から見た物体の重なりをレイヤごとに色を分けて生成する

	//最大の値、最大レイヤから1画素ごとに下の層を遡る
	//自分より下のオブジェクトに対してupdate
	//一番最下層まで黒く塗りつぶす
	//自分が第何層か知っていれば一番下(第0層)まで塗りつぶせる
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

  for(int j=0; j <lay.get_layer(lay.get_end()); j++){cout<<"laynaiID:"<<j<<"内部"<<lay.returnl_array(j)<<endl;}


	for(int j=0; j < Height; j++){
		for(int i=0; i < Width; i++){
			//この順番で見ると上から見たようになる
			for(std::vector<int>::reverse_iterator it = lay.m_val(i,j).rbegin();it != lay.m_val(i,j).rend();it++)
			{		
				    index=0;
					if(lay.get_layer(*it)!=-1){
					if(*it != -1){
						laynum++;
						
						//オブジェクトIDがもつレイヤをget

						if(laynum==1){//kawa?
							
							index = lay.get_layer(*it);//key_idを返す
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
#if 0 //レイヤ画像の保存
	char fname2[30];
	sprintf(fname2,"%08lu-out2.png",frame);
	det.save(fname2);
#endif
#endif
	cout << __FUNCTION__ << " l_max:" << l_max << endl;
	//=============================================================
	//レイヤごとのテクスチャと上から見たマスクとのANDをとる
	//↓
	//次状態でのマッチングに使用する物体の予想された形＆テクスチャになる
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
		//レイヤ→物体の対応ファイル作成
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

#if 0  //結果画像出力----------------------------//
	//あとで一般化する
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
	//順番が関係ないなら(mask_img1～9やったあとにobject1～9とやる必要がないなら)上記に組み込んでもよい(未検証)
	for(int i=1;i<10;i++){
		sprintf(filenameObject,"obj-%30d.png",i);
		object_img[i].save(filenameObject);
	}
#endif


	/*
	* 影のIDを追加
	*/
	
	//if(interpre_flag == true)
	fprintf(fl,"%d\n",0);
	fclose(fl);
	//else
	//	fprintf(fl,"%d",1);


	//全てのデータを書いたので改行して次の物体画像の入力を待つ

	//ダイアログのLOGに検知状況を出力+++++++++++++++++++++++++++++++++++++++++++++++

	/*
	* 時間取得
	*「検知時刻:」の部分に表示されるもの
	*/

	char* hour = Util::string2Char(Util::Time().getCurrentTimeString("%H").c_str());
	char* minute = Util::string2Char(Util::Time().getCurrentTimeString("%M").c_str());

	//シーン・物体ID出力
	if(delete_flag==true){
		//持ち去り
		//pParent2->SetScene(delete_key,frame,posCenter.getX(),posCenter.getY(),hour,minute,!delete_flag,track_flag);//kawa
		pParent2->SetScene(obj_id,frame,posCenter.getX(),posCenter.getY(),hour,minute,!delete_flag,track_flag);//持ち込み出力の問題
	}else{
		//持ち込み
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



	//char*からMatへ
	cv::Mat imgDet = Util::convertBmpChar2Mat(det);
	//int dx[] = {0,1,0,-1};
	//int dy[] = {1,0,-1,0};
	ObjectInfo objectInfo;
	PII posIdOutput(20,imgDet.rows - 20);

	EventInfo eventInfo = getDetectedObjectInfomation();//情報のよみこ見
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
	//登録してあるイベント情報を元にレイヤ画面にidとイベント検知部位からのラインを表示する

	for(std::vector<ObjectInfo>::iterator itr = eventInfo.bringInObjList.begin();itr != itrEnd; itr++){
		Geometory::Point2D posObj = itr->getPosCenter();
		int id = itr->getID();
		cv::circle(imgDet, cv::Point(posObj.getX(), imgDet.rows - posObj.getY()), 3, cv::Scalar(255,0,255));
		//最新のidなら黄色(0,255,255)、既存のidなら紫色(255,0,255)で塗る
		cv::Scalar paintColor = (id == eventInfo.bringInObjList.getLatestDetectedObjId() ? cv::Scalar(0,255,255) : cv::Scalar(255,0,255));
		cv::line(imgDet, cv::Point(posObj.getX(), imgDet.rows - posObj.getY()), cv::Point(posIdOutput.first, imgDet.rows - posIdOutput.second), paintColor);
		cv::putText(imgDet, ("id:" + Util::toString(id)).c_str(), cv::Point(posIdOutput.first, imgDet.rows - posIdOutput.second), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,0), 1.5, CV_AA);
		posIdOutput.first += 35;//表示位置を20(適当)ずつずらす
	}
	//Matからchar*へ
	Util::convertMat2BmpChar(imgDet, det);
	/*int numn = lay.layer_size(200,90);
	char msg[10];
	itoa(numn,msg,10);
	pParent->Disp_Log(msg);*/
	// TODO: レイヤー情報出力
	pParent2->DetectOut();
	pParent2->LayerOut(&det.B(0,0));
	//階層も出力すべき？？？

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
	return 0;
}

//TODO:物体検知の閾値
bool Cinterpretation::check(image &now, bmp &pre)//input object_img
{
	int x1,y1,x2,y2;
	long same_pix = 0;//同じ場所の画素
	long now_num  = 0; //現在の差分の数
	long pre_num =0; //pre物体の面積
	long total_num = 0; //OR
	int imageHeight = now.getheight();
	int imageWidth = now.getwidth();
	
	//############################################
	//注目したラベルの領域について
	//その領域に過去にも同じ大きさくらいの同じラベルが
	//あったのか無かったのか調べる
	//あれば　＋１　無ければ　－１
	//１つ前のフレームと物体領域について画素の数を調べる


	//cout << "[" << __FUNCTION__ << "]"  << "pre.new_objnum: " << pre.getNewObjNum() << endl;
	//TODO:"AND / OR が一定割合以上なら"に書き変える


		//レイヤ→物体の対応ファイル作成


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
	//閾値以内なら物体検知したことにする
	if(same_pix > hold){ return true; }
#else 
	//pre.isDiff(i,j) == OBJECTはなし
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
	//見つからなかった時
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

//持ち込みチェック
//TODO:持ち込みチェック
bool Cinterpretation::in_obj_tracking(image &first, unsigned long now_frame,std::vector<Pair*> pair,unsigned long leaveframe)
{
	IplImage *input2;
	char file[128];
	char tmp[10];

	//unsigned long i = now_frame-1;
	unsigned long i = search_file(now_frame);//最後のフレーム
	bool eflag = false;
	bool mflag = false;

	int x1, y1, x2, y2;
	
	int xs,ys,xl,yl;

	//CamShift関係+++++++++++++++++++++++++++++++++++++++++++++++++++
	IplImage *image=0,*hsv=0,*hue=0,*mask=0,*backproject=0, *back;
	CvHistogram *hist=0;

	int track_object=0;
	CvRect selection;//領域x,y,width,height
	CvRect track_window;
	CvConnectedComp track_comp;//連結成分 /* セグメント化された連結成分の面積 */ /* セグメント化された連結成分のグレースケール値 *//* セグメント化された連結成分のROI */
	int hdims=16;
	float hranges_arr[]={0,180};
	float *hranges = hranges_arr;
	int vmin=10,vmax=256,smin=30;
	CvBox2D track_box;//回転が考慮された2次元の箱

	//追加（対象物体の周りを考慮）
	CvRect selection2;
	IplImage *hue2=0, *mask2=0;
	CvHistogram *hist2=0, *hist_original=0;
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if(pair.size()==0)
	{cout<<"持ち去りのデータがありません"<<endl;
		return false;}
	for(int c = 0; c < (int)pair.size(); c++){//持ち去り領域
		x1 = pair[c]->get_x();
		y1 = 240 - pair[c]->get_y();
		x2 = pair[c]->get_vx();
		y2 = 240 - pair[c]->get_vy();
	}


	//過去に遡って追跡
	while(1){
		//初期値の設定
		unsigned long frame_human = search_file(now_frame);
		cout << "    " << i << ":" << frame_human << endl;
		if(frame_human == 0) {
			cerr <<"frame human break" << endl;
			break;
		}
		if(i == frame_human){
			Area o_area = first.get_area();
			selection.x = o_area.s_w;//持ち込み領域
			selection.y = 240 - o_area.e_h;
			selection.width=o_area.e_w-o_area.s_w;
			selection.height=o_area.e_h-o_area.s_h;
			track_object=-1;
		
		}

		//画像読み込む
		/*if(mode==ONLINE){
			sprintf_s(file,"\\\\Proliant2/okamoto/Human-DB/Original/");
		}else if(mode==OFFLINE){*/
//			sprintf_s(file,"../DB/Human-DB/Original/");
		//}
		cout << "now_frame:" << i << endl;
		sprintf_s(file, "../DB/Human-DB/Original/%lu/%08lu.%s", search_folder(now_frame), i, "png");

		//画像がない
		if((input2 = cvLoadImage(file,CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR)) == NULL){
			if(eflag==true && i < search_folder(now_frame)){ //終了
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

		//最初の領域確保
		if( !image ){
			image = cvCreateImage( cvSize(320,240), 8, 3 );
			cvZero(image);
			image->origin = input2->origin;
			hsv = cvCreateImage( cvGetSize(image), 8, 3 );
			hue = cvCreateImage( cvGetSize(image), 8, 1 );
			mask = cvCreateImage( cvGetSize(image), 8, 1 );
			backproject = cvCreateImage( cvGetSize(image), 8, 1 );
			hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );  //ヒストグラムの作成

			hue2 = cvCreateImage( cvGetSize(image), 8, 1 );
			mask2 = cvCreateImage( cvGetSize(image), 8, 1 );
			hist2 = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
			hist_original = cvCreateHist(1,&hdims,CV_HIST_ARRAY,&hranges,1);
			track_window = selection;
		}

		//読み込んだ画像をimageにコピーし、HSV表色系に変換してhsvに格納
		cvCopy(input2,image,NULL);
		cvCvtColor(image,hsv,CV_BGR2HSV);

		////hsv画像の各画素が値の範囲内に入っているかチェックし、マスク画像を作成
		/*cvInRangeS(hsv,cvScalar(0,50,50,0),
					cvScalar(180,256,200,0),mask);*/

		//hsv画像のうち、Hチャンネルをhueとして分離
		cvSplit(hsv,hue,0,0,0);

		//差分画像の読み込み
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

		//マスク調整
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

		//追跡領域のヒストグラム計算
		if(track_object<0){  //追跡開始
			float max_val; //ヒストグラムの最大頻度

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



			//近傍の対象外のヒストグラム計算
			//selection2.x = selection.x-around;
			//selection2.y = selection.y-around;
			//selection2.width = selection.width+around*2;
			//selection2.height = selection.height+around*2;
			cvCopy(hue,hue2,0);//hue->originalのhの値
			cvCopy(mask,mask2,0);//mask1->processの0、255の値
			//背景計算のため、対象領域の周りのみ計算するようにマスク調整
			for(int k=selection2.y;k<selection2.y+selection2.height;k++){	if(k==240){break;}//kawa
				for(int l=selection2.x;l<selection2.x+selection2.width;l++){if(l==320){break;}//kawa
					if(l>=selection.x && l<=selection.x+selection.width && 
						k>=selection.y && k<=selection.y+selection.height)
							mask2->imageData[(k*mask2->width+l)] = 0;
					else
						mask2->imageData[(k*mask2->width+l)] = 255;
				}
			}
			//対象物体の周辺のヒストグラムの計算
			cvSetImageROI(hue2,selection2);//(画像, 矩形情報)
			cvSetImageROI(mask2,selection2);
			cvCalcHist(&hue2,hist2,0,mask2);//画像（群）のヒストグラムを計算する,(入力画像群（ CvMat** 形式でも構わない）．全て同じサイズ・タイプ．,ヒストグラムへのポインタ． ,処理マスク．入力画像中のどのピクセルをカウントするかを決定する．)

			cvResetImageROI(hue2);
			cvResetImageROI(mask2);

			//対象物体のヒストグラム計算
			cvSetImageROI(hue,selection);
			cvSetImageROI(mask,selection);
			cvCalcHist(&hue,hist,0,mask);

			//対象物体の最初のヒストグラムを記憶
			cvCopyHist(hist,&hist_original);

			cvGetMinMaxHistValue(hist,0,&max_val,0,0);//対象物体の最大値，最小値を持つビンを求める
			cvConvertScale( hist->bins, hist->bins, max_val ? 255. / max_val : 0., 0 );//配列に対して任意の線形変換を行います.最大値順に直す？
			cvGetMinMaxHistValue( hist2, 0, &max_val, 0, 0 );//周辺領域
			cvConvertScale( hist2->bins, hist2->bins, max_val ? 255. / max_val : 0., 0 );

//-----------------------------------------------------------------------------------
			for(int k=0;k<hdims;k++){//hdims=16
				double re = cvGetReal1D(hist->bins,k)-(cvGetReal1D(hist2->bins,k));
				if(re<0)	re=0;
				cvSetReal1D(hist->bins,k,re);
			}
//-----------------------------------------------------------------------------------

			cvGetMinMaxHistValue(hist,0,&max_val,0,0);
			//ヒストグラムの縦軸(頻度)を0-255のダイナミックレンジに正規化
			if(max_val==0.0)
				cvConvertScale(hist->bins,hist->bins,0.0,0);
			else
				cvConvertScale(hist->bins,hist->bins,255.0 / max_val,0);
			//hue,mask画像に設定されたROIをリセット
			cvResetImageROI(hue);
			cvResetImageROI(mask);
			track_window = selection;//
			track_object=1;
		}//if(track_object<0){  //追跡開始



		//近傍の対象外のヒストグラム計算
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
		//背景計算のため、対象領域の周りのみ計算するようにマスク調整
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
		//ヒストグラムの縦軸(頻度)を0-255のダイナミックレンジに正規化
		if(max_val==0.0)
			cvConvertScale(hist->bins,hist->bins,0.0,0);
		else
			cvConvertScale(hist->bins,hist->bins,255.0 / max_val,0);

		//バックプロジェクションを計算する
		cvCalcBackProject(&hue,backproject,hist);//(入力画像群（ CvMat** 形式でも構わない）．すべて同一サイズ，同一タイプ、出力のバックプロジェクション画像．入力画像群と同じタイプ,ヒストグラム．)
		cvAnd(backproject,mask,backproject,0);
		sprintf_s(file,"../backproject/");
		sprintf_s(tmp,"%08lu",i);
		strcat_s(file,tmp);
		strcat_s(file,".png");
		cvSaveImage(file,backproject);
		
		//CamShiftによる追跡(オブジェクトヒストグラムのバックプロジェクション （cvCalcBackProjectを参照）．初期探索ウィンドウ．結果として得られる構造体．収束した探索ウィンドウの座標（comp->rect フィールド），およびウィンドウ内の全ピクセルの合計値（comp->area フィールド）が含まれる． オブジェクトの外接矩形．NULLでない場合，オブジェクトのサイズと姿勢が含まれる．   )
		int res = cvCamShift( backproject, track_window,
				 cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
				 &track_comp, &track_box );
		cout << "res:" << res <<";track_box:"<<track_box.angle<<","<<track_box.center.x<<","<<track_box.center.y<<","<<track_box.size.height<<","<<track_box.size.width<< endl;
		//camshift追跡に失敗した場合終了
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
		{cvEllipseBox( image, track_box, CV_RGB(255,0,0), 3, CV_AA, 0 );}//極小の場合書かずにスルーさせる


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

		//ウィンドウの生成
		cvNamedWindow("object-tracking",1);
		cvMoveWindow("object-tracking",600,480);

		//結果保存
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

		//画像表示
		cvShowImage("object-tracking",image);
		cvWaitKey(1);

		if(i==search_file(now_frame)){
			cvSaveImage("result.png",image);
		}

		i--;
		eflag = false;
		//終了判定(持ち込みと判定されれば終了)

		//もしくは、完全にだめだと分かったら終了
		if(i == search_folder(now_frame) + 2){
			cerr << "search folder break" << endl;
			break;
		}

		//最後の持ち去りのフレーム地点まで遡れば終了//kawa(数値部分は正確な持ち去り時の誤差)
		if(i==(leaveframe-19))
		{   
			cerr<<"Last leave frame "<<endl;
		    cout<<"最後の持ち去り地点まで戻りました。"<<endl;
			break;
		}
		//持ち込みと持ち去りのあいだで人物が途中で途切れている場合追跡不可//kawa
		if(leaveframe<search_folder(now_frame))
		{ mflag = false;
		cvDestroyWindow("object-tracking");
		  cerr<<"Last human frame "<<endl;
		  cout<<"最近の持ち込みと持ち去りに同一の人物が関連していない可能性があります。"<<endl;
		  return false;
		 }


	}//while(1){
	//ウィンドウの破棄
	cvDestroyWindow("object-tracking");
	if(mflag)cout << "end" << endl;
	return mflag;
}