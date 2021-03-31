#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
class layer
{
	//===================================
private:
	int imageWidth, imageHeight;
	std::vector<int> *pixels;
	std::vector<int> *m_pixels;
	std::vector<int> l_array;
	int count[320*240];

	std::vector<float>*depth;//�����f�[�^
	//===================================
	//-----------------------------------
public:
	layer(void)
	{
		imageWidth  = 320;//��
		imageHeight = 240;//����
		init();//������
	}

	~layer(void)
	{
		release();
	}
	
	//������
	void init()
	{
		pixels = new std::vector<int>[imageWidth*imageHeight];
		m_pixels = new std::vector<int>[imageWidth*imageHeight];

		depth = new std::vector<float>[imageWidth*imageHeight];

		for(int j=0; j < imageHeight; j++){
			for(int i=0; i < imageWidth; i++){
				pixels[i+(imageWidth*j)].reserve(50);
				m_pixels[i+(imageWidth*j)].reserve(50);
				depth[i+(imageWidth*j)].reserve(50);

				count[i+(imageWidth+j)]=0;
				add(i,j,-1);
				add_mask(i,j,-1);
				
				add_depth(i,j,-1);
			}
		}
        
	}

	layer(const int width,const int height)
	{
		imageWidth = width;//��
		imageHeight = height;//����
		init();//������
	}

	void set(const int w, const int h)
	{
		imageWidth = w;
		imageHeight = h; 
	}

	//���
	void release()
	{
		if(pixels) delete [] pixels;
		if(m_pixels) delete [] m_pixels;
		if(depth)delete []depth;
	}
	
	size_t size() const
	{
		return imageWidth * imageHeight;
	}

	//���̂̃e�N�X�`������1��f���i�[
	void add(const int x,const int y,const int num)
	{
		//kawa nun!=1�̎��̏����́H�Ȃ���
		pixels[y*imageWidth+x].push_back(num);
	}

	//���̂̃}�X�N�����P��f���i�[
	void add_mask(const int x,const int y,const int num)
	{
		if(num!=-1)	count[y*imageWidth+x]++;
		m_pixels[y*imageWidth+x].push_back(num);
	}

	//�����f�[�^�̏���1��f���i�[
	void add_depth(const int x,const int y,float num)
	{
		depth[y*imageWidth+x].push_back(num);
	}

	int get_nval(int x,int y)
	{
		return count[y*imageWidth+x];
	}

	int begin_val(const int x,const int y)
	{
		/*std::vector<int>::iterator it = pixels[y*imageWidth+x].begin();
		return *it;*/
		return pixels[y*imageWidth+x][0];//ikegami
	}

	int end_val(const int x,const int y)
	{
		std::vector<int>::iterator it  = pixels[y*imageWidth+x].end();
		std::vector<int>::iterator it2 = pixels[y*imageWidth+x].begin();
		it--;
		while(it != it2){
			if(*it != -1) return *it;
			it--;
		}

		return *it;
	}

	int get_val(const int x,const int y,const int lay)
	{
		int size = (int)pixels[y*imageWidth+x].size();
		size--;
		//�ۑ����Ă���z��𒴂�����
		if(size < lay){
			return 255;
		}
		int val = pixels[y*imageWidth+x][lay];
		return val;
	}

	float get_depth(const int x,const int y,const int lay)
	{
		int size = (int)depth[y*imageWidth+x].size();
		size--;
		//�ۑ����Ă���z��𒴂�����
		if(size < lay){
			return -1;
		}
		float val = depth[y*imageWidth+x][lay];
		return val;
	}

	size_t layer_size(const int x,const int y) const
	{
		return pixels[y*imageWidth+x].size();
	}
	

	std::vector<int> &val(const int x,const int y) const
	{
		return pixels[y*imageWidth+x];
	}

	std::vector<int> &m_val(const int x,const int y) const
	{
		return m_pixels[y*imageWidth+x];
	}

	std::vector<float> &depth_val(const int x,const int y) const
	{
		return depth[y*imageWidth+x];
	}
	//-------------------------------------------
	//  ����������̓��C���̃}�X�N���̑���
	//
	// ���w�ɂǂ̃I�u�W�F�N�gID�������Ă��邩�H


	void set_layer(int key)
	{
		l_array.push_back(key);
	}

	int get_end()
	{
		if(l_array.size()<=0){
			return -1;
		}
		std::vector<int>::iterator it = l_array.end();
		it--;
		return *it;
	}

	std::vector<int> get_layer_all()
	{
		return l_array;
	}

	//�w�肵���l�������Ă��郌�C���̍폜
	void erase_layer(int key)
	{
		std::vector<int>::iterator it = l_array.begin();
		while( it != l_array.end()){
			if(*it == key){
				//std::cout << *it << "���폜" << std::endl;
				//l_array.erase(it);//kawa
				std::vector<int>::iterator end_it=std::remove(l_array.begin(),l_array.end(),*it);
				l_array.erase(end_it,l_array.end());
				return;
			}
			++it;
		}
	}

	//�w�肵���l�������C���ԍ�
	int get_layer(int key)
	{
		if(key==-1) return -1;
		int num = 0;
		std::vector<int>::iterator it = l_array.begin();
		
		while( it != l_array.end()){
			++num;
			if(*it == key){
				return num;
			}
			++it;
		}
		//���C���������ǉ����ꂸ�����Ƃ��� 0
		
		num = -1;
		return num;
	}

	int returnl_array(int num)//kawa
	{
	return l_array[num];
	}


	//------------------------------------------------------
};
