
#include	"stdafx.h"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <io.h>

#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include "Scene.h"
//#include "Primitive.h"


using namespace std;

const float imagePlaneWidth = 4; //8 units in world space
const float imagePlaneHeight = 3;

int count = 0;
/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value

	float c = cos(degree * PI/180);
	float s = sin(degree * PI/180);

	GzMatrix rotXMat = 
	{
		1.0,	0.0,	0.0,	0.0,
		0.0,	  c,	 -s,    0.0,
		0.0,	  s,	  c,	0.0,
		0.0,	0.0,	0.0,	1.0
	};

	memcpy(mat,rotXMat,sizeof(GzMatrix));


	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value

	float c = cos(degree * PI/180);
	float s = sin(degree * PI/180);

	GzMatrix rotYMat = 
	{
		  c,		0.0,		s,		0.0,
		0.0,		1.0,	  0.0,		0.0,
		 -s,		0.0,		c,		0.0,
		0.0,		0.0,	  0.0,		1.0
	};


	memcpy(mat,rotYMat,sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value

	float c = cos(degree * PI/180);
	float s = sin(degree * PI/180);

	GzMatrix rotZMat = 
	{
		  c,		-s,		0.0,	0.0,
		  s,		 c,		0.0,	0.0,
		0.0,	   0.0,		1.0,	0.0,
		0.0,	   0.0,	    0.0,	1.0
	};

	memcpy(mat,rotZMat,sizeof(GzMatrix));


	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value

	GzMatrix Transmat = 
	{
		1.0,	0.0,	0.0,	translate[X],
		0.0,	1.0,	0.0,	translate[Y],
		0.0,	0.0,	1.0,	translate[Z],
		0.0,	0.0,	0.0,	1.0
	};

	memcpy(mat,Transmat,sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value

	GzMatrix scaleMat = 
	{
		scale[X],			   0.0,			 0.0,		0.0,
			 0.0,		  scale[Y],			 0.0,		0.0,
			 0.0,			   0.0,		scale[Z],		0.0,
			 0.0,			   0.0,		     0.0,		1.0	
	};

	memcpy(mat,scaleMat,sizeof(GzMatrix));

	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 

	if(!display)
		return GZ_FAILURE;

	(*render) = new GzRender();		

	(*render)->renderClass = renderClass;   //make sure class = GZ_Z_BUFFER_RENDER

	(*render)->display = display;
	
	(*render)->open = 0; //the renderer is kept closed
	(*render)->matlevel = 0; //initialize top of stack as 0
	(*render)->normmatlevel = 0; //initialize top of normal stack as 0
	(*render)->modworldtop = 0;
	(*render)->worldmodtop = 0;
	(*render)->dirinversetop = 0;

	for(int i = 0; i < 3; i++)
		(*render)->flatcolor[i] = 0.0;

	//init default camera
	(*render)->camera.position[X] = DEFAULT_IM_X;
	(*render)->camera.position[Y] = DEFAULT_IM_Y;
	(*render)->camera.position[Z] = DEFAULT_IM_Z;

	(*render)->camera.lookat[X] = 0.0;
	(*render)->camera.lookat[Y] = 0.0;
	(*render)->camera.lookat[Z] = -1.0;

	(*render)->camera.worldup[X] = 0.0;
	(*render)->camera.worldup[Y] = 1.0;
	(*render)->camera.worldup[Z] = 0.0;

	(*render)->camera.FOV = DEFAULT_FOV;

	
	//initially there is no texture map function available - so initialize the pointer to function as NULL
	(*render)->tex_fun = NULL;

	//for now initialize all matrices to identity matrix
	LoadIdentity(&(*render)->camera.Xiw); 
	LoadIdentity(&(*render)->camera.Xpi); 
	LoadIdentity(&(*render)->Xsp);


	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	render->display = NULL;
    delete render;

	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 

	 GzInitDisplay(render->display);  
	
	 //at the moment using default camera values
	 //compute Xpi (perspective projection matrix)
	
	 float d = 1/tan((render->camera.FOV / 2) * PI/180.0f);

	 GzMatrix Xpitemp = 
	 {
		 1.0,		0.0,		0.0,		0.0,
		 0.0,		1.0,		0.0,		0.0,
		 0.0,		0.0,		1.0,		0.0,
		 0.0,		0.0,	  1.0/d,		1.0
	 };

	 memcpy(&(render->camera.Xpi),Xpitemp,sizeof(GzMatrix));

	 //compute Xiw
	 ComputeXiw(&(render->camera));

	 //initialize Xsp to its proper value

	 float xs = render->display->xres;
	 float ys = render->display->yres;
	 float zmax = INT_MAX;

	 GzMatrix Xsptemp = 
	 {
		 xs/2.0,		  0.0,		   0.0,		xs/2,
		    0.0,		-ys/2,		   0.0,		ys/2,
			0.0,		  0.0,		zmax/d,		 0.0,
		    0.0,		  0.0,		   0.0,		 1.0
		 
	 };

	 memcpy(&(render->Xsp),&Xsptemp,sizeof(GzMatrix));
	 
	 //now push Xsp, Xpi and Xiw on to the Ximage matrix stack
	  // GzPushMatrix(render,render->Xsp);
	  // GzPushMatrix(render,render->camera.Xpi);
	  //GzPushMatrix(render,render->camera.Xiw);
	
	   render->open = 1;  /// open the renderer

	
	return GZ_SUCCESS;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/

	memcpy(&(render->camera),camera,sizeof(GzCamera));

	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/

	if(render->matlevel == MATLEVELS)
	{
		//stack overflow
		AfxMessageBox("Matrix Stack Overflow",MB_OK);
		//PostQuitMessage(1);
		return GZ_FAILURE;
	}
	
	
	if(render->matlevel == 0)
	{

		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
				{
					render->Ximage[render->matlevel][i][j] = matrix[i][j];
				}
		}

		render->matlevel++;
		
	}
	else
	///multiply the current matrix with matlevel - 1 matrix and then store the resultant
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				render->Ximage[render->matlevel][i][j] = 0;

				for(int k = 0; k < 4; k++)
					{
						render->Ximage[render->matlevel][i][j] += render->Ximage[render->matlevel-1][i][k] * matrix[k][j];
						//render->Ximage[render->matlevel][i][j] += render->Ximage[render->matlevel-1][k][j] * matrix[i][k];
					}
			}
		}
		render->matlevel++;	
	}
	// normals stack
	GzMatrix tempMat;
	memcpy(tempMat, matrix, sizeof(GzMatrix));
	if(isValidMatrix(render, tempMat))
	{
		if(render->normmatlevel == 0)
		{
			for(int m = 0; m < 4; m++)
			{
				for(int n = 0; n < 4; n++)
				{
					render->Xnorm[render->normmatlevel][m][n] = tempMat[m][n];
				}
			}
			//memcpy(render->Xnorm[render->matlevelimage], tempMat, sizeof(GzMatrix));  //safe
			render->normmatlevel++;
		}
		else
		{
			GzMatrix mat;
			for(int i = 0; i < 4; i++)
			{
				for(int j = 0; j < 4; j++)
				{
					//mat[i][j] = 0;
					render->Xnorm[render->normmatlevel][i][j] = 0;
					for(int k = 0; k < 4; k++)
						{
							render->Xnorm[render->normmatlevel][i][j] += render->Xnorm[render->normmatlevel-1][i][k] * tempMat[k][j];
								//render->Xnorm[render->normmatlevel][i][j] += render->Xnorm[render->normmatlevel-1][k][j] * tempMat[i][k];
						}
				}
			}

			render->normmatlevel++;
		}
	}

	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
//	GzMatrix poppedMatrix;

	if(render->matlevel == 0)
	{
		//stack underflow
		AfxMessageBox("Matrix Stack Underflow", MB_OK);
		//PostQuitMessage(1);
		return GZ_FAILURE;
	}

	render->matlevel = render->matlevel - 1;  //since not swapping, it doesnt effectively delete the matrix but still it wont be accessible
	
	if(render->normmatlevel == 0)
	{
		//stack underflow
		AfxMessageBox("Normal Matrix Stack Underflow", MB_OK);
		return GZ_FAILURE;
	}

	render->normmatlevel--;
	
	return GZ_SUCCESS;
}

int PushModelWorldMatrix(GzRender* render, GzMatrix matrix,int type)
{
	
	if(render->modworldtop == MATLEVELS)
	{
		//stack overflow
		AfxMessageBox("Matrix Stack Overflow",MB_OK);
		//PostQuitMessage(1);
		return GZ_FAILURE;
	}	

	//ModelWorld stack
	if(render->modworldtop == 0)
		{
			memcpy(render->XModelWorld[render->modworldtop],matrix,sizeof(GzMatrix));
			render->modworldtop++;
		}

	else 
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				render->XModelWorld[render->modworldtop][i][j] = 0;

				for(int k = 0; k < 4; k++)
					render->XModelWorld[render->modworldtop][i][j] += render->XModelWorld[render->modworldtop-1][i][k] * matrix[k][j];
			}
		}
		render->modworldtop++;	
	}

	//inverse matrix stack
	//based on type convert to inverse and then add/multiply
	GzMatrix matInverse;
	if(type == 0) //scale matrix Sinv = 1/S;
	{
		memcpy(matInverse,matrix,sizeof(GzMatrix));

		matInverse[0][0] = (1 / matrix[0][0]);
		matInverse[1][1] = (1 / matrix[1][1]);
		matInverse[2][2] = (1 / matrix[2][2]);
	}
	else if(type == 1) //pure rotation matrix Rinv = Rtranspose
	{
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				matInverse[i][j] = matrix[j][i];
			}
		}
	}
	else if(type == 2) //translation matrix Tinv = -T
	{
		memcpy(matInverse,matrix,sizeof(GzMatrix));

		matInverse[0][3] = (-1.0) * matrix[0][3];
		matInverse[1][2] = (-1.0) * matrix[1][3];
		matInverse[2][3] = (-1.0) * matrix[2][3];
	}

	if(render->worldmodtop == 0)
	{
		memcpy(render->XinverseWorldModel[render->worldmodtop],matInverse,sizeof(GzMatrix));
		render->worldmodtop++;
	}
	else
	{
		for(int i = 0 ; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				render->XinverseWorldModel[render->worldmodtop][i][j] = 0;

				for(int k = 0; k < 4; k++)
				{
					render->XinverseWorldModel[render->worldmodtop][i][j] += matInverse[i][k] * render->XinverseWorldModel[render->worldmodtop-1][k][j]; //multiplying in reverse order
				}
			}
		}

		render->worldmodtop++;
	}

	//now the inverse stack for the ray direction Xform
	//for the time being don't consider inverse (to be used with normals)
	if(type == 1) // no translations allowed or scaling as well
	{

		if(render->dirinversetop == 0)
		{
			memcpy(render->XinverseDirXformMat[render->dirinversetop],matrix,sizeof(GzMatrix));
			render->dirinversetop++;
		}
		else
		{
			for(int i = 0 ; i < 4; i++)
			{
				for(int j = 0; j < 4; j++)
				{
					render->XinverseDirXformMat[render->dirinversetop][i][j] = 0;

					for(int k = 0; k < 4; k++)
					{
						render->XinverseDirXformMat[render->dirinversetop][i][j] += render->XinverseDirXformMat[render->dirinversetop-1][i][k] * matrix[k][j] ; 
					}
				}
			}

			render->dirinversetop++;
		}

	}
	return GZ_SUCCESS;
}

int PopModelWorldMatrix(GzRender *render)
{
	if(render->modworldtop == 0)
	{
		//stack underflow
		AfxMessageBox("Model World Stack Underflow", MB_OK);
		//PostQuitMessage(1);
		return GZ_FAILURE;
	}

	render->modworldtop = render->modworldtop - 1;

	if(render->worldmodtop == 0)
	{
		AfxMessageBox("Inverse Matrix Stack Underflow", MB_OK);
		return GZ_FAILURE;
	}

	render->worldmodtop = render->worldmodtop - 1;
	
	render->dirinversetop--;
	return GZ_SUCCESS;
}

bool isValidMatrix(GzRender* render, GzMatrix matrix)
{
	bool flag = true;

	//first check if matrix has only rotation - Rx, Ry or Rz
	if(matrix[0][0] == 1)
	{
		if(matrix[1][2] == -matrix[2][1])
			flag &= true;
		else
			flag &= false;
			
	}

	if(matrix[1][1] == 1)
	{
		if(matrix[0][2] == -matrix[2][0])
			flag &= true;
		else
			flag &= false;
			
	}

	if(matrix[2][2] == 1)
	{
		if(matrix[1][0] == -matrix[0][1])
			flag &= true;
		else
			flag &= false;

	}

	//now check if no translation, scaling gets covered
	if(matrix[0][3] == 0)  
		flag &= true;
	else
		flag &= false;
	if(matrix[1][3] == 0)
		flag &= true;
	else 
		flag &= false;
	if(matrix[2][3] == 0)
		flag &= true;
	else
		flag &= false;

	if(matrix[3][2] == 0) // no perspective correction
		flag &= true;
	else
		flag &= false;
		

	//if flag still remains false, means it HAS to be Xiw and can b no other matrix
	if(!flag)
	{
		for(int i = 0; i < 4; i++)
			for(int  j = 0; j < 4; j++)
			{
				if(matrix[i][j] != render->camera.Xiw[i][j])
					return false;
			}

			// if it remains true, remove translations from Xiw and then normalize Xiw so that we can push a normalized Xiw
			matrix[0][3] = 0.0f;
			matrix[1][3] = 0.0f;
			matrix[2][3] = 0.0f;
			matrix[3][3] = 1.0f;

			//normalize the matrix by normalizing the rows individually
			GzCoord rowVec;
			
			for(int i = 0; i < 3; i++)
			{
				memcpy(&rowVec,matrix[i],sizeof(GzCoord));
				Normalize(&rowVec);
				memcpy(matrix[i],&rowVec,sizeof(GzCoord));
			}
	}

	return true;
	
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	for(int i = 0; i < numAttributes; i++)
	{
		switch(nameList[i])
		{
			//flat shading
		case GZ_RGB_COLOR:
			memcpy(&(render->flatcolor), valueList[i],sizeof(GzColor));
			break;

			// light attributes
		case GZ_DIRECTIONAL_LIGHT:
			memcpy(&(render->lights[render->numlights]), valueList[i], sizeof(GzLight));  //store the directional lights in the lights array
			render->numlights++;
			break;

		case GZ_AMBIENT_LIGHT:
			memcpy(&(render->ambientlight), valueList[i], sizeof(GzLight));		//store the ambient light
			break;

			//shading attributes
		case GZ_DIFFUSE_COEFFICIENT:	
			memcpy(&(render->Kd), valueList[i], sizeof(GzColor));
			break;

		case GZ_AMBIENT_COEFFICIENT:
			memcpy(&(render->Ka), valueList[i], sizeof(GzColor));
			break;

		case GZ_SPECULAR_COEFFICIENT:
			memcpy(&(render->Ks), valueList[i], sizeof(GzColor));
			break;

		case GZ_DISTRIBUTION_COEFFICIENT:
			memcpy(&(render->spec), valueList[i], sizeof(float));
			break;

		case GZ_INTERPOLATE:
			memcpy(&(render->interp_mode), valueList[i], sizeof(int));
			break;

		case GZ_TEXTURE_MAP:
			//memcpy(render->tex_fun, valueList[i],sizeof(GzTexture));	//not sure of this statement
			render->tex_fun = (GzTexture)valueList[i];
			break;
		}
		
	}

	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList,		//is slightly different from HW2, so change it
				  GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts  
- Clip - just discard any triangle with verts behind view plane 
       - test for triangles with all three verts off-screen 
- invoke triangle rasterizer  
*/
	if(render->open == 0)
	{
		AfxMessageBox("Renderer not open!",MB_OK);
		return GZ_FAILURE;
	}

	GzCoord* pVertexList;  
	GzCoord* pNormalList;
	GzTextureIndex* puvList;

	for(int i = 0; i < numParts; i++)
	{
		if(nameList[i] == GZ_POSITION)
		{
			pVertexList = (GzCoord*)valueList[i];
		}

		if(nameList[i] == GZ_NORMAL)
			pNormalList = (GzCoord*)valueList[i];

		if(nameList[i] == GZ_TEXTURE_INDEX)
			puvList = (GzTextureIndex*)valueList[i];
	}

	//call the rasterizer analyser
	ScanLine(render, pVertexList, pNormalList,puvList);

	return GZ_SUCCESS;
}

int InterpolateZ(GzCoord vertexA,GzCoord* normal,int x,int y)
{
	int z; float D;

	//to find D in the plane equation at a vertexA and substitute it in the general plane equation
	D = ((*normal)[0]*vertexA[0] + (*normal)[1]*vertexA[1] + (*normal)[2]*vertexA[2]);

	//now find the z value at the given pixel
	z = (int) ((D - (*normal)[0]*x - (*normal)[1]*y) / (*normal)[2]);

	return z;

}

void  GetDirectionNormals(GzCoord vertexA,GzCoord vertexB,GzCoord vertexC,GzCoord* normal)
{
	GzCoord edgeA, edgeB;

	edgeA[0] = vertexB[0] - vertexA[0]; // i component  
	edgeA[1] = vertexB[1] - vertexA[1]; // j component
	edgeA[2] = vertexB[2] - vertexA[2]; // k component

	edgeB[0] = vertexC[0] - vertexB[0]; // i component  
	edgeB[1] = vertexC[1] - vertexB[1]; // j component
	edgeB[2] = vertexC[2] - vertexB[2]; // k component

	//now that we got the 2 edges, do their cross product  because r.n = a is the vector equation of the plane
	crossProduct(edgeA,edgeB,normal);
/*	(*normal)[0] = (edgeA[1] * edgeB[2]) - (edgeA[2] * edgeB[1]);
	(*normal)[1] = (edgeA[2] * edgeB[0]) - (edgeA[0] * edgeB[2]);
	(*normal)[2] = (edgeA[0] * edgeB[1]) - (edgeA[1] * edgeB[0]);
	*/
}

int ScanLineRaster(GzRender* render,GzCoord* pVertex, float xmin,float xmax,float zmin,float zmax,GzCoord* normal,GzTextureIndex* uvList,GzColor* specList,GzColor* diffList)	//for Phong the last 2 terms are NULL
{
	float m1,m2,dx1,dx2,dy1,dy2;
	GzCoord zVal, vN4;
	GzColor col, ambient, diffuse, specular;
	GzCoord texCoordList[3];
	GzCoord texCoord;
	
		//choose the 1st 2 edges involving the 1st vertex (least y value) - this is where the scan line traversal will start from
		dx1 =  pVertex[1][0] - pVertex[0][0];
		dy1 =  pVertex[1][1] - pVertex[0][1];
		if(dx1 != 0)
			m1 = dy1 / dx1;
		else
			m1 = INT_MAX; //assign some high value as infinity for vertical line

		dx2 =  pVertex[2][0] - pVertex[0][0];
		dy2 =  pVertex[2][1] - pVertex[0][1];
		if(dx2 != 0)
			m2 = dy2 / dx2;
		else
			m2 = INT_MAX;

		// traverse through the scan lines until we reach the 2nd (or middle) vertex  - USE int values, don't use float cuz then it will behave weirdly
		for(int y = (int)pVertex[0][1]+1; y <= (int)pVertex[1][1]; y++)
		{
			//since we are traversing top-bottom check if y > pVertex[0][1]  cuz it will causing jagged and pointed edges in some triangles
		 if((float)y >= pVertex[0][1])
		 {
			float xLeft,xRight; // this is were the scanline will intersect the 2 given edges

			//before assigning the x-values, check if the lines are horizontal or vertical
			if(dx1 != 0) //2 point line equation
				{
					xLeft = pVertex[0][0] + (((float)y - pVertex[0][1]) / m1);
			    }
			else //horizontal line or vertical line
				xLeft = pVertex[0][0];

			if(dx2 != 0)
				{
					xRight = pVertex[0][0] + (((float)y - pVertex[0][1]) / m2);
			    }
			else
				xRight = pVertex[0][0];

			if(xLeft > xRight) // swap the values - dont use XOR because the type is float
			{
				xLeft += xRight;
				xRight = xLeft - xRight;
				xLeft -= xRight;
			}

			//check with the x boundary
			if(xLeft < xmin)
				xLeft = xmin;

			if(xRight > xmax)
				xRight = xmax;

			//now interpolate the color values at xleft,y and xright,y  
			
			//now that we have the 2 x values, we can interpolate the color values in the region as well as interpolate z
			for(int x = (int)xLeft+1; x <= (int)xRight; x++)
			{
				//we have the x,y value so now compute z and check for Z-Buffer before setting the pixel may not work since we have to do it barycentricly
				//int z = InterpolateZ(pVertex[0],normal,x,y); 
				Interpolation(x, y, pVertex, pVertex, &zVal);	// we pass vertex in the normalList argument only for z-buffer interpolation

				float z = zVal[2]; 

				//check with the z boundary
				if(z >= zmax)
					z = zmax;
				
				if(z <= zmin)
					z = zmin;

				//now that we have Zscreen, we have to interpolate U,V coordinates and unwarp them back 
				//we can use the same interpolation equation for u,v - put the 3rd coordinate as 0 since we r in 2D space now
				for(int i = 0; i < 3; i++)
				{
					texCoordList[i][U] = uvList[i][U];
					texCoordList[i][V] = uvList[i][V];
					texCoordList[i][Z] = 0;  //because we r in 2D Space
				}
				//now pass this list to the interpolation function to get interpolated U,V and then unwarp them
				Interpolation(x, y, pVertex, texCoordList, &texCoord); 

				//now unwarp texCoord (U,V) to get back u,v at each pixel
				float zPrime;
					zPrime = z / (INT_MAX - z);
					texCoord[U] = texCoord[U] * (zPrime + 1);
					texCoord[V] = texCoord[V] * (zPrime + 1);

					//now scale this texCoord value by texture size
					//for Phong shading we need to set Kd and Ka to texture color
					(*render->tex_fun)((float)texCoord[U],(float)texCoord[V],render->Kd);
					(*render->tex_fun)((float)texCoord[U], (float)texCoord[V], render->Ka);
					//for Gouraud shading we need to set Ks also to texture color
					if(render->interp_mode == GZ_COLOR)
						(*render->tex_fun)((float)texCoord[U], (float)texCoord[V], render->Ks);

				//now based on the shading mode, do normal or color interpolation
				//Color Interpolation for Gouraud
			 if(render->interp_mode == GZ_NORMALS)  //Phong Shading
			 {
				Interpolation(x, y, pVertex, normal, &vN4);
				Normalize(&vN4);
				VertexShader(render,vN4,&specular,&diffuse);
				MultiplyColor(render,specular,diffuse,&col);
			 }
			 else if(render->interp_mode == GZ_COLOR)	//Gouraud Shading
			 {
				 //interpolate  diffuse, specular - dont have to interpolate ambient because it is the same everywhere
				 Interpolation(x,y,pVertex, specList, &specular);
				 Interpolation(x,y,pVertex, diffList, &diffuse);
				 MultiplyColor(render,specular,diffuse,&col);
				 //memcpy(&col, &vN4, sizeof(GzColor));
			 }
				
			else //Flat Shading
			{
					memcpy(&col, &render->flatcolor, sizeof(GzColor));				
			}

				//now set the pixel
				GzIntensity r,g,b;

					r = ctoi(col[RED]);
					g = ctoi(col[GREEN]);
					b = ctoi(col[BLUE]);

					PutPixel(render->display,x,y,z,r,g,b);
				
			}
		 }
		}  //end-for loop til middle vertex

		// now we have finished filling the pixels til the middle vertex; now from middle vertex to the final vertex

		dx1 =  pVertex[2][0] - pVertex[1][0];
		dy1 =  pVertex[2][1] - pVertex[1][1];
		if(dx1 != 0)
			m1 = dy1 / dx1;
		else
			m1 = INT_MAX;

		//other line remains the same as it still continues
		//continue traversing
		for(int y = (int)pVertex[1][1]+1; y <= (int)pVertex[2][1]; y++)
		{
		
			if((float)y >= pVertex[1][1])
			{

				float xLeft,xRight; 

				//before assigning the x-values, check if the lines are horizontal or vertical
				if(dx1 != 0)
					{
						xLeft = pVertex[1][0] + (((float)y - pVertex[1][1]) / m1);
					}
				else
					xLeft = pVertex[1][0];  

				if(dx2 != 0)
					{
						xRight = pVertex[0][0] + (((float)y - pVertex[0][1]) / m2);
					}
				else
					xRight = pVertex[0][0];
				

				if(xLeft > xRight) // swap the values - dont use XOR because the type is float
				{
					xLeft += xRight;
					xRight = xLeft - xRight;
					xLeft -= xRight;
				}

				//check with the x boundary
				if(xLeft < xmin)
					xLeft = xmin;

				if(xRight > xmax)
					xRight = xmax;

				//now that we have the 2 x values, we can fill the region between the 2 lines and interpolate the color values as well
				for(int x = (int)xLeft+1; x <= (int)xRight; x++)
				{
					//we have the x,y value so now compute z and check for Z-Buffer before setting the pixel
				//	int z = InterpolateZ(pVertex[0],normal,x,y);

					Interpolation(x, y, pVertex, pVertex, &zVal);
					float z = zVal[2]; 
					//check with the z boundary
					if(z >= zmax)
						z = zmax;

					if(z <= zmin)
					z = zmin;

					for(int i = 0; i < 3; i++)
					{
						texCoordList[i][U] = uvList[i][U];
						texCoordList[i][V] = uvList[i][V];
						texCoordList[i][Z] = 0;  //because we r in 2D Space
					}
				//now pass this list to the interpolation function to get interpolated U,V and then unwarp them
				Interpolation(x, y, pVertex, texCoordList, &texCoord); 

				//now unwarp texCoord (U,V) to get back u,v at each pixel
				float zPrime;
					zPrime = z / (INT_MAX - z);
					texCoord[U] = texCoord[U] * (zPrime + 1);
					texCoord[V] = texCoord[V] * (zPrime + 1);

					//now scale this texCoord value by texture size
					//for Phong shading we need to set Kd and Ka to texture color
					(*render->tex_fun)((float)texCoord[U],(float)texCoord[V],render->Kd);
					(*render->tex_fun)((float)texCoord[U], (float)texCoord[V], render->Ka);
					//for Gouraud shading we need to set Ks also to texture color
					if(render->interp_mode == GZ_COLOR)
						(*render->tex_fun)((float)texCoord[U], (float)texCoord[V], render->Ks);

					//first check the Shading Mode
					
				if(render->interp_mode == GZ_NORMALS)		//Phong Shading
				{
					Interpolation(x, y, pVertex, normal, &vN4);
					Normalize(&vN4);
					VertexShader(render,vN4,&specular,&diffuse);
					MultiplyColor(render,specular,diffuse,&col);
				}
				else if(render->interp_mode == GZ_COLOR) //Gouraud Shading
				{
					Interpolation(x,y,pVertex, specList, &specular);
					Interpolation(x,y,pVertex, diffList, &diffuse);
					MultiplyColor(render,specular,diffuse,&col);					
				}
				else //Flat Shading
				{
					memcpy(&col, &render->flatcolor, sizeof(GzColor));				
				}

					GzIntensity r,g,b;

					r = ctoi(col[RED]);
					g = ctoi(col[GREEN]);
					b = ctoi(col[BLUE]);

						//now set the pixel
						PutPixel(render->display,x,y,z,r,g,b);						
     
				}
			} 
		} 
	return GZ_SUCCESS;
}


void Interpolation(float x, float y, GzCoord* pVertex, GzCoord* normalList, GzCoord* zVal)
{
	GzCoord curPoint = {x,y,0};
	//calculating barycentric coordinates in terms of Areas..so store A1,A2,A3 and A in an array
	float Areas[4];
	Areas[0] = GetTriangleArea(pVertex[0],pVertex[1],pVertex[2]); // Areas[0] stores the entire area

	for(int i = 0; i < 3; i++) //now find the areas of PAB, PBC, and PAC triangles  - the areas / area[0] will give the barycentric coordinates
	{
		Areas[i+1] = GetTriangleArea(curPoint,pVertex[(i+1)%3],pVertex[(i+2)%3]) / Areas[0];
	}

	//then the interpolated coordinates are given by A1x1 + A2x2 + A3x3
	//for zinterpolation pNormal argument stores the pVertex
	(*zVal)[X] = normalList[0][0] * Areas[1] + normalList[1][0] * Areas[2] + normalList[2][0] * Areas[3]; //because A[0] stores total area
	(*zVal)[Y] = normalList[0][1] * Areas[1] + normalList[1][1] * Areas[2] + normalList[2][1] * Areas[3];
	(*zVal)[Z] = normalList[0][2] * Areas[1] + normalList[1][2] * Areas[2] + normalList[2][2] * Areas[3];
	
}

void NormalInterpolation(GzCoord point, GzCoord* pVertex, GzCoord* normal, GzCoord* resultant)
{

	for(int i = 0; i < 3; i++)
		Normalize(&normal[i]);

	float Areas[4];
	Areas[0] = GetTriangleArea(pVertex[0],pVertex[1],pVertex[2]);

	for(int i = 0; i < 3; i++) 
	{
		Areas[i+1] = GetTriangleArea(point,pVertex[(i+1)%3],pVertex[(i+2)%3]) / Areas[0];
	}

	//then the interpolated coordinates are given by A1x1 + A2x2 + A3x3
	(*resultant)[X] = normal[0][0] * Areas[1] + normal[1][0] * Areas[2] + normal[2][0] * Areas[3];
	(*resultant)[Y] = normal[0][1] * Areas[1] + normal[1][1] * Areas[2] + normal[2][1] * Areas[3];
	(*resultant)[Z] = normal[0][2] * Areas[1] + normal[1][2] * Areas[2] + normal[2][2] * Areas[3];

}
bool BarycentricInterpolationTest(GzCoord point, GzCoord* pVertexList, GzCoord normal)
{
	//we have to discard either x,y or z depending on which has higher abs value in the normal
	GzCoord u,v; //edges

	if(fabs(normal[X]) > fabs(normal[Y]))
	{
		if(fabs(normal[X]) > fabs(normal[Z]))
		{
			//discard x
			u[0] = point[Y] - pVertexList[0][Y];
			u[1] = pVertexList[1][Y] - pVertexList[0][Y];
			u[2] = pVertexList[2][Y] - pVertexList[0][Y];

			v[0] = point[Z] - pVertexList[0][Z];
			v[1] = pVertexList[1][Z] - pVertexList[0][Z];
			v[2] = pVertexList[2][Z] - pVertexList[0][Z];

		}

		else
		{
			//discard z
			u[0] = point[X] - pVertexList[0][X];
			u[1] = pVertexList[1][X] - pVertexList[0][X];
			u[2] = pVertexList[2][X] - pVertexList[0][X];

			v[0] = point[Y] - pVertexList[0][Y];
			v[1] = pVertexList[1][Y] - pVertexList[0][Y];
			v[2] = pVertexList[2][Y] - pVertexList[0][Y];
		}
	}
	else 
	{
		if(fabs(normal[Y]) > fabs(normal[Z]))
		{
			//discard Y
			u[0] = point[X] - pVertexList[0][X];
			u[1] = pVertexList[1][X] - pVertexList[0][X];
			u[2] = pVertexList[2][X] - pVertexList[0][X];

			v[0] = point[Z] - pVertexList[0][Z];
			v[1] = pVertexList[1][Z] - pVertexList[0][Z];
			v[2] = pVertexList[2][Z] - pVertexList[0][Z];
		}
		else
		{
			//discard Z
			u[0] = point[X] - pVertexList[0][X];
			u[1] = pVertexList[1][X] - pVertexList[0][X];
			u[2] = pVertexList[2][X] - pVertexList[0][X];

			v[0] = point[Y] - pVertexList[0][Y];
			v[1] = pVertexList[1][Y] - pVertexList[0][Y];
			v[2] = pVertexList[2][Y] - pVertexList[0][Y];
		}
	}

	//now before computing the coordinates compute the denominator of the equation
	//if it is zero return false
	float denom = (u[Y] * v[Z]) - (v[Y] * u[Z]);
	
	if(denom == 0)
	{
		return false;
	}
	//else compute the barycentric coordinates
	denom = 1.0f / denom;

	float alpha = ((u[X] * v[Z]) - (v[Z] * u[Z])) * denom;
	if(!(alpha >= 0.0f))  //actually we might have to check if <= 1.0f
		return false;
	
	float beta = ((u[Y] * v[X]) - (v[Y] * u[X])) * denom;
	if(!(beta >= 0.0f))
		return false;

	float gamma = 1.0f - alpha - beta;
	if(!(gamma >= 0.0f))
		return false;

	return true;
}

float GetTriangleArea(GzCoord a, GzCoord b, GzCoord c)
{
	float area;
	GzCoord crossProd;
	GzCoord v1 = {a[0],a[1],1};
	GzCoord v2 = {b[0], b[1], 1};
	GzCoord v3 = {c[0], c[1], 1};
	GzCoord edgeA, edgeB;

	edgeA[X] = v2[0] - v1[0];
	edgeA[Y] = v2[1] - v1[1];
	edgeA[Z] = v2[2] - v1[2];

	edgeB[X] = v3[0] - v1[0];
	edgeB[Y] = v3[1] - v1[1];
	edgeB[Z] = v3[2] - v1[2];

	crossProd[X] = (edgeA[1] * edgeB[2]) - (edgeA[2] * edgeB[1]);
	crossProd[Y] = (edgeA[2] * edgeB[0]) - (edgeA[0] * edgeB[2]);
	crossProd[Z] = (edgeA[0] * edgeB[1]) - (edgeA[1] * edgeB[0]);

	//now 1/2 * magnitude of crossProd = Area of triangle
	area = 0.5 * sqrt(crossProd[X] * crossProd[X] + crossProd[Y] * crossProd[Y] + crossProd[Z] * crossProd[Z]);
	
	return area;
}

void LoadIdentity(GzMatrix* pMatrix)
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(i == j)
				(*pMatrix)[i][j] = 1;
			else
				(*pMatrix)[i][j] = 0;
		}
	}
}

void Normalize(GzCoord* colVector)
{
	float sum = pow((*colVector)[X],2) + pow((*colVector)[Y],2) + pow((*colVector)[Z],2);

	sum = sqrt(sum);

	for(int i = 0; i < 3; i++)
		(*colVector)[i] /= sum;
} 


void ComputeXiw(GzCamera* camera)
{
	 //1st we need to get he camera's 3 axes

		 //1st we need to get he camera's 3 axes
	 //declare 3 column vectors
	 GzCoord camZaxis, camYaxis, camXaxis;

	 for(int i = 0; i < 3; i++)
		 camZaxis[i] = camera->lookat[i] - camera->position[i];   //position vector of b - position vector of a if coming inverted reverse it

	 Normalize(&camZaxis);

	  //now compute camYaxis using the up vector
	 float dotProd = 0;
	 for(int i = 0; i < 3; i++)
		 dotProd += (camera->worldup[i] * camZaxis[i]);	 

	 camYaxis[X] = camera->worldup[X] - (dotProd * camZaxis[X]);
	 camYaxis[Y] = camera->worldup[Y] - (dotProd * camZaxis[Y]);
	 camYaxis[Z] = camera->worldup[Z] - (dotProd * camZaxis[Z]);

	 Normalize(&camYaxis);

	 //now find X = Y x Z

	 camXaxis[X] = (camYaxis[Y] * camZaxis[Z]) - (camYaxis[Z] * camZaxis[Y]);
	 camXaxis[Y] = (camYaxis[Z] * camZaxis[X]) - (camYaxis[X] * camZaxis[Z]);
	 camXaxis[Z] = (camYaxis[X] * camZaxis[Y]) - (camYaxis[Y] * camZaxis[X]);
	 
	 float xC = 0.0;
	 float yC = 0.0;
	 float zC = 0.0;

	 for(int i = 0; i < 3; i++)
	 {
		 xC += camXaxis[i] * camera->position[i];
		 yC += camYaxis[i] * camera->position[i];
		 zC += camZaxis[i] * camera->position[i];
	 }

	GzMatrix Xiwtemp = 
	{
		camXaxis[X],	camXaxis[Y],	camXaxis[Z],	-xC,
		camYaxis[X],	camYaxis[Y],	camYaxis[Z],	-yC,
		camZaxis[X],    camZaxis[Y],    camZaxis[Z],	-zC,
				0.0,			0.0,			0.0,	1.0
	};

	memcpy(&(camera->Xiw),&Xiwtemp,sizeof(GzMatrix));

	//also calculate its inverse but remove camera->Position (to be used to transform light direction)
	GzMatrix Xwitemp = 
	{
		camXaxis[X],	camYaxis[X],	camZaxis[X],	0.0,
		camXaxis[Y],	camYaxis[Y],	camZaxis[Y],	0.0,
		camXaxis[Z],	camYaxis[Z],	camZaxis[Z],	0.0,
				0.0,			0.0,			0.0,	1.0
	};

	//also before copying normalize it
	GzCoord rowVec;
		for(int i = 0; i < 3; i++)
		{
			memcpy(&rowVec,Xwitemp[i],sizeof(GzCoord));
			Normalize(&rowVec);
			memcpy(Xwitemp[i],&rowVec,sizeof(GzCoord));
		}
	
	memcpy(&(camera->Xwi),&Xwitemp,sizeof(GzMatrix));


}

void VertexTransformation(GzCoord pVertex, GzMatrix xformMat,GzCoord* resultant)
{  
	//convert the 3d vertex to homogeneous system by initially adding 1 as the 4th element
	float w = 1;
	GzCoord tempVert;

	w = xformMat[3][0] * pVertex[X] + xformMat[3][1] * pVertex[Y] + xformMat[3][2] * pVertex[Z] + xformMat[3][3];  // * 1
	tempVert[X] = xformMat[0][0] * pVertex[X] + xformMat[0][1] * pVertex[Y] + xformMat[0][2] * pVertex[Z] + xformMat[0][3];
	tempVert[Y] = xformMat[1][0] * pVertex[X] + xformMat[1][1] * pVertex[Y] + xformMat[1][2] * pVertex[Z] + xformMat[1][3];
	tempVert[Z] = xformMat[2][0] * pVertex[X] + xformMat[2][1] * pVertex[Y] + xformMat[2][2] * pVertex[Z] + xformMat[2][3];

	if(w == 0)
		w = 1;

	///now homogenize

	tempVert[X] = tempVert[X] / w;
	tempVert[Y] = tempVert[Y] / w;
	tempVert[Z] = tempVert[Z] / w;

	memcpy(resultant,&tempVert,sizeof(GzCoord));
}

void NormalTransformation(GzCoord pNormal, GzMatrix xformMat,GzCoord* resultant)
{
	float w = 1;
	GzCoord tempNorm;

	w = xformMat[3][0] * pNormal[X] + xformMat[3][1] * pNormal[Y] + xformMat[3][2] * pNormal[Z] + xformMat[3][3];
	tempNorm[X] = xformMat[0][0] * pNormal[X] + xformMat[0][1] * pNormal[Y] + xformMat[0][2] * pNormal[Z] + xformMat[0][3];
	tempNorm[Y] = xformMat[1][0] * pNormal[X] + xformMat[1][1] * pNormal[Y] + xformMat[1][2] * pNormal[Z] + xformMat[1][3];
	tempNorm[Z] = xformMat[2][0] * pNormal[X] + xformMat[2][1] * pNormal[Y] + xformMat[2][2] * pNormal[Z] + xformMat[2][3];

	memcpy(resultant,&tempNorm,sizeof(GzCoord));

	//we dont have to homogenize the normals cuz they already are
}

void ImageWorldXform(GzRender* render,GzCoord *pLightDir)
{
	//first get the resultant inverse matrix
	GzMatrix tempMat;
	GzMatrix inverseMat;
	memcpy(&inverseMat,&render->XinverseDirXformMat[render->dirinversetop-1],sizeof(GzMatrix));

	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			tempMat[i][j] = 0;

			for(int k = 0; k < 4; k++)
			{
				tempMat[i][j] += inverseMat[i][k] * render->camera.Xwi[k][j];
			}
		}
	}
	GzCoord lightDir;
	memcpy(&lightDir,pLightDir,sizeof(GzCoord));

	NormalTransformation(lightDir,tempMat,pLightDir);
}

//Xform vertices and normals to World Space (image space for now)
vector<SceneMesh> XformPrimitives(GzRender* render, vector<SceneMesh*>pScene)
{
	vector<SceneMesh> pXformedScene;

	for(int i = 0; i < pScene.size(); i++)
	{
		GzCoord tempVertex[3], tempNormal[3];
		SceneMesh tempScene;
		for(int j = 0; j < 3; j++)
		{
			//VertexTransformation(pScene.at(i)->vertexList[j], render->XModelWorld[render->modworldtop-1], &tempVertex[j]);			
			VertexTransformation(pScene.at(i)->vertexList[j], render->Ximage[render->matlevel-1], &tempVertex[j]);
			//NormalTransformation(pScene.at(i)->normalList[j], render->XinverseDirXformMat[render->dirinversetop-1], &tempNormal[j]); //for now use this matrix and then switch to normal stack
			NormalTransformation(pScene.at(i)->normalList[j], render->Xnorm[render->normmatlevel-1], &tempNormal[j]);
		}

		if(i == 0)
			{
				printf("Before Xform, normal 0: %f, %f, %f \n\n", pScene.at(i)->normalList[0][X], pScene.at(i)->normalList[0][Y], pScene.at(i)->normalList[0][Z]);
				printf("After Xform,  normal 0: %f, %f, %f \n\n", tempNormal[0][X], tempNormal[0][Y], tempNormal[0][Z]);
			}

		memcpy(&(tempScene.vertexList),&tempVertex,sizeof(GzCoord) * 3);
		memcpy(tempScene.normalList,&tempNormal,sizeof(GzCoord) * 3);
		memcpy(tempScene.uvList,pScene.at(i)->uvList,sizeof(GzCoord) * 3);
		pXformedScene.push_back(tempScene);

	}

	return pXformedScene;
}

void FlatShader(GzRender *render, GzCoord* normalList)
{
	float colorCoeff = 0.0f;
	GzColor specTerm = {0,0,0};
	GzColor diffTerm = {0,0,0};

	Normalize(&normalList[0]);
	for(int i = 0; i < render->numlights; i++)
	{
		Normalize(&render->lights[i].direction);
	}

	GzCoord Evec = {0,0,-1}; //direction of camera in image space which is {0,0,-1}
	
	//apply lighting equation
	float nl,ne; // dot product of N & L, and N & E
	
	for(int i = 0; i < render->numlights; i++)
	{
		nl = dotProduct(normalList[0], render->lights[i].direction);
		ne = dotProduct(normalList[0], Evec);

		// check if we should include the light in the lighting model depending on its direction with the camera
		if((nl > 0 && ne < 0) || (nl < 0 && ne > 0)) //ignore the light
			continue;
		else if(nl < 0 && ne < 0) //invert the normal and calculate the lighting model
		{
			normalList[0][0] *= -1;
			normalList[0][1] *= -1;
			normalList[0][2] *= -1;

			nl *= -1;
		}

		// if it reaches here means compute the lighting model and also that both nl and ne >= 0
	
		GzCoord Rvec;
		//r = 2(n.l)N - L
		Rvec[X] = (2 * nl * normalList[0][0]) - render->lights[i].direction[X];
		Rvec[Y] = (2 * nl * normalList[0][1]) - render->lights[i].direction[Y];
		Rvec[Z] = (2 * nl * normalList[0][2]) - render->lights[i].direction[Z];

		Normalize(&Rvec);
		float re; //R.E
		re = dotProduct(Rvec,Evec);
		//clamp it to [0,1]
		if(re > 1.0) 
			re = 1.0f;
		else if(re < 0.0)
			re = 0.0f; 
		re = pow(re,render->spec);  //specular power
		specTerm[RED] += render->lights[i].color[RED] * re;		//multiply these terms by shadowTerm
		specTerm[GREEN] += render->lights[i].color[GREEN] * re;
		specTerm[BLUE] += render->lights[i].color[BLUE] * re;

		diffTerm[RED] += render->lights[i].color[RED] * nl;
		diffTerm[GREEN] += render->lights[i].color[GREEN] * nl;
		diffTerm[BLUE] += render->lights[i].color[BLUE] * nl;
	}

	// now multiply the respective terms by Ks or Kd and add the ambient light component, the resulting color is stored in render->flatcolor
	render->flatcolor[RED] = (render->Ks[RED] * specTerm[RED]) + (render->Kd[RED] * diffTerm[RED]) + (render->Ka[RED] * render->ambientlight.color[RED]);
	render->flatcolor[GREEN] = (render->Ks[GREEN] * specTerm[GREEN]) + (render->Kd[GREEN] * diffTerm[GREEN]) + (render->Ka[GREEN] * render->ambientlight.color[GREEN]);
	render->flatcolor[BLUE] = (render->Ks[BLUE] * specTerm[BLUE]) + (render->Kd[BLUE] * diffTerm[BLUE]) + (render->Ka[BLUE] * render->ambientlight.color[BLUE]);
		
		// now clamp the color values so that they don't excede 1.0
		if(render->flatcolor[0] > 1.0f)
			render->flatcolor[0] = 1.0f;

		if(render->flatcolor[1] > 1.0f)
			render->flatcolor[1] = 1.0f;

		if(render->flatcolor[2] > 1.0f)
			render->flatcolor[2] = 1.0f; 
}

float dotProduct(GzCoord vec1, GzCoord vec2)
{
	return ((vec1[X] * vec2[X]) + (vec1[Y] * vec2[Y]) + (vec1[Z] * vec2[Z]));
}

void crossProduct(GzCoord vec1, GzCoord vec2, GzCoord *result)
{
	GzCoord vec3;

	vec3[X] = (vec1[Y] * vec2[Z]) - (vec1[Z] * vec2[Y]);
	vec3[Y] = (vec1[Z] * vec2[X]) - (vec1[X] * vec2[Z]);
	vec3[Z] = (vec1[X] * vec2[Y]) - (vec1[Y] * vec2[X]);

	memcpy(result,vec3,sizeof(vec3));
}

void VertexShader(GzRender *render, GzCoord normalList, GzColor* specular, GzColor* diffuse)
{
	float colorCoeff = 0.0f;
	GzColor specTerm = {0,0,0};
	GzColor diffTerm = {0,0,0};
	GzCoord Rvec;
	GzCoord tempNorm = {normalList[X],normalList[Y],normalList[Z]};
	//GzColor resultant = {0,0,0};
	//normalize the vectors except R before entering this function

	GzCoord Evec = {0,0,-1}; //direction of camera in image space which is {0,0,-1}
	
	float nl,ne,re; // dot product of N & L,  N & E and R & E
	bool flag; //to detect if lighting model should be computed

	
			for(int j = 0; j < render->numlights; j++)
			{	
				flag = false;
				nl = dotProduct(tempNorm, render->lights[j].direction);
				ne = dotProduct(tempNorm, Evec);

				// check if we should include the light in the lighting model depending on its direction with the camera
				
				if(nl < 0 && ne < 0) //invert the normal and calculate the lighting model
				{
					tempNorm[X] *= -1;			
					tempNorm[Y] *= -1;
					tempNorm[Z] *= -1;
					Normalize(&tempNorm);
					nl = dotProduct(tempNorm, render->lights[j].direction);
					ne = dotProduct(tempNorm, Evec);
					//renormalize N and recompute nl and ne
					flag = true;
				}
				 else if(nl > 0 && ne > 0) // compute the lighting model
					 flag = true;
							
				if(flag)
				{
					//r = 2(n.l)N - L
					Rvec[X] = (2 * nl * tempNorm[X]) - render->lights[j].direction[X];
					Rvec[Y] = (2 * nl * tempNorm[Y]) - render->lights[j].direction[Y];
					Rvec[Z] = (2 * nl * tempNorm[Z]) - render->lights[j].direction[Z];

					Normalize(&Rvec);
					 
					re = dotProduct(Rvec,Evec);
					//clamp it to [0,1] 
				//	if(re > 1.0) 
				//		re = 1.0f;
				//	else if(re < 0.0)
					//	re = 0.0f;

					re = pow(re,render->spec);  //specular power

					specTerm[RED] += render->lights[j].color[RED] * re;			//clamp the colors individually as well
					specTerm[GREEN] += render->lights[j].color[GREEN] * re;
					specTerm[BLUE] += render->lights[j].color[BLUE] * re;
					
					diffTerm[RED] += render->lights[j].color[RED] * nl;
					diffTerm[GREEN] +=render->lights[j].color[GREEN] * nl;
					diffTerm[BLUE] += render->lights[j].color[BLUE] * nl;
				}

				//now reset the normal back for the next light
				memcpy(tempNorm,normalList,sizeof(GzCoord));
			}

			memcpy(specular, &specTerm, sizeof(GzColor));
			memcpy(diffuse,  &diffTerm, sizeof(GzColor));
}

void MultiplyColor(GzRender *render, GzColor specular, GzColor diffuse, GzColor* resultant)
{
	GzColor result = {0,0,0};

	result[RED]   =     render->Ka[RED] *   render->ambientlight.color[RED]	 + render->Ks[RED]    * specular[RED]    + render->Kd[RED]   * diffuse[RED];
	result[GREEN] =   render->Ka[GREEN] * render->ambientlight.color[GREEN]   + render->Ks[GREEN] * specular[GREEN]  + render->Kd[GREEN] * diffuse[GREEN];
	result[BLUE]  =    render->Ka[BLUE] *  render->ambientlight.color[BLUE]	 + render->Ks[BLUE]   * specular[BLUE]   + render->Kd[BLUE]  * diffuse[BLUE];
	
	if(result[RED] > 1.0f)
		result[RED] = 1.0f;

	if(result[RED] < 0.0f)
		result[RED ]= 0.0f;

	if(result[GREEN] > 1.0f)
		result[GREEN] = 1.0f;

	if(result[GREEN] < 0.0f)
		result[GREEN] = 0.0f;
			
	if(result[BLUE] > 1.0f)
		result[BLUE] = 1.0f;

	if(result[BLUE] < 0.0f)
		result[BLUE] = 0.0f;

	memcpy(resultant,result,sizeof(GzColor));

}

void PhongIllumination(GzRender *render, GzRay ray, GzCoord normal,GzColor *intensity)
{
	float colorCoeff = 0.0f;
	GzColor specTerm = {0,0,0};
	GzColor diffTerm = {0,0,0};
	GzCoord Rvec;
	GzCoord Evec;
	GzCoord tempNorm = {normal[X],normal[Y],normal[Z]};

	memcpy(&Evec,&ray.direction,sizeof(GzCoord));

	float nl,ne,re; // dot product of N & L,  N & E and R & E
	bool flag; //to detect if lighting model should be computed

	for(int j = 0; j < render->numlights; j++)
		{	
			flag = false;
			GzCoord LVec;
			memcpy(&LVec,&render->lights[j].direction,sizeof(GzCoord));
			//we have to do inverse Xform on the LVec to convert it from image to world space;
			NormalTransformation(LVec,render->camera.Xwi,&LVec);
			//ImageWorldXform(render,&LVec);

			nl = dotProduct(tempNorm, LVec);
			ne = dotProduct(tempNorm, Evec);

			// check if we should include the light in the lighting model depending on its direction with the camera
				
			if(nl < 0 && ne < 0) //invert the normal and calculate the lighting model
			{
				tempNorm[X] *= -1;			
				tempNorm[Y] *= -1;
				tempNorm[Z] *= -1;
				Normalize(&tempNorm);
				nl = dotProduct(tempNorm, LVec);
				ne = dotProduct(tempNorm, Evec);
				//renormalize N and recompute nl and ne
				flag = true;
			}
				else if(nl > 0 && ne > 0) // compute the lighting model
					flag = true;
							
			if(flag)
			{
				//r = 2(n.l)N - L 
				Rvec[X] = (2 * nl * tempNorm[X]) - LVec[X];
				Rvec[Y] = (2 * nl * tempNorm[Y]) - LVec[Y];
				Rvec[Z] = (2 * nl * tempNorm[Z]) - LVec[Z];

				Normalize(&Rvec);
					 
				re = dotProduct(Rvec,Evec);
				//clamp it to [0,1] 
			//	if(re > 1.0) 
			//		re = 1.0f;
			//	else if(re < 0.0)
				//	re = 0.0f;

				re = pow(re,render->spec);  //specular power

				specTerm[RED] += render->lights[j].color[RED] * re;			//clamp the colors individually as well
				specTerm[GREEN] += render->lights[j].color[GREEN] * re;
				specTerm[BLUE] += render->lights[j].color[BLUE] * re;
					
				diffTerm[RED] += render->lights[j].color[RED] * nl;
				diffTerm[GREEN] +=render->lights[j].color[GREEN] * nl;
				diffTerm[BLUE] += render->lights[j].color[BLUE] * nl;
			}

			//now reset the normal back for the next light
			memcpy(tempNorm,normal,sizeof(GzCoord));
		}
		GzColor specular, diffuse;
		memcpy(specular, &specTerm, sizeof(GzColor));
		memcpy(diffuse,  &diffTerm, sizeof(GzColor));

		MultiplyColor(render,specular,diffuse,intensity);
}

int ScanLine(GzRender *render, GzCoord* vertexList, GzCoord* normalList,GzTextureIndex* uvList)
{
	
	GzColor specular[3];
	GzColor diffuse [3];
	
	float zPrime;	//Vzscreen'

	for(int i = 0; i < 3; i++)
	{
		VertexTransformation(vertexList[i],render->Ximage[render->matlevel-1],&vertexList[i]);  //converting vertices to SCREEN SPACE
		NormalTransformation(normalList[i],render->Xnorm[render->normmatlevel-1],&normalList[i]); //converting normals to IMAGE SPACE
		//apply perspective to uv coordinates - i.e. convert them to UV
		zPrime = vertexList[i][Z] / (INT_MAX - vertexList[i][Z]);
		uvList[i][U] = uvList[i][U] / (zPrime + 1);
		uvList[i][V] = uvList[i][V] / (zPrime + 1);
	//	PerformPerspectiveCorrection(zPrime,&uvList[i]);
	}


	//do clipping - discard the triangles if all 3 vertices have z < 0
			if(vertexList[0][2] < 0 || vertexList[1][2] < 0 || vertexList[2][2] < 0)
				return GZ_SUCCESS;
			
			GzCoord tempVertex, tempNormal;
			GzTextureIndex tempuv;
				
			//implementing Scan Line Algorithm, sort the vertices in the increasing order of y - dont think about Z value NOW!

			//bubble sort the vertices according to the y-value
			for(int i = 0; i < 3; i++)
			{
				for(int j = i+1; j < 3; j++)
				{
					if(vertexList[j][1] < vertexList[i][1])
					{
						tempVertex[0] = vertexList[i][0];
						tempVertex[1] = vertexList[i][1];
						tempVertex[2] = vertexList[i][2];
						tempNormal[0] = normalList[i][0];
						tempNormal[1] = normalList[i][1];
						tempNormal[2] = normalList[i][2];
						tempuv[0]	  = uvList[i][0];
						tempuv[1]	  = uvList[i][1];

						vertexList[i][0] = vertexList[j][0];
						vertexList[i][1] = vertexList[j][1];
						vertexList[i][2] = vertexList[j][2];
						normalList[i][0] = normalList[j][0];
						normalList[i][1] = normalList[j][1];
						normalList[i][2] = normalList[j][2];
						uvList[i][0]	 = uvList[j][0];
						uvList[i][1]	 = uvList[j][1];

						vertexList[j][0] = tempVertex[0];
						vertexList[j][1] = tempVertex[1];
						vertexList[j][2] = tempVertex[2];
						normalList[j][0] = tempNormal[0];
						normalList[j][1] = tempNormal[1];
						normalList[j][2] = tempNormal[2];
						uvList[j][0]	 = tempuv[0];
						uvList[j][1]	 = tempuv[1];
					}
				}
			}
		
			float xmin, xmax, zmin,zmax; //so that we know the bounding limits

			//give some initial value before computing them
			xmin = vertexList[0][0];
			xmax = xmin;
			zmin = vertexList[0][2];
			zmax = zmin;

			//find  x and z boundaries

			for(int i = 0; i < 3; i++)
			{

				if(xmin > vertexList[i][0])
					xmin = vertexList[i][0];

				if(zmin > vertexList[i][2])
					zmin = vertexList[i][2];

				if(xmax < vertexList[i][0])
					xmax = vertexList[i][0];

				if(zmax < vertexList[i][2])
					zmax = vertexList[i][2];
			}
		
	if(render->interp_mode == GZ_COLOR)	//shading applied per vertex
	{
		for(int i = 0; i < 3; i++)
		{
			VertexShader(render,normalList[i],&specular[i],&diffuse[i]);		//for gouraud shading, saving the color in the normal itself (since color interpolation is required, so the normal argument stores the color)
		}
	}
	
	if(render->interp_mode == GZ_COLOR)
		ScanLineRaster(render,vertexList,xmin,xmax,zmin,zmax,normalList,uvList,specular,diffuse);

	else if(render->interp_mode == GZ_NORMALS)
		ScanLineRaster(render,vertexList,xmin,xmax,zmin,zmax,normalList,uvList,NULL,NULL);

	return GZ_SUCCESS;
}


//Raycasting algorithm

void RayTracing(GzRender *render, vector<SceneMesh*> pScene)
{

	 //1st convert all the vertices of the triangles to World Space 
	//also Xform the normals
	vector<SceneMesh>pXformedScene;
	pXformedScene = XformPrimitives(render,pScene);


	printf("Camera Origin : %f, %f, %f \n\n", render->camera.position[X], render->camera.position[Y], render->camera.position[Z]);
	printf("Before Xform, vertex 0 : %f, %f, %f\n\n", pScene.at(0)->normalList[0][X], pScene.at(0)->normalList[0][Y], pScene.at(0)->normalList[0][Z]);
	printf("After world xform, vertex 0 : %f, %f, %f\n",pXformedScene.at(0).normalList[0][X],pXformedScene.at(0).normalList[0][Y],pXformedScene.at(0).normalList[0][Z]);
	//loop through each pixel of the image plane
	for(int j = 0; j < render->display->yres; j++)
	{
		for(int i = 0; i < render->display->xres; i++)
		{
			//cast a ray from the camera / eye to the Screen Pixel (x,y)
			GzRay ray;
			ConstructRayThroughPixel(render,i,j,&ray);
				
			//we are using World space 
			//now find the closest point of intersection of the ray with an object
			//in our case a triangle from the scene with xformed vertices and normals
			GzCoord intersectionPoint;
			int primitiveIdx = -1; //for now, initialize to some value
			
			FindIntersection(ray,pXformedScene,&intersectionPoint,primitiveIdx);
			
			//now find the light intensity at that point
			GzColor intensity;

			if(primitiveIdx != -1)
				{
					GetColor(render,ray,intersectionPoint, pXformedScene.at(primitiveIdx).vertexList, pXformedScene.at(primitiveIdx).normalList,&intensity);
				}
			else
			{
				//set the color to background color since no intersection was found or no closest point was found
				intensity[RED]	 = 0.5;
				intensity[GREEN] = 0.5;
				intensity[BLUE]  = 0.5;
			}
			
			//now assign the current pixel this intensity
			GzIntensity r,g,b;

			r = intensity[RED];
			g = intensity[GREEN];
			b = intensity[BLUE];
			
			//clamping the color to values between 0-255
			if(r > 255)
				r = 255;
			if(r < 0)
				r = 0;
			if(g > 255)
				g = 255;
			if(g < 0)
				g = 0;
			if(b > 255)
				b = 255;
			if(b < 0)
				b = 0;
			render->display->fbuf[i+j*render->display->xres].blue   = b;
			render->display->fbuf[i+j*render->display->xres].green  = g;
			render->display->fbuf[i+j*render->display->xres].red	= r;
		}
	}

}

void RayTracing(GzRender *render, Scene* pScene) 
{
	int count1 = 0;
	
	//do the transformations
	for(int p = 0; p < pScene->numPrimitives; p++)
	{
		if(pScene->getObjectAt(p)->getType() == pScene->getObjectAt(p)->SPHERE) //if sphere object
		{
			GzCoord centre;
			static_cast<CSphere*>(pScene->getObjectAt(p))->getCentre(&centre);
			VertexTransformation(centre,render->Ximage[render->matlevel-1],&centre);
			static_cast<CSphere*>(pScene->getObjectAt(p))->setCentre(&centre);
		}

		else if(pScene->getObjectAt(p)->getType() == pScene->getObjectAt(p)->PLANE)
		{
			//only transform normals
			GzCoord norm;
			static_cast<CPlane*>(pScene->getObjectAt(p))->getNormal(&norm);
			NormalTransformation(norm,render->Xnorm[render->normmatlevel-1], &norm);
			static_cast<CPlane*>(pScene->getObjectAt(p))->setNormal(&norm);
		}
		else if(pScene->getObjectAt(p)->getType() == pScene->getObjectAt(p)->TRIANGLE)
		{
			SceneMesh triMesh = static_cast<CTriangle*>(pScene->getObjectAt(p))->getPrimitives();
			for(int i = 0; i < 3; i++)
			{
				VertexTransformation(triMesh.vertexList[i],render->Ximage[render->matlevel-1],&triMesh.vertexList[i]);
				NormalTransformation(triMesh.normalList[i], render->Xnorm[render->normmatlevel-1],&triMesh.normalList[i]);
			}

			static_cast<CTriangle*>(pScene->getObjectAt(p))->m_trianglePrimitives = triMesh;
		}
	}
	//transform the triangle 
/*	SceneMesh triMesh = static_cast<CTriangle*>(pScene->getObjectAt(0))->getPrimitives();
	for(int i = 0; i < 3; i++)
	{
		VertexTransformation(triMesh.vertexList[i],render->Ximage[render->matlevel-1],&triMesh.vertexList[i]);
		NormalTransformation(triMesh.normalList[i], render->Xnorm[render->normmatlevel-1],&triMesh.normalList[i]);
	}

	static_cast<CTriangle*>(pScene->getObjectAt(0))->m_trianglePrimitives = triMesh;

*/

	//loop through each pixel of the image plane
	for(int j = 0; j < render->display->yres; j++)
	{
		for(int i = 0; i < render->display->xres; i++)
		{	
			int depth = 1;
			GzColor intensity;
			//cast a ray from the camera / eye to the Screen Pixel (x,y)
			GzRay ray;
			ConstructRayThroughPixel(render,i,j,&ray);
			
			//we are using World space 
			//now find the closest point of intersection of the ray with an object
			//in our case a triangle from the scene with xformed vertices and normals
			
			//pScene->getObjectAt(primitiveIdx)->getColor(render,ray,intersectionPoint, &intensity);

			GetColor(render,ray, pScene, depth,&intensity);

			//now assign the current pixel this intensity
			GzIntensity r,g,b;


			intensity[RED] *= 256;
			intensity[GREEN] *= 256;
			intensity[BLUE] *= 256;

			r = intensity[RED];
			g = intensity[GREEN];
			b = intensity[BLUE];
			
			//clamping the color to values between 0-255
			if(r > 255)
				r = 255;
			if(r < 0)
				r = 0;
			if(g > 255)
				g = 255;
			if(g < 0)
				g = 0;
			if(b > 255)
				b = 255;
			if(b < 0)
				b = 0;
			render->display->fbuf[i+j*render->display->xres].blue   = b;
			render->display->fbuf[i+j*render->display->xres].green  = g;
			render->display->fbuf[i+j*render->display->xres].red	= r;
		}
	}

}

void ConstructRayThroughPixel(GzRender* render, int i, int j,GzRay *ray)
{

	//1st find u and v unit vectors i.e the image plane's x and y respectively

	//try Orthographic Projection 1st and then check perspective

	GzCoord u,v,w; //cam axes

	for(int k = 0; k < 3; k++)  //cam z-axis
		 w[k] = render->camera.lookat[k] - render->camera.position[k]; 

	Normalize(&w);
	
	//compute camY axis = up' normalized
	
//	v[X] = render->camera.worldup[X] - (dotProduct(render->camera.worldup,w) * w[X]);
//	v[Y] = render->camera.worldup[Y] - (dotProduct(render->camera.worldup,w) * w[Y]);
//	v[Z] = render->camera.worldup[Z] - (dotProduct(render->camera.worldup,w) * w[Z]);
//	Normalize(&v);
	
	crossProduct(render->camera.worldup,w, &u);
	Normalize(&u);

	crossProduct(w,u,&v);
//	crossProduct(v,w,&u);
	Normalize(&v);	
	
	
	float d = 1 / tan(render->camera.FOV * 0.5 * PI/180.0f);  //z- distance to the image plane
	float aspect = render->display->xres / render->display->yres;  // width / height

	
	//for orthographic projection rayDir = -w
	//calculate the coordinates of the pixel in world space 
	//s = origin + Au + Bv

	//plane coordinates in world space
	float planeLeft = -4.0 * (1/d);
	float planeRight = 4.0 * (1/d);
	float planeTop =   3.0 * (aspect / d);
	float planeBottom = -3.0 * (aspect / d);

	float pixU, pixV;

	//1st method
	pixU = planeLeft + (imagePlaneWidth * (i + 0.5) / (render->display->xres));
	pixV = planeBottom + (imagePlaneHeight * (j + 0.5) / (render->display->yres));

	//in image space
//	pixU =   (imagePlaneWidth * (i + 0.5) / (render->display->xres));
//	pixV =   (imagePlaneHeight * (j + 0.5) / (render->display->yres));

	//2nd method
	//pixU = -ex + (planeWidth * (i + 0.5) / (render->display->xres));
	//pixV = -ey + (planeHeight * (j + 0.5) / (render->display->yres));

	//for perspective rayDir = s - e = Au +Bv - dW
	//float d = 8; //distance to the image plane from the z-axis (any value)
	//d = (imagePlaneWidth) / (2 * tan(render->camera.FOV * 0.5 * PI/180.0f));
	
  GzCoord rayDirection;
	rayDirection[X] = (pixU * u[X]) + (pixV * v[X]) - (d * w[X]);  
	rayDirection[Y] = (pixU * u[Y]) + (pixV * v[Y]) - (d * w[Y]);
	rayDirection[Z] = (pixU * u[Z]) + (pixV * v[Z]) - (d * w[Z]);

	//we are using orthographic projection (in image space)
	//so direction is constant - look at 0,0,-1

//	rayDirection[X] = 0;
//	rayDirection[Y] = 0;
//	rayDirection[Z] = d;

	//normalize the direction
	Normalize(&rayDirection);

	GzCoord rOrigin;
	//origin = e + Au + Bv
//	rOrigin[X] = render->camera.position[X] + (pixU * u[X]) + (pixV * v[X]);
//	rOrigin[Y] = render->camera.position[Y] + (pixU * u[Y]) + (pixV * v[Y]);
//	rOrigin[Z] = render->camera.position[Z] + (pixU * u[Z]) + (pixV * v[Z]);

	//for orthographic rOrigin = pixX, pixY, 0 in camera/image space
///	rOrigin[X] = pixU;
//	rOrigin[Y] = pixV;
//	rOrigin[Z] = 0;

	//for perspective rOrigin = e
	memcpy(&rOrigin, &(render->camera.position),sizeof(GzCoord));

	//ray equation is r = rOrigin + t * direction
	
	memcpy(&ray->origin, &rOrigin, sizeof(GzCoord));
	memcpy(&ray->direction, &rayDirection, sizeof(GzCoord));
	

}

void FindIntersection(GzRay ray, vector<SceneMesh> pScene, GzCoord* intersectionPoint,int& primitiveIndex)
{
	GzCoord ClosestPoint;
	int ClosestPrimitiveIdx = -1; //closest primitive not found
	float closestDistance = INT_MAX; //sentinel value

	ClosestPoint[X] = INT_MAX;
	ClosestPoint[Y] = INT_MAX;
	ClosestPoint[Z] = INT_MAX;

	for(int i = 0; i < pScene.size(); i++)  //loop through all the primitives (for now all the primitives are triangles that make up the teapot)
	{

		float tValue;
		bool intersects = Intersect(ray,pScene[i].vertexList,tValue);
			
			if(intersects == true)
			{
				//assign closest point
				if(tValue < closestDistance)
				{
				  closestDistance = tValue;
				  ClosestPrimitiveIdx = i;
				}
			}
	}

	 //point is 
	if(ClosestPrimitiveIdx != -1) //means closest point was found
	{
		ClosestPoint[X] = ray.origin[X] + (closestDistance * ray.direction[X]); 
		ClosestPoint[Y] = ray.origin[Y] + (closestDistance * ray.direction[Y]);
		ClosestPoint[Z] = ray.origin[Z] + (closestDistance * ray.direction[Z]);
	}

	memcpy(intersectionPoint,ClosestPoint,sizeof(GzCoord)); //can store either sentinel or closest point
	primitiveIndex = ClosestPrimitiveIdx;
}

void FindIntersection(GzRay ray, Scene* pScene, GzCoord* intersectionPoint, int& primitiveIndex)
{
	GzCoord ClosestPoint;
	int ClosestPrimitiveIdx = -1; //closest primitive not found
	float closestDistance = INT_MAX; //sentinel value

	ClosestPoint[X] = INT_MAX;
	ClosestPoint[Y] = INT_MAX;
	ClosestPoint[Z] = INT_MAX;

	for(int i = 0; i < pScene->numPrimitives; i++)
	{
		float tValue;

		bool intersects = pScene->getObjectAt(i)->Intersects(ray,tValue); 
		if(intersects == true)
		{
			//assign closest point
				if(tValue < closestDistance)
				{
				  closestDistance = tValue;
				  ClosestPrimitiveIdx = 0;
				}
		}

	}

	 //point is 
	if(closestDistance != INT_MAX) //means closest point was found
	{
		ClosestPoint[X] = ray.origin[X] + (closestDistance * ray.direction[X]); 
		ClosestPoint[Y] = ray.origin[Y] + (closestDistance * ray.direction[Y]);
		ClosestPoint[Z] = ray.origin[Z] + (closestDistance * ray.direction[Z]);
	}

	memcpy(intersectionPoint,ClosestPoint,sizeof(GzCoord)); //can store either sentinel or closest point
	primitiveIndex = ClosestPrimitiveIdx;
}

bool Intersect(GzRay ray, GzCoord* pVertexList,float& tValue)
{
	//1st find the surface normal to the plane containing the triangle vertices
	//GzCoord normal;
	//GzCoord tempPoint;

/*	GetDirectionNormals(pVertexList[0],pVertexList[1],pVertexList[2],&normal);
	//	Normalize(&normal);

	//equation of the plane containing the triangle : ax+by+cz = d
	float dValue = dotProduct(normal,pVertexList[0]); // technically its -d but just check both cases

	//1st check if ray.direction . N > 0, then no intersection - early elimination
	float dotDirNorm = dotProduct(normal,ray.direction);
	if(dotDirNorm == 0)
		return false;
*/	
	
	GzCoord L1, L2;
	L1[X] = pVertexList[1][X] - pVertexList[0][X];
	L1[Y] = pVertexList[1][Y] - pVertexList[0][Y];
	L1[Z] = pVertexList[1][Z] - pVertexList[0][Z];

	L2[X] = pVertexList[2][X] - pVertexList[0][X];
	L2[Y] = pVertexList[2][Y] - pVertexList[0][Y];
	L2[Z] = pVertexList[2][Z] - pVertexList[0][Z];

	GzCoord distanceVec;
	distanceVec[X] = ray.origin[X] - pVertexList[0][X];
	distanceVec[Y] = ray.origin[Y] - pVertexList[0][Y];
	distanceVec[Z] = ray.origin[Z] - pVertexList[0][Z];

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

/*	if(!(dotDirNorm < 0.0))
	{
		//no intersection - so early elimination
		tValue = INT_MAX;
		return false;
	}
	
	//now find t = (d - rayOrigin.N)/direction.N
	tValue = (dValue - dotProduct(normal,ray.origin)); //don't divide yet

	if(!(tValue <= 0.0)) //we won't intersection only at the front side
	{
		//no frontal intersection
		tValue = INT_MAX;
		return false;
	}

	tValue /= dotDirNorm;

		//now that we have t, we can find the point from the ray equation
		tempPoint[X] = ray.origin[X] + (tValue * ray.direction[X]);
		tempPoint[Y] = ray.origin[Y] + (tValue * ray.direction[Y]);
		tempPoint[Z] = ray.origin[Z] + (tValue * ray.direction[Z]);

		//check if the point lies inside or outside the triangle using barycentric coordinates
		if(BarycentricInterpolationTest(tempPoint,pVertexList,normal) == true)
		{
			return true;
		}
		else //point probably lies outside
		{
		tValue = INT_MAX;
		return false;
		}	
*/
}

float getVectorMagnitude(GzCoord vec)
{
	return sqrt(pow(vec[X],2) + pow(vec[Y],2) + pow(vec[Z],2));
}

void GetColor(GzRender* render,GzRay ray, GzCoord hitPoint,GzCoord* vertexList,GzCoord* pNormalList,GzColor* intensity)  //normals of the hit point object
{
	//1st interpolate the normals at the hit point
	GzCoord normal;
	GzColor resultIntensity;

	if(count == 0)
	{	
		fprintf(stdout,"Valid Point!");
		printf("Hit Point: %f, %f, %f\n",hitPoint[X], hitPoint[Y], hitPoint[Z]);
		//-0.225410, 5.733967, 30.610357
	}
	count++;

		NormalInterpolation(hitPoint,vertexList,pNormalList,&normal);
		//Interpolation(hitPoint[X], hitPoint[Y], vertexList, pNormalList, &normal);
		//now that we have got the normal at the hit point, apply the Shading equation
		PhongIllumination(render,ray,normal,&resultIntensity);
	
		if(count == 1)
		{
			printf("\nr = %f", resultIntensity[RED]);
			printf("\nb = %f", resultIntensity[GREEN]);
			printf("\ng = %f", resultIntensity[BLUE]);
		}

		resultIntensity[RED]	*= 256;
		resultIntensity[GREEN]	*= 256;
		resultIntensity[BLUE]	*= 256;
		
		memcpy(intensity,&resultIntensity,sizeof(GzColor));

}
void GetColor(GzRender* render,GzRay ray,Scene* pScene,int depth,GzColor* intensity)  //normals of the hit point object
{
	//1st interpolate the normals at the hit point
	GzCoord normal;
	GzColor resultIntensity;	
	GzColor specColor,localIntensity,refracColor;	
	GzRay reflection,refraction;		
	GzColor tempKr,tempKt;		

	tempKr[RED] = 0.5;
	tempKr[GREEN] = 0.5;
	tempKr[BLUE] = 0.5;
	tempKt[RED] = 0.1;
	tempKt[GREEN] = 0.1;
	tempKt[BLUE] = 0.1;

	GzCoord intersectionPoint;
	int primitiveIdx = -1; //for now, initialize to some value
	FindIntersection(ray,pScene,&intersectionPoint,primitiveIdx);

	//pScene->getObjectAt(primitiveIdx)->getColor(render,ray,intersectionPoint, &intensity);

	
	if(primitiveIdx != -1){
		//if hit something then we need to give back some value
	
		//NormalInterpolation(intersectionPoint,pXformedScene.at(primitiveIdx).vertexList, pXformedScene.at(primitiveIdx).normalList,&normal);
		pScene->getObjectAt(primitiveIdx)->getSurfaceNormal(intersectionPoint,&normal);
		//pScene->getObjectAt(primitiveIdx)->getKr(&tempKr);
		//pScene->getObjectAt(primitiveIdx)->getKt(&tempKt);
		PhongIllumination(render,ray,normal,&localIntensity);		
		
		//checking the Ks to decide do we just give the phong value or added the reflection value
		
		if(tempKr[RED]>0||tempKr[GREEN]>0||tempKr[BLUE]>0){
			/*
			if(depth <= 2)
			{
				printf("\nInRay.origin[X] = %f", ray.origin[X]);
				printf("\nInRay.origin[Y] = %f", ray.origin[Y]);
				printf("\nInRay.origin[Z] = %f", ray.origin[Z]);
				printf("\nInRay.direction[X] = %f", ray.direction[X]);
				printf("\nInRay.direction[Y] = %f", ray.direction[Y]);
				printf("\nInRay.direction[Z] = %f", ray.direction[Z]);
				printf("\nintersectionPoint[X] = %f", intersectionPoint[X]);
				printf("\nintersectionPoint[Y] = %f", intersectionPoint[Y]);
				printf("\nintersectionPoint[Z] = %f", intersectionPoint[Z]);


			}
			*/
			ComputeReflectionRay(&ray,normal,intersectionPoint,&reflection);
			/*
			if(depth <= 2)
			{
				printf("\nOutRay.origin[X] = %f", reflection.origin[X]);
				printf("\nOutRay.origin[Y] = %f", reflection.origin[Y]);
				printf("\nOutRay.origin[Z] = %f", reflection.origin[Z]);
				printf("\nOutRay.direction[X] = %f", reflection.direction[X]);
				printf("\nOutRay.direction[Y] = %f", reflection.direction[Y]);
				printf("\nOutRay.direction[Z] = %f", reflection.direction[Z]);

			}
			*/
			if(depth<=5){
				GetColor(render,reflection,pScene,depth+1,&specColor);
				//multiply by Ks value
				specColor[RED] *= tempKr[RED]*(1.0/(float)depth);
				specColor[GREEN] *= tempKr[GREEN]*(1.0/(float)depth);
				specColor[BLUE] *= tempKr[BLUE]*(1.0/(float)depth);
			}
			else{
				specColor[RED] = 0;
				specColor[GREEN] = 0;
				specColor[BLUE] = 0;
			}
			
		}
		else{

				specColor[RED] = 0;
				specColor[GREEN] = 0;
				specColor[BLUE] = 0;
		}
		
		if(tempKt[RED]>0||tempKt[GREEN]>0||tempKt[BLUE]>0){
			ComputeRefractionRay(&ray,normal,intersectionPoint,&refraction);
			if(depth<=3){
				GetColor(render,refraction,pScene,depth+1,&refracColor);
				//multiply by Ks value
				refracColor[RED] *= tempKt[RED]*(1.0/(float)depth);
				refracColor[GREEN] *= tempKt[GREEN]*(1.0/(float)depth);
				refracColor[BLUE] *= tempKt[BLUE]*(1.0/(float)depth);
			}
			else{
				refracColor[RED] = 0;
				refracColor[GREEN] = 0;
				refracColor[BLUE] = 0;
			}
			
		}
		else{
		
				refracColor[RED] = 0;
				refracColor[GREEN] = 0;
				refracColor[BLUE] = 0;
		}

		resultIntensity[RED] = specColor[RED] + localIntensity[RED] + refracColor[RED];
		resultIntensity[GREEN] = specColor[GREEN] + localIntensity[GREEN]+ refracColor[RED];
		resultIntensity[BLUE] = specColor[BLUE] + localIntensity[BLUE]+ refracColor[BLUE];
	}
	else{
		//not hitting anything
		//if (depth >1){
		//	resultIntensity[RED] = 0.9;
		//	resultIntensity[GREEN] = 0.1;
		//	resultIntensity[BLUE] = 0.1;
		//}
		//else{
			resultIntensity[RED]   = 0;
			resultIntensity[GREEN] = 0;
			resultIntensity[BLUE]  = 0;
		//}
	}	
	memcpy(intensity,&resultIntensity,sizeof(GzColor));		

}

void ComputeReflectionRay(GzRay* ray,GzCoord normal,GzCoord hitPoint,GzRay* reflectionRay){
	GzRay tempRay;
	GzCoord tempIn,tempOut,tempPoint;
	float raydotN;
	
	float dT = 1;
	
	tempIn[X] = ray->direction[X];
	tempIn[Y] = ray->direction[Y];
	tempIn[Z] = ray->direction[Z];

	raydotN = dotProduct(normal, tempIn);
	
	if(raydotN>0){
		normal[X] *= -1;
		normal[Y] *= -1;
		normal[Z] *= -1;
		//Normalize(&normal);
		raydotN = dotProduct(normal, tempIn);
	}
	/*
	tempOut[X] = (2 * raydotN * normal[X]) - tempIn[X];
	tempOut[Y] = (2 * raydotN * normal[Y]) - tempIn[Y];
	tempOut[Z] = (2 * raydotN * normal[Z]) - tempIn[Z];	
	Normalize(&tempOut);
	memcpy(tempRay.direction,tempOut,sizeof(GzCoord));
	

	*/
	tempOut[X] = tempIn[X] - (2 * abs(raydotN) * normal[X]) ;
	tempOut[Y] = tempIn[Y] - (2 * abs(raydotN) * normal[Y]) ;
	tempOut[Z] = tempIn[Z] - (2 * abs(raydotN) * normal[Z]) ;
	Normalize(&tempOut);
	memcpy(tempRay.direction,tempOut,sizeof(GzCoord));
	
	tempPoint[X] = hitPoint[X] + dT * tempOut[X];
	tempPoint[Y] = hitPoint[Y] + dT * tempOut[Y];
	tempPoint[Z] = hitPoint[Z] + dT * tempOut[Z];

	memcpy(tempRay.origin,tempPoint,sizeof(GzCoord));
	memcpy(reflectionRay,&tempRay,sizeof(GzRay));
	return ;
}

void ComputeRefractionRay(GzRay* ray,GzCoord normal,GzCoord hitPoint,GzRay* refractionRay){
	GzRay tempRay;
	GzCoord tempIn,tempOut,tempPoint;
	float raydotN;
	
	float dT = 1;

	memcpy(tempRay.direction,ray->direction,sizeof(GzCoord));
	
	tempPoint[X] = ray->origin[X] + dT * ray->direction[X];
	tempPoint[Y] = ray->origin[Y] + dT * ray->direction[Y];
	tempPoint[Z] = ray->origin[Z] + dT * ray->direction[Z];

	memcpy(tempRay.origin,tempPoint,sizeof(GzCoord));
	memcpy(refractionRay,&tempRay,sizeof(GzRay));

	return;
}

