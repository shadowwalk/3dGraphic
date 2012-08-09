/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/
	unsigned int iBufferSize = width * height * 3;//sizeof(GzPixel);	//change this to 3 x width x height

	//now that we have got the Buffer Size, we can allocate this memory to our frame buffer
	(*framebuffer) = new char[iBufferSize];

	if((*framebuffer) == NULL)
		return GZ_FAILURE;

	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{

/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/
	(*display) = new GzDisplay();

	(*display)->dispClass = dispClass;
	(*display)->open = 0;

	if(xRes < MAXXRES)
		(*display)->xres = xRes;
	else
		(*display)->xres = MAXXRES;

	if(yRes < MAXYRES)
		(*display)->yres = yRes;
	else
		(*display)->yres = MAXYRES;

	(*display)->fbuf = new GzPixel[(*display)->xres * (*display)->yres];    //could also write xRes * yRes

	if((*display) == NULL)
		return GZ_FAILURE;

	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */

	delete [] display->fbuf;
	display->fbuf = NULL;

	delete display;
	display = NULL;

	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */

	//add a check if display is open

		*xRes = display->xres;
		*yRes = display->yres;
		*dispClass = display->dispClass;

		return GZ_SUCCESS;	
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */

/*	if(display->open == 1) //already open
		{
			AfxMessageBox("Display already opened",MB_OK);
			return GZ_SUCCESS;
		}
*/
	display->open = 1;
	

	for(int i = 0; i < (display->xres * display->yres); i++)
	{
		display->fbuf[i].alpha = 255;
		display->fbuf[i].z = INT_MAX;		//initializing z-buffer to INT_MAX
		display->fbuf[i].blue =  0.1 * MAXCOLOR;			//we right shift by 4 because in our framebuffer we can only store 8 bits. So we need to convert from 12 bits to 8 bits
		display->fbuf[i].green =  0.1 * MAXCOLOR;
		display->fbuf[i].red =  0.1 * MAXCOLOR;
	}
	


	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	
	if((i < 0 || i >= display->xres) || (j < 0 || j >= display->yres))   //bounds checking  (earlier was giving an access violation)
	   return GZ_FAILURE;

	int iFBufindex = ARRAY(i,j);

	if(r < 0)
		r = 0;
	if(g < 0)
		g = 0;
	if(b < 0)
		b = 0;

	display->fbuf[iFBufindex].alpha = a;
	display->fbuf[iFBufindex].z = z;
	
	if(r <= MAXCOLOR)
		display->fbuf[iFBufindex].red = r >> 4;
	else
		display->fbuf[iFBufindex].red = MAXCOLOR >> 4;

	if(g <= MAXCOLOR)
		display->fbuf[iFBufindex].green = g >> 4;
	else
		display->fbuf[iFBufindex].green = MAXCOLOR >> 4;
	
	if(b <= MAXCOLOR)
		display->fbuf[iFBufindex].blue = b >> 4;
	else
		display->fbuf[iFBufindex].blue = MAXCOLOR >> 4; 



	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */
	
	if((i < 0 || i >= display->xres) || (j < 0 || j >= display->yres))
		return GZ_FAILURE;

	int iFbufindex = ARRAY(i,j);
	
	*r = display->fbuf[iFbufindex].red;		
	*g = display->fbuf[iFbufindex].green;
	*b = display->fbuf[iFbufindex].blue;
	*a = display->fbuf[iFbufindex].alpha;
	*z = display->fbuf[iFbufindex].z;

	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	//first write the header
	fprintf(outfile,"P6 %d %d 255\r",display->xres,display->yres);

	for(int i = 0; i < (display->xres * display->yres); i++)
	{
		fprintf(outfile,"%c%c%c",display->fbuf[i].red,display->fbuf[i].green,display->fbuf[i].blue);			//check the output and then add \n if necessary - might get truncated cuz the size of char and GzIntensity are different; check that
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

	int iFrameBufferSize = display->xres * display->yres;
	int i = 0,j = 0;

	while(j < iFrameBufferSize)
	{
		framebuffer[i++] = (char)display->fbuf[j].blue;     //need to store the pixels in bgr format not in rgb format
		framebuffer[i++] = (char)display->fbuf[j].green;
		framebuffer[i++] = (char)display->fbuf[j].red;		
		j++;
	}
	

	return GZ_SUCCESS;
}


int PutPixel(GzDisplay* display, int x, int y,int z, GzIntensity r, GzIntensity g, GzIntensity b)
{

	if((x < 0 || x >= display->xres) || (y < 0 || y >= display->yres))
		return GZ_FAILURE;

	//before putting the pixel on the display, check the z-buffer for hidden surface removal
	int frameBufferIndex = ARRAY(x,y);

	if(z < display->fbuf[frameBufferIndex].z) 
	{
		GzPutDisplay(display,(int)x,(int)y,r,g,b,255,z); //updating the z buffer by passing the new value
	}

	return GZ_SUCCESS;
}

int PutPixel(GzDisplay* display, int x, int y, GzIntensity r, GzIntensity g, GzIntensity b)
{
	if((x < 0 || x >= display->xres) || (y < 0 || y >= display->yres))
		return GZ_FAILURE;

	int frameBufferIndex = ARRAY(x,y);

	if(r < 0)
		r = 0;
	if(g < 0)
		g = 0;
	if(b < 0)
		b = 0;
	
	display->fbuf[frameBufferIndex].alpha = 255;
	display->fbuf[frameBufferIndex].red = r;
	display->fbuf[frameBufferIndex].green = g;
	display->fbuf[frameBufferIndex].blue = b;

/*	if(r <= MAXCOLOR)
		display->fbuf[frameBufferIndex].red = r >> 4;
	else
		display->fbuf[frameBufferIndex].red = MAXCOLOR >> 4;

	if(g <= MAXCOLOR)
		display->fbuf[frameBufferIndex].green = g >> 4;
	else
		display->fbuf[frameBufferIndex].green = MAXCOLOR >> 4;
	
	if(b <= MAXCOLOR)
		display->fbuf[frameBufferIndex].blue = b >> 4;
	else
		display->fbuf[frameBufferIndex].blue = MAXCOLOR >> 4; 
*/
	return GZ_SUCCESS;
}