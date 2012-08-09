#ifndef GZRENDER
#define GZRENDER

#include <vector>
#include "disp.h" /* include your own disp.h file (e.g. hw1)*/

using namespace std;

/* Camera defaults */
#define	DEFAULT_FOV		55.0
#define	DEFAULT_IM_Z	(-10.0)  /* world coords for image plane origin */
#define	DEFAULT_IM_Y	(5.0)    /* default look-at point = 0,0,0 */
#define	DEFAULT_IM_X	(-10.0)

#define	DEFAULT_AMBIENT	{0.1, 0.1, 0.1}
#define	DEFAULT_DIFFUSE	{0.7, 0.6, 0.5}
#define	DEFAULT_SPECULAR	{0.2, 0.3, 0.4}
#define	DEFAULT_SPEC		32

#define	MATLEVELS	100		/* how many matrix pushes allowed */
#define	MAX_LIGHTS	10		/* how many lights allowed */

#define PI 3.14		// pi value to convert from degree to radians

class Scene;
class Object;
class CSphere;
class CPlane;
class CTriangle;
//#ifndef GZRENDER
//#define GZRENDER
typedef struct {			/* define a renderer */
  GzRenderClass	renderClass;
  GzDisplay		*display;
  short		    open;
  GzCamera		camera;
  short		    matlevel;	        /* top of stack - current xform */
  short			normmatlevel;		/* top of stack of normals */	
  short			modworldtop;		//top of stack of model-world matrices
  short			worldmodtop;		//top of inverse of model-world stack
  short			dirinversetop;		//top of inverse Xform for ray direction
  GzMatrix		Ximage[MATLEVELS];	/* stack of xforms (Xsm) */
  GzMatrix		Xnorm[MATLEVELS];	/* xforms for norms (Xim) */
  GzMatrix		Xsp;		        /* NDC to screen (pers-to-screen) */
  GzMatrix		XModelWorld[MATLEVELS];		// stores the model-world transformation matrices - stack is helpful in calculation of inverse matrix Xform
  GzMatrix		XinverseWorldModel[MATLEVELS];   ///to do inverse transformation from World to Object Space
  GzMatrix		XinverseDirXformMat[MATLEVELS];  //for now, since ray direction do not undergo translation
  GzColor		flatcolor;          /* color state for flat shaded triangles */
  int			interp_mode;
  int			numlights;			//dont include ambient light in the count
  GzLight		lights[MAX_LIGHTS];
  GzLight		ambientlight;
  GzColor		Ka, Kd, Ks;
  float			shadowTerm[MAX_LIGHTS];		/// shadow term to determine if the light sources are blocked
  float		    spec;		/* specular power */
  GzTexture		tex_fun;    /* tex_fun(float u, float v, GzColor color) */
}  GzRender;
//#endif

// Function declaration
// HW2
int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display);
int GzFreeRender(GzRender *render);
int GzBeginRender(GzRender	*render);
int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, GzPointer *valueList);
int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList, GzPointer *valueList);
void  GetDirectionNormals(GzCoord vertexA,GzCoord vertexB,GzCoord vertexC,GzCoord* normal);
int InterpolateZ(GzCoord vertexA, GzCoord* normal,int x,int y);

int ScanLine(GzRender *render, GzCoord* vertexList, GzCoord* normalList,GzTextureIndex* uvList);	//feeds the color or normals to the rasterizer and also does the sorting of vertices
int ScanLineRaster(GzRender* render,GzCoord* pVertex, float xmin,float xmax,float zmin,float zmax,GzCoord* normalList,GzTextureIndex* uvList,GzColor* specList,GzColor* diffList);
void LoadIdentity(GzMatrix* pMatrix);  //reset the matrix to Identity matrix

void Normalize(GzCoord* colVector);
void ComputeXiw(GzCamera* camera);  //also calculates its inverse
// HW3
int GzPutCamera(GzRender *render, GzCamera *camera);
int GzPushMatrix(GzRender *render, GzMatrix	matrix);
int GzPopMatrix(GzRender *render);
bool isValidMatrix(GzRender* render, GzMatrix matrix);		//check if the matrix has only rotation or is Xiw - required for norm Xform 
int PushModelWorldMatrix(GzRender *render, GzMatrix matrix, int type);  //inserts Model-World as well as its inverse - type- 0,1,2 = Scale, Rotation, Translate - no combination allowed yet
int PopModelWorldMatrix(GzRender* render);  //pop both matrices

// Object Translation
int GzRotXMat(float degree, GzMatrix mat);
int GzRotYMat(float degree, GzMatrix mat);
int GzRotZMat(float degree, GzMatrix mat);
int GzTrxMat(GzCoord translate, GzMatrix mat);
int GzScaleMat(GzCoord scale, GzMatrix mat);

float getVectorMagnitude(GzCoord vec);
float dotProduct(GzCoord vec1, GzCoord vec2);
void crossProduct(GzCoord vec1, GzCoord vec2, GzCoord* vec3);
void VertexTransformation(GzCoord pVertex, GzMatrix xformMat,GzCoord* resultant); 
void NormalTransformation(GzCoord pNormal, GzMatrix xformMat,GzCoord* resultant);
void ImageWorldXform(GzRender* render,GzCoord *pLightDir);
vector<SceneMesh> XformPrimitives(GzRender* render, vector<SceneMesh*>pScene);
//vector<SceneMesh> XformPrimitives(GzRender* render, vector<CTriangle>pScene);

//Barycentric Interpolation
void Interpolation(float x, float y, GzCoord* pVertex, GzCoord* pNormal, GzCoord* zVal); //on XY Plane since we consider z = 0
float GetTriangleArea(GzCoord a, GzCoord b, GzCoord c);
bool BarycentricInterpolationTest(GzCoord point, GzCoord* pVertexList, GzCoord normal); //returns true if the point lies inside the triangle
void NormalInterpolation(GzCoord point, GzCoord* pVertex, GzCoord* pNormal, GzCoord* resultant);
//shaders
void FlatShader(GzRender *render, GzCoord* normalList);  
void VertexShader(GzRender *render, GzCoord normal, GzColor* specular, GzColor* diffuse);
void MultiplyColor(GzRender *render, GzColor specular, GzColor diffuse, GzColor* resultant);
void PhongIllumination(GzRender *render, GzRay ray, GzCoord normal, GzColor* intensity, GzCoord hitPoint, Scene *pScene);

//Raytracing
void RayTracing(GzRender * render, vector<SceneMesh*> pScene);
void RayTracing(GzRender *render, Scene* pScene); //Scene version
//void RayTracing(GzRender *render, vector<CTriangle> teapot);
void ConstructRayThroughPixel(GzRender* render, int i, int j, GzRay* ray);
void FindIntersection(GzRay ray, vector<SceneMesh> pScene, GzCoord* intersectionPoint,int& primitiveIndex);
void FindIntersection(GzRay ray, Scene* pScene, GzCoord* intersectionPoint, int& primitiveIndex);
bool Intersect(GzRay ray, GzCoord* pVertexList,float &tValue);  //returns the value of t (which is the distance) and if it intersects or not
void GetColor(GzRender* render,GzRay ray, GzCoord hitPoint,GzCoord* vertexList,GzCoord* pNormalList,GzColor* intensity); //calculates the light intensity at the hit point	

void GetColor(GzRender* render,GzRay ray,Scene* pScene,int depth,GzColor* intensity); //calculates the light intensity at the hit point	
void ComputeReflectionRay(GzRay* ray,GzCoord normal,GzCoord hitPoint,GzRay* reflectionRay);
void ComputeRefractionRay(GzRay* ray,GzCoord normal,GzCoord hitPoint,GzRay* refractionRay);
bool ShadowRay(GzRay ray, Scene* pScene);

#endif

