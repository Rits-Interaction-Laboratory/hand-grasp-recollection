#pragma
#include "KinectPack.h"
#include "opencv2\opencv.hpp"
#include "picojson.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <string>
#include "Util.h"
#include"image.h"
namespace KinectPackUtil{

	void convertImageToMat( const Image& src, cv::Mat& dst ){
		dst = Util::convertVec2Mat( src.image );
	}
	void convertDepth16UToDepth8U( const cv::Mat& src, cv::Mat& dst ){
		dst = cv::Mat::zeros( src.rows, src.cols, CV_8UC1 );
		for ( int row = 0 ; row < src.rows ; ++row )
		{
			for ( int col = 0 ; col < src.cols ; ++col )
			{
				dst.at< unsigned char >( row, col ) = src.at< short >( row, col ) / 8000. * 255.;
			}
		}
	}
	void drawSkeleton(image &input, const KinectPack kinectPack, const int radius = 1, const int thickness = 2 )
	{
		picojson::value v;
		std::string err;
		//cout << kinectPack.bodies.jsonBodyInfo.c_str() << endl;
		picojson::parse( v, kinectPack.bodies.jsonBodyInfo.begin(), kinectPack.bodies.jsonBodyInfo.end(), &err );
		
		input.track_skelton.clear();
		
		std::vector<std::vector<PII>> skeletons;
		cv::Mat dst;
		input.img_resize.copyTo(dst);	//					 480
		int x=(480/2)-160;				//  		 |￣￣￣￣￣￣￣￣￣|  P(x, y)
		int y=(270/2)-120;				//  		 | 　 P￣￣￣￣|	|
										//		 270 | 240|   320  |	|
										//			 |	　|＿＿＿＿|	|
										//			 |＿＿＿＿＿＿＿＿＿|
		if ( err.empty() )
		{
			picojson::object &objBodyInfo = v.get< picojson::object >();
			std::string version = objBodyInfo["version"].get< std::string >();

			picojson::array arrayBody = objBodyInfo["bodies"].get< picojson::array >();
			for ( auto itrBody = arrayBody.begin() ; itrBody != arrayBody.end() ; ++itrBody )
			{
				picojson::object &objBody = itrBody->get< picojson::object >();
				if ( objBody["isTracked"].get< bool >() )
				{
					picojson::array arrayJoint = objBody["Joints"].get< picojson::array >();
					// cout << "arrayJoint: " << arrayJoint.size() << endl;
					std::vector<PII> skelton;
					int cc=0;					// 枠外に出てしまっている関節x座標の数？　(コメント書いてよ...)
					int xx=0;					// 枠外に出てしまっている関節y座標の数？
					int tt=0;					// 送られてきた骨格情報の関節の数？
					for( auto itrJoint = arrayJoint.begin() ; itrJoint != arrayJoint.end() ; ++itrJoint )
					{
						picojson::object &objJoint = itrJoint->get< picojson::object >();
						picojson::object &objPositionColorSpace = objJoint["PositionColorSpace"].get< picojson::object >();
						int X= (int)( objPositionColorSpace["X"].get<double>() / 4 ) - x;
						int Y= (int)( objPositionColorSpace["Y"].get<double>() / 4 ) - y;	// フルHD骨格情報⇒1/4サイズ骨格情報⇒320*240サイズ骨格情報
						
						if(X<0){				// 320*240枠からはみ出した場合(X左側)?
							X=0;
							cc++;
						}
						else if(X>=320){		// 320*240枠からはみ出した場合(X右側)?
							X=320;
							cc++;
						}
						if(Y<0){				// 320*240枠からはみ出した場合(Y上側)?
							Y=0;
							xx++;
						}
						else if(Y>=240){		// 320*240枠からはみ出した場合(Y下側)?
							Y=240;
							xx++;
						}
						tt++;

						// cout << "X: " << X << ", Y: " << Y << endl;

						// if(X<280)
						// {
						if(skelton.size()==22||skelton.size()==24)									// 22個目の関節の時、24個目の関節の時(たしか、右手と左手？)
						{																			// サイズで判定するのはどうなのか？順番じゃなかった場合・・・
							cv::circle( dst, cv::Point( X, Y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4);	// 青丸
						}
						else
						{																						// ↑以外の関節の時
							cv::circle( dst, cv::Point( X, Y ), radius, cv::Scalar( 0, 200, 200 ), thickness );	// くすんだ黄色で丸
						}
						skelton.push_back(PII(X,Y));
						// }	
					}
					/*
					// 関節が25箇所全て取得できないと人物として認めないはやり過ぎだと思うのでコメントアウト
					if(cc<tt-1&&xx<tt-1&&skelton.size()>24)
					{
						//stringstream aa;
						//aa<<"sID:"<<skeletons.size();
						//cv::putText(dst, aa.str(),cv::Point(skelton[0].first+10,skelton[0].second+10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
						//
						long long  a=objBody[ "trackingID" ].get<double>();
						input.track_skelton.insert( map<int, long long>::value_type(skeletons.size(),a));
						skeletons.push_back(skelton);
					}
					*/
					if (cc < tt - 1 && xx < tt - 1 && skelton.size() > 12 )										// 取得できた関節数 - 1より、枠外だった関節数が小さい
					{																							// かつ、取れた関節数が12超

						//long long a = objBody["trackingID"].get<double>();									//Debug時、trackingIDの値をdouble型で取得できていなかったので
						//↓																					//一旦string型で取得しlong long型に変換(2019/04/05)
						std::string a_str = objBody["trackingID"].get<std::string>();	
						long long a = stoll( a_str.c_str(), nullptr, 10 );
			
						input.track_skelton.insert( map<int, long long>::value_type( skeletons.size(), a ) );	// track_skelton に skeletons のサイズと trackingID の値 を挿入
						skeletons.push_back(skelton);															// vec<vec<pair(int, int)>> skeletons に、
					}																							// skelton(trackできている人体の骨格情報が入った配列) をプッシュ
					// cout << "skelton: " << skelton.size() << endl;
				}
			}
		}
		else
		{
			std::cerr << __FUNCTION__ << ":" << err << std::endl;
		}
		if( skeletons.size() ){
			//cout << skeletons.size() << "(skeletons.size())!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		}
		// cout << "skeletons: " << skeletons.size() << endl;
		imshow("onSkelton", dst);
		cv::waitKey(1);
		input.set_skeltons()=skeletons;
		input.depth_skelton_copyTo(dst);
		input.resize_depth();
	}

	void drawSkeletonFlip(image &input, const KinectPack kinectPack, const int radius = 1, const int thickness = 2 )
	{
		picojson::value v;
		std::string err;
		//cout << kinectPack.bodies.jsonBodyInfo.c_str() << endl;
		picojson::parse( v, kinectPack.bodies.jsonBodyInfo.begin(), kinectPack.bodies.jsonBodyInfo.end(), &err );
		
		input.track_skelton.clear();
		
		std::vector<std::vector<PII>> skeletons;
		cv::Mat dst;
		input.img_resize.copyTo(dst);	//					 480
		int x=(480/2)-160;				//  		 |￣￣￣￣￣￣￣￣￣|  P(x, y)
		int y=(270/2)-120;				//  		 | 　 P￣￣￣￣|	|
										//		 270 | 240|   320  |	|
										//			 |	　|＿＿＿＿|	|
										//			 |＿＿＿＿＿＿＿＿＿|
		if ( err.empty() )
		{
			picojson::object &objBodyInfo = v.get< picojson::object >();
			std::string version = objBodyInfo["version"].get< std::string >();

			picojson::array arrayBody = objBodyInfo["bodies"].get< picojson::array >();
			for ( auto itrBody = arrayBody.begin() ; itrBody != arrayBody.end() ; ++itrBody )
			{
				picojson::object &objBody = itrBody->get< picojson::object >();
				if ( objBody["isTracked"].get< bool >() )
				{
					picojson::array arrayJoint = objBody["Joints"].get< picojson::array >();
					// cout << "arrayJoint: " << arrayJoint.size() << endl;
					std::vector<PII> skelton;
					int cc=0;					// 枠外に出てしまっている関節x座標の数？　(コメント書いてよ...)
					int xx=0;					// 枠外に出てしまっている関節y座標の数？
					int tt=0;					// 送られてきた骨格情報の関節の数？
					for( auto itrJoint = arrayJoint.begin() ; itrJoint != arrayJoint.end() ; ++itrJoint )
					{
						picojson::object &objJoint = itrJoint->get< picojson::object >();
						picojson::object &objPositionColorSpace = objJoint["PositionColorSpace"].get< picojson::object >();
						int X= 479 - (int)( objPositionColorSpace["X"].get<double>() / 4 ) - x;
						int Y= (int)( objPositionColorSpace["Y"].get<double>() / 4 ) - y;	// フルHD骨格情報⇒1/4サイズ骨格情報⇒320*240サイズ骨格情報
						
						if(X<0){				// 320*240枠からはみ出した場合(X左側)?
							X=0;
							cc++;
						}
						else if(X>=320){		// 320*240枠からはみ出した場合(X右側)?
							X=320;
							cc++;
						}
						if(Y<0){				// 320*240枠からはみ出した場合(Y上側)?
							Y=0;
							xx++;
						}
						else if(Y>=240){		// 320*240枠からはみ出した場合(Y下側)?
							Y=240;
							xx++;
						}
						tt++;

						// cout << "X: " << X << ", Y: " << Y << endl;

						// if(X<280)
						// {
						if(skelton.size()==22||skelton.size()==24)									// 22個目の関節の時、24個目の関節の時(たしか、右手と左手？)
						{																			// サイズで判定するのはどうなのか？順番じゃなかった場合・・・
							cv::circle( dst, cv::Point( X, Y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4);	// 青丸
						}
						else
						{																						// ↑以外の関節の時
							cv::circle( dst, cv::Point( X, Y ), radius, cv::Scalar( 0, 200, 200 ), thickness );	// くすんだ黄色で丸
						}
						skelton.push_back(PII(X,Y));
						// }	
					}
					/*
					// 関節が25箇所全て取得できないと人物として認めないはやり過ぎだと思うのでコメントアウト
					if(cc<tt-1&&xx<tt-1&&skelton.size()>24)
					{
						//stringstream aa;
						//aa<<"sID:"<<skeletons.size();
						//cv::putText(dst, aa.str(),cv::Point(skelton[0].first+10,skelton[0].second+10), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,0,0), 2, CV_AA);     
						//
						long long  a=objBody[ "trackingID" ].get<double>();
						input.track_skelton.insert( map<int, long long>::value_type(skeletons.size(),a));
						skeletons.push_back(skelton);
					}
					*/
					if (cc < tt - 1 && xx < tt - 1 && skelton.size() > 12 )										// 取得できた関節数 - 1より、枠外だった関節数が小さい
					{																							// かつ、取れた関節数が12超

						//long long a = objBody["trackingID"].get<double>();									//Debug時、trackingIDの値をdouble型で取得できていなかったので
						//↓																					//一旦string型で取得しlong long型に変換(2019/04/05)
						std::string a_str = objBody["trackingID"].get<std::string>();	
						long long a = stoll( a_str.c_str(), nullptr, 10 );
			
						input.track_skelton.insert( map<int, long long>::value_type( skeletons.size(), a ) );	// track_skelton に skeletons のサイズと trackingID の値 を挿入
						skeletons.push_back(skelton);															// vec<vec<pair(int, int)>> skeletons に、
					}																							// skelton(trackできている人体の骨格情報が入った配列) をプッシュ
					// cout << "skelton: " << skelton.size() << endl;
				}
			}
		}
		else
		{
			std::cerr << __FUNCTION__ << ":" << err << std::endl;
		}
		if( skeletons.size() ){
			//cout << skeletons.size() << "(skeletons.size())!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
		}
		// cout << "skeletons: " << skeletons.size() << endl;
		//imshow("onSkelton", dst);
		//cv::waitKey(1);
		input.set_skeltons()=skeletons;
		input.depth_skelton_copyTo(dst);
		input.resize_depth();
	}
}