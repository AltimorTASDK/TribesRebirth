//--------------------------------------------------------------
// fxRenderImage
//--------------------------------------------------------------

#include <ts.h>
#include <fxRenderImage.h>
#include <g_surfac.h>

// assumes dirY is normalized
void fxRenderImage::faceDirection(Point3F & dirY)
{
	// if we rotate about an axis, use a different routine...
	if (useRotationAxis)
	{
		faceDirection(dirY,rotationAxis);
		return;
	}

	Point3F dirX,dirZ;

	if (fabs(dirY.z) < 0.95)
	{
		// dirY is not near vector (0,0,1), so we can
		// use it as the pivot vector
		m_cross(dirY, Point3F(0,0,1), &dirX);
		dirX.normalize();
		m_cross(dirX, dirY, &dirZ);
	}
	else
	{
		// dirY is near vector (0,0,1), so use
		// pivot Point3F(1,0,0) instead
		m_cross(Point3F(1,0,0), dirY, &dirZ);
		dirZ.normalize();
		m_cross(dirY, dirZ, &dirX);
	}

	transform.setRow(0,dirX);
	transform.setRow(1,dirY);
	transform.setRow(2,dirZ);
	transform.flags |=  TMat3F::Matrix_HasRotation;
	transform.flags &= ~TMat3F::Matrix_HasScale;

   if (useAxisSpin == true) {
      RMat3F tempRot(EulerF(0, axisSpin, 0));
      TMat3F tempOutput;
      m_mul(tempRot, transform, &tempOutput);
      transform = tempOutput;
   }
}

// assumes dirY and dirZ are normalized
void fxRenderImage::faceDirection(Point3F & _dirY, Point3F & dirZ)
{
	// dirZ is the fixed axis we rotate about, dirY can change some
	Point3F dirX, dirY = _dirY;

	m_cross(dirY, dirZ, &dirX);
	float xlenSq = m_dot(dirX,dirX);
	if (IsEqual(xlenSq,0.0f))
	{
		// hard-luck case -- dirY and dirZ are parallel
		// find any old normalized dirX perp. to dirZ
		if (fabs(dirY.z) < 0.95)
		{
			// dirY is not near vector (0,0,1), so we can
			// use it as the pivot vector
			m_cross(dirY, Point3F(0,0,1), &dirX);
			dirX.normalize();
		}
		else
		{
			// dirY is near vector (0,0,1), so use
			// pivot Point3F(1,0,0) instead
			m_cross(dirY, Point3F(1,0,0), &dirX);
			dirX.normalize();
		}
	}
	else
		dirX *= m_invsqrtf(xlenSq);
	m_cross(dirZ, dirX, &dirY);

	transform.setRow(0,dirX);
	transform.setRow(1,dirY);
	transform.setRow(2,dirZ);
	transform.flags |=  TMat3F::Matrix_HasRotation;
	transform.flags &= ~TMat3F::Matrix_HasScale;
}

void
fxRenderImage::setAxisSpin(const float in_spin)
{
   axisSpin    = in_spin;
   useAxisSpin = true;
}

void
fxRenderImage::clearAxisSpin()
{
   useAxisSpin = false;
}

void fxRenderImage::findCameraAxis(TSRenderContext &rc)
{
	TMat3F wCamMat = rc.getCamera()->getTCW();

	camAxis = transform.p;
	camAxis -= wCamMat.p;
	camDist = m_dot(camAxis,camAxis);
	if (IsEqual(camDist,0.0f))
		camAxis.set(0,1,0); // camAxis.len()==0, just use standard axes
	else
	{
		camDist = m_sqrtf(camDist);
		camAxis *= 1/camDist;
	}
}

void fxRenderImage::faceCamera(TSRenderContext & rc, bool _findCamAxis)
{
	if (_findCamAxis)
		findCameraAxis(rc);
	faceDirection(camAxis);
}

void fxRenderImage::faceAwayCamera(TSRenderContext & rc, bool _findCamAxis)
{
	if (_findCamAxis)
		findCameraAxis(rc);
        Point3F awayAxis = camAxis;
        awayAxis *= -1.0f;
	faceDirection(awayAxis);
}


void fxRenderImage::setRotationAxis(const Point3F & _rotAxis)
{
	rotationAxis = _rotAxis;
	useRotationAxis = true;
}

void fxRenderImage::clearRotationAxis()
{
	useRotationAxis = false;
}

bool fxRenderImage::testBackfacing(const Point3F & front)
{
	return m_dot(front,camAxis)>0;
}

// call only when it changes (and first time)
void fxRenderImage::setTranslucent(bool nv)
{
	TSShapeInstance::Thread *oldThread = thread;

	drawTranslucent = nv;
	if (nv)
	{
		shape = translucentShape;
		thread = translucentThread;
		itype = SimRenderImage::Translucent;
	}
	else
	{
		shape = nonTranslucentShape;
		thread = nonTranslucentThread;
		itype = SimRenderImage::Normal;
	}

	if (oldThread && thread)
	{
		float tpos = oldThread->getPosition();
		thread->SetPosition(tpos);
	}
}

//------------------------------------------------------------------------------
float fxRenderImage::findCameraDistance(TSCamera * camera)
{
   const TMat3F & twc = camera->getTWC();
   // don't need x & y, so skip the extra work...
   return twc.p.y + transform.p.x * twc.m[0][1] +
                    transform.p.y * twc.m[1][1] +
                    transform.p.z * twc.m[2][1];
}

void fxRenderImage::setSortValue(TSCamera * camera)
{
	if (itype != SimRenderImage::Translucent)
	{
		sortValue = 0;
		return;
	}
   
   float cd = findCameraDistance(camera)-0.1f; // pretend we're a touch closer
                                               // to the camera in case it's an
                                               // explosion "on" a shape
   float nd = camera->getNearDist();
   sortValue = (cd<=nd) ? 0.99f : nd/cd;
}

//------------------------------------------------------------------------------

void fxRenderImage::render(TSRenderContext &rc)
{
	if (itype == SimRenderImage::Translucent)
	   rc.getSurface()->setZTest(GFX_ZTEST);

  	rc.getCamera()->pushTransform(transform);
   shape->selectDetail(rc);
   shape->animate();
   shape->render(rc);
   rc.getCamera()->popTransform();

	if (itype == SimRenderImage::Translucent)
	   rc.getSurface()->setZTest(GFX_ZTEST_AND_WRITE);
}
