// Application5.cpp: implementation of the Application5 class.
//
//////////////////////////////////////////////////////////////////////

/*
 * application test code for homework assignment #5
*/

#include "stdafx.h"
#include "CS580HW.h"
#include "Application6.h"
#include "Gz.h"
#include "disp.h"
#include "rend.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#define INFILE  "ppot.asc"
#define INFILE  "fullteapot.asc"
#define OUTFILE "output.ppm"

#define OUTFILE0 "output0.ppm"
#define OUTFILE1 "output1.ppm"
#define OUTFILE2 "output2.ppm"
#define OUTFILE3 "output3.ppm"
#define OUTFILE4 "output4.ppm"
#define OUTFILE5 "output5.ppm"

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
	GzToken     nameListAA[2];		/* light info */
	GzPointer   valueListAA[2];
	int			shaderType, interpStyle;
	float		specpower;
	int		status; 
 
	status = 0; 

	/* 
	 * Allocate memory for user input
	 */
	m_pUserInput = new GzInput;
	
	float AAFilter[AAKERNEL_SIZE][3]= 
			{
			-0.52, 0.38, 0.128,
			0.41, 0.56, 0.119,
			0.27, 0.08, 0.294,
			-0.17, -0.29, 0.249,
			0.58, -0.55, 0.104,
			-0.31, -0.71, 0.106
			};
	
	 /*
	 * initialize the display and the renderer 
	 */ 
 	m_nWidth = 256;		// frame buffer and display width
	m_nHeight = 256;    // frame buffer and display height

	status |= GzNewFrameBuffer(&m_pFrameBuffer, m_nWidth, m_nHeight);

	status |= GzNewDisplay(&m_pDisplay, GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);

	status |= GzGetDisplayParams(m_pDisplay, &xRes, &yRes, &dispClass); 
	 
	status |= GzInitDisplay(m_pDisplay); 
 
	status |= GzNewRender(&m_pRender, GZ_Z_BUFFER_RENDER, m_pDisplay); 

	for(int i=0;i<AAKERNEL_SIZE;i++){

		status |= GzNewFrameBuffer(&aa_pFrameBuffer[i], m_nWidth, m_nHeight);

		status |= GzNewDisplay(&aa_pDisplay[i], GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);

		status |= GzGetDisplayParams(aa_pDisplay[i], &xRes, &yRes, &dispClass); 
	 
		status |= GzInitDisplay(aa_pDisplay[i]); 
 
		status |= GzNewRender(&aa_pRender[i], GZ_Z_BUFFER_RENDER, aa_pDisplay[i]); 
	
	}

/* Translation matrix */
GzMatrix	scale = 
{ 
	3.25,	0.0,	0.0,	0.0, 
	0.0,	3.25,	0.0,	-3.25, 
	0.0,	0.0,	3.25,	3.5, 
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

#if 0 	/* set up app-defined camera if desired, else use camera defaults */
    camera.position[X] = -3;
    camera.position[Y] = -25;
    camera.position[Z] = -4;

    camera.lookat[X] = 7.8;
    camera.lookat[Y] = 0.7;
    camera.lookat[Z] = 6.5;

    camera.worldup[X] = -0.2;
    camera.worldup[Y] = 1.0;
    camera.worldup[Z] = 0.0;

    camera.FOV = 63.7;              /* degrees *              /* degrees */

	status |= GzPutCamera(m_pRender, &camera); 
	
	for(int i=0;i<AAKERNEL_SIZE;i++){
		status |= GzPutCamera(aa_pRender[i], &camera); 
	}

#endif 

	/* Start Renderer */
	status |= GzBeginRender(m_pRender);

	for(int i=0;i<AAKERNEL_SIZE;i++){
		status |= GzBeginRender(aa_pRender[i]); 
	}	

	/* Light */
	GzLight	light1 = { {-0.7071, 0.7071, 0}, {0.5, 0.5, 0.9} };
	GzLight	light2 = { {0, -0.7071, -0.7071}, {0.9, 0.2, 0.3} };
	GzLight	light3 = { {0.7071, 0.0, -0.7071}, {0.2, 0.7, 0.3} };
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
		for(int i=0;i<AAKERNEL_SIZE;i++){
			status |= GzPutAttribute(aa_pRender[i], 3, nameListLights, valueListLights);
		}	

		nameListLights[0] = GZ_AMBIENT_LIGHT;
        valueListLights[0] = (GzPointer)&ambientlight;
        status |= GzPutAttribute(m_pRender, 1, nameListLights, valueListLights);
		for(int i=0;i<AAKERNEL_SIZE;i++){
			status |= GzPutAttribute(aa_pRender[i], 1, nameListLights, valueListLights);
		}	        
		
		for(int i=0;i<AAKERNEL_SIZE;i++){
			nameListAA[0] = GZ_AASHIFTX;
			valueListAA[0] = (GzPointer)&AAFilter[i][0];
			nameListAA[1] = GZ_AASHIFTY;
			valueListAA[1] = (GzPointer)&AAFilter[i][1];
			status |= GzPutAttribute(aa_pRender[i], 2, nameListAA, valueListAA);
		}	   
		
		/*
         * Tokens associated with shading 
         */
        nameListShader[0]  = GZ_DIFFUSE_COEFFICIENT;
        valueListShader[0] = (GzPointer)diffuseCoefficient;

	/* 
	* Select either GZ_COLOR or GZ_NORMALS as interpolation mode  
	*/
        nameListShader[1]  = GZ_INTERPOLATE;
#if 0
		interpStyle = GZ_COLOR;
#else
        interpStyle = GZ_NORMALS;         /* Phong shading */
#endif      
		valueListShader[1] = (GzPointer)&interpStyle;

        nameListShader[2]  = GZ_AMBIENT_COEFFICIENT;
        valueListShader[2] = (GzPointer)ambientCoefficient;
        nameListShader[3]  = GZ_SPECULAR_COEFFICIENT;
        valueListShader[3] = (GzPointer)specularCoefficient;
        nameListShader[4]  = GZ_DISTRIBUTION_COEFFICIENT;
        specpower = 32;
        valueListShader[4] = (GzPointer)&specpower;

        nameListShader[5]  = GZ_TEXTURE_MAP;
#if 1   /* set up null texture function or valid pointer */
        valueListShader[5] = (GzPointer)0;
#elif 0
        valueListShader[5] = (GzPointer)(tex_fun);	/* or use ptex_fun */
#elif 0
		valueListShader[5] = (GzPointer)(ptex_fun);
#endif
       
		
	status |= GzPutAttribute(m_pRender, 6, nameListShader, valueListShader);

	status |= GzPushMatrix(m_pRender, scale);  
	status |= GzPushMatrix(m_pRender, rotateY); 
	status |= GzPushMatrix(m_pRender, rotateX); 

	for(int i=0;i<AAKERNEL_SIZE;i++){
		status |= GzPutAttribute(aa_pRender[i], 6, nameListShader, valueListShader);

		status |= GzPushMatrix(aa_pRender[i], scale);  
		status |= GzPushMatrix(aa_pRender[i], rotateY); 
		status |= GzPushMatrix(aa_pRender[i], rotateX);	
	}

	if (status) exit(GZ_FAILURE); 

	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Render() 
{
	GzToken		nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer	valueListTriangle[3]; 	/* vertex attribute pointers */
	GzCoord		vertexList[3];	/* vertex position coordinates */ 
	GzCoord		normalList[3];	/* vertex normals */ 
	GzTextureIndex  	uvList[3];		/* vertex texture map indices */ 
	char		dummy[256]; 
	int			status; 
	float AAFilter[AAKERNEL_SIZE][3]= 
	{
		-0.52, 0.38, 0.128,
		0.41, 0.56, 0.119,
		0.27, 0.08, 0.294,
		-0.17, -0.29, 0.249,
		0.58, -0.55, 0.104,
		-0.31, -0.71, 0.106
	};
	/* Initialize Display */
	status |= GzInitDisplay(m_pDisplay); 
	for(int i=0;i<AAKERNEL_SIZE;i++){
		status |= GzInitDisplay(aa_pDisplay[i]); 
	}
		
	/* 
	* Tokens associated with triangle vertex values 
	*/ 
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  

	// I/O File open
	FILE *infile;
	if( (infile  = fopen( INFILE , "r" )) == NULL )
	{
         AfxMessageBox( "The input file was not opened\n" );
		 return GZ_FAILURE;
	}

	FILE *outfile;
	if( (outfile  = fopen( OUTFILE , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}

	FILE *outfile0;
	if( (outfile0  = fopen( OUTFILE0 , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}

	FILE *outfile1;
	if( (outfile1  = fopen( OUTFILE1 , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}
	FILE *outfile2;
	if( (outfile2  = fopen( OUTFILE2 , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}
	FILE *outfile3;
	if( (outfile3  = fopen( OUTFILE3 , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}
	FILE *outfile4;
	if( (outfile4  = fopen( OUTFILE4 , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}
	FILE *outfile5;
	if( (outfile5  = fopen( OUTFILE5 , "wb" )) == NULL )
	{
         AfxMessageBox( "The output file was not opened\n" );
		 return GZ_FAILURE;
	}
	/* 
	* Walk through the list of triangles, set color 
	* and render each triangle 
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

	    /* 
	     * Set the value pointers to the first vertex of the 	
	     * triangle, then feed it to the renderer 
	     * NOTE: this sequence matches the nameList token sequence
	     */ 
	     valueListTriangle[0] = (GzPointer)vertexList; 
		 valueListTriangle[1] = (GzPointer)normalList; 
		 valueListTriangle[2] = (GzPointer)uvList; 
		 GzPutTriangle(m_pRender, 3, nameListTriangle, valueListTriangle); 

		for(int i=0;i<AAKERNEL_SIZE;i++){
			GzPutTriangle(aa_pRender[i], 3, nameListTriangle, valueListTriangle); 
		}
	} 
	//GzFlushDisplay2File(outfile, aa_pDisplay[0]); 	/* write out or update display to file*/
	//GzFlushDisplay2FrameBuffer(m_pFrameBuffer, aa_pDisplay[5]);	// write out or update display to frame buffer
	//GzFlushDisplay2File(outfile0, aa_pDisplay[0]);
	//GzFlushDisplay2File(outfile1, aa_pDisplay[1]);
	//GzFlushDisplay2File(outfile2, aa_pDisplay[2]);
	//GzFlushDisplay2File(outfile3, aa_pDisplay[3]);
	//GzFlushDisplay2File(outfile4, aa_pDisplay[4]);
	//GzFlushDisplay2File(outfile5, aa_pDisplay[5]);
	
	for(int j=0;j<m_pDisplay->yres;j++){
		for(int i=0;i<m_pDisplay->xres;i++){
			((m_pDisplay->fbuf)+ i + j*(m_pDisplay->xres))->red = 
				  ((aa_pDisplay[0]->fbuf)+ i + j*(aa_pDisplay[0]->xres))->red * AAFilter[0][2] 
				+ ((aa_pDisplay[1]->fbuf)+ i + j*(aa_pDisplay[1]->xres))->red * AAFilter[1][2]
				+ ((aa_pDisplay[2]->fbuf)+ i + j*(aa_pDisplay[2]->xres))->red * AAFilter[2][2]
				+ ((aa_pDisplay[3]->fbuf)+ i + j*(aa_pDisplay[3]->xres))->red * AAFilter[3][2]
				+ ((aa_pDisplay[4]->fbuf)+ i + j*(aa_pDisplay[4]->xres))->red * AAFilter[4][2]
				+ ((aa_pDisplay[5]->fbuf)+ i + j*(aa_pDisplay[5]->xres))->red * AAFilter[5][2];
			((m_pDisplay->fbuf)+ i + j*(m_pDisplay->xres))->green = 
				  ((aa_pDisplay[0]->fbuf)+ i + j*(aa_pDisplay[0]->xres))->green * AAFilter[0][2]
				+ ((aa_pDisplay[1]->fbuf)+ i + j*(aa_pDisplay[1]->xres))->green * AAFilter[1][2]
				+ ((aa_pDisplay[2]->fbuf)+ i + j*(aa_pDisplay[2]->xres))->green * AAFilter[2][2]
				+ ((aa_pDisplay[3]->fbuf)+ i + j*(aa_pDisplay[3]->xres))->green * AAFilter[3][2]
				+ ((aa_pDisplay[4]->fbuf)+ i + j*(aa_pDisplay[4]->xres))->green * AAFilter[4][2]
				+ ((aa_pDisplay[5]->fbuf)+ i + j*(aa_pDisplay[5]->xres))->green * AAFilter[5][2];
			((m_pDisplay->fbuf)+ i + j*(m_pDisplay->xres))->blue = 
				  ((aa_pDisplay[0]->fbuf)+ i + j*(aa_pDisplay[0]->xres))->blue * AAFilter[0][2]
				+ ((aa_pDisplay[1]->fbuf)+ i + j*(aa_pDisplay[1]->xres))->blue * AAFilter[1][2]
				+ ((aa_pDisplay[2]->fbuf)+ i + j*(aa_pDisplay[2]->xres))->blue * AAFilter[2][2]
				+ ((aa_pDisplay[3]->fbuf)+ i + j*(aa_pDisplay[3]->xres))->blue * AAFilter[3][2]
				+ ((aa_pDisplay[4]->fbuf)+ i + j*(aa_pDisplay[4]->xres))->blue * AAFilter[4][2]
				+ ((aa_pDisplay[5]->fbuf)+ i + j*(aa_pDisplay[5]->xres))->blue * AAFilter[5][2];	
		}
	}


	GzFlushDisplay2File(outfile, m_pDisplay); 	/* write out or update display to file*/
	GzFlushDisplay2FrameBuffer(m_pFrameBuffer, m_pDisplay);	// write out or update display to frame buffer

	/* 
	 * Close file
	 */ 

	if( fclose( infile ) )
      AfxMessageBox( "The input file was not closed\n" );

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

	for(int i=0;i<AAKERNEL_SIZE;i++){
		status |= GzFreeRender(aa_pRender[i]); 
		status |= GzFreeDisplay(aa_pDisplay[i]);		
	}
	
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS);
}



