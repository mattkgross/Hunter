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
int diffuse = 50; // Diffuse intensity (%)
int local = 0; // Local Viewer Model
int mode = 1; // Light Enabled
int num = 1; // Number of quads
int shininess = 3; // Shininess Varible
float shinyvec[] = {8}; // Shininess (value)
int smooth = 1; // Smooth/Flat shading
int specular = 50; // Specular intensity (%)

// Textures
unsigned int deer_skin;

// View
double asp = 1.0; // Aspect ratio
double dim = 8.0; // World size
int fov = 53; // Field of view
int mouse_x = 0; // X-coordinate of the mouse
int mouse_y = 0; // Y-coordinate of the mouse
int window_height = 600; // Window height
int window_width = 600; // Window width


/* Basic Methods */
void cube(double x, double y, double z, double dx,
		  double dy, double dz, float color[3], double th)
{
	// Save transformation
	glPushMatrix();
	// Offset
	glTranslated(x, y, z);
	// Rotation
	glRotated(th, 1, 0, 0); // rotate about x-axis
	// Scaling
	glScaled(dx, dy, dz);


	// Draw sides
	glBegin(GL_QUADS);
	glColor3f(color[0], color[1], color[2]);

	// Bottom of the cube
	glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

	// Top of the cube
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

	// Left side
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

	// Back side
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    
    // Right side
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    
    // Front side
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

	glEnd();

	// Undo transformations
    glPopMatrix();
}

/* Game Methods */

// Draws a deer
void deer()
{

}

// The HUD - used to display info such as ammo count, time, etc.
void hud()
{
	glWindowPos2i(5, 5);
	Print("Bullets: %i", bullets);
}

void test()
{
	float color[3] = {0.5, 0.5, 0.5};
	cube(0, 0, 0, 1, 1, 1, color, 0);
}


/* GLUT Routines */

// OpenGL (GLUT) calls this routine to display the scene
void display()
{
	// Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    // Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    // Undo previous transformations
    glLoadIdentity();

    //  Display HUD
	hud();
	test();

	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	glutSwapBuffers();
}

// GLUT calls this toutine when there is nothing else to do
void idle()
{
	//  Elapsed time in seconds
	time = glutGet(GLUT_ELAPSED_TIME);

	//  Tell GLUT it is necessary to redisplay the scene
	glutPostRedisplay();
}

// GLUT calls this routine when a key is pressed
void key(unsigned char ch, int x, int y)
{
	//  Exit on ESC
	if (ch == 27)
	  exit(0);
	  
	//  Reproject
	Project(mode?fov:0, asp, dim);
	//  Tell GLUT it is necessary to redisplay the scene
	glutPostRedisplay();
}

// GLUT calls this routine when the mouse moves
void mouse(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}

// GLUT calls this routine when the mouse is clicked
void mouseClick(int button, int state, int x, int y)
{

}

// GLUT calls this routine when the window is resized
void reshape(int width, int height)
{
	window_width = width;
	window_height = height;
	mouse_x = window_width/2;
	mouse_y = window_height/2;

	//  Ratio of the width to the height of the window
	asp = (height>0) ? (double)width/height : 1;
	//  Set the viewport to the entire window
	glViewport(0, 0, width, height);
	//  Set projection
	Project(mode?fov:0, asp, dim);
}

// GLUT calls this routine when an arrow key is pressed
void special(int key, int x, int y)
{ 
	//  Update projection
	Project(mode?fov:0, asp, dim);
	//  Tell GLUT it is necessary to redisplay the scene
	glutPostRedisplay();
}

// Start up GLUT and tell it what to do
int main(int argc, char* argv[])
{
   	//  Initialize GLUT
   	glutInit(&argc,argv);
   	//  Request double buffered, true color window with Z buffering at 600x600
   	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   	glutInitWindowSize(600,600);
   	glutCreateWindow("Hunting Frenzy");
   
   	//  Set Callbacks
   	glutDisplayFunc(display);
   	glutReshapeFunc(reshape);
   	glutSpecialFunc(special);
   	glutKeyboardFunc(key);
   	glutIdleFunc(idle);
   	glutPassiveMotionFunc(mouse);
   	glutMouseFunc(mouseClick);
   
	//glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	// Load Textures
	deer_skin = LoadTexBMP("deer_fur.bmp");

	ErrCheck("init");
	glutMainLoop();
	return 0;
}
