#pragma once

using namespace std;

class Pair
{
 private:
  int x,y,vx,vy,frame,id;
  
 public:
	 Pair();
	 Pair(int _x, int _y, int _vx, int _vy);

	 virtual ~Pair();
  
  int get_x();
  int get_y();
  int get_vx();
  int get_vy();
  int get_frame();
  int get_id();

  void set_x(int _x);
  void set_y(int _y);
  void set_vx(int _vx);
  void set_vy(int _vy);
  void set_frame(int _frame);
  void set_id(int _id);
};
