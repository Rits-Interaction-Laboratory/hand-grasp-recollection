#include "geometory.h"
#define REP(i,a,n) for(int i = a; i < n ; i++)
#define rep(i,n) REP(i,0,n)
const double EPS = 1e-8;
const double INF = 1e12;
const double NAN = DBL_MIN;



namespace Geometory{
	namespace std{
		bool operator < (const Point& a, const Point& b) {
			return real(a) != real(b) ? real(a) < real(b) : imag(a) < imag(b);
		}
	}
	double cross(const Point& a, const Point& b) { return imag(conj(a)*b); }
	double dot(const Point& a, const Point& b)   { return real(conj(a)*b); }

	int ccw(Point a, Point b, Point c) {
		b -= a; c -= a;
		if (cross(b, c) > 0)          return +1;  // counter clockwise
		if (cross(b, c) < 0)   return -1;  // clockwise
		if (dot(b, c) < 0)     return +2;  // c--a--b on line
		if (norm(b) < norm(c)) return -2;  // a--b--c on line
		return 0;
	}

	Point rotate(Point a, double rad){ // 点aを, 原点を中心に反時計まわりにrad回転する radはラジアン角
		double nx = a.real() * cos(rad) - a.imag() * sin(rad);
		double ny = a.real() * sin(rad) + a.imag() * cos(rad);
		return Point(nx, ny);
	}

	double getAngle(Point a, Point b){ // ベクトルaとbのなす角をラジアン角で返す
		return acos(dot(a, b) / abs(a) / abs(b));
	}

	vector<int> fromPoint(Point a , Point b){ // 座標から ax+by+c の直線を生成し、a,b,cを返す
		Point c = b - a;
		vector<int> l;
		l.push_back( (int)c.imag());
		l.push_back( (int)-c.real());
		l.push_back( (int)(c.real() * a.imag() - c.imag() * a.real()));
		return l;
	}

	// 2点を与えた時に ax + by + c = 0 の形式に変換
	void fromPoint(double x1, double y1, double x2, double y2) {
		double dx = x2 - x1;
		double dy = y2 - y1;
		double a = dy, b =  -dx ,c = dx * y1 - dy * x1;
	}


	// 凸多角形の最遠点対（キャリパー法）　最悪 O(n log n)
	Point projection(const L &l, const Point &p) {
		double t = dot(p-l[0], l[0]-l[1]) / norm(l[0]-l[1]);
		return l[0] + t*(l[0]-l[1]);
	}

	double distanceLP(const L &l, const Point &p) {
		return abs(p - projection(l, p));
	}
	double distanceLL(const L &l, const L &m) {
		return intersectLL(l, m) ? 0 : distanceLP(l, m[0]);
	}
	double distanceLS(const L &l, const L &s) {
		if (intersectLS(l, s)) return 0;
		return min(distanceLP(l, s[0]), distanceLP(l, s[1]));
	}
	double distanceSP(const L &s, const Point &p) {
		const Point r = projection(s, p);
		if (intersectSP(s, r)) return abs(r - p);
		return min(abs(s[0] - p), abs(s[1] - p));
	}
	double distanceSS(const L &s, const L &t) {
		if (intersectSS(s, t)) return 0;
		return min(min(distanceSP(s, t[0]), distanceSP(s, t[1])),
			min(distanceSP(t, s[0]), distanceSP(t, s[1])));
	}
	Point crosspoint(const L &l, const L &m) {
		double A = cross(l[1] - l[0], m[1] - m[0]);
		double B = cross(l[1] - l[0], l[1] - m[0]);
		if (abs(A) < EPS && abs(B) < EPS) return m[0]; // same line
		if (abs(A) < EPS) assert(false); // !!!PRECONDITION NOT SATISFIED!!!
		return m[0] + B / A * (m[1] - m[0]);
	}
	bool intersectLL(const L &l, const L &m) {
		return abs(cross(l[1]-l[0], m[1]-m[0])) > EPS || // non-parallel
			abs(cross(l[1]-l[0], m[0]-l[0])) < EPS;   // same line
	}
	bool intersectLS(const L &l, const L &s) {
		return cross(l[1]-l[0], s[0]-l[0])*       // s[0] is left of l
			cross(l[1]-l[0], s[1]-l[0]) < EPS; // s[1] is right of l
	}
	bool intersectLP(const L &l, const Point &p) {
		return abs(cross(l[1]-p, l[0]-p)) < EPS;
	}
	bool intersectSS(const L &s, const L &t) {
		return ccw(s[0],s[1],t[0])*ccw(s[0],s[1],t[1]) <= 0 &&
			ccw(t[0],t[1],s[0])*ccw(t[0],t[1],s[1]) <= 0;
	}
	bool intersectSP(const L &s, const Point &p) {
		return abs(s[0]-p)+abs(s[1]-p)-abs(s[1]-s[0]) < EPS; // triangle inequality
	}
}

