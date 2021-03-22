#include "Kinectsensor.h"
#include <zmq.h>
#include <cassert>
#include <msgpack.hpp>
#include <fstream>
#include "Util.h"
KinectSensor::KinectSensor(){
	context = zmq_init(3);
	responder = zmq_socket(context, ZMQ_REQ);
	assert( responder );
	string ipaddress, port;
	{
		ifstream ifs("configKinectServer.txt");
		string line;
		while( getline(ifs, line) )
		{
			vector<string> configs = Util::split( line, ":" );
			if( configs.size() != 2 )
			{
				cout << "invalid config : configKinectServer.txt" << endl;
				return;
			}
			ipaddress = configs[0];
			port = configs[1];
		}
	}

	try {
		string addr = "tcp://" + ipaddress + ":" + port;
		int rc = zmq_connect( responder, addr.c_str() );
		assert (rc == 0);
	} catch ( const zmq_event_t &e ) {
		std::cerr << "connection error: " << e.event << std::endl;
	}
	int rc =	zmq_setsockopt( responder, ZMQ_SUBSCRIBE, "", 0 );
	keyRGB = "kinectimage_rgb";
	keyDepth = "kinectimage_depth";
	keySkelton = "kinectimage_skenton";
}
KinectSensor::~KinectSensor(){
	zmq_close( responder );
}
void KinectSensor::recieve(){
	zmq_msg_t recv_message;
	zmq_msg_init( &recv_message );
	int rc = zmq_recvmsg( responder, &recv_message,  0 );
	assert (rc != -1);
	msgpack::sbuffer sbuf;
	msgpack::zone z;
	msgpack::object obj = msgpack::unpack( (char*)(zmq_msg_data)(&recv_message), zmq_msg_size(&recv_message), z);
	msgpack::convert( kinectpack, obj );
	//std::cout << "kinectServerFrame:" <<  imagepack["frame"] << endl;
	zmq_msg_close( &recv_message );
}
void KinectSensor::send(std::string message){
	zmq_msg_t send_message;
	zmq_msg_init(&send_message);
	memcpy(&send_message, "ok", string("ok").size());
	zmq_sendmsg( responder, &send_message, 0);
	zmq_msg_close( &send_message );
}
cv::Mat KinectSensor::getImageRGB(){
	return Util::convertString2Mat(kinectpack.kinectimage_rgb );
}
//cv::Mat KinectSensor::getImageDepth(){
//	return Util::convertString2Mat( kinectpack.kinectimage_depth );
//}
cv::Mat KinectSensor::getImageDepthOriginal(){
 return Util::convertString2Mat( kinectpack.kinectimage_depth_original );
}

void KinectSensor::VecToMat( cv::Mat &dst)
{
vector< vector<USHORT> > a=getImageDepthOriginal();
  int height = a.size(), width = a[0].size();
  dst = cv::Mat( height, width, CV_16UC1 );
  
  for( int i = 0; i < height; i++ ){
   for( int j = 0; j < width; j++ )
   {
	 
    dst.at<USHORT>( i, j ) =a[i][j];
   }
  }
}
std::vector<std::vector<PII>> KinectSensor:: set_skeltons(void)
{
	/*double size_width=320/640;
	double size_height=240/480;
	for(int i = 0 ; i <(int)kinectpack.skeletons.size(); i++)
	{
		for(int j = 0; j < (int)kinectpack.skeletons[i].size() ; j++)
		{
			kinectpack.skeletons[i][j].first=(int)((double)(kinectpack.skeletons[i][j].first)/640*320);
			kinectpack.skeletons[i][j].second=(int)((double)(kinectpack.skeletons[i][j].second)/480*240);
		}
}*/
	return kinectpack.skeletons;
}

PII  KinectSensor::kenectpoint(int n,int t)
{
	return kinectpack.skeletons[n][t];
}
void  KinectSensor::born_set(image &input)
{
	cv::Mat image_data;
	getImageRGB().copyTo(image_data);
	//cv::flip(image_data,image_data, 1);
	for(int i = 0 ; i <(int)kinectpack.skeletons.size(); i++)
	{
		for(int j = 0; j < (int)kinectpack.skeletons[i].size() ; j++)
		{
			int colorX =  kinectpack.skeletons[i][j].first;
			int colorY = kinectpack.skeletons[i][j].second;
			cv::Scalar color_data=cv::Scalar( 0, 0, 0 );
			switch( i )
			{
				case 1:
				color_data[0]=0;
                color_data[1]=0;
                color_data[2]=255; 
				break;
				case 2:
                color_data[0]=255;
                color_data[1]=128;
                color_data[2]=0; 
				break;
				case 3:
                color_data[0]=255;
                color_data[1]=128;
                color_data[2]=128; 
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
			if(j==7)//赤右手
			{
				color_data[0]=0;
                color_data[1]=0;
                color_data[2]=255;
			}
			if(j==11)//青左手
			{
				color_data[0]=255;
                color_data[1]=0;
                color_data[2]=0;
			}
			if(j==3)//頭
			{
				color_data[0]=255;
                color_data[1]=255;
                color_data[2]=128;
				cv::circle( image_data, cv::Point( colorX, colorY ), 3, color_data,3 );
			}

				cv::circle( image_data, cv::Point( colorX, colorY ), 2.5, color_data,2.5 );
		}
	}
	 input.depth_skelton_copyTo(image_data); 
	
}

bool KinectSensor::RGB_Maching(cv::Vec3b a,cv::Vec3b b)
{
	bool flag=false;
	if(((b[0]-10)<a[0]&&a[0]<(b[0]+10))&&((b[1]-10)<a[1]&&a[1]<(b[1]+10))&&((b[2]-10)<a[2]&&a[2]<(b[2]+10)))
	{
		flag=true;
	}
	return flag;
}

void KinectSensor::get_human_id(image &input)
{
	//cv::Mat cap=cv::Mat(cv::Size(320, 240), CV_8UC3,cv::Scalar(0,0,0));
	std::vector<cv::Vec3b> player_data;
	player_data.push_back(cv::Vec3b(0,0,255));
    player_data.push_back(cv::Vec3b(0,122,122));
    player_data.push_back(cv::Vec3b(0,255,0));
    player_data.push_back(cv::Vec3b(122,122,0));
    player_data.push_back(cv::Vec3b(255,0,0));
    player_data.push_back(cv::Vec3b(122,0,122));
    player_data.push_back(cv::Vec3b(255,255,0));
    player_data.push_back(cv::Vec3b(0,255,255));

	 for(int y=0;y<240;y++)
	 {
		for(int x=0;x<320;x++)
		{
			cv::Vec3b color_data = input.img_depth.at<cv::Vec3b>(y,x);

			if( RGB_Maching(color_data,player_data[0]))
			{
				input.set_human_id(x,y,1);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}
			else if(RGB_Maching(color_data,player_data[1]))
			{
                input.set_human_id(x,y,2);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}
			else if(RGB_Maching(color_data,player_data[2]))
			{
               input.set_human_id(x,y,3);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}	
			else if(RGB_Maching(color_data,player_data[3]))
			{
				input.set_human_id(x,y,4);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}
			else if(RGB_Maching(color_data,player_data[4]))
			{	
				input.set_human_id(x,y,5);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}	
			else if(RGB_Maching(color_data,player_data[5]))
			{	
				input.set_human_id(x,y,6);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}
			else if(RGB_Maching(color_data,player_data[6]))
			{	
				input.set_human_id(x,y,7);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}
			else if(RGB_Maching(color_data,player_data[7]))
			{	
				input.set_human_id(x,y,8);
				//cap.at<cv::Vec3b>(y,x)=color_data;
			}
			else
			{
				input.set_human_id(x,y,0);
			}
		}
	}
	/* cv::imshow("kinect_diff",cap);
	cv::waitKey(1);*/
	 input.expand_human_id();
}
//--------------------------------------------------
// Name     : run(image &input)
// Author   : 川本 (CV-lab.)
// Updata   : 2013.07.26
// Function : imageにkinectから得た画像とDepth情報を送る
// Argument : 
//            
// return   : なし
//--------------------------------------------------
void KinectSensor::run(image &input)
  {
    cv::Mat image1;

    // メインループ
   // while ( 1 ) {
      // データの更新を待つ
     DWORD ret = ::WaitForSingleObject( streamEvent, INFINITE );
     ::ResetEvent( streamEvent );

	
      /*drawRgbImage( image1 );
	  drawRgbImage( image2 );*/
	 drawRgbImage(image1);

	  

	 cvtColor(image1,input.img_original,CV_RGBA2RGB);

	 input.set_depth_size(width,height);
	 
	 drawDepthImage(input);
	 input.resize_depth();

	 cv::Mat image_data=cv::Mat(240,320,CV_8UC3,cv::Scalar::all(0));
	 cv::Vec3b pixel_DevValue;
	 for(int y=0;y<240;y++)
	 {
		for(int x=0;x<320;x++)
		{
				// RGB値をセット
			switch((int)input.get_human_id(x,y))
			{
				case 1:
				pixel_DevValue.val[0]=255;
                pixel_DevValue.val[1]=0;
                pixel_DevValue.val[2]=0; 
				break;
				case 2:
                pixel_DevValue.val[0]=0;
                pixel_DevValue.val[1]=255;
                pixel_DevValue.val[2]=0;
				break;
				case 3:
                pixel_DevValue.val[0]=0;
                pixel_DevValue.val[1]=0;
                pixel_DevValue.val[2]=255;
				break;
				case 4:
				pixel_DevValue.val[0]=255;
                pixel_DevValue.val[1]=255;
                pixel_DevValue.val[2]=255/4;
				break;
				case 5:
				pixel_DevValue.val[0]=255;
                pixel_DevValue.val[1]=255/4;
                pixel_DevValue.val[2]=255;
				break;
				case 6:
				pixel_DevValue.val[0]=255/2;
                pixel_DevValue.val[1]=255/2;
                pixel_DevValue.val[2]=255;
				break;
				default:
                pixel_DevValue.val[0]=0;
                pixel_DevValue.val[1]=0;
                pixel_DevValue.val[2]=0;
				break;
			}
			
			image_data.at<cv::Vec3b>(y,x)=pixel_DevValue;	
		}
	 }
	 //cv::imshow("a",image_data);
	 //input.depth_medianBlur();
	 //input.set_depth_Y_size(320,240);
	 cv::resize(input.img_depth,input.depth_Out(),  input.depth_Out().size(), cv::INTER_CUBIC);
	 image1.release();
  }
