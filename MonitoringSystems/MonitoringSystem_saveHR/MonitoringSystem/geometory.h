#pragma once
#ifndef __GOMETORY_H__
#define __GEOMETORY_H__
#define _USE_MATH_DEFINES
#include <complex>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <cfloat>
#include <climits>
using namespace std;

#define PI M_PI
typedef complex<double> Point;
typedef struct L : public vector<Point> {
	L(const Point &a, const Point &b) {
		push_back(a); push_back(b);
	}
}L;

namespace Geometory{
	class Point2D{
	private:
		int x,y;
	public:
		Point2D(int x,int y):x(x),y(y){
		}
		Point2D(){
		}
		~Point2D(){
		}
		int getX(){return x;}
		int getY(){return y;}
		Point2D operator + (const Point2D &p)const{
			return Point2D(x + p.x, y + p.y);
		}
		Point2D operator - (const Point2D &p)const{
			return Point2D(x - p.x, y - p.y);
		}
		Point2D operator * (const Point2D &p)const{
			return Point2D(x * p.x, y * p.y);
		}
		Point2D operator / (const Point2D &p)const{
			return Point2D(x / p.x, y / p.y);
		}
		bool operator == (const Point2D &p)const{
			return (x == p.x && y ==p.y);
		}
	};
	class Area{
	private:
		Point2D posTopLeft;
		Point2D posBottomRight;
		int width;
		int height;
		int area;
	public:
		Area(){
		}
		Area(const Area &o){
			posTopLeft = o.posTopLeft;
			posBottomRight = o.posBottomRight;
			width = o.width;
			height = o.height;
			area = o.area;
		}
		Area(Point2D posTopLeft, Point2D posBottomRight):posTopLeft(posTopLeft), posBottomRight(posBottomRight){
			width = posBottomRight.getX() - posTopLeft.getX();
			height = posBottomRight.getY() - posTopLeft.getY();
			area = width * height;
		}
		Area(int x, int y, int width, int height):width(width), height(height){
			posTopLeft = Point2D(x, y);
			posBottomRight = Point2D(x + width, y + height);
			area = width * height;
		}
		Area(Point2D posTopLeft, int width, int height):posTopLeft(posTopLeft), width(width), height(height){
			posBottomRight = Point2D(posTopLeft.getX() + width, posTopLeft.getY() + height);
			area = width * height;
		}
		Geometory::Point2D getPosTopLeft(){
			return posTopLeft;
		}
		Geometory::Point2D getPosBottomRight(){
			return posBottomRight;
		}
		int getWidth(){
			return width;
		}
		int getHeight(){
			return height;
		}
		int getArea(){
			return area;
		}
	};

	double cross(const Point& a, const Point& b);
	double dot(const Point& a, const Point& b);
	int ccw(Point a, Point b, Point c);

	Point rotate(Point a, double rad); // 点aを, 原点を中心に反時計まわりにrad回転する radはラジアン角
	double getAngle(Point a, Point b); // ベクトルaとbのなす角をラジアン角で返す
	vector<int> fromPoint(Point a , Point b); // 座標から ax+by+c の直線を生成し、a,b,cを返す
	// 2点を与えた時に ax + by + c = 0 の形式に変換
	void fromPoint(double x1, double y1, double x2, double y2) ;	// 円と円の交点
	pair<Point, Point> pointCC(Point a , double r1 , Point b , double r2);// 円と線分の交点
	Point projection(const L &l, const Point &p) ;

	double distanceLP(const L &l, const Point &p) ;
	double distanceLL(const L &l, const L &m) ;
	double distanceLS(const L &l, const L &s) ;
	double distanceSP(const L &s, const Point &p) ;
	double distanceSS(const L &s, const L &t) ;
	Point crosspoint(const L &l, const L &m) ;
	bool intersectLL(const L &l, const L &m) ;
	bool intersectLS(const L &l, const L &s) ;
	bool intersectLP(const L &l, const Point &p) ;
	bool intersectSS(const L &s, const L &t) ;
	bool intersectSP(const L &s, const Point &p) ;
}
#endif
