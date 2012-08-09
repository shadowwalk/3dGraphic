/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"


int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	if (mat != NULL){
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				mat[i][j] = 0;
			}
		}
		mat[0][0] = 1;
		mat[1][1] = cos(degree*atan(1.0)*4/180);
		mat[1][2] = -sin(degree*atan(1.0)*4/180);
		mat[2][1] = sin(degree*atan(1.0)*4/180);
		mat[2][2] = cos(degree*atan(1.0)*4/180);
		mat[3][3] = 1;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value

	if (mat != NULL){
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				mat[i][j] = 0;
			}
		}
		mat[1][1] = 1;
		mat[0][0] = cos(degree*atan(1.0)*4/180);
		mat[0][2] = sin(degree*atan(1.0)*4/180);
		mat[2][0] = -sin(degree*atan(1.0)*4/180);
		mat[2][2] = cos(degree*atan(1.0)*4/180);
		mat[3][3] = 1;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value

	if (mat != NULL){
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				mat[i][j] = 0;
			}
		}
		mat[2][2] = 1;
		mat[0][0] = cos(degree*atan(1.0)*4/180);
		mat[0][1] = -sin(degree*atan(1.0)*4/180);
		mat[1][0] = sin(degree*atan(1.0)*4/180);
		mat[1][1] = cos(degree*atan(1.0)*4/180);
		mat[3][3] = 1;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value


	if (mat != NULL){
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				mat[i][j] = 0;
			}
		}
		mat[0][0] = 1;
		mat[1][1] = 1;
		mat[2][2] = 1;
		mat[3][3] = 1;
		mat[0][3] = translate[0];
		mat[1][3] = translate[1];
		mat[2][3] = translate[2];
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value

	if (mat != NULL){
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				mat[i][j] = 0;
			}
		}
		mat[0][0] = scale[0];
		mat[1][1] = scale[1];
		mat[2][2] = scale[2];
		mat[3][3] = 1;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
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

	if(renderClass == GZ_Z_BUFFER_RENDER &&  display != NULL){
		*render = (GzRender*)malloc(sizeof(GzRender));
		(**render).display = display;
		(**render).renderClass = renderClass;
		(**render).open = GZ_FAILURE;

		(**render).camera.lookat[0] = 0;
		(**render).camera.lookat[1] = 0;
		(**render).camera.lookat[2] = 0;
		(**render).camera.position[0] = DEFAULT_IM_X;
		(**render).camera.position[1] = DEFAULT_IM_Y;
		(**render).camera.position[2] = DEFAULT_IM_Z;
		(**render).camera.worldup[0] = 0;
		(**render).camera.worldup[1] = 1;
		(**render).camera.worldup[2] = 0;
		(**render).camera.FOV = DEFAULT_FOV;
		(**render).xOffset = 0;
		(**render).yOffset = 0;

		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}

	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	if(render != NULL){
		delete(render);
		//delete(render->ambientlight);
		//delete(render->display);
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
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
	GzMatrix tempXform;
	float camVectorX,camVectorY,camVectorZ;
	float upVectorX,upVectorY,upVectorZ;
	float sideVectorX,sideVectorY,sideVectorZ;
	float tempLength;
	

	if(render != NULL){
		//-------------initial param----------------
		
		render->ambientlight.color[0] = 0;
		render->ambientlight.color[1] = 0;
		render->ambientlight.color[2] = 0;
		render->ambientlight.direction[0] = 0;
		render->ambientlight.direction[1] = 0;
		render->ambientlight.direction[2] = 0;
		
		render->interp_mode = GZ_NONE;
		render->numlights = 0;
		render->spec = DEFAULT_SPEC;
		render->tex_fun = NULL;

		for(int i=0;i<MAX_LIGHTS;i++){
			render->lights[i].color[0] = 0;
			render->lights[i].color[1] = 0;
			render->lights[i].color[2] = 0;
			render->lights[i].direction[0] = 0;
			render->lights[i].direction[1] = 0;
			render->lights[i].direction[2] = 0;
			
		}
		
		for(int i=0;i<3;i++){
			render->flatcolor[i] = GZ_NONE;
			render->Ka[i] = 0.1;
		}
		render->Kd[0] = 0.7;
		render->Kd[1] = 0.6;
		render->Kd[2] = 0.5;
		render->Ks[0] = 0.2;
		render->Ks[1] = 0.3;
		render->Ks[2] = 0.4;
		


		//-------------------------

		render->open = 1;
		//------------already have camera, initial matrix-------
		render->matlevel = 0;		
		for(int j=0;j<4;j++){
			for(int k=0;k<4;k++){
				if(j == k){
					render->Xsp[j][k] = 1;
				}
				else{
					render->Xsp[j][k] = 0;
				}
			}
		}
		for(int i=0;i<MATLEVELS;i++){
			for(int j=0;j<4;j++){
				for(int k=0;k<4;k++){
					if(j == k){
						render->Ximage[i][j][k] = 1;
						render->Xnorm[i][j][k] = 1;
					}
					else{
						render->Ximage[i][j][k] = 0;
						render->Xnorm[i][j][k] = 0;
					}
				}
			}
		}
		//--------------------------------------------------

		//-------------push initial matrix Xsp------------
		//tempD = 1/(tan(render->camera.FOV*atan(1.0)*4*90));
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				tempXform[i][j] = 0;
			}
		}
		tempXform[0][0] = ((*render).display->xres)/(float)2;
		tempXform[0][3] = ((*render).display->xres)/(float)2;
		tempXform[1][1] = -((*render).display->yres)/(float)2;
		tempXform[1][3] = ((*render).display->yres)/(float)2;
		tempXform[2][2] = INT_MAX * tan(render->camera.FOV*atan(1.0)*4/360);
		tempXform[3][3] = 1;
		GzPushMatrix(render, tempXform);
		//----------------push and generate Xpi--------------
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				tempXform[i][j] = 0;
			}
		}
		tempXform[0][0] = 1;
		tempXform[1][1] = 1;
		tempXform[2][2] = 1;
		tempXform[3][3] = 1;
		tempXform[3][2] = tan(render->camera.FOV*atan(1.0)*4/360);
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				render->camera.Xpi[i][j] = tempXform[i][j];
			}
		}
		GzPushMatrix(render, tempXform);


		//----------------generate and push Xiw---------------
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				tempXform[i][j] = 0;
			}
		}
		camVectorX = render->camera.lookat[X] - render->camera.position[X];
		camVectorY = render->camera.lookat[Y] - render->camera.position[Y];
		camVectorZ = render->camera.lookat[Z] - render->camera.position[Z];
		tempLength = sqrt(camVectorX*camVectorX + camVectorY*camVectorY + camVectorZ*camVectorZ);
		camVectorX = camVectorX/tempLength;
		camVectorY = camVectorY/tempLength;
		camVectorZ = camVectorZ/tempLength;
		tempXform[2][0] = camVectorX;
		tempXform[2][1] = camVectorY;
		tempXform[2][2] = camVectorZ;
		
		upVectorX = render->camera.worldup[X];// - render->camera.position[X];//can be modified
		upVectorY = render->camera.worldup[Y];// - render->camera.position[Y];
		upVectorZ = render->camera.worldup[Z];// - render->camera.position[Z];
		tempLength = sqrt(upVectorX*upVectorX + upVectorY*upVectorY + upVectorZ*upVectorZ);
		upVectorX = upVectorX/tempLength;
		upVectorY = upVectorY/tempLength;
		upVectorZ = upVectorZ/tempLength;
		
		/*
		upVectorX = 0  ;
		upVectorY = 1  ;
		upVectorZ = 0 ;
		*/
		sideVectorX = upVectorY*camVectorZ - upVectorZ*camVectorY;
		sideVectorY = upVectorZ*camVectorX - upVectorX*camVectorZ;
		sideVectorZ = upVectorX*camVectorY - upVectorY*camVectorX;
		tempLength = sqrt(sideVectorX*sideVectorX + sideVectorY*sideVectorY + sideVectorZ*sideVectorZ);
		sideVectorX = sideVectorX/tempLength;
		sideVectorY = sideVectorY/tempLength;
		sideVectorZ = sideVectorZ/tempLength;
		tempXform[0][0] = sideVectorX;
		tempXform[0][1] = sideVectorY;
		tempXform[0][2] = sideVectorZ;
		
		upVectorX = camVectorY*sideVectorZ - camVectorZ*sideVectorY;
		upVectorY = camVectorZ*sideVectorX - camVectorX*sideVectorZ;
		upVectorZ = camVectorX*sideVectorY - camVectorY*sideVectorX;		
		tempLength = sqrt(upVectorX*upVectorX + upVectorY*upVectorY + upVectorZ*upVectorZ);
		upVectorX = upVectorX/tempLength;
		upVectorY = upVectorY/tempLength;
		upVectorZ = upVectorZ/tempLength;
		tempXform[1][0] = upVectorX;
		tempXform[1][1] = upVectorY;
		tempXform[1][2] = upVectorZ;
		
		tempXform[0][3] = -(sideVectorX*(render->camera.position[X]) + sideVectorY*(render->camera.position[Y]) + sideVectorZ*(render->camera.position[Z]));
		tempXform[1][3] = -(upVectorX*(render->camera.position[X]) + upVectorY*(render->camera.position[Y]) + upVectorZ*(render->camera.position[Z]));
		tempXform[2][3] = -(camVectorX*(render->camera.position[X]) + camVectorY*(render->camera.position[Y]) + camVectorZ*(render->camera.position[Z]));
		tempXform[3][3] = 1;
		GzPushMatrix(render, tempXform);
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				render->camera.Xiw[i][j] = tempXform[i][j];
			}
		}
		

		return GZ_SUCCESS;
  }
  else{
		return GZ_FAILURE;
  }
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	if(render != NULL && camera != NULL){
		render->camera.FOV = camera->FOV;
		for(int i=0;i<3;i++){
			render->camera.lookat[i] = camera->lookat[i];
			render->camera.position[i] = camera->position[i];
			render->camera.worldup[i] = camera->worldup[i];
		}
		for (int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				render->camera.Xiw[i][j] = camera->Xiw[i][j];
				render->camera.Xpi[i][j] = camera->Xpi[i][j];
			}
		}
		return GZ_SUCCESS;	
	}
	else{
		return GZ_FAILURE;
	}
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	GzMatrix tempMatrix;
	float scaleFactor[3];

	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			tempMatrix[i][j] = matrix[i][j];
		}
	}

	if(render != NULL && render->matlevel < MATLEVELS){
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				render->Ximage[render->matlevel][i][j] = tempMatrix[i][j];
			}
		}


		//---------------push uniform matrix Xnorm-------------------
		if(render->matlevel > 1){
			tempMatrix[0][3] = 0;
			tempMatrix[1][3] = 0;
			tempMatrix[2][3] = 0;
			tempMatrix[3][3] = 1;

			tempMatrix[3][2] = 0;
			tempMatrix[3][1] = 0;
			tempMatrix[3][0] = 0;

		
			scaleFactor[0] = sqrt(tempMatrix[0][0]*tempMatrix[0][0] + tempMatrix[0][1]*tempMatrix[0][1] + tempMatrix[0][2]*tempMatrix[0][2]);
			scaleFactor[1] = sqrt(tempMatrix[1][0]*tempMatrix[1][0] + tempMatrix[1][1]*tempMatrix[1][1] + tempMatrix[1][2]*tempMatrix[1][2]);
			scaleFactor[2] = sqrt(tempMatrix[2][0]*tempMatrix[2][0] + tempMatrix[2][1]*tempMatrix[2][1] + tempMatrix[2][2]*tempMatrix[2][2]);
	
			for (int i=0;i<3;i++){
				for (int j=0;j<3;j++){
					tempMatrix[i][j] = tempMatrix[i][j]/scaleFactor[i];
				}
			}
			for(int i=0;i<4;i++){
				for(int j=0;j<4;j++){
					render->Xnorm[render->matlevel][i][j] = tempMatrix[i][j];
				}
			}
		}
		render->matlevel ++;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if(render != NULL && render->matlevel >0 ){
		//for(int i=0;i<4;i++){
		//	for(int j=0;j<4;j++){
		//		render->Xnorm[render->matlevel][i][j] = render->Ximage[render->matlevel][i][j];
		//	}
		//}
		render->matlevel --;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	GzColor *test;
	
	if(render != NULL && nameList != NULL && valueList != NULL){
		for (int x=0; x<numAttributes;x++){
			if(*(nameList+x) == GZ_RGB_COLOR){
				render->flatcolor[0] = (*((GzColor*)(valueList)[x]))[0];
				render->flatcolor[1] = (*((GzColor*)(valueList)[x]))[1];
				render->flatcolor[2] = (*((GzColor*)(valueList)[x]))[2];
			}
			if(*(nameList+x) == GZ_DIRECTIONAL_LIGHT){
				render->lights[render->numlights].color[0] = ((GzLight*)(valueList)[x])->color[0];
				render->lights[render->numlights].color[1] = ((GzLight*)(valueList)[x])->color[1];
				render->lights[render->numlights].color[2] = ((GzLight*)(valueList)[x])->color[2];
				render->lights[render->numlights].direction[0] = ((GzLight*)(valueList)[x])->direction[0];
				render->lights[render->numlights].direction[1] = ((GzLight*)(valueList)[x])->direction[1];
				render->lights[render->numlights].direction[2] = ((GzLight*)(valueList)[x])->direction[2];
				render->numlights ++;
			}
			if(*(nameList+x) == GZ_AMBIENT_LIGHT){
				render->ambientlight.color[0] = ((GzLight*)(valueList)[x])->color[0];
				render->ambientlight.color[1] = ((GzLight*)(valueList)[x])->color[1];
				render->ambientlight.color[2] = ((GzLight*)(valueList)[x])->color[2];
				render->ambientlight.direction[0] = ((GzLight*)(valueList)[x])->direction[0];
				render->ambientlight.direction[1] = ((GzLight*)(valueList)[x])->direction[1];
				render->ambientlight.direction[2] = ((GzLight*)(valueList)[x])->direction[2];
			}
			if(*(nameList+x) == GZ_DIFFUSE_COEFFICIENT){
				render->Kd[0] = (*((GzColor*)(valueList)[x]))[0];
				render->Kd[1] = (*((GzColor*)(valueList)[x]))[1];
				render->Kd[2] = (*((GzColor*)(valueList)[x]))[2];
			}
			if(*(nameList+x) == GZ_AMBIENT_COEFFICIENT){
				//test = (GzColor*)(valueList)[x];
				render->Ka[0] = (*((GzColor*)(valueList)[x]))[0];
				render->Ka[1] = (*((GzColor*)(valueList)[x]))[1];
				render->Ka[2] = (*((GzColor*)(valueList)[x]))[2];
			}
			if(*(nameList+x) == GZ_SPECULAR_COEFFICIENT){
				render->Ks[0] = (*((GzColor*)(valueList)[x]))[0];
				render->Ks[1] = (*((GzColor*)(valueList)[x]))[1];
				render->Ks[2] = (*((GzColor*)(valueList)[x]))[2];
			}
			if(*(nameList+x) == GZ_DISTRIBUTION_COEFFICIENT){
				render->spec = *((float*)(valueList)[x]);
			}
			if(*(nameList+x) == GZ_INTERPOLATE){
				render->interp_mode = *((int*)(valueList)[x]);
			}	
			if(*(nameList+x) == GZ_TEXTURE_MAP){
				render->tex_fun = (GzTexture)(valueList)[x];
			}	
			if(*(nameList+x) == GZ_AASHIFTX){
				render->xOffset = *((float*)(valueList)[x]);
			}	
			if(*(nameList+x) == GZ_AASHIFTY){
				render->yOffset = *((float*)(valueList)[x]);
			}			
		}
		
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}

	
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, 
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
	
	float newPointVector[3][4];
	float oldPointVector[3][4];
	float newNormVector[3][4];
	float oldNormVector[3][4];
	float newUVValue[3][2];
	float oldUVValue[3][2];

	GzMatrix comboXform,tempCombo,normCombo;

	if(render != NULL && nameList != NULL && valueList != NULL){
		float x[3],y[3],z[3];

		float dX,dY;
		int highX ,lowX ;
		int highY ,lowY ;
		float A,B,C,nD;
		float Ar,Br,Cr,nDr;
		float Ag,Bg,Cg,nDg;
		float Ab,Bb,Cb,nDb;
		float alpha,beta,gama;
		GzDepth tempZ;
		int pos;

		float nx[3],ny[3],nz[3],pointNorm[3];
		float ambient[3][3],diffuse[3][3],specular[3][3];
		float pAmbient[3],pDiffuse[3],pSpecular[3];
		float color[3][3];
		float pColor[3];
		float pScale;
		float finalR,finalG,finalB;
		float Rvector[3];
		float NdotL,RdotE,NdotE;

		float u[3],v[3];
		float pU,pV;
		
		float pTexture[3];
		float vZPrime[3];
		float interpZ;
		//------------------fetch data-------------------------

		for(int m=0;m<numParts;m++){
			if(*(nameList+m) == GZ_POSITION){

				x[0] = (*((GzCoord*)(valueList)[m]))[0];
				y[0] = (*((GzCoord*)(valueList)[m]))[1];
				z[0] = (*((GzCoord*)(valueList)[m]))[2];
				x[1] = (*((GzCoord*)(valueList)[m] + 1))[0];
				y[1] = (*((GzCoord*)(valueList)[m] + 1))[1];
				z[1] = (*((GzCoord*)(valueList)[m] + 1))[2];
				x[2] = (*((GzCoord*)(valueList)[m] + 2))[0];
				y[2] = (*((GzCoord*)(valueList)[m] + 2))[1];
				z[2] = (*((GzCoord*)(valueList)[m] + 2))[2];

			}
			if(*(nameList+m) == GZ_NORMAL){
				nx[0] = (*((GzCoord*)(valueList)[m]))[0];
				ny[0] = (*((GzCoord*)(valueList)[m]))[1];
				nz[0] = (*((GzCoord*)(valueList)[m]))[2];
				nx[1] = (*((GzCoord*)(valueList)[m] + 1))[0];
				ny[1] = (*((GzCoord*)(valueList)[m] + 1))[1];
				nz[1] = (*((GzCoord*)(valueList)[m] + 1))[2];
				nx[2] = (*((GzCoord*)(valueList)[m] + 2))[0];
				ny[2] = (*((GzCoord*)(valueList)[m] + 2))[1];
				nz[2] = (*((GzCoord*)(valueList)[m] + 2))[2];
			}
			if(*(nameList+m) == GZ_TEXTURE_INDEX){
				u[0] = (*((GzTextureIndex*)(valueList)[m]))[0];
				v[0] = (*((GzTextureIndex*)(valueList)[m]))[1];
				u[1] = (*((GzTextureIndex*)(valueList)[m] + 1))[0];
				v[1] = (*((GzTextureIndex*)(valueList)[m] + 1))[1];
				u[2] = (*((GzTextureIndex*)(valueList)[m] + 2))[0];
				v[2] = (*((GzTextureIndex*)(valueList)[m] + 2))[1];
			}		
		}

		//----------------------------------------------------
			

		//float a = *(((GzCoord*)(*valueList))[0]+1);
		//float b = *(((GzCoord*)(*valueList))[1]+1);
		//float c = *(((GzCoord*)(*valueList))[2]+1);
		
		for(int i=0;i<3;i++){
			oldPointVector[i][0] = x[i];
			oldPointVector[i][1] = y[i];
			oldPointVector[i][2] = z[i];
			oldPointVector[i][3] = 1;
		}
		for(int i=0;i<3;i++){
			oldNormVector[i][0] = nx[i];
			oldNormVector[i][1] = ny[i];
			oldNormVector[i][2] = nz[i];
			oldNormVector[i][3] = 1;
		}
		for(int i=0;i<3;i++){
			for(int j=0;j<3;j++){	
				color[i][j] = 0;
			}
			pColor[i] = 0;
		}	
		for(int i=0;i<3;i++){
			oldUVValue[i][0] = u[i];
			oldUVValue[i][1] = v[i];
		}

		//----------get the combo matrix-----------
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				if(i == j){
					comboXform[i][j] = 1;
					tempCombo[i][j] = 1;
				}
				else{
					comboXform[i][j] = 0;
					tempCombo[i][j] = 0;
				}
			}
		}

		for(int m=0;m<render->matlevel;m++){
			for(int i=0;i<4;i++){
				for(int j=0;j<4;j++){
					comboXform[i][j] = tempCombo[i][0]*(render->Ximage[m][0][j]) 
									+ tempCombo[i][1]*(render->Ximage[m][1][j]) 
									+ tempCombo[i][2]*(render->Ximage[m][2][j]) 
									+ tempCombo[i][3]*(render->Ximage[m][3][j]);
				}
			}

			for(int i=0;i<4;i++){
				for(int j=0;j<4;j++){
					tempCombo[i][j] = comboXform[i][j];
				}
			}
		}
		//------------------------------------------

		//-----------get new cood vector--------------
		for(int m=0;m<3;m++){
			for(int i=0;i<4;i++){
				newPointVector[m][i] = comboXform[i][0]*oldPointVector[m][0]
									+ comboXform[i][1]*oldPointVector[m][1]
									+ comboXform[i][2]*oldPointVector[m][2]
									+ comboXform[i][3]*oldPointVector[m][3];
			}
			
			for(int i=0;i<4;i++){
				newPointVector[m][i] = newPointVector[m][i]/newPointVector[m][3];
			}
		}
		//---------------------------------------------------

		//-------------------trans for N---------------------
		for(int i=0;i<4;i++){
			for(int j=0;j<4;j++){
				if(i == j){
					normCombo[i][j] = 1;
					tempCombo[i][j] = 1;
				}
				else{
					normCombo[i][j] = 0;
					tempCombo[i][j] = 0;
				}
			}
		}

		for(int m=0;m<render->matlevel;m++){
			for(int i=0;i<4;i++){
				for(int j=0;j<4;j++){
					normCombo[i][j] = tempCombo[i][0]*(render->Xnorm[m][0][j]) 
									+ tempCombo[i][1]*(render->Xnorm[m][1][j]) 
									+ tempCombo[i][2]*(render->Xnorm[m][2][j]) 
									+ tempCombo[i][3]*(render->Xnorm[m][3][j]);
				}
			}

			for(int i=0;i<4;i++){
				for(int j=0;j<4;j++){
					tempCombo[i][j] = normCombo[i][j];
				}
			}
		}

		for(int m=0;m<3;m++){
			for(int i=0;i<4;i++){
				newNormVector[m][i] = normCombo[i][0]*oldNormVector[m][0]
									+ normCombo[i][1]*oldNormVector[m][1]
									+ normCombo[i][2]*oldNormVector[m][2]
									+ normCombo[i][3]*oldNormVector[m][3];
			}
			
			for(int i=0;i<4;i++){
				newNormVector[m][i] = newNormVector[m][i]/newNormVector[m][3];
			}
			
		}


		//---------------------------------------------------
		
		//------------ambient and vertex norm&color calculation-------------------
		
		
		//================ this part for HW4======================================= 

		if(render->tex_fun == NULL){

			for(int n=0;n<3;n++){		//for each vertex
				for(int i=0;i<3;i++){			//for R,G,B channel
					ambient[n][i] = render->Ka[i] * render->ambientlight.color[i];
					color[n][i] += ambient[n][i];
				}
			}
			for(int n=0;n<3;n++){		//for each vertex
			
				for(int m=0;m<render->numlights;m++){ //for every light
		
					NdotL = render->lights[m].direction[0]*newNormVector[n][0] + render->lights[m].direction[1]*newNormVector[n][1] + render->lights[m].direction[2]*newNormVector[n][2];
					NdotE = - newNormVector[n][2];
				
					//check for sign---------
					if(NdotL >=0 && NdotE >=0){
						//cal for R
						for(int i=0;i<3;i++){
							Rvector[i] = 2*NdotL*newNormVector[n][i] - render->lights[m].direction[i];
						}
						RdotE = -Rvector[2];
						for(int i=0;i<3;i++){			//for R,G,B channel
							diffuse[n][i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
						}
						for(int i=0;i<3;i++){
							specular[n][i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
						}
						for(int i=0;i<3;i++){
							color[n][i] += specular[n][i] + diffuse[n][i];
							//color[n][i] +=  specular[n][i];
						}

					}
				
					else if(NdotL < 0 && NdotE <0){
						//filp and cal
						NdotL = -(render->lights[m].direction[0]*newNormVector[n][0]) - render->lights[m].direction[1]*newNormVector[n][1] - render->lights[m].direction[2]*newNormVector[n][2];
						NdotE = newNormVector[n][2];
						for(int i=0;i<3;i++){
							Rvector[i] = - 2*NdotL*newNormVector[n][i] - render->lights[m].direction[i];
						}
						RdotE = -Rvector[2];
						for(int i=0;i<3;i++){			//for R,G,B channel
							diffuse[n][i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
						}
						for(int i=0;i<3;i++){
							specular[n][i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
						}
						for(int i=0;i<3;i++){
							color[n][i] += specular[n][i] + diffuse[n][i];
							//color[n][i] +=  specular[n][i];
						}
					}
				

					else{
						//in different plane, do nothing for this light			
					}

				}
				for(int i=0;i<3;i++){
					if(color[n][i] >1){
						color[n][i] = 1;
					}
					else if(color[n][i] <0){
						color[n][i] = 0;
					}
				}
			}
		}
		//==================== this part for HW4======================================= 
		

		//======================this part for HW5=================================
		else{
			for(int n=0;n<3;n++){		//for each vertex
				for(int i=0;i<3;i++){			//for R,G,B channel
					ambient[n][i] = render->ambientlight.color[i];
					color[n][i] += ambient[n][i];
				}
			}
			for(int n=0;n<3;n++){		//for each vertex
			
				for(int m=0;m<render->numlights;m++){ //for every light
		
					NdotL = render->lights[m].direction[0]*newNormVector[n][0] + render->lights[m].direction[1]*newNormVector[n][1] + render->lights[m].direction[2]*newNormVector[n][2];
					NdotE = - newNormVector[n][2];
				
					//check for sign---------
					if(NdotL >=0 && NdotE >=0){
						//cal for R
						for(int i=0;i<3;i++){
							Rvector[i] = 2*NdotL*newNormVector[n][i] - render->lights[m].direction[i];
						}
						RdotE = -Rvector[2];
						for(int i=0;i<3;i++){			//for R,G,B channel
							diffuse[n][i] = (render->lights[m].color[i])*NdotL;
						}
						for(int i=0;i<3;i++){
							specular[n][i] = (render->lights[m].color[i])*pow(RdotE,render->spec);
						}
						for(int i=0;i<3;i++){
							color[n][i] += specular[n][i] + diffuse[n][i];
						}

					}
				
					else if(NdotL < 0 && NdotE <0){
						//filp and cal
						NdotL = -(render->lights[m].direction[0]*newNormVector[n][0]) - render->lights[m].direction[1]*newNormVector[n][1] - render->lights[m].direction[2]*newNormVector[n][2];
						NdotE = newNormVector[n][2];
						for(int i=0;i<3;i++){
							Rvector[i] = - 2*NdotL*newNormVector[n][i] - render->lights[m].direction[i];
						}
						RdotE = -Rvector[2];
						for(int i=0;i<3;i++){			//for R,G,B channel
							diffuse[n][i] = (render->lights[m].color[i])*NdotL;
						}
						for(int i=0;i<3;i++){
							specular[n][i] = (render->lights[m].color[i])*pow(RdotE,render->spec);
						}
						for(int i=0;i<3;i++){
							color[n][i] += specular[n][i] + diffuse[n][i];
						}
					}
					//else{
						//in different plane, do nothing for this light			
					//}

				}
				for(int i=0;i<3;i++){
					if(color[n][i] >1){
						color[n][i] = 1;
					}
					else if(color[n][i] <0){
						color[n][i] = 0;
					}
				}
			}
		}
		//=====================this part for HW5======================================


		//----------------------call rasterizer-----------------
		if(newPointVector[0][2]>0 || newPointVector[1][2]>0 || newPointVector[1][2]>0){
			
			x[0] =  newPointVector[0][0] + render->xOffset;
			y[0] =  newPointVector[0][1] + render->yOffset;
			z[0] =  newPointVector[0][2];
			x[1] =  newPointVector[1][0] + render->xOffset;
			y[1] =  newPointVector[1][1] + render->yOffset;
			z[1] =  newPointVector[1][2];
			x[2] =  newPointVector[2][0] + render->xOffset;
			y[2] =  newPointVector[2][1] + render->yOffset;
			z[2] =  newPointVector[2][2];

			highX = getMax((int)x[0],(int)x[1],(int)x[2]);
			lowX = getMin((int)x[0],(int)x[1],(int)x[2]);
			highY = getMax((int)y[0],(int)y[1],(int)y[2]);
			lowY = getMin((int)y[0],(int)y[1],(int)y[2]);

			A = y[0]*(z[1]-z[2]) + y[1]*(z[2]-z[0]) + y[2]*(z[0]-z[1]);
			B = z[0]*(x[1] - x[2]) + z[1]*(x[2] - x[0]) + z[2]*(x[0] - x[1]);
			C = x[0]*(y[1] - y[2]) + x[1]*(y[2] - y[0]) + x[2]*(y[0] - y[1]);
			nD = x[0]*(y[1]*z[2] - y[2]*z[1]) + x[1]*(y[2]*z[0] - y[0]*z[2]) + x[2]*(y[0]*z[1] - y[1]*z[0]);

			Ar = y[0]*(color[1][0]- color[2][0]) + y[1]*(color[2][0]-color[0][0]) + y[2]*(color[0][0]-color[1][0]);
			Br = color[0][0]*(x[1] - x[2]) + color[1][0]*(x[2] - x[0]) + color[2][0]*(x[0] - x[1]);
			Cr = x[0]*(y[1] - y[2]) + x[1]*(y[2] - y[0]) + x[2]*(y[0] - y[1]);
			nDr = x[0]*(y[1]*color[2][0] - y[2]*color[1][0]) + x[1]*(y[2]*color[0][0] - y[0]*color[2][0]) + x[2]*(y[0]*color[1][0] - y[1]*color[0][0]);

			Ag = y[0]*(color[1][1]-color[2][1]) + y[1]*(color[2][1]-color[0][1]) + y[2]*(color[0][1]-color[1][1]);
			Bg = color[0][1]*(x[1] - x[2]) + color[1][1]*(x[2] - x[0]) + color[2][1]*(x[0] - x[1]);
			Cg = x[0]*(y[1] - y[2]) + x[1]*(y[2] - y[0]) + x[2]*(y[0] - y[1]);
			nDg = x[0]*(y[1]*color[2][1] - y[2]*color[1][1]) + x[1]*(y[2]*color[0][1] - y[0]*color[2][1]) + x[2]*(y[0]*color[1][1] - y[1]*color[0][1]);
			
			Ab = y[0]*(color[1][2]-color[2][2]) + y[1]*(color[2][2]-color[0][2]) + y[2]*(color[0][2]-color[1][2]);
			Bb = color[0][2]*(x[1] - x[2]) + color[1][2]*(x[2] - x[0]) + color[2][2]*(x[0] - x[1]);
			Cb = x[0]*(y[1] - y[2]) + x[1]*(y[2] - y[0]) + x[2]*(y[0] - y[1]);
			nDb = x[0]*(y[1]*color[2][2] - y[2]*color[1][2]) + x[1]*(y[2]*color[0][2] - y[0]*color[2][2]) + x[2]*(y[0]*color[1][2] - y[1]*color[0][2]);

			//================perspective correction ======================
			for(int i=0;i<3;i++){
				vZPrime[i] = z[i]/(INT_MAX - z[i]);
				newUVValue[i][0] = oldUVValue[i][0]/(vZPrime[i] + 1);
				newUVValue[i][1] = oldUVValue[i][1]/(vZPrime[i] + 1);
			}
			//=================perspective correction======================

			if(y[0] > y[1] && y[0] > y[2]){
				dX = x[0] - x[1];
				dY = y[0] - y[1];
				if( (dX*y[2]-dX*y[1]+dY*x[1])/dY > x[2]){
					//0,2,1
					//(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2])
					//(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1])
					//(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0]) >= 0){										
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;

										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){

											
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];
											
											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}

											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
				else{
					//0,1,2
					//(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1])
					//(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2])
					//(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{											//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}

											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}

											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];										
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				

				}

			}
			
			else if(y[1] > y[0] && y[1] > y[2]){
				dX = x[1] - x[2];
				dY = y[1] - y[2];
				if( (dX*y[0]-dX*y[2]+dY*x[2])/dY > x[0]){
					//1,0,2
					//(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0])
					//(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2])
					//(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}						
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				
				}
				else{
					//1,2,0
					//(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2])
					//(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0])
					//(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){

												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}		
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}		
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}

			}
			
			else if(y[2] > y[1] && y[2] > y[0]){
				dX = x[2] - x[0];
				dY = y[2] - y[0];
				if( (dX*y[1]-dX*y[0]+dY*x[0])/dY > x[1]){
					//2,1,0
					//(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1])
					//(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0])
					//(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{	
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}			
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				

				}
				else{
					//2,0,1
					//(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0])
					//(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1])
					//(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}	
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR),ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}		
			}
			
			else if(y[0] == y[1] && y[0] > y[2]){
				if(x[0] > x[1]){
					//1,2,0
					//(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2])
					//(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0])
					//(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}			
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR),ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
				else{
					//0,2,1
					//(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2])
					//(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1])
					//(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{	
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}		
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
			}
			else if(y[0] == y[2] && y[0] > y[1]){
				if(x[0] > x[2]){
					//2,1,0
					//(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1])
					//(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0])
					//(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{	
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
	
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}		
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j,ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
				else{
					//0,1,2
					//(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1])
					//(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2])
					//(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}			
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
			}
			else if(y[1] == y[2] && y[1] > y[0]){
				if(x[1] > x[2]){
					//2,0,1
					//(y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0])
					//(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1])
					//(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[2]-y[0])*(i-x[0])-(x[2]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[1])*(i-x[1])-(x[0]-x[1])*(j-y[1]) >= 0 &&
								(y[1]-y[2])*(i-x[2])-(x[1]-x[2])*(j-y[2]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}		
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j, ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
				else{
					//1,0,2
					//(y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0])
					//(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2])
					//(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1])
				
					for(int j=lowY;j<=highY;j++){
						for(int i=lowX;i<=highX;i++){
							if((y[1]-y[0])*(i-x[0])-(x[1]-x[0])*(j-y[0]) >= 0 &&
								(y[0]-y[2])*(i-x[2])-(x[0]-x[2])*(j-y[2]) >= 0 &&
								(y[2]-y[1])*(i-x[1])-(x[2]-x[1])*(j-y[1]) >= 0){
								if(i<render->display->xres && i>0 && j<render->display->yres && j>0){
									pos =  i + j*(render->display->xres);
									tempZ = (int)((nD - A*i - B*j)/C);	
									if(tempZ < (((render->display->fbuf)+pos)->z)){
										alpha = ((y[1]-y[2])*(i-x[2]) + (x[2]-x[1])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										beta = ((y[2]-y[0])*(i-x[2]) + (x[0]-x[2])*(j-y[2]))/((y[1]-y[2])*(x[0]-x[2]) + (x[2]-x[1])*(y[0]-y[2]));
										gama = 1 - alpha - beta;
										if(render->interp_mode == GZ_COLOR){
											if(render->tex_fun == NULL){
												finalR = alpha*color[0][0] + beta*color[1][0] + gama*color[2][0];
												finalG = alpha*color[0][1] + beta*color[1][1] + gama*color[2][1];
												finalB = alpha*color[0][2] + beta*color[1][2] + gama*color[2][2];											
											}
											else{												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												//=================perspective correction======================
												/*
												finalR = (nDr - Ar*i - Br*j)/Cr;
												finalG = (nDg - Ag*i - Bg*j)/Cg;
												finalB = (nDb - Ab*i - Bb*j)/Cb;
												*/						
												finalR = pTexture[0] * (alpha*color[0][0] + beta*color[1][0] + gama*color[2][0]);
												finalG = pTexture[1] * (alpha*color[0][1] + beta*color[1][1] + gama*color[2][1]);
												finalB = pTexture[2] * (alpha*color[0][2] + beta*color[1][2] + gama*color[2][2]);
											}
										}
										else if(render->interp_mode == GZ_NORMALS){
											pointNorm[0] = alpha*newNormVector[0][0] + beta*newNormVector[1][0] + gama*newNormVector[2][0];
											pointNorm[1] = alpha*newNormVector[0][1] + beta*newNormVector[1][1] + gama*newNormVector[2][1];
											pointNorm[2] = alpha*newNormVector[0][2] + beta*newNormVector[1][2] + gama*newNormVector[2][2];

											pScale = sqrt(pointNorm[0]*pointNorm[0] + pointNorm[1]*pointNorm[1] + pointNorm[2]*pointNorm[2]);
											pointNorm[0] = pointNorm[0]/pScale;
											pointNorm[1] = pointNorm[1]/pScale;
											pointNorm[2] = pointNorm[2]/pScale;
											if(render->tex_fun != NULL){
												//================perspective correction ======================
												pU = alpha*newUVValue[0][0] + beta*newUVValue[1][0] + gama*newUVValue[2][0];
												pV = alpha*newUVValue[0][1] + beta*newUVValue[1][1] + gama*newUVValue[2][1];
												interpZ = alpha*z[0] + beta*z[1] + gama*z[2];
												pU = pU*((interpZ/(INT_MAX - interpZ)) + 1);
												pV = pV*((interpZ/(INT_MAX - interpZ)) + 1);
												render->tex_fun(pU,pV,pTexture);
												render->Kd[0] = pTexture[0];
												render->Kd[1] = pTexture[1];
												render->Kd[2] = pTexture[2];
												render->Ka[0] = pTexture[0];
												render->Ka[1] = pTexture[1];
												render->Ka[2] = pTexture[2];
												//=================perspective correction======================
											}
											for(int i=0;i<3;i++){
												pColor[i] = 0;
											}
											for(int i=0;i<3;i++){			//for R,G,B channel				
												pColor[i] += render->Ka[i] * render->ambientlight.color[i];
											}

											for(int m=0;m<render->numlights;m++){ //for every light
		
												NdotL = render->lights[m].direction[0]*pointNorm[0] + render->lights[m].direction[1]*pointNorm[1] + render->lights[m].direction[2]*pointNorm[2];
												NdotE = - pointNorm[2];
				
												//check for sign---------
												if(NdotL >=0 && NdotE >=0){
													//cal for R
													for(int i=0;i<3;i++){
														Rvector[i] = 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
												else if(NdotL < 0 && NdotE <0){
													//filp and cal
													NdotL = -(render->lights[m].direction[0]*pointNorm[0]) - render->lights[m].direction[1]*pointNorm[1] - render->lights[m].direction[2]*pointNorm[2];
													NdotE = pointNorm[2];
													for(int i=0;i<3;i++){
														Rvector[i] = - 2*NdotL*pointNorm[i] - render->lights[m].direction[i];
													}
													RdotE = -Rvector[2];
													for(int i=0;i<3;i++){			//for R,G,B channel
														pDiffuse[i] = (render->Kd[i] * render->lights[m].color[i])*NdotL;
													}
													for(int i=0;i<3;i++){
														pSpecular[i] = (render->Ks[i] * render->lights[m].color[i])*pow(RdotE,render->spec);
													}
													for(int i=0;i<3;i++){
														pColor[i] += pSpecular[i] + pDiffuse[i];
														//color[n][i] +=  specular[n][i];
													}
												}
				
												/*
												else{
													//in different plane, do nothing for this light			
												}
												*/
											}
											for(int i=0;i<3;i++){
												if(pColor[i] >1){
													pColor[i] = 1;
												}
												else if(pColor[i] <0){
													pColor[i] = 0;
												}
											}		
											finalR = pColor[0];
											finalG = pColor[1];
											finalB = pColor[2];
										}
										GzPutDisplay( render->display, i,  j,ctoi(finalR), ctoi(finalG),ctoi(finalB), 1, tempZ);
									}
								}
							}
						}
					}
				
				}
			}
			
		//drawLine(render,(int)newPointVector[0][0], (int)newPointVector[0][1],(int)newPointVector[1][0],(int)newPointVector[1][1]);
		//drawLine(render,(int)newPointVector[1][0], (int)newPointVector[1][1],(int)newPointVector[2][0],(int)newPointVector[2][1]);
		//drawLine(render,(int)newPointVector[2][0], (int)newPointVector[2][1],(int)newPointVector[0][0],(int)newPointVector[0][1]);

		}
		
		//drawLine(render,(int) x[1], (int)y[1],(int)x[2],(int)y[2]);
		//drawLine(render,(int)x[2], (int)y[2],(int)x[0],(int)y[0]);

		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}

int getMax (int a, int b, int c){
	int temp = 0;
	if(a>temp){
		temp = a;
	}
	if(b>temp){
		temp = b;
	}
	if(c>temp){
		temp = c;
	}
	return temp;
}

int getMin (int a, int b, int c){
	int temp = INT_MAX;
	if(a<temp){
		temp = a;
	}
	if(b<temp){
		temp = b;
	}
	if(c<temp){
		temp = c;
	}
	return temp;
}

void drawLine(GzRender *render ,int x0, int y0, int x1, int y1)
{
	int slope;
	int dx, dy, incE, incNE, d, x, y;
	/*
	if(x0>x1){
		minX = x1;
		maxX = x0;
	}
	else{
		minX = x0;
		maxX = x1;
	}
	if(y0>y1){
		minY = y1;
		maxY = y0;
	}
	else{
		minY = y0;
		maxY = y1;
	}
	
	for (int i= minX;i<=maxX;i++){
		for(int j=minY;j<=maxY;j++){
			GzPutDisplay( render->display,  i,  j, 4095, 4095,4095, 1, 0);
		}
	}
	*/
	
	if(abs(x1-x0) >= abs(y1-y0)){
		// Reverse lines where x1 > x2
		if (x0 > x1)
		{
			drawLine(render,x1, y1, x0, y0);
			return;
		}
		dx = x1 - x0;
		dy = y1 - y0;
		// Adjust y-increment for negatively sloped lines
		if (dy < 0)
		{
			slope = -1;
			dy = -dy;
		}
		else
		{
			slope = 1;
		}
		// Bresenham constants
		incE = 2 * dy;
		incNE = 2 * dy - 2 * dx;
		d = 2 * dy - dx;
		y = y0;
		// Blit
		for (int x = x0; x <= x1; x++)
		{
			GzPutDisplay( render->display,  x,  y, 4095, 4095,4095, 1, 0);
			if (d <= 0){
				d += incE;
			}
			else{
				d += incNE;
				y += slope;
			}
		}
		return;
	}
	
	else{
		
		if (y0 > y1)
		{
			drawLine(render,x1, y1, x0, y0);
			return;
		}
		dx = x1 - x0;
		dy = y1 - y0;
		// Adjust y-increment for negatively sloped lines
		if (dx < 0)
		{
			slope = -1;
			dx = -dx;
		}
		else
		{
			slope = 1;
		}
		// Bresenham constants
		incE = 2 * dx;
		incNE = 2 * dx - 2 * dy;
		d = 2 * dx - dy;
		x = x0;
		// Blit
		for (int y = y0; y <= y1; y++)
		{
			GzPutDisplay( render->display,  x, y, 4095, 4095,4095, 1, 0);
			if (d <= 0){
				d += incE;
			}
			else{
				d += incNE;
				x += slope;
			}
		}
		return;
	
	}
	
	
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

