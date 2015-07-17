//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _mover_H_
#define _mover_H_

#include <simRenderGrp.h>
#include <ts_shapeInst.h>

#define COREAPI
// #include <simtime.h>

enum { moveAbsolute, moveRelative, moveSemiRelative, moveOrbital };

class moverHelper {
	int moveMode;

	Point3F pos;
	Point3F posVel;
	EulerF rot;
	EulerF rotVel;
	float orbitDist; // for move orbital only
	EulerF orbitRot; // for move orbital

	void adjAngle(const float &a,float &b,const float within=M_PI);

public:
	moverHelper();
	~moverHelper();
	void init();
	void processAction(int action, float mag);
	void update(SimTime t);
	void setMode(int nm) {moveMode=nm;}
	const Point3F& getPos() {return pos;}
	const Point3F& getPosVel() {return posVel;}
	const EulerF& getRot() {return rot;}
	const EulerF& getRotVel() {return rotVel;}
	float& getOrbitDist() { return orbitDist; }
	const EulerF& getOrbitRot() { return orbitRot; }
	void setPos(Point3F np) { pos.x=np.x;pos.y=np.y;pos.z=np.z;}
	void setPosVel(Point3F nv) { posVel.x=nv.x;posVel.y=nv.y;posVel.z=nv.z;}
	void setRot(EulerF nr) { rot.x = nr.x; rot.y=nr.y;rot.z=nr.z;}
	void setRotVel(EulerF nv) { rotVel.x=nv.x; rotVel.y=nv.y; rotVel.z=nv.z;}
	void setOrbitDist(float nd) { orbitDist = nd; }
	void setOrbitRot(EulerF nr) { orbitRot.x= nr.x;orbitRot.y=nr.y;orbitRot.z=nr.z; }
};
#endif