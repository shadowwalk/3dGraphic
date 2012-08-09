////////////// Contains the descriptions of the some of the functions used /////////////////
///i have used comments in most of the functions but still here is a brief summary //////////

In rend.h 

int ScanLine(GzRender *render, GzCoord* vertexList, GzCoord* normalList,GzTextureIndex* uvList);	//feeds the color or normals to the rasterizer and also does the sorting of vertices
int ScanLineRaster(GzRender* render,GzCoord* pVertex, float xmin,float xmax,float zmin,float zmax,GzCoord* normalList,GzTextureIndex* uvList,GzColor* specList,GzColor* diffList); //takes the colors separately now

//Vertex shader function has been modified so as to incorporate Gouraud Shading after appplying the texture map since it required delayed multiplication
void VertexShader(GzRender *render, GzCoord normal, GzColor* specular, GzColor* diffuse);

//does the delayed multiplication
void MultiplyColor(GzRender *render, GzColor specular, GzColor diffuse, GzColor* resultant);

in tex_fun.cpp

double Noise(float x, float y);	//generates random values based on the sine function (modified wave equation y = Asin(wt)
double SmoothNoise(float x, float y);	//applies noise functions at corner, side and center based on the input
double InterpolatedNoise(float x, float y); // interpolates the noise output received from smoothnoise function using cosine Interpolation
double CosineInterpolation(float a, float b, float x); //performs cosine interpolation