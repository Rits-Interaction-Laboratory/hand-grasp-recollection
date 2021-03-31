#include "opencv2\opencv.hpp"
#include "picojson.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <string>
#include "image.h"

using namespace std;
using namespace cv;

namespace ImageEditUtil{

	void drawSkeleton( image &input, const string jsonBodyInfo,  const int radius = 1, const int thickness = 2 )
	{
		picojson::value v;
		std::string err;
		picojson::parse( v, jsonBodyInfo.begin(), jsonBodyInfo.end(), &err );
		
		input.track_skeleton.clear();
		
		std::vector<std::vector<PII>> skeletons;
		cv::Mat dst;
		int resoX = input.Width(); int resoY = input.Height(); int magnification = 1920 / resoX;

		input.img_original.copyTo(dst);	//					 1960
		//int x=(1920/2)-640;			//  		 |￣￣￣￣￣￣￣￣￣|  P(x, y)
		//int y=(1080/2)-480;			//  		 | 　 P￣￣￣￣|	|
										//		1080 | 960|  1280  |	|
										//			 |	　|＿＿＿＿|	|
										//			 |＿＿＿＿＿＿＿＿＿|
		if ( err.empty() )
		{
			picojson::object &objBodyInfo = v.get< picojson::object >();
			std::string version = objBodyInfo["version"].get< std::string >();

			picojson::array arrayBody = objBodyInfo["bodies"].get< picojson::array >();
			int numHuman = 0;
			for ( auto itrBody = arrayBody.begin() ; itrBody != arrayBody.end() ; ++itrBody )
			{
				picojson::object &objBody = itrBody->get< picojson::object >();
				if ( objBody["isTracked"].get< bool >() )
				{
					picojson::array arrayJoint = objBody["Joints"].get< picojson::array >();
					// cout << "arrayJoint: " << arrayJoint.size() << endl;
					std::vector<PII> skeleton;
					int cc=0;					// 枠外に出てしまっている関節x座標の数？　(コメント書いてよ...)
					int xx=0;					// 枠外に出てしまっている関節y座標の数？
					int tt=0;					// 送られてきた骨格情報の関節の数？
					int sumX = 0, sumY = 0;
					double aveX = 0, aveY = 0;
					for( auto itrJoint = arrayJoint.begin() ; itrJoint != arrayJoint.end() ; ++itrJoint )
					{
						picojson::object &objJoint = itrJoint->get< picojson::object >();
						picojson::object &objPositionColorSpace = objJoint["PositionColorSpace"].get< picojson::object >();
						int X= (int)( objPositionColorSpace["X"].get<double>() / magnification );// - x;
						int Y= (int)( objPositionColorSpace["Y"].get<double>() / magnification );// - y;	// フルHD骨格情報⇒1/4サイズ骨格情報⇒320*240サイズ骨格情報
						
						if(X<0){				// 320*240枠からはみ出した場合(X左側)?
							X=0;
							cc++;
						}
						else if(X>=1920){		// 320*240枠からはみ出した場合(X右側)?
							X=1920;
							cc++;
						}
						if(Y<0){				// 320*240枠からはみ出した場合(Y上側)?
							Y=0;
							xx++;
						}
						else if(Y>=1080){		// 320*240枠からはみ出した場合(Y下側)?
							Y=1080;
							xx++;
						}
						tt++;
						sumX += X; sumY += Y;

						// cout << "X: " << X << ", Y: " << Y << endl;

						// if(X<280)
						// {
						if(skeleton.size() == 22 || skeleton.size() == 24 )									// 22個目の関節の時、24個目の関節の時(たしか、右手と左手？)
						{																			// サイズで判定するのはどうなのか？順番じゃなかった場合・・・
							cv::circle( dst, cv::Point( X, Y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4);	// 青丸
						}
						else if( skeleton.size() == 5 || skeleton.size() == 9 || skeleton.size() == 1 )
						{
							cv::circle( dst, cv::Point( X, Y ), radius, cv::Scalar( 200, 0, 200 ), thickness );
						}
						else
						{																						// ↑以外の関節の時
							cv::circle( dst, cv::Point( X, Y ), radius, cv::Scalar( 0, 200, 200 ), thickness );	// くすんだ黄色で丸
						}
						skeleton.push_back(PII(X,Y));
						// }	
					}
					/*
					// 関節が25箇所全て取得できないと人物として認めないはやり過ぎだと思うのでコメントアウト
					if(cc<tt-1&&xx<tt-1&&skeleton.size()>24)
					{
						//stringstream aa;
						//aa<<"sID:"<<skeletons.size();
						//cv::putText(dst, aa.str(),cv::Point(skeleton[0].first+10,skeleton[0].second+10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
						//
						long long  a=objBody[ "trackingID" ].get<double>();
						input.track_skeleton.insert( map<int, long long>::value_type(skeletons.size(),a));
						skeletons.push_back(skeleton);
					}
					*/
					if ( cc < tt - 1 && xx < tt - 1 && skeleton.size() > 12 )										// 取得できた関節数 - 1より、枠外だった関節数が小さい
					{																						// かつ、取れた関節数が12超
						//double  a = objBody["trackingID"].get<double>();
						//input.track_skeleton.insert(map<int, long long>::value_type(skeletons.size(), (long long)a));	// track_skeleton に skeletons のサイズと trackingID の値 を挿入
						skeletons.push_back(skeleton);														// vec<vec<pair(int, int)>> skeletons に、skeleton(trackできている人体の骨格情報が入った配列) をプッシュ
						numHuman++;
						aveX = (double)sumX / (double)tt;
						aveY = (double)sumY / (double)tt;
						putText( dst, "human: " + to_string( numHuman ), Point( (int)aveX, (int)aveY ), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0,0,255), 2, CV_AA);

					}																						
					// cout << "skeleton: " << skeleton.size() << endl;
				}
			}
		}
		else
		{
			std::cerr << __FUNCTION__ << ":" << err << std::endl;
		}
		if( skeletons.size() ){
			cout << skeletons.size() << "(skeletons.size())!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		}
		// cout << "skeletons: " << skeletons.size() << endl;
		input.set_skeletons() = skeletons;
		input.img_coverSkeletonColor = dst.clone();
	}	

	void drawOpenPoseSkeleton( image &input, const string jsonBodyInfo,  const int radius = 1, const int thickness = 2 )
	{
		picojson::value v;
		std::string err;
		picojson::parse( v, jsonBodyInfo.begin(), jsonBodyInfo.end(), &err );

		std::ofstream JSONtoCSV;			//
		//outputID << HumanOpenPoseID << "(HumanOpenPoseID)";																						//
		//outputID.close();

		input.track_skeleton.clear();
		
		//std::vector<std::map<std::string, std::vector<PII>>> skeletonSets2D;
		//std::vector<std::map<std::string, std::vector<PIII>>> skeletonSets3D;
		//std::vector<PII> poseSkeleton2D;
		//std::vector<PII> faceSkeleton2D;
		//std::vector<PII> RhandSkeleton2D;
		//std::vector<PII> LhandSkeleton2D;
		//std::vector<PIII> poseSkeleton3D;
		//std::vector<PIII> faceSkeleton3D;
		//std::vector<PIII> RhandSkeleton3D;
		//std::vector<PIII> LhandSkeleton3D;

		std::vector<std::map<std::string, std::vector<point2D>>> skeletonSets2D;
		std::vector<std::map<std::string, std::vector<point3D>>> skeletonSets3D;
		std::vector<point2D> poseSkeleton2D;
		std::vector<point2D> faceSkeleton2D;
		std::vector<point2D> RhandSkeleton2D;
		std::vector<point2D> LhandSkeleton2D;
		std::vector<point3D> poseSkeleton3D;
		std::vector<point3D> faceSkeleton3D;
		std::vector<point3D> RhandSkeleton3D;
		std::vector<point3D> LhandSkeleton3D;
		cv::Mat dstColor;
		cv::Mat dstDepth;
		cv::Mat dstDepth8U;
		int resoX = input.Width(); int resoY = input.Height(); int magnification = 1920 / resoX;

		input.img_original.copyTo(dstColor);
		input.img_original_depth.copyTo(dstDepth);
		input.img_original_depth8U.copyTo(dstDepth8U);
										//					 1960
		//int x=(1920/2)-640;			//  		 |￣￣￣￣￣￣￣￣￣|  P(x, y)
		//int y=(1080/2)-480;			//  		 | 　 P￣￣￣￣|	|
										//		1080 | 960|  1280  |	|
										//			 |	　|＿＿＿＿|	|
										//			 |＿＿＿＿＿＿＿＿＿|
		//OpnePoseの関節番号と関節の名前
		//Pose(BODY_25)					Face2D											 Hand
		//{0,  "Nose",       "鼻"}		{0,  "右顳顬"}	{26, "左眉尻"}   {52, "左唇山"}	 {0,  "小指球"}
		//{1,  "Neck",       "首"}		{1,  "右靨"}	{27, "眉間"}     {53, "上唇左外"}{1,  "拇指球"}
		//{2,  "RShoulder",  "右肩"}	{2,  "右頬上"}	{28, "鼻根"}     {54, "左口角外"}{2,  "親指基節"}
		//{3,  "RElbow",     "右肘"}	{3,  "右頬中"}	{29, "鼻背"}     {55, "下唇左外"}{3,  "親指末節"}
		//{4,  "RWrist",     "右手首"}	{4,  "右頬下"}	{30, "鼻尖"}     {56, "下唇山左"}{4,  "親指先"}
		//{5,  "LShoulder",  "左肩"}	{5,  "右顎上"}	{31, "右小鼻"}   {57, "下唇山中"}{5,  "人差し指基底"}
		//{6,  "LElbow",     "左肘"}	{6,  "右顎中"}	{32, "右鼻腔"}   {58, "下唇山右"}{6,  "人差し指中節"}
		//{7,  "LWrist",     "左手首"}	{7,  "右顎下"}	{33, "鼻柱"}     {59, "下唇右外"}{7,  "人差し指末節"}
		//{8,  "MidHip",     "尻の中心"}{8,  "顎先"}	{34, "左鼻腔"}   {60, "右口角内"}{8,  "人差し指先"}
		//{9,  "RHip",       "右尻"}	{9,  "左顎下"}	{35, "左小鼻"}   {61, "上唇右内"}{9,  "中指基底"}
		//{10, "RKnee",      "右膝"}	{10, "左顎中"}	{36, "右目尻"}   {62, "上唇中内"}{10,  "中指中節"}
		//{11, "RAnkle",     "右足首"}	{11, "左顎上"}	{37, "右上瞼外"} {63, "上唇左内"}{11,  "中指末節"}
		//{12, "LHip",       "左尻"}	{12, "左頬下"}	{38, "右上瞼内"} {64, "左口角内"}{12,  "中指先"}
		//{13, "LKnee",      "左ひざ"}	{13, "左頬中"}	{39, "右目頭"}	 {65, "下唇左内"}{13,  "薬指基底"}
		//{14, "LAnkle",     "左足首"}	{14, "左頬上"}	{40, "右下瞼内"} {66, "下唇中内"}{14,  "薬指中節"}
		//{15, "REye",       "右目"}	{15, "左靨"}	{41, "右下瞼外"} {67, "下唇右内"}{15,  "薬指末節"}
		//{16, "LEye",       "左目"}	{16, "左顳顬"}	{42, "左目頭"}	 {68, "右瞳"}	 {16,  "薬指先"}
		//{17, "REar",       "右耳"}	{17, "右眉尻"}	{43, "左上瞼内"} {69, "左瞳"}	 {17,  "小指基底"}
		//{18, "LEar",       "左耳"}	{18, "右眉外"}	{44, "左上瞼外"} {70, "背景"}	 {18,  "小指中節"}
		//{19, "LBigToe",    "左足親指"}{19, "右眉中"}	{45, "左目尻"}					 {19,  "小指末節"}
		//{20, "LSmallToe",  "左足小指"}{20, "右眉内"}	{46, "左下瞼外"}				 {20,  "小指先"}
		//{21, "LHeel",      "左踵"}	{21, "右眉頭"}	{47, "左下瞼内"}				 {21,  "背景"}
		//{22, "RBigToe",    "右足親指"}{22, "左眉頭"}	{48, "右口角外"}	
		//{23, "RSmallToe",  "右足小指"}{23, "左眉内"}	{49, "上唇右外"}	
		//{24, "RHeel",      "右踵"}	{24, "左眉中"}	{50, "右唇山"}	
		//{25, "Background", "背景"}	{25, "左眉外"}	{51, "人中溝"}	
		if ( err.empty() )
		{
			picojson::object &objOpenPoseKeypointsJson = v.get< picojson::object >();
			double version = objOpenPoseKeypointsJson["version"].get< double >();
			picojson::array arrayPeople = objOpenPoseKeypointsJson["people"].get< picojson::array >();
			int numHuman = 0;
			int personID = 0;
			for ( auto itrPeople = arrayPeople.begin() ; itrPeople != arrayPeople.end() ; ++itrPeople )
			{
				std::map<std::string, std::vector<point2D>> typeSets2D;
				std::map<std::string, std::vector<point3D>> typeSets3D;
				int sumPoseX = 0; int sumPoseY = 0; int numPosePoints = 0;
				double aveX = 0.; double aveY = 0.;

				//ログ用
				JSONtoCSV.open( ( input.eventID_logs_json_dirPath + "\\json_pose2D_person" + to_string(personID).c_str() + "_in_" + input.saveFrame + ".csv" ).c_str(), ios::out );
				
				picojson::object objPerson = itrPeople->get< picojson::object >();
				picojson::array arrayPose2D = objPerson["pose_keypoints_2d"].get< picojson::array >();
				if( arrayPose2D.size() > 0 ){
					for( auto itrJoints = arrayPose2D.begin(); itrJoints != arrayPose2D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;	//keypointのx座標
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;	//keypointのy座標　※推定確率は読み飛ばす　⇒ 読み飛ばさない
						double a = ( itrJoints -> get<double>() );
						point2D point = input.make_point2D( x, y, a );
						poseSkeleton2D.push_back( point );
						JSONtoCSV << x << "," << y << "," << a << endl;
						if( ( itrJoints -> picojson::value::get<double>() ) < 0.3 ) continue;		//Accuracyが30%未満の場合は描画しない
						if( poseSkeleton2D.size() == 5 || poseSkeleton2D.size() == 8 )				// 5個目の関節の時、8個目の関節の時(右手首と左手首)
						{																			// サイズで判定するのはどうなのか？順番じゃなかった場合・・・
							cv::circle( dstColor, cv::Point( x, y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4 );	// 青丸
							cv::circle( dstDepth, cv::Point( x, y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4 );
							cv::circle( dstDepth8U, cv::Point( x, y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4 );
						}
						else if( poseSkeleton2D.size() == 4 || poseSkeleton2D.size() == 7 || poseSkeleton2D.size() == 2 )
						{
							cv::circle( dstColor, cv::Point( x, y ), radius, cv::Scalar( 200, 0, 200 ), thickness );
							cv::circle( dstDepth, cv::Point( x, y ), radius, cv::Scalar( 200, 0, 200 ), thickness );
							cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 200, 0, 200 ), thickness );
						}
						else
						{																						// ↑以外の関節の時
							cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 0, 200, 200 ), thickness );	// くすんだ黄色で丸
							cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 0, 200, 200 ), thickness );
							cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 0, 200, 200 ), thickness );
						}
						sumPoseX += x; sumPoseY += y; numPosePoints ++;
					}
				}
				typeSets2D["pose2D"] = poseSkeleton2D;	//"pose2D"という名前で「全身」のkeypointsの座標セットを挿入
				poseSkeleton2D.clear();

				JSONtoCSV.close();

				picojson::array arrayFace2D = objPerson["face_keypoints_2d"].get< picojson::array >();
				if( arrayFace2D.size() > 0 ){
					for( auto itrJoints = arrayFace2D.begin(); itrJoints != arrayFace2D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;	//keypointのx座標
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;	//keypointのy座標　※推定確率は読み飛ばす　⇒ 読み飛ばさない
						double a = ( itrJoints -> get<double>() );
						point2D point = input.make_point2D( x, y, a );
						faceSkeleton2D.push_back( point );
						if( ( itrJoints -> get<double>() ) == 0 ) continue;
						cv::circle( dstColor, cv::Point( x, y ), radius, cv::Scalar( 200, 200, 0 ), thickness );
						cv::circle( dstDepth, cv::Point( x, y ), radius, cv::Scalar( 200, 200, 0 ), thickness );
						cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 200, 200, 0 ), thickness );
					}
				}
				typeSets2D["face2D"] = faceSkeleton2D;	//"face2D"という名前で「顔」のkeypointsの座標セットを挿入
				faceSkeleton2D.clear();

				JSONtoCSV.open( ( input.eventID_logs_json_dirPath + "\\json_Lhand2D_person" + to_string(personID).c_str() + "_in_" + input.saveFrame + ".csv" ).c_str(), ios::out );

				picojson::array arrayLeftHand2D = objPerson["hand_left_keypoints_2d"].get< picojson::array >();
				if( arrayLeftHand2D.size() > 0 ){
					for( auto itrJoints = arrayLeftHand2D.begin(); itrJoints != arrayLeftHand2D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;
						double a = ( itrJoints -> get<double>() );
						point2D point = input.make_point2D( x, y, a );
						LhandSkeleton2D.push_back( point );				//左手指関節21点のセット⇒LhandSkeleton2D

						JSONtoCSV << x << "," << y << "," << a << endl;
						if( ( itrJoints -> picojson::value::get<double>() ) < 0.11 ) continue;		//Accuracyが30%未満の場合は描画しない
						//if( ( itrJoints -> get<double>() ) == 0 ) continue;
						cv::circle( dstColor, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), thickness );
						cv::circle( dstDepth, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), thickness );
						cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 0, 255, 0 ), thickness );
					}
				}
				typeSets2D["Lhand2D"] = LhandSkeleton2D;	//"Lhand2D"という名前で「左手」のkeypointsの座標セットを挿入
				LhandSkeleton2D.clear();

				JSONtoCSV.close();
				JSONtoCSV.open( ( input.eventID_logs_json_dirPath + "\\json_Rhand2D_person" + to_string(personID).c_str() + "_in_" + input.saveFrame + ".csv" ).c_str(), ios::out );

				picojson::array arrayRightHand2D = objPerson["hand_right_keypoints_2d"].get< picojson::array >();
				if( arrayRightHand2D.size() > 0 ){
					for( auto itrJoints = arrayRightHand2D.begin(); itrJoints != arrayRightHand2D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;
						double a = ( itrJoints -> get<double>() );
						point2D point = input.make_point2D( x, y, a );
						RhandSkeleton2D.push_back( point );
						JSONtoCSV << x << "," << y << "," << a << endl;
						if( ( itrJoints -> picojson::value::get<double>() ) < 0.11 ) continue;		//Accuracyが30%未満の場合は描画しない
						//if( ( itrJoints -> get<double>() ) == 0 ) continue;
						cv::circle( dstColor, cv::Point( x, y ), radius, cv::Scalar( 0, 0, 255 ), thickness );
						cv::circle( dstDepth, cv::Point( x, y ), radius, cv::Scalar( 0, 0, 255 ), thickness );
						cv::circle( dstDepth8U, cv::Point( x, y ), radius, cv::Scalar( 0, 0, 255 ), thickness );
						
					}
				}
				typeSets2D["Rhand2D"] = RhandSkeleton2D;	//"Rhand2D"という名前で「右手」のkeypointsの座標セットを挿入
				RhandSkeleton2D.clear();

				JSONtoCSV.close();

				picojson::array arrayPose3D = objPerson["pose_keypoints_3d"].get< picojson::array >();
				if( arrayPose3D.size() > 0 ){
					for( auto itrJoints = arrayPose3D.begin(); itrJoints != arrayPose3D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;
						int z = (int)( itrJoints -> get<double>() ); itrJoints++;
						double a = ( itrJoints -> get<double>() );
						point3D point = input.make_point3D( x, y, z, a );
						poseSkeleton3D.push_back( point );
					}
				}
				typeSets3D["pose3D"] = poseSkeleton3D;	//"pose3D"という名前で「全身」のkeypointsの座標セットを挿入
				poseSkeleton3D.clear();

				picojson::array arrayFace3D = objPerson["face_keypoints_3d"].get< picojson::array >();
				if( arrayFace3D.size() > 0 ){
					for( auto itrJoints = arrayFace3D.begin(); itrJoints != arrayFace3D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;
						int z = (int)( itrJoints -> get<double>() ); itrJoints++;
						double a = ( itrJoints -> get<double>() );
						point3D point = input.make_point3D( x, y, z, a );
						faceSkeleton3D.push_back( point );
					}
				}
				typeSets3D["face3D"] = faceSkeleton3D;	//"face3D"という名前で「顔」のkeypointsの座標セットを挿入
				faceSkeleton3D.clear();

				picojson::array arrayLeftHand3D = objPerson["hand_left_keypoints_3d"].get< picojson::array >();
				if( arrayLeftHand3D.size() > 0 ){
					for( auto itrJoints = arrayLeftHand3D.begin(); itrJoints != arrayLeftHand3D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;
						int z = (int)( itrJoints -> get<double>() ); itrJoints++;
						double a = ( itrJoints -> get<double>() );
						point3D point = input.make_point3D( x, y, z, a );
						LhandSkeleton3D.push_back( point );
					}
				}
				typeSets3D["Lhand3D"] = LhandSkeleton3D;	//"Lhand3D"という名前で「左手」のkeypointsの座標セットを挿入
				LhandSkeleton3D.clear();

				picojson::array arrayRightHand3D = objPerson["hand_right_keypoints_3d"].get< picojson::array >();
				if( arrayRightHand3D.size() > 0 ){
					for( auto itrJoints = arrayRightHand3D.begin(); itrJoints != arrayRightHand3D.end(); itrJoints++ )
					{
						int x = (int)( itrJoints -> get<double>() ); itrJoints++;
						int y = (int)( itrJoints -> get<double>() ); itrJoints++;
						int z = (int)( itrJoints -> get<double>() ); itrJoints++;
						double a = ( itrJoints -> get<double>() );
						point3D point = input.make_point3D( x, y, z, a );
						RhandSkeleton3D.push_back( point );
					}
				}
				typeSets3D["Rhand3D"] = RhandSkeleton3D;	//"Rhand3D"という名前で「右手」のkeypointsの座標セットを挿入
				RhandSkeleton3D.clear();

				skeletonSets2D.push_back( typeSets2D );  //typeSets2Dのclear忘れてる
				skeletonSets3D.push_back( typeSets3D );  //typeSets3Dのclear忘れてる
				numHuman++;
				personID++;
				aveX = (double)sumPoseX / (double)numPosePoints;
				aveY = (double)sumPoseY / (double)numPosePoints;
				cv::putText( dstColor, "human: " + to_string( numHuman ), Point( (int)aveX, (int)aveY ), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0,0,255), 2, CV_AA);
				cv::putText( dstDepth, "human: " + to_string( numHuman ), Point( (int)aveX, (int)aveY ), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0,0,255), 2, CV_AA);
				cv::putText( dstDepth8U, "human: " + to_string( numHuman ), Point( (int)aveX, (int)aveY ), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0,0,255), 2, CV_AA);
			}
		}
		else
		{
			std::cerr << __FUNCTION__ << ":" << err << std::endl;
		}
		if( skeletonSets2D.size() ){
			cout << skeletonSets2D.size() << "(skeletonSets2D.size())!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		}
		// cout << "skeletons: " << skeletons.size() << endl;
		input.set_skeletons2D() = skeletonSets2D;
		input.set_skeletons3D() = skeletonSets3D;
		input.img_coverSkeletonColor = dstColor.clone();
		input.img_coverSkeletonDepth = dstDepth.clone();
		input.img_coverSkeletonDepth8U = dstDepth8U.clone();
	}
}