/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"
#include	"math.h"

GzColor	*image;
int xs, ys;
int reset = 1;

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
	unsigned char		pixel[3];
	unsigned char     dummy;
	char  		foo[8];
	int   		i, j;
	FILE			*fd;

	float px,py,s,t;
	int Ax,Ay,Cx,Cy;

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

	//if(u>=0 && u<=1 && v>=0 && v<=1){
	if(u<0){
		u = 0;
	}
	if(v<0){
		v = 0;
	}
	if(u>1){
		u = 1;
	}
	if(v>1){
		v = 1;
	}
		px = u*(xs-1);
		py = v*(ys-1);

		Ax = floor(px);
		Ay = floor(py);
		Cx = Ax + 1;
		Cy = Ay + 1;
		s = px - Ax;
		t = py - Ay;
		
		color[RED] = s*t*image[xs*Cy + Cx][RED] + (1-s)*t*image[xs*Cy + Ax][RED]
					+ s*(1-t)*image[xs*Ay + Cx][RED] + (1-s)*(1-t)*image[xs*Ay + Ax][RED];
		color[GREEN] = s*t*image[xs*Cy + Cx][GREEN] + (1-s)*t*image[xs*Cy + Ax][GREEN]
					+ s*(1-t)*image[xs*Ay + Cx][GREEN] + (1-s)*(1-t)*image[xs*Ay + Ax][GREEN];
		color[BLUE] = s*t*image[xs*Cy + Cx][BLUE] + (1-s)*t*image[xs*Cy + Ax][BLUE]
					+ s*(1-t)*image[xs*Ay + Cx][BLUE] + (1-s)*(1-t)*image[xs*Ay + Ax][BLUE];

/* bounds-test u,v to make sure nothing will overflow image array bounds */
/* determine texture cell corner values and perform bilinear interpolation */
/* set color to interpolated GzColor value and return */
	return 0;
}


/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
	float px,py;

	float iterX,iterY;
	float length;
	float colorLUT[3][11];
	float a,b;

	if(u<0){
		u = 0;
	}
	if(v<0){
		v = 0;
	}
	if(u>1){
		u = 1;
	}
	if(v>1){
		v = 1;
	}


	//------------LUT initialize--------------
	colorLUT[0][0] = 0;
	colorLUT[1][0] = 255;
	colorLUT[2][0] = 102;
	colorLUT[0][1] = 30;
	colorLUT[1][1] = 210;
	colorLUT[2][1] = 90;
	colorLUT[0][2] = 60;
	colorLUT[1][2] = 190;
	colorLUT[2][2] = 80;
	colorLUT[0][3] = 90;
	colorLUT[1][3] = 170;
	colorLUT[2][3] = 70;
	colorLUT[0][4] = 120;
	colorLUT[1][4] = 150;
	colorLUT[2][4] = 60;
	colorLUT[0][5] = 150;
	colorLUT[1][5] = 120;
	colorLUT[2][5] = 50;
	colorLUT[0][6] = 180;
	colorLUT[1][6] = 100;
	colorLUT[2][6] = 40;
	colorLUT[0][7] = 200;
	colorLUT[1][7] = 90;
	colorLUT[2][7] = 30;
	colorLUT[0][8] = 220;
	colorLUT[1][8] = 70;
	colorLUT[2][8] = 20;
	colorLUT[0][9] = 250;
	colorLUT[1][9] = 51;
	colorLUT[2][9] = 10;
	colorLUT[0][10] = 50;
	colorLUT[1][10] = 50;
	colorLUT[2][10] = 50;


	//================chess board texture====================
	/*
	px = u*4;
	py = v*4;

	if(((int)px % 2) == ((int)py % 2)){
		color[RED] = 0.8;
		color[GREEN] = 0.8;
		color[BLUE] = 0.8;	
	}
	else{
		color[RED] = 0.2;
		color[GREEN] = 0.2;
		color[BLUE] = 0.2;
	}
	*/
	//================chess board texture====================

	//================julia set texture========================
	
	

	px = 3 * (u - 0.5); //-1.5 to 1.5
	py = 2 * (v - 0.5);	//-1 to 1
	
	if(px<0 && py < 0){
		px = px*2 + 1.5;
		py = py*2 + 1;
	}
	else if (px < 0 && py >= 0){
		px = px*2 + 1.5;
		py = py*2 - 1;
	}
	else if (px >= 0 && py < 0){
		px = px*2 - 1.5;
		py = py*2 + 1;
	}
	else if (px >= 0 && py >= 0){
		px = px*2 - 1.5;
		py = py*2 - 1;
	}
	
#if 0	
	iterX = px*px - 0.12375;
	iterY = py*py + 0.56805;
			
	length = 0;
	for (int i=0;i<200;i++){
		iterX = iterX*iterX - iterY*iterY - 0.12375;
		iterY = 2*iterX*iterY +  0.56805;
		if(sqrt(iterX*iterX + iterY*iterY) > 20){
			length = 1;
			break;
		}
	}
	if(length != 1){
		length = sqrt(iterX*iterX + iterY*iterY);
	}
		
	//range 0.4-0.6, for 1st one
	length = 10*((length - 0.4)/0.2);// now length from 0 to 10

#elif 0

	
	iterX = px*px - 0.0123;
	iterY = py*py + 0.745;
			
	length = 0;
	for (int i=0;i<500;i++){
		iterX = iterX*iterX - iterY*iterY - 0.0123;
		iterY = 2*iterX*iterY +  0.745;
		if(sqrt(iterX*iterX + iterY*iterY) > 20){
			length = 1;
			break;
		}
	}
	if(length != 1){
		length = sqrt(iterX*iterX + iterY*iterY);
	}
		
	//range 0.4-0.9, for 2nd one
	length = 10*((length - 0.4)/0.5);// now length from 0 to 10
#elif 1
	iterX = px*px - 0.75;
	iterY = py*py;
			
	length = 0;
	for (int i=0;i<200;i++){
		iterX = iterX*iterX - iterY*iterY - 0.75;
		iterY = 2*iterX*iterY;
		if(sqrt(iterX*iterX + iterY*iterY) > 20){
			length = 1;
			break;
		}
	}
	if(length != 1){
		length = sqrt(iterX*iterX + iterY*iterY);
	}
		
	//range 0.4-0.9, for 3rd one
	length = 10*((length - 0.4)/0.5);// now length from 0 to 10	
#endif

	
	if(length < 0){
		length = 0;
	}
	if(length > 10){
		length = 10;
	}
	
	a = (int)length + 1 - length;
	b = length - (int)length;

	color[RED] = (a * colorLUT[0][(int)length] + b * colorLUT[0][(int)length + 1])/255;
	color[GREEN] = (a * colorLUT[1][(int)length] + b * colorLUT[1][(int)length + 1])/255;
	color[BLUE] = (a * colorLUT[2][(int)length] + b * colorLUT[2][(int)length + 1])/255;
	//================julia set texture========================

	return 0;
}

