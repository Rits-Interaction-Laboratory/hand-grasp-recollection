#pragma once
#include <vector>

//ラベルの情報を格納する構造体
typedef struct tagLABELDATATABLE {
	int x1, x2, y1, y2;    //上下左右の領域
	int xcenter, ycenter;  //重心
	double xsum, ysum;     //（重心計算用）
	long int size;    //ラベル面積
} LABELDATATABLE;

const int LTMAX=10000;//ラベルの個数の最大値

//------------------------
//label class
//------------------------
class label
{
	int Width, Height;
	int *Pixels;
	int Num;
	LABELDATATABLE *LTable;

public://-------------------------------------------------

	// constructor
	label()
		: Width(0), Height(0), Pixels(0), Num(0)
	{
		LTable = new LABELDATATABLE [LTMAX];

		for (int i = 0; i < LTMAX; i++) {
			LTable[i].size = 0;
			LTable[i].xsum = 0;
			LTable[i].ysum = 0;
		}
	}

	// destructor
	virtual ~label()
	{
		delete [] Pixels;
		delete [] LTable;
	}


	void init(void)
	{
		Num = 0;
		for (int i = 0; i < Width*Height; i++)
			Pixels[i] = 0;
		for (int i = 0; i < LTMAX; i++) {
			LTable[i].size = 0;
			LTable[i].xsum = 0;
			LTable[i].ysum = 0;
		}
	}

	void setArea(const int W, const int H){
		Width  = W;
		Height = H;
		Pixels = new int[Width*Height];
		init();
	}

	void setPoint(const int x, const int y) {place(x,y) = -1;}
	int &getPoint(const int x, const int y) {return place(x,y);}
	int getMaxNum(void) {return Num;}
	//int getLargestLabel(void);
	//nt getLargestObjLabel(std::vector<int> &obj_label);
	int getLabelSize(const int LabelName);
	void getLabelArea(const int LabelName, int &x1, int &x2, int &y1, int &y2);
	void getGravityCenter(const int LabelName, int &x, int &y);
	void Labeling(void);
	//std::vector<int> getminLabel(void);
	//std::vector<int> getlargeLabel(void);
	std::vector<int> getobjLabel(void);

	void setLabelSize(const int LabelName,int size){LTable[LabelName].size = size;}
	void setLabelGravityCenter(const int LabelName,int x,int y){LTable[LabelName].xcenter=x; LTable[LabelName].ycenter=y;}

private://============================================================================
	int &place(const int x, const int y) {return Pixels[y*Width+x];}
	void RunAnalysis(const int LabelName, const int x, const int y);
	int setLabel(const int LabelName, const int x, const int y);
	int seekLeft(const int x, const int y);
	int seekRight(const int x, const int y);
	void setLabelTable(void);
	//=================================================================================
};