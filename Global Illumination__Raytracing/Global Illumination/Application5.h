// Application4.h: interface for the Application4 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APPLICATION5_H__43A7FA9C_6CD6_4A79_9567_2354BFEFAFFB__INCLUDED_)
#define AFX_APPLICATION5_H__43A7FA9C_6CD6_4A79_9567_2354BFEFAFFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Application.h"
#include "Scene.h"
//#include "Primitive.h"
#include<vector>

using namespace std;

class Application5 : public Application  
{
private:
	vector<SceneMesh*> m_pMesh;

	vector<CTriangle> m_pTeapot;
	Scene m_Scene;
	CSphere* mSphere;
public:
	Application5();
	virtual ~Application5();
	
	int	Initialize();
	int LoadScene();
	virtual int Render(); 
	int Clean();
	//for debugging
void ReDirectIOToConsole();
};

#endif // !defined(AFX_APPLICATION5_H__43A7FA9C_6CD6_4A79_9567_2354BFEFAFFB__INCLUDED_)
