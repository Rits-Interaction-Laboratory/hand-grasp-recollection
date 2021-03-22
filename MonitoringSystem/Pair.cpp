#include "stdafx.h"
#include "Pair.h"
//common
#include "common.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

Pair::Pair(){
  x = 0;
  y = 0;
  vx = 0;
  vy = 0;
  frame=0;
  id=0;
}

Pair::Pair(int _x, int _y, int _vx, int _vy){
	x = _x;
	y = _y;
	vx = _vx;
	vy = _vy;
	frame=0;
	id=0;
}

Pair::~Pair()
{
}

int Pair::get_x(){
	return x;
}

int Pair::get_y(){
	return y;
}

int  Pair::get_vx(){
	return vx;
}

int Pair::get_vy(){
	return vy;
}

int Pair::get_frame(){
	return frame;
}

int Pair::get_id()
{
	return id;
}

void Pair::set_x(int _x){
  x = _x;
}

void Pair::set_y(int _y){
	y = _y;
}

void Pair::set_vx(int _vx){
	vx = _vx;
}

void Pair::set_vy(int _vy){
	vy = _vy;
}

void Pair::set_frame(int _frame){
	frame = _frame;
}

void Pair::set_id(int _id)
{
   id=_id;
}