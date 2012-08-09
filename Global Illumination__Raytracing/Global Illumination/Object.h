////General Object class from which other primitives will be derived ///////////

#pragma once

#ifndef OBJECT_H
#define OBJECT_H

#include "Gz.h"
#include "rend.h"


class Object
{
protected:
	GzColor	Kt, Kr;	//Kt - refractive index, Kr - reflective index

public:
	enum ObjectID 
	{
		SPHERE		= 0,
		PLANE		= 1,
		TRIANGLE	= 2		//since we implemented triangles, as well keep them
	};

	Object() {}
	inline void getKt(GzColor* transmittance)
	{
		memcpy(transmittance,&Kt,sizeof(GzCoord));
	}

	inline void getKr(GzColor* reflectance)
	{
		memcpy(reflectance,&Kr, sizeof(GzCoord));
	}

	virtual bool Intersects(GzRay ray, float &tValue) = 0;
	virtual void getSurfaceNormal(GzCoord atPoint, GzCoord* normal) = 0;
	virtual void getColor(GzRender* render, GzRay ray, GzCoord hitPoint, GzColor* intensity) = 0;
	virtual ObjectID getType() = 0;
	virtual ~Object() {}
	virtual void setKr(GzColor reflectance) = 0;

};

#endif