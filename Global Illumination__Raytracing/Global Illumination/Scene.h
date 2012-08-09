////Scene Class that contains a list of all the primitives ////

#pragma once

#ifndef SCENE_H
#define SCENE_H

#include "Primitive.h"

class Scene
{
private:
	Object** m_pPrimitives;
public:
	static int numPrimitives;

	Scene();
	~Scene();
	void SetupScene();
	Object* getObjectAt(int idx) {return m_pPrimitives[idx]; }
};

#endif