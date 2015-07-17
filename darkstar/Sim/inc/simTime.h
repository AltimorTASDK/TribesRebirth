//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _STIME_H_
#define _STIME_H_

#include <m_point.h>

struct SimTime
{
	float time;

	SimTime() {}
	SimTime(float x) { time = x; }
	SimTime(const SimTime& x) { time = x.time; }
	operator float() const { return time; }
	//operator long() const { return time; }

	SimTime operator-(const SimTime& x) const { return SimTime(time - x.time); }
	SimTime operator+(const SimTime& x) const { return SimTime(time + x.time); }
	SimTime& operator=(const SimTime& x) { time = x.time; return *this; }
	//SimTime& operator=(const int x) { time = x; return *this; }

	SimTime& operator+=(const SimTime& x) { time += x.time; return *this; }
	SimTime& operator-=(const SimTime& x) { time -= x.time; return *this; }
	SimTime& operator+=(float f) { time += f; return *this; }
	SimTime& operator-=(float f) { time -= f; return *this; }

	bool operator==(const SimTime& x) const { return time == x.time; }
	bool operator!=(const SimTime& x) const { return time != x.time; }
	bool operator<=(const SimTime& x) const { return time <= x.time; }
	bool operator>=(const SimTime& x) const { return time >= x.time; }
	bool operator<(const SimTime& x) const { return time < x.time; }
	bool operator>(const SimTime& x) const { return time > x.time; }

	int   scale(int x) const;
	float scale(float x) const;
	Vector2F* scale(Vector2F* v) const;
	Vector3F* scale(Vector3F* v) const;
};


inline int SimTime::scale(int x) const
{
	return int(x * time);
}

inline float SimTime::scale(float x) const
{
	return float(x * time);
}

inline Vector2F* SimTime::scale(Vector2F* v) const
{
	v->x = scale(v->x);
	v->y = scale(v->y);
	return v;
}

inline Vector3F* SimTime::scale(Vector3F* v) const
{
	v->x = scale(v->x);
	v->y = scale(v->y);
	v->z = scale(v->z);
	return v;
}

/*
inline Point2* SimTime::scale(Point2* v)
{
	v->x = scale(v->x);
	v->y = scale(v->y);
	return v;
}

inline Point3* SimTime::scale(Point3* v)
{
	v->x = scale(v->x);
	v->y = scale(v->y);
	v->z = scale(v->z);
	return v;
}
*/

/*
namespace SimTime
{
	enum Second {
		SecondShift = 8,
		OneSecond = 1 << SecondShift,
	};

	typedef int Time;
	int scale(int x, Time dt);
	Vector2* scale(Vector3* v,Time dt);
	Vector3* scale(Vector3* v,Time dt);
};

inline int SimTime::scale(int x,Time dt)
{
	return (time * second) >> SimSecondShift;
}

inline Vector2* SimTime::scale(Vector2* v,Time dt)
{
	v->x = scale(v->x,dt);
	v->y = scale(v->y,dt);
	return *v;
}

inline Vector3* SimTime::scale(Vector3* v,Time dt)
{
	v->x = scale(v->x,dt);
	v->y = scale(v->y,dt);
	v->z = scale(v->z,dt);
	return v;
}

*/

#endif
