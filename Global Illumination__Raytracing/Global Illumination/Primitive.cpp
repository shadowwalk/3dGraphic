////definitions of all the functions used by the primitive objects ////

#include "StdAfx.h"
#include "Primitive.h"


//Sphere functions ////
CSphere::CSphere(GzCoord centre, float radius)
{
	m_radius = radius;
	memcpy(m_centre, centre,sizeof(GzCoord));
}

void CSphere::setCentre(GzCoord* centre)
{
	memcpy(m_centre,centre,sizeof(GzCoord));
}
bool CSphere::Intersects(GzRay ray, float &tValue)
{
	GzCoord origVec;
	origVec[X] =  m_centre[X] - ray.origin[X];
	origVec[Y] =  m_centre[Y] - ray.origin[Y];
	origVec[Z] =  m_centre[Z] - ray.origin[Z];

	float a = dotProduct(origVec,ray.direction); 

	float b = dotProduct(origVec, origVec) - (a*a); //bsquare value using pythagoras theorem

	float f = (m_radius * m_radius) - b;

	if(f < 0) //means square root wil definitely be negative
		return false;
	
	tValue = a - sqrt(f);
	return true;

}

void CSphere::getSurfaceNormal(GzCoord atPoint, GzCoord *normal)
{
	(*normal)[X] = atPoint[X] - m_centre[X];
	(*normal)[Y] = atPoint[Y] - m_centre[Y];
	(*normal)[Z] = atPoint[Z] - m_centre[Z];

	Normalize(normal);
}

void CSphere:: getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity)
{
	GzCoord normal; //need to calculate normal at hitpoint
	GzColor resultIntensity;
	
	getSurfaceNormal(hitPoint, &normal);

	//PhongIllumination(render,ray,normal,&resultIntensity);

	resultIntensity[RED] *= 256;
	resultIntensity[GREEN] *= 256;
	resultIntensity[BLUE] *= 256;

	memcpy(intensity,resultIntensity,sizeof(GzColor));

}

void CSphere::setKr(GzColor reflectance)
{
	memcpy(&Kr,&reflectance,sizeof(GzColor));
}

////////////////////

////Plane functions /////

CPlane::CPlane(GzCoord normal, float Dval)
{
	m_D = Dval;
	memcpy(m_Normal, normal, sizeof(GzCoord));
}

bool CPlane::Intersects(GzRay ray, float &tValue)
{
	float dN = dotProduct(ray.direction, m_Normal);

	if(dN == 0)   // means ray is parallel to the plane
		return false;

	tValue = m_D - ((dotProduct(ray.origin,m_Normal)) / dN);
	if(tValue < 0)
		return false;

	return true;
}

void CPlane::getSurfaceNormal(GzCoord atPoint, GzCoord *normal)
{
	memcpy(normal,m_Normal,sizeof(GzCoord));
}

void CPlane::setKr(GzColor reflectance)
{
	memcpy(&Kr,&reflectance,sizeof(GzColor));
}

void CPlane::getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity)
{
	//use the surface normal
	GzColor resultIntensity;

	//PhongIllumination(render,ray,m_Normal,&resultIntensity);

	resultIntensity[RED] *= 256;
	resultIntensity[GREEN] *= 256;
	resultIntensity[BLUE] *= 256;

	memcpy(intensity,resultIntensity,sizeof(GzColor));
}

/////////////////////////

CTriangle::CTriangle(SceneMesh prim)
{
	memcpy(&m_trianglePrimitives, &prim, sizeof(SceneMesh));
}

void CTriangle::setKr(GzColor reflectance)
{
	memcpy(&Kr,&reflectance,sizeof(GzColor));
}

bool CTriangle::Intersects(GzRay ray, float &tValue)
{
	GzCoord L1, L2;
	L1[X] = m_trianglePrimitives.vertexList[1][X] - m_trianglePrimitives.vertexList[0][X];
	L1[Y] = m_trianglePrimitives.vertexList[1][Y] - m_trianglePrimitives.vertexList[0][Y];
	L1[Z] = m_trianglePrimitives.vertexList[1][Z] - m_trianglePrimitives.vertexList[0][Z];

	L2[X] = m_trianglePrimitives.vertexList[2][X] - m_trianglePrimitives.vertexList[0][X];
	L2[Y] = m_trianglePrimitives.vertexList[2][Y] - m_trianglePrimitives.vertexList[0][Y];
	L2[Z] = m_trianglePrimitives.vertexList[2][Z] - m_trianglePrimitives.vertexList[0][Z];

	GzCoord distanceVec;
	distanceVec[X] = ray.origin[X] - m_trianglePrimitives.vertexList[0][X];
	distanceVec[Y] = ray.origin[Y] - m_trianglePrimitives.vertexList[0][Y];
	distanceVec[Z] = ray.origin[Z] - m_trianglePrimitives.vertexList[0][Z];

	float distance = getVectorMagnitude(distanceVec);

	GzCoord S1;
	crossProduct(ray.direction,L2,&S1);
	float d = 1 / dotProduct(S1,L1);

	float u = dotProduct(distanceVec, S1) * d; //1st barycentric coordinate

	if(u < 0 || u > 1) 
		return false;

	GzCoord S2;
	crossProduct(distanceVec,L1,&S2);
	
	float v = dotProduct(ray.direction, S2) * d; //2nd barycentric coordinate

	if(v < 0 || (u+v) > 1)
		return false;

	tValue = dotProduct(L2, S2) * d;

	return true;
}


void CTriangle::getSurfaceNormal(GzCoord atPoint, GzCoord* normal)
{
	NormalInterpolation(atPoint, (m_trianglePrimitives.vertexList), m_trianglePrimitives.normalList, normal);
}

void CTriangle::getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity)
{
	GzColor resultIntensity;
	GzCoord norm;
	GzCoord temp;
	memcpy(&temp, &hitPoint, sizeof(GzCoord));

	getSurfaceNormal(hitPoint,&temp);

	//PhongIllumination(render,ray,temp,&resultIntensity);

	resultIntensity[RED] *= 256;
	resultIntensity[GREEN] *= 256;
	resultIntensity[BLUE] *= 256;

	memcpy(intensity,resultIntensity,sizeof(GzColor));
}