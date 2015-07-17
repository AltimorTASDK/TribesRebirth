//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

#ifndef _ESFTRAIL_H_
#define _ESFTRAIL_H_

#include <sim.h>
#include <ts.h>
#include "simRenderGrp.h"
class DLLAPI ESFTrailImage : public SimRenderImage
{
	struct Particle
	{
		Point3F pos;
		Point3F vel;
		float age;
		bool alive;
		int random;
	};

	int numParticles;
	float numActive;
	float lifetime;
	float invLifetime;

	Point3F pos;
	Point3F axis;         // back end of rocket...
	float speed;          // out of back of rocket (- for forward speed)
	Point3F drift;        // wind + flotation velocity
	float spread;         // velocity at which particles spread apart
	float radius;         // initial separation (orthogonal to axis) of particles

	int numColors;
	ColorF birthColor;
	ColorF deathColor;

	Particle * particles;
	int * colors;
	bool renderedYet;     // find colors on first render
	bool updatedYet;      // don't render till first update

	// detailing information
	float profileDetailDistance;  // distance that detailing begins when viewing profile
	float trailingDetailDistance; // distance that detailing begins when viewing from trail
	int detailProportion; // current proportion (out of 255) particles to draw

	// run-time variable used for detailing
	Point3F oldestPos;
	// run-time variable used for spreading particles
	Point3F prevPos;

	// regenerate particles
	bool regen;

	void newParticle(Particle *, const Point3F & step, int & newThisRound);
	void selectDetail(const Point3F & camPos);

public:
	void init(int _numParticles, float _duration, 
	                  float _speed = 0, float _radius = 0, float _spread = 0);
	void setPos(const Point3F & pos, const Point3F & backend);
	void setDrift(const Point3F & _drift);
	void setColors(const ColorF & _birth, const ColorF & _death, int _numColors);
	void setDetailDistance(float _pdd, float _tdd);
	void setRegen(int nv) { regen=nv; }
	void update(float dt);
	void render(TSRenderContext &rc);

	ESFTrailImage();
	~ESFTrailImage();
};


#endif
