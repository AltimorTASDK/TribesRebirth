//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

#include "esfTrail.h"
#include <g_surfac.h>
#include <g_pal.h>

static Random trailRand;

void 
ESFTrailImage::init(int _numParticles, float _duration, 
                    float _speed, float _radius, float _spread)
{
	numParticles = _numParticles;
	numActive = 0.0f;
	regen = true;

	particles = new Particle[numParticles];

	lifetime = _duration;
	invLifetime = 1.0f/lifetime;

	speed = _speed;
	radius = _radius;
	spread = _spread;
}

ESFTrailImage::ESFTrailImage()
{
	// set up defaults
	drift.set(0,0,0.25f); // particles float up a little by default
	particles=0;
	colors=0;
	trailingDetailDistance = 10.0f; // default distance to begin detailing from back
	profileDetailDistance = 100.0f; // default distance to begin detailing from profile

	updatedYet = false;
	renderedYet = false;
}

ESFTrailImage::~ESFTrailImage()
{
	delete [] particles;
	delete [] colors;
}


void 
ESFTrailImage::setPos(const Point3F & _pos, const Point3F & _backend)
{
	prevPos = pos;
	pos  = _pos;
	axis = _backend;
}


void 
ESFTrailImage::setDrift(const Point3F & _drift)
{
	drift = _drift;
}

void 
ESFTrailImage::setColors(const ColorF & _birth, const ColorF & _death, int _numColors)
{
	birthColor = _birth;
	deathColor = _death;
	if (_numColors<1)
		numColors=1;
	else
		numColors  = _numColors;
}

void 
ESFTrailImage::setDetailDistance(float _pdist, float _tdist)
{
	profileDetailDistance = _pdist;
	trailingDetailDistance = _tdist;
}

void 
ESFTrailImage::update(float dt)
{
	if (!updatedYet)
	{
		prevPos = pos;
		oldestPos = pos;
		updatedYet = true;
	}

	// determine increment to deposit particles
	// over length of trail (from pos to prevPos)
	Point3F step = prevPos;
	step -= pos;
	int newThisRound=0;

	Particle * pidx = particles;
	Particle * endP = particles + (int) numActive;

	// how many more particles to add this round?
	float newActive = 0;
	if (numActive<numParticles)
	{
		newActive = numParticles * (invLifetime * dt);
		if (numActive+newActive>numParticles)
			newActive=numParticles-numActive;
	}

	for (; pidx!=endP; pidx++)
	{
		if (!pidx->alive)
			continue;

		pidx->age += dt;
		if (pidx->age<lifetime)
		{
			Point3F dv = pidx->vel;
			dv *= dt;
			pidx->pos += dv;
		}
		else if (regen)
		{
			oldestPos = pidx->pos;
			newParticle(pidx,step,newThisRound);
		}
		else
			pidx->alive=false;
	}
	if (!regen)
		return;

	int numNew = (int) (numActive+newActive) - (int) numActive;
	endP += numNew;

	for (; pidx!=endP; pidx++)
	{
		newParticle(pidx,step,newThisRound);
		pidx->alive=true;
	}
	numActive += newActive;
}

void 
ESFTrailImage::newParticle(Particle * p,const Point3F & step, int & numNew)
{
	// use step and numNew to determine where from pos to pos+step to place particle
	float t=0.0f;
	float pt5mult = 0.5; // multiple of .5
	int bitIdx = 1;
	int bitMask = ~0;
	while (numNew & bitMask && numNew < 128)
	{
		if (bitIdx&numNew)
			t += pt5mult;
		bitMask &= ~bitIdx;
		bitIdx <<= 1;
		pt5mult *= .5f;
	}
	Point3F position = step;
	position *= t;
	position += pos;
	numNew++;

	Point3F otherComp(trailRand.getFloat(-1,1),
	                  trailRand.getFloat(-1,1),
					  trailRand.getFloat(-1,1));
	p->pos  = otherComp;
	p->pos *= radius;
	p->pos += position;

	// make a new particle
	p->vel  = axis;
	p->vel *= speed;
	otherComp *= spread;
	p->vel += otherComp;
	p->vel += drift;

	p->age = 0;
	p->random = trailRand.getInt() & 0xff;
}

void 
ESFTrailImage::selectDetail(const Point3F & camPos)
{
	Point3F vec;
	vec.x = pos.x - oldestPos.x;
	vec.y = pos.y - oldestPos.y;
	vec.z = pos.z - oldestPos.z;
	float len2 = m_dot(vec,vec);
	Point3F closestPoint; // of camera to trail
	if (len2<1.0f)
		// just use pos to check
		closestPoint = pos;
	else
	{
		float kop = m_dot(vec,oldestPos);
		float kp  = m_dot(vec,pos);
		float kcp = m_dot(vec,camPos);
		if (kcp<kop)
			// use oldest pos to check detail
			closestPoint = oldestPos;
		else if (kcp>kp)
			// use pos to check detail
			closestPoint = pos;
		else
		{
			float k = (kcp-kop)/(kp-kop);
			// midPoint = oldestPos + k * vec
			closestPoint.x = oldestPos.x + k * vec.x;
			closestPoint.y = oldestPos.y + k * vec.y;
			closestPoint.z = oldestPos.z + k * vec.z;
		}
	}

	float dist = m_distf(camPos,closestPoint);

	// what is the interpolation parameter k between trailing and profile details
	float k;
	if (len2<1.0f)
		// use profile
		k = 0;
	else
	{
		vec *= 1.0f/m_sqrt(len2);
		closestPoint -= camPos;
		closestPoint *= 1/dist;
		k = fabs(m_dot(vec,closestPoint));
	}

	float prop=1;
	float profileProp, trailingProp;
	if (fabs(dist)>0)
	{
		profileProp = profileDetailDistance / dist;
		trailingProp = trailingDetailDistance / dist;
		prop = (1.0f-k) * profileProp + k * trailingProp;
	}
	if (prop>1.0f)
		prop=1.0f;
	detailProportion = 255.0f * prop;
}


void 
ESFTrailImage::render(TSRenderContext &rc)
{
	if (!updatedYet)
		return;

	GFXSurface * gfxSurface = rc.getSurface();

	if (!renderedYet)
	{
		renderedYet=true;

		colors = new int[numColors];

		// set colors
		GFXPalette *gfxPalette=gfxSurface->getPalette();
		float red,green,blue;

		float denom;
		if (numColors==1)
			denom=1.0f;
		else
			denom = 1.0f/(float)(numColors-1);

		for (int i=0;i<numColors;i++)
		{
			float t = float(i)*denom;
			red   = birthColor.red   * (1.0f-t) + deathColor.red   * t;
			green = birthColor.green * (1.0f-t) + deathColor.green * t;
			blue  = birthColor.blue  * (1.0f-t) + deathColor.blue  * t;
			colors[i] = gfxPalette->GetNearestColor(red,green,blue);
		}
	}

	gfxSurface->setTransparency(true);
	gfxSurface->setHazeSource(GFX_HAZE_NONE);
	gfxSurface->setShadeSource(GFX_SHADE_NONE);
	gfxSurface->setAlphaSource(GFX_ALPHA_NONE);

	TSCamera * camera = rc.getCamera();

	selectDetail(camera->getTCW().p);

	Particle * pidx = particles;
	Particle * endP = particles + (int)numActive;

	for (;pidx!=endP;pidx++)
	{
		if (!pidx->alive)
			continue;

		// randomly skip some particles at a distance
		if (pidx->random>detailProportion)
			continue;

		// find color
		float t = pidx->age * invLifetime;
		int colorIndex = t * ((float)(numColors)-0.01f);

		TS::TransformedVertex v;
		if (camera->transformProject(pidx->pos,&v) == TS::TransformedVertex::Projected)
			gfxSurface->drawPoint3d(&Point2I(v.fPoint.x,v.fPoint.y),
		   	                      v.fPoint.z,
		      	                   colors[colorIndex]);
	}
}


