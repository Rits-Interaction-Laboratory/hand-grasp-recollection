#pragma once
#include"image.h"

typedef struct {}log_Data;

class DetectionLog
{
private:
	

protected:


public:

	void timelog(int frame,image &input)
	{
			static bool isFirst = true;
			static cv::Mat timeline;
			static std::vector<int>save_data;
			static std::map<int,int>line;
			static std::map<int,int>track;

			if(isFirst)
			{
				//白地
				timeline = cv::Mat(cv::Size(320,240), CV_8UC3, cv::Scalar(255,255,255));
				//時間軸表示
				cv::line(timeline, cv::Point(0, 120), cv::Point(320, 120), cv::Scalar(0,0,0), 1, 4);
	
				isFirst = false;
			}
//すべての画像を右にずらす
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			for(int i = 0 ; i < 240 ; i++)
			{
				for(int j = timeline.cols-2 ; j >=80 ; j--)
				{

					timeline.at<cv::Vec3b>(i,j+1) = timeline.at<cv::Vec3b>(i,j);
					if(j == 80)
					{
						timeline.at<cv::Vec3b>(i,80) = cv::Vec3b(255,255,255);
					}
					if(i==120)
					{
						timeline.at<cv::Vec3b>(120,j) = cv::Vec3b(0,0,0);
					}

				} 
			}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		
			//点表示
			int graph_y = 120;
	       
			for(int t=0;t<input.HumanData();t++)
			{
				cv::circle(timeline, cv::Point(80, graph_y+(110-10*(t%11))), 0.5, cv::Scalar(0,0,0), 1, 0.5);
			}


			for(int t=0;t<input.EventMAX();t++)
			{
			
				/*string strEventLabel;
				string strFrame = "["+ Util::toString(frame) +"]";*/
				//int Event_y;
				if(input.get_Event_Eve(t)==1||input.get_Event_Eve(t)==2)
				{
					/*strEventLabel = "in";*/
					save_data.push_back(input.get_Event_model(t));
					if(input.get_Event_Eve(t)==2)//移動
					{
						
						line.insert(std::pair<int,int>(input.get_Event_model(t),input.get_Event_key(t)));
						graph_y=10+10*(line[save_data[save_data.size()-1]]%11);
						cv::circle(timeline, cv::Point(100, graph_y), 5, cv::Scalar(0,255,255), 1, 1);
					    cout<<input.get_Event_key(t)<<endl;
						//移動間隔
				        cv::line(timeline, cv::Point(100, graph_y), cv::Point(100, 240), cv::Scalar(255,255,0), 0.5, 4);
				        cv::line(timeline, cv::Point(100+frame-track[input.get_Event_key(t)], graph_y), cv::Point(100+frame-track[input.get_Event_key(t)], 240), cv::Scalar(0,255,255), 0.5, 4);
	                    
						//持ち去り場所補正
					    cv::line(timeline, cv::Point(100, graph_y), cv::Point(100+frame-track[input.get_Event_key(t)], graph_y), cv::Scalar(0,0,255), 0.5, 4);
						
						//持ち去り情報削除
						track.erase(input.get_Event_key(t));
						if(!track.empty()){if(frame-track[0]>1000){track.erase(track.begin());}}
							
					}
					else
					{
						line.insert(std::pair<int,int>(input.get_Event_model(t),input.get_Event_model(t)));
				
						graph_y=10+10*(line[save_data[save_data.size()-1]]%11);
						cv::circle(timeline, cv::Point(100, graph_y), 5, cv::Scalar(0,0,255), 1, 1);
					//持ち込場所補正
					
					}
					cv::line(timeline, cv::Point(80, graph_y), cv::Point(100, graph_y), cv::Scalar(0,0,0), 0.5, 4);
					/*graph_y = 100+20*t%2;*/
					/*Event_y = 80+20*t%2;
					cv::line(timeline, cv::Point(80, 100), cv::Point(80, 140), cv::Scalar(0,0,0), 1, 4);*/
				}
				else{
					for(int data=0;data<(int)save_data.size();data++)
					{
						if(save_data[data]==input.get_Event_model(t))
						{
							graph_y=10+10*(line[save_data[data]]%11);
							cv::circle(timeline, cv::Point(100, graph_y), 5, cv::Scalar(255,0,0), 1, 1);

							track.insert(std::pair<int,int>(line[input.get_Event_model(t)],frame));

							save_data.erase(save_data.begin()+data);
							line.erase(input.get_Event_model(t));
							//持ち去り補正
							cv::line(timeline, cv::Point(80, graph_y), cv::Point(100,graph_y ), cv::Scalar(255,255,255), 0.5, 4);
							
						}
					}
					/*strEventLabel = "out";

					graph_y = 140+20*t%2;
					Event_y = 180+20*t%2;
					cv::line(timeline, cv::Point(80, 100), cv::Point(80, 140), cv::Scalar(0,0,0), 1, 4);*/

				}
				
				

			}
			
			/*pParent->isEvent = false*/
			/*cv::putText(timeline, strEventLabel, cv::Point(80,Event_y), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0,0,0), 0.5, CV_AA);
			cv::putText(timeline, strFrame, cv::Point(80,Event_y-20), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0,0,0), 0.5, CV_AA);
	*/
			for(int detect=0;detect<(int)save_data.size();detect++)
			{
				graph_y=10+10*(line[save_data[detect]]%11);
				cv::circle(timeline, cv::Point(80, graph_y), 0.5, cv::Scalar(0,0,0), 1, 4);
			}
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

			
	
		
	//-------------------↑可変表示↑-----------------------------//
	cv::Mat outputTimeline = timeline.clone();
	//-------------------↓固定表示↓------------------------------//

	cv::line(outputTimeline, cv::Point(80, 0), cv::Point(80, 240), cv::Scalar(0,255,0), 1, 4);
	cv::putText(outputTimeline, "frame", cv::Point(0,110), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
	cv::putText(outputTimeline, "object", cv::Point(0,50), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);
	cv::putText(outputTimeline, "human", cv::Point(0,200), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar(0,0,0), 1, CV_AA);


	//bmp time;
	//Util::convertMat2BmpChar(outputTimeline, time);
	//pParent->showtimeline(&time.B(0,0));
	cv::imshow("detectlog",outputTimeline);
	cv::waitKey(1);
	}
	
};