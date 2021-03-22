#include "stdafx.h"
#include "Surf.h"
#include "MonitoringSystemDlg.h"
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEBUG 0
const int DIM_VECTOR = 128;    // 128�����x�N�g��
const double THRESHOLD = 0.35;  // �����Ȃ�������臒l
SurfPoint::SurfPoint(void){};
SurfPoint::~SurfPoint(void){};
/**
* ���̗̈悩��SURF�����ʂ𒊏o����
*
* @param[in]  png      �摜�t�@�C����
* @param[in]  region   ���͈̔�
*
* @return 0
*/ 
int SurfPoint::Pointing(unsigned long frame,image &input, int delete_key, std::vector<int> key_idname,std::vector<int>obj_idname,int num){
//(img_original,cvRect(x1,y1,x2,y2),coefficient(1),frame,det, d_input, flag, tracker, outId);
	int numObjects = -1;
	//CvMemStorage* storage = cvCreateMemStorage(0);	//
	//CvSeq* imageKeypoints = 0;		//CV�V�[�P���X:�����_
	//CvSeq* imageDescriptors = 0;	//CV�V�[�P���X:

	//CvSURFParams params = cvSURFParams(100, 1);//臒l300 �g���f�B�X�N���v�^ 128�v�f
	//IplImage* mask = cvCreateImage(cvSize(input.img_original.cols,input.img_original.rows),IPL_DEPTH_8U,1);
	//cvZero(mask);
	//cv::rectangle(cv::Mat(mask),cv::Rect(input.get_ObjectTab_s_w(num)*input.sizeWidth,input.get_ObjectTab_s_h(num)*input.sizeHeigth,(input.get_ObjectTab_e_w(num)-input.get_ObjectTab_s_w(num))*input.sizeWidth,(input.get_ObjectTab_e_h(num)-input.get_ObjectTab_s_h(num))*input.sizeHeigth),CV_RGB (255, 255, 255), CV_FILLED);
	//IplImage png =input.img_original;
	//IplImage* grayImage = cvCreateImage(cvGetSize(&png), 8, 1);
	//cv::Mat gray_image = cvCreateImage(cvGetSize(&png), 8, 1);
	//cvCvtColor(&png, grayImage, CV_BGR2GRAY);
	//// �t���[���摜����SURF���擾
	//cvExtractSURF(grayImage, mask, &imageKeypoints, &imageDescriptors, storage, params);

	//cvReleaseImage(&mask);
	//cvReleaseImage(&grayImage);

	 //�摜�ǂݍ���
    cv::Mat colorImg=input.img_original.clone();
	cv::Mat grayImg;
	std::vector<cv::KeyPoint> imgkeypoints;
	std::vector<float> imgDesctiptors;

	cv::SURF calc_surf = cv::SURF(100, 4, 2, true);
	cv::cvtColor(colorImg, grayImg, CV_BGR2GRAY);
	cv::Mat mask(cv::Size(input.img_original.cols,input.img_original.rows), CV_8UC1,cv::Scalar(0));
	cv::rectangle(mask,cv::Rect(input.get_ObjectTab_s_w(num)*input.sizeWidth,input.get_ObjectTab_s_h(num)*input.sizeHeigth,(input.get_ObjectTab_e_w(num)-input.get_ObjectTab_s_w(num))*input.sizeWidth,(input.get_ObjectTab_e_h(num)-input.get_ObjectTab_s_h(num))*input.sizeHeigth),CV_RGB (255, 255, 255), CV_FILLED);
	calc_surf(grayImg, mask, imgkeypoints, imgDesctiptors);


	cout << " SurfMatching"<< endl;

	//	_mask.release();
	//	imgGray.release();

	//   // �摜�ɃL�[�|�C���g��`��
	//   for (int j = 0; j < imageKeypoints->total; j++) 
	//{
	//       CvSURFPoint* point = (CvSURFPoint*)cvGetSeqElem(imageKeypoints, j);  // i�Ԗڂ̃L�[�|�C���g
	//       CvPoint center;  // �L�[�|�C���g�̒��S���W
	//       center.x = cvRound(point->pt.x);
	//       center.y = cvRound(point->pt.y);
	//	cvCircle(captureImage, center, 2, cvScalar(255,255,0), CV_FILLED);
	//   }
	//cvRectangle (captureImage, cvPoint (region.x+10, png->height-(region.y+10))
	//	, cvPoint (region.width-10, png->height-(region.height-10)), CV_RGB (255, 0, 0), 2);

	// cvShowImage("SURF", captureImage);
	//cvWaitKey(30);
	//sprintf(fname,"../input/%05d.png",frame);
	//cvSaveImage(fname,captureImage);

	//	numObjects = SurfMatching(imageKeypoints, imageDescriptors, png/*, captureImage*/, frame
	//		, cvRect(region.x+10,png->height-(region.height+10), ((region.width-10)-region.x),((region.height-10)-region.y)), det, input, delete_key, tracker, outId);
	//

	numObjects = SurfMatching(imgkeypoints, imgDesctiptors,  frame,input
		, cvRect(input.get_ObjectTab_s_w(num)*input.sizeWidth,(input.get_ObjectTab_s_h(num)*input.sizeHeigth), (input.get_ObjectTab_e_w(num)-input.get_ObjectTab_s_w(num))*input.sizeWidth,(input.get_ObjectTab_e_h(num)-input.get_ObjectTab_s_h(num))*input.sizeHeigth), delete_key, key_idname,obj_idname,num);

	cout << " SurfMatching end"<< endl;
	//sprintf(keyname,"../output/comparison/keypoint/keypoint-%05d.png",count);
	//cvSaveImage(keyname,captureImage);

	//while(1){
	//	cvShowImage("window", png);
	//			
	//	int key = cvWaitKey(30);
	//	if(key == 32){
	//		break;
	//	}
	//}
	//TODO:comment
#if DEBUG
	//cout<<"#"<<numObjects<<":"<<imgkeypoints.size()<<endl;
#endif
	/*	cout<<numObjects<<":";
	numObjects = imgkeypoints.size();
	cout<<numObjects<<endl;*/
	// ���[�v���ō쐬�����I�u�W�F�N�g�͎n��
	//cout<<"imageKeypoints"<<endl;

	//cvClearSeq(imageKeypoints);
	//cvClearSeq(imageDescriptors);
	//cvReleaseMemStorage(&storage);

	//cvReleaseImage(&captureImage);

	return numObjects;
}

int SurfPoint::Votes_Registered(){
	return Registered;
}
//saveFile(kyoriId, imageKeypoints, imageDescriptors, png, entry_pairs, cvRect(region.x, region.y, region.width, region.height), id2name, frame, new_obj, file,idname);
void SurfPoint::saveFile(int objId, std::vector<cv::KeyPoint> keypoints1, std::vector<float> descriptors1,IplImage* output, vector<int> entry_pairs, CvRect region, map<int, int> id2name, unsigned long frame, bool new_obj, bool file,std::vector<int> key_idname,std::vector<int> obj_idname) {
	cout<<"saveFile"<<endl;
	IplImage* colorImagesurf1 = cvCreateImage(cvSize(output->width, output->height), 8, 3);
	IplImage* colorImagesurf2 = cvCreateImage(cvSize(output->width, output->height), 8, 3);
	IplImage* colorImagesurf3 = cvCreateImage(cvSize(output->width, output->height), 8, 3);
	//cout<<"output-width,height"<<output->width<<output->height<<endl;
	cvCopy(output, colorImagesurf1,NULL);
	cvCopy(output, colorImagesurf2,NULL);
	cvCopy(output, colorImagesurf3,NULL);
	//TODO:comment out
	//cv::imshow("outputimage",cv::Mat(output));
	char keyname[128];
	char roiname[128];

//	char plusname[128];
	//cout<<"output-width,height"<<output->width<<output->height<<endl;
	CvFont font;
	cvInitFont (&font, CV_FONT_HERSHEY_DUPLEX, 0.5, 0.5);

	// �����_ID�t�@�C�����J��
	std::ofstream ofs( "../DB/output-img/keypoint.txt", std::ios::trunc );

	//TODO:�����ʃt�@�C�����J��
	std::ofstream dfs( "../DB/output-img/description.txt", std::ios::app );

	//TODO:keypoint
//	if(objId == id2name.size() || file == false){
		//if(entry_pairs.empty()){
		for (int j = 0; j < keypoints1.size(); j++) 
		{
			//CvSURFPoint* point = (CvSURFPoint*)cvGetSeqElem(keypoints1, j);  // i�Ԗڂ̃L�[�|�C���g
			CvPoint center;  // �L�[�|�C���g�̒��S���W
			center.x = keypoints1[j].pt.x;//(int)point->pt.x;
			center.y = keypoints1[j].pt.y;//(int)point->pt.y;
			//cvCircle(colorImagesurf1, center, cvRound(point->size*0.25), cvScalar(255,255,0));
			cvCircle(colorImagesurf1, center, 2, cvScalar(255,255,0), CV_FILLED);
			//cvCircle(colorImagesurf3, center, 2, cvScalar(0,0,255), CV_FILLED);
		}
		cvSetImageROI(colorImagesurf1, cvRect(region.x, region.y, region.width, region.height));
		cvSetImageROI(colorImagesurf2, cvRect(max(region.x-5,0), max(0,region.y-5), min(320,region.width+5), min(240,region.height+5)));
		//cout<<"cvRect(region.x, region.y, region.width, region.height))"<<region.x<<","<<region.y<<","<<region.height<<","<<region.width<<endl;
		//TODO:write output keypoint <objId>.bmp
//		sprintf(keyname,"../DB/output-img/keypoint/keypoint-%05d.png",objId);
		sprintf(keyname,"../DB/output-img/keypoint/keypoint-%05d.png",key_idname.size());
		cvSaveImage(keyname,colorImagesurf1);
		//TODO:write output roi <objId>.bmp
//		sprintf(roiname,"../DB/output-img/roi/roi-%05d.png",objId);
		sprintf(roiname,"../DB/output-img/roi/roi-%05d.png",key_idname.size());
		//TODO::imshow(roi);���̃��X�g�ɓo�^����摜
		//TODO:comment out
		//cv::imshow(roiname,cv::Mat(colorImagesurf2));
		cvSaveImage(roiname,colorImagesurf2);
		cv::waitKey(1);
/*	}
	else{
		if(new_obj == false){
			for (int j = 0; j < keypoints1->total; j++) 
			{
				CvSURFPoint* point = (CvSURFPoint*)cvGetSeqElem(keypoints1, j);  // i�Ԗڂ̃L�[�|�C���g
				CvPoint center;  // �L�[�|�C���g�̒��S���W
				center.x = point->pt.x;
				center.y = point->pt.y;
				cvCircle(colorImagesurf3, center, 2, cvScalar(255,255,0), CV_FILLED);
			}
			for (int j = 0; j < (int)entry_pairs.size(); j++) 
			{
				CvSURFPoint* point = (CvSURFPoint*)cvGetSeqElem(keypoints1, entry_pairs[j]);  // i�Ԗڂ̃L�[�|�C���g
				CvPoint center;  // �L�[�|�C���g�̒��S���W
				center.x = point->pt.x;
				center.y = point->pt.y;
				cvCircle(colorImagesurf3, center, 2, cvScalar(0,0,255), CV_FILLED);
			}
		}
		else{
			for (int j = 0; j < keypoints1->total; j++) 
			{
				CvSURFPoint* point = (CvSURFPoint*)cvGetSeqElem(keypoints1, j);  // i�Ԗڂ̃L�[�|�C���g
				CvPoint center;  // �L�[�|�C���g�̒��S���W
				center.x = point->pt.x;
				center.y = point->pt.y;
				cvCircle(colorImagesurf3, center, 2, cvScalar(0,0,255), CV_FILLED);
			}
		}
	}

	if(objId == id2name.size() || file == false){
*/		//if(entry_pairs.empty()){
		for(int k = 0; k < (int)id2name.size(); k++){
			ofs << k << "\t";

			ofs << id2name[k] << "\t";
			ofs << endl;
		}
		// ����ID�t�@�C���֓o�^
		//ofs << objId << "\t";//kawa
		ofs <<  key_idname.size() << "\t";
		// ���̂̃L�[�|�C���g��
		ofs << keypoints1.size() << "\t";
		ofs << endl;
/*	}

	else{
		for(int k = 0; k < (int)id2name.size(); k++){
			ofs << k << "\t";

			if(k == objId){
				if(new_obj == false){
					ofs << id2name[k] + entry_pairs.size() << "\t";
				}
				else
					ofs << id2name[k] + keypoints1->total << "\t";
			}
			else
				ofs << id2name[k] << "\t";
			ofs << endl;
		}
	}

	if(objId == id2name.size() || file == false){
*/		// �I�u�W�F�N�gID, ���v���V�A��, 128�̐������^�u��؂�ŏo��

		//dfs << key_idname.size() << "\t";

		for (int i = 0; i < keypoints1.size(); i++) {  // �e�L�[�|�C���g�̓����ʂɑ΂�
			// �I�u�W�F�N�gID
			//fprintf(flkey,"%ld,",t);
		//	dfs << objId << "\t";//kawa
			dfs << key_idname.size() << "\t";  //kawa?
			// �����_�̃��v���V�A���iSURF�����ʂł̓x�N�g���̔�r���Ɏg�p�j
			//const CvSURFPoint* kp = (const CvSURFPoint*)cvGetSeqElem(keypoints1, i);
			int laplacian = keypoints1[i].class_id;//kp->laplacian;
			//fprintf(flkey,"%ld,",laplacian);
			dfs << laplacian << "\t";

			// �����_��x��
			int x =(int)keypoints1[i].pt.x;//(int) kp->pt.x;
			dfs << x << "\t";

			// �����_��y��
			int y = (int)keypoints1[i].pt.y;//kp->pt.y;
			dfs << y << "\t";

			// 128�����x�N�g��
			//const float *descriptor = (const float *)cvGetSeqElem(descriptors1, i);
			for (int d = 0; d < DIM_VECTOR; d++) {
				//fprintf(flkey,"%ld\n",descriptor[d]);
				//dfs << descriptor[d] << "\t";
				dfs << descriptors1[i*DIM_VECTOR+d]<< "\t";
			}		
			dfs << endl;
		}
/*	}

	else{
		if(new_obj == false){
			// �I�u�W�F�N�gID, ���v���V�A��, 128�̐������^�u��؂�ŏo��
			for (int j = 0; j < (int)entry_pairs.size(); j++) {  // �e�L�[�|�C���g�̓����ʂɑ΂�
				// �I�u�W�F�N�gID
				//fprintf(flkey,"%ld,",t);
				dfs << objId << "\t";

				// �����_�̃��v���V�A���iSURF�����ʂł̓x�N�g���̔�r���Ɏg�p�j
				const CvSURFPoint* kp = (const CvSURFPoint*)cvGetSeqElem(keypoints1, entry_pairs[j]);
				int laplacian = kp->laplacian;
				//fprintf(flkey,"%ld,",laplacian);
				dfs << laplacian << "\t";

				// �����_��x��
				int x = kp->pt.x;
				dfs << x << "\t";

				// �����_��y��
				int y = kp->pt.y;
				dfs << y << "\t";

				// 128�����x�N�g��
				const float *descriptor = (const float *)cvGetSeqElem(descriptors1, entry_pairs[j]);
				for (int d = 0; d < DIM_VECTOR; d++) {
					//fprintf(flkey,"%ld\n",descriptor[d]);
					dfs << descriptor[d] << "\t";
				}		
				dfs << endl;
			}
		}
		else{
			for (int i = 0; i < descriptors1->total; i++) {  // �e�L�[�|�C���g�̓����ʂɑ΂�
				// �I�u�W�F�N�gID
				//fprintf(flkey,"%ld,",t);
				dfs << objId << "\t";

				// �����_�̃��v���V�A���iSURF�����ʂł̓x�N�g���̔�r���Ɏg�p�j
				const CvSURFPoint* kp = (const CvSURFPoint*)cvGetSeqElem(keypoints1, i);
				int laplacian = kp->laplacian;
				//fprintf(flkey,"%ld,",laplacian);
				dfs << laplacian << "\t";

				// �����_��x��
				int x = kp->pt.x;
				dfs << x << "\t";

				// �����_��y��
				int y = kp->pt.y;
				dfs << y << "\t";

				// 128�����x�N�g��
				const float *descriptor = (const float *)cvGetSeqElem(descriptors1, i);
				for (int d = 0; d < DIM_VECTOR; d++) {
					//fprintf(flkey,"%ld\n",descriptor[d]);
					dfs << descriptor[d] << "\t";
				}		
				dfs << endl;
			}
		}
	}
*/
	/*int x = 0;
	int y = 0;
	x = ((region.x+region.width)/2)-320/2;
	y = (region.y+region.height/2)-240/2;
	cvSetImageROI(colorImagesurf3, cvRect(x, y, 320, 240));*/

	//if(objId != id2name.size()){
	//TODO:write output keypoint_plus <frame> <objId>.bmp
	//sprintf_s(plusname,"../DB/output-img/keypoint_plus/%08d-keypoint-%05d.png",frame ,objId);
	//sprintf_s(plusname,"../DB/output-img/keypoint_plus/%08d-keypoint-%05d.png",frame ,key_idname.size());
	//cvSaveImage(plusname,colorImagesurf3);
	//}

	/*char str[64];
	CvFont number;
	cvInitFont (&font, CV_FONT_HERSHEY_DUPLEX, 1.0, 1.0);
	sprintf(str,"%d",objId);
	cvPutText (colorImagesurf3, str, cvPoint ( x+10, y+10 ), &number, CV_RGB(255, 0, 0) );*/

	//cvShowImage("keypoint", colorImagesurf3);
	//cvWaitKey(1);


	//cout<<"saveFile"<<endl;
	cvResetImageROI(colorImagesurf1);
	cvReleaseImage(&colorImagesurf1);
	cvResetImageROI(colorImagesurf2);
	cvReleaseImage(&colorImagesurf2);
	cvResetImageROI(colorImagesurf3);
	cvReleaseImage(&colorImagesurf3);
	//cvReleaseImage(&ImageSurf);
	//cvDestroyWindow("keypoint");
}

/**
* ���̃��f�����t�@�C�����猟������
*
* @param[in]  png                 �摜�t�@�C����
* @param[in]  captureImage        �摜�t�@�C����
* @param[in]  imageKeypoints      �L�[�|�C���g
* @param[in]  imageDescriptors    �e�L�[�|�C���g�̓�����
*
* @return �Ȃ�
*/
int SurfPoint::SurfMatching(std::vector<cv::KeyPoint> imageKeypoints, std::vector<float> imageDescriptors,unsigned long frame,image &input,CvRect region ,int delete_key,std::vector<int> key_idname,std::vector<int> obj_idname,int num/*����g�p����Otable�ɕۑ�����Ă��镨�̂̏��*/)
{
		map<int, int> id2name;
		vector<int> labels;     // �L�[�|�C���g�̃��x���iobjMat�ɑΉ��j
		vector<int> laplacians;  // �L�[�|�C���g�̃��v���V�A��

		bool file = false;
		//���ꕨ�̊m��
		CvMemStorage* storage = cvCreateMemStorage(0);
		//3�����̏ꍇ��CV_32FC2��CV_32FC3�ɁACvPoint2D32f��CvPoint3D32f�ɂȂ�
		CvSeq* point_seq = cvCreateSeq( CV_32FC2, sizeof(CvSeq), sizeof(CvPoint2D32f), storage );
		CvMat* objMat=NULL;           // �e�s�����̂̃L�[�|�C���g�̓����x�N�g��

		file = loadKeypointId(id2name/*,labels*/);

		cout << " loadKeypointId"<< endl;

		// �L�[�|�C���g�̓����x�N�g����objMat�s��Ƀ��[�h
		if(file == true)file = loadDescription(storage, point_seq, labels, laplacians, objMat, id2name);

		cout << " loadDescription"<< endl;

		IplImage* kakutei = 0;
		int maxVote = 0;

		std::vector<double> dist; // �������[
		double kyori = 1e6;
		int kyoriId = -1;
		int numObjects = -1;
		std::vector<int> votes; // �e���̂̏W�߂����[��
		std::vector<int> box_votes;

		if(file == true)
		{
			// �N�G���̊e�L�[�|�C���g��1-NN�̕���ID���������ē��[
			numObjects = (int)id2name.size();  // �f�[�^�x�[�X���̕��̐�
			Registered = (int)id2name.size();

			for(int e = 0; e < (int)id2name.size(); e++)
			{
				votes.push_back(0);
				box_votes.push_back(0);
			}

			// �N�G���̊e�L�[�|�C���g�����̃f�[�^�x�[�X�̂ǂ̓_�ƑΉ����邩
			vector<int> ptpairs;  // keypoints�̃C���f�b�N�X���i�[

			//---------------------------------------------------------------------------------------
			//---------------------------------------------------------------------------------------
			// Kd-tree�p

			if(labels.size()>0)
			{
				// ���̃��f���f�[�^�x�[�X���C���f�L�V���O
				//cout << "���̃��f���f�[�^�x�[�X���C���f�L�V���O���܂� ... " << flush;
				CvFeatureTree* ft = cvCreateKDTree(objMat);  // objMat�̓R�s�[����Ȃ��̂ŉ�����Ă̓_��
				//cout << "OK" << endl;
				//cout<<"@ ���̂炵�����̌��m3.1."<<imageDescriptors->total<<endl;
				// �N�G���̃L�[�|�C���g�̓����x�N�g����CvMat�ɓW�J
				CvMat* queryMat = cvCreateMat(imageKeypoints.size(), DIM_VECTOR, CV_32FC1);
				//cout<<"@ ���̂炵�����̌��m3.2."<<imageDescriptors->total<<endl;
				//CvSeqReader reader;
				//float* ptr = queryMat->data.fl;
				//CvMat* Descriptors=cvCreateMat(imageKeypoints.size()/*tate*/, DIM_VECTOR/*yoko */, CV_32FC1);
				
				//cvStartReadSeq(imageDescriptors, &reader);
				for(int i = 0; i < imageDescriptors.size(); i++)queryMat->data.fl[i]=imageDescriptors[i];
				
				//for (int i = 0; i < imageKeypoints.size(); i++)
				//{
				//	//float* descriptor = (float*)reader.ptr;
				//	ptr[i];
				//	//CV_NEXT_SEQ_ELEM(reader.seq->elem_size, reader);
				//	memcpy(ptr, descriptor, DIM_VECTOR * sizeof(float));  // DIM�����̓����x�N�g�����R�s�[
				//	ptr += DIM_VECTOR;
				//}

				//cout << " for (int i = 0; i < imageDescriptors->total; i++) {"<< endl;

				// kd-tree��1-NN�̃L�[�|�C���g�C���f�b�N�X������
				int k = 1;  // k-NN��k
				CvMat* indices = cvCreateMat(imageKeypoints.size(), k, CV_32SC1);   // 1-NN�̃C���f�b�N�X
				CvMat* dists = cvCreateMat(imageKeypoints.size(), k, CV_64FC1);     // ���̋���
				cvFindFeatures(ft, queryMat, indices, dists, k, 250);

				// 1-NN�L�[�|�C���g���܂ޕ��̂ɓ��[
				for (int i = 0; i < indices->rows; i++) 
				{
					int idx = CV_MAT_ELEM(*indices, int, i, 0);
					//std::cerr << "idx "  << i << "=" << idx << std::endl;
					double dist = CV_MAT_ELEM(*dists, double, i, 0);
					if(dist < THRESHOLD)
						if((int)labels.size()>idx&&(int)labels.size()!=0)votes[labels[idx]]++;
				}

				//cout << " for (int i = 0; i < indices->rows; i++) {"<< endl;

				cvReleaseMat(&queryMat);
				cvReleaseMat(&indices);
				cvReleaseMat(&dists);
				cvReleaseFeatureTree(ft);
			}
			// �����܂�
			//---------------------------------------------------------------------------------------

			//for (int i = 0; i < imageDescriptors->total; i++) {
			//	CvSURFPoint *p = (CvSURFPoint *)cvGetSeqElem(imageKeypoints, i);
			//       float *vec = (float *)cvGetSeqElem(imageDescriptors, i);
			//       int lap = p->laplacian;
			//       int nnId = searchNN(vec, lap, labels, laplacians, objMat, p);
			//	if (nnId > -1){
			//		//std::cerr << " i=" << i << std::endl;
			//		votes[nnId]++;
			//	}
			//   }

			// ����ID���Ƃ̓��[�������߂�
			int maxId = -1;
			int maxVal = -1;
			for (int i = 0; i < (int)votes.size(); i++) 
			{ 
				if (votes[i] > maxVal && votes[i] > 15) 
				{
					maxId = i;
					maxVal = votes[i];
				}
			}

			//cout << " for (int i = 0; i < (int)votes.size(); i++) { //numObjects; i++) {"<< endl;

			int wOffset = 0;

			CvScalar color;
			CvScalar colors[] = 
			{
				CV_RGB(255,0,0),CV_RGB(0,255,0),CV_RGB(0,0,255),
				CV_RGB(0,255,255),CV_RGB(120,120,0),CV_RGB(255,0,255),
				CV_RGB(120,0,0),CV_RGB(0,120,0),CV_RGB(0,0,120),CV_RGB(255,255,255)
			};

			for(int q = 0; q < (int)id2name.size(); q++)
			{
				dist.push_back(1e6);//10��6��
			}

			int s = 0; int t = 0;
			int flag = -1;

			//cout << " for(int q = 0; q < (int)id2name.size(); q++){"<< endl;

			// ���̖���NearestNeighbor�@�œ��[
			for (int i = 0; i <(int)votes.size();i++)// numObjects; i++) 
			{
				if(votes[i]>4)
				{
					int vote = votes[i];
					findPairs(imageKeypoints, imageDescriptors,labels, laplacians, objMat, ptpairs, i);
					for (int j = 0; j < (int)ptpairs.size()-1; j += 2) 
					{
						//CvSURFPoint* point = (CvSURFPoint*)cvGetSeqElem(imageKeypoints, ptpairs[j]);  // j�Ԗڂ̃L�[�|�C���g
						CvSURFPoint* pt1 = (CvSURFPoint*)cvGetSeqElem(point_seq,ptpairs[j+1]);
						//(x,y)=�Q�Ɖ摜
						int x = (int)pt1->pt.x;
						int y = (int)pt1->pt.y;
						//CvPoint to = cvPoint(cvRound(point->pt.x), cvRound(point->pt.y));
						CvPoint to = cvPoint(cvRound(imageKeypoints[ptpairs[j]].pt.x), cvRound(imageKeypoints[ptpairs[j]].pt.y));
						//(vx,vy)=���͉摜
						int vx = (int)imageKeypoints[ptpairs[j]].pt.x;//point->pt.x;
						int vy = (int)imageKeypoints[ptpairs[j]].pt.y;//point->pt.y;
						color = colors[i%10];//kawa
						Pair* pf = new Pair(x,y,vx,vy);
						pair.push_back(pf);
					}

					dist[i] = affine(pair,&IplImage(input.img_original),color,i,vote,box_votes,imageKeypoints,flag);
					//if( dist[i] < 30  && votes[i] > 4 ){
					if( dist[i] < 200  && box_votes[i] > 14 ){
						for (int j = 0; j < (int)ptpairs.size()-1; j += 2) {
							ptpairs2.push_back(ptpairs[j]);
						}
					}
					flag = -1;

					//release memory of pair
					if( !pair.empty() ){//��ɂ���
						for(int j = 0; j < (int)pair.size() ; j++)
							delete pair[j];
					}
					pair.clear();
				}
			}	

			

			for(int j = 0; j < (int)dist.size(); j++)
			{
				if( j<(int)box_votes.size()&& dist[j] < kyori && dist[j] < 200  && box_votes[j] > 14 ){
					kyoriId = j;
					kyori = dist[j];				
					maxVote = box_votes[j];
				}
			}

			 //cout << " loadDescription"<< endl;

			if(kyoriId >= 0){
				for (int k = 0; k < imageKeypoints.size(); k++){
					nums.insert(k);
				}
				vector<int>::iterator it1 = ptpairs2.begin();
				while(it1 != ptpairs2.end()){
					int tmp=*it1;
					//			set<int>::iterator itiNext =
					//				remove ( nums.begin(),nums.end(),tmp);
					set<int>::iterator itiNext =nums.begin();
					nums.erase( itiNext, nums.end() );
					++it1;
				}

				set<int>::iterator itiNext = nums.begin();
				while( itiNext != nums.end() ){
					entry_pairs.push_back(*itiNext);
					++itiNext;
				}
			}
			nums.clear();
		}
		else
		{
			kyoriId = 0;
			//TODO:comment

		}

		if(kyoriId < 0)kyoriId = numObjects;

		bool objId = false;

		if(maxVote > 4 && kyori < 200 && file == true){
			new_obj = false;
			saveFile(kyoriId, imageKeypoints, imageDescriptors, &IplImage(input.img_original), entry_pairs, cvRect(region.x, region.y, region.width, region.height), id2name, frame, new_obj, file,key_idname,obj_idname);
		}
		else
		{
			new_obj = false;
			saveFile(kyoriId, imageKeypoints, imageDescriptors, &IplImage(input.img_original), entry_pairs, cvRect(region.x, region.y, region.width, region.height), id2name, frame, new_obj, file,key_idname,obj_idname);
		}
		entry_pairs.clear();
		ptpairs2.clear();
		ptpairs.clear();
		// �e�L�X�g�ɏ����o��
		// ����ID�t�@�C�����J��
		std::ofstream ofs( "../DB/output-img/comparison.txt", std::ios::app );

		ofs<< "���̓��f��Frame: " << frame << endl;
		ofs << "���ʌ���: " << kyoriId << endl;

		for (int i = 0; i < numObjects; i++) 
		{
			// ����ID�t�@�C���֓o�^
			ofs << i << "\t";

			// ���̂̃L�[�|�C���g��
			ofs << box_votes[i] << "\t";
			ofs << dist[i] << "\t";
			ofs << endl;
		}
		ofs << endl;


		cvReleaseImage(&kakutei);

		cvClearSeq(point_seq);
		cvReleaseMemStorage(&storage);
		if(objMat != NULL && file == true)
		cvReleaseMat(&objMat);
		id2name.clear();
		labels.clear();
		laplacians.clear();
		votes.clear();
		box_votes.clear();


		//cout<<"@ ���̂炵�����̌��m5"<<endl;
		return kyoriId;
}

/**
* �N�G���̃L�[�|�C���g��1-NN�L�[�|�C���g�𕨑̃��f���f�[�^�x�[�X����T���Ă��̕���ID��Ԃ�
*
* @param[in] vec          �N�G���L�[�|�C���g�̓����x�N�g��
* @param[in] lap          �N�G���L�[�|�C���g�̃��v���V�A��
* @param[in] labels       ���̃��f���f�[�^�x�[�X�̊e�L�[�|�C���g�̕���ID
* @param[in] laplacians   ���̃��f���f�[�^�x�[�X�̊e�L�[�|�C���g�̃��v���V�A��
* @param[in] objMat       ���̃��f���f�[�^�x�[�X�̊e�L�[�|�C���g�̓����x�N�g��
*
* @return �w�肵���L�[�|�C���g�ɂ����Ƃ��߂��L�[�|�C���g�̕���ID
*/
int SurfPoint::searchNN(float *vec, int lap, vector<int> &labels, vector<int> &laplacians, CvMat* objMat, CvSURFPoint *p) {
	int neighborObj = -1;
	double minDist = 1e6;
	int minIdx = -1;
	for (int i = 0; i < objMat->rows; i++) {
		// �N�G���̃L�[�|�C���g�ƃ��v���V�A�����قȂ�L�[�|�C���g�͖���
		if (lap != laplacians[i]) {
			continue;
		}
		// i�s�ڂ̐擪�f�[�^�̃|�C���^�i��������128��i�s�ڂ̓����x�N�g���j
		float *mvec = (float *)(objMat->data.fl + i * DIM_VECTOR);
		//Neighbor* nearest;
		double d = euclidDistance(vec, mvec, DIM_VECTOR);
		if (d < minDist) {
			neighborObj = labels[i];  // NN�L�[�|�C���g�̕���ID���X�V
			minDist = d;           // �ŏ��̋������X�V
			minIdx = i;
		}
	}
	// return neighborObj;
	// �ŋߖT�_�ł�������臒l�ȏゾ�����疳������
	if (minDist < THRESHOLD) {
		//std::cerr << "min index:" << minIdx << "label:" << labels[minIdx] ;
		return neighborObj;
	}
	// �ŋߖT�_��������Ȃ��ꍇ
	if(neighborObj <0 )
	{
		return -1;
	}
	//labels[neighborObj];
	return -1;
}

bool SurfPoint::loadKeypointId(map<int, int>& id2name/*, vector<int>& labels*/) {
	// ���f��ID�Ƃ��̃��f���̓�����������
	std::ifstream ofs( "../DB/output-img/keypoint.txt");

	// 1�s���ǂݍ��݁A����ID->���̖���map���쐬
	string line;
	while (getline(ofs, line, '\n')) {
		// �^�u�ŕ��������������ldata�֊i�[
		vector<string> ldata;
		istringstream ss(line);
		string s;
		while (getline(ss, s, '\t')) {
			ldata.push_back(s);
		}
		// ����ID�ƃL�[�|�C���g���𒊏o���Ċi�[
		int objId = atol(ldata[0].c_str());
		int keypoint = atoi(ldata[1].c_str());
		id2name[objId] = keypoint;
		/*for(int j = 0; j < keypoint ; j++)
		{
		labels.push_back(objId);
		}*/
		//id2name.insert(map<int, int>::value_type(objId, keypoint));
	}

	if(id2name.size() >= 1)
		return true;
	else
		return false;
}

/**
* �L�[�|�C���g�̃��x���i���o���̕���ID�j�ƃ��v���V�A���Ɠ����x�N�g�������[�h��labels��objMat�֊i�[
*
* @param[in]  filename         �����x�N�g�����i�[�����t�@�C��
* @param[out] labels           �����x�N�g�����o���̕���ID
* @param[out] laplacians       �����x�N�g���̃��v���V�A��
* @param[out] objMat           �����ʂ��i�[�����s��i�e�s��1�̓����x�N�g���j
*(storage, point_seq, labels, laplacians, objMat, id2name);
* @return �����Ȃ�true�A���s�Ȃ�false
*/
bool SurfPoint::loadDescription(CvMemStorage* storage, CvSeq* point_seq, vector<int> &labels, vector<int> &laplacians, CvMat* &objMat, map<int, int>& id2name) {
	int keypointsize = 0;
#if DEBUG
	//cout<<__FUNCTION__<<endl;
#endif
	// ����ID�Ɠ����x�N�g�����i�[�����t�@�C�����J��
	std::ifstream ifs( "../DB/output-img/description.txt");

	// �s��̃T�C�Y�����肷�邽�߃L�[�|�C���g�̑������J�E���g
	for(int i = 0; i < (int)id2name.size(); i++)
		keypointsize += id2name[i];
	int numKeypoints = keypointsize;
	//cout << "keypoints:::: " << numKeypoints << endl;
	if(numKeypoints==0){return true;}
	string line;
	objMat = cvCreateMat(numKeypoints, DIM_VECTOR, CV_32FC1);
	cvZero(objMat);
	// �f�[�^��ǂݍ���ōs��֊i�[
	for (int i=0;getline(ifs, line, '\n');i++) {
		// �^�u�ŕ��������������ldata�֊i�[
		vector<string> ldata;
		//		cout<<line<<endl;
		istringstream ss(line);
		string s;
		//s = ss.getline('\t');
		while (getline(ss, s, '\t')) {
			ldata.push_back(s);
			//			
		}
		//cout<<ldata.size()<<endl;
		//		cout<<"end"<<endl;
		// ����ID�����o���ē����x�N�g���̃��x���Ƃ���
		int objId = atol(ldata[0].c_str());
		labels.push_back(objId);
		
		// ���v���V�A�������o���Ċi�[
		int laplacian = atoi(ldata[1].c_str());
		laplacians.push_back(laplacian);

		//cout<<"objId,laplacian"<<objId<<","<<laplacian<<endl;

		// ���W�����o���Ċi�[

		int x = atoi(ldata[2].c_str());
		int y = atoi(ldata[3].c_str());

		cvSeqPush(point_seq, &cvPoint2D32f(x,y));

		//obj.push_back();
		// DIM�����x�N�g���̗v�f���s��֊i�[
		for (int j = 0; j < DIM_VECTOR; j++) {
			float val = (float)atof(ldata[j+4].c_str());  // �����x�N�g����ldata[4]����
			//CV_MAT_ELEM(mat,elemtype,row,col);
			CV_MAT_ELEM(*objMat, float, i, j) = val;
		}
	}
	return true;
}

double SurfPoint::affine(std::vector<Pair*> pair/*, IplImage* compari*/, cv::Mat output, /*IplImage* matchingImage,*/ CvScalar color,int object,int vote
	, std::vector<int>& box_votes, std::vector<cv::KeyPoint> keypoints1/*, std::vector<Pair*>& entry*/, int flag){
		//(vx,vy)=���͉摜
		//random_shuffle( pair.begin(), pair.end() );
		CvMat *affineMatrix = cvCreateMat(2, 3, CV_32FC1); 
		CvMat *Max_affineMatrix = cvCreateMat(2, 3, CV_32FC1);
		CvMemStorage* affinestorage = cvCreateMemStorage(0);
		//CvSize sz = cvSize(output->width, output->height);
		//IplImage *dstImg = cvCreateImage(sz, IPL_DEPTH_8U, 3);
		//IplImage* img =  cvCreateImage(sz, IPL_DEPTH_8U, 1);
		//IplImage* img_copy =  cvCreateImage(sz, IPL_DEPTH_8U, 1);
		//IplImage *dstImg_gray = cvCreateImage(sz, IPL_DEPTH_8U, 1);
		CvSeq* find_contour = NULL;
		int MaxVote = 0;
		CvFont font;
		cvInitFont (&font, CV_FONT_HERSHEY_DUPLEX, 0.5, 0.5);

		std::vector<double> counts(100,1e6);
		double c = 1e6;
		//int flag = -1;
		int cou = 0;
		int Max = 0;
		std::vector<int> indexer(pair.size(),0);
		for(int i = 0; i < (int)indexer.size() ; i++)
		{
			indexer[i] = i;
		}
		//random_shuffle(indexer.begin(),indexer.end() );
		//std::vector<int> affineResult(pair.size(),0) , affineVotingResult(pair.size(),0);
		for(int j = 0; j < (int)counts.size(); j++){
			//random_shuffle( pair.begin(), pair.end() );
			//cout << "indexer:" << indexer[0] << endl;
			random_shuffle(indexer.begin(),indexer.end() );

			if((pair[indexer[0]]->get_x() != pair[indexer[1]]->get_x() && pair[indexer[0]]->get_x() != pair[indexer[2]]->get_x() && pair[indexer[1]]->get_x() != pair[indexer[2]]->get_x()
				&& pair[indexer[0]]->get_x() != pair[indexer[3]]->get_x() && pair[indexer[1]]->get_x() != pair[indexer[3]]->get_x() && pair[indexer[2]]->get_x() != pair[indexer[3]]->get_x()
				&& pair[indexer[0]]->get_y() != pair[indexer[1]]->get_y() && pair[indexer[0]]->get_y() != pair[indexer[2]]->get_y() && pair[indexer[1]]->get_y() != pair[indexer[2]]->get_y()
				&& pair[indexer[0]]->get_y() != pair[indexer[3]]->get_y() && pair[indexer[1]]->get_y() != pair[indexer[3]]->get_y() && pair[indexer[2]]->get_y() != pair[indexer[3]]->get_y()
				&& pair[indexer[0]]->get_vx() != pair[indexer[1]]->get_vx() && pair[indexer[0]]->get_vx() != pair[indexer[2]]->get_vx() && pair[indexer[1]]->get_vx() != pair[indexer[2]]->get_vx()
				&& pair[indexer[0]]->get_vx() != pair[indexer[3]]->get_vx() && pair[indexer[1]]->get_vx() != pair[indexer[3]]->get_vx() && pair[indexer[2]]->get_vx() != pair[indexer[3]]->get_vx()
				&& pair[indexer[0]]->get_vy() != pair[indexer[1]]->get_vy() && pair[indexer[0]]->get_vy() != pair[indexer[2]]->get_vy() && pair[indexer[1]]->get_vy() != pair[indexer[2]]->get_vy()
				&& pair[indexer[0]]->get_vy() != pair[indexer[3]]->get_vy() && pair[indexer[1]]->get_vy() != pair[indexer[3]]->get_vy() && pair[indexer[2]]->get_vy() != pair[indexer[3]]->get_vy())){

					//flag = 1;

					//cout << "counts:" << counts[j] << endl;

					for (int i = 0; i < (int)pair.size(); i ++)
					{

						// �ϊ��O���W 3�_ 
						CvPoint2D32f original[] = {cvPoint2D32f(pair[indexer[0]]->get_x(), pair[indexer[0]]->get_y()), 
							cvPoint2D32f(pair[indexer[1]]->get_x(), pair[indexer[1]]->get_y()), 
							cvPoint2D32f(pair[indexer[2]]->get_x(), pair[indexer[2]]->get_y()),
							cvPoint2D32f(pair[indexer[3]]->get_x(), pair[indexer[3]]->get_y())}; 
						// �ϊ�����W 3�_ 
						CvPoint2D32f translate[] = {cvPoint2D32f(pair[indexer[0]]->get_vx(), pair[indexer[0]]->get_vy()), 
							cvPoint2D32f(pair[indexer[1]]->get_vx(), pair[indexer[1]]->get_vy()),
							cvPoint2D32f(pair[indexer[2]]->get_vx(), pair[indexer[2]]->get_vy()),
							cvPoint2D32f(pair[indexer[3]]->get_vx(), pair[indexer[3]]->get_vy())};

						affineMatrix = cvCreateMat(3, 3, CV_32FC1); 
						// �ϊ��s������߂� 
						cvGetPerspectiveTransform(original, translate, affineMatrix); 
					}
					//std::vector<Pair*> affine_pair;
					//counts[j] = Ransac(pair, /*affine_pair,*/ affineMatrix/*, matchingImage*//*,vote_ransac*/,object);
					counts[j] = Ransac(pair, /*affine_pair, */affineMatrix/*, output*//*,vote_ransac_affine*/,object);
					if(counts[j] < c){			
						Max_affineMatrix = affineMatrix;
						c = counts[j];
						//cout << "counts:" << c << endl;

						//cout << object << ":" << pair[indexer[0]]->get_x() << endl;
					}
			}
			//if(flag == -1)
			else
				//cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
				//return 1e6;
				counts[j] = 1e6;
		}

		//cout << object << ":" << c << endl;
		// �ϊ��s���K�p����(�A�t�B���ϊ�) 
		if(c != 1e6){
			/*cvWarpPerspective(compari, dstImg, Max_affineMatrix, 
			CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS, cvScalarAll(0)); 

			cvCvtColor(dstImg, dstImg_gray, CV_BGR2GRAY);

			cvThreshold(dstImg_gray, img, 1, 255, CV_THRESH_BINARY);

			img_copy = cvCloneImage(img);*/

			int Keypoint_count = keypoints1.size();
			int Pair_count = pair.size();

			//cout<< "Keypoint" << object << ":" << Keypoint_count << endl;
			if(Keypoint_count != 0)
				MaxVote = Pair_count*100/Keypoint_count;
			else
				MaxVote = 0;

			box_votes[object] = MaxVote;
		}
		cvReleaseMemStorage(&affinestorage);

		counts.clear();
		indexer.clear();

		return c;
}
//
double SurfPoint::Ransac(const std::vector<Pair*>& pair,/*const std::vector<Pair*>& affine_pair,*/ CvMat *affineMatrix/*, int count*//*, IplImage *output*/,int Id)
{
	//(x,y)=�Q�Ɖ摜
	//(vx,vy)=���͉摜
	vector<Pair*> affine_pair;

	CvMat *matA = cvCreateMat(3, 1, CV_32FC1);
	CvMat *matB = cvCreateMat(3, 1, CV_32FC1);
	CvMat *matX = cvCreateMat(3, 1, CV_32FC1); //�摜1
	CvMat *matY = cvCreateMat(3, 1, CV_32FC1); //�摜2

	CvMat *matRan = cvCreateMat(3, 3, CV_32FC1);

	cvmInvert(affineMatrix, matRan);

	for (int i = 0; i < (int)pair.size(); i ++) {
		// �ϊ��s���K�p����(�A�t�B���ϊ�) 
		CvPoint trans_center;

		double x = pair[i]->get_x();
		double y = pair[i]->get_y();

		double vx = pair[i]->get_vx();
		double vy = pair[i]->get_vy();

		cvmSet(matB, 0, 0, vx);
		cvmSet(matB, 1, 0, vy);
		cvmSet(matB, 2, 0, 1);
		cvmMul(matRan, matB, matY);

		trans_center.x = cvRound(cvmGet(matY, 0, 0));
		trans_center.y = cvRound(cvmGet(matY, 1, 0));
		Pair* p = new Pair((int)x,(int)y,trans_center.x,trans_center.y);
		affine_pair.push_back(p);
	}

	std::vector<double> dist; // �e���̂̏W�߂����[��
	//double dist = 0;
	//int count = 0;


	// �e�L�X�g�ɏ����o��
	// ����ID�t�@�C�����J��

	/*std::ofstream ofs( "../output/comparison/kyori.txt", std::ios::app );
	if(Id == 9){
	ofs << "ID" << Id << "\t";
	ofs << "Size" << affine_pair.size() << "\t";
	ofs << endl;
	}*/

	for(int i = 4; i < (int)affine_pair.size(); i++)	{
		double affine_x = (affine_pair[i]->get_x()-affine_pair[i]->get_vx())*(affine_pair[i]->get_x()-affine_pair[i]->get_vx());
		double affine_y = (affine_pair[i]->get_y()-affine_pair[i]->get_vy())*(affine_pair[i]->get_y()-affine_pair[i]->get_vy());

		double affine_dis = sqrt(affine_x + affine_y);
		dist.push_back(affine_dis);
	}
	sort( dist.begin(), dist.end() );
	double kyori = 1e6;
	if(dist.size()!=0)kyori = dist[dist.size()/2];
	cvReleaseMat(&matA);
	cvReleaseMat(&matB);
	cvReleaseMat(&matX);
	cvReleaseMat(&matY);
	cvReleaseMat(&matRan);

	if(!affine_pair.empty() )
	{
		for(int i = 0; i < (int)affine_pair.size() ; i++)
			delete affine_pair[i];
	}

	affine_pair.clear();
	dist.clear();
	return kyori;
}

/**
* 2�̃x�N�g���̃��[�N���b�h�������v�Z���ĕԂ�
*
* @param[in] vec     �x�N�g��1�̔z��
* @param[in] mvec    �x�N�g��2�̔z��
* @param[in] length  �x�N�g���̒���
*
* @return ���[�N���b�h����
*/
double SurfPoint::euclidDistance(float* vec1, float* vec2, int length) {
	double sum = 0.0;
	for (int i = 0; i < length; i++) {
		sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
	}
	return sqrt(sum);
}

/**
*	@param[in] mvec �^�[�Q�b�g�ƂȂ������
*	@param[in] laplacians ���v���V�A��
*	@param[in] keypoints �T������L�[�|�C���g�W��
*	@param[in] descriptors �T����������ʏW��
*
*/
int SurfPoint::nearestNeighbor(float *mvec, int lap,const std::vector<int>& laplacians, CvMat* objMat) {
	int neighbor = -1;
	double minDist = 1e6;
	for (int i = 0; i < objMat->rows; i++) {
		// ���v���V�A�����قȂ�L�[�|�C���g�͖���
		if (lap != laplacians[i]) continue;
		float* v = (float *)(objMat->data.fl + i * DIM_VECTOR);
		double d = euclidDistance(v, mvec, DIM_VECTOR);
		// ���߂��_����������u��������
		if (d < minDist) {
			minDist = d;
			neighbor = i;
		}
	}

	// �ŋߖT�_�ł�������臒l�ȏゾ�����疳������
	if (minDist < THRESHOLD) {
		return neighbor;
	}

	// �ŋߖT�_��������Ȃ��ꍇ
	return -1;
}

/**
* �摜1�̃L�[�|�C���g�Ƌ߂��摜2�̃L�[�|�C���g��g�ɂ��ĕԂ�
*
* @param[in]  keypoints1       �摜1�̃L�[�|�C���g
* @param[in]  descriptors1     �摜1�̓����x�N�g��
* @param[in]  keypoints2       �摜2�̃L�[�|�C���g
* @param[in]  descriptors2     �摜2�̓����x�N�g��
* @param[out] ptpairs          �ގ��L�[�|�C���g�C���f�b�N�X�̗�i2��1�g�j
*
* @return �Ȃ�
*/
void SurfPoint::findPairs(std::vector<cv::KeyPoint> keypoints1, std::vector<float> descriptors1,vector<int> &labels, vector<int> &laplacians, CvMat* objMat,std::vector<int> &ptpairs,int maxVal) {
	ptpairs.clear();
	int nn = -1;
	//int count = 0;

	//cout << "ID���ʌ���: " << maxVal << endl;
	// �摜1�̊e�L�[�|�C���g�Ɋւ��čŋߖT�_������
	for (int i = 0; i < keypoints1.size(); i++) {
		//CvSURFPoint *p = (CvSURFPoint *)cvGetSeqElem(keypoints1, i);
		//float *vec = (float *)cvGetSeqElem(descriptors1, i);
		float vec[128];
		for(int t=0;t<128;t++){vec[t]=descriptors1[i*DIM_VECTOR+t];}
		int lap = keypoints1[i].class_id;// p->laplacian;
		int nn = nearestNeighbor(vec, lap, laplacians, objMat);
		if(nn >= 0 && labels[nn] == maxVal)
			;
		else
			nn = -1;
		//cout << "nn: " << nn << endl;
		// ����������摜1�̃C���f�b�N�X�Ɖ摜2�̃C���f�b�N�X�����Ԃɓo�^
		if (nn >= 0) {
			//std::cerr << i << " " << labels[i] << " " << nn << " " << labels[nn] << std::endl;
			ptpairs.push_back(i);
			ptpairs.push_back(nn);
			//count++;
			//cout << "maxID���ʌ���: " << nn << endl;
		}
	}
	//cout << "count: " << count << endl;
}