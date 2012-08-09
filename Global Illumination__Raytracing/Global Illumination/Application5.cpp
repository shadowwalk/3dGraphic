// Application5.cpp: implementation of the Application5 class.
//
//////////////////////////////////////////////////////////////////////

/*
 * application test code for homework assignment #5
*/



#include "stdafx.h"
#include "CS580HW.h"
#include "Application5.h"
#include "Gz.h"
#include "disp.h"
#include "rend.h"

#include <windows.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

#ifndef _USE_OLD_IOSTREAMS
using namespace std;
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#define INFILE  "POT4.asc"
//#define INFILE	"tri.asc"
#define INFILE "ppot.asc"
#define OUTFILE "outputlight1.ppm"

static const WORD MAX_CONSOLE_LINES = 500;

extern int tex_fun(float u, float v, GzColor color); /* image texture function */
extern int ptex_fun(float u, float v, GzColor color); /* procedural texture function */

void shade(GzCoord norm, GzCoord color);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Application5::Application5()
{
}

Application5::~Application5()
{
	
}

int Application5::Initialize()
{
	GzCamera	camera;  
	int		    xRes, yRes, dispClass;	/* display parameters */ 

	GzToken		nameListShader[9]; 	    /* shader attribute names */
	GzPointer   valueListShader[9];		/* shader attribute pointers */
	GzToken     nameListLights[10];		/* light info */
	GzPointer   valueListLights[10];
	int			shaderType, interpStyle;
	float		specpower;
	int		status; 
 
	status = 0; 

	//create the console window
	//ReDirectIOToConsole();

	//Load the scene objects
	m_Scene.SetupScene();
	GzCoord centre = {-5,-8,18};
	//-0.225410, 5.733967, 30.610357
	mSphere = new CSphere(centre,10);
	//1st load the triangles from the file and save them in the vector
	status |= LoadScene();
	/* 
	 * Allocate memory for user input
	 */
	m_pUserInput = new GzInput;

	/* 
	 * initialize the display and the renderer 
	 */ 
 	m_nWidth = 640;		// frame buffer and display width
	m_nHeight = 480;    // frame buffer and display height

	status |= GzNewFrameBuffer(&m_pFrameBuffer, m_nWidth, m_nHeight);

	status |= GzNewDisplay(&m_pDisplay, GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);

	status |= GzGetDisplayParams(m_pDisplay, &xRes, &yRes, &dispClass); 
	 
	status |= GzInitDisplay(m_pDisplay); 
 
	status |= GzNewRender(&m_pRender, GZ_Z_BUFFER_RENDER, m_pDisplay); 

/* Translation matrix */
GzMatrix	translate = 
{
	1.0,	0.0,	0.0,	0.0,
	0.0,	1.0,	0.0,	-3.25,
	0.0,	0.0,	1.0,	3.5,
	0.0,	0.0,	0.0,	1.0
};

GzMatrix	scale = 
{ 
	3.0,	0.0,	0.0,	0.0, 
	0.0,	3.0,	0.0,	0.0, 
	0.0,	0.0,	3.0,	0.0, 
	0.0,	0.0,	0.0,	1.0 
}; 
 
GzMatrix	rotateX = 
{ 
	1.0,	0.0,	0.0,	0.0, 
	0.0,	.7071,	.7071,	0.0, 
	0.0,	-.7071,	.7071,	0.0, 
	0.0,	0.0,	0.0,	1.0 
}; 
 
GzMatrix	rotateY = 
{ 
	.866,	0.0,	-0.5,	0.0, 
	0.0,	1.0,	0.0,	0.0, 
	0.5,	0.0,	.866,	0.0, 
	0.0,	0.0,	0.0,	1.0 
}; 

//#if 1 	/* set up app-defined camera if desired, else use camera defaults */
  /*  camera.position[X] = -3;
    camera.position[Y] = -25;
    camera.position[Z] = -4;

    camera.lookat[X] = 7.8;
    camera.lookat[Y] = 0.7;
    camera.lookat[Z] = 6.5;

    camera.worldup[X] = -0.2;
    camera.worldup[Y] = 1.0;
    camera.worldup[Z] = 0.0;

    camera.FOV = 63.7;              /* degrees *              /* degrees */

//	status |= GzPutCamera(m_pRender, &camera); 
//#endif 

	/* Start Renderer */
	status |= GzBeginRender(m_pRender);

	/* Light */
	GzLight	light1 = { {-0.7071, 0.7071, 0.1}, {0.5, 0.5, 0.9} };
	GzLight	light2 = { {0.1, -0.7071, -0.7071}, {0.9, 0.2, 0.3} };
	//GzLight	light3 = { {0.7071, 0.0, -0.7071}, {0.2, 0.7, 0.3} };
	GzLight	light3 = { {0.7071, 0.1, -0.7071}, {0.2, 0.7, 0.3} };
	GzLight	ambientlight = { {0, 0, 0}, {0.3, 0.3, 0.3} };

	/* Material property */
	GzColor specularCoefficient = { 0.3, 0.3, 0.3 };
	GzColor ambientCoefficient = { 0.1, 0.1, 0.1 };
	GzColor diffuseCoefficient = {0.7, 0.7, 0.7};

/* 
  renderer is ready for frame --- define lights and shader at start of frame 
*/

        /*
         * Tokens associated with light parameters
         */
        nameListLights[0] = GZ_DIRECTIONAL_LIGHT;
        valueListLights[0] = (GzPointer)&light1;
        nameListLights[1] = GZ_DIRECTIONAL_LIGHT;
        valueListLights[1] = (GzPointer)&light2;
       nameListLights[2] = GZ_DIRECTIONAL_LIGHT;
       valueListLights[2] = (GzPointer)&light3;
        status |= GzPutAttribute(m_pRender, 3, nameListLights, valueListLights);

        nameListLights[0] = GZ_AMBIENT_LIGHT;
        valueListLights[0] = (GzPointer)&ambientlight;
        status |= GzPutAttribute(m_pRender, 1, nameListLights, valueListLights);

        /*
         * Tokens associated with shading 
         */
        nameListShader[0]  = GZ_DIFFUSE_COEFFICIENT;
        valueListShader[0] = (GzPointer)diffuseCoefficient;

	/* 
	* Select either GZ_COLOR or GZ_NORMALS as interpolation mode  
	*/
        nameListShader[1]  = GZ_INTERPOLATE;
	//	interpStyle = GZ_COLOR;			  /* Gouraud Shading */
        interpStyle = GZ_NORMALS;         /* Phong shading */
        valueListShader[1] = (GzPointer)&interpStyle;

        nameListShader[2]  = GZ_AMBIENT_COEFFICIENT;
        valueListShader[2] = (GzPointer)ambientCoefficient;
        nameListShader[3]  = GZ_SPECULAR_COEFFICIENT;
        valueListShader[3] = (GzPointer)specularCoefficient;
        nameListShader[4]  = GZ_DISTRIBUTION_COEFFICIENT;
        specpower = 32;
        valueListShader[4] = (GzPointer)&specpower;

        nameListShader[5]  = GZ_TEXTURE_MAP;
#if 0   /* set up null texture function or valid pointer */
        valueListShader[5] = (GzPointer)0;
#else
        valueListShader[5] = (GzPointer)(tex_fun);	/* or use ptex_fun */
#endif
        status |= GzPutAttribute(m_pRender, 6, nameListShader, valueListShader);

		//converts to world coordinates - also push them to the model world matrix for the time being
	status |= GzPushMatrix(m_pRender, translate);
	status |= PushModelWorldMatrix(m_pRender, translate,2);

	status |= GzPushMatrix(m_pRender, scale); 
//	status |= PushModelWorldMatrix(m_pRender, scale,0);

	status |= GzPushMatrix(m_pRender, rotateY); 
//	status |= PushModelWorldMatrix(m_pRender, rotateY,1);

	status |= GzPushMatrix(m_pRender, rotateX); 
	//status |= PushModelWorldMatrix(m_pRender, rotateX,1);

	if (status) exit(GZ_FAILURE); 

	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Render() 
{
//	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
//	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
//	GzCoord		vertexList[3];	/* vertex position coordinates */ 
//	GzCoord		normalList[3];	/* vertex normals */ 
//	GzTextureIndex  	uvList[3];		/* vertex texture map indices */ 
//	char		dummy[256]; 
	int			status; 


	/* Initialize Display */
	status |= GzInitDisplay(m_pDisplay); 
	
	/* 
	* Tokens associated with triangle vertex values 
	*/ 
/*	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  
*/
	
	FILE *outfile;
	if( (outfile  = fopen( OUTFILE , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}

/*	//run through the mesh triangles and render them using scan line rendering
	for(int i = 0; i < m_pMesh.size(); i++)
	{
		memcpy(vertexList, m_pMesh.at(i)->vertexList, sizeof(GzCoord) * 3);
		memcpy(normalList, m_pMesh.at(i)->normalList, sizeof(GzCoord) * 3);
		memcpy(uvList, m_pMesh.at(i)->uvList, sizeof(GzCoord) * 3);

		valueListTriangle[0] = (GzPointer) vertexList;
		valueListTriangle[1] = (GzPointer) normalList;
		valueListTriangle[2] = (GzPointer) uvList;
		GzPutTriangle(m_pRender,3,nameListTriangle,valueListTriangle);
	}
	
*/

	//call the raycasting function every frame
	//RayTracing(m_pRender,m_pMesh);
	RayTracing(m_pRender,&m_Scene);
	//RayTracing(m_pRender,m_pTeapot);

	GzFlushDisplay2File(outfile, m_pDisplay); 	/* write out or update display to file*/
	GzFlushDisplay2FrameBuffer(m_pFrameBuffer, m_pRender->display);	// write out or update display to frame buffer

	/* 
	 * Close file
	 */ 

	if( fclose( outfile ) )
      AfxMessageBox( "The output file was not closed\n" );
 
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Clean()
{
	/* 
	 * Clean up and exit 
	 */ 
	int	status = 0; 

	status |= GzFreeRender(m_pRender); 
	status |= GzFreeDisplay(m_pDisplay);
	
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS);
}

//for loading the scene objects - in this case all the triangles of the teapot
int Application5::LoadScene()
{
	SceneMesh*			pMeshObject = NULL;
	GzCoord				vertexList[3];	/* vertex position coordinates */ 
	GzCoord				normalList[3];	/* vertex normals */ 
	GzTextureIndex  	uvList[3];		/* vertex texture map indices */ 
	char				dummy[256]; 

	// I/O File open
	FILE *infile;
	if( (infile  = fopen( INFILE , "r" )) == NULL )
	{
         AfxMessageBox( "The input file was not opened\n" );
		 return GZ_FAILURE;
	}

	/* 
	* Walk through the list of triangles, set color 
	* and save each triangle in the mesh vector
	*/ 
	while( fscanf(infile, "%s", dummy) == 1) { 	/* read in tri word */
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[0][0]), &(vertexList[0][1]),  
		&(vertexList[0][2]), 
		&(normalList[0][0]), &(normalList[0][1]), 	
		&(normalList[0][2]), 
		&(uvList[0][0]), &(uvList[0][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[1][0]), &(vertexList[1][1]), 	
		&(vertexList[1][2]), 
		&(normalList[1][0]), &(normalList[1][1]), 	
		&(normalList[1][2]), 
		&(uvList[1][0]), &(uvList[1][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[2][0]), &(vertexList[2][1]), 	
		&(vertexList[2][2]), 
		&(normalList[2][0]), &(normalList[2][1]), 	
		&(normalList[2][2]), 
		&(uvList[2][0]), &(uvList[2][1]) ); 

		//store these values in the mesh vector
		pMeshObject = new SceneMesh();
		memcpy(pMeshObject->vertexList,vertexList,sizeof(GzCoord) * 3);
		memcpy(pMeshObject->normalList,normalList,sizeof(GzCoord) * 3);
		memcpy(pMeshObject->uvList,uvList,sizeof(GzCoord) * 3);

		m_pMesh.push_back(pMeshObject);

		CTriangle tempObject;
		memcpy(&(tempObject.getPrimitives()),pMeshObject,sizeof(SceneMesh));

		m_pTeapot.push_back(tempObject);
		
	} 
	
	if( fclose( infile ) )
      AfxMessageBox( "The input file was not closed\n" );

	return GZ_SUCCESS;
}

void Application5::ReDirectIOToConsole()
{
	int hConHandle;

long lStdHandle;

CONSOLE_SCREEN_BUFFER_INFO coninfo;

FILE *fp;

// allocate a console for this app
AllocConsole();

// set the screen buffer to be big enough to let us scroll text
GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

// redirect unbuffered STDOUT to the console

lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

fp = _fdopen( hConHandle, "w" );

*stdout = *fp;

setvbuf( stdout, NULL, _IONBF, 0 );

// redirect unbuffered STDIN to the console

lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

fp = _fdopen( hConHandle, "r" );

*stdin = *fp;

setvbuf( stdin, NULL, _IONBF, 0 );

// redirect unbuffered STDERR to the console

lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

fp = _fdopen( hConHandle, "w" );

*stderr = *fp;

setvbuf( stderr, NULL, _IONBF, 0 );

// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog

// point to console as well

ios::sync_with_stdio();
}