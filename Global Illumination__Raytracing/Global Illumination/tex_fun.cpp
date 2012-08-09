/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"
#include   <math.h>

#define PI 3.1415927	

GzColor	*image;
int xs, ys;
int reset = 1;

double Noise(float x, float y);
double SmoothNoise(float x, float y);
double InterpolatedNoise(float x, float y);
double CosineInterpolation(float a, float b, float x);

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */

  //we can either clamp, mirror or tile (repeat) while bounds checking
  //here i shall be using tiling since the final image given is a tiled one
  if (u < 0)
	  u *= -1;

  if(u > 1)		//so in case u = -2.5 we need to store only 0.5 so thats y we make it positive and make a 2nd comparison
  {
	  u = u - (int)u;	//just the fractional part is needed
  }

  if(v < 0)
	  v *= -1;

  if(v > 1)
  {
	  v = v - (int)v;
  }

/* determine texture cell corner values and perform bilinear interpolation */

  //now mutliply u,v by the texture size so as to get the texel location
  float texelU, texelV;
  texelU = u * (xs - 1);
  texelV = v * (ys - 1);

  //now perform bilinear interpolation
  //before that we need to get the surrouding pixels ( the bounding pixels or texels technically )
  int texU1, texV1, texU2, texV2;
  
  texU1 = (int)texelU;
  texV1 = (int)texelV;

  texU2 = (texU1 + 1);	//check if we need to %xs if texU2 crosses the boundary
  texV2 = (texV1 + 1);

  //we need to calculate s,t for bilinear interpolation
  float s,t;
  s = texelU - texU1;
  t = texelV - texV1;

  int colorIdxA, colorIdxB, colorIdxC, colorIdxD;
  colorIdxA = texU1 + (texV1 * xs);	
  colorIdxB = texU2 + (texV1 * xs);
  colorIdxC = texU2 + (texV2 * xs);
  colorIdxD = texU1 + (texV2 * xs);

  //now apply the bilinear interpolation eqn: Color(p) = s*t*C + (1-s)*t*D + s*(1-t)*B + (1-s)(1-t)*A
  /* set color to interpolated GzColor value and return */
  color[RED] = (s * t * image[colorIdxC][RED]) + ((1-s) * t * image[colorIdxD][RED]) + (s * (1-t) * image[colorIdxB][RED]) + ((1-s) * (1-t) * image[colorIdxA][RED]);
  color[GREEN] = (s * t * image[colorIdxC][GREEN]) + ((1-s) * t * image[colorIdxD][GREEN]) + (s * (1-t) * image[colorIdxB][GREEN]) + ((1-s) * (1-t) * image[colorIdxA][GREEN]);
  color[BLUE] = (s * t * image[colorIdxC][BLUE]) + ((1-s) * t * image[colorIdxD][BLUE]) + (s * (1-t) * image[colorIdxB][BLUE]) + ((1-s) * (1-t) * image[colorIdxA][BLUE]);

  return GZ_SUCCESS;
}


/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{

	//write ur own function
	if(reset)
	{
		//set the size
		xs = 256;
		ys = 256;

		image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
		if (image == NULL) {
			fprintf (stderr, "malloc for texture image failed\n");
			exit(-1);
			}	

		float noise;

		for(int i = 0; i < (xs * ys); i++)
		{	
			//use wave equation to generate the noise
			//noise = InterpolatedNoise(u,v);
			image[i][RED] =    CosineInterpolation(u,v,i*u*v) * color[RED] ;	
			image[i][GREEN] =   CosineInterpolation(u,v,i*u/v) * color[GREEN] * InterpolatedNoise(i,u/v);
			image[i][BLUE] =    Noise(i,u) * color[BLUE] * SmoothNoise(u,i*v);
		}

		reset = 0;
	}

	/* bounds-test u,v to make sure nothing will overflow image array bounds */

  //we can either clamp, mirror or tile (repeat) while bounds checking
  //here i shall be using tiling since the final image given is a tiled one
  if (u < 0)
	  u *= -1;

  if(u > 1)		//so in case u = -2.5 we need to store only 0.5 so thats y we make it positive and make a 2nd comparison
  {
	  u = u - (int)u;	//just the fractional part is needed
  }

  if(v < 0)
	  v *= -1;

  if(v > 1)
  {
	  v = v - (int)v;
  }

/* determine texture cell corner values and perform bilinear interpolation */

  //now mutliply u,v by the texture size so as to get the texel location
  float texelU, texelV;
  texelU = u * (xs - 1);
  texelV = v * (ys - 1);

  //now perform bilinear interpolation
  //before that we need to get the surrouding pixels ( the bounding pixels or texels technically )
  int texU1, texV1, texU2, texV2;
  
  texU1 = (int)texelU;
  texV1 = (int)texelV;

  texU2 = (texU1 + 1);	//check if we need to %xs if texU2 crosses the boundary
  texV2 = (texV1 + 1);

  //we need to calculate s,t for bilinear interpolation
  float s,t;
  s = texelU - texU1;
  t = texelV - texV1;

  int colorIdxA, colorIdxB, colorIdxC, colorIdxD;
  colorIdxA = texU1 + (texV1 * xs);	
  colorIdxB = texU2 + (texV1 * xs);
  colorIdxC = texU2 + (texV2 * xs);
  colorIdxD = texU1 + (texV2 * xs);

  //now apply the bilinear interpolation eqn: Color(p) = s*t*C + (1-s)*t*D + s*(1-t)*B + (1-s)(1-t)*A
  /* set color to interpolated GzColor value and return */
  color[RED] = (s * t * image[colorIdxC][RED]) + ((1-s) * t * image[colorIdxD][RED]) + (s * (1-t) * image[colorIdxB][RED]) + ((1-s) * (1-t) * image[colorIdxA][RED]);
  color[GREEN] = (s * t * image[colorIdxC][GREEN]) + ((1-s) * t * image[colorIdxD][GREEN]) + (s * (1-t) * image[colorIdxB][GREEN]) + ((1-s) * (1-t) * image[colorIdxA][GREEN]);
  color[BLUE] = (s * t * image[colorIdxC][BLUE]) + ((1-s) * t * image[colorIdxD][BLUE]) + (s * (1-t) * image[colorIdxB][BLUE]) + ((1-s) * (1-t) * image[colorIdxA][BLUE]);

	return GZ_SUCCESS;
}

double Noise(float x, float y)
{
	double n = sin(x + y * PI);
	n = pow(n,2);

	return n;
}

double SmoothNoise(float x, float y)
{
	double corners = ( Noise(x-1, y-1) + Noise(x+1,y-1) + Noise(x-1,y+1) + Noise(x+1,y+1)) / 16;
	double sides   = (Noise(x-1,y) + Noise(x+1,y) + Noise(x,y-1) + Noise(x,y+1)) / 8;
	double center  =  Noise(x,y) / 4;

	return (corners + sides + center);
}

double InterpolatedNoise(float x, float y)
{
	int intX, intY;
	float fracX, fracY;

	intX = (int)x;
	intY = (int)y;

	fracX = x - intX;
	fracY = y - intY;

	float v1, v2 ,v3, v4;
	v1 = SmoothNoise(intX, intY);
	v2 = SmoothNoise(intX + 1, intY);
	v3 = SmoothNoise(intX, intY + 1);
	v4 = SmoothNoise(intX + 1, intY + 1);

	double i1, i2;
	i1 = CosineInterpolation(v1,v2,fracX);
	i2 = CosineInterpolation(v3,v4,fracX);

	return CosineInterpolation(i1,i2,fracY);
}

double CosineInterpolation(float a, float b, float x)
{
	double ft = x * PI;
	double f = (1 - cos(ft)) * 0.5;

	return( a * (1-f) + (b * f));
}
