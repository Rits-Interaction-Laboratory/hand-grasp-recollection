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
		int x=(480/2)-160;				//  		 |�P�P�P�P�P�P�P�P�P|  P(x, y)
		int y=(270/2)-120;				//  		 | �@ P�P�P�P�P|	|
										//		 270 | 240|   320  |	|
										//			 |	�@|�Q�Q�Q�Q|	|
										//			 |�Q�Q�Q�Q�Q�Q�Q�Q�Q|
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
					int cc=0;					// �g�O�ɏo�Ă��܂��Ă���֐�x���W�̐��H�@(�R�����g�����Ă�...)
					int xx=0;					// �g�O�ɏo�Ă��܂��Ă���֐�y���W�̐��H
					int tt=0;					// �����Ă������i���̊֐߂̐��H
					for( auto itrJoint = arrayJoint.begin() ; itrJoint != arrayJoint.end() ; ++itrJoint )
					{
						picojson::object &objJoint = itrJoint->get< picojson::object >();
						picojson::object &objPositionColorSpace = objJoint["PositionColorSpace"].get< picojson::object >();
						int X= (int)( objPositionColorSpace["X"].get<double>() / 4 ) - x;
						int Y= (int)( objPositionColorSpace["Y"].get<double>() / 4 ) - y;	// �t��HD���i����1/4�T�C�Y���i����320*240�T�C�Y���i���
						
						if(X<0){				// 320*240�g����͂ݏo�����ꍇ(X����)?
							X=0;
							cc++;
						}
						else if(X>=320){		// 320*240�g����͂ݏo�����ꍇ(X�E��)?
							X=320;
							cc++;
						}
						if(Y<0){				// 320*240�g����͂ݏo�����ꍇ(Y�㑤)?
							Y=0;
							xx++;
						}
						else if(Y>=240){		// 320*240�g����͂ݏo�����ꍇ(Y����)?
							Y=240;
							xx++;
						}
						tt++;

						// cout << "X: " << X << ", Y: " << Y << endl;

						// if(X<280)
						// {
						if(skelton.size()==22||skelton.size()==24)									// 22�ڂ̊֐߂̎��A24�ڂ̊֐߂̎�(�������A�E��ƍ���H)
						{																			// �T�C�Y�Ŕ��肷��̂͂ǂ��Ȃ̂��H���Ԃ���Ȃ������ꍇ�E�E�E
							cv::circle( dst, cv::Point( X, Y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4);	// ��
						}
						else
						{																						// ���ȊO�̊֐߂̎�
							cv::circle( dst, cv::Point( X, Y ), radius, cv::Scalar( 0, 200, 200 ), thickness );	// �����񂾉��F�Ŋ�
						}
						skelton.push_back(PII(X,Y));
						// }	
					}
					/*
					// �֐߂�25�ӏ��S�Ď擾�ł��Ȃ��Ɛl���Ƃ��ĔF�߂Ȃ��͂��߂����Ǝv���̂ŃR�����g�A�E�g
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
					if (cc < tt - 1 && xx < tt - 1 && skelton.size() > 12 )										// �擾�ł����֐ߐ� - 1���A�g�O�������֐ߐ���������
					{																							// ���A��ꂽ�֐ߐ���12��

						//long long a = objBody["trackingID"].get<double>();									//Debug���AtrackingID�̒l��double�^�Ŏ擾�ł��Ă��Ȃ������̂�
						//��																					//��Ustring�^�Ŏ擾��long long�^�ɕϊ�(2019/04/05)
						std::string a_str = objBody["trackingID"].get<std::string>();	
						long long a = stoll( a_str.c_str(), nullptr, 10 );
			
						input.track_skelton.insert( map<int, long long>::value_type( skeletons.size(), a ) );	// track_skelton �� skeletons �̃T�C�Y�� trackingID �̒l ��}��
						skeletons.push_back(skelton);															// vec<vec<pair(int, int)>> skeletons �ɁA
					}																							// skelton(track�ł��Ă���l�̂̍��i��񂪓������z��) ���v�b�V��
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
		int x=(480/2)-160;				//  		 |�P�P�P�P�P�P�P�P�P|  P(x, y)
		int y=(270/2)-120;				//  		 | �@ P�P�P�P�P|	|
										//		 270 | 240|   320  |	|
										//			 |	�@|�Q�Q�Q�Q|	|
										//			 |�Q�Q�Q�Q�Q�Q�Q�Q�Q|
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
					int cc=0;					// �g�O�ɏo�Ă��܂��Ă���֐�x���W�̐��H�@(�R�����g�����Ă�...)
					int xx=0;					// �g�O�ɏo�Ă��܂��Ă���֐�y���W�̐��H
					int tt=0;					// �����Ă������i���̊֐߂̐��H
					for( auto itrJoint = arrayJoint.begin() ; itrJoint != arrayJoint.end() ; ++itrJoint )
					{
						picojson::object &objJoint = itrJoint->get< picojson::object >();
						picojson::object &objPositionColorSpace = objJoint["PositionColorSpace"].get< picojson::object >();
						int X= 479 - (int)( objPositionColorSpace["X"].get<double>() / 4 ) - x;
						int Y= (int)( objPositionColorSpace["Y"].get<double>() / 4 ) - y;	// �t��HD���i����1/4�T�C�Y���i����320*240�T�C�Y���i���
						
						if(X<0){				// 320*240�g����͂ݏo�����ꍇ(X����)?
							X=0;
							cc++;
						}
						else if(X>=320){		// 320*240�g����͂ݏo�����ꍇ(X�E��)?
							X=320;
							cc++;
						}
						if(Y<0){				// 320*240�g����͂ݏo�����ꍇ(Y�㑤)?
							Y=0;
							xx++;
						}
						else if(Y>=240){		// 320*240�g����͂ݏo�����ꍇ(Y����)?
							Y=240;
							xx++;
						}
						tt++;

						// cout << "X: " << X << ", Y: " << Y << endl;

						// if(X<280)
						// {
						if(skelton.size()==22||skelton.size()==24)									// 22�ڂ̊֐߂̎��A24�ڂ̊֐߂̎�(�������A�E��ƍ���H)
						{																			// �T�C�Y�Ŕ��肷��̂͂ǂ��Ȃ̂��H���Ԃ���Ȃ������ꍇ�E�E�E
							cv::circle( dst, cv::Point( X, Y ), 1, cv::Scalar( 255, 0, 0 ), 5, 4);	// ��
						}
						else
						{																						// ���ȊO�̊֐߂̎�
							cv::circle( dst, cv::Point( X, Y ), radius, cv::Scalar( 0, 200, 200 ), thickness );	// �����񂾉��F�Ŋ�
						}
						skelton.push_back(PII(X,Y));
						// }	
					}
					/*
					// �֐߂�25�ӏ��S�Ď擾�ł��Ȃ��Ɛl���Ƃ��ĔF�߂Ȃ��͂��߂����Ǝv���̂ŃR�����g�A�E�g
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
					if (cc < tt - 1 && xx < tt - 1 && skelton.size() > 12 )										// �擾�ł����֐ߐ� - 1���A�g�O�������֐ߐ���������
					{																							// ���A��ꂽ�֐ߐ���12��

						//long long a = objBody["trackingID"].get<double>();									//Debug���AtrackingID�̒l��double�^�Ŏ擾�ł��Ă��Ȃ������̂�
						//��																					//��Ustring�^�Ŏ擾��long long�^�ɕϊ�(2019/04/05)
						std::string a_str = objBody["trackingID"].get<std::string>();	
						long long a = stoll( a_str.c_str(), nullptr, 10 );
			
						input.track_skelton.insert( map<int, long long>::value_type( skeletons.size(), a ) );	// track_skelton �� skeletons �̃T�C�Y�� trackingID �̒l ��}��
						skeletons.push_back(skelton);															// vec<vec<pair(int, int)>> skeletons �ɁA
					}																							// skelton(track�ł��Ă���l�̂̍��i��񂪓������z��) ���v�b�V��
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