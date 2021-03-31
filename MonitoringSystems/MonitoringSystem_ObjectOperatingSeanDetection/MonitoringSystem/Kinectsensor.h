//------------------------------------------------------------------------------
// <copyright file="KinectSensor.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------
#pragma once

#include"image.h"

#include <iostream>
#include <sstream>
#include "sensor.h"
// NuiApi.h�̑O��Windows.h���C���N���[�h����
#include <Windows.h>
#include <NuiApi.h>
#include <conio.h>

#include <opencv2/opencv.hpp>
#include <zmq.hpp>
#include <map>
#include<msgpack.hpp>

#define DEPTH_RANGE_MAX 3500//3500
#define DEPTH_RANGE_MIN 500//800
#define ERROR_CHECK( ret )  \
  if ( ret != S_OK ) {    \
    std::stringstream ss;	\
    ss << "failed " #ret " " << std::hex << ret << std::endl;			\
    throw std::runtime_error( ss.str().c_str() );			\
  }
const NUI_IMAGE_RESOLUTION CAMERA_RESOLUTION = NUI_IMAGE_RESOLUTION_640x480;
//�l���F����


class KinectPack{
public:
	std::string kinectimage_rgb;
	std::string kinectimage_depth_original;
	std::vector<std::vector< std::pair<int, int> > > skeletons;

	MSGPACK_DEFINE( kinectimage_rgb,
		kinectimage_depth_original,
		skeletons );
};


class KinectSensor : public Sensor{
private:
	void* context;
	void* responder;
	std::map<std::string, std::string > imagepack;
	INuiSensor* kinect;
	HANDLE imageStreamHandle;
	HANDLE depthStreamHandle;
	HANDLE streamEvent;

	DWORD width;
	DWORD height;

	  INuiColorCameraSettings *cameraSettings;
private:
	std::string keyRGB;
	std::string keyDepth;
	std::string keyDepthOriginal;
	std::string keySkelton;
public:
	KinectSensor();
	~KinectSensor();
public:
	KinectPack kinectpack;
	void recieve();
	void send(std:: string message);
	//RGB���̎擾
	cv::Mat getImageRGB();
	//Depth���̎擾
	//cv::Mat getImageDepth();
	cv::Mat KinectSensor::getImageDepthOriginal();

	cv::Mat getImageSkenlton();
	void VecToMat( cv::Mat &dst);

	std::pair<int,int>kenectpoint();

	std::vector<std::vector<PII>> set_skeltons(void);

	 PII kenectpoint(int n,int t);
	 void born_set(image &input);

	 void get_human_id(image &input);

	//--------------------------------------
	//���ڐڑ��p
	 void initialize(bool &conect)
  {
   conect= createInstance();
   
   if(conect==true){
    // Kinect�̐ݒ������������
    ERROR_CHECK( kinect->NuiInitialize(
    NUI_INITIALIZE_FLAG_USES_COLOR |
	NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX|
    NUI_INITIALIZE_FLAG_USES_SKELETON ) );

    // RGB�J����������������
    ERROR_CHECK( kinect->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR,
    NUI_IMAGE_RESOLUTION_640x480/* CAMERA_RESOLUTION*/, 0, 2, 0, &imageStreamHandle ) );


	// �����J����������������
	ERROR_CHECK( kinect->NuiImageStreamOpen( NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, CAMERA_RESOLUTION,
      0, 2, 0, &depthStreamHandle ) );

    // �X�P���g��������������
    ERROR_CHECK( kinect->NuiSkeletonTrackingEnable(
      0, NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS ) );


    // �t���[���X�V�C�x���g�̃n���h�����쐬����
    streamEvent = ::CreateEvent( 0, TRUE, FALSE, 0 );
    ERROR_CHECK( kinect->NuiSetFrameEndEvent( streamEvent, 0 ) );

    // �w�肵���𑜓x�́A��ʃT�C�Y���擾����
    ::NuiImageResolutionToSize(CAMERA_RESOLUTION, width, height );
	
	cameraSettings = 0;
    kinect->NuiGetColorCameraSettings( &cameraSettings );
	cv::waitKey( 1 );
	BOOL whitebalance=false;
	//cameraSettings->SetAutoExposure(whitebalance);
	//cameraSettings->SetAutoWhiteBalance(whitebalance);
   }

  }
	 void run(image &input);


private:

  bool createInstance()
  {
    // �ڑ�����Ă���Kinect�̐����擾����
    int count = 0;
    ERROR_CHECK( ::NuiGetSensorCount( &count ) );

    if ( count == 0 ) {
      //throw std::runtime_error( "Kinect ��ڑ����Ă�������" );
	  return false;
    }

    // �ŏ���Kinect�̃C���X�^���X���쐬����
    ERROR_CHECK( ::NuiCreateSensorByIndex( 0, &kinect ) );


    // Kinect�̏�Ԃ��擾����
    HRESULT status = kinect->NuiStatus();
    if ( status != S_OK ) {
      throw std::runtime_error( "Kinect �����p�\�ł͂���܂���" );
    }

  return true;
  }

  void drawRgbImage( cv::Mat& image )
  {
      // RGB�J�����̃t���[���f�[�^���擾����
      NUI_IMAGE_FRAME imageFrame = { 0 };
      ERROR_CHECK( kinect->NuiImageStreamGetNextFrame(
        imageStreamHandle, INFINITE, &imageFrame ) ); 

      // �摜�f�[�^���擾����
      NUI_LOCKED_RECT colorData;
      imageFrame.pFrameTexture->LockRect( 0, &colorData, 0, 0 );

     // cameraSettings->SetAutoWhiteBalance( false );
      // �摜�f�[�^���R�s�[����
      image = cv::Mat( height, width, CV_8UC4, colorData.pBits );


      // �t���[���f�[�^���������
      ERROR_CHECK( kinect->NuiImageStreamReleaseFrame(
        imageStreamHandle, &imageFrame ) );
  }

  void humandata(image &input,BYTE *depth)
  {
    // RGBQUAD c;
	 //int lv =0;
	 //BYTE iv=0;
	 cv::Mat image_data=cv::Mat(height,width,CV_8UC3,cv::Scalar::all(0));
	 input.img_original.copyTo(image_data);
	 USHORT* pRun=(USHORT*) depth;
	 cv::Vec3b pixel_DevValue;
	 std::vector<PII> skeleton_data;
	 PII data;
	 for(int y=0;y<(int)height;y++)
	 {
		for(int x=0;x<(int)width;x++)
		{
			
			/*cv::Vec3b data = image_data.at<cv::Vec3b>(y,x);*/
			
			// ���i��ID���擾
			UCHAR id = *pRun & 7;

			input.set_human_id(x,y,id);
			// �������擾
			//USHORT RealDepth = (*pRun & 0xfff8) >> 3;
			//
			//	// RGB�l���Z�b�g
			//switch(input.get_human_id(x,y))
			//{
			//	case 1:
			//	pixel_DevValue.val[0]=255;
   //             pixel_DevValue.val[1]=0;
   //             pixel_DevValue.val[2]=0; 
			//	break;
			//	case 2:
   //             pixel_DevValue.val[0]=0;
   //             pixel_DevValue.val[1]=255;
   //             pixel_DevValue.val[2]=0;
			//	break;
			//	case 3:
   //             pixel_DevValue.val[0]=0;
   //             pixel_DevValue.val[1]=0;
   //             pixel_DevValue.val[2]=255;
			//	break;
			//	case 4:
			//	pixel_DevValue.val[0]=255;
   //             pixel_DevValue.val[1]=255;
   //             pixel_DevValue.val[2]=255/4;
			//	break;
			//	case 5:
			//	pixel_DevValue.val[0]=255;
   //             pixel_DevValue.val[1]=255/4;
   //             pixel_DevValue.val[2]=255;
			//	break;
			//	case 6:
			//	pixel_DevValue.val[0]=255/2;
   //             pixel_DevValue.val[1]=255/2;
   //             pixel_DevValue.val[2]=255;
			//	break;
			//	default:
   //             pixel_DevValue.val[0]=data[0];
   //             pixel_DevValue.val[1]=data[1];
   //             pixel_DevValue.val[2]=data[2];
			//	break;
			//}
			//
			//image_data.at<cv::Vec3b>(y,x)=pixel_DevValue;
			pRun++;		
		}
	 }


	
    
    NUI_SKELETON_FRAME skeletonFrame = { 0 };
    kinect->NuiSkeletonGetNextFrame( 0, &skeletonFrame );
	if(FAILED(kinect))return;
    selectActiveSkeleton( skeletonFrame );
    for ( int i = 0; i < NUI_SKELETON_COUNT; ++i ) {
      NUI_SKELETON_DATA& skeletonData = skeletonFrame.SkeletonData[i];//�X�P���g���f�[�^
      if ( skeletonData.eTrackingState == NUI_SKELETON_TRACKED ) {
        for ( int j = 0; j < NUI_SKELETON_POSITION_COUNT; ++j ) {
          if ( skeletonData.eSkeletonPositionTrackingState[j] != NUI_SKELETON_POSITION_NOT_TRACKED )
		  {
              drawJoint( image_data, skeletonData.SkeletonPositions[j],i+1,data );
			  /*data.first= skeletonData.SkeletonPositions[j].x/2;
			  data.second=skeletonData.SkeletonPositions[j].y/2;
			  cout << data.first<< ":"<<data.second  << endl;*/
			  skeleton_data.push_back(data);
          }
		 
        }
		input.set_skeltons().push_back( skeleton_data);
		skeleton_data.clear();
      }
      else if ( skeletonData.eTrackingState == NUI_SKELETON_POSITION_ONLY ) {
        drawJoint( image_data, skeletonData.Position,i+1,data );
      }
    }
	input.depth_skelton_copyTo(image_data); 
    // �X�P���g���̃t���[�����擾����
  }


  void drawJoint( cv::Mat& image, Vector4 position,int id,PII &data)
  {
    FLOAT depthX = 0, depthY = 0;
    ::NuiTransformSkeletonToDepthImage(
      position, &depthX, &depthY, CAMERA_RESOLUTION );

    LONG colorX = 0;
    LONG colorY = 0;

    kinect->NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
      CAMERA_RESOLUTION, CAMERA_RESOLUTION,
      0, (LONG)depthX , (LONG)depthY, 0, &colorX, &colorY );
	data.first= colorX/2;
	data.second=colorY/2;
	cv::Scalar color_data=cv::Scalar( 0, 0, 0 );
	switch( id )
			{
				case 1:
				color_data[0]=0;
                color_data[1]=0;
                color_data[2]=255; 
				break;
				case 2:
                color_data[0]=255;
                color_data[1]=0;
                color_data[2]=0; 
				break;
				case 3:
                color_data[0]=255;
                color_data[1]=0;
                color_data[2]=0; 
				break;
				case 4:
				color_data[0]=255/4;
                color_data[1]=255;
                color_data[2]=255; 
				break;
				case 5:
				color_data[0]=255;
                color_data[1]=255;
                color_data[2]=255/4; 
				break;
				case 6:
				color_data[0]=255/4;
                color_data[1]=255;
                color_data[2]=255/4; 
				break;
				default:
                color_data[0]=255;
                color_data[1]=255;
                color_data[2]=255; 
				break;
			}

    cv::circle( image, cv::Point( colorX, colorY ), 5, color_data, 5 );
  }

  DWORD activeTrackId;

  void selectActiveSkeleton( NUI_SKELETON_FRAME& skeletonFrame )
  {
    const int center = width / 2;
    FLOAT minpos = 0;
    DWORD trackedId = 0;

    // ��Ԓ��S�ɋ߂��v���C���[��T��
    for ( int i = 0; i < NUI_SKELETON_COUNT; ++i ) {
      NUI_SKELETON_DATA& skeletonData = skeletonFrame.SkeletonData[i];
      if ( skeletonData.eTrackingState != NUI_SKELETON_NOT_TRACKED ) {
        FLOAT depthX = 0, depthY = 0;
        ::NuiTransformSkeletonToDepthImage(
          skeletonData.Position, &depthX, &depthY, CAMERA_RESOLUTION );

        if ( abs(minpos - center) > abs(depthX - center) ) {
          minpos = depthX;
          trackedId = skeletonData.dwTrackingID;
        }
      }
    }

    // ���ݒǐՂ��Ă���v���C���[�łȂ���΁A���S�ɋ߂��v���C���[���A�N�e�B�u�ɂ���
    if ( (trackedId != 0) || (trackedId != activeTrackId) ) {
      DWORD trackedIds[] = { trackedId, 0 };
      kinect->NuiSkeletonSetTrackedSkeletons( trackedIds );
      activeTrackId = trackedId;
    }
  }
   void drawDepthImage(image &input )
  {
	  input.img_depth=cv::Mat( height, width, CV_8UC3 );
      // �����J�����̃t���[���f�[�^���擾����
      NUI_IMAGE_FRAME depthFrame = { 0 };
      
	  ERROR_CHECK( kinect->NuiImageStreamGetNextFrame( depthStreamHandle, INFINITE, &depthFrame ) );
	  //ERROR_CHECK( kinect->NuiImageStreamGetNextFrame( depthStreamHandle, 0, &depthFrame ) );

      // �����f�[�^���擾����
      NUI_LOCKED_RECT depthData = { 0 };
      depthFrame.pFrameTexture->LockRect( 0, &depthData, 0, 0 );

      USHORT* depth = (USHORT*)depthData.pBits;
	  humandata(input,depthData.pBits);
	  USHORT* pRun=(USHORT*) depth;
	  for ( int i = 0; i < (int)(depthData.size / sizeof(USHORT)); ++i )
	  {
        USHORT distance = ::NuiDepthPixelToDepth( depth[i] );
		//USHORT id = *pRun & 7;
		
        LONG depthX = i % width;
        LONG depthY = i / width;
        LONG colorX = depthX;
        LONG colorY = depthY;
		

        // �����J�����̍��W���ARGB�J�����̍��W�ɕϊ�����
        kinect->NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
          CAMERA_RESOLUTION, CAMERA_RESOLUTION,
          0, depthX , depthY, depth[i], &colorX, &colorY );


		//input.set_human_id(colorX,colorY,id);
        // ���ȏ�̋�����`�悵�Ȃ�
        //if ( distance >= 1000 ) {
		input.set_depth( colorX, colorY,distance);

		//unsigned short tt=input.get_depth(colorX,colorY);

          /*int index = ((colorY * width) + colorX) * 3;
          UCHAR* data = &input.img_depth.data[index];
		  data[0] = 255*(max(0,distance-DEPTH_RANGE_MIN))/DEPTH_RANGE_MAX;
          data[1] =255*(max(0,distance-DEPTH_RANGE_MIN))/DEPTH_RANGE_MAX;
          data[2] = 255*(max(0,distance-DEPTH_RANGE_MIN))/DEPTH_RANGE_MAX;*/
        //}
		
		pRun++;	
      }

      // �t���[���f�[�^���������
      ERROR_CHECK( kinect->NuiImageStreamReleaseFrame( depthStreamHandle, &depthFrame ) );
  }

  bool RGB_Maching(cv::Vec3b a,cv::Vec3b b);
};

