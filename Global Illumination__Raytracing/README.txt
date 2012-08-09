Our topic  is Global Illumination using raytracing.

We modified the application and the rend.cpp to make it able to do the raytracing work.
We also add primitive classes which are sphere, triangle, plane
and load these primitive into our scene. 
Then our render can construct rays and detect the intersection if it hits these primitives.
Also after detect hitting we will shoot reflection ray and refraction ray to do the recusive call
Which make the basic ray tracing works.

We also implement the shadow which can show the shadow on a teapot