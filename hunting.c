/*
hunting.c
By: Matthew Gross
Modified: 10/29/2013
Description: 
*/

// OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

/* Preprocessor Definitions */


/* Includes */
#include "CSCIx229.h"
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Global Variables */

// Game
int bullets = 5; // Number of bullets remaining
double time = 0.0; // Elapsed time

// Light Variables
int ambient = 30; // Ambient intensity (%)
int diffuse = 100; // Diffuse intensity (%)
int distance = -5; // Light distance
int local = 0; // Local Viewer Model
int emission = 0; // Emission intensity (%)
int shininess = 3; // Shininess (power of two)
float shinyvec[1]; // Shininess (value)
int specular = 0; // Specular intensity (%)
float ylight = 3; // Elevation of light
int zh = 90; // Light azimuth

// View
double asp = 1; //  Aspect ratio
double dim = 3.0; //  Size of world
int fov = 55; //  Field of view (for perspective)
int ph = 0; //  Elevation of view angle
int th = 0; //  Azimuth of view angle

int mouse_x = 0; // X-coordinate of the mouse
int mouse_y = 0; // Y-coordinate of the mouse
int window_height = 600; // Window height
int window_width = 600; // Window width

// Textures
unsigned int deer_texture;
double rep = 5.0; // repeat texture variable


// Determine the cross product (for obtaining normal)
float* crossProduct(float x[3], float y[3], float z[3])
{
   float* ans = malloc(sizeof(float)*3);
   float* vec1 = malloc(sizeof(float)*3);
   float* vec2 = malloc(sizeof(float)*3);

   vec1[0] = x[1] - x[0];
   vec1[1] = y[1] - y[0];
   vec1[2] = z[1] - z[0];

   vec2[0] = x[2] - x[1];
   vec2[1] = y[2] - y[1];
   vec2[2] = z[2] - z[1];

   // Find the normal
   ans[0] = (vec1[1] * vec2[2]) - (vec2[1] * vec1[2]);
   ans[1] = (vec1[0] * vec2[2]) - (vec2[0] * vec1[2]);
   ans[2] = (vec1[0] * vec2[1]) - (vec2[0] * vec1[1]);

   free(vec1);
   free(vec2);

   return ans;
}

/*
 *  Draw vertex in polar coordinates with normal
 */
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glTexCoord2d(th/360.0,ph/180.0+0.5);
   glVertex3d(x,y,z);
}

// Golden pyramid
void pyramid(double x,double y,double z,
             double dx,double dy,double dz,
             double th)
{
   float* normal = malloc(sizeof(float)*3);
   float* xc = malloc(sizeof(float)*3);
   float* yc = malloc(sizeof(float)*3);
   float* zc = malloc(sizeof(float)*3);

   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   //  Rotation
   glRotated(th,1,0,0); // rotate about x-axis
   //  Scaling
   glScaled(dx,dy,dz);


   // Texture
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, deer_texture);

   // Draw Triangular Sides
   glBegin(GL_TRIANGLES);
   glColor3f(0.8, 0.6, 0.3);

   // front triangle
   xc[1] = 0.0f; xc[0] = 1.0f; xc[2] = -1.0f;
   yc[1] = 1.0f; yc[0] = -1.0f; yc[2] = -1.0f;
   zc[1] = 0.0f; zc[0] = 1.0f; zc[2] = 1.0f;
   normal = crossProduct(xc, yc, zc);
   glNormal3f(normal[0], normal[1], normal[2]);
         
   glTexCoord2f(rep/2,rep); glVertex3f(0.0f, 1.0f, 0.0f); // top
   glTexCoord2f(0.0,0.0); glVertex3f(1.0f, -1.0f, 1.0f); // right
   glTexCoord2f(rep,0.0); glVertex3f(-1.0f, -1.0f, 1.0f); // left

   // left triangle
   xc[1] = 0.0f; xc[0] = -1.0f; xc[2] = -1.0f;
   yc[1] = 1.0f; yc[0] = -1.0f; yc[2] = -1.0f;
   zc[1] = 0.0f; zc[0] = 1.0f; zc[2] = -1.0f;
   normal = crossProduct(xc, yc, zc);
   glNormal3f(normal[0], normal[1], normal[2]);
   glTexCoord2f(rep/2,rep); glVertex3f(0.0f, 1.0f, 0.0f); // top
   glTexCoord2f(0.0,0.0); glVertex3f(-1.0f, -1.0f, 1.0f); // right
   glTexCoord2f(rep,0.0); glVertex3f(-1.0f, -1.0f, -1.0f); // left

   // back traingle
   xc[1] = 0.0f; xc[0] = -1.0f; xc[2] = 1.0f;
   yc[1] = 1.0f; yc[0] = -1.0f; yc[2] = -1.0f;
   zc[1] = 0.0f; zc[0] = -1.0f; zc[2] = -1.0f;
   normal = crossProduct(xc, yc, zc);
   glNormal3f(normal[0], -normal[1], normal[2]);
   glTexCoord2f(rep/2,rep); glVertex3f(0.0f, 1.0f, 0.0f); // top
   glTexCoord2f(0.0,0.0); glVertex3f(-1.0f, -1.0f, -1.0f); // right 
   glTexCoord2f(rep,0.0); glVertex3f(1.0f, -1.0f, -1.0f); // left

   // right triangle
   xc[1] = 0.0f; xc[0] = 1.0f; xc[2] = 1.0f;
   yc[1] = 1.0f; yc[0] = -1.0f; yc[2] = -1.0f;
   zc[1] = 0.0f; zc[0] = -1.0f; zc[2] = 1.0f;
   normal = crossProduct(xc, yc, zc);
   glNormal3f(normal[0], normal[1], normal[2]);
   glTexCoord2f(rep/2,rep); glVertex3f(0.0f, 1.0f, 0.0f); // top
   glTexCoord2f(0.0,0.0); glVertex3f(1.0f, -1.0f, -1.0f); // right
   glTexCoord2f(rep,0.0); glVertex3f(1.0f, -1.0f, 1.0f); // left
   glEnd();

   // Draw Base
   glBegin(GL_QUADS);
   glColor3f(0.8, 0.6, 0.3);

   glNormal3f(0.0f, -1.0f, 0.0f);
   glTexCoord2f(0.0,0.0); glVertex3f(-1.0f, -1.0f, 1.0f);
   glTexCoord2f(rep,0.0); glVertex3f(-1.0f, -1.0f, -1.0f);
   glTexCoord2f(rep,rep); glVertex3f(1.0f, -1.0f, 1.0f);
   glTexCoord2f(0.0,rep); glVertex3f(1.0f, -1.0f, -1.0f);
   
   glNormal3f(0.0f, -1.0f, 0.0f);
   glTexCoord2f(0.0,0.0); glVertex3f(-1.0f, -1.0f, -1.0f);
   glTexCoord2f(rep,0.0); glVertex3f(1.0f, -1.0f, 1.0f);
   glTexCoord2f(rep,rep); glVertex3f(1.0f, -1.0f, -1.0f);
   glTexCoord2f(0.0,rep); glVertex3f(-1.0f, -1.0f, 1.0f);
   glEnd();

   glDisable(GL_TEXTURE_2D);

   //  Undo transformations
   glPopMatrix();

   free(zc);
   free(yc);
   free(xc);
   free(normal);
}

/*
 *  Draw a ball
 *     at (x,y,z)
 *     radius (r)
 */
static void ball(double x,double y,double z,double r, float color[3])
{
   int th,ph;
   float yellow[4] = {1.0,1.0,0.0,1.0};
   float Emission[4]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(color[0],color[1],color[2]);
   glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=10)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*10)
      {
         Vertex(th,ph);
         Vertex(th,ph+10);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}

static void moon(double x,double y,double z,double r, float color[3])
{
   int th,ph;
   float yellow[4] = {1.0,1.0,0.0,1.0};
   float Emission[4]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, deer_texture);
   //  White ball
   glColor3f(color[0],color[1],color[2]);
   glMaterialfv(GL_FRONT,GL_SHININESS,shinyvec);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=10)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*10)
      {
         Vertex(th,ph);
         Vertex(th,ph+10);
      }
      glEnd();
   }

   glDisable(GL_TEXTURE_2D);
   //  Undo transofrmations
   glPopMatrix();
}

void ground()
{
   glPushMatrix();

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, deer_texture);

   glBegin(GL_QUADS);

   //glColor3f(0.137255, 0.556863, 0.137255);
   glColor3f(1.0, 1.0, 1.0);
   glTexCoord2f(0.0,0.0); glVertex3f(2, -0.251, 2);
   glTexCoord2f(rep,0.0); glVertex3f(2, -0.251, -2);
   glTexCoord2f(rep,rep); glVertex3f(-2, -0.251, -2);
   glTexCoord2f(0.0,rep); glVertex3f(-2, -0.251, 2);
   glEnd();

   glDisable(GL_TEXTURE_2D);

   glPopMatrix();
}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   
   //  Perspective - set eye position
   double Ex = -2*dim*Sin(th)*Cos(ph);
   double Ey = +2*dim        *Sin(ph);
   double Ez = +2*dim*Cos(th)*Cos(ph);
   gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);

   //  Smooth shading
   glShadeModel(GL_SMOOTH);

   //  Translate intensity to color vectors
   float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
   float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
   float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
   //  Light position
   float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
   //  Draw light position as ball (still no lighting here)
   glColor3f(1,1,1);
   float coloring[3] = {1,1,1};
   ball(Position[0],Position[1],Position[2] , 0.1, coloring);
   //  OpenGL should normalize normal vectors
   glEnable(GL_NORMALIZE);
   //  Enable lighting
   glEnable(GL_LIGHTING);
   //  Location of viewer for specular calculations
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
   //  glColor sets ambient and diffuse color materials
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   //  Enable light 0
   glEnable(GL_LIGHT0);
   //  Set ambient, diffuse, specular components and position of light 0
   glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
   glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
   glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
   glLightfv(GL_LIGHT0,GL_POSITION,Position);
   
   // Pyramids
   pyramid(0, 0, 0, 0.25, 0.25, 0.25, 0);
   pyramid(-1, 0, 1, 0.25, 0.25, 0.25, 0);
   pyramid(1, 0, 1, 0.25, 0.25, 0.25, 0);

   ground();

   // Moon
   float colors[3] = {0, 0, 0.61};
   moon(-1, 2, 0, 0.25, colors);

   //  Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);

   //  Display parameters
   glWindowPos2i(5,5);
   Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s Light=%s",
     th,ph,dim,fov,"Perpective","On");

   glWindowPos2i(5,45);
   Print("Model=%s LocalViewer=%s Distance=%d Elevation=%.1f","Smooth",local?"On":"Off",distance,ylight);
   glWindowPos2i(5,25);
   Print("Ambient=%d  Diffuse=%d Specular=%d Emission=%d Shininess=%.0f",ambient,diffuse,specular,emission,shinyvec[0]);

   //  Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this toutine when there is nothing else to do
 */
void idle()
{
   //  Elapsed time in seconds
   time = glutGet(GLUT_ELAPSED_TIME)/1000.0;

   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(fov,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);

   //  Translate shininess power to value (-1 => 0)
   shinyvec[0] = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(fov,asp,dim);
   //  Animate if requested
   glutIdleFunc(idle);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(fov,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);

   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Hunting Frenzy - Matthew Gross");

   //  Set callbacks
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);

   //  Load textures
   deer_texture = LoadTexBMP("deer_fur.bmp");

   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
