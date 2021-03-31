#define _USE_MATH_DEFINES
#include <cmath>
#include "opencv2\opencv.hpp"
#include "picojson.h"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <direct.h>
#include <sys\stat.h>
#include <Windows.h>
#include <vector>
#include "image.h"
#include "ImageEditUtility.h"
#include <stdlib.h>
#include <iomanip>
using namespace std;
using namespace cv;

template
<   
    typename TYPE,
    std::size_t SIZE
>
std::size_t arrlen(const TYPE (&)[SIZE])
{   
    return SIZE;
}

//�w�肵���t�H���_(dir_name)����w�肵���g���q(extension)�̃t�@�C������񋓂���
std::vector<std::string> get_file_path_in_dir(const std::string& dir_name , const std::string& extension)
{
    HANDLE hFind;
    WIN32_FIND_DATA win32fd;//defined at Windwos.h
    vector<std::string> file_names;

    //�g���q�̐ݒ�
    std::string search_name = dir_name + "\\*." + extension;
	//cout << "search & list [." << extension.c_str() << "] from: " << search_name.c_str() << endl;

    hFind = FindFirstFile( search_name.c_str() , &win32fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        throw runtime_error("file not found");
    }

    do {
        if ( win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
        }
        else {
            file_names.push_back(win32fd.cFileName);
            //printf("%s\n", file_names.back().c_str() );

        }
    } while (FindNextFile(hFind, &win32fd));

    FindClose(hFind);

    return file_names;
}

//�w�肵���t�H���_(dir_name)����T�u�t�H���_����񋓂���(��2�A��3�����Ƀ`�F�b�N����t�@�C�����w��)
std::vector<std::string> get_dir_path_in_dir( const std::string& dir_name, const std::string& checkfile1, const std::string& checkfile2 )
{
    HANDLE hFind;
    WIN32_FIND_DATA win32fd;//defined at Windwos.h
    vector<std::string> subDir_names;

    std::string search_name = dir_name + "\\*";
	//cout << "search & list [FOLDER] from: " << search_name.c_str() << endl;

    hFind = FindFirstFile( search_name.c_str() , &win32fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        throw runtime_error("file not found");
    }
    do {
		if( strcmp( win32fd.cFileName, "." ) && strcmp( win32fd.cFileName, ".." ) ){
			if ( win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

				//case1: �K�v�t�@�C��(checkfile1)�Ə��O�t�@�C��(checkfile2)���Ɏw�肳��Ă���ꍇ
				if( !checkfile1.empty() && !checkfile2.empty() ){
					std::string checkFilePath1 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile1;
					std::string checkFilePath2 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile2;
					ifstream ifs1( checkFilePath1 );
					ifstream ifs2( checkFilePath2 );
					if( ifs1.is_open() && !ifs2.is_open() ){
						subDir_names.push_back(win32fd.cFileName);
						//printf("%s\n", subDir_names.back().c_str() );
					}
				}
				//case2: �K�v�t�@�C��(checkfile1)�݂̂��w�肳��Ă���ꍇ
				else if( !checkfile1.empty() && checkfile2.empty() ){
					std::string checkFilePath1 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile1;
					ifstream ifs1( checkFilePath1 );
					if( ifs1.is_open() ){
						subDir_names.push_back(win32fd.cFileName);
						//printf("%s\n", subDir_names.back().c_str() );
					}
				}
				//case3: ���O�t�@�C��(checkfile2)�݂̂��w�肳��Ă���ꍇ
				else if( checkfile1.empty() && !checkfile2.empty() ){
					std::string checkFilePath2 = dir_name + "\\" + win32fd.cFileName + "\\" + checkfile2;
					ifstream ifs2( checkFilePath2 );
					if( !ifs2.is_open() ){
						subDir_names.push_back(win32fd.cFileName);
						//printf("%s\n", subDir_names.back().c_str() );
					}
				}
				//case4: �����w�肳��Ă��Ȃ��ꍇ
				else if( checkfile1.empty() && checkfile2.empty() ){
					subDir_names.push_back(win32fd.cFileName);
					//printf("%s\n", subDir_names.back().c_str() );
				}
			}
			else {
			}
		}
    } while (FindNextFile(hFind, &win32fd));

    FindClose(hFind);

    return subDir_names;
}

void getHumanBox( cv::Mat &a ){

}

//�󂯎�����摜�̎w�肵�����W����w�肵���l���O���̍��W���Ԃ�(�摜�͈͊O���l��)
std::vector<std::pair<int, int>> getNearestPoints( std::pair<int, int> origPoint, int round, image& input ){

	std::vector<std::pair<int, int>> resultPoints;

	//�����̊֐��ł�邱�Ƃ̐}��

	//origPoint ���� round �̒l���O���̉�f�̍��W���Ԃ�
	//��[�Ɖ��[��x�����S���A����ȊO�͍��[�ƉE�[�݂̂��擾		�����������@����[
	//															���E�E�E��
	//															���E���E��			�� = origPoint, �� = �擾������W, ���̗�ł�round = 2
	//															���E�E�E��
	//															�����������@�����[
	//
	//															���@�@�@��
	//															���@�@�@�E
	//															�[�@�@�@�[

	int yTop = origPoint.second - round;	//��[��y���W�v�Z
	int yBottom = origPoint.second + round; //���[��y���W�v�Z
	int xLeft = origPoint.first - round;	//���[��x���W�v�Z
	int xRight = origPoint.first + round;	//�E�[��x���W�v�Z
	int edgeSize = 1 + 2 * round;

	//��[�Ɖ��[�̍��W�����l �� �܂�origPoint�P�_�̂�( �� round = 0 )
	if( yTop == yBottom ){
		//�摜�͈͊O�H(�ʏ킠�肦�Ȃ�)
		if( xRight > input.Width() - 1 || xLeft < 0 || yTop < 0 || yBottom > input.Height() - 1 ){
			return resultPoints;
		}
		//�摜�͈͓��Ȃ���W���P�ǉ����ĕԂ�
		else{
			resultPoints.push_back( make_pair( xLeft, yTop ) ); //1�_�Ȃ̂ŁAxRight, yBottom�ł�����
			return resultPoints;
		}
	}

	//��Ӊ�f�̍��W��擾
	for( int i = 0; i < edgeSize; i++ ){
		//�摜�͈͓����`�F�b�N
		if( !( xLeft + i > input.Width() - 1 || xLeft + i < 0 || yTop < 0 || yTop > input.Height() - 1 ) ){
			resultPoints.push_back( make_pair( xLeft + i, yTop ) );
		}
	}
	//���ԉ�f�̍��W��擾
	for( int j = 0; j < edgeSize - 2; j++ ){
		//�摜�͈͓����`�F�b�N
		if( !( xLeft < 0 || xLeft > input.Width() - 1 || yTop + 1 + j < 0 || yTop + 1 + j > input.Height() - 1 ) ){
			resultPoints.push_back( make_pair( xLeft, yTop + 1 + j ) );
		}
		if( !( xRight < 0 || xRight > input.Width() - 1 || yTop + 1 + j < 0 || yTop + 1 + j > input.Height() - 1 ) ){
			resultPoints.push_back( make_pair( xRight, yTop + 1 + j ) );
		}
	}
	//���Ӊ�f�̍��W��擾
	for( int i = 0; i < edgeSize; i++ ){
		//�摜�͈͓����`�F�b�N
		if( !( xLeft + i > input.Width() - 1 || xLeft + i < 0 || yBottom < 0 || yBottom > input.Height() - 1 ) ){
			resultPoints.push_back( make_pair( xLeft + i, yBottom ) );
		}
	}
	return resultPoints;
}

//Kinect���l���ǐՂɊ���U�����l��ID��OpenPose�Ō��������l��BOX��ID���}�b�`���O����
int getKinectHumanID( int min_humanX, int max_humanX, int min_humanY, int max_humanY, cv::Mat& bodyindex, image& input ){

	//�l���̈�BOX�̒��S���W�ɑΉ�����BodyIndex��ID���擾(������Ȃ���΂��̋ߕӂ̉�f������)

	int center_humanX, center_humanY;
	int resultID = 255;
	vector<int> neighbor8Points;
	vector<int> idCounts(6, 0);		// ids[0]�`ids[5]�̒l
	
	//���S�v�Z
	center_humanX = ( min_humanX + max_humanX ) / 2;
	center_humanY = ( min_humanY + max_humanY ) / 2;

	resultID = bodyindex.data[ center_humanY * bodyindex.step + center_humanX * bodyindex.elemSize() ];
	if( !( resultID >= 0 && resultID <= 5 ) ){	// �l��BOX�̒��S�ɐl��ID���Ȃ������ꍇ�͂���8�ߖT�_���������đ�����
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 1 ) * bodyindex.step + ( center_humanX - 1 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 1 ) * bodyindex.step + ( center_humanX - 0 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 1 ) * bodyindex.step + ( center_humanX + 1 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 0 ) * bodyindex.step + ( center_humanX - 1 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 0 ) * bodyindex.step + ( center_humanX + 1 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY + 1 ) * bodyindex.step + ( center_humanX - 1 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY + 1 ) * bodyindex.step + ( center_humanX - 0 ) * bodyindex.elemSize() ] );
		neighbor8Points.push_back( bodyindex.data[ ( center_humanY + 1 ) * bodyindex.step + ( center_humanX + 1 ) * bodyindex.elemSize() ] );
		for( int i = 0; i < 8; i++ ){
			if( neighbor8Points[i] >= 0 && neighbor8Points[i] <= 5 ){ idCounts[ neighbor8Points[i] ]++; }
		}
		std::vector<int>::iterator iter = std::max_element( idCounts.begin(), idCounts.end() );
		size_t index = std::distance( idCounts.begin(), iter );
		if( idCounts[index] != 0 ){		// �������ɂ��I�΂ꂽID(���Y��index�̒l)�ɓ����Ă���J�E���g�񐔂�0 -> �܂�8�ߖT�ɂ��l��ID�Ȃ��Ȃ� else
			resultID = index;
		}
		else{	// 8�ߖT���������Ă��l��ID���������Ȃ��ꍇ�A�l��BOX�Ɠ��̈��BodyIndex�؂蔲�������ԑ���ID������
			idCounts = vector<int>(6, 0);
			for( int j = min_humanY; j < max_humanY; j++ ){
				for( int i = min_humanX; i < max_humanX; i++ ){
					int v = bodyindex.data[ j * bodyindex.step + i * bodyindex.elemSize() ];
					if( v >= 0 && v <= 5 ){
						idCounts[ v ]++;
					}
				}
			}
			std::vector<int>::iterator iter = std::max_element( idCounts.begin(), idCounts.end() );
			size_t index = std::distance( idCounts.begin(), iter );
			if( idCounts[index] != 0 ){		// �������ɂ��I�΂ꂽID(���Y��index�̒l)�ɓ����Ă���J�E���g�񐔂�0 -> �܂�8�ߖT�ɂ��l��ID�Ȃ��Ȃ� else
				resultID = index;
			}
			else{
				resultID = 255;
			}
		}
	}

	return resultID;
}

//�e�l����BOX���W���󂯎��ABodyIndex�Ɖ�f�Ή�����KinectID�̏o���񐔂��J�E���g����
std::vector<int> countID_in_HumanBox( int min_humanX, int max_humanX, int min_humanY, int max_humanY, cv::Mat& bodyindex ){

	std::vector<int> idCounts( 6, 0 );

	//�l��BOX�̗̈�Əd�Ȃ�BodyIndex�̗̈�̊e��f��ID�l���擾
	for( int j = min_humanY; j < max_humanY; j++ ){
		for( int i = min_humanX; i < max_humanX; i++ ){

			int v = bodyindex.data[ j * bodyindex.step + i * bodyindex.elemSize() ];
			
			//KinectID(0�`5)�̒l�������Ă��邩�ǂ���
			if( v >= 0 && v <= 5 ){
				//KinectID�Ȃ�AidCounts�̓��l�Ԗڂ̗v�f�l�����Z
				idCounts[v] ++;
			}
		}
	}
	return idCounts;
}
//�e�l��BOX��KinectID�o���񐔃J�E���g�ƁA�e�l��BOX�ʐς��󂯎��AKinectID���Ƃ̐�L�����v�Z����
std::vector<double> calcIDOccupancies( std::vector<int> idCounts, int BoxSize ){

	std::vector<double> idOccupancies;

	for( int i = 0; i < idCounts.size(); i++ ){ //�K��6�񃋁[�v�̂͂�
		if( BoxSize != 0 ){
			idOccupancies.push_back( (double)idCounts[i] / (double)BoxSize );
		}else{
			idOccupancies.push_back( 0. );
		}
	}
	return idOccupancies;
}

//11�t���[���ڕ��̐ڐG���ɉf�����S�l��BOX��BodyIndex(KinectID)��Ή��Â���
void mapHumanBoxToBodyIndex( image& input, cv::Mat& bodyindex, int flag11F ){

	std::vector<std::vector<int>> peopleIdCounts;
	std::vector<int> peopleBoxSizes;
	std::vector<std::vector<double>> peopleIdOccupancies;

	//KinectID[ 0[ [ humanID, Occupancy ], [ humanID, Occupancy ], ... ], 1[ , ], 2[ , ], ... , 5[ , ] ]
	std::vector<std::vector<pair<int, double>>> IdOccupancyRank(6), IdOccRankCopy;

	std::vector<image::HumanBox>& humanBoxes = std::vector<image::HumanBox>();
	if( flag11F == 1 ){
		humanBoxes = input.humanBoxes;
	}
	else{
		humanBoxes = input.frameHumanBoxes;
	}
	//���ʁF�l���l������KinectID���蓖�Ĕz��(�������AID�����蓖�Ă��Ȃ������l��������)
	std::vector<int> HumanBoxKinectIDs( humanBoxes.size(), -1 );

	if( humanBoxes.size() == 0 ){
		if( flag11F == 1 ){
			input.HumanBoxKinectIDs.clear();
			input.HumanBoxKinectIDs = HumanBoxKinectIDs;
		}else if( flag11F == 0 ){
			input.frameHumanBoxKinectIDs.clear();
			input.frameHumanBoxKinectIDs = HumanBoxKinectIDs;
		}
		return;
	}

	//�S�l��BOX�ɑ΂���
	for( int p = 0; p < humanBoxes.size(); p++ ){

		//BOX��BodyIndexID�l�̐�L�ʂ��J�E���g
		peopleIdCounts.push_back( countID_in_HumanBox( humanBoxes[p].minX, humanBoxes[p].maxX, humanBoxes[p].minY, humanBoxes[p].maxY, bodyindex ) );
		//BOX�̖ʐς��v�Z
		peopleBoxSizes.push_back( ( humanBoxes[p].maxX - humanBoxes[p].minX ) * ( humanBoxes[p].maxY - humanBoxes[p].minY ) );
		//KinectID�l(0�`5)���Ƃ̐�L�����v�Z
		peopleIdOccupancies.push_back( calcIDOccupancies( peopleIdCounts[p], peopleBoxSizes[p] ) );
	}

	//���ō쐬�����e�l���̑Ή�KinectID�ʂ̃f�[�^����A�ŏI�I��KinectID�����蓖��
	for( int p = 0; p < humanBoxes.size(); p++ ){
		for( int i = 0; i < 6; i++ ){
			//�l�����Ƃ�0�`6��KinectID�̊e��L���𑖍����āAKinectID�x�[�X�̔z��ɐl���ԍ��Ɛ�L����o�^
			IdOccupancyRank[i].push_back( make_pair( p, peopleIdOccupancies[p][i] ) );
		}
	}
	//���ō쐬�����z����eKinectID���Ƃɐ�L���ō~���\�[�g
	for( int i = 0; i< IdOccupancyRank.size(); i++ ){
		if( IdOccupancyRank[i].size() > 0 ){
			std::sort( IdOccupancyRank[i].begin(), IdOccupancyRank[i].end(), [](const pair<int, double>& x, const pair<int, double>& y) { return x.second > y.second; });
		}
	}
	//�O�̂��߃R�s�[����vector�I�u�W�F�N�g���g��
	IdOccRankCopy = IdOccupancyRank;

	//��ɊeKinectID��1���(��L���ő�)�̃f�[�^�����āA�l���ԍ��ɑΉ�����KinectID���m�肳���Ă���
	bool emptyRank = false;
	std::vector<bool> AssignedIDs( 6, false );
	std::vector<bool> AssignedHuman( humanBoxes.size(), false );

	while( !emptyRank ){

		pair<int, double> maxOccupancy = make_pair( -1, -1. );
		int maxOccID = -1;

		for( int id = 0; id < 6; id++ ){
			if( IdOccRankCopy[id].size() == 0 ) continue;
			if( maxOccupancy.first == -1 && maxOccupancy.second == -1. && maxOccID == -1 ){
				maxOccupancy.first = IdOccRankCopy[id][0].first;
				maxOccupancy.second = IdOccRankCopy[id][0].second;
				maxOccID = id;
			}
			if( IdOccRankCopy[id][0].second > maxOccupancy.second ){
				maxOccupancy.first = IdOccRankCopy[id][0].first;
				maxOccupancy.second = IdOccRankCopy[id][0].second;
				maxOccID = id;
			}
		}
		//����KinectID�������蓖�Ă������ꍇ�A��L�����ő傾�����l��BOX��KinectID�̃y�A��o�^
		if( AssignedIDs[maxOccID] == false && AssignedHuman[maxOccupancy.first] == false && maxOccupancy.second != 0. ){
			HumanBoxKinectIDs[maxOccupancy.first] = maxOccID;
			AssignedIDs[maxOccID] = true;
			AssignedHuman[maxOccupancy.first] = true;
			IdOccRankCopy[maxOccID].erase( IdOccRankCopy[maxOccID].begin() + 0 );
		}
		//����KinectID�����蓖�čς݂������ꍇ�A���������ɂ���[�l��-��L��]�f�[�^��j��
		else{
			IdOccRankCopy[maxOccID].erase( IdOccRankCopy[maxOccID].begin() + 0 );
		}

		if( IdOccRankCopy[0].size() == 0 && IdOccRankCopy[1].size() == 0 && IdOccRankCopy[2].size() == 0 &&
			IdOccRankCopy[3].size() == 0 && IdOccRankCopy[4].size() == 0 && IdOccRankCopy[5].size() == 0 ){
			emptyRank = true;
		}
	}
	if( flag11F == 1 ){
		input.HumanBoxKinectIDs.clear();
		input.HumanBoxKinectIDs = HumanBoxKinectIDs;
	}else if( flag11F == 0 ){
		input.frameHumanBoxKinectIDs.clear();
		input.frameHumanBoxKinectIDs = HumanBoxKinectIDs;
	}
}

//���͉摜��1920x1080�̏ꍇ��p�AKinect��Depth(512x424)���c����3�{�Ɋg�債�Ă���(1536x1272)���̂Ƃ��Čv�Z
std::pair<double, double> mmTransform(int pointX, int pointY, int depth)
{
	//KinectDepth�摜���ʒ��S�ƃJ�����ʒu�̋���(����͒萔�A��365[pixel] )
	//X��(��512pixel�A����p35��)�Ōv�Z���Ă��邪
	//Y��(��212pixel�A����p30��)�Ōv�Z���Ă��قړ����l�ɂȂ�
	double camDistPixel = (512 / 2.) / tan(35. * M_PI / 180.);

	//�^����ꂽ�_�̍��W�l(����)���摜���S��ɕϊ�
	double WfromCenter_HR = pointX - (1920 / 2.);
	double HfromCenter_HR = pointY - (1080 / 2.);

	//�^����ꂽ�_�̍��W�l(1920x1080���ʂł̒l)���g��O(KinectDepth�摜512x424���ʂł̒l)�ɖ߂�(��2.93�`3.04�{)
	double WfromCenter = WfromCenter_HR / 3.;
	double HfromCenter = HfromCenter_HR / 3.;

	//KinectDepth�摜���ʏ�ł̓_�̃J�����ʒu�Ƃ̋���(pixel)���v�Z(�O�����̒藝)
	double pointDistPixel = sqrt(pow(camDistPixel, 2.) + pow(WfromCenter, 2.) + pow(HfromCenter, 2.));

	//�����l������pixel�����̔䗦�v�Z
	double rate_pix2mm = depth / pointDistPixel;

	//�摜���S�����X�EY�����̎�����(mm)���v�Z
	double WfromCenter_mm = WfromCenter * rate_pix2mm;
	double HfromCenter_mm = HfromCenter * rate_pix2mm;
	//depth�����Ă��Ȃ��ꍇ�A�������Ȓl�ɂȂ�̂ŁA-10000�ɕ␳
	if(depth == -10000)
	{
		WfromCenter_mm = -10000.;
		HfromCenter_mm = -10000.;
	}
	std::pair<double, double> cood = std::make_pair(WfromCenter_mm, HfromCenter_mm);

	return cood;
}

//���͉摜��1920x1080�̏ꍇ��p�AKinect��Depth(512x424)���c����3�{�Ɋg�債�Ă���(1536x1272)���̂Ƃ��Čv�Z
std::pair<double, double> pxScaleUniform(double PWfromC, double PHfromC, double DReal, double DBaseC)
{
	//KinectColor�摜���ʒ��S�ƃJ�����ʒu�̉��z����(����͒萔�A��538[pixel] )
	//X��(��1536pixel�A����p70��)�Ōv�Z���Ă��邪
	//Y��(��1272pixel�A����p60��)�Ōv�Z���Ă��قړ����l�ɂȂ�
	double tangent = tan(35. * M_PI / 180.);
	double halfwidth = (512 / 2.) * 3;
	double camDistPixel = halfwidth / tangent;

	//�Ώۓ_����J�������_�ւ̓��ˊp���v�Z
	double Wtheta = atan(PWfromC / camDistPixel);
	double Htheta = atan(PHfromC / camDistPixel);

	//depth�x�[�X����(���S�̐[�x��3000mm, �[�ɍs���قǉ����Ȃ�)��z�肵�A�Ώۓ_�̃x�[�X���ʏ㋗�����v�Z
	double DBasePW = DBaseC / cos(Wtheta);
	double DBasePH = DBaseC / cos(Htheta);

	//�����l�����ƃx�[�X���ʋ����̔䗦�v�Z
	double ratePW = DReal * cos(Htheta) / DBasePW;
	double ratePH = DReal * cos(Wtheta) / DBasePH;

	//�䗦�������ăX�P�[���ψꉻ
	double suPW = PWfromC * ratePW;
	double suPH = PHfromC * ratePH;
	//depth�����Ă��Ȃ��ꍇ�A�������Ȓl�ɂȂ�̂ŁA-10000�ɕ␳
	if(DReal == -10000 || DReal == 0)
	{
		suPW = -10000.;
		suPH = -10000.;
	}
	std::pair<double, double> cood = std::make_pair(suPW, suPH);

	return cood;
}


double get_median_px(std::vector<point3D>& hand3Ds)
{
	//depth�l��0��������-10000�ȊO�̎�w�L�[�|�C���g��depth�l���ɕ��בւ��Ē����l��Ԃ�
	//depth�L���l���ЂƂ��Ȃ������ꍇ��0��Ԃ�
	int size = hand3Ds.size();
    std::vector<double> _handDepths;
	for(int p = 0; p < size; p++){
		if(hand3Ds[p].z != -10000 && hand3Ds[p].z != 0){ //Depth�l���Ƃ�Ă��Ȃ��Ƃ���͏��O
			_handDepths.push_back(hand3Ds[p].z);
		}
	}
    //copy(hand3Ds.begin(), hand3Ds.end(), back_inserter(_hand3Ds));
	size = _handDepths.size(); //���߂ăT�C�Y�v�Z
	if(size == 0){
		return 0;
	}
    double tmp;
    for (int i = 0; i < size - 1; i++){
        for (int j = i + 1; j < size; j++) {
            if (_handDepths[i] > _handDepths[j]){
                tmp = _handDepths[i];
                _handDepths[i] = _handDepths[j];
                _handDepths[j] = tmp;
            }
        }
    }
    if (size % 2 == 1) {
        return _handDepths[(size - 1) / 2];
    } else {
        return (_handDepths[(size / 2) - 1] + _handDepths[size / 2]) / 2;
    }
}

double search_nearest_depth_px(Mat depth, int x_c, int y_c, double limin_c, double limax_c, double orig_span)
{
	//depth�����͈͏����𖞂����A�ߖT��f���̕��ϒl��Ԃ�
	//��₪2�_�ȏ㌩���������_�ŒT���I��
	//��̔��T�C�Y��茟���{�b�N�X���傫���Ȃ�ΒT���I��
	//��₪�܂�����������Ȃ������ꍇ-10000��Ԃ�
	std::vector<double> candidate;
	int boxsize = 1;
	while(1){
		int min_y = max(y_c-boxsize, 0);
		int min_x = max(x_c-boxsize, 0);
		int max_y = min(y_c+boxsize, depth.rows-1);
		int max_x = min(x_c+boxsize, depth.cols-1);
		for(int iy = min_y; iy <= max_y; iy++){
			for(int ix = min_x; ix <= max_x; ix++){
				if(depth.at<ushort>(iy, ix) > limin_c && depth.at<ushort>(iy, ix) < limax_c){
					candidate.push_back(depth.at<ushort>(iy, ix));
				}
			}
		}
		if(candidate.size() > 1){ break; }
		boxsize += 1;
		if(boxsize > orig_span / 2){ break; }
	}
	double sum = 0.;
	if(candidate.size() == 0){
		return -10000;
	}
	for(int ic = 0; ic < candidate.size(); ic++){
		sum += candidate[ic];
	}
	return sum / candidate.size();
}

void limit_hand_depth_px(std::vector<point3D>& hand3Ds, double med, double span, double orig_span, double Center, Mat depth)
{
	//depth�����͈�(-(span/2)�`med�`+(span/2))�����𖞂����ߖT��f�̒l���ς��擾
	//med��0�̏ꍇ��Center�𐧌��͈͂̒��S�Ƃ���
	//XY��������Ȃ���Ή������Ȃ��B(depth��0�������Ă���͂�)
	//�ߖT������Ȃ����(-10000���Ԃ��)med�Ŗ��߂�
	if(med == 0){med = Center;}
	double min = max(500, med - span / 2.);	//�J�������W�ł�
	double max = min(8000, med + span / 2.);	//Depth�����͈�

	cout << "hand depth span(camera base): " << min << " �` " << max << endl;
					
	for(int i = 0; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //�֐�i��xy���Ă�Ȃ�
			if(hand3Ds[i].z < min || hand3Ds[i].z > max){ //�����͈͊O�̒l�����Ă���֐ߓ_�@��hand3Ds�͕��̈ʒu����W
				hand3Ds[i].z = (int)search_nearest_depth_px(depth, hand3Ds[i].x, hand3Ds[i].y, min, max, orig_span); //�����̓J�����ʒu������͈�
				if(hand3Ds[i].z == -10000){ hand3Ds[i].z = med; }
			}
		}
	}
}

double get_median(std::vector<Point3D>& hand3Ds)
{
	int size = hand3Ds.size();
    std::vector<double> _handDepths;
	for(int p = 0; p < size; p++){
		if(hand3Ds[p].z != -10000 && hand3Ds[p].z != 0){ //Depth�l���Ƃ�Ă��Ȃ��Ƃ���͏��O
			_handDepths.push_back(hand3Ds[p].z);
		}
	}
    //copy(hand3Ds.begin(), hand3Ds.end(), back_inserter(_hand3Ds));
	size = _handDepths.size(); //���߂ăT�C�Y�v�Z
	if(size == 0){
		return 0;
	}
    double tmp;
    for (int i = 0; i < size - 1; i++){
        for (int j = i + 1; j < size; j++) {
            if (_handDepths[i] > _handDepths[j]){
                tmp = _handDepths[i];
                _handDepths[i] = _handDepths[j];
                _handDepths[j] = tmp;
            }
        }
    }
    if (size % 2 == 1) {
        return _handDepths[(size - 1) / 2];
    } else {
        return (_handDepths[(size / 2) - 1] + _handDepths[size / 2]) / 2;
    }
}

double search_nearest_depth(Mat depth, int x_c, int y_c, double limin_c, double limax_c, double orig_span)
{
	std::vector<double> candidate;
	int boxsize = 1;
	while(1){
		int min_y = max(y_c-boxsize, 0);
		int min_x = max(x_c-boxsize, 0);
		int max_y = min(y_c+boxsize, depth.rows-1);
		int max_x = min(x_c+boxsize, depth.cols-1);
		for(int iy = min_y; iy <= max_y; iy++){
			for(int ix = min_x; ix <= max_x; ix++){
				if(depth.at<ushort>(iy, ix) > limin_c && depth.at<ushort>(iy, ix) < limax_c){
					candidate.push_back(depth.at<ushort>(iy, ix));
				}
			}
		}
		if(candidate.size() > 1){ break; }
		boxsize += 1;
		if(boxsize > orig_span / 2){ break; }
	}
	double sum = 0.;
	if(candidate.size() == 0){
		return -10000;
	}
	for(int ic = 0; ic < candidate.size(); ic++){
		sum += candidate[ic];
	}
	return sum / candidate.size();
}

void limit_hand_depth(std::vector<Point3D>& hand3Ds, double med, double span, double orig_span, double ObjCenter, Mat depth)
{
	double min_o = max(500 - ObjCenter, med - span / 2.);	//���̈ʒu����W�ł�
	double max_o = min(8000 - ObjCenter, med + span / 2.);	//Depth�����͈�

	double min_c = min_o + ObjCenter;						//�J�����ʒu����W�ł�
	double max_c = max_o + ObjCenter;						//Depth�����͈�(Depth�摜�̒l)

	cout << "hand depth span(object base): " << min_o << " �` " << max_o << endl;
	cout << "hand depth span(camera base): " << min_c << " �` " << max_c << endl;
					
	for(int i = 0; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //�֐�i��xy���Ă�Ȃ�
			if(hand3Ds[i].z < min_o || hand3Ds[i].z > max_o){ //�����͈͊O�̒l�����Ă���֐ߓ_�@��hand3Ds�͕��̈ʒu����W
				hand3Ds[i].z = search_nearest_depth(depth, hand3Ds[i].orig_x, hand3Ds[i].orig_y, min_c, max_c, orig_span); //�����̓J�����ʒu������͈�
				if(hand3Ds[i].z == -10000){ hand3Ds[i].z = med; }
				else{ hand3Ds[i].z -= ObjCenter; } 
			}
		}
	}
}

bool checkFileExistence(const std::string& str)
{
    std::ifstream ifs(str);
    return ifs.is_open();
}

int get_actDepth(Mat inputDepth, int y, int x, int boxsize )
{
	int min_y = max(y-boxsize, 0);
	int min_x = max(x-boxsize, 0);
	int max_y = min(y+boxsize, inputDepth.rows-1);
	int max_x = min(x+boxsize, inputDepth.cols-1);
	int min_depth = 10000;
	for(int iy = min_y; iy <= max_y; iy++){
		for(int ix = min_x; ix <= max_x; ix++){
			int idepth = (int)inputDepth.at<ushort>( iy, ix );
			if(idepth <= 500){continue;}
			if(idepth >= 8000){continue;}
			min_depth = min(min_depth, idepth);
		}
	}
	if(min_depth == 10000){
		min_depth = -10000;
	}
	return min_depth;
}

double calcDist(Point3D p1, Point3D p2)
{
	if(p1.z == -10000 || p2.z == -10000){
		return 0;
	}
	double dist = sqrt(pow(p2.x - p1.x, 2.) + pow(p2.y - p1.y, 2.) + pow(p2.z - p1.z, 2.));
	return dist;
}

void fix_hand_dist
	(
	bool *isEs,
	double *ELs,
	int *idxt,
	int *idxc,
	std::vector<Point3D>& hand3Ds,
	Vector3D dir,
	Vector3D *EVs,
	double med
	)
{
	bool is_changed = TRUE;
	int num_cycle = 0;
	//0-1�G�b�W����Ƃ����e�G�b�W�̔䗦
	double rate[20] = {
		1.000,1.186,1.059,0.830, //�e�w
		2.474,1.011,0.693,0.693, //�l�����w
		2.390,1.109,0.792,0.813, //���w
		2.324,1.019,0.694,0.918, //��w
		2.353,0.792,0.572,0.678  //���w
	};
	int p1[20] = {0,1,2,3,0,5,6,7,0,9, 10,11,0, 13,14,15,0, 17,18,19}; //20�{�̃G�b�W�̎n�_
	int p2[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; //20�{�̃G�b�W�̏I�_
	double tempZs[5];

	//�G�b�W�O���[�v0���ǂ���
	int eg;
	if(idxt[0] == 0){eg = 0;}else if(idxt[0] == 1){eg =1;}else if(idxt[0] == 2){eg = 2;}else{eg = 3;}

	//�ŏ��Ƀ^�[�Q�b�g��Z���W�����ׂĊo���Ă���
	for(int i = 0; i < 5; i++){
		tempZs[i] = hand3Ds[p2[idxt[i]]].z;
	}

	int f=0;
	while(is_changed || num_cycle == 0){
		is_changed = FALSE;
		for(int ft = 0; ft < 5; ft++){								//�␳�Ώێw�G�b�W5�{�����[�v
			if(isEs[idxt[ft]] == TRUE){								//��2D���Ȃ�
				for(int fc = 0; fc < 5; fc++){						//��r�Ώێw�G�b�W5�{�����[�v
					if(idxt[ft] == idxc[fc]){continue;}				//�␳�ΏۂƔ�r�Ώۂ������G�b�W�̏ꍇ�X�L�b�v
					if(ELs[idxc[fc]] != -10000 && ELs[idxc[fc]] != 0){ //��r�ΏۃG�b�W�̒������v�Z�\�Ȃ�
						//���G�b�W�Ƃ̔䗦(���O����)����O��Ă��邩
						if(
							(ELs[idxt[ft]] / ELs[idxc[fc]] > rate[idxt[ft]] / rate[idxc[fc]] + 0.1)      //���z�䗦����0.1����
							//|| (ELs[idxt[ft]] / ELs[idxc[fc]] < rate[idxt[ft]] / rate[idxc[fc]] - 0.1) //���z�䗦����0.1�Z��
							)
						{ 
							//�O��Ă����ꍇ�ADepth�𒲐����ċ�����␳
							//cout << "�^�[�Q�b�g�̒����F" << Ls[idx[f]] << endl;
							//cout << "��r�Ώۂ̒����F" << Ls[idxc[id]] << endl;
							//cout << "L[" << idx[f] << "] �� L[" << idxc[id] << "] = " << Ls[idx[f]] / Ls[idxc[id]] << " (���z�䗦�F" << rate[idx[f]] / rate[idxc[id]] << ")" << endl;
							
							//���z�̋������Z�o(2��)
							double expectDist = pow(ELs[idxc[fc]] * rate[idxt[ft]] / rate[idxc[fc]], 2.);
							//cout << "���z����(2��)�F" << expectDist << endl;
							
							//�񎟌����ʋ���(2��)���v�Z
							double dist2D = pow(hand3Ds[p2[idxt[ft]]].x - hand3Ds[p1[idxt[ft]]].x, 2.) + pow(hand3Ds[p2[idxt[ft]]].y - hand3Ds[p1[idxt[ft]]].y, 2.);
							//cout << "XY���ʋ���(2��)�F" << dist2D << endl;
							
							//�␳�ΏۃG�b�W�̕����x�N�g��
							Vector3D vect = EVs[ft];
							//��r�ΏۃG�b�W�̕����x�N�g��
							Vector3D vecc = EVs[fc];
							//��w�S�̂̕��ϕ����x�N�g��
							Vector3D orgDir = dir;
							
							//if(hand3Ds[p2nds[idx[f]]].z - hand3Ds[p1sts[idx[f]]].z >= 0){orgDirection = 1.;}
							//else{orgDirection = -1.;}
							
							//���z����(2��)�Ɠ񎟌����ʋ���(2��)����Depth�������Z�o
							if(dist2D == ELs[idxt[ft]]){
								//cout << "depth�v�Z�s��(�␳�ς�)" << endl;
								continue;
							}
							//���z����(2��)���񎟌����ʋ���(2��)�̕����傫���Ȃ�j�]���Ă���̂�,
							//�אڃG�b�W�̃x�N�g��������Z������(�Ȃ��ꍇ��median��ݒ�)
							if(dist2D > expectDist){
								double rateK; //���s�x�N�g���̌W��
								//cout << "depth�v�Z�s��(XY�ُ�)" << endl;
								if(eg == 0 || EVs[idxt[ft]-1].dx == 0 || EVs[idxt[ft]-1].dy == 0){
									hand3Ds[p2[idxt[ft]]].z = med;
								}
								else{
									if(abs(EVs[idxt[ft]].dx / EVs[idxt[ft]-1].dx) > abs(EVs[idxt[ft]].dy / EVs[idxt[ft]-1].dy)){
										rateK = EVs[idxt[ft]].dx / EVs[idxt[ft]-1].dx;
									}
									else{ rateK = EVs[idxt[ft]].dy / EVs[idxt[ft]-1].dy; }
									hand3Ds[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + EVs[idxt[ft]-1].dz * rateK;
									cout << hand3Ds[p2[idxt[ft]]].z << "(fixed)" << endl;
									//rateK���傫������ꍇ�͒�������
									if(hand3Ds[p2[idxt[ft]]].z < med - 100 || hand3Ds[p2[idxt[ft]]].z > med + 100){
										//�G�b�W�̒������Čv�Z
										cout << "recalculation." << endl;
										double EL = calcDist(hand3Ds[p2[idxt[ft]]], hand3Ds[p1[idxt[ft]]]);
										cout << EL << "(EL)" << endl;
										double reduction = (100 - abs(hand3Ds[p1[idxt[ft]]].z - med)) / EL;
										cout << reduction << "(reduction)" << endl;
										hand3Ds[p2[idxt[ft]]].x = hand3Ds[p1[idxt[ft]]].x + EVs[idxt[ft]-1].dx * reduction;
										hand3Ds[p2[idxt[ft]]].y = hand3Ds[p1[idxt[ft]]].y + EVs[idxt[ft]-1].dy * reduction;
										hand3Ds[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + EVs[idxt[ft]-1].dz * reduction;
									}
								}
								//�␳���ʂ����ɒ����ƃx�N�g�����Čv�Z���Ĕ��f
								ELs[idxt[ft]] = calcDist(hand3Ds[p2[idxt[ft]]], hand3Ds[p1[idxt[ft]]]);
								EVs[idxt[ft]].dx = hand3Ds[p2[idxt[ft]]].x - hand3Ds[p1[idxt[ft]]].x;
								EVs[idxt[ft]].dy = hand3Ds[p2[idxt[ft]]].y - hand3Ds[p1[idxt[ft]]].y;
								EVs[idxt[ft]].dz = hand3Ds[p2[idxt[ft]]].z - hand3Ds[p1[idxt[ft]]].z;
								continue;
							}
							//���z�����Ɠ񎟌����ʋ������痝�z���s�����Z��
							double distDepth = sqrt(expectDist - dist2D);
							
							//���������̂��߉�������2�p�^�[��							
							/*//2�p�^�[�����ꂼ��̕ύX��w�S�̒������v����œK����(����)�𐄒�
							int fingernum = idxt[ft] / 4; //�w�ԍ����v�Z(�e�w0�`���w4)
							//�w1�{�������̃X�P���g���f�[�^�R�s�[�쐬
							Point3D finger3Ds_p[5], finger3Ds_m[5];
							finger3Ds_p[0] = hand3Ds[0]; finger3Ds_m[0] = hand3Ds[0];
							for(int j = 1; j <= 4; j++ ){
								if(hand3Ds[4 * fingernum + j].x == -10000 || hand3Ds[4 * fingernum + j].y == -10000 ||
									hand3Ds[4 * fingernum + j].x == 0 || hand3Ds[4 * fingernum + j].y == 0)
								{
									if(j != 4){
										finger3Ds_p[j].x = (hand3Ds[4 * fingernum + j+1].x + hand3Ds[4 * fingernum + j-1].x) / 2.;
										finger3Ds_p[j].y = (hand3Ds[4 * fingernum + j+1].y + hand3Ds[4 * fingernum + j-1].y) / 2.;
										finger3Ds_p[j].z = med;
										finger3Ds_m[j].x = (hand3Ds[4 * fingernum + j+1].x + hand3Ds[4 * fingernum + j-1].x) / 2.;
										finger3Ds_m[j].y = (hand3Ds[4 * fingernum + j+1].y + hand3Ds[4 * fingernum + j-1].y) / 2.;
										finger3Ds_m[j].z = med;
									}else{
										finger3Ds_p[j].x = hand3Ds[4 * fingernum + j-1].x + (hand3Ds[4 * fingernum + j-1].x - hand3Ds[4 * fingernum + j-2].x)
									}
								}
								finger3Ds_p[j] = hand3Ds[4 * fingernum + j];
								finger3Ds_m[j] = hand3Ds[4 * fingernum + j];
							}
							//�v�Z�������z���s���𐳕�2�p�^�[���ő�������
							finger3Ds_p[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + distDepth;
							finger3Ds_m[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z - distDepth;
							double sumLen_p, sumLen_m;*/
							//�G�b�W�O���[�v�̕��ϕ����x�N�g�����Q��
							int ecnt = 0;
							double avedz = 0.;
							double sign;
							for(int e = 0; e < 5; e++){
								if(EVs[idxt[e]].dz != 0.){
									ecnt ++;
									avedz += EVs[idxt[e]].dz;
								}
							}
							if(ecnt > 0){avedz /= ecnt;}
							if(avedz > 0){sign = 1.;}else if(avedz < 0){sign = -1.;}
							else{
								if(dir.dz > 0){sign = 1.;}else if(dir.dz < 0){sign = -1.;}
								else{sign = 0;}
							}
							//cout << "���z���s�������F" << distDepth << endl;
							//�v�Z����Depth����������z��␳
							cout << "�␳�Odepth�F" << hand3Ds[p2[idxt[ft]]].z << endl;
							if(sign == 0){hand3Ds[p2[idxt[ft]]].z = med;}
							else{hand3Ds[p2[idxt[ft]]].z = hand3Ds[p1[idxt[ft]]].z + distDepth * sign;}
							cout << "�␳��depth�F" << hand3Ds[p2[idxt[ft]]].z << endl;
							//�␳���ʂ����ɒ����ƃx�N�g�����Čv�Z���Ĕ��f
							ELs[idxt[ft]] = calcDist(hand3Ds[p2[idxt[ft]]], hand3Ds[p1[idxt[ft]]]);
							EVs[idxt[ft]].dx = hand3Ds[p2[idxt[ft]]].x - hand3Ds[p1[idxt[ft]]].x;
							EVs[idxt[ft]].dy = hand3Ds[p2[idxt[ft]]].y - hand3Ds[p1[idxt[ft]]].y;
							EVs[idxt[ft]].dz = hand3Ds[p2[idxt[ft]]].z - hand3Ds[p1[idxt[ft]]].z;
							//cout << "�Čv�Z��̃^�[�Q�b�g�̒����F" << ELs[idxt[ft]] << endl;
						}
					}
				}
			}
		}
		num_cycle++;
		for(int i = 0; i < 5; i++){
			if(tempZs[i] != hand3Ds[p2[idxt[i]]].z){
				//�O��ƕς���Ă镔��������Όp��
				is_changed = TRUE;
			}
			tempZs[i] = hand3Ds[p2[idxt[i]]].z;
		}
		if(num_cycle >= 5){is_changed = FALSE;}
		//cout << is_changed << endl;
		cout << num_cycle << "(cycle)" << endl;
	}
}

void fix_hand_depth(std::vector<Point3D>& hand3Ds, double hmed_d)
{
	//�e�֐ߓ_�������Ă��邩
	bool isPs[21];						
	//�e�G�b�W(XY)���݂��邩
	bool isEs[20];	
	//�e�G�b�W����(XYZ)
	double ELs[20];
	//�e�G�b�W�x�N�g��(XYZ)
	Vector3D EVs[20];
	int p1sts[20] = {0,1,2,3,0,5,6,7,0,9, 10,11,0, 13,14,15,0, 17,18,19}; //20�{�̃G�b�W�̎n�_
	int p2nds[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; //20�{�̃G�b�W�̏I�_
	//�G�b�Windex
	//[  0:0-1 ,  1:1-2  ,  2:2-3  ,  3:3-4  , //�e�w
	//   4:0-5 ,  5:5-6  ,  6:6-7  ,  7:7-8  , //�l�����w
	//   8:0-9 ,  9:9-10 , 10:10-11, 11:11-12, //���w
	//  12:0-13, 13:13-14, 14:14-15, 15:15-16, //��w
	//  16:0-17, 17:17-18, 18:18-19, 19:19-20 ]//���w

	//std::vector<pair<int, int>> Lgroup1, Lgroup2, Lgroup3, Lgroup4, Lgroup5, Lgroup6, Lgroup7;
	//Lgroup1.push_back(make_pair(0,1)); Lgroup1.push_back(make_pair(2,3)); Lgroup1.push_back(make_pair(5,6)); Lgroup1.push_back(make_pair(13,14));
	//Lgroup2.push_back(make_pair(1,2)); Lgroup2.push_back(make_pair(9,10));
	//Lgroup3.push_back(make_pair(3,4)); Lgroup3.push_back(make_pair(10,11)); Lgroup3.push_back(make_pair(11,12)); Lgroup3.push_back(make_pair(17,18));
	//Lgroup4.push_back(make_pair(0,5)); Lgroup4.push_back(make_pair(0,9)); Lgroup4.push_back(make_pair(0,13)); Lgroup4.push_back(make_pair(0,17));
	//Lgroup5.push_back(make_pair(6,7)); Lgroup5.push_back(make_pair(7,8)); Lgroup5.push_back(make_pair(14,15)); Lgroup5.push_back(make_pair(19,20));
	//Lgroup6.push_back(make_pair(15,16));
	//Lgroup7.push_back(make_pair(18,19));

	//�܂�P0��-10000�łȂ����`�F�b�N
	if(hand3Ds[0].x == -10000 && hand3Ds[0].y == -10000){return;} //���_�Ȃ��Ȃ����
	//���ׂĂ̓_�ɂ��đ��݂��邩�ǂ����`�F�b�N
	for(int i = 0; i < hand3Ds.size(); i++){
		if(hand3Ds[i].x != -10000 && hand3Ds[i].y != -10000){ //�֐�i��xy���Ă�Ȃ�
			isPs[i] = TRUE;//�֐�i XY��
		}
		else{isPs[i] = FALSE;}//�֐�i XY�~
	}
	//���ׂẴG�b�W�ɂ��đ��݂��邩�ǂ����`�F�b�N&�����v�Z&�x�N�g���v�Z(depth���Ă��Ȃ��ꍇ�͋���0)
	for(int e = 0; e < arrlen(EVs); e++){
		if(isPs[p1sts[e]] == TRUE && isPs[p2nds[e]] == TRUE){
			isEs[e] = TRUE;
			ELs[e] = calcDist(hand3Ds[p1sts[e]], hand3Ds[p2nds[e]]);
			EVs[e].dx = hand3Ds[p2nds[e]].x - hand3Ds[p1sts[e]].x;
			EVs[e].dy = hand3Ds[p2nds[e]].y - hand3Ds[p1sts[e]].y;
			EVs[e].dz = hand3Ds[p2nds[e]].z - hand3Ds[p1sts[e]].z;
		}
		else{
			isEs[e] = FALSE;
			ELs[e] = 0.;
			EVs[e].dx = 0.; EVs[e].dy = 0.; EVs[e].dz = 0.;
		}
	}
	//��w�S�̂̌����x�N�g�����e�G�b�W�x�N�g���̑������ςŐ���
	Vector3D direction = {0., 0., 0.};
	int ecnt = 0;
	for(int di = 0; di < arrlen(EVs); di++){
		if(isEs[di] == TRUE){
			direction.dx += EVs[di].dx;
			direction.dy += EVs[di].dy;
			direction.dz += EVs[di].dz;
			ecnt ++;
		}
	}
	if(ecnt > 0){
		direction.dx /= ecnt;
		direction.dy /= ecnt;
		direction.dz /= ecnt;
	}
	cout << "��w�S�̕��σx�N�g��\nx�F" << direction.dx << "\ny�F" << direction.dy << "\nz�F" << direction.dz << endl;

	//��������e�_����уG�b�W��␳
	//�܂��͌��_����̊5�{
	int idx[5] = {0,4,8,12,16};
	int idx2[5] = {1,5,9,13,17};
	int idx3[5] = {2,6,10,14,18};
	int idx4[5] = {3,7,11,15,19};

	//fix_hand_dist(isLs, Ls, idx, idx, hand3Ds, direction, EVs, hmed_d);
	//fix_hand_dist(isLs, Ls, idx2, idx, hand3Ds, direction, EVs, hmed_d);
	fix_hand_dist(isEs, ELs, idx3, idx, hand3Ds, direction, EVs, hmed_d);
	fix_hand_dist(isEs, ELs, idx4, idx, hand3Ds, direction, EVs, hmed_d);
}

//OpenPose�Ō��������l��BOX�ɑ΂��A�����Kinect�̐l���ǐ՗pID���ǂ̒��x����U���Ă��邩�A���̉�f�����J�E���g���ĕԂ�
int countKinectID_in_HumanBox( int t_kinectID, int min_humanX, int max_humanX, int min_humanY, int max_humanY, cv::Mat& bodyindex ){
	
	//�l���̈�BOX�̒��S���W�ɑΉ�����BodyIndex��ID���擾(������Ȃ���΂��̋ߕӂ̉�f������)
	//int center_humanX, center_humanY;
	//int resultID = 255;
	//vector<int> neighbor8Points;
	//vector<int> idCounts(6, 0);		// ids[0]�`ids[5]�̒l
	//int min_ObjX = input.obj_minX, max_ObjX = input.obj_maxX, min_ObjY = input.obj_minY, max_ObjY = input.obj_maxY;
	//int humanBoxSize = ( max_humanX - min_humanX + 1 ) * ( max_humanY - min_humanY + 1 );
	//int resultRate = 0.;
	int idCount = 0;
	
	//���S�v�Z(�� ���܂��ܒ��S�₻��8�ߖT���m�C�Y���������F���ďI���̂ŃR�����g�A�E�g)
	//center_humanX = ( min_humanX + max_humanX ) / 2;
	//center_humanY = ( min_humanY + max_humanY ) / 2;
	//resultID = bodyindex.data[ center_humanY * bodyindex.step + center_humanX * bodyindex.elemSize() ];
	//if( !( resultID >= 0 && resultID <= 5 ) ){	// �l��BOX�̒��S�ɐl��ID���Ȃ������ꍇ�͂���8�ߖT�_���������đ�����
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 1 ) * bodyindex.step + ( center_humanX - 1 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 1 ) * bodyindex.step + ( center_humanX - 0 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 1 ) * bodyindex.step + ( center_humanX + 1 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 0 ) * bodyindex.step + ( center_humanX - 1 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY - 0 ) * bodyindex.step + ( center_humanX + 1 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY + 1 ) * bodyindex.step + ( center_humanX - 1 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY + 1 ) * bodyindex.step + ( center_humanX - 0 ) * bodyindex.elemSize() ] );
	//	neighbor8Points.push_back( bodyindex.data[ ( center_humanY + 1 ) * bodyindex.step + ( center_humanX + 1 ) * bodyindex.elemSize() ] );
	//	for( int i = 0; i < 8; i++ ){
	//		if( neighbor8Points[i] >= 0 && neighbor8Points[i] <= 5 ){ idCounts[ neighbor8Points[i] ]++; }
	//	}
	//	std::vector<int>::iterator iter = std::max_element( idCounts.begin(), idCounts.end() );
	//	size_t index = std::distance( idCounts.begin(), iter );
	//	if( idCounts[index] != 0 ){		// �������ɂ��I�΂ꂽID(���Y��index�̒l)�ɓ����Ă���J�E���g�񐔂�0 -> �܂�8�ߖT�ɂ��l��ID�Ȃ��Ȃ� else
	//		resultID = index;
	//	}
	//	else{	// 8�ߖT���������Ă��l��ID���������Ȃ��ꍇ�A�l��BOX�Ɠ��̈��BodyIndex�؂蔲�������ԑ���ID������
			//idCounts = vector<int>(6, 0);
			for( int j = min_humanY; j < max_humanY; j++ ){
				for( int i = min_humanX; i < max_humanX; i++ ){
					int v = bodyindex.data[ j * bodyindex.step + i * bodyindex.elemSize() ];
					//if( v >= 0 && v <= 5 ){
					if( v == t_kinectID ){
						//idCounts[ v ]++;
						idCount ++;
					}
				}
			}
			//std::vector<int>::iterator iter = std::max_element( idCounts.begin(), idCounts.end() );
			//size_t index = std::distance( idCounts.begin(), iter );
			//if( idCounts[index] != 0 ){		// �������ɂ��I�΂ꂽID(���Y��index�̒l)�ɓ����Ă���J�E���g�񐔂�0 -> �܂�8�ߖT�ɂ��l��ID�Ȃ��Ȃ� else
			//	resultID = index;
			//}
			//else{
			//	resultID = 255;
			//}
	//	}
	//}
	//resultRate = idCount / humanBoxSize;

	//return resultRate;
	//return resultID;
	return idCount;
}

int main()
{
	int mode = 0;
	vector<int> param = vector<int>(2);			// PNG�ۑ��̈��k��(0�`9)
	param[0] = CV_IMWRITE_PNG_COMPRESSION;		// ���Ԃƃt�@�C���T�C�Y�̃g���[�h�I�t
	param[1] = 9;								// ���ɍ��͎��Ԃ��C�ɂ��Ȃ��̂Ń��x��9

	while(1)
	{
		//��𑜓x���T�C�Y���邩�ǂ���(�����𑜓x�Ƃ̔�r�p�Ȃ̂Ŋ�{No��OK)
		cout << "resize? (Yes = 1/No = 0)" << endl;
		cin >> mode;
		if( mode == 1 || mode == 0 ){ break; }
		else{ cout << "error input !" << endl; }
	}

	//�t�H���_���̓ǂݍ���@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//�x�[�X�t�H���_�̓ǂݍ���(�e�L�X�g�t�@�C���w��)
	ifstream ifs( "FolderPath.txt" );	// �x�[�X�t�H���_�@C:\Users\administrator\Desktop\MonitoringSystem(�K�w�^�C�x���g���m_���𑜌��o)\img
	string base_dirPath;				// �ύX����ꍇ�́@C:\Users\administrator\Desktop\AutoPoseTriming\PoseEstimator\FolderPath.txt �̓��e����������

	if( ifs.fail() )
	{
		cerr << "read file error !" << endl;
		return -1;
	}
	getline( ifs, base_dirPath );
	//cout << "base folder is: " << base_dirPath << endl;

	//�������e��������

	std::string pose2D = "pose2D";
	std::string Lhand2D = "Lhand2D";
	std::string Rhand2D = "Rhand2D";

	while(1){
		//���t_�����t�H���_�̒T���E���X�g�A�b�v
		std::vector<std::string> date_time_dirNames;
		std::string needFile;
		std::string notNeedFile = "checked.txt";
		
		//date_time_dirNames�ɁA�uchecked.txt�v�����݂��Ȃ��t�H���_�̖��O�����X�g�A�b�v
		date_time_dirNames = get_dir_path_in_dir( base_dirPath, needFile, notNeedFile );

		//���t_�����t�H���_���̏���
		for( int i = 0; i < date_time_dirNames.size(); i++ ){

			//�e���t_�����t�H���_���Atouch�t�H���_�̓ǂݍ���
			std::string date_time_dirPath = base_dirPath + "\\" + date_time_dirNames[i] + "\\touch";

			//�C�x���gID�t�H���_�̒T���E���X�g�A�b�v
			std::vector<std::string> eventID_dirNames;
			needFile = "fin_save.txt";
			notNeedFile = "edited.txt";

			//eventID_dirNames�ɁA�ufin_save.txt�v�����݂��A�uedited.txt�v�����݂��Ȃ��t�H���_�̖��O�����X�g�A�b�v
			eventID_dirNames = get_dir_path_in_dir( date_time_dirPath, needFile, notNeedFile );

			//�C�x���gID�t�H���_���̏���
			for( int j = 0; j < eventID_dirNames.size(); j++ ){

				//�e�C�x���gID�t�H���_���Acolor�t�H���_��depth�t�H���_��bodyindex�t�H���_�A�ڐG���t�H���_�̓ǂݍ���
				std::string eventID_dirPath = date_time_dirPath + "\\" + eventID_dirNames[j];
				std::string eventID_color_dirPath = eventID_dirPath + "\\color";
				std::string eventID_depth_dirPath = eventID_dirPath + "\\depth";
				std::string eventID_bodyindex_dirPath = eventID_dirPath + "\\bodyindex";
				std::string eventID_touch_info_dirPath = eventID_dirPath + "\\touch_info";
				std::string eventID_logs_dirPath = eventID_dirPath + "\\logs";
				std::string eventID_logs_json_dirPath = eventID_logs_dirPath + "\\json_csv";
				//std::string eventID_color_dirPath = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\color";			���ǂ��������Ȃ̂ŃR�����g�A�E�g
				//std::string eventID_depth_dirPath = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\depth";
				//std::string eventID_touch_info_dirPath = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\touch_info";
				std::ostringstream oss;
				oss << std::setfill('0') << std::setw(4) << eventID_dirNames[j];
				std::string dt_ev = date_time_dirNames[i] + "_" + oss.str() + "_";

				//�yOpenPose�ō��i���菈���z

				//���i���茋�ʂ̕ۑ��t�H���_�w�肨��э쐬
				std::string saveOpenPose_dirPath = eventID_dirPath + "\\openpose";
				//std::string saveOpenPose_dirPath = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\openpose";			���ǂ��������Ȃ̂ŃR�����g�A�E�g
				struct stat statBuf;
				if ( stat( saveOpenPose_dirPath.c_str(), &statBuf ) != 0 )
				{
					if( _mkdir( saveOpenPose_dirPath.c_str() ) != 0 )
					{
						cout << "cannot create directory: " << saveOpenPose_dirPath.c_str() << endl;
						return -1;
					}
				}
				//img�t�H���_
				std::string saveOpenPoseImg_dir = saveOpenPose_dirPath + "\\img";
				if(stat(saveOpenPoseImg_dir.c_str(), &statBuf) != 0)
				{
					if(_mkdir(saveOpenPoseImg_dir.c_str()) != 0)
					{
						cout << "cannot create directory: " << saveOpenPoseImg_dir.c_str() << endl;
						return -1;
					}
				}
				//skeleton�t�H���_
				std::string saveOpenPoseSkeleton_dir = saveOpenPose_dirPath + "\\skeleton";
				if(stat(saveOpenPoseSkeleton_dir.c_str(), &statBuf) != 0)
				{
					if(_mkdir(saveOpenPoseSkeleton_dir.c_str()) != 0)
					{
						cout << "cannot create directory: " << saveOpenPoseSkeleton_dir.c_str() << endl;
						return -1;
					}
				}
				//hand_points�t�H���_
				std::string saveHandPoints_dir = eventID_dirPath + "\\hand_points";
				if(stat(saveHandPoints_dir.c_str(), &statBuf) != 0)
				{
					if(_mkdir(saveHandPoints_dir.c_str()) != 0)
					{
						cout << "cannot create directory: " << saveHandPoints_dir.c_str() << endl;
						return -1;
					}
				}
				//hand_points\\pxmm�t�H���_
				std::string saveHandPoints_pxmm_dir = saveHandPoints_dir + "\\pxmm";
				if(stat(saveHandPoints_pxmm_dir.c_str(), &statBuf) != 0)
				{
					if(_mkdir(saveHandPoints_pxmm_dir.c_str()) != 0)
					{
						cout << "cannot create directory: " << saveHandPoints_pxmm_dir.c_str() << endl;
						return -1;
					}
				}
				//hand_points\\mm�t�H���_
				std::string saveHandPoints_mm_dir = saveHandPoints_dir + "\\mm";
				if(stat(saveHandPoints_mm_dir.c_str(), &statBuf) != 0)
				{
					if(_mkdir(saveHandPoints_mm_dir.c_str()) != 0)
					{
						cout << "cannot create directory: " << saveHandPoints_mm_dir.c_str() << endl;
						return -1;
					}
				}
				//hand_points\\dataset2d�t�H���_
				std::string saveHandPoints_dataset2d_dir = saveHandPoints_dir + "\\dataset2d";
				if(stat(saveHandPoints_mm_dir.c_str(), &statBuf) != 0)
				{
					if(_mkdir(saveHandPoints_dataset2d_dir.c_str()) != 0)
					{
						cout << "cannot create directory: " << saveHandPoints_dataset2d_dir.c_str() << endl;
						return -1;
					}
				}
				//���O�ۑ��p�t�H���_���쐬
				if ( stat( eventID_logs_dirPath.c_str(), &statBuf ) != 0 )
				{
					if( _mkdir( eventID_logs_dirPath.c_str() ) != 0 )
					{
						cout << "cannot create directory: " << eventID_logs_dirPath.c_str() << endl;
						return -1;
					}
				}
				if( stat( eventID_logs_json_dirPath.c_str(), &statBuf ) != 0 )
				{
					if( _mkdir( eventID_logs_json_dirPath.c_str() ) != 0 )
					{
						cout << "cannot create directory: " << eventID_logs_json_dirPath.c_str() << endl;
						return -1;
					}
				}

				//�R�}���h���̐���(OpenPoseDemo.exe �� --hand --write_images --write_json �̃I�v�V�����t���Ŏ��s
				//���͌���eventID_color_dirPath�A�o�͐��saveOpenPoseImg_dir ����� saveOpenPoseSkeleton_dir)
				//OpenPoseDemo.exe���̂�bin�t�H���_�������A���s��bin�t�H���_�̏��models�t�H���_������ꏊ������s����K�v�L��
				std::string command_str = "cd C:\\openpose\\openpose-1.4.0-win64-gpu-binaries && bin\\OpenPoseDemo.exe --image_dir " + eventID_color_dirPath + " --hand --write_images " 
					+ saveOpenPoseImg_dir + " --write_json " + saveOpenPoseSkeleton_dir;
				std::string openpose_checkfile = saveOpenPose_dirPath + "\\done.txt";
				if(!checkFileExistence(openpose_checkfile))
				{
					//�R�}���h���s
					system(command_str.c_str());

					FILE *fp;
					fp = fopen( openpose_checkfile.c_str(), "w" );
					fclose( fp );
				}

				//�y���i���茋�ʂ���摜�؂蔲�������z

				//color�t�H���_����PNG�t�@�C�����X�g�A�b�v
				vector<std::string> colorFileNames = get_file_path_in_dir( eventID_color_dirPath, "png" );

				//depth�t�H���_����PNG�t�@�C�����X�g�A�b�v
				vector<std::string> depthFileNames = get_file_path_in_dir( eventID_depth_dirPath, "png" );

				vector<std::string> bodyindexFileNames = get_file_path_in_dir( eventID_bodyindex_dirPath, "pgm" );

				//JSON�e�L�X�g�t�@�C���̃��X�g�A�b�v
				vector<std::string> skeletonFileNames = get_file_path_in_dir( saveOpenPoseSkeleton_dir, "json" );

				//���o���p���̐ڐG���e�L�X�g�t�@�C���̃��X�g�A�b�v
				vector<std::string> touch_infoFileNames = get_file_path_in_dir( eventID_touch_info_dirPath, "txt" );

				//�摜��JSON�e�L�X�g�̐��̃`�F�b�N
				if( depthFileNames.size() != skeletonFileNames.size() ){
					cout << "ERROR�F.PNG�t�@�C����.TXT�t�@�C���̐����قȂ�܂�" << endl;
					return -1;
				}

				//���o���p����////////////////////////////////////////////////////////////�ǂ̐l�����ǂ̕��̂ɐڐG�����̂��H

				int objPos_minX, objPos_maxX, objPos_minY, objPos_maxY; //�������̈ʒu
				
				for( int v = 0; v < touch_infoFileNames.size(); v++ ){
					std::string temp_infoStr = touch_infoFileNames[v];
					std::string temp_imgStr = colorFileNames[0];

					temp_infoStr.erase( temp_infoStr.end() - 6, temp_infoStr.end() );	// "_x.txt"���폜
					temp_imgStr.erase( temp_imgStr.end() - 4, temp_imgStr.end() );		// ".png"�@���폜

					long long frame1 = stoll( temp_infoStr.c_str() );
					long long frame2 = stoll( temp_imgStr.c_str() );

					std::string inputInfoPath = eventID_touch_info_dirPath + "\\" + touch_infoFileNames[v];
					std::string inputImagePath = eventID_color_dirPath + "\\" + colorFileNames[ (int)( frame1 - frame2 ) ];

					ifstream ifs( inputInfoPath.c_str() );
					if ( ifs.fail() )
					{
						cerr << "read TEXT file error !" << endl;
						return -1;
					}
					istreambuf_iterator<char> it( ifs );
					istreambuf_iterator<char> last;
					string touchInfo(it, last);
					Mat inputImage = imread( inputImagePath, 1 );

					istringstream iss( touchInfo.c_str() );
					string s;

					int info_params[13];
					int index = 0;
					//[0]obj_x1, [1]obj_x2, [2]obj_y1, [3]obj_y2, [4]obj_xcenter, [5]obj_ycenter, [6]obj_size
					//[7]human_x1, [8]human_x2, [9]human_y1, [10]human_y2, [11]human_xcenter, [12]human_ucenter
					while (iss >> s) {
						info_params[index] = stoi(s);
						index++;
					}
					info_params[0] += 80; info_params[1] += 80; info_params[4] += 80; info_params[7] += 80; info_params[8] += 80; info_params[11] += 80;
					info_params[2] += 15; info_params[3] += 15; info_params[5] += 15; info_params[9] += 15; info_params[10] += 15; info_params[12] += 15;
					info_params[0] *= 4; info_params[1] *= 4; info_params[4] *= 4; info_params[7] *= 4; info_params[8] *= 4; info_params[11] *= 4;
					info_params[2] *= 4; info_params[3] *= 4; info_params[5] *= 4; info_params[9] *= 4; info_params[10] *= 4; info_params[12] *= 4;
					info_params[6] *= 16;
					int min_obj_x = min( info_params[0], info_params[1] );		//cout << min_obj_x << "minOx" << endl;
					int max_obj_x = max( info_params[0], info_params[1] );		//cout << max_obj_x << "maxOx" << endl;
					int min_obj_y = min( info_params[2], info_params[3] );		//cout << min_obj_y << "minOy" << endl;
					int max_obj_y = max( info_params[2], info_params[3] );		//cout << max_obj_y << "maxOy" << endl;
					int min_human_x = min( info_params[7], info_params[8] );	//cout << min_human_x << "minHx" << endl;
					int max_human_x = max( info_params[7], info_params[8] );	//cout << max_human_x << "maxHx" << endl;
					int min_human_y = min( info_params[9], info_params[10] );	//cout << min_human_y << "minHy" << endl;
					int max_human_y = max( info_params[9], info_params[10] );	//cout << max_human_y << "maxHy" << endl;
					cout << "obj box: [" << info_params[0] << ", " << info_params[2] << "] - [" << info_params[1] << ", " << info_params[3] << "]" << endl;
					cout << "hmn box: [" << info_params[7] << ", " << info_params[9] << "] - [" << info_params[8] << ", " << info_params[10] << "]" << endl;
					cv::rectangle( inputImage, cv::Rect( min_obj_x, min_obj_y, max_obj_x - min_obj_x, max_obj_y - min_obj_y ),cv::Scalar(0,0,255));
					cv::rectangle( inputImage, cv::Rect( min_human_x, min_human_y, max_human_x - min_human_x, max_human_y - min_human_y ),cv::Scalar(0,255,0));
					std::string outputImagePath = eventID_touch_info_dirPath + "\\" + to_string( frame1 ) + "_touch.png";
					cout << "visualized image: " << outputImagePath.c_str() << endl;
					/*int in;
					cin >> in;*/
					imwrite( outputImagePath, inputImage );
					if( v == 0 ){
						objPos_minX = min_obj_x; objPos_maxX = max_obj_x; objPos_minY = min_obj_y; objPos_maxY = max_obj_y;  //�ŏ��Ɍ��m�������̈ʒu���o���Ă���
					}
				}

				////////////////////////////////////////////////////////////���o���p����//

				//���̂ɐڐG�����l���̓���(openpose���������g���b�L���O�ł��Ă�����̂Ƃ���)�˃g���b�L���O�ł��Ă��Ȃ�����(2019/11/15)
				//openpose���������S�l���̗���̒��ōł����̂ɋ߂����̂�����΁A���ꂪ�ڐG���Ă����ł���Ɣ��f����
				//�ڐG��11�t���[���ڂł��邽�߁A11�t���[����(imageFileNames[10])��ǂݍ���
				//Color�ǂݍ���
				std::string inputColorPath = eventID_color_dirPath + "\\" + colorFileNames[10];
				Mat inputColor = imread( inputColorPath, -1 );
				if(  inputColor.data == NULL )
				{
					cout << "read Color(PNG) image error !" << endl;
					return -1;
				}else{ //cout << "success reading PNG image."<< endl;
				}
				//Depth�ǂݍ���
				std::string inputDepthPath = eventID_depth_dirPath + "\\" + depthFileNames[10];
				//cout << "open PNG depth file: " << inputDepthPath.c_str() << endl;
				Mat inputDepth = imread( inputDepthPath, -1 );
				if( inputDepth.data == NULL )
				{
					cout << "read Depth(PNG) image error !" << endl;
					return -1;
				}else{ //cout << "success reading PNG depth."<< endl;
				}
				//BodyIndex�ǂݍ���
				std::string inputBodyIndexPath = eventID_bodyindex_dirPath + "\\" + bodyindexFileNames[10];
				//cout << "open PGM bodyindex file: " << inputBodyIndexPath.c_str() << endl;
				Mat inputBodyIndex = imread( inputBodyIndexPath, -1 );
				if( inputBodyIndex.data == NULL )
				{
					cout << "read BodyIndex(PGM) image error !" << endl;
					return -1;
				}else{ //cout << "success reading PGM bodyindex."<< endl;
				}
				//Skeleton�ǂݍ���
				std::string inputSkeletonPath = saveOpenPoseSkeleton_dir + "\\" + skeletonFileNames[10];
				//cout << "open JSON_TXT file: " << inputSkeletonPath.c_str() << endl;
				ifstream ifs4( inputSkeletonPath.c_str() );
				if ( ifs4.fail() )
				{
					cerr << "read Skeleton(JSON_TXT) file error !" << endl;
					return -1;
				}
				istreambuf_iterator<char> it( ifs4 );
				istreambuf_iterator<char> last;
				string BodyInfo(it, last);

				//input�쐬
				image input( inputColor.cols, inputColor.rows );

				//input�ɕۑ���t�H���_�p�X�o�^
				input.eventID_dirPath = eventID_dirPath;
				input.eventID_color_dirPath = eventID_color_dirPath;
				input.eventID_depth_dirPath = eventID_depth_dirPath;
				input.eventID_bodyindex_dirPath = eventID_bodyindex_dirPath;
				input.eventID_touch_info_dirPath =  eventID_touch_info_dirPath;
				input.eventID_logs_dirPath =  eventID_logs_dirPath;
				input.eventID_logs_json_dirPath = eventID_logs_json_dirPath;
				input.saveOpenPose_dirPath = saveOpenPose_dirPath;
				input.saveOpenPoseImg_dir = saveOpenPoseImg_dir;
				input.saveOpenPoseSkeleton_dir = saveOpenPoseSkeleton_dir;
				input.saveHandPoints_dir = saveHandPoints_dir;
				input.saveHandPoints_pxmm_dir = saveHandPoints_pxmm_dir;
				input.saveHandPoints_mm_dir = saveHandPoints_mm_dir;

				//Skeleton�̉��
				input.img_original = inputColor.clone();
				input.obj_minX = objPos_minX; input.obj_maxX = objPos_maxX; input.obj_minY = objPos_minY; input.obj_maxY = objPos_maxY;
				ImageEditUtil::drawOpenPoseSkeleton( input, BodyInfo );

				double minDistance = -1;
				double touched_person = -1;
				int TouchHand_0L1R = -1;
				int Left = 0, Right = 1;

				//std::vector<PII> test = input.get_skeletonType2D( 1, Lhand2D ); �l�����ƂɃf�[�^�����Ă���Ǝv������A�܂Ƃ߂Ď��Ă��܂��Ă���
				
				//�e�X�g(��点)
				//objPos_minX = 881;
				//objPos_minY = 405;
				//objPos_maxX = 908;
				//objPos_maxY = 441;

				// ���̂̏d�S�ʒu���v�Z
				bool isObjDepthGet = false;
				int ObjCenterX = (int)( (double)( objPos_maxX + objPos_minX ) / 2. );
				int ObjCenterY = (int)( (double)( objPos_maxY + objPos_minY ) / 2. );
				int ObjCenterDepth = (int)input.get_fixedDepth( objPos_minX, objPos_minY, objPos_maxX, objPos_maxY, inputDepth );
				//int ObjCenterDepthOld = inputDepth.at<ushort>( ObjCenterY, ObjCenterX );  // �O��l�����O���Ď擾���邽�߂̊֐������K�v
				//int ObjCenterDepth = inputDepth.at<ushort>( 398, 876 ); //�e�X�g(��点)

				if( ObjCenterDepth > 0 ){
					isObjDepthGet = true;
				}else if( ObjCenterDepth == -1 ){
					cout << "Object : Invalid area (main.cpp return from image::get_fixedDepth())" << endl;
				}else if( ObjCenterDepth == -2 ){
					cout << "Object : no valid depth value (main.cpp return from image::get_fixedDepth())" << endl;
				}

				if( isObjDepthGet == false ){
					cout << "cannot get depth value of the object center.( end of this event )" << endl;
					std::string editedTextFileName = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\edited.txt";
					std::string depth_XTextFileName = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\NoObjDepth.txt";
					FILE *fp;
					fp = fopen( editedTextFileName.c_str(), "w" );
					fclose( fp );
					fp = fopen( depth_XTextFileName.c_str(), "w" );
					fclose( fp );
					continue;
				}

				//pixel ���� mm �ւ̕ϊ�
				//�摜���E������mm���v�Z
				//�摜���S(960, 540)��Depth�擾

				//��XY�����̎Z�o���@
				//�摜��������摜�[�܂ł̕�(960pix)��KinectV2�̎���p�̔���(35��)���Atan��p���āA�J��������摜���ʂ̒����܂ł�pixel���������߂�
				//���߂���������(pixel)�ƖړI���̂̉摜���ʏ㒆������̕�(pixel)���A�O�����̒藝��p���āA�J��������摜���ʂ̖ړI���̂܂ł�pixel���������߂�
				//�ړI���̂܂ł̎�������(mm)�ƁA��ŋ��߂��摜���ʏ�ł̖ړI���̂܂ł�pixel�����̔䗦����A�摜�㕝(pixel)�����l��(mm)�ɕϊ�

				//�摜���ʏ㒆���܂ł�pixel�������v�Z(����͒萔)
				double CenterDepthPixel_X = ( input.Width() / 2. ) / tan( 35. * M_PI / 180. );
				double CenterDepthPixel_Y = ( input.Height() / 2. ) / tan( 30. * M_PI / 180. );

				//���̂̉摜���ʏ㒆�������pixel�����v�Z(���S��荶�A��Ȃ炻�ꂼ��XY���̒l)
				double width_ObjCenterToImgCenter = ObjCenterX - input.Width() / 2.;
				double height_ObjCenterToImgCenter = ObjCenterY - input.Height() / 2.;

				//��������̕��ƒ�����������O�����̒藝���A�摜���ʏ�ł̕��̈ʒu�܂ł�pixel�������v�Z
				double ObjCenterDepthPixel_X = sqrt( pow( CenterDepthPixel_X, 2. ) + pow( width_ObjCenterToImgCenter, 2. ) );
				double ObjCenterDepthPixel_Y = sqrt( pow( CenterDepthPixel_Y, 2. ) + pow( height_ObjCenterToImgCenter, 2. ) );

				//�����l�����ƌv�Zpixel�������A���̑��݈ʒu���ʂł̐���/�����������ꂼ���pixel->mm�ϊ��䗦�����߂�
				double ratioPixToMm_ObjPlaneX = ObjCenterDepth / ObjCenterDepthPixel_X;
				double ratioPixToMm_ObjPlaneY = ObjCenterDepth / ObjCenterDepthPixel_Y;

				//�摜���������_�Ƃ��镨�̏d�S��X�EY���W��pixel->mm�ϊ�
				double w_OCTIC_mm = width_ObjCenterToImgCenter * ratioPixToMm_ObjPlaneX;
				double h_OCTIC_mm = height_ObjCenterToImgCenter * ratioPixToMm_ObjPlaneY;
				cout << "ObjectPosition(mm): [ " << w_OCTIC_mm << ", " << h_OCTIC_mm << ", " << ObjCenterDepth << " ]" << endl;

				//�t���[���Ɏʂ��Ă���l��1�l������
				for( int i = 0; i < input.skeletonSets2D.size(); i++ )
				{
					//cout << "check touch: person_" << i+1 << "(/" << input.skeletonSets2D.size() << ")" << endl;

					// ����̏d�S�v�Z�p��\�_�̍��W�擾					
					std::vector<point2D> Lhand2Ds = input.get_skeletonType2D( i, Lhand2D );
					std::vector<point2D> Rhand2Ds = input.get_skeletonType2D( i, Rhand2D );
					std::vector<point2D> pose2Ds = input.get_skeletonType2D( i, pose2D );
					double numFoundLHPoint = 0.;
					double numFoundRHPoint = 0.;
					int sumLHPointX = 0; int sumLHPointY = 0;
					int sumRHPointX = 0; int sumRHPointY = 0;
					int min_humanX = -1, min_humanY = -1, max_humanX = -1, max_humanY = -1;
					int min_LhandX = -1, min_LhandY = -1, max_LhandX = -1, max_LhandY = -1;
					int min_RhandX = -1, min_RhandY = -1, max_RhandX = -1, max_RhandY = -1;

					for( int lp = 0; lp < /*21*/Lhand2Ds.size(); lp++  )
					{
						if( Lhand2Ds[lp/*+i*21*/].accuracy > 0.05 )
						//if( !( Lhand2Ds[lp+i*21].x == 0 && Lhand2Ds[lp+i*21].y == 0 ) )
						{
							if( min_LhandX == -1 ){ min_LhandX = Lhand2Ds[ lp ].x; }
							if( min_LhandY == -1 ){ min_LhandY = Lhand2Ds[ lp ].y; }
							if( max_LhandX == -1 ){ max_LhandX = Lhand2Ds[ lp ].x; }
							if( max_LhandY == -1 ){ max_LhandY = Lhand2Ds[ lp ].y; }
							if( Lhand2Ds[ lp/*+i*25*/ ].x < min_LhandX ){ min_LhandX = Lhand2Ds[ lp/*+i*25*/ ].x; }
							if( Lhand2Ds[ lp/*+i*25*/ ].y < min_LhandY ){ min_LhandY = Lhand2Ds[ lp/*+i*25*/ ].y; }
							if( Lhand2Ds[ lp/*+i*25*/ ].x > max_LhandX ){ max_LhandX = Lhand2Ds[ lp/*+i*25*/ ].x; }
							if( Lhand2Ds[ lp/*+i*25*/ ].y > max_LhandY ){ max_LhandY = Lhand2Ds[ lp/*+i*25*/ ].y; }
							sumLHPointX += Lhand2Ds[lp/*+i*21*/].x;
							sumLHPointY += Lhand2Ds[lp/*+i*21*/].y;
							numFoundLHPoint ++;
						}
					}
					for( int rp = 0; rp < /*21*/Rhand2Ds.size(); rp++  )
					{
						if( Rhand2Ds[rp/*+i*21*/].accuracy > 0.05 )
						//if( !( Rhand2Ds[rp+i*21].x == 0 && Rhand2Ds[rp+i*21].y == 0 ) )
						{
							if( min_RhandX == -1 ){ min_RhandX = Rhand2Ds[ rp ].x; }
							if( min_RhandY == -1 ){ min_RhandY = Rhand2Ds[ rp ].y; }
							if( max_RhandX == -1 ){ max_RhandX = Rhand2Ds[ rp ].x; }
							if( max_RhandY == -1 ){ max_RhandY = Rhand2Ds[ rp ].y; }
							if( Rhand2Ds[ rp/*+i*25*/ ].x < min_RhandX ){ min_RhandX = Rhand2Ds[ rp/*+i*25*/ ].x; }
							if( Rhand2Ds[ rp/*+i*25*/ ].y < min_RhandY ){ min_RhandY = Rhand2Ds[ rp/*+i*25*/ ].y; }
							if( Rhand2Ds[ rp/*+i*25*/ ].x > max_RhandX ){ max_RhandX = Rhand2Ds[ rp/*+i*25*/ ].x; }
							if( Rhand2Ds[ rp/*+i*25*/ ].y > max_RhandY ){ max_RhandY = Rhand2Ds[ rp/*+i*25*/ ].y; }
							sumRHPointX += Rhand2Ds[rp/*+i*21*/].x;
							sumRHPointY += Rhand2Ds[rp/*+i*21*/].y;
							numFoundRHPoint ++;
						}
					}
					//�֐ߓ_1�P�̍��W�𑖍����āAmin��max��������
					for( int pp = 0; pp < /*25*/pose2Ds.size(); pp++ ){
						if( pose2Ds[ pp/*+i*25*/ ].accuracy >= 0.3 ){
						//if( !( pose2Ds[ pp+i*25 ].x == 0 && pose2Ds[ pp+i*25 ].y == 0 ) ){
							if( min_humanX == -1 ){ min_humanX = pose2Ds[ pp/*+i*25*/ ].x; }
							if( min_humanY == -1 ){ min_humanY = pose2Ds[ pp/*+i*25*/ ].y; }
							if( max_humanX == -1 ){ max_humanX = pose2Ds[ pp/*+i*25*/ ].x; }
							if( max_humanY == -1 ){ max_humanY = pose2Ds[ pp/*+i*25*/ ].y; }
							if( pose2Ds[ pp/*+i*25*/ ].x < min_humanX ){ min_humanX = pose2Ds[ pp/*+i*25*/ ].x; }
							if( pose2Ds[ pp/*+i*25*/ ].y < min_humanY ){ min_humanY = pose2Ds[ pp/*+i*25*/ ].y; }
							if( pose2Ds[ pp/*+i*25*/ ].x > max_humanX ){ max_humanX = pose2Ds[ pp/*+i*25*/ ].x; }
							if( pose2Ds[ pp/*+i*25*/ ].y > max_humanY ){ max_humanY = pose2Ds[ pp/*+i*25*/ ].y; }
						}
					}
					
					if( numFoundLHPoint > 0 || numFoundRHPoint > 0 ){	// �ǂ��炩�̎肪�������Ă���l������input�ɐl���̈�Ƃ���BOX�o�^
						image::HumanBox iHumanBox;
						iHumanBox.minX = min_humanX;
						iHumanBox.minY = min_humanY;
						iHumanBox.maxX = max_humanX;
						iHumanBox.maxY = max_humanY;
						iHumanBox.centerX = ( min_humanX + max_humanX ) / 2;
						iHumanBox.centerY = ( min_humanY + max_humanY ) / 2;

						//�t���[����i�Ԗ�(OpenPose�̐l�����m��������ID)�̐l����BOX���W��input.humanBoxes�ɓo�^
						input.humanBoxes.push_back( iHumanBox );
					}

					/*int finger1 = 1; int finger2 = 5; int finger3 = 9; int finger4 = 13; int finger5 = 17;
					int f1_lx = Lhand2Ds[finger1].first;
					int f1_ly = Lhand2Ds[finger1].second; if( !( f1_lx == 0 && f1_ly == 0 ) ) numFoundLHPoint++;
					int f2_lx = Lhand2Ds[finger2].first;
					int f2_ly = Lhand2Ds[finger2].second; if( !( f2_lx == 0 && f2_ly == 0 ) ) numFoundLHPoint++;
					int f3_lx = Lhand2Ds[finger3].first;
					int f3_ly = Lhand2Ds[finger3].second; if( !( f3_lx == 0 && f3_ly == 0 ) ) numFoundLHPoint++;
					int f4_lx = Lhand2Ds[finger4].first;
					int f4_ly = Lhand2Ds[finger4].second; if( !( f4_lx == 0 && f4_ly == 0 ) ) numFoundLHPoint++;
					int f5_lx = Lhand2Ds[finger5].first;
					int f5_ly = Lhand2Ds[finger5].second; if( !( f5_lx == 0 && f5_ly == 0 ) ) numFoundLHPoint++;
					
					int f1_rx = Rhand2Ds[finger1].first;
					int f1_ry = Rhand2Ds[finger1].second; if( !( f1_rx == 0 && f1_ry == 0 ) ) numFoundRHPoint++;
					int f2_rx = Rhand2Ds[finger2].first;
					int f2_ry = Rhand2Ds[finger2].second; if( !( f2_rx == 0 && f2_ry == 0 ) ) numFoundRHPoint++;
					int f3_rx = Rhand2Ds[finger3].first;
					int f3_ry = Rhand2Ds[finger3].second; if( !( f3_rx == 0 && f3_ry == 0 ) ) numFoundRHPoint++;
					int f4_rx = Rhand2Ds[finger4].first;
					int f4_ry = Rhand2Ds[finger4].second; if( !( f4_rx == 0 && f4_ry == 0 ) ) numFoundRHPoint++;
					int f5_rx = Rhand2Ds[finger5].first;
					int f5_ry = Rhand2Ds[finger5].second; if( !( f5_rx == 0 && f5_ry == 0 ) ) numFoundRHPoint++;*/
					
					// ����̏d�S���W���v�Z
					// �l������([0, 0]�łȂ�)�w�̊֐ߓ_�݂̂Ōv�Z����
					int LhandCenterX = -1, LhandCenterY = -1, RhandCenterX = -1, RhandCenterY = -1;
					bool isLHandFound, isRHandFound;
					if( numFoundLHPoint == 0 ){			// ����̏d�S�v�Z�p�֐ߓ_������������Ă��Ȃ��ꍇ�A���肪�������Ă��Ȃ����̂Ƃ���
						isLHandFound = false;
					}
					else{
						LhandCenterX = (int)( (double)( sumLHPointX/*f1_lx + f2_lx + f3_lx + f4_lx + f5_lx*/ ) / numFoundLHPoint );
						LhandCenterY = (int)( (double)( sumLHPointY/*f1_ly + f2_ly + f3_ly + f4_ly + f5_ly*/ ) / numFoundLHPoint );
						isLHandFound = true;
					}
					if( numFoundRHPoint == 0 ){			// �E��̏d�S�v�Z�p�֐ߓ_������������Ă��Ȃ��ꍇ�A�E�肪�������Ă��Ȃ����̂Ƃ���
						isRHandFound = false;
					}
					else{
						RhandCenterX = (int)( (double)( sumRHPointX/*f1_rx + f2_rx + f3_rx + f4_rx + f5_rx*/ ) / numFoundRHPoint );
						RhandCenterY = (int)( (double)( sumRHPointY/*f1_ry + f2_ry + f3_ry + f4_ry + f5_ry*/ ) / numFoundRHPoint );
						isRHandFound = true;
					}

					// ��ƕ��̂̐[�x���擾
					int LhandCenterDepth = -1, RhandCenterDepth = -1;
					bool isLHDepthGet = false, isRHDepthGet = false;
					if( isLHandFound ){
						LhandCenterDepth = (int)input.get_fixedDepth( min_LhandX, min_LhandY, max_LhandX, max_LhandY, inputDepth );
						//LhandCenterDepth = inputDepth.at<ushort>( LhandCenterY, LhandCenterX );// �O��l�����O���Ď擾���邽�߂̊֐������K�v
						if( LhandCenterDepth > 0 ){
							isLHDepthGet = true;
						}else if( LhandCenterDepth == -1 ){
							cout << "Lhand : Invalid area (main.cpp return from image::get_fixedDepth())" << endl;
						}else if( LhandCenterDepth == -2 ){
							cout << "Lhand : no valid depth value (main.cpp return from image::get_fixedDepth())" << endl;
						}
					}else{
						isLHDepthGet = false;
					}
					if( isRHandFound ){
						RhandCenterDepth = (int)input.get_fixedDepth( min_RhandX, min_RhandY, max_RhandX, max_RhandY, inputDepth );
						///RhandCenterDepth = inputDepth.at<ushort>( RhandCenterY, RhandCenterX );// �O��l�����O���Ď擾���邽�߂̊֐������K�v
						if( RhandCenterDepth > 0 ){
							isRHDepthGet = true;
						}else if( RhandCenterDepth == -1 ){
							cout << "Rhand : Invalid area (main.cpp return from image::get_fixedDepth())" << endl;
						}else if( RhandCenterDepth == -2 ){
							cout << "Rhand : no valid depth value (main.cpp return from image::get_fixedDepth())" << endl;
						}
					}else{
						isRHDepthGet = false;
					}

					//image�N���X�ɁA11�t���[���ڂ̐ڐG�摜�Ŏʂ��Ă���l���̎��̏���o�^( OpenPoseID, LorR, x, y, z(depth) )���[�v�őS�����@�ˁ@�g���̂͂�߂�
					image::Hand11F person_i_L, person_i_R;
					if( isLHandFound ){
						person_i_L.OPID = i;
						person_i_L.L0R1 = Left;
						person_i_L.x = LhandCenterX; person_i_L.y = LhandCenterY;
						person_i_L.z = LhandCenterDepth;
						input.Lhands.push_back( person_i_L );
					}
					if( isRHandFound ){
						person_i_R.OPID = i;
						person_i_R.L0R1 = Right;
						person_i_R.x = RhandCenterX; person_i_R.y = RhandCenterY;
						person_i_R.z = RhandCenterDepth;
						input.Rhands.push_back( person_i_R );
					}

					//int imgCenterDepth = inputDepth.at<ushort>( input.Height(), input.Width() );

					// ��ƕ��̂�3��������(2��)���v�Z
					double distanceLHtoO = -1, distanceRHtoO = -1;
					bool canCalcLHtoO, canCalcRHtoO;
					if( isLHDepthGet ){

						//�����XYpixel���W��mm���W�ɕϊ�
						double width_LHCenterToImgCenter = LhandCenterX - input.Width() / 2.;
						double height_LHCenterToImgCenter = LhandCenterY - input.Height() / 2.;
						double LHCenterDepthPixel_X = sqrt( pow( CenterDepthPixel_X, 2. ) + pow( width_LHCenterToImgCenter, 2. ) );
						double LHCenterDepthPixel_Y = sqrt( pow( CenterDepthPixel_Y, 2. ) + pow( height_LHCenterToImgCenter, 2. ) );
						double ratioPixToMm_LHPlaneX = LhandCenterDepth / LHCenterDepthPixel_X;
						double ratioPixToMm_LHPlaneY = LhandCenterDepth / LHCenterDepthPixel_Y;
						double w_LHCTIC_mm = width_LHCenterToImgCenter * ratioPixToMm_LHPlaneX;
						double h_LHCTIC_mm = height_LHCenterToImgCenter * ratioPixToMm_LHPlaneY;
						cout << "human "<< i <<" LeftHandPosition(mm): [ " << w_LHCTIC_mm << ", " << h_LHCTIC_mm << ", " << LhandCenterDepth << " ]" << endl;

						distanceLHtoO = pow( w_LHCTIC_mm - w_OCTIC_mm, 2 ) + pow( h_LHCTIC_mm - h_OCTIC_mm, 2 ) + pow( LhandCenterDepth - ObjCenterDepth, 2 );
						canCalcLHtoO = true;
					}else{ canCalcLHtoO = false; }
					if( isRHDepthGet ){

						//�E���XYpixel���W��mm���W�ɕϊ�
						double width_RHCenterToImgCenter = RhandCenterX - input.Width() / 2.;
						double height_RHCenterToImgCenter = RhandCenterY - input.Height() / 2.;
						double RHCenterDepthPixel_X = sqrt( pow( CenterDepthPixel_X, 2. ) + pow( width_RHCenterToImgCenter, 2. ) );
						double RHCenterDepthPixel_Y = sqrt( pow( CenterDepthPixel_Y, 2. ) + pow( height_RHCenterToImgCenter, 2. ) );
						double ratioPixToMm_RHPlaneX = RhandCenterDepth / RHCenterDepthPixel_X;
						double ratioPixToMm_RHPlaneY = RhandCenterDepth / RHCenterDepthPixel_Y;
						double w_RHCTIC_mm = width_RHCenterToImgCenter * ratioPixToMm_RHPlaneX;
						double h_RHCTIC_mm = height_RHCenterToImgCenter * ratioPixToMm_RHPlaneY;
						cout << "human "<< i <<" RightHandPosition(mm): [ " << w_RHCTIC_mm << ", " << h_RHCTIC_mm << ", " << RhandCenterDepth << " ]" << endl;

						distanceRHtoO = pow( w_RHCTIC_mm - w_OCTIC_mm, 2 ) + pow( h_RHCTIC_mm - h_OCTIC_mm, 2 ) + pow( RhandCenterDepth - ObjCenterDepth, 2 );
						canCalcRHtoO = true;
					}else{ canCalcRHtoO = false; }

					// ���E���ꂼ��̎�ƕ��̂�3�����������r���āA���̐l���́A���̂ɍł��߂��������
					double PersonDistHtoO;
					int TouchHand = -1;
					bool existPersonDistHtoO;
					if( canCalcLHtoO && canCalcRHtoO ){	//�����̎�ƕ��̒��S�Ƃ�3��������(2��)���v�Z�o���Ă����ꍇ
						if( distanceLHtoO > distanceRHtoO ){ PersonDistHtoO = distanceRHtoO; TouchHand = Right; }
						else if( distanceRHtoO > distanceLHtoO ){ PersonDistHtoO = distanceLHtoO; TouchHand = Left; }
						existPersonDistHtoO = true;
					}
					else if( !canCalcRHtoO && canCalcLHtoO ){ PersonDistHtoO = distanceLHtoO; TouchHand = Left; existPersonDistHtoO = true; } //����ƕ��̒��S�Ƃ�3��������(2��)�݂̂��v�Z�o���Ă����ꍇ
					else if( !canCalcLHtoO && canCalcRHtoO ){ PersonDistHtoO = distanceRHtoO; TouchHand = Right; existPersonDistHtoO = true; } //�E��ƕ��̒��S�Ƃ�3��������(2��)�݂̂��v�Z�o���Ă����ꍇ
					else{ existPersonDistHtoO = false; } //����Ƃ����̒��S�Ƃ̋������v�Z�ł��Ă��Ȃ������ꍇ
					
					if( existPersonDistHtoO && minDistance == -1 ){ minDistance = PersonDistHtoO; TouchHand_0L1R = TouchHand; touched_person = i; continue; }
					if( existPersonDistHtoO && ( PersonDistHtoO < minDistance ) ){ minDistance = PersonDistHtoO; TouchHand_0L1R = TouchHand; touched_person = i; }
				}
				//���O�c���p(�l��ID(OpenPose)�A��ƕ��̂̍ŏ������A�ǂ���̎�H)//////////////////////////////
																											//
				std::string saveFileName( colorFileNames[10] );												//
				std::string saveFrame = saveFileName.substr( 0, 8 );										//
				ofstream outputID( ( eventID_logs_dirPath + "\\touched_personID.txt" ).c_str());			//
				outputID << touched_person;																	//
				outputID.close();																			//
				ofstream outputDistance( ( eventID_logs_dirPath + "\\minDistanceHtoO.txt" ).c_str());		//
				outputDistance << minDistance ;																//
				outputDistance.close();																		//
				ofstream outputWhichHand( ( eventID_logs_dirPath + "\\TouchHand.txt" ).c_str());			//
				if( TouchHand_0L1R == 0 ) outputWhichHand << "Left" ;										//
				if( TouchHand_0L1R == 1 ) outputWhichHand << "Right" ;										//
				if( TouchHand_0L1R == -1 ) outputWhichHand << "Unknown" ;									//
				outputWhichHand.close();																	//
																											//
				//�R���\�[���\��(����)�E�t���O�Ǘ�////////////////////////////////////////////////////////////////////////////////
																																//
				bool t_person_exist;	//���̂ɐڐG����l����11�t���[���ڂɌ���������										//
				if( minDistance == -1 && TouchHand_0L1R == -1 && touched_person == -1 ){										//
					cout << "no person touched in this frame." << endl;															//
					t_person_exist = false;																						//
				}																												//
				else{																											//
					if( TouchHand_0L1R == 0 ){																					//
						cout << "person " << touched_person << " touched with Left(" << TouchHand_0L1R << ") hand " << endl;	//
					}																											//
					if( TouchHand_0L1R == 1 ){																					//
						cout << "person " << touched_person << " touched with Right(" << TouchHand_0L1R << ") hand " << endl;	//
					}																											//
					if( minDistance > -1 ){																						//
						cout << "minimum distance to object from hand is: " << minDistance << endl;								//
					}																											//
					if( touched_person > -1 ){																					//
						cout << "ID: " << touched_person << "(openposeID) person tocuhed." << endl;								//
					}																											//
					t_person_exist = true;																						//
				}																												//
																																//
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				//11�t���[���ڐڐG���Ɏʂ��Ă���l���S���̎�ʒu����BodyIndex�Ƃ�Ή��Â��� �� input���ɑΉ�����ێ�
				//mapHandsToBodyIndex( input, inputBodyIndex ); �����A��߂�

				//�t���[���ɉf��l��BOX��BodyIndex��ID���(KinectID�A�l���ǐ�)��Ή��Â���
				mapHumanBoxToBodyIndex( input, inputBodyIndex, 1 );
				
				//���O�c���p//////////////////////////////////////////////////////////////////////////////////////
																												//
				ofstream outputPersonIDMap11F( ( eventID_logs_dirPath + "\\person_id_map11F.txt" ).c_str());	//
				outputPersonIDMap11F << "OpenPoseID / KinectID" << endl;										//
				for( int log = 0; log < input.HumanBoxKinectIDs.size(); log++ ){								//
					outputPersonIDMap11F << log << " / " << input.HumanBoxKinectIDs[log] << endl;				//
				}																								//
				outputPersonIDMap11F.close();																	//
																												//
				//////////////////////////////////////////////////////////////////////////////////////////////////

				//�l��(OpenPose)�؂蔲���摜�̐���

				//Mat t_humanROI_11F;
				//Mat t_humanROI_11F_G;
				//Mat v_t_humanOnBodyIndex = inputBodyIndex.clone();
				////int min_humanX = -1, min_humanY = -1, max_humanX = -1, max_humanY = -1;
				//
				////if( touched_person != -1 ){
				////	std::vector<PII> pose2Ds = input.get_skeletonType2D( touched_person, pose2D );
				////	
				////	//�֐ߓ_1�P�̍��W�𑖍����āAmin��max��������
				////	for( int point = 0; point < 25; point++ ){
				////		if( !( pose2Ds[ point+touched_person*25 ].first == 0 && pose2Ds[ point+touched_person*25 ].second == 0 ) ){
				////			if( min_humanX == -1 ){ min_humanX = pose2Ds[ point+touched_person*25 ].first; }
				////			if( min_humanY == -1 ){ min_humanY = pose2Ds[ point+touched_person*25 ].second; }
				////			if( max_humanX == -1 ){ max_humanX = pose2Ds[ point+touched_person*25 ].first; }
				////			if( max_humanY == -1 ){ max_humanY = pose2Ds[ point+touched_person*25 ].second; }
				////			if( pose2Ds[ point+touched_person*25 ].first < min_humanX ){ min_humanX = pose2Ds[ point+touched_person*25 ].first; }
				////			if( pose2Ds[ point+touched_person*25 ].second < min_humanY ){ min_humanY = pose2Ds[ point+touched_person*25 ].second; }
				////			if( pose2Ds[ point+touched_person*25 ].first > max_humanX ){ max_humanX = pose2Ds[ point+touched_person*25 ].first; }
				////			if( pose2Ds[ point+touched_person*25 ].second > max_humanY ){ max_humanY = pose2Ds[ point+touched_person*25 ].second; }
				////		}
				////	}
				//
				//	Mat roi( inputColor, cv::Rect( min_humanX, min_humanY, max_humanX - min_humanX, max_humanY - min_humanY ) );
				//	t_humanROI_11F = roi.clone();
				//	cv::rectangle( v_t_humanOnBodyIndex, cv::Rect( min_humanX, min_humanY, max_humanX - min_humanX, max_humanY - min_humanY ), 100, cv::LINE_4 );
				//	cvtColor( t_humanROI_11F, t_humanROI_11F_G, CV_RGB2GRAY );
				//}
				//imshow( "humanBOX on BodyIndex[11frame]", v_t_humanOnBodyIndex );
				//cv::waitKey(100);

				//Kinect���l���ǐՂɊ���U�����l��ID�����
				int TouchedHumanKinectID;
				//TouchedHumanKinectID = getKinectHumanID( min_humanX, max_humanX, min_humanY, max_humanY, inputBodyIndex, input );
				TouchedHumanKinectID = input.HumanBoxKinectIDs[touched_person];

				//���̐ڐG�l�����͂�+�e�l��KinectID���t�^�ɂ�����
				Mat inputColorCopy = inputColor.clone();
				int tPerson_minX = input.humanBoxes[touched_person].minX;
				int tPerson_minY = input.humanBoxes[touched_person].minY;
				int tPerson_maxX = input.humanBoxes[touched_person].maxX;
				int tPerson_maxY = input.humanBoxes[touched_person].maxY;

				cv::rectangle( inputColorCopy, cv::Point( tPerson_minX, tPerson_minY ), cv::Point( tPerson_maxX, tPerson_maxY ), cv::Scalar(0,0,200), 5, 8 );
				
				for( int v = 0; v < input.HumanBoxKinectIDs.size(); v++ ){
					if( input.HumanBoxKinectIDs[v] == -1 ){
						continue;
					}
					else{
						string num = to_string( input.HumanBoxKinectIDs[v] );
						int person_centerX = input.humanBoxes[v].centerX;
						int person_centerY = input.humanBoxes[v].centerY;

						cv::putText( inputColorCopy, num.c_str(), cv::Point( person_centerX, person_centerY ), cv::FONT_HERSHEY_SIMPLEX|cv::FONT_ITALIC, 4, cv::Scalar(100,200,100), 4, CV_AA);
					}
				}
				string outputIDImage11F = eventID_logs_dirPath + "\\touched_frameIDinfo.png";
				imwrite( outputIDImage11F, inputColorCopy );

				//���O�c���p(�l��ID(Kinect, �ǐՂ���))////////////////////////////////////////////////////////////
																												//
				ofstream outputpKID( ( eventID_logs_dirPath + "\\touched_personKinectID.txt" ).c_str());		//
				outputpKID << TouchedHumanKinectID;																//
				outputpKID.close();																				//
																												//
				//////////////////////////////////////////////////////////////////////////////////////////////////

				//�l���̈�BOX�̒��S���W�ɑΉ�����BodyIndex��ID���擾(������Ȃ���΂��̋ߕӂ̉�f������)
				//���S�v�Z
				//int center_humanX, center_humanY;
				//center_humanX = ( min_humanX + max_humanX ) / 2;
				//center_humanY = ( min_humanY + max_humanY ) / 2;
				//
				//KinectHumanID = inputBodyIndex.data[ center_humanY * inputBodyIndex.step + center_humanX * inputBodyIndex.elemSize ];
				//if( !( KinectHumanID >= 1 && KinectHumanID <= 6 ) ){	// �l��BOX�̒��S�ɐl��ID���Ȃ������ꍇ�͂���8�ߖT�_���������đ�����
				//	vector<int> neighbor8Points;
				//	vector<int> idCounts(7, 0);		// ids[0] �͖������� ids[1]�`ids[6]�̒l
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY - 1 ) * inputBodyIndex.step + ( center_humanX - 1 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY - 1 ) * inputBodyIndex.step + ( center_humanX - 0 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY - 1 ) * inputBodyIndex.step + ( center_humanX + 1 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY - 0 ) * inputBodyIndex.step + ( center_humanX - 1 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY - 0 ) * inputBodyIndex.step + ( center_humanX + 1 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY + 1 ) * inputBodyIndex.step + ( center_humanX - 1 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY + 1 ) * inputBodyIndex.step + ( center_humanX - 0 ) * inputBodyIndex.elemSize ] );
				//	neighbor8Points.push_back( inputBodyIndex.data[ ( center_humanY + 1 ) * inputBodyIndex.step + ( center_humanX + 1 ) * inputBodyIndex.elemSize ] );
				//	for( int i = 0; i < 8; i++ ){
				//		if( neighbor8Points[i] >= 1 && neighbor8Points[i] <= 6 ){ idCounts[ neighbor8Points[i] ]++; }
				//	}
				//	std::vector<int>::iterator iter = std::max_element( idCounts.begin(), idCounts.end() );
				//	size_t index = std::distance( idCounts.begin(), iter );
				//	if( idCounts[index] != 0 ){		// �������ɂ��I�΂ꂽID(���Y��index�̒l)�ɓ����Ă���J�E���g�񐔂�0 -> �܂�8�ߖT�ɂ��l��ID�Ȃ��Ȃ� else
				//		KinectHumanID = index;
				//	}
				//	else{	// 8�ߖT���������Ă��l��ID���������Ȃ��ꍇ�A�l��BOX�Ɠ��̈��BodyIndex�؂蔲�������ԑ���ID������
				//		idCounts.clear();
				//		for( int j = min_humanY; j < max_humanY; j++ ){
				//			for( int i = min_humanX; i < max_humanX; i++ ){
				//				int v = inputBodyIndex.data[ j * inputBodyIndex.step + i * inputBodyIndex.elemSize ];
				//				if( v >= 1 && v <= 6 ){
				//					idCounts[ v ]++;
				//				}
				//			}
				//		}
				//		std::vector<int>::iterator iter = std::max_element( idCounts.begin(), idCounts.end() );
				//		size_t index = std::distance( idCounts.begin(), iter );
				//		if( idCounts[index] != 0 ){		// �������ɂ��I�΂ꂽID(���Y��index�̒l)�ɓ����Ă���J�E���g�񐔂�0 -> �܂�8�ߖT�ɂ��l��ID�Ȃ��Ȃ� else
				//			KinectHumanID = index;
				//		}
				//		else{
				//			KinectHumanID = 0;
				//		}
				//	}
				//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				////�e�t���[���̕��̂ɐڐG�����l��ID�̒ǐ�(OpenPose������U��l��ID�͐l���ǐՂ����Ă��Ȃ�����)
				////���̐ڐG�����Ă���͂���11�t���[���ڂ���o�b�N���Đl����ǐՂ�����ƁA11�t���[���ȍ~�̐l����ǐՂ�����Ƃŕ����ă��[�v
				//vector<int> touched_personIDs;
				//Mat past_humanROI = t_humanROI_11F_G.clone();
				//
				//for( int i = 0; i < 10; i++ ){
				//	touched_personIDs.push_back( -1 );
				//}
				//
				//for( int p = 9; p >= 0; p-- ){
				//
				//	//PNG�t�@�C�����X�g���J���[�摜��1���ǂݍ���
				//	std::string inputImagePath = eventID_color_dirPath + "\\" + colorFileNames[p];
				//	//cout << "open PNG image file: " << inputImagePath.c_str() << endl;
				//	Mat inputImage = imread( inputImagePath, -1 );
				//	if( inputImage.data == NULL )
				//	{
				//		cout << "read PNG image error !" << endl;
				//		return -1;
				//	}else{ /*cout << "success reading PNG image."<< endl;*/ }
				//
				//	//PNG�t�@�C�����X�g���[�x�摜��1���ǂݍ���
				//	std::string inputDepthPath = eventID_depth_dirPath + "\\" + depthFileNames[p];
				//	//cout << "open PNG depth file: " << inputDepthPath.c_str() << endl;
				//	Mat inputDepth = imread( inputDepthPath, -1 );
				//	if( inputDepth.data == NULL )
				//	{
				//		cout << "read PNG image error !" << endl;
				//		return -1;
				//	}else{ /*cout << "success reading PNG depth."<< endl;*/ }
				//
				//	//16bit�[�x�摜����8bit�[�x�摜�𐶐�
				//	Mat inputDepth8U;
				//	inputDepth.convertTo( inputDepth8U, CV_8U, 255. / 2990., -1625. * 255. / 2990. );
				//	if( inputDepth8U.data == NULL )
				//	{
				//		cout << "generate depth(CV_8U) image error !" << endl;
				//		return -1;
				//	}else{ /*cout << "success generating depth(CV_8U) image." << endl;*/ }
				//
				//	//JSON�e�L�X�g�t�@�C�����X�g��荜�i����1�f�[�^���ǂݍ���
				//	std::string inputSkeletonPath = saveOpenPoseSkeleton_dir + "\\" + skeletonFileNames[p];
				//	//cout << "open JSON_TXT file: " << inputSkeletonPath.c_str() << endl;
				//	ifstream ifs4( inputSkeletonPath.c_str() );
				//	if ( ifs4.fail() )
				//	{
				//		cerr << "read JSON_TXT file error !" << endl;
				//		return -1;
				//	}
				//	istreambuf_iterator<char> it( ifs4 );
				//	istreambuf_iterator<char> last;
				//	string BodyInfo(it, last);
				//
				//	//Skeleton�̉��
				//	//image input( inputImage.cols, inputImage.rows );
				//	input.img_original = inputImage.clone();
				//	input.img_original_depth = inputDepth.clone();
				//	input.img_original_depth8U = inputDepth8U.clone();
				//	input.obj_minX = objPos_minX; input.obj_maxX = objPos_maxX; input.obj_minY = objPos_minY; input.obj_maxY = objPos_maxY;
				//	ImageEditUtil::drawOpenPoseSkeleton( input, BodyInfo );
				//
				//	vector<Mat> t_humanROIs;
				//	
				//	//�t���[���Ɏʂ��Ă���l��1�l����BOX�C���[�W���쐬
				//	for( int i = 0; i < input.skeletonSets2D.size(); i++ )
				//	{
				//		std::string pose2D = "pose2D";
				//		std::vector<PII> pose2Ds = input.get_skeletonType2D( i, pose2D );
				//		int min_humanX = -1, min_humanY = -1, max_humanX = -1, max_humanY = -1;
				//
				//		//�֐ߓ_1�P�̍��W�𑖍����āAmin��max��������
				//		for( int point = 0; point < 25; point++ ){
				//			if( !( pose2Ds[ point ].first == 0 && pose2Ds[ point ].second == 0 ) ){
				//				if( min_humanX == -1 ){ min_humanX = pose2Ds[ point ].first; }
				//				if( min_humanY == -1 ){ min_humanY = pose2Ds[ point ].second; }
				//				if( max_humanX == -1 ){ max_humanX = pose2Ds[ point ].first; }
				//				if( max_humanY == -1 ){ max_humanY = pose2Ds[ point ].second; }
				//				if( pose2Ds[ point ].first < min_humanX ){ min_humanX = pose2Ds[ point ].first; }
				//				if( pose2Ds[ point ].second < min_humanY ){ min_humanY = pose2Ds[ point ].second; }
				//				if( pose2Ds[ point ].first > max_humanX ){ max_humanX = pose2Ds[ point ].first; }
				//				if( pose2Ds[ point ].second > max_humanY ){ max_humanY = pose2Ds[ point ].second; }
				//			}
				//		}
				//
				//		Mat roiColor( inputImage, cv::Rect( min_humanX, min_humanY, max_humanX - min_humanX, max_humanY - min_humanY ) );
				//		//Mat roiDepth( inputDepth, cv::Rect( min_humanX, min_humanY, max_humanX - min_humanX, max_humanY - min_humanY ) );
				//		t_humanROIs.push_back(roiColor);
				//	}
				//
				//	//���̃t���[����1�l���l�����f���Ă��Ȃ������ꍇ���̃t���[����
				//	if( t_humanROIs.size() == 0 ){
				//		//touched_personIDs.push_back( -1 );
				//		continue;
				//	}
				//
				//	//�q�X�g�O�����ގ��x�v�Z
				//	double frame_correlation = -1;
				//	bool isTracked = false;
				//	//touched_personIDs.push_back( -1 );
				//
				//	for( i = 0; i < t_humanROIs.size(); i++ ){
				//		//���݂̃t���[����i�Ԗ�ID�̐l���؂蔲���摜���O���[�X�P�[����
				//		Mat tempGrayImg;
				//		cvtColor(t_humanROIs[i], tempGrayImg, CV_RGB2GRAY );
				//
				//		int imageCount = 1; // ���͉摜�̖���
				//		int channelsToUse[] = { 0 }; // 0�Ԗڂ̃`���l�����g��
				//		int dimention = 1; // �q�X�g�O�����̎�����
				//		int binCount = 256; //�q�X�g�O�����̃r���̐�
				//		int binCounts[] = { binCount };
				//		float range[] = { 0, 256 }; // �f�[�^�͈̔͂�0�`255
				//
				//		const float* histRange[] = { range };
				//
				//		Mat histogram1;
				//		calcHist( &tempGrayImg, imageCount, channelsToUse, Mat(), histogram1, dimention, binCounts, histRange );
				//		Mat histogram2;
				//		calcHist( &past_humanROI, imageCount, channelsToUse, Mat(), histogram2, dimention, binCounts, histRange );
				//
				//		//�ގ��x�𒲂ׂ�
				//		double correlation = compareHist( histogram1, histogram2, CV_COMP_CORREL );
				//		if( frame_correlation == -1 ){
				//			if( correlation > 0.3 ){
				//				frame_correlation = correlation;
				//				isTracked = true;
				//				touched_personIDs[p] = i;	//TODO�F�ގ��x���̂�pair�ŕۑ�����ׂ�
				//			}
				//		}
				//		if( correlation < frame_correlation ){
				//			if( correlation > 0.3 ){
				//				frame_correlation = correlation;
				//				isTracked = true;
				//				touched_personIDs[p] = i;	//TODO�F�ގ��x���̂�pair�ŕۑ�����ׂ�
				//			}
				//		}
				//
				//	}
				//	if( touched_personIDs[p] != -1 ){
				//		past_humanROI = t_humanROIs[touched_personIDs[p]]; // ���̃t���[���̕��̐ڐG�l����ID���ǐՂł��Ă�����A���̐l���̐؂蔲���摜�����̗ގ��x�v�Z�̔�r�ΏۂƂ���
				//	}
				//}
				//
				//touched_personIDs.push_back( touched_person ); //11�t���[���ڂ͂��łɐl���̒ǐՂ��s�����̂ł��̂܂ܑ��
				//
				//for( int p = 11; p < colorFileNames.size(); p++ ){ //12�t���[���ڈȍ~�̑S�t���[���ɑ΂��Ă��l����ID�̒ǐ�
				//
				//	//PNG�t�@�C�����X�g���J���[�摜��1���ǂݍ���
				//	std::string inputImagePath = eventID_color_dirPath + "\\" + colorFileNames[p];
				//	//cout << "open PNG image file: " << inputImagePath.c_str() << endl;
				//	Mat inputImage = imread( inputImagePath, -1 );
				//	if( inputImage.data == NULL )
				//	{
				//		cout << "read PNG image error !" << endl;
				//		return -1;
				//	}else{ /*cout << "success reading PNG image."<< endl;*/ }
				//
				//	//PNG�t�@�C�����X�g���[�x�摜��1���ǂݍ���
				//	std::string inputDepthPath = eventID_depth_dirPath + "\\" + depthFileNames[p];
				//	//cout << "open PNG depth file: " << inputDepthPath.c_str() << endl;
				//	Mat inputDepth = imread( inputDepthPath, -1 );
				//	if( inputDepth.data == NULL )
				//	{
				//		cout << "read PNG image error !" << endl;
				//		return -1;
				//	}else{ /*cout << "success reading PNG depth."<< endl;*/ }
				//
				//	//16bit�[�x�摜����8bit�[�x�摜�𐶐�
				//	Mat inputDepth8U;
				//	inputDepth.convertTo( inputDepth8U, CV_8U, 255. / 2990., -1625. * 255. / 2990. );
				//	if( inputDepth8U.data == NULL )
				//	{
				//		cout << "generate depth(CV_8U) image error !" << endl;
				//		return -1;
				//	}else{ /*cout << "success generating depth(CV_8U) image." << endl;*/ }
				//
				//	//JSON�e�L�X�g�t�@�C�����X�g��荜�i����1�f�[�^���ǂݍ���
				//	std::string inputSkeletonPath = saveOpenPoseSkeleton_dir + "\\" + skeletonFileNames[p];
				//	//cout << "open JSON_TXT file: " << inputSkeletonPath.c_str() << endl;
				//	ifstream ifs4( inputSkeletonPath.c_str() );
				//	if ( ifs4.fail() )
				//	{
				//		cerr << "read JSON_TXT file error !" << endl;
				//		return -1;
				//	}
				//	istreambuf_iterator<char> it( ifs4 );
				//	istreambuf_iterator<char> last;
				//	string BodyInfo(it, last);
				//
				//	//Skeleton�̉��
				//	//image input( inputImage.cols, inputImage.rows );
				//	input.img_original = inputImage.clone();
				//	input.img_original_depth = inputDepth.clone();
				//	input.img_original_depth8U = inputDepth8U.clone();
				//	input.obj_minX = objPos_minX; input.obj_maxX = objPos_maxX; input.obj_minY = objPos_minY; input.obj_maxY = objPos_maxY;
				//	ImageEditUtil::drawOpenPoseSkeleton( input, BodyInfo );
				//
				//	vector<Mat> t_humanROIs;
				//	
				//	//�t���[���Ɏʂ��Ă���l��1�l����BOX�C���[�W���쐬
				//	for( int i = 0; i < input.skeletonSets2D.size(); i++ )
				//	{
				//		std::string pose2D = "pose2D";
				//		std::vector<PII> pose2Ds = input.get_skeletonType2D( i, pose2D );
				//		int min_humanX = -1, min_humanY = -1, max_humanX = -1, max_humanY = -1;
				//
				//		//�֐ߓ_1�P�̍��W�𑖍����āAmin��max��������
				//		for( int point = 0; point < 25; point++ ){
				//			if( !( pose2Ds[ point ].first == 0 && pose2Ds[ point ].second == 0 ) ){
				//				if( min_humanX == -1 ){ min_humanX = pose2Ds[ point ].first; }
				//				if( min_humanY == -1 ){ min_humanY = pose2Ds[ point ].second; }
				//				if( max_humanX == -1 ){ max_humanX = pose2Ds[ point ].first; }
				//				if( max_humanY == -1 ){ max_humanY = pose2Ds[ point ].second; }
				//				if( pose2Ds[ point ].first < min_humanX ){ min_humanX = pose2Ds[ point ].first; }
				//				if( pose2Ds[ point ].second < min_humanY ){ min_humanY = pose2Ds[ point ].second; }
				//				if( pose2Ds[ point ].first > max_humanX ){ max_humanX = pose2Ds[ point ].first; }
				//				if( pose2Ds[ point ].second > max_humanY ){ max_humanY = pose2Ds[ point ].second; }
				//			}
				//		}
				//
				//		Mat roiColor( inputImage, cv::Rect( min_humanX, min_humanY, max_humanX - min_humanX, max_humanY - min_humanY ) );
				//		//Mat roiDepth( inputDepth, cv::Rect( min_humanX, min_humanY, max_humanX - min_humanX, max_humanY - min_humanY ) );
				//		t_humanROIs.push_back(roiColor);
				//	}
				//
				//	//���̃t���[����1�l���l�����f���Ă��Ȃ������ꍇ���̃t���[����
				//	if( t_humanROIs.size() == 0 ){
				//		touched_personIDs.push_back( -1 );
				//		continue;
				//	}
				//
				//	//�q�X�g�O�����ގ��x�v�Z
				//	double frame_correlation = -1;
				//	bool isTracked = false;
				//	touched_personIDs.push_back( -1 );
				//
				//	for( i = 0; i < t_humanROIs.size(); i++ ){
				//		//���݂̃t���[����i�Ԗ�ID�̐l���؂蔲���摜���O���[�X�P�[����
				//		Mat tempGrayImg;
				//		cvtColor(t_humanROIs[i], tempGrayImg, CV_RGB2GRAY );
				//
				//		int imageCount = 1; // ���͉摜�̖���
				//		int channelsToUse[] = { 0 }; // 0�Ԗڂ̃`���l�����g��
				//		int dimention = 1; // �q�X�g�O�����̎�����
				//		int binCount = 256; //�q�X�g�O�����̃r���̐�
				//		int binCounts[] = { binCount };
				//		float range[] = { 0, 256 }; // �f�[�^�͈̔͂�0�`255
				//
				//		const float* histRange[] = { range };
				//
				//		Mat histogram1;
				//		calcHist( &tempGrayImg, imageCount, channelsToUse, Mat(), histogram1, dimention, binCounts, histRange );
				//		Mat histogram2;
				//		calcHist( &past_humanROI, imageCount, channelsToUse, Mat(), histogram2, dimention, binCounts, histRange );
				//
				//		//�ގ��x�𒲂ׂ�
				//		double correlation = compareHist( histogram1, histogram2, CV_COMP_CORREL );
				//		if( frame_correlation == -1 ){
				//			if( correlation > 0.3 ){
				//				frame_correlation = correlation;
				//				isTracked = true;
				//				touched_personIDs[p] = i;	//TODO�F�ގ��x���̂�pair�ŕۑ�����ׂ�
				//			}
				//		}
				//		if( correlation < frame_correlation ){
				//			if( correlation > 0.3 ){
				//				frame_correlation = correlation;
				//				isTracked = true;
				//				touched_personIDs[p] = i;	//TODO�F�ގ��x���̂�pair�ŕۑ�����ׂ�
				//			}
				//		}
				//
				//	}
				//	if( touched_personIDs[p] != -1 ){
				//		past_humanROI = t_humanROIs[touched_personIDs[p]]; // ���̃t���[���̕��̐ڐG�l����ID���ǐՂł��Ă�����A���̐l���̐؂蔲���摜�����̗ގ��x�v�Z�̔�r�ΏۂƂ���
				//	}
				//}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				//�摜1�����Ƃ̏���
				for( int p = 0; p < colorFileNames.size(); p++){
					cout << p << ":" << colorFileNames[p] << endl;
					if(p == 1){
						cout << "it: " << colorFileNames[p] << endl;
					}
					//PNG�t�@�C�����X�g���J���[�摜��1���ǂݍ���
					std::string inputImagePath = eventID_color_dirPath + "\\" + colorFileNames[p];
					//cout << "open PNG image file: " << inputImagePath.c_str() << endl;
					Mat inputImage = imread( inputImagePath, -1 );
					if( inputImage.data == NULL )
					{
						cout << "read PNG image error !" << endl;
						return -1;
					}else{ /*cout << "success reading PNG image."<< endl;*/ }

					//PNG�t�@�C�����X�g���[�x�摜��1���ǂݍ���
					std::string inputDepthPath = eventID_depth_dirPath + "\\" + depthFileNames[p];
					//cout << "open PNG depth file: " << inputDepthPath.c_str() << endl;
					Mat inputDepth = imread( inputDepthPath, -1 );
					if( inputDepth.data == NULL )
					{
						cout << "read PNG image error !" << endl;
						return -1;
					}else{ /*cout << "success reading PNG depth."<< endl;*/ }
					
					//PGM�t�@�C�����X�g���l���̈�摜��1���ǂݍ���
					std::string inputBodyIndexPath = eventID_bodyindex_dirPath + "\\" + bodyindexFileNames[p];
					//cout << "open PGM bodyindex file: " << inputBodyIndexPath.c_str() << endl;
					Mat inputBodyIndex = imread( inputBodyIndexPath, -1 );
					if( inputBodyIndex.data == NULL )
					{
						cout << "read PGM image error !" << endl;
						return -1;
					}else{ /*cout << "success reading PGM bodyindex."<< endl;*/ }

					//16bit�[�x�摜����8bit�[�x�摜�𐶐�
					Mat inputDepth8U;
					inputDepth.convertTo( inputDepth8U, CV_8U, 255. / 2990., -1625. * 255. / 2990. );
					if( inputDepth8U.data == NULL )
					{
						cout << "generate depth(CV_8U) image error !" << endl;
						return -1;
					}else{ /*cout << "success generating depth(CV_8U) image." << endl;*/ }

					//JSON�e�L�X�g�t�@�C�����X�g��荜�i����1�f�[�^���ǂݍ���
					std::string inputSkeletonPath = saveOpenPoseSkeleton_dir + "\\" + skeletonFileNames[p];
					//cout << "open JSON_TXT file: " << inputSkeletonPath.c_str() << endl;
					ifstream ifs4( inputSkeletonPath.c_str() );
					if ( ifs4.fail() )
					{
						cerr << "read JSON_TXT file error !" << endl;
						return -1;
					}
					istreambuf_iterator<char> it( ifs4 );
					istreambuf_iterator<char> last;
					string BodyInfo(it, last);

					//Skeleton�̉��
					//image input( inputImage.cols, inputImage.rows );
					input.img_original = inputImage.clone();
					input.img_original_depth = inputDepth.clone();
					input.img_original_depth8U = inputDepth8U.clone();
					input.img_original_bodyindex = inputBodyIndex.clone();
					input.obj_minX = objPos_minX; input.obj_maxX = objPos_maxX; input.obj_minY = objPos_minY; input.obj_maxY = objPos_maxY;
					
					//�ۑ��t�@�C�����̐���
					std::string saveFileName( colorFileNames[p] );
					std::string saveFrame = saveFileName.substr( 0, 8 );
					//saveFileName = saveFrame + "_openpose";
					input.saveFrame = saveFrame;

					if( p == 17 ){
						cout << "872 frame" << endl;
					}
					ImageEditUtil::drawOpenPoseSkeleton( input, BodyInfo );

					//�X�P���g���d�ˉ摜�̕ۑ�
					//�t�H���_�p�X�̎w��
					string coverSkeletonDirPath = eventID_dirPath + "\\cover skeleton";
					string coverSkeletonColorDirPath = coverSkeletonDirPath + "\\color";
					string coverSkeletonDepthDirPath = coverSkeletonDirPath + "\\depth";
					string coverSkeletonDepth8UDirPath = coverSkeletonDirPath + "\\depth8U";
					//input�Ƀp�X��o�^
					input.coverSkeletonDirPath = coverSkeletonDirPath;
					input.coverSkeletonColorDirPath = coverSkeletonColorDirPath;
					input.coverSkeletonDepthDirPath = coverSkeletonDepthDirPath;
					input.coverSkeletonDepth8UDirPath = coverSkeletonDepth8UDirPath;
					//�e�t�H���_�쐬
					//�y�֐ߓ_�Q�`��t�H���_�z
					if( stat( coverSkeletonDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( coverSkeletonDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << coverSkeletonDirPath << endl;
							return -1;
						}
					}
					//�y�֐ߓ_�Q�d�ˉ摜(color)�z
					if ( stat( coverSkeletonColorDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( coverSkeletonColorDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << coverSkeletonColorDirPath << endl;
							return -1;
						}
					}
					//�y�֐ߓ_�Q�d�ˉ摜(depth)�z
					if ( stat( coverSkeletonDepthDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( coverSkeletonDepthDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << coverSkeletonDepthDirPath << endl;
							return -1;
						}
					}
					//�y�֐ߓ_�Q�d�ˉ摜(depth8U)�z
					if ( stat( coverSkeletonDepth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( coverSkeletonDepth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << coverSkeletonDepth8UDirPath << endl;
							return -1;
						}
					}
					//�t�@�C���p�X�����E�摜�ۑ�
					std::string saveFilePath_coverColor = coverSkeletonColorDirPath + "\\" + saveFrame + "_cover_color.png";
					std::string saveFilePath_coverDepth = coverSkeletonDepthDirPath + "\\" + saveFrame + "_cover_depth.png";
					std::string saveFilePath_coverDepth8U = coverSkeletonDepth8UDirPath + "\\" + saveFrame + "_cover_depth8U.png";
					imwrite( saveFilePath_coverColor, input.img_coverSkeletonColor, param );
					imwrite( saveFilePath_coverDepth, input.img_coverSkeletonDepth, param );
					imwrite( saveFilePath_coverDepth8U, input.img_coverSkeletonDepth8U, param );

					//���e�\��
					//�C���[�W
					//resize( input.img_coverSkeleton, input.img_resize, Size( 960, 540 ) );
					//imshow( "skelton on image", input.img_coverSkeleton );
					//waitKey(0);
					//
					//�ʂ��Ă���l���������ӂ�؂蔲���ĕۑ�(���Ȃ�2019/08/06)
					//cout << input.skeletonSets2D.size() << "person detected" << endl;	//3D���[�h���Ή�
					//
					//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
					//for( int i = 0; i < input.skeletonSets2D.size(); i++ )
					//{
					//	//���̂������̎�𐄒�@��������A���̈ʒu���L�^���Ă����͍폜����
					//	//Neck(1) �ɑ΂��āA RElbow(3) �� LElbow(6) �̂ǂ��炪��艓����(�����ق������̂�͂�ł���Ɖ���)
					//	int Neck = 1, RElbow = 3, LElbow = 6, RWrist = 4, LWrist = 7;	// OpenPose(BODY_25)��JointType�Ɠ������l
					//	std::string pose2D = "pose2D";
					//	std::vector<PII> pose2Ds = input.get_skeletonType2D( i, pose2D );
					//
					//	int midX = pose2Ds[Neck].first;
					//	int midY = pose2Ds[Neck].second;
					//	int RElbowX = pose2Ds[RElbow].first;
					//	int RElbowY = pose2Ds[RElbow].second;
					//	int LElbowX = pose2Ds[LElbow].first;
					//	int LElbowY = pose2Ds[LElbow].second;
					//	int RWristX = pose2Ds[RWrist].first;
					//	int RWristY = pose2Ds[RWrist].second;
					//	int LWristX = pose2Ds[LWrist].first;
					//	int LWristY = pose2Ds[LWrist].second;
					//	
					//	/*
					//	double RDiff = sqrt( pow( RWristX - midX, 2. ) + pow( RWristY - midY, 2. ) );
					//	double LDiff = sqrt( pow( LWristX - midX, 2. ) + pow( LWristY - midY, 2. ) );
					//	int WristX, WristY, ElbowX, ElbowY;
					//	if( LDiff - RDiff >= 250. )
					//	{
					//		WristX = LWristX;
					//		WristY = LWristY;
					//		ElbowX = LElbowX;
					//		ElbowY = LElbowY;
					//	}
					//	else if( LDiff - RDiff <= -250. )
					//	{
					//		WristX = RWristX;
					//		WristY = RWristY;
					//		ElbowX = RElbowX;
					//		ElbowY = RElbowY;
					//	}
					//	else
					//	{
					//		cout << "I don't know which hand is active ! ( human:" << i+1 << ")" << endl;
					//		cout << "Please Select one [ Left: 0 | Right: 1 ]" << endl;
					//
					//		imshow( "skeleton on image", input.img_resize );
					//		waitKey(1);
					//		int whichHand;
					//		while(1)
					//		{
					//			cin >> whichHand;
					//			if( whichHand == 0 || whichHand == 1 )
					//			{
					//				break;
					//			}
					//			else
					//			{
					//				cout << "Error ! input again [ Left: 0 | Right: 1 ]" << endl;
					//			}
					//		}
					//
					//		if( whichHand == 0 )
					//		{
					//			WristX = LWristX;
					//			WristY = LWristY;
					//			ElbowX = LElbowX;
					//			ElbowY = LElbowY;
					//		}
					//		else if( whichHand == 1 )
					//		{
					//			WristX = RWristX;
					//			WristY = RWristY;
					//			ElbowX = RElbowX;
					//			ElbowY = RElbowY;
					//		}
					//	}
					//	//�̂̒��S����r�������ق��𕨑̑��쒆�̎�Ƃ݂Ȃ��āA�e�w�ʒu( ThumbLeft(22) or ThumbRight(24) )�𒆐S�ɁA
					//	//��������I�ʒu( ElbowLeft(5) or ElbowRight(9) )�܂ł̋����𔼌a�Ƃ���̈�Ő؂蔲���ۑ�
					//	*/
					//
					//	//����w�̎��ӂ�؂蔲��
					//	double radius_R;
					//	double radius_L;
					//	int roiX_R, roiY_R, roiWidth_R, roiHeight_R;
					//	int roiX_L, roiY_L, roiWidth_L, roiHeight_L;
					//
					//	radius_R = sqrt( pow( RElbowX - RWristX, 2. ) + pow( RElbowY - RWristY, 2. ) );
					//	roiX_R = max( RWristX - (int)radius_R, 0 );
					//	roiY_R = max( RWristY - (int)radius_R, 0 );
					//	roiWidth_R = min( (int)radius_R * 2, 1080 );
					//	roiHeight_R = min( (int)radius_R * 2, 1080 );
					//	if( roiX_R + roiWidth_R >= 1920 ){ roiWidth_R = 1920 - roiX_R; }
					//	if( roiY_R + roiHeight_R >= 1080 ){ roiHeight_R = 1080 - roiY_R; }
					//	roiWidth_R = min( roiWidth_R, roiHeight_R );
					//	roiHeight_R = roiWidth_R;
					//	cout << "x: " << roiX_R << "\ny: " << roiY_R << "\nwidth: " << roiWidth_R << "\nheight: " << roiHeight_R << endl;
					//
					//	
					//	Mat roi_img_R( imageCopyColor, cv::Rect( roiX_R, roiY_R, roiWidth_R, roiHeight_R ) );
					//
					//	//����p�����
					//	radius_L = sqrt( pow( LElbowX - LWristX, 2. ) + pow( LElbowY - LWristY, 2. ) );
					//	roiX_L = max( LWristX - (int)radius_L, 0 );
					//	roiY_L = max( LWristY - (int)radius_L, 0 );
					//	roiWidth_L = min( (int)radius_L * 2, 1080 );
					//	roiHeight_L = min( (int)radius_L * 2, 1080 );
					//	if( roiX_L + roiWidth_L >= 1920 ){ roiWidth_L = 1920 - roiX_L; }
					//	if( roiY_L + roiHeight_L >= 1080 ){ roiHeight_L = 1080 - roiY_L; }
					//	roiWidth_L = min( roiWidth_L, roiHeight_L );
					//	roiHeight_L = roiWidth_L;
					//	cout << "x: " << roiX_L << "\ny: " << roiY_L << "\nwidth: " << roiWidth_L << "\nheight: " << roiHeight_L << endl;
					//	Mat roi_img_L( imageCopyColor, cv::Rect( roiX_L, roiY_L, roiWidth_L, roiHeight_L ) );
					//
					//	
					//}
					//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

					Mat imageCopyColor = input.img_original.clone();
					Mat imageCopyDepth = input.img_original_depth.clone();
					Mat imageCopyDepth8U = input.img_original_depth8U.clone();
					Mat imageCopyBodyIndex = input.img_original_bodyindex.clone();
					Mat imageCopyBodyIndexDraw;
					Mat imageCopyColorDraw;

					
					//���O�c���p(�l��ID(Kinect, �ǐՂ���))////////////////////////////////////////////////////////////////////////////////
																																		//
					ofstream outputID( ( eventID_logs_dirPath + "\\touched_personOPID_on_" + saveFrame + ".txt" ).c_str());				//
					if( input.skeletonSets2D.size() == 0 ){																				//
						outputID << "no person detected." << endl;																		//
						continue;																										//
					} //��l���l���������Ă��Ȃ��ꍇ���̃t���[����																	//
					if( !t_person_exist ){																								//
						outputID << "no touch on 11 frame." << endl;																	//
						continue;																										//
					} //11�t���[���ڂŕ��̐ڐG���Ă���l�����������Ă��Ȃ��ꍇ���̃t���[����											//
					if( TouchedHumanKinectID == -1 ){																					//
						outputID << "cannot match KinectID to touched_person."<< endl;													//
						continue;																										//
					} //11�t���[���ڂŕ��̐ڐG���s�����ł��낤�l����KinectID���킩��Ȃ��ꍇ���̃t���[����								//
					//outputID.close();																									//
																																		//
					//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					
					//���݃t���[���ɂ����镨�̐ڐG���s�����l���̗̈�BOX��Kinect�̐l��ID�̐������Ƃ�
					int HumanOpenPoseID = -1;
					bool canMatchID = false;
					double maxCount = -1;

					for( int i = 0; i < input.skeletonSets2D.size(); i++ ){		//�f���Ă���l���̐������[�v
						imageCopyBodyIndexDraw = imageCopyBodyIndex.clone();
						int min_pHumanX = -1, min_pHumanY = -1, max_pHumanX = -1, max_pHumanY = -1;
						int min_lhpX = -1, min_lhpY = -1, max_lhpX = -1, max_lhpY = -1;
						int min_rhpX = -1, min_rhpY = -1, max_rhpX = -1, max_rhpY = -1;
						std::vector<point2D> pose2Ds = input.get_skeletonType2D( i, pose2D );
						// ����̏d�S�v�Z�p��\�_�̍��W�擾					
						std::vector<point2D> Lhand2Ds = input.get_skeletonType2D( i, Lhand2D );
						std::vector<point2D> Rhand2Ds = input.get_skeletonType2D( i, Rhand2D );
						double numFoundLHPoint = 0.;
						double numFoundRHPoint = 0.;
						int HumanKinectID;
						int count_sameKinectID = 0.;

						//����̊֐ߓ_1��1�̍��W�𑖍����āAmin��max��������
						for( int lp = 0; lp < /*21*/Lhand2Ds.size(); lp++  )
						{
							if( Lhand2Ds[lp/*+i*21*/].accuracy > 0.05 )
								//if( !( Lhand2Ds[lp+i*21].x == 0 && Lhand2Ds[lp+i*21].y == 0 ) )
							{
								if( min_lhpX == -1 ){ min_lhpX = Lhand2Ds[ lp/*+i*21*/ ].x; }
								if( min_lhpY == -1 ){ min_lhpY = Lhand2Ds[ lp/*+i*21*/ ].y; }
								if( max_lhpX == -1 ){ max_lhpX = Lhand2Ds[ lp/*+i*21*/ ].x; }
								if( max_lhpY == -1 ){ max_lhpY = Lhand2Ds[ lp/*+i*21*/ ].y; }
								if( Lhand2Ds[ lp/*+i*21*/ ].x < min_lhpX ){ min_lhpX = Lhand2Ds[ lp/*+i*21*/ ].x; }
								if( Lhand2Ds[ lp/*+i*21*/ ].y < min_lhpY ){ min_lhpY = Lhand2Ds[ lp/*+i*21*/ ].y; }
								if( Lhand2Ds[ lp/*+i*21*/ ].x > max_lhpX ){ max_lhpX = Lhand2Ds[ lp/*+i*21*/ ].x; }
								if( Lhand2Ds[ lp/*+i*21*/ ].y > max_lhpY ){ max_lhpY = Lhand2Ds[ lp/*+i*21*/ ].y; }
								numFoundLHPoint ++;
							}
						}
						//����̒��S���v�Z
						int center_lhpX = (int)( (double)( min_lhpX + max_lhpX ) / 2. );
						int center_lhpY = (int)( (double)( min_lhpY + max_lhpY ) / 2. );
						
						//�E��̊֐ߓ_1��1�̍��W�𑖍����āAmin��max��������
						for( int rp = 0; rp < /*21*/Rhand2Ds.size(); rp++  )
						{
							if( Rhand2Ds[rp/*+i*21*/].accuracy > 0.05 )
								//if( !( Rhand2Ds[rp+i*21].x == 0 && Rhand2Ds[rp+i*21].y == 0 ) )
							{
								if( min_rhpX == -1 ){ min_rhpX = Rhand2Ds[ rp/*+i*21*/ ].x; }
								if( min_rhpY == -1 ){ min_rhpY = Rhand2Ds[ rp/*+i*21*/ ].y; }
								if( max_rhpX == -1 ){ max_rhpX = Rhand2Ds[ rp/*+i*21*/ ].x; }
								if( max_rhpY == -1 ){ max_rhpY = Rhand2Ds[ rp/*+i*21*/ ].y; }
								if( Rhand2Ds[ rp/*+i*21*/ ].x < min_rhpX ){ min_rhpX = Rhand2Ds[ rp/*+i*21*/ ].x; }
								if( Rhand2Ds[ rp/*+i*21*/ ].y < min_rhpY ){ min_rhpY = Rhand2Ds[ rp/*+i*21*/ ].y; }
								if( Rhand2Ds[ rp/*+i*21*/ ].x > max_rhpX ){ max_rhpX = Rhand2Ds[ rp/*+i*21*/ ].x; }
								if( Rhand2Ds[ rp/*+i*21*/ ].y > max_rhpY ){ max_rhpY = Rhand2Ds[ rp/*+i*21*/ ].y; }
								numFoundRHPoint ++;
							}
						}
						//�E��̒��S���v�Z
						int center_rhpX = (int)( (double)( min_rhpX + max_rhpX ) / 2. );
						int center_rhpY = (int)( (double)( min_rhpY + max_rhpY ) / 2. );

						//�S�g�̊֐ߓ_1�P�̍��W�𑖍����āAmin��max��������(Neck��MidHip�͂���ɌʂɎ擾)
						int Neck = 1, MidHip = 8;
						int pNeckX = -1, pNeckY = -1, pMidHipX = -1, pMidHipY = -1;
						for( int point = 0; point < /*25*/pose2Ds.size(); point++ ){
							if( pose2Ds[ point/*+i*25*/ ].accuracy >= 0.3 ){
							//if( !( pose2Ds[ point+i*25 ].x == 0 && pose2Ds[ point+i*25 ].y == 0 ) ){
								if( min_pHumanX == -1 ){ min_pHumanX = pose2Ds[ point/*+i*25*/ ].x; }
								if( min_pHumanY == -1 ){ min_pHumanY = pose2Ds[ point/*+i*25*/ ].y; }
								if( max_pHumanX == -1 ){ max_pHumanX = pose2Ds[ point/*+i*25*/ ].x; }
								if( max_pHumanY == -1 ){ max_pHumanY = pose2Ds[ point/*+i*25*/ ].y; }
								if( pose2Ds[ point/*+i*25*/ ].x < min_pHumanX ){ min_pHumanX = pose2Ds[ point/*+i*25*/ ].x; }
								if( pose2Ds[ point/*+i*25*/ ].y < min_pHumanY ){ min_pHumanY = pose2Ds[ point/*+i*25*/ ].y; }
								if( pose2Ds[ point/*+i*25*/ ].x > max_pHumanX ){ max_pHumanX = pose2Ds[ point/*+i*25*/ ].x; }
								if( pose2Ds[ point/*+i*25*/ ].y > max_pHumanY ){ max_pHumanY = pose2Ds[ point/*+i*25*/ ].y; }
								if( point == Neck ){ pNeckX = pose2Ds[ point/*+i*25*/ ].x; pNeckY = pose2Ds[ point/*+i*25*/ ].y; }
								if( point == MidHip ){ pMidHipX = pose2Ds[ point/*+i*25*/ ].x; pMidHipY = pose2Ds[ point/*+i*25*/ ].y; }
							}
						}
						//Neck����MidHip�ɂ����Ă̋����Ƃ���4����1�̒������v�Z(��̐؂蔲��BOX�̃T�C�Y��Ƃ���p)
						double trunkLength = sqrt( pow( (double)( pNeckX - pMidHipX ), 2. ) + pow( (double)( pNeckY - pMidHipY ), 2. ) );
						int quartTrunk = (int)( trunkLength / 4. );

						//cv::rectangle( imageCopyBodyIndexDraw, cv::Rect( min_pHumanX, min_pHumanY, max_pHumanX - min_pHumanX, max_pHumanY - min_pHumanY ), 100, cv::LINE_4 );
						//imshow( "humanBOX on BodyIndex", imageCopyBodyIndexDraw );
						//cv::waitKey(1);

						//��_�ł����̐l���́A�M���ł���(confidence 0.05�ȏ�)��̍��i�_�������input�ɓo�^����
						if( numFoundLHPoint > 0 || numFoundRHPoint > 0 ){
							image::HumanBox iHumanBox;
							iHumanBox.minX = min_pHumanX;
							iHumanBox.minY = min_pHumanY;
							iHumanBox.maxX = max_pHumanX;
							iHumanBox.maxY = max_pHumanY;
							iHumanBox.centerX = (int)( ( min_pHumanX + max_pHumanX ) / 2. );
							iHumanBox.centerY = (int)( ( min_pHumanY + max_pHumanY ) / 2. );

							//���̐l���́A�M���ł���(confidence 0.05�ȏ�)����̍��i�_�������input�ɓo�^����
							if( numFoundLHPoint > 0 ){
								image::LHandBox iLHandBox;
								iLHandBox.minX = max( /*min_lhpX*/center_lhpX - quartTrunk, 0 );
								iLHandBox.minY = max( /*min_lhpY*/center_lhpY - quartTrunk, 0 );
								iLHandBox.maxX = min( /*max_lhpX*/center_lhpX + quartTrunk, 1919 );
								iLHandBox.maxY = min( /*max_lhpY*/center_lhpY + quartTrunk, 1079 );
								iLHandBox.centerX = center_lhpX;
								iLHandBox.centerY = center_lhpY;
								iLHandBox.numFoundPoint = numFoundLHPoint;

								input.frameLHandBoxes.push_back( iLHandBox );
							}
							//�Ȃ���ΑS�v�f-1��input�ɓo�^
							else{
								image::LHandBox iLHandBox;
								iLHandBox.minX = -1;
								iLHandBox.minY = -1;
								iLHandBox.maxX = -1;
								iLHandBox.maxY = -1;
								iLHandBox.centerX = -1;
								iLHandBox.centerY = -1;
								iLHandBox.numFoundPoint = 0;
								
								input.frameLHandBoxes.push_back( iLHandBox );
							}

							//���̐l���́A�M���ł���(confidence 0.05�ȏ�)�E��̍��i�_�������input�ɓo�^����
							if( numFoundRHPoint > 0 ){
								image::RHandBox iRHandBox;
								iRHandBox.minX = max( /*min_rhpX*/center_rhpX - quartTrunk, 0 );
								iRHandBox.minY = max( /*min_rhpY*/center_rhpY - quartTrunk, 0 );
								iRHandBox.maxX = min( /*max_rhpX*/center_rhpX + quartTrunk, 1919 );
								iRHandBox.maxY = min( /*max_rhpY*/center_rhpY + quartTrunk, 1079 );
								iRHandBox.centerX = center_rhpX;
								iRHandBox.centerY = center_rhpY;
								iRHandBox.numFoundPoint = numFoundRHPoint;

								input.frameRHandBoxes.push_back( iRHandBox );
							}
							//�Ȃ���ΑS�v�f-1��input�ɓo�^
							else{
								image::RHandBox iRHandBox;
								iRHandBox.minX = -1;
								iRHandBox.minY = -1;
								iRHandBox.maxX = -1;
								iRHandBox.maxY = -1;
								iRHandBox.centerX = -1;
								iRHandBox.centerY = -1;
								iRHandBox.numFoundPoint = 0;
								
								input.frameRHandBoxes.push_back( iRHandBox );
							}

							//�t���[����i�Ԗ�(OpenPose�̐l�����m��������ID)�̐l����BOX���W��input.frameHumanBoxes�ɓo�^
							input.frameHumanBoxes.push_back( iHumanBox );
						}

						//HumanKinectID = getKinectHumanID( min_pHumanX, max_pHumanX, min_pHumanY, max_pHumanY, imageCopyBodyIndex );
						//count_sameKinectID = countKinectID_in_HumanBox( TouchedHumanKinectID, min_pHumanX, max_pHumanX, min_pHumanY, max_pHumanY, imageCopyBodyIndex );

						//if( HumanKinectID != 255 && HumanKinectID == TouchedHumanKinectID ){
						//	HumanOpenPoseID = i;
						//	canMatchID = true;
						//	break;
						//}

						//��ԁA�w�肵��ID(TouchedHumanKinectID)�Ɠ���ID��̈���Ɏ����Ă���BOX���̗p
						//if( maxCount == -1 ){
						//	maxCount = count_sameKinectID;
						//	HumanOpenPoseID = i;
						//}
						//else if( count_sameKinectID > maxCount ){
						//	maxCount = count_sameKinectID;
						//	HumanOpenPoseID = i;
						//}
					}
					//��ԑ���KinectID���J�E���g�����ł��낤maxCount��0�Ȃ炻�������S�l��BOX���ɁA11�t���[���ڂŐڐG���s�����l���͉f���Ă��Ȃ�
					//if( maxCount != 0 ){
					//	canMatchID = true;
					//}

					//���݃t���[���̑S�l���ԍ�(OpenPoseID)��KinectID�̃}�b�`���O
					mapHumanBoxToBodyIndex( input, imageCopyBodyIndex, 0 );

					//���O�c���p//////////////////////////////////////////////////////////////////////////////////////////////////
																																//
					ofstream outputPersonIDMap( ( eventID_logs_dirPath + "\\person_id_map" + saveFrame + ".txt" ).c_str());		//
					outputPersonIDMap << "OpenPoseID / KinectID" << endl;														//
					for( int log = 0; log < input.frameHumanBoxKinectIDs.size(); log++ ){										//
						outputPersonIDMap << log << " / " << input.frameHumanBoxKinectIDs[log] << endl;							//
					}																											//
					outputPersonIDMap.close();																					//
																																//
					//////////////////////////////////////////////////////////////////////////////////////////////////////////////

					/*if( p == 17 ){
						cout << "872 frame" << endl;
					}*/
					
					imageCopyColorDraw = imageCopyColor.clone();

					//11�t���[���ڂŕ��̐ڐG���s�����l����KinectID�����ɁA���݃t���[���̓��l���ԍ������߂�
					for( int person = 0; person < input.frameHumanBoxKinectIDs.size(); person++ ){
						if( input.frameHumanBoxKinectIDs[person] == TouchedHumanKinectID ){
							HumanOpenPoseID = person;
							canMatchID = true;
							break;
						}
					}

					for( int v = 0; v < input.frameHumanBoxKinectIDs.size(); v++ ){
						if( input.frameHumanBoxKinectIDs[v] == -1 ){
							continue;
						}
						else{
							string KID = "KinectID: " + to_string( input.frameHumanBoxKinectIDs[v] );
							string OPID = "OpenPoseID: " + to_string( v );
							int person_centerX = input.frameHumanBoxes[v].centerX;
							int person_centerY = input.frameHumanBoxes[v].centerY;

							cv::putText( imageCopyColorDraw, KID.c_str(), cv::Point( person_centerX, person_centerY ), cv::FONT_HERSHEY_SIMPLEX|cv::FONT_ITALIC, 2, cv::Scalar(100,200,100), 4, CV_AA);
							cv::putText( imageCopyColorDraw, OPID.c_str(), cv::Point( person_centerX, person_centerY+50 ), cv::FONT_HERSHEY_SIMPLEX|cv::FONT_ITALIC, 2, cv::Scalar(0,0,255), 4, CV_AA);
						}
					}

					int tPerson_minX, tPerson_minY, tPerson_maxX, tPerson_maxY;
					int tPersonLH_minX, tPersonLH_minY, tPersonLH_maxX, tPersonLH_maxY;
					int tPersonRH_minX, tPersonRH_minY, tPersonRH_maxX, tPersonRH_maxY;
					if( canMatchID ){
					//���̐ڐG�l�����͂�+�e�l��KinectID���t�^�ɂ�����
						tPerson_minX = input.frameHumanBoxes[HumanOpenPoseID].minX;
						tPerson_minY = input.frameHumanBoxes[HumanOpenPoseID].minY;
						tPerson_maxX = input.frameHumanBoxes[HumanOpenPoseID].maxX;
						tPerson_maxY = input.frameHumanBoxes[HumanOpenPoseID].maxY;

						tPersonLH_minX = input.frameLHandBoxes[HumanOpenPoseID].minX;
						tPersonLH_minY = input.frameLHandBoxes[HumanOpenPoseID].minY;
						tPersonLH_maxX = input.frameLHandBoxes[HumanOpenPoseID].maxX;
						tPersonLH_maxY = input.frameLHandBoxes[HumanOpenPoseID].maxY;

						tPersonRH_minX = input.frameRHandBoxes[HumanOpenPoseID].minX;
						tPersonRH_minY = input.frameRHandBoxes[HumanOpenPoseID].minY;
						tPersonRH_maxX = input.frameRHandBoxes[HumanOpenPoseID].maxX;
						tPersonRH_maxY = input.frameRHandBoxes[HumanOpenPoseID].maxY;

						//���̐ڐG�l�����͂�
						cv::rectangle( imageCopyColorDraw, cv::Point( tPerson_minX, tPerson_minY ), cv::Point( tPerson_maxX, tPerson_maxY ), cv::Scalar(0,0,200), 5, 8 );
						//���̐ڐG���Ă������͂�
						if( TouchHand_0L1R == 0 && input.frameLHandBoxes[HumanOpenPoseID].numFoundPoint != 0 ){
							cv::rectangle( imageCopyColorDraw, cv::Point( tPersonLH_minX, tPersonLH_minY ), cv::Point( tPersonLH_maxX, tPersonLH_maxY ), cv::Scalar(0,150,150), 4, 8 );
						}
						if( TouchHand_0L1R == 1 && input.frameRHandBoxes[HumanOpenPoseID].numFoundPoint != 0 ){
							cv::rectangle( imageCopyColorDraw, cv::Point( tPersonRH_minX, tPersonRH_minY ), cv::Point( tPersonRH_maxX, tPersonRH_maxY ), cv::Scalar(0,150,150), 4, 8 );
						}
						//���̂��͂�
						cv::rectangle( imageCopyColorDraw, cv::Point( objPos_minX, objPos_minY ), cv::Point( objPos_maxX, objPos_maxY ), cv::Scalar(200,0,0), 4, 8 );
					}

					string outputIDImage = eventID_logs_dirPath + "\\frameIDinfo" + saveFrame + ".png";
					imwrite( outputIDImage, imageCopyColorDraw );

					//���O�c���p(�l��ID(Kinect, �ǐՂ���))////////////////////////////////////////////////////////////////////////////////
																																		//
					//ofstream outputID( ( eventID_logs_dirPath + "\\touched_personOPID_on_" + saveFrame + ".txt" ).c_str());			//
					if( !canMatchID ){																									//
						outputID << "not found touched person in this frame." << endl;													//
						input.frameHumanBoxes.clear();																					//
						input.frameLHandBoxes.clear();																					//
						input.frameRHandBoxes.clear();																					//
						continue;																										//
					} //11�t���[���ڂŕ��̐ڐG�����l����KinectID�ƃ}�b�`����l�����݂���Ȃ������炻�̃t���[���̓X���[				//
					else{																												//
						outputID << HumanOpenPoseID << "(OpenPoseID)";																	//
						outputID.close();																								//
					}																													//
																																		//
					//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

					//���̐ڐG���Ă���l���̎�(�Е�)�̐؂蔲��
					//Neck����MidHip�܂ł̒������l���̎ʂ��Ă���T�C�Y�ɒP�ˑ�����Ƃ݂Ȃ��A����1/4�̒����l���Ŏ����ӂ�؂蔲��
					int Neck = 1, /*RWrist = 4, LWrist = 7, */MidHip = 8;
					int finger1 = 1; int finger2 = 5; int finger3 = 9; int finger4 = 13; int finger5 = 17;
					std::vector<point2D> pose2Ds = input.get_skeletonType2D( /*touched_personIDs[p],*/HumanOpenPoseID, pose2D );
					std::vector<point2D> Lhand2Ds = input.get_skeletonType2D( /*touched_personIDs[p],*/HumanOpenPoseID, Lhand2D );
					std::vector<point2D> Rhand2Ds = input.get_skeletonType2D( /*touched_personIDs[p],*/HumanOpenPoseID, Rhand2D );
					int roi_handX1, roi_handX2, roi_handY1, roi_handY2;
					bool trackLhand = false, trackRhand = false, whichEverHand = false;
					
					////�l���؂蔲�����W�̌���
					//int min_humanX = -1, min_humanY = -1, max_humanX = -1, max_humanY = -1;
					////�֐ߓ_1�P�̍��W�𑖍����āAmin��max��������
					//for( int point = 0; point < 25; point++ ){
					//  if( pose2Ds[ point+HumanOpenPoseID*25 ].accuracy >= 0.3 ){
					//	  //if( !( pose2Ds[ point+HumanOpenPoseID*25 ].first == 0 && pose2Ds[ point+HumanOpenPoseID*25 ].second == 0 ) ){
					//	  if( min_humanX == -1 ){ min_humanX = pose2Ds[ point+HumanOpenPoseID*25 ].x; }
					//	  if( min_humanY == -1 ){ min_humanY = pose2Ds[ point+HumanOpenPoseID*25 ].y; }
					//	  if( max_humanX == -1 ){ max_humanX = pose2Ds[ point+HumanOpenPoseID*25 ].x; }
					//	  if( max_humanY == -1 ){ max_humanY = pose2Ds[ point+HumanOpenPoseID*25 ].y; }
					//	  if( pose2Ds[ point+HumanOpenPoseID*25 ].x < min_humanX ){ min_humanX = pose2Ds[ point+HumanOpenPoseID*25 ].x; }
					//	  if( pose2Ds[ point+HumanOpenPoseID*25 ].y < min_humanY ){ min_humanY = pose2Ds[ point+HumanOpenPoseID*25 ].y; }
					//	  if( pose2Ds[ point+HumanOpenPoseID*25 ].x > max_humanX ){ max_humanX = pose2Ds[ point+HumanOpenPoseID*25 ].x; }
					//	  if( pose2Ds[ point+HumanOpenPoseID*25 ].y > max_humanY ){ max_humanY = pose2Ds[ point+HumanOpenPoseID*25 ].y; }
					//  }
					//}
					//�l���؂蔲���摜�̍쐬
					//Mat roi_colorHuman( imageCopyColor, cv::Rect( min_humanX, min_humanY, max_humanX-min_humanX, max_humanY-min_humanY ) );
					//Mat roi_depthHuman( imageCopyDepth, cv::Rect( min_humanX, min_humanY, max_humanX-min_humanX, max_humanY-min_humanY ) );
					//Mat roi_depth8UHuman( imageCopyDepth8U, cv::Rect( min_humanX, min_humanY, max_humanX-min_humanX, max_humanY-min_humanY ) );

					//�l���؂蔲���摜�̐���
					Mat roi_colorHuman( imageCopyColor, cv::Rect( tPerson_minX, tPerson_minY, tPerson_maxX-tPerson_minX, tPerson_maxY-tPerson_minY ) );
					Mat roi_depthHuman( imageCopyDepth, cv::Rect( tPerson_minX, tPerson_minY, tPerson_maxX-tPerson_minX, tPerson_maxY-tPerson_minY ) );
					Mat roi_depth8UHuman( imageCopyDepth8U, cv::Rect( tPerson_minX, tPerson_minY, tPerson_maxX-tPerson_minX, tPerson_maxY-tPerson_minY ) );

					//��w�ƑS�g�̊e�֐ߓ_�𕨑̈ʒu����W�n�ɕϊ����ăe�L�X�g�ۑ�
					//XY�ɂ��Ă�pixel���W�n��mm���W�n2��ލ쐬
					//�t���[�����ƂɑS�֐ߓ_��ԍ����ɕ��ׂďo��
					
					std::vector<point2D> person2Ds;
					std::vector<point2D> person_hand2Ds;
					
					//�S�g�̊֐ߓ_�Q�f�[�^�ɍ��E�ǂ��炩�̎�w�̊֐ߓ_�Q�f�[�^��A���������̂����炽�ɍ쐬(��֐߂݂̂��쐬)
					copy( pose2Ds.begin(), pose2Ds.end(), back_inserter( person2Ds ) );
					if( TouchHand_0L1R == 0 ){
						copy( Lhand2Ds.begin(), Lhand2Ds.end(), back_inserter( person2Ds ) );
						copy( Lhand2Ds.begin(), Lhand2Ds.end(), back_inserter( person_hand2Ds ) );
					}
					else if( TouchHand_0L1R == 1 ){
						copy( Rhand2Ds.begin(), Rhand2Ds.end(), back_inserter( person2Ds ) );
						copy( Rhand2Ds.begin(), Rhand2Ds.end(), back_inserter( person_hand2Ds ) );
					}

					//�S�֐ߓ_�QXY��depth����ǉ�����3D�f�[�^���쐬(��֐߂݂̂��쐬)
					std::vector<point3D> person3Ds( person2Ds.size() );
					std::vector<point3D> person_hand3Ds( person_hand2Ds.size() );
					for( int p3d = 0; p3d < person3Ds.size(); p3d++ ){
						if( person2Ds[p3d].accuracy > 0.05 ){ //////////////////////confidense0.11�ȏ�Ȃ�̗p(�ύX��)
							person3Ds[p3d].x = person2Ds[p3d].x;
							person3Ds[p3d].y = person2Ds[p3d].y;
						}else{
							person3Ds[p3d].x = -10000;
							person3Ds[p3d].y = -10000;
						}
						person3Ds[p3d].accuracy = person2Ds[p3d].accuracy;
						if(person3Ds[p3d].x < 0 || person3Ds[p3d].x > 1919 || person3Ds[p3d].y < 0 || person3Ds[p3d].y > 1079){
							person3Ds[p3d].z = 0;
						}else{
							person3Ds[p3d].z = (int)get_actDepth(inputDepth, person3Ds[p3d].y, person3Ds[p3d].x, 10);
						}
						//person3Ds[p3d].z = (int)inputDepth.at<ushort>( person2Ds[p3d].y, person2Ds[p3d].x );
						//if( person3Ds[p3d].z < 500 || person3Ds[p3d].z >= 8000 ){
						//	person3Ds[p3d].z = -10000;
						//}
					}

					//��֐ߓ_�QXY��depth����ǉ�����3D�f�[�^���쐬
					double min_handX_px = NULL;
					double min_handY_px = NULL;
					double max_handX_px = NULL;
					double max_handY_px = NULL;
					double hand_spanX_px, hand_spanY_px, hand_span_px = 0;
					for( int ph3d = 0; ph3d < person_hand3Ds.size(); ph3d++ ){
						if( person_hand2Ds[ph3d].accuracy > 0.05 ){ ////////////////confidense0.11�ȏ�Ȃ�̗p(�ύX��)
							person_hand3Ds[ph3d].x = person_hand2Ds[ph3d].x;
							person_hand3Ds[ph3d].y = person_hand2Ds[ph3d].y;
							if(min_handX_px == NULL && max_handX_px == NULL){min_handX_px = person_hand3Ds[ph3d].x; max_handX_px = person_hand3Ds[ph3d].x;}
							if(min_handY_px == NULL && max_handY_px == NULL){min_handY_px = person_hand3Ds[ph3d].y; max_handY_px = person_hand3Ds[ph3d].y;}
							if(min_handX_px > person_hand3Ds[ph3d].x){min_handX_px = person_hand3Ds[ph3d].x;}
							if(max_handX_px < person_hand3Ds[ph3d].x){max_handX_px = person_hand3Ds[ph3d].x;}
							if(min_handY_px > person_hand3Ds[ph3d].y){min_handY_px = person_hand3Ds[ph3d].y;}
							if(max_handY_px < person_hand3Ds[ph3d].y){max_handY_px = person_hand3Ds[ph3d].y;}
						}else{
							person_hand3Ds[ph3d].x = -10000;
							person_hand3Ds[ph3d].y = -10000;
						}
						person_hand3Ds[ph3d].accuracy = person_hand2Ds[ph3d].accuracy;
						//person_hand3Ds[ph3d].z = (int)get_actDepth(inputDepth, person_hand3Ds[ph3d].y, person_hand3Ds[ph3d].x, 10);
						if(person_hand3Ds[ph3d].x < 0 || person_hand3Ds[ph3d].x > 1919 || person_hand3Ds[ph3d].y < 0 || person_hand3Ds[ph3d].y > 1079){
							person_hand3Ds[ph3d].z = 0;
						}else{
							person_hand3Ds[ph3d].z = (int)inputDepth.at<ushort>( person_hand3Ds[ph3d].y, person_hand3Ds[ph3d].x );
						}
						//if( person_hand3Ds[ph3d].z < 500 || person_hand3Ds[ph3d].z >= 8000 ){
						//	person_hand3Ds[ph3d].z = -10000;
						//}
					}
					if(min_handX_px != max_handX_px && min_handY_px != max_handY_px){
						hand_spanX_px = max_handX_px - min_handX_px;
						hand_spanY_px = max_handY_px - min_handY_px;
						hand_span_px = max(hand_spanX_px, hand_spanY_px);
						cout << "hand span(px): " << hand_span_px << endl;
					}
					double hand_median_px = get_median_px(person_hand3Ds);
					limit_hand_depth_px(person_hand3Ds, hand_median_px, 200, hand_span_px, ObjCenterDepth, inputDepth);

					std::ofstream Points_pxmmToCSV;
					std::ofstream Points_mmToCSV;
					std::ofstream Points_hand_mmToCSV;
					std::ofstream Points_hand_pxmmToCSV;
					Points_pxmmToCSV.open( ( input.saveHandPoints_dir + "\\pose_pxmm_" + dt_ev + input.saveFrame + ".csv" ).c_str(), ios::out );
					Points_mmToCSV.open( ( input.saveHandPoints_dir + "\\pose_mm_" + dt_ev + input.saveFrame + ".csv" ).c_str(), ios::out );
					Points_hand_mmToCSV.open( ( input.saveHandPoints_mm_dir + "\\hand_mm_" + dt_ev + input.saveFrame + ".csv" ).c_str(), ios::out );
					Points_hand_pxmmToCSV.open( ( input.saveHandPoints_pxmm_dir + "\\hand_pxmm_" + dt_ev + "_" + input.saveFrame + ".csv" ).c_str(), ios::out );
					//��������A���̒��S�����_�Ƃ������W�n�ɕϊ�����
					//���̈ʒu���W
					//int ObjCenterX, ObjCenterY, ObjCenterDepth; �擾�ς�

					//2021/02/17�ǉ�
					//���ʒu�����_�Ƃ����񎟌��X�P���g�����A�����ɂ��X�P�[�������ψꉻ�������ďo�͂���
					std::vector<Point2D> scaleUniformedH2Ds_px(person_hand3Ds.size());
					std::pair<double, double> suObj = pxScaleUniform((ObjCenterX - input.getwidth() / 2), (ObjCenterY - input.getheight() / 2), ObjCenterDepth, 3000.);


					cout << "��2D" << endl;
					//���ʒu��depth���擾
					double H0_depth = person_hand3Ds[0].z;
					if(ObjCenterX < 0 || ObjCenterX > 1919 || ObjCenterY < 0 || ObjCenterY > 1079 || ObjCenterDepth == 0 || ObjCenterDepth == -10000){
						//���̈ʒuXY���킩��Ȃ� or ���̈ʒudepth��0 or ���̈ʒudepth��-10000
						for(int err = 0; err < person_hand3Ds.size(); err++){
							//�����l�ƐM���x0�𖄂߂ďI��
							scaleUniformedH2Ds_px[err].x = -10000;
							scaleUniformedH2Ds_px[err].y = -10000;
							scaleUniformedH2Ds_px[err].confidence = 0;
						}
					}else{
						for( int h2d = 0; h2d < person_hand3Ds.size(); h2d++ ){
							if( person_hand3Ds[h2d].accuracy > 0.05){
								//�M���x�l�͂��̂܂ܑ��
								scaleUniformedH2Ds_px[h2d].confidence = person_hand3Ds[h2d].accuracy;
								if(person_hand3Ds[h2d].x != 0 && person_hand3Ds[h2d].x != -10000 && person_hand3Ds[h2d].y != 0 && person_hand3Ds[h2d].y != -10000){
									//�܂��͉摜���S����W�n�ɕϊ�
									double pWfromC = person_hand3Ds[h2d].x - input.getwidth() / 2;
									double pHfromC = person_hand3Ds[h2d].y - input.getheight() / 2;
									//depth3000���C������Ƃ������߂ɂ��s�N�Z���X�P�[���ψꉻ�␳
									std::pair<double, double> suLHp = pxScaleUniform(pWfromC, pHfromC, hand_median_px, 3000.);
									scaleUniformedH2Ds_px[h2d].x = suLHp.first;
									scaleUniformedH2Ds_px[h2d].y = suLHp.second;
								}
								else{
									scaleUniformedH2Ds_px[h2d].x = -10000;
									scaleUniformedH2Ds_px[h2d].y = -10000;
								}
							}else{
								//�M���x�l�͂��̂܂ܑ��
								scaleUniformedH2Ds_px[h2d].confidence = person_hand3Ds[h2d].accuracy;
								scaleUniformedH2Ds_px[h2d].x = -10000;
								scaleUniformedH2Ds_px[h2d].y = -10000;
							}
						}
						for( int h2d = 0; h2d < person_hand3Ds.size(); h2d++ ){
							if(person_hand3Ds[h2d].x != 0 && person_hand3Ds[h2d].x != -10000 && person_hand3Ds[h2d].y != 0 && person_hand3Ds[h2d].y != -10000){
								//���ʒu����W�n�ɕϊ�
								scaleUniformedH2Ds_px[h2d].x -= suObj.first;
								scaleUniformedH2Ds_px[h2d].y -= suObj.second;
							}
						}
					}
					for( int h2d = 0; h2d < person_hand3Ds.size(); h2d++ ){
						Points_hand_pxmmToCSV << h2d << "," << scaleUniformedH2Ds_px[h2d].x << "," << scaleUniformedH2Ds_px[h2d].y << ","
							<< scaleUniformedH2Ds_px[h2d].confidence << endl;
					}
					Points_hand_pxmmToCSV.close();
					
					//XYZ => pix pix mm ���W�n
					std::vector<point3D> ObjCoord3Ds_pxmm( person3Ds.size() );
					for( int p3d = 0; p3d < person3Ds.size(); p3d++ ){
						if( person3Ds[p3d].accuracy > 0.05 ){
							ObjCoord3Ds_pxmm[p3d].x = (int)( person3Ds[p3d].x - ObjCenterX );
							ObjCoord3Ds_pxmm[p3d].y = (int)( person3Ds[p3d].y - ObjCenterY );
							if( person3Ds[p3d].z != -10000 ){
								ObjCoord3Ds_pxmm[p3d].z = (int)( person3Ds[p3d].z - ObjCenterDepth );
							}
							else{
								ObjCoord3Ds_pxmm[p3d].z = -10000;
							}
						}else{
							ObjCoord3Ds_pxmm[p3d].x = -10000;
							ObjCoord3Ds_pxmm[p3d].y = -10000;
							ObjCoord3Ds_pxmm[p3d].z = -10000;
						}
						ObjCoord3Ds_pxmm[p3d].accuracy = person3Ds[p3d].accuracy;
						Points_pxmmToCSV << p3d << "," << ObjCoord3Ds_pxmm[p3d].x << "," << ObjCoord3Ds_pxmm[p3d].y << ","
							<< ObjCoord3Ds_pxmm[p3d].accuracy << "," << ObjCoord3Ds_pxmm[p3d].z << endl;
					}
					Points_pxmmToCSV.close();

					//XYZ => mm mm mm ���W�n(��֐߂݂̂�)
					std::vector<Point3D> Objbased3Ds_mm( person3Ds.size() );
					std::vector<Point3D> Objbasedhand3Ds_mm( person_hand3Ds.size() );
					std::pair<double, double> OBJ_mm;

					//���̈ʒu��mm mm mm ���W�n�ϊ�
					OBJ_mm = mmTransform(ObjCenterX, ObjCenterY, ObjCenterDepth);
					cout << "OBJ(mm) : [" << OBJ_mm.first << ", " << OBJ_mm.second << ", " << ObjCenterDepth << "]" << endl;
					
					//�S�g�e�֐ߓ_�̍��W��mm mm mm ���W�n�ϊ�
					cout << "�S�g" << endl;
					for( int p3d = 0; p3d < person3Ds.size(); p3d++ ){
						if( person3Ds[p3d].accuracy > 0.05 && person3Ds[p3d].z != -10000){
							Objbased3Ds_mm[p3d].z = person3Ds[p3d].z - ObjCenterDepth;
							std::pair<double, double> POSE_mm = mmTransform(person3Ds[p3d].x, person3Ds[p3d].y, person3Ds[p3d].z);
							Objbased3Ds_mm[p3d].x = POSE_mm.first - OBJ_mm.first;
							Objbased3Ds_mm[p3d].y = POSE_mm.second - OBJ_mm.second;
						}else{
							Objbased3Ds_mm[p3d].x = -10000;
							Objbased3Ds_mm[p3d].y = -10000;
							Objbased3Ds_mm[p3d].z = -10000;
						}
						Objbased3Ds_mm[p3d].confidense = person3Ds[p3d].accuracy;
						//���̈ʒu����W�n�ɕϊ�����O�̏����ێ����Ă���(XY��pixel�P��)
						Objbased3Ds_mm[p3d].orig_x = person3Ds[p3d].x;
						Objbased3Ds_mm[p3d].orig_y = person3Ds[p3d].y;
						Objbased3Ds_mm[p3d].orig_z = person3Ds[p3d].z;
					}
					//depth�̕␳
					//fix_pose_depth(Objbased3Ds_mm); //fix_pose_depth()�͖�����
					//���o�X�P���g����CSV�ۑ�
					for( int p3d = 0; p3d < person3Ds.size(); p3d++ ){
						Points_mmToCSV << p3d << "," << Objbased3Ds_mm[p3d].x << "," << Objbased3Ds_mm[p3d].y << ","
							<< Objbased3Ds_mm[p3d].confidense << "," << Objbased3Ds_mm[p3d].z << endl;
					}
					Points_mmToCSV.close();


					//��e�֐ߓ_�̍��W��mm mm mm ���W�n�ϊ�
					cout << "��w" << endl;
					double min_handX = NULL;
					double min_handY = NULL;
					double max_handX = NULL;
					double max_handY = NULL;
					double hand_spanX, hand_spanY, hand_span = 0;

					double min_orig_handX = NULL;
					double min_orig_handY = NULL;
					double max_orig_handX = NULL;
					double max_orig_handY = NULL;
					double orig_hand_spanX, orig_hand_spanY, orig_hand_span = 0;

					for( int ph3d = 0; ph3d < person_hand3Ds.size(); ph3d++ ){
						if( person_hand3Ds[ph3d].accuracy > 0.05 && person_hand3Ds[ph3d].z != -10000){
							Objbasedhand3Ds_mm[ph3d].z = person_hand3Ds[ph3d].z - ObjCenterDepth;
							std::pair<double, double> HAND_mm = mmTransform(person_hand3Ds[ph3d].x, person_hand3Ds[ph3d].y, person_hand3Ds[ph3d].z);
							Objbasedhand3Ds_mm[ph3d].x = HAND_mm.first - OBJ_mm.first;
							Objbasedhand3Ds_mm[ph3d].y = HAND_mm.second - OBJ_mm.second;
							if(min_handX == NULL && max_handX == NULL){min_handX = Objbasedhand3Ds_mm[ph3d].x; max_handX = Objbasedhand3Ds_mm[ph3d].x;}
							if(min_handY == NULL && max_handY == NULL){min_handY = Objbasedhand3Ds_mm[ph3d].y; max_handY = Objbasedhand3Ds_mm[ph3d].y;}
							if(min_handX > Objbasedhand3Ds_mm[ph3d].x){min_handX = Objbasedhand3Ds_mm[ph3d].x;}
							if(max_handX < Objbasedhand3Ds_mm[ph3d].x){max_handX = Objbasedhand3Ds_mm[ph3d].x;}
							if(min_handY > Objbasedhand3Ds_mm[ph3d].y){min_handY = Objbasedhand3Ds_mm[ph3d].y;}
							if(max_handY < Objbasedhand3Ds_mm[ph3d].y){max_handY = Objbasedhand3Ds_mm[ph3d].y;}
						}else{
							Objbasedhand3Ds_mm[ph3d].x = -10000;
							Objbasedhand3Ds_mm[ph3d].y = -10000;
							Objbasedhand3Ds_mm[ph3d].z = -10000;
						}
						Objbasedhand3Ds_mm[ph3d].confidense = person_hand3Ds[ph3d].accuracy;
						//���̈ʒu����W�n�ɕϊ�����O�̏����ێ����Ă���(XY��pixel�P��)
						Objbasedhand3Ds_mm[ph3d].orig_x = person_hand3Ds[ph3d].x;
						Objbasedhand3Ds_mm[ph3d].orig_y = person_hand3Ds[ph3d].y;
						Objbasedhand3Ds_mm[ph3d].orig_z = person_hand3Ds[ph3d].z;
						if(min_orig_handX == NULL && max_orig_handX == NULL && Objbasedhand3Ds_mm[ph3d].orig_x != -10000){min_orig_handX = Objbasedhand3Ds_mm[ph3d].orig_x; max_orig_handX = Objbasedhand3Ds_mm[ph3d].orig_x;}
						if(min_orig_handY == NULL && max_orig_handY == NULL && Objbasedhand3Ds_mm[ph3d].orig_y != -10000){min_orig_handY = Objbasedhand3Ds_mm[ph3d].orig_y; max_orig_handY = Objbasedhand3Ds_mm[ph3d].orig_y;}
						if(min_orig_handX > Objbasedhand3Ds_mm[ph3d].orig_x && Objbasedhand3Ds_mm[ph3d].orig_x != -10000){min_orig_handX = Objbasedhand3Ds_mm[ph3d].orig_x;}
						if(max_orig_handX < Objbasedhand3Ds_mm[ph3d].orig_x && Objbasedhand3Ds_mm[ph3d].orig_x != -10000){max_orig_handX = Objbasedhand3Ds_mm[ph3d].orig_x;}
						if(min_orig_handY > Objbasedhand3Ds_mm[ph3d].orig_y && Objbasedhand3Ds_mm[ph3d].orig_y != -10000){min_orig_handY = Objbasedhand3Ds_mm[ph3d].orig_y;}
						if(max_orig_handY < Objbasedhand3Ds_mm[ph3d].orig_y && Objbasedhand3Ds_mm[ph3d].orig_y != -10000){max_orig_handY = Objbasedhand3Ds_mm[ph3d].orig_y;}
						//cout << min_orig_handX << "," << max_orig_handX << "," << min_orig_handY << "," << max_orig_handY << endl;
					}
					//depth�̕␳
					//��̑S��(x����, y����)���v�Z���đ傫���������s�����̐����͈͂Ƃ��č̗p
					/*if(min_handX != max_handX && min_handY != max_handY){ //��̕����Ȃ��̂͂��������肪�������ĂȂ�
						hand_spanX = max_handX - min_handX;
						hand_spanY = max_handY - min_handY;
						hand_span = max(hand_spanX, hand_spanY);
						cout << "hand span(mm): " << hand_span << endl;
					}*/
					hand_span = 200; //��̉��s�������͈͂��Œ��20cm�ɂ���
					//pixel�P�ʂł��v�Z���Ă���
					if(min_orig_handX != max_orig_handX && min_orig_handY != max_orig_handY){
						orig_hand_spanX = max_orig_handX - min_orig_handX;
						orig_hand_spanY = max_orig_handY - min_orig_handY;
						orig_hand_span = max(orig_hand_spanX, orig_hand_spanY);
						cout << "hand span(px): " << orig_hand_span << endl;
					}
					//���depth�̒����l����̒��S�Ƃ��Ď擾
					double hand_median = get_median(Objbasedhand3Ds_mm);
					cout << "hand median(mm): " << hand_median << endl; 
					limit_hand_depth(Objbasedhand3Ds_mm, hand_median, hand_span, orig_hand_span, ObjCenterDepth, inputDepth);
					fix_hand_depth(Objbasedhand3Ds_mm, hand_median);
					for( int ph3d = 0; ph3d < person_hand3Ds.size(); ph3d++ ){
						Objbasedhand3Ds_mm[ph3d].confidense = person_hand3Ds[ph3d].accuracy;
						Points_hand_mmToCSV << ph3d << "," << Objbasedhand3Ds_mm[ph3d].x << "," << Objbasedhand3Ds_mm[ph3d].y << ","
							<< Objbasedhand3Ds_mm[ph3d].confidense << "," << Objbasedhand3Ds_mm[ph3d].z << endl;
					}
					Points_hand_mmToCSV.close();
					/*for( int p3d = 0; p3d < person3Ds.size(); p3d++ ){
						if( person3Ds[p3d].z != -10000 && person3Ds[p3d].accuracy != 0 ){
							ObjCoord3Ds_mm[p3d].z = (int)( person3Ds[p3d].z - ObjCenterDepth );
							double width_PointToImgCenter = person3Ds[p3d].x - input.Width() / 2.;
							double height_PointToImgCenter = person3Ds[p3d].y - input.Height() / 2.;
							double PointDepthPixel_X = sqrt( pow( CenterDepthPixel_X, 2. ) + pow( width_PointToImgCenter, 2. ) );
							double PointDepthPixel_Y = sqrt( pow( CenterDepthPixel_Y, 2. ) + pow( height_PointToImgCenter, 2. ) );
							double ratioPixToMm_PointPlaneX = person3Ds[p3d].z / PointDepthPixel_X;
							double ratioPixToMm_PointPlaneY = person3Ds[p3d].z / PointDepthPixel_Y;
							double w_PTIC_mm = width_PointToImgCenter * ratioPixToMm_PointPlaneX;
							double h_PTIC_mm = height_PointToImgCenter * ratioPixToMm_PointPlaneY;
							ObjCoord3Ds_mm[p3d].x = (int)( w_PTIC_mm - w_OCTIC_mm );
							ObjCoord3Ds_mm[p3d].y = (int)( h_PTIC_mm - h_OCTIC_mm );
						}else{
							ObjCoord3Ds_mm[p3d].x = -10000;
							ObjCoord3Ds_mm[p3d].y = -10000;
							ObjCoord3Ds_mm[p3d].z = -10000;
						}
						ObjCoord3Ds_mm[p3d].accuracy = person3Ds[p3d].accuracy;
						Points_mmToCSV << p3d << "," << ObjCoord3Ds_mm[p3d].x << "," << ObjCoord3Ds_mm[p3d].y << ","
							<< ObjCoord3Ds_mm[p3d].accuracy << "," << ObjCoord3Ds_mm[p3d].z << endl;
					}
					Points_mmToCSV.close();

					for( int ph3d = 0; ph3d < person_hand3Ds.size(); ph3d++ ){
						if( person_hand3Ds[ph3d].z != -10000 && person_hand3Ds[ph3d].accuracy != 0 ){
							ObjCoord_hand3Ds_mm[ph3d].z = (int)( person_hand3Ds[ph3d].z - ObjCenterDepth );
							double width_Point_handToImgCenter = person_hand3Ds[ph3d].x - input.Width() / 2.;
							double height_Point_handToImgCenter = person_hand3Ds[ph3d].y - input.Height() / 2.;
							double Point_handDepthPixel_X = sqrt( pow( CenterDepthPixel_X, 2. ) + pow( width_Point_handToImgCenter, 2. ) );
							double Point_handDepthPixel_Y = sqrt( pow( CenterDepthPixel_Y, 2. ) + pow( height_Point_handToImgCenter, 2. ) );
							double ratioPixToMm_Point_handPlaneX = person_hand3Ds[ph3d].z / Point_handDepthPixel_X;
							double ratioPixToMm_Point_handPlaneY = person_hand3Ds[ph3d].z / Point_handDepthPixel_Y;
							double w_PhTIC_mm = width_Point_handToImgCenter * ratioPixToMm_Point_handPlaneX;
							double h_PhTIC_mm = height_Point_handToImgCenter * ratioPixToMm_Point_handPlaneY;
							ObjCoord_hand3Ds_mm[ph3d].x = (int)( w_PhTIC_mm - w_OCTIC_mm );
							ObjCoord_hand3Ds_mm[ph3d].y = (int)( h_PhTIC_mm - h_OCTIC_mm );
						}else{
							ObjCoord_hand3Ds_mm[ph3d].x = -10000;
							ObjCoord_hand3Ds_mm[ph3d].y = -10000;
							ObjCoord_hand3Ds_mm[ph3d].z = -10000;
						}
						ObjCoord_hand3Ds_mm[ph3d].accuracy = person_hand3Ds[ph3d].accuracy;
						Points_hand_mmToCSV << ph3d << "," << ObjCoord_hand3Ds_mm[ph3d].x << "," << ObjCoord_hand3Ds_mm[ph3d].y << ","
							<< ObjCoord_hand3Ds_mm[ph3d].accuracy << "," << ObjCoord_hand3Ds_mm[ph3d].z << endl;
					}
					Points_hand_mmToCSV.close();*/
					//if( TouchHand_0L1R == 0 ){						//����(0)�ŐڐG
					//	double Ldivisor = 0.;
					//	double LHpointSumX = 0.;
					//	double LHpointSumY = 0.;
					//	//int f1_lx = Lhand2Ds[finger1+HumanOpenPoseID*21].x;
					//	//int f1_ly = Lhand2Ds[finger1+HumanOpenPoseID*21].y; if( !( f1_lx == 0 && f1_ly == 0 ) ) Ldivisor++;
					//	//int f2_lx = Lhand2Ds[finger2+HumanOpenPoseID*21].x;
					//	//int f2_ly = Lhand2Ds[finger2+HumanOpenPoseID*21].y; if( !( f2_lx == 0 && f2_ly == 0 ) ) Ldivisor++;
					//	//int f3_lx = Lhand2Ds[finger3+HumanOpenPoseID*21].x;
					//	//int f3_ly = Lhand2Ds[finger3+HumanOpenPoseID*21].y; if( !( f3_lx == 0 && f3_ly == 0 ) ) Ldivisor++;
					//	//int f4_lx = Lhand2Ds[finger4+HumanOpenPoseID*21].x;
					//	//int f4_ly = Lhand2Ds[finger4+HumanOpenPoseID*21].y; if( !( f4_lx == 0 && f4_ly == 0 ) ) Ldivisor++;
					//	//int f5_lx = Lhand2Ds[finger5+HumanOpenPoseID*21].x;
					//	//int f5_ly = Lhand2Ds[finger5+HumanOpenPoseID*21].y; if( !( f5_lx == 0 && f5_ly == 0 ) ) Ldivisor++;
					//	for( int LHpoint = 0; LHpoint < 21; LHpoint++ ){
					//		if( Lhand2Ds[ LHpoint+HumanOpenPoseID*21 ].accuracy >= 0.3 ){
					//			LHpointSumX += Lhand2Ds[ LHpoint+HumanOpenPoseID*21 ].x;
					//			LHpointSumY += Lhand2Ds[ LHpoint+HumanOpenPoseID*21 ].y;
					//			Ldivisor += 1.;
					//		}
					//	}
					//	int MidLhandX, MidLhandY;
					//	if( Ldivisor > 0 ){
					//		MidLhandX = (int)( LHpointSumX / Ldivisor );
					//		MidLhandY = (int)( LHpointSumY / Ldivisor );
					//		//MidLhandX = (int)( (double)( f1_lx + f2_lx + f3_lx + f4_lx + f5_lx ) / Ldivisor );
					//		//MidLhandY = (int)( (double)( f1_ly + f2_ly + f3_ly + f4_ly + f5_ly ) / Ldivisor );
					//		trackLhand = true;
					//	}
					//	//int MidLhandX = Lhand2Ds[LWrist].first;												//pose2D�̎��(Wrist)�𒆐S�ɂ���ƃY����̂�hand2D�̊֐߂̏d�S�ɕύX
					//	//int MidLhandY = Lhand2Ds[LWrist].second;
					//	if( trackLhand ){
					//		int NeckX = pose2Ds[Neck+HumanOpenPoseID*25].x;
					//		int NeckY = pose2Ds[Neck+HumanOpenPoseID*25].y;
					//		int MidHipX = pose2Ds[MidHip+HumanOpenPoseID*25].x;
					//		int MidHipY = pose2Ds[MidHip+HumanOpenPoseID*25].y;
					//		double trunk = sqrt( pow( NeckX - MidHipX, 2 ) + pow( NeckY - MidHipY, 2 ) );
					//		roi_handX1 = max( MidLhandX - (int)( trunk / 4. ), 0 ); cout << "roi_handX1 " << roi_handX1 << endl;
					//		roi_handX2 = min( MidLhandX + (int)( trunk / 4. ), 1919 ); cout << "roi_handX2 " << roi_handX2 << endl;
					//		roi_handY1 = max( MidLhandY - (int)( trunk / 4. ), 0 ); cout << "roi_handY1 " << roi_handY1 << endl;
					//		roi_handY2 = min( MidLhandY + (int)( trunk / 4. ), 1079 ); cout << "roi_handY2 " << roi_handY2 << endl;
					//		whichEverHand = true;
					//	}
					//}
					//else{
					//	double Rdivisor = 0.;
					//	double RHpointSumX = 0.;
					//	double RHpointSumY = 0.;
					//	//int f1_rx = Rhand2Ds[finger1+HumanOpenPoseID*21].x;
					//	//int f1_ry = Rhand2Ds[finger1+HumanOpenPoseID*21].y; if( !( f1_rx == 0 && f1_ry == 0 ) ) Rdivisor++;
					//	//int f2_rx = Rhand2Ds[finger2+HumanOpenPoseID*21].x;
					//	//int f2_ry = Rhand2Ds[finger2+HumanOpenPoseID*21].y; if( !( f2_rx == 0 && f2_ry == 0 ) ) Rdivisor++;
					//	//int f3_rx = Rhand2Ds[finger3+HumanOpenPoseID*21].x;
					//	//int f3_ry = Rhand2Ds[finger3+HumanOpenPoseID*21].y; if( !( f3_rx == 0 && f3_ry == 0 ) ) Rdivisor++;
					//	//int f4_rx = Rhand2Ds[finger4+HumanOpenPoseID*21].x;
					//	//int f4_ry = Rhand2Ds[finger4+HumanOpenPoseID*21].y; if( !( f4_rx == 0 && f4_ry == 0 ) ) Rdivisor++;
					//	//int f5_rx = Rhand2Ds[finger5+HumanOpenPoseID*21].x;
					//	//int f5_ry = Rhand2Ds[finger5+HumanOpenPoseID*21].y; if( !( f5_rx == 0 && f5_ry == 0 ) ) Rdivisor++;
					//	for( int RHpoint = 0; RHpoint < 21; RHpoint++ ){
					//		if( Rhand2Ds[ RHpoint+HumanOpenPoseID*21 ].accuracy >= 0.3 ){
					//			RHpointSumX += Rhand2Ds[ RHpoint+HumanOpenPoseID*21 ].x;
					//			RHpointSumY += Rhand2Ds[ RHpoint+HumanOpenPoseID*21 ].y;
					//			Rdivisor += 1.;
					//		}
					//	}
					//	int MidRhandX, MidRhandY;
					//	if( Rdivisor > 0 ){
					//		MidRhandX = (int)( RHpointSumX / Rdivisor );
					//		MidRhandY = (int)( RHpointSumY / Rdivisor );
					//		//MidRhandX = (int)( (double)( f1_rx + f2_rx + f3_rx + f4_rx + f5_rx ) / Rdivisor );
					//		//MidRhandY = (int)( (double)( f1_ry + f2_ry + f3_ry + f4_ry + f5_ry ) / Rdivisor );
					//		trackRhand = true;
					//	}
					//	//int MidRhandX = pose2Ds[RWrist].first;
					//	//int MidRhandY = pose2Ds[RWrist].second;
					//	if( trackRhand ){
					//		int NeckX = pose2Ds[Neck+HumanOpenPoseID*25].x;
					//		int NeckY = pose2Ds[Neck+HumanOpenPoseID*25].y;
					//		int MidHipX = pose2Ds[MidHip+HumanOpenPoseID*25].x;
					//		int MidHipY = pose2Ds[MidHip+HumanOpenPoseID*25].y;
					//		double trunk = sqrt( pow( NeckX - MidHipX, 2 ) + pow( NeckY - MidHipY, 2 ) );
					//		roi_handX1 = max( MidRhandX - (int)( trunk / 4. ), 0 ); cout << "roi_handX1 " << roi_handX1 << endl;
					//		roi_handX2 = min( MidRhandX + (int)( trunk / 4. ), 1919 ); cout << "roi_handX2 " << roi_handX2 << endl;
					//		roi_handY1 = max( MidRhandY - (int)( trunk / 4. ), 0 ); cout << "roi_handY1 " << roi_handY1 << endl;
					//		roi_handY2 = min( MidRhandY + (int)( trunk / 4. ), 1079 ); cout << "roi_handY2 " << roi_handY2 << endl;
					//		whichEverHand = true;
					//	}
					//}

					//��w�؂蔲���摜�̐���
					if( TouchHand_0L1R == 0 ){						//����(0)�ŐڐG
						if( tPersonLH_minX != -1 && tPersonLH_minY != -1 && tPersonLH_maxX != -1 && tPersonLH_maxY != -1 ){
							roi_handX1 = tPersonLH_minX;
							roi_handY1 = tPersonLH_minY;
							roi_handX2 = tPersonLH_maxX;
							roi_handY2 = tPersonLH_maxY;
							whichEverHand = true;
						}
					}
					else if( TouchHand_0L1R == 1 ){					//�E��(1)�ŐڐG
						if( tPersonRH_minX != -1 && tPersonRH_minY != -1 && tPersonRH_maxX != -1 && tPersonRH_maxY != -1 ){
							roi_handX1 = tPersonRH_minX;
							roi_handY1 = tPersonRH_minY;
							roi_handX2 = tPersonRH_maxX;
							roi_handY2 = tPersonRH_maxY;
							whichEverHand = true;
						}
					}
					Mat roi_colorHand, roi_depthHand, roi_depth8UHand;
					if( whichEverHand ){
						Mat colorHand( imageCopyColor, cv::Rect( roi_handX1, roi_handY1, roi_handX2-roi_handX1, roi_handY2-roi_handY1 ) );
						Mat depthHand( imageCopyDepth, cv::Rect( roi_handX1, roi_handY1, roi_handX2-roi_handX1, roi_handY2-roi_handY1 ) );
						Mat depth8UHand( imageCopyDepth8U, cv::Rect( roi_handX1, roi_handY1, roi_handX2-roi_handX1, roi_handY2-roi_handY1 ) );
						roi_colorHand = colorHand;
						roi_depthHand = depthHand;
						roi_depth8UHand = depth8UHand;
					}

					//���̐؂蔲���摜�̐���
					Mat roi_colorObj( imageCopyColor, cv::Rect( objPos_minX, objPos_minY, objPos_maxX-objPos_minX, objPos_maxY-objPos_minY ) );
					Mat roi_depthObj( imageCopyDepth, cv::Rect( objPos_minX, objPos_minY, objPos_maxX-objPos_minX, objPos_maxY-objPos_minY ) );
					Mat roi_depth8UObj( imageCopyDepth8U, cv::Rect( objPos_minX, objPos_minY, objPos_maxX-objPos_minX, objPos_maxY-objPos_minY ) );

					//�y���́���w�z�摜�̍��W����
					int roi_HandObjX1, roi_HandObjY1, roi_HandObjX2, roi_HandObjY2;	//�肪���o�ł��Ă��Ȃ��̂���u���Ă���Ƃ����̒l���Ԃ��Ƃ�(�v�C��)(�C����2019/09/07)
					int roi_HumanObjX1, roi_HumanObjY1, roi_HumanObjX2, roi_HumanObjY2; 
					if( whichEverHand ){
						roi_HandObjX1 = min( objPos_minX, roi_handX1 );
						roi_HandObjX2 = max( objPos_maxX, roi_handX2 );
						roi_HandObjY1 = min( objPos_minY, roi_handY1 );
						roi_HandObjY2 = max( objPos_maxY, roi_handY2 );
					}
					
					//�y���́��l���z�摜�̍��W����
					roi_HumanObjX1 = min( objPos_minX, tPerson_minX );
					roi_HumanObjX2 = max( objPos_maxX, tPerson_maxX );
					roi_HumanObjY1 = min( objPos_minY, tPerson_minY );
					roi_HumanObjY2 = max( objPos_maxY, tPerson_maxY );
					

					//�e��f�[�^�p�t�H���_�̐���
					
					string depth8UDirPath = eventID_depth_dirPath + "\\depth8U";
					string handDirPath = eventID_dirPath + "\\hand";
					string handColorDirPath = handDirPath + "\\color";
					string handDepthDirPath = handDirPath + "\\depth";
					string handDepth8UDirPath = handDirPath + "\\depth8U";
					
					string hmnDirPath = eventID_dirPath + "\\human";
					string hmnColorDirPath = hmnDirPath + "\\color";
					string hmnDepthDirPath = hmnDirPath + "\\depth";
					string hmnDepth8UDirPath = hmnDirPath + "\\depth8U";
					string objDirPath = eventID_dirPath + "\\object";
					string objColorDirPath = objDirPath + "\\color";
					string objDepthDirPath = objDirPath + "\\depth";
					string objDepth8UDirPath = objDirPath + "\\depth8U";
					string touchHandDirPath = eventID_dirPath + "\\touchHand image";
					string touchHandColorDirPath = touchHandDirPath + "\\color";
					string touchHandDepthDirPath = touchHandDirPath + "\\depth";
					string touchHandDepth8UDirPath = touchHandDirPath + "\\depth8U";
					string touchHumanDirPath = eventID_dirPath + "\\touchHuman image";
					string touchHumanColorDirPath = touchHumanDirPath + "\\color";
					string touchHumanDepthDirPath = touchHumanDirPath + "\\depth";
					string touchHumanDepth8UDirPath = touchHumanDirPath + "\\depth8U";

					input.depth8UDirPath =depth8UDirPath;
					input.handDirPath = handDirPath;
					input.handColorDirPath = handColorDirPath;
					input.handDepthDirPath;
					input.handDepth8UDirPath = handDepth8UDirPath;
					
					input.hmnDirPath = hmnDirPath;
					input.hmnColorDirPath = hmnColorDirPath;
					input.hmnDepthDirPath = hmnDepthDirPath;
					input.hmnDepth8UDirPath = hmnDepth8UDirPath;
					input.objDirPath = objDirPath;
					input.objColorDirPath = objColorDirPath;
					input.objDepthDirPath = objDepthDirPath;
					input.objDepth8UDirPath = objDepth8UDirPath;
					input.touchHandDirPath = touchHandDirPath;
					input.touchHandColorDirPath = touchHandColorDirPath;
					input.touchHandDepthDirPath = touchHandDepthDirPath;
					input.touchHandDepth8UDirPath = touchHandDepth8UDirPath;
					input.touchHumanDirPath = touchHumanDirPath;
					input.touchHumanColorDirPath = touchHumanColorDirPath;
					input.touchHumanDepthDirPath = touchHumanDepthDirPath;
					input.touchHumanDepth8UDirPath = touchHumanDepth8UDirPath;

					struct stat statBuf;
					//�ydepth8U(��������8bit�[�x�摜)�z
					if( stat( depth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( depth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << depth8UDirPath << endl;
							return -1;
						}
					}
					//�y��̐؂蔲���t�H���_�z
					if ( stat( handDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( handDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << handDirPath << endl;
							return -1;
						}
					}
					//�y��̐؂蔲���摜(color)�z
					if ( stat( handColorDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( handColorDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << handColorDirPath << endl;
							return -1;
						}
					}
					//�y��̐؂蔲���摜(depth)�z
					if ( stat( handDepthDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( handDepthDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << handDepthDirPath << endl;
							return -1;
						}
					}
					//�y��̐؂蔲���摜(depth8U)�z
					if ( stat( handDepth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( handDepth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << handDepth8UDirPath << endl;
							return -1;
						}
					}
					//�y�l���؂蔲���摜�t�H���_�z
					if( stat( hmnDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( hmnDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << hmnDirPath << endl;
							return -1;
						}
					}
					//�y�l���؂蔲���摜(color)�z
					if ( stat( hmnColorDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( hmnColorDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << hmnColorDirPath << endl;
							return -1;
						}
					}
					//�y�l���؂蔲���摜(depth)�z
					if ( stat( hmnDepthDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( hmnDepthDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << hmnDepthDirPath << endl;
							return -1;
						}
					}
					//�y�l���؂蔲���摜(depth8U)�z
					if ( stat( hmnDepth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( hmnDepth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << hmnDepth8UDirPath << endl;
							return -1;
						}
					}
					//�y���̐؂蔲���摜�t�H���_�z
					if( stat( objDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( objDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << objDirPath << endl;
							return -1;
						}
					}
					//�y���̐؂蔲���摜(color)�z
					if ( stat( objColorDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( objColorDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << objColorDirPath << endl;
							return -1;
						}
					}
					//�y���̐؂蔲���摜(depth)�z
					if ( stat( objDepthDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( objDepthDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << objDepthDirPath << endl;
							return -1;
						}
					}
					//�y���̐؂蔲���摜(depth8U)�z
					if ( stat( objDepth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( objDepth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << objDepth8UDirPath << endl;
							return -1;
						}
					}
					//�y��ƕ��̂̐ڐG�t�H���_�z
					if( stat( touchHandDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHandDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHandDirPath << endl;
							return -1;
						}
					}
					//�y��ƕ��̂̐ڐG�摜(color)�z
					if ( stat( touchHandColorDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHandColorDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHandColorDirPath << endl;
							return -1;
						}
					}
					//�y��ƕ��̂̐ڐG�摜(depth)�z
					if ( stat( touchHandDepthDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHandDepthDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHandDepthDirPath << endl;
							return -1;
						}
					}
					//�y��ƕ��̂̐ڐG�摜(depth8U)�z
					if ( stat( touchHandDepth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHandDepth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHandDepth8UDirPath << endl;
							return -1;
						}
					}
					//�y��Ɛl���̐ڐG�t�H���_�z
					if( stat( touchHumanDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHumanDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHumanDirPath << endl;
							return -1;
						}
					}
					//�y��Ɛl���̐ڐG�摜(color)�z
					if ( stat( touchHumanColorDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHumanColorDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHumanColorDirPath << endl;
							return -1;
						}
					}
					//�y��Ɛl���̐ڐG�摜(depth)�z
					if ( stat( touchHumanDepthDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHumanDepthDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHumanDepthDirPath << endl;
							return -1;
						}
					}
					//�y��Ɛl���̐ڐG�摜(depth8U)�z
					if ( stat( touchHumanDepth8UDirPath.c_str(), &statBuf ) != 0 )
					{
						if( _mkdir( touchHumanDepth8UDirPath.c_str() ) != 0 )
						{
							cout << "cannot create directory: " << touchHumanDepth8UDirPath << endl;
							return -1;
						}
					}

					std::string saveFilePath_depth8U = depth8UDirPath + "\\" + saveFileName;
					std::string saveFilePath_handColor = handColorDirPath + "\\" + saveFrame + "_person" + to_string( (int)touched_person ) + "_color.png";
					std::string saveFilePath_handDepth = handDepthDirPath + "\\" + saveFrame + "_person" + to_string( (int)touched_person ) + "_depth.png";
					std::string saveFilePath_handDepth8U = handDepth8UDirPath + "\\" + saveFrame + "_person" + to_string( (int)touched_person ) + "_depth8U.png";
					
					std::string saveFilePath_humanColor = hmnColorDirPath + "\\" + saveFrame + "_color.png";
					std::string saveFilePath_humanDepth = hmnDepthDirPath + "\\" + saveFrame + "_depth.png";
					std::string saveFilePath_humanDepth8U = hmnDepth8UDirPath + "\\" + saveFrame + "_depth8U.png";
					std::string saveFilePath_objectColor = objColorDirPath + "\\" + saveFrame + "_color.png";
					std::string saveFilePath_objectDepth = objDepthDirPath + "\\" + saveFrame + "_depth.png";
					std::string saveFilePath_objectDepth8U = objDepth8UDirPath + "\\" + saveFrame + "_depth8U.png";
					//imwrite( saveFilePath_R, roi_img_R, param );
					//imwrite( saveFilePath_L, roi_img_L, param );
					if( whichEverHand ){
						imwrite( saveFilePath_handColor, roi_colorHand, param );
						imwrite( saveFilePath_handDepth, roi_depthHand, param );
						imwrite( saveFilePath_handDepth8U, roi_depth8UHand, param );
					}
					imwrite( saveFilePath_depth8U, input.img_original_depth8U, param );
					
					imwrite( saveFilePath_humanColor, roi_colorHuman, param );
					imwrite( saveFilePath_humanDepth, roi_depthHuman, param );
					imwrite( saveFilePath_humanDepth8U, roi_depth8UHuman, param );
					imwrite( saveFilePath_objectColor, roi_colorObj, param );
					imwrite( saveFilePath_objectDepth, roi_depthObj, param );
					imwrite( saveFilePath_objectDepth8U, roi_depth8UObj, param );

					if( whichEverHand ){
						Mat roi_colorTouchHand( imageCopyColor, cv::Rect( roi_HandObjX1, roi_HandObjY1, roi_HandObjX2-roi_HandObjX1, roi_HandObjY2-roi_HandObjY1 ) );
						Mat roi_depthTouchHand( imageCopyDepth, cv::Rect( roi_HandObjX1, roi_HandObjY1, roi_HandObjX2-roi_HandObjX1, roi_HandObjY2-roi_HandObjY1 ) );
						Mat roi_depth8UTouchHand( imageCopyDepth8U, cv::Rect( roi_HandObjX1, roi_HandObjY1, roi_HandObjX2-roi_HandObjX1, roi_HandObjY2-roi_HandObjY1 ) );
						std::string saveFilePath_touchHandColor = touchHandColorDirPath + "\\" + saveFrame + "_touchHand_color.png";
						std::string saveFilePath_touchHandDepth = touchHandDepthDirPath + "\\" + saveFrame + "_touchHand_depth.png";
						std::string saveFilePath_touchHandDepth8U = touchHandDepth8UDirPath + "\\" + saveFrame + "_touchHand_depth8U.png";
						imwrite( saveFilePath_touchHandColor, roi_colorTouchHand, param );
						imwrite( saveFilePath_touchHandDepth, roi_depthTouchHand, param );
						imwrite( saveFilePath_touchHandDepth8U, roi_depth8UTouchHand, param );
					}
					Mat roi_colorTouchHuman( imageCopyColor, cv::Rect( roi_HumanObjX1, roi_HumanObjY1, roi_HumanObjX2-roi_HumanObjX1, roi_HumanObjY2-roi_HumanObjY1 ) );
					Mat roi_depthTouchHuman( imageCopyDepth, cv::Rect( roi_HumanObjX1, roi_HumanObjY1, roi_HumanObjX2-roi_HumanObjX1, roi_HumanObjY2-roi_HumanObjY1 ) );
					Mat roi_depth8UTouchHuman( imageCopyDepth8U, cv::Rect( roi_HumanObjX1, roi_HumanObjY1, roi_HumanObjX2-roi_HumanObjX1, roi_HumanObjY2-roi_HumanObjY1 ) );
					std::string saveFilePath_touchHumanColor = touchHumanColorDirPath + "\\" + saveFrame + "_touchHuman_color.png";
					std::string saveFilePath_touchHumanDepth = touchHumanDepthDirPath + "\\" + saveFrame + "_touchHuman_depth.png";
					std::string saveFilePath_touchHumanDepth8U = touchHumanDepth8UDirPath + "\\" + saveFrame + "_touchHuman_depth8U.png";
					imwrite( saveFilePath_touchHumanColor, roi_colorTouchHuman, param );
					imwrite( saveFilePath_touchHumanDepth, roi_depthTouchHuman, param );
					imwrite( saveFilePath_touchHumanDepth8U, roi_depth8UTouchHuman, param );

					input.frameHumanBoxes.clear();
					input.frameLHandBoxes.clear();
					input.frameRHandBoxes.clear();
				}
				std::string editedTextFileName = date_time_dirPath + "\\" + eventID_dirNames[j] + "\\edited.txt";
				FILE *fp;
				fp = fopen( editedTextFileName.c_str(), "w" );
				fclose( fp );
			}
			////���߂ē��t_�����t�H���_���́u���ׂāv�̃t�H���_�����X�g�A�b�v
			//std::vector<std::string> check_eventID_dirNames;
			//needFile.erase( needFile.begin(), needFile.end() );
			//notNeedFile.erase( notNeedFile.begin(), notNeedFile.end() );
			//check_eventID_dirNames = get_dir_path_in_dir( date_time_dirPath, needFile, notNeedFile );
			//
			////�u���ׂāv�̃C�x���g�t�H���_�Łuedited.txt�v���J������(���݂�����)���t_�����t�H���_�Ɂuchecked.txt�v���쐬
			//bool notCheckedExist = false;
			//for( int k = 0; k < check_eventID_dirNames.size(); k++ )
			//{
			//	std::string editedTextFilePath = date_time_dirPath + "\\" + check_eventID_dirNames[k] + "\\edited.txt";
			//	std::ifstream ifs1( editedTextFilePath );
			//	if( !(ifs1.is_open()) ){
			//		notCheckedExist = true;
			//		break;
			//	}
			//}
			//if( !notCheckedExist ){
			//	std::string checkedTextFileName = base_dirPath + "\\" + date_time_dirNames[i] + "\\checked.txt";
			//	FILE *fp;
			//	fp = fopen( checkedTextFileName.c_str(), "w" );
			//	fclose( fp );
			//}
		}
	}
	//	
	//�ȉ��A���v���O����(PoseEstimator)��
	////�g���~���O�����ׂďI�����Ă�����t�̃t�H���_�͏��O���� ���t�H���_���X�g���֐�(get_dir_path_in_dir())���ŏ���
	//for( int i = 0; i < date_time_dirNames.size(); i++ ){
	//	string check_end_dir = base_dirPath + date_time_dirNames[i] + "\\trimEnd.txt";		//�g���~���O�I���̖ڈ�Ƃ��Ă�trimEnd.txt��u��(�\��)
	//
	//	ifstream ifs( check_end_dir );
	//	if( ifs.is_open() ){
	//		date_time_dirNames.erase( date_time_dirNames.begin() + i );
	//	}
	//}
	//
	//�C�x���gID�t�H���_�̒T��
	//
	//color�t�H���_�p�X�Edepth�t�H���_�p�X�̎w��
	//
	//cout << "program will collect images from: " << dirPath << endl;
	//
	////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	////�t�@�C�����̎����ǂݍ��݁E���X�g��(�t���[���ԍ�������)
	//ifstream ifs2( "FileList.txt" );
	//string fileName;
	//vector<string> fileNameList;
	//
	//if( ifs2.fail() )
	//{
	//	cerr << "read file list error !" << endl;
	//	return -1;
	//}
	//while( getline( ifs2, fileName ) )
	//{
	//	fileNameList.push_back( fileName );
	//}*/
	//
	////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	////�t�@�C�������X�g�̓ǂݍ���(�t���[���ԍ�������)
	//ifstream ifs2( "FileList.txt" );
	//string fileName;
	//vector<string> fileNameList;
	//
	//if( ifs2.fail() )
	//{
	//	cerr << "read file list error !" << endl;
	//	return -1;
	//}
	//while( getline( ifs2, fileName ) )
	//{
	//	fileNameList.push_back( fileName );
	//}*/
	//
	////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	////���X�g�̊e�t�@�C�����Ƃ̏���
	//for( auto listItr = fileNameList.begin(); listItr != fileNameList.end(); listItr++ )
	//{
	//	//�t�@�C����(.png �� .txt)�̐���
	//	string frameNum = (*listItr).c_str();
	//
	//	string imageFilePath = dirPath + "/" + frameNum + ".png";
	//	string skeletonFilePath = dirPath + "/" + frameNum + ".txt";
	//
	//	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	//	//PNG�C���[�W�̓ǂݍ���
	//	cout << "open PNG image file: " << imageFilePath << endl;
	//	Mat inputImage = imread( imageFilePath, -1 );
	//	if( inputImage.data == NULL )
	//	{
	//		cout << "read PNG image error !" << endl;
	//		return -1;
	//	}else{ cout << "success reading PNG image."<< endl; }
	//
	//	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	//	//Skeleton �e�L�X�g�̓ǂݍ���
	//	cout << "open skeleton text file: " << skeletonFilePath << endl;
	//	ifstream ifs4( skeletonFilePath.c_str() );
	//	if ( ifs4.fail() )
	//	{
	//		cerr << "read skeleton text error !" << endl;
	//		return -1;
	//	}
	//	istreambuf_iterator<char> it( ifs4 );
	//	istreambuf_iterator<char> last;
	//	string BodyInfo(it, last);
	//	
	//
	//	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	//	//���e�\��
	//	//�e�L�X�g
	//	//cout << "->\n[" << BodyInfo << "]" << endl;
	//	//�C���[�W
	//	//imshow( frameNum.c_str(), inputImage );
	//	//waitKey(1);
	//
	//	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	//	//��𑜓x�Ƀ��T�C�Y�R�s�[(���T�C�Y���[�h)
	//	if( mode == 1 ){
	//		Mat imageResize;
	//		resize( inputImage, imageResize, Size( 480, 270 ) );
	//		imshow( "resize(480x270)", imageResize );
	//		waitKey(1);
	//		string resizeImgDirPath = dirPath + "/resize_image";
	//		struct stat statBuf;
	//		if ( stat( resizeImgDirPath.c_str(), &statBuf ) != 0 )
	//		{
	//			if( _mkdir( resizeImgDirPath.c_str() ) != 0 )
	//			{
	//				cout << "cannot create directory: " << resizeImgDirPath << endl;
	//				return -1;
	//			}
	//		}
	//		vector<int> param = vector<int>(2);			// PNG�ۑ��̈��k��(0�`9)
	//		param[0] = CV_IMWRITE_PNG_COMPRESSION;		// ���Ԃƃt�@�C���T�C�Y�̃g���[�h�I�t
	//		param[1] = 9;								// ���ɍ��͎��Ԃ��C�ɂ��Ȃ��̂Ń��x��9
	//
	//		string resizeFilePath = resizeImgDirPath + "/" + frameNum + ".png";
	//		imwrite( resizeFilePath, imageResize );
	//	}
	//
	//	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	//	//Skeleton �e�L�X�g�̉��
	//	image input( inputImage.cols, inputImage.rows );
	//	input.img_original = inputImage.clone();
	//	ImageEditUtil::drawSkeleton( input, BodyInfo );
	//	resize( input.img_coverSkeleton, input.img_resize, Size( 960, 540 ) );
	//	//imshow( "skeleton on image", input.img_coverSkeleton );
	//	//waitKey();
	//
	//	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//
	//	//�ʂ��Ă���l���������ӂ�؂蔲���ĕۑ�
	//	cout << input.skeletons.size() << "person detected" << endl;
	//
	//	for( int i = 0; i < input.skeltons.size(); i++ )
	//	{
	//		//���̂������̎�𐄒�@��������A���̈ʒu���L�^���Ă����͍폜����
	//		//SpineMid(1) �ɑ΂��āA ElbowLeft(5) �� ElbowRight(9) �̂ǂ��炪��艓����(�����ق������̂�͂�ł���Ɖ���)
	//		int SpineMid = 1, ElbowLeft = 5, ElbowRight = 9, ThumbLeft = 22, ThumbRight = 24;	// Kinect��JointType�Ɠ��l
	//
	//		int midX = input.skeltons[i][SpineMid].first;
	//		int midY = input.skeltons[i][SpineMid].second;
	//		int elLeftX = input.skeltons[i][ElbowLeft].first;
	//		int elLeftY = input.skeltons[i][ElbowLeft].second;
	//		int elRightX = input.skeltons[i][ElbowRight].first;
	//		int elRightY = input.skeltons[i][ElbowRight].second;
	//		int thLeftX = input.skeltons[i][ThumbLeft].first;
	//		int thLeftY = input.skeltons[i][ThumbLeft].second;
	//		int thRightX = input.skeltons[i][ThumbRight].first;
	//		int thRightY = input.skeltons[i][ThumbRight].second;
	//		double diffLeft = sqrt( pow( thLeftX - midX, 2. ) + pow( thLeftY - midY, 2. ) );
	//		double diffRight = sqrt( pow( thRightX - midX, 2. ) + pow( thRightY - midY, 2. ) );
	//		int thumbX, thumbY, elbowX, elbowY;
	//		if( diffLeft - diffRight >= 250. )
	//		{
	//			thumbX = thLeftX;
	//			thumbY = thLeftY;
	//			elbowX = elLeftX;
	//			elbowY = elLeftY;
	//		}
	//		else if( diffLeft - diffRight <= -250. )
	//		{
	//			thumbX = thRightX;
	//			thumbY = thRightY;
	//			elbowX = elRightX;
	//			elbowY = elRightY;
	//		}
	//		else
	//		{
	//			cout << "I don't know which hand is active ! ( human:" << i+1 << ")" << endl;
	//			cout << "Please Select one [ Left: 0 | Right: 1 ]" << endl;
	//
	//			imshow( "skelton on image", input.img_resize );
	//			waitKey(1);
	//			int whichHand;
	//			while(1)
	//			{
	//				cin >> whichHand;
	//				if( whichHand == 0 || whichHand == 1 )
	//				{
	//					break;
	//				}
	//				else
	//				{
	//					cout << "Error ! input again [ Left: 0 | Right: 1 ]" << endl;
	//				}
	//			}
	//
	//			if( whichHand == 0 )
	//			{
	//				thumbX = thLeftX;
	//				thumbY = thLeftY;
	//				elbowX = elLeftX;
	//				elbowY = elLeftY;
	//			}
	//			else if( whichHand == 1 )
	//			{
	//				thumbX = thRightX;
	//				thumbY = thRightY;
	//				elbowX = elRightX;
	//				elbowY = elRightY;
	//			}
	//		}
	//		//�̂̒��S����r�������ق��𕨑̑��쒆�̎�Ƃ݂Ȃ��āA�e�w�ʒu( ThumbLeft(22) or ThumbRight(24) )�𒆐S�ɁA
	//		//��������I�ʒu( ElbowLeft(5) or ElbowRight(9) )�܂ł̋����𔼌a�Ƃ���̈�Ő؂蔲���ۑ�
	//		double radius;
	//		int roiX, roiY, roiWidth, roiHeight;
	//
	//		radius = sqrt( pow( elbowX - thumbX, 2. ) + pow( elbowY - thumbY, 2. ) );
	//
	//
	//		roiX = max( thumbX - (int)radius, 0 );
	//		roiY = max( thumbY - (int)radius, 0 );
	//		roiWidth = min( (int)radius * 2, 1080 );
	//		roiHeight = min( (int)radius * 2, 1080 );
	//		if( roiX + roiWidth >= 1920 ){ roiWidth = 1920 - roiX; }
	//		if( roiY + roiHeight >= 1080 ){ roiHeight = 1080 - roiY; }
	//		roiWidth = min( roiWidth, roiHeight );
	//		roiHeight = roiWidth;
	//		cout << "x: " << roiX << "\ny: " << roiY << "\nwidth: " << roiWidth << "\nheight: " << roiHeight << endl;
	//
	//		Mat imageCopy = input.img_original.clone();
	//		Mat roi_img( imageCopy, cv::Rect( roiX, roiY, roiWidth, roiHeight ) );
	//
	//		string cutImgDirPath = dirPath + "/cut_image";
	//		struct stat statBuf;
	//		if ( stat( cutImgDirPath.c_str(), &statBuf ) != 0 )
	//		{
	//			if( _mkdir( cutImgDirPath.c_str() ) != 0 )
	//			{
	//				cout << "cannot create directory: " << cutImgDirPath << endl;
	//				return -1;
	//			}
	//		}
	//		vector<int> param = vector<int>(2);			// PNG�ۑ��̈��k��(0�`9)
	//		param[0] = CV_IMWRITE_PNG_COMPRESSION;		// ���Ԃƃt�@�C���T�C�Y�̃g���[�h�I�t
	//		param[1] = 9;								// ���ɍ��͎��Ԃ��C�ɂ��Ȃ��̂Ń��x��9
	//
	//		string saveFilePath = cutImgDirPath + "/" + frameNum + "_" + to_string(i) + ".png";
	//		imwrite( saveFilePath, roi_img );
	//	}
	//}

	return 0;
}
