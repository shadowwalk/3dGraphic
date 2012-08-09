////declaration of all the primitives derived from the object class

#pragma once

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include "Object.h"


class CSphere: public Object
{
private:
	GzCoord m_centre;
	float m_radius;

public:
	ObjectID getType() {return SPHERE; }
	CSphere() {}
	CSphere(GzCoord centre, float radius);
	~CSphere() {}

	bool Intersects(GzRay ray, float &tValue);
	void getSurfaceNormal(GzCoord atPoint, GzCoord* normal);
	void getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity);
	void setCentre(GzCoord* centre);
	void setKr(GzColor reflectance);
	float getRadius() {return m_radius; }
	inline void getCentre(GzCoord* centre)
	{
		memcpy(centre, &m_centre, sizeof(GzCoord));
	}
};

class CPlane: public Object
{
private:
	GzCoord m_Normal;
	float m_D;	//a

public:
	ObjectID getType() {return PLANE; }
	CPlane() {}
	CPlane(GzCoord normal, float D);

	bool Intersects(GzRay ray, float &tValue);
	void getSurfaceNormal(GzCoord atPoint, GzCoord* normal);
	void getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity);
	void setKr(GzColor reflectance);

	inline void getNormal(GzCoord* norm)
	{
		memcpy(norm, m_Normal, sizeof(GzCoord));
	}

	inline void setNormal(GzCoord * norm)
	{
		memcpy(m_Normal, norm, sizeof(GzCoord));
	}

	float getDValue() {return m_D; }
};

class CTriangle: public Object
{
public:
	SceneMesh m_trianglePrimitives;
	ObjectID getType() {return TRIANGLE;}
	CTriangle() {}
	CTriangle(SceneMesh prim);
	~CTriangle() {}

	void setKr(GzColor reflectance);
	bool Intersects(GzRay ray, float &tValue);
	void getSurfaceNormal(GzCoord atPoint, GzCoord* normal);
	void getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity);

	SceneMesh getPrimitives() {return m_trianglePrimitives;}
};

#endif
