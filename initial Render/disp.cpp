/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)RGB x width x height
 -- pass back pointer 
*/	
	if(width<=MAXXRES && height<=MAXYRES){
		*framebuffer = (char*)malloc(3*sizeof(char)*width*height);

		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{

/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/
	if(xRes<=MAXXRES && yRes<=MAXYRES){
		*display = (GzDisplay*)malloc(sizeof(GzDisplay));
		(**display).xres = xRes;
		(**display).yres = yRes;
		(**display).dispClass = dispClass;
		(**display).fbuf = (GzPixel*)malloc(sizeof(GzPixel)*xRes*yRes);
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	if(display != NULL){
		free(display);
	
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */
	if(display != NULL){
		*xRes = display->xres;
		*yRes = display->yres;
		*dispClass = display->dispClass;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	if(display != NULL){
		for(int i=0;i<display->xres;i++){
			for(int j=0;j<display->yres;j++){
				((display->fbuf)+ARRAY(i,j))->red = 255;
				((display->fbuf)+ARRAY(i,j))->green = 255;
				((display->fbuf)+ARRAY(i,j))->blue = 1023;
				((display->fbuf)+ARRAY(i,j))->alpha = 1; 
				((display->fbuf)+ARRAY(i,j))->z = INT_MAX;

			}
		}
	
		display->open = 1;
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}

}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if(display != NULL){
		if(i >= 0 && i < display->xres && j>=0 && j<display->yres){
			if(r > 4095){
				((display->fbuf)+ARRAY(i,j))->red = 4095;
			}
			else if(r < 0){
				((display->fbuf)+ARRAY(i,j))->red = 0;
			}
			else{
				((display->fbuf)+ARRAY(i,j))->red = r;
			}

			if(g > 4095){
				((display->fbuf)+ARRAY(i,j))->green = 4095;
			}
			else if(g < 0){
				((display->fbuf)+ARRAY(i,j))->green = 0;
			}
			else{
				((display->fbuf)+ARRAY(i,j))->green = g;
			}

			if(b > 4095){
				((display->fbuf)+ARRAY(i,j))->blue = 4095;
			}
			else if(b < 0){
				((display->fbuf)+ARRAY(i,j))->blue = 0;
			}
			else{
				((display->fbuf)+ARRAY(i,j))->blue = b;
			}


			//((display->fbuf)+ARRAY(i,j))->green = g;
			//((display->fbuf)+ARRAY(i,j))->blue = b;
			((display->fbuf)+ARRAY(i,j))->alpha = a;
			
			//if(z>0 && z<INT_MAX){
				((display->fbuf)+ARRAY(i,j))->z = z;
			//}
		}
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */
	if(display != NULL){
		if(i >= 0 && i < display->xres && j>=0 && j<display->yres){
			*r = ((display->fbuf)+ARRAY(i,j))->red;
			*g = ((display->fbuf)+ARRAY(i,j))->green ;
			*b = ((display->fbuf)+ARRAY(i,j))->blue ;
			*a = ((display->fbuf)+ARRAY(i,j))->alpha;
			*z = ((display->fbuf)+ARRAY(i,j))->z;
		}
		return GZ_SUCCESS;
	}
	else{
		return GZ_FAILURE;
	}

	
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	fprintf(outfile,"P6\n%d %d\n255\n",display->xres,display->yres);
	//fprintf(outfile,"P6 %d %d 255 \n\r",display->xres,display->yres);
	for(int j=0;j<display->yres;j++){
		for(int i=0;i<display->xres;i++){
			//(display->fbuf + ARRAY(i,j))->red >> 4;
			//(display->fbuf + ARRAY(i,j))->green >> 4;
			//(display->fbuf + ARRAY(i,j))->blue >> 4;
			//printf("%c%c%c",((display->fbuf + ARRAY(i,j))->blue)>>4,((display->fbuf + ARRAY(i,j))->red)>>4 ,(( display->fbuf + ARRAY(i,j))->green ))>>4;
			//fprintf(outfile,"%c%c%c",((display->fbuf + ARRAY(i,j))->blue)>>4,((display->fbuf + ARRAY(i,j))->red)>>4 ,(( display->fbuf + ARRAY(i,j))->green )>>4);
			fprintf(outfile,"%c%c%c",((display->fbuf + ARRAY(i,j))->red)>>4,((display->fbuf + ARRAY(i,j))->green)>>4 ,(( display->fbuf + ARRAY(i,j))->blue )>>4);
			//fprintf(outfile,"%c%c%c",0,255,0);
			
		}
		//fprintf(outfile,"\r");
	}
	

	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/
	for(int j=0;j<display->yres;j++){
		for(int i=0;i<display->xres;i++){
			*(framebuffer + 3*ARRAY(i,j)) = (char)(((display->fbuf + ARRAY(i,j))->blue)>>4); 
			*(framebuffer + 3*ARRAY(i,j) + 1) = (char)(((display->fbuf + ARRAY(i,j))->green)>>4); 
			*(framebuffer + 3*ARRAY(i,j) + 2) = (char)(((display->fbuf + ARRAY(i,j))->red)>>4); 
		}
	
	}


	return GZ_SUCCESS;
}