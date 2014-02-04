/*
hunting.c
By: Matthew Gross
Modified: 12/10/2013
Description: A hunting game with a twist of lunar fun.
*/

// OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <GLUT/freeglut.h>
#else
#include <GL/glut.h>
#include <GL/freeglut.h>
#endif

/* Preprocessor Definitions */
#define BG_MUSIC_LENGTH 180000      // Length of background sound in seconds
#define DEER_HEAD 8                 // Speed of deer head
#define DEER_SPEED 12               // Speed of deer
#define EASTER_MUSIC_LENGTH 185000  // Length of easter background sound in seconds
#define PH_K 6                      // Phi tolerance constant
#define TH_K 6.7                    // Theta tolerance constant
#define X_MAX 4                     // Max x-coord in bounds
#define X_MIN -4                    // Min x-coord in bounds
#define X_SHIFT 0.015               // Shift factor for tolerance in x_plane
#define Y_COORD -0.251              // Base ground y-coordinate
#define Y_SHIFT 0.215               // Shift factor for tolerance in y-plane
#define Z_MAX 5                     // Max z-coord in bounds
#define Z_MIN -3                    // Min z-coord in bounds

/* Includes */
#include "CSCIx229.h"
#include <math.h>
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Structs & Enums */
typedef enum {RESTING, GRAZING, RUNNING, DEAD} State; // Deer animation state

typedef struct Deer  // Deer Definition
{
   int front_leg_th; // Front legs angle
   int back_leg_th;  // Back legs angle
   int head_th;      // Head angle

   double x;   // x-coord
   double y;   // y-coord
   double z;   // z-coord

   double scale;  // scale factor
   double dir;    // direction facing
   /* Direction Info */
   /*
      0 = +z
      90 = +x
      180 = -z
      270 = -x
   */
   double x_move; // x movement (sin(dir))
   double z_move; // z movement (cos(dir))

   State state;   // state of the deer

   int neg;             // Is the leg rotation positive or negative
   int neg2;            // Is the head movement positive or negative
   int graze_count;     // How many times has head dipped
   double rest_time;    // How long has been resting
   double graze_time;   // How long it's been grazing

} Deer;


/* Global Variables */

// Game
int bullets = 0; // Number of bullets remaining
int started = 0; // Playing or no

double hunter_x = 0;    // Viewer's x-position
double hunter_y = 0;    // Viewer's y-position
double hunter_z = -1.5; // Viewer's z-position

double deer_dist;                               // Distance between the deer and the hunter
double deer_ph;                                 // Phi of deer
double deer_th;                                 // Theta of deer
const double delta_y = Y_SHIFT + Y_COORD;   	// Y-distance between hunter and center of deer
double ph_tol;                                  // Phi tolerance
double th_tol;                                  // Theta tolerance

// Light Variables
int ambient = 30;    // Ambient intensity (%)
int diffuse = 50;    // Diffuse intensity (%)
int distance = -5;   // Light distance
int local = 0;       // Local Viewer Model
int emission = 0;    // Emission intensity (%)
int shininess = 3;   // Shininess (power of two)
float shinyvec[2];   // Shininess (value)
int specular = 0;    // Specular intensity (%)
int zh = 90;         // Light azimuth

// Objects
int bamboo_obj;               // Bamboo object
unsigned int bamboo_texture;  // Bamboo texture
int deer_body_obj;            // Deer body object
int deer_head_obj;            // Deer head object
int deer_front_legs_obj;      // Deer front legs
int deer_back_legs_obj;       // Deer back legs
int rock_obj;                 // Rock object
unsigned int rock_texture;    // Rock texture
int tree_obj;                 // Tree object
unsigned int tree_texture;    // Tree texture

// View
double asp = 1;   // Aspect ratio
double dim = 3.5; // Size of world
int fov = 55;     // Field of view (for perspective)
double ph = 0;    // Elevation of view angle
double th = 0;    // Azimuth of view angle

double direction_x = 0; // Looking x-direction
double direction_y = 0; // Looking y-direction
double direction_z = 0; // Looking z-direction

int mouse_x = 0;           // X-coordinate of the mouse
int mouse_y = 0;           // Y-coordinate of the mouse
int window_height = 600;   // Window height
int window_width = 600;    // Window width

// Textures
unsigned int deer_texture;    // Deer fur
double rep1 = 5.0;            // Repeat texture variable
unsigned int ground_texture;  // Grass
double rep2 = 50.0;           // Repeat texture variable
unsigned int gun_texture;     // Gun metal
unsigned int log_texture;     // Log bark
unsigned int moon_texture;    // Moon craters
unsigned int sky[2];          // Sky textures

// Sounds
Mix_Music* bg_music;    // Background music
Mix_Music* emptymag;    // Empty magazine sound
Mix_Music* gunshot;     // Gunshot sound
int music_start = 0; 	// When music started
double music_pos = 0.0; // Position of bg music

double bg_resume = 0.0; // Time at which to resume bg
int bg_wait = 0;        // If bg is waiting to play

// Deer
unsigned int cur_deer = 0;       // Current deer being displayed
unsigned int num_deer;           // Number of deer in the environment
Deer deers[5];                   // Deer in environment
int deer_speed = DEER_SPEED;     // Determines how fast the deer moves
int deer_head_speed = DEER_HEAD; // Determines how fast the deer's head dips
double step = DEER_SPEED/160.0;  // Percentage of deer_speed moved per cycle

// EASTER EGG
int activated = 0;            // Is the egg activated
Mix_Music* easter_music;      // Background music (it's classic)
int easter_obj;               // Spaceship Object
double easter_x = -3.9;       // Spaceman x-coord
unsigned int easter_sky[2];   // Moon skybox
unsigned int easter_texture;  // Moon ground
int rep_easter = 20;          // Ground repeat
int spaceman_obj;             // Astronaut object


/* Prototypes */
void Vertex(double th,double ph);

void animate_deer(Deer* deer, unsigned int size);
void bamboo(double x, double y, double z, double dx, double dy, double dz);
void ground();
void gun();
void hud();
void logs(double x, double y, double z, double radius, double height, double th);
void moon(double x,double y,double z,double r, float color[3]);
void rock(double x, double y, double z, double dx, double dy, double dz);
void Sky(double D);
void spaceman(double x, double y, double z, double scale, double deg);
void spaceship(double x, double y, double z, double scale, double td, double pd);
void tree(double x, double y, double z, double dx, double dy, double dz);


/* Basic Methods */

// Convert radians to degrees
inline double Degrees(double rad)
{
   return (rad/M_PI)*180;
}

// Convert degrees to radians
inline double Radians(double deg)
{
   return (deg/180)*M_PI;
}

// Draw vertex in polar coordinates with normal
void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z = Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glTexCoord2d(th/360.0,ph/180.0+0.5);
   glVertex3d(x,y,z);
}

/* Game Methods */

// Handles all deer animation - whew!
void animate_deer(Deer* deer, unsigned int size)
{  
   // Check if deer is off screen (escaped)
   if(deers[cur_deer].x > X_MAX || deers[cur_deer].x < X_MIN ||
      deers[cur_deer].z > Z_MAX || deers[cur_deer].z < Z_MIN)
   {
      // Reset deer speed
      if(deer_speed < 0)
         deer_speed *= -1;

      // Next deer
      cur_deer++;

      // If all deer are gone, end game
      if(cur_deer >= size)
         started++;
   }


   // Do action calculations
   if(deers[cur_deer].state == RESTING)
   {
      if(deers[cur_deer].rest_time == 0)
         deers[cur_deer].rest_time = glutGet(GLUT_ELAPSED_TIME);

      // If have been resting for 10 secs, start grazing
      if(glutGet(GLUT_ELAPSED_TIME) > deers[cur_deer].rest_time + 10000)
      {
         deers[cur_deer].rest_time = 0;
         deers[cur_deer].state = GRAZING;
      }
   }
   else if(deers[cur_deer].state == GRAZING)
   {
      // Find next angle
      int diff = (deers[cur_deer].head_th+(deers[cur_deer].neg2?-deer_head_speed:deer_head_speed))%45;

      // Prompt Head Lowering
      if(!deers[cur_deer].neg2 && diff < deers[cur_deer].head_th)
      {
         deers[cur_deer].graze_count++;
         deers[cur_deer].neg2 = 1;
      }

      // Start head movement down
      if(deers[cur_deer].graze_count%3 == 0)
      {
         deers[cur_deer].head_th = diff;
      }
      // Pause for eating
      else if (deers[cur_deer].graze_count%3 == 1)
      {
         // Keep head at 45 degree angle
         deers[cur_deer].head_th = 45;

         if(deers[cur_deer].graze_time == 0)
            deers[cur_deer].graze_time = glutGet(GLUT_ELAPSED_TIME);

         // Graze for 5 seconds
         if(glutGet(GLUT_ELAPSED_TIME) > deers[cur_deer].graze_time + 5000)
         {
            deers[cur_deer].graze_time = 0;
            deers[cur_deer].graze_count++;
         }
      }
      // Move head back up
      else
      {
         // Rest once head is returned to 0 degrees
         if(deers[cur_deer].head_th < 0)
         {
            deers[cur_deer].head_th = 0;
            deers[cur_deer].graze_count++;
            deers[cur_deer].neg2 = 0;
            deers[cur_deer].state = RESTING;
         }
         // Otherwise, keep moving head up
         else
            deers[cur_deer].head_th = diff;
      }
   }
   else if(deers[cur_deer].state == RUNNING)
   {
      // Find next leg angle
      int diff = (deers[cur_deer].front_leg_th+deer_speed)%45;

      // Dictate forward or backward leg movement
      if(!deers[cur_deer].neg && diff < deers[cur_deer].front_leg_th)
      {
         deer_speed *= -1;
         deers[cur_deer].neg = 1;
      }

      if(deers[cur_deer].neg && diff > deers[cur_deer].front_leg_th)
      {
         deer_speed *= -1;
         deers[cur_deer].neg = 0;
      }

      // Set leg angle
      deers[cur_deer].back_leg_th = -1*(deers[cur_deer].front_leg_th = diff);
      // Set back legs back one cycle
      deers[cur_deer].back_leg_th += deers[cur_deer].neg?(-deer_speed):(deer_speed);


      // Move deer
      deers[cur_deer].x += deers[cur_deer].x_move;
      deers[cur_deer].z += deers[cur_deer].z_move;
   }
   else  //deers[cur_deer].state == DEAD
   {
      // Animate the death

      // If deer is underground, then we are done
      if(deers[cur_deer].y < -0.6)
      {
         // Reset deer speed
         if(deer_speed < 0)
            deer_speed *= -1;

         // Move on to next deer
         cur_deer++;

         // If all deer are gone, end game
         if(cur_deer >= size)
            started++;
      }
      // If haven't yet bowed head, start that
      else if(deers[cur_deer].head_th < 45)
      {
         deers[cur_deer].head_th += deer_head_speed;
      }
      // Otherwise, start lowering into the ground
      else
      {
         deers[cur_deer].y -= 0.01;
      }
   }

   // Draw updated deer
   glPushMatrix();
   glTranslated(deers[cur_deer].x,deers[cur_deer].y,deers[cur_deer].z);
   glScaled(deers[cur_deer].scale, deers[cur_deer].scale, deers[cur_deer].scale);
   glRotated(deers[cur_deer].dir,0,1,0); // rotate about y axis
   activated?glColor3f(0.196078, 0.8, 0.196078):glColor3f(0.647059, 0.164706, 0.164706);
   glCallList(deer_body_obj);

   // Former Calls for Reference when Editing Blender Files
   //deer_head(0, 2.2, 1.3, deers[cur_deer].head_th, deer_head_obj);
   //deer_front_leg_right(-0.27, 1.8, 0.8, deers[cur_deer].front_leg_th, deer_front_right_leg_obj);
   //deer_front_leg_left(0.27, 1.8, 0.8, deers[cur_deer].front_leg_th, deer_front_left_leg_obj);
   //deer_back_leg_right(-0.27, 2, -0.75, deers[cur_deer].back_leg_th, deer_back_right_leg_obj);
   //deer_back_leg_left(0.27, 2, -0.75, deers[cur_deer].back_leg_th, deer_back_left_leg_obj);

   // Head
   glTranslated(0, 2.2+Sin(deers[cur_deer].head_th), 0);
   glRotated(deers[cur_deer].head_th, 1, 0, 0);
   glCallList(deer_head_obj);
   // Front Legs
   glTranslated(0, -0.4-Sin(deers[cur_deer].head_th), Sin(deers[cur_deer].head_th));
   glRotated(deers[cur_deer].front_leg_th - deers[cur_deer].head_th, 1, 0, 0);
   glCallList(deer_front_legs_obj);
   // Back Legs
   glRotated(deers[cur_deer].back_leg_th - deers[cur_deer].front_leg_th, 1, 0, 0);
   glCallList(deer_back_legs_obj);

   glPopMatrix();
}

// Draws a tree scaled dx,dy,dz
void bamboo(double x, double y, double z, double dx, double dy, double dz)
{
   //  Draw the model
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(dx,dy,dz);
   glColor3f(0.419608, 0.556863, 0.137255);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, bamboo_texture);
   glCallList(bamboo_obj);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

// Draws the ground
void ground()
{
   glPushMatrix();

   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, activated?easter_texture:ground_texture);

   // Draw grass
   glBegin(GL_QUADS);
   glColor3f(1.0, 1.0, 1.0);
   glTexCoord2f(0.0,0.0); glVertex3f(4, Y_COORD, 5);
   glTexCoord2f(activated?rep_easter:rep2,0.0); glVertex3f(4, Y_COORD, -3);
   glTexCoord2f(activated?rep_easter:rep2,activated?rep_easter:rep2); glVertex3f(-4, Y_COORD, -3);
   glTexCoord2f(0.0,activated?rep_easter:rep2); glVertex3f(-4, Y_COORD, 5);
   glEnd();

   glDisable(GL_TEXTURE_2D);
   glPopMatrix();

   // Draw plant life & rocks
   rock(0, Y_COORD, 1, 0.001, 0.001, 0.001);
   rock(2, Y_COORD, 0.5, 0.01, 0.01, 0.01);
   rock(1.4, Y_COORD, 0.4, 0.001, 0.001, 0.001);
   rock(0.5, Y_COORD, 1.1, 0.001, 0.001, 0.001);
   rock(-0.3, Y_COORD, 0.8, 0.0008, 0.0008, 0.0008);
   rock(-0.4, Y_COORD, 1.2, 0.0012, 0.001, 0.0014);
   rock(-1.5, Y_COORD, 0.95, 0.0015, 0.0015, 0.0015);
   rock(-1.7, Y_COORD, 0.6, 0.0006, 0.002, 0.001);
   rock(-2.7, Y_COORD, 0.6, 0.001, 0.0008, 0.002);
   rock(-2.75, Y_COORD, 0.69, 0.001, 0.0017, 0.003);
   rock(-2.87, Y_COORD, 0.65, 0.002, 0.0024, 0.002);
   rock(-0.5, Y_COORD, 0.3, 0.003, 0.004, 0.001);
   rock(0, Y_COORD, 1.5, 0.001, 0.001, 0.001);
   rock(-0.3, Y_COORD, 1.8, 0.0017, 0.001, 0.0012);
   rock(0.05, Y_COORD, 1.85, 0.0014, 0.002, 0.0016);
   rock(0.25, Y_COORD, 1.85, 0.0013, 0.0019, 0.002);
   rock(0.3, Y_COORD, 1.95, 0.003, 0.0025, 0.0028);
   rock(0.45, Y_COORD, 1.87, 0.0017, 0.0021, 0.0018); 
   rock(0.5, Y_COORD, 1.97, 0.0032, 0.0029, 0.003);
   rock(0.65, Y_COORD, 1.89, 0.002, 0.0023, 0.0021); 
   rock(0.7, Y_COORD, 1.99, 0.0035, 0.0032, 0.0035);
   rock(2.4, Y_COORD, 2.2, 0.008, 0.007, 0.009);
   rock(2.9, Y_COORD, 2.25, 0.007, 0.01, 0.006);
   rock(0.5, Y_COORD, -0.75, 0.0035, 0.002, 0.004);
   if(!activated)
   {
      bamboo(-2.8, Y_COORD, 0.69, 0.001, 0.001, 0.001);
      bamboo(-2.65, Y_COORD, 0.65, 0.002, 0.002, 0.002);
      bamboo(-2.75, Y_COORD, 0.63, 0.002, 0.002, 0.002);
      bamboo(2.1, Y_COORD, 0.5, 0.0035, 0.0035, 0.0035);
      bamboo(1.9, Y_COORD, 0.6, 0.002, 0.002, 0.002);
      bamboo(1.8, Y_COORD, 0.5, 0.002, 0.002, 0.002);
      bamboo(1.9, Y_COORD, 0.4, 0.0018, 0.0018, 0.0018);
      bamboo(2, Y_COORD, 0.4, 0.0018, 0.0018, 0.0018);
      bamboo(0.65, Y_COORD, 2.7, 0.004, 0.004, 0.004);
      bamboo(-2, Y_COORD, 2.2, 0.002, 0.002, 0.002);
      bamboo(-2.2, Y_COORD, 2.2, 0.002, 0.002, 0.002);
      bamboo(-2.6, Y_COORD, 2.1, 0.0025, 0.0025, 0.0025);
      bamboo(-2.9, Y_COORD, 1.9, 0.003, 0.003, 0.003);
      bamboo(-3.3, Y_COORD, 2, 0.003, 0.003, 0.003);
      bamboo(-3.7, Y_COORD, 2, 0.0025, 0.0025, 0.0025);
      bamboo(-4.2, Y_COORD, 2.2, 0.002, 0.002, 0.002);
      bamboo(-2.1, Y_COORD, 2.5, 0.002, 0.002, 0.002);
      bamboo(-2.3, Y_COORD, 2.5, 0.0025, 0.0025, 0.0025);
      bamboo(-2.6, Y_COORD, 2.45, 0.0027, 0.0027, 0.0027);
      bamboo(-2.8, Y_COORD, 2.4, 0.003, 0.003, 0.003);
      bamboo(-3.1, Y_COORD, 2.3, 0.0018, 0.0018, 0.0018);
      bamboo(-3.45, Y_COORD, 2.3, 0.003, 0.003, 0.003);
      bamboo(-3.6, Y_COORD, 2.5, 0.0023, 0.0023, 0.0023);
      bamboo(-2.9, Y_COORD, 2.8, 0.0013, 0.0013, 0.0013);
      bamboo(-3.2, Y_COORD, 2.7, 0.0012, 0.0012, 0.0012);
      bamboo(-3.6, Y_COORD, 2.7, 0.0015, 0.0015, 0.0015);
      bamboo(-3.8, Y_COORD, 2.8, 0.0011, 0.0011, 0.0011);
      bamboo(-2.9, Y_COORD, -2, 0.0015, 0.0015, 0.0015);
      bamboo(-2.75, Y_COORD, -2.1, 0.0016, 0.0016, 0.0016);
      bamboo(2.1, Y_COORD, -2, 0.0017, 0.0017, 0.0017);
      bamboo(2, Y_COORD, -1.75, 0.0015, 0.0015, 0.0015);
      bamboo(2.2, Y_COORD, -1.85, 0.0014, 0.0014, 0.0014);
      tree(-2.7, Y_COORD, 1.6, 0.0075, 0.0075, 0.0075);
      tree(2.5, Y_COORD, -1, 0.01, 0.01, 0.01);
      tree(2.75, Y_COORD, -0.35, 0.01, 0.01, 0.01);
      tree(-3, Y_COORD, 0, 0.0075, 0.0075, 0.0075);
      tree(-3.25, Y_COORD, -0.5, 0.0085, 0.0085, 0.0085);
      tree(-0.5, Y_COORD, -1, 0.002, 0.002, 0.002);
      tree(-0.65, Y_COORD, -1.12, 0.002, 0.002, 0.002);
      tree(-3, Y_COORD, -0.85, 0.0078, 0.0078, 0.0078);
      tree(-3.3, Y_COORD, -1.5, 0.0075, 0.0075, 0.0075);
      tree(2.6, Y_COORD, -1.45, 0.0075, 0.0075, 0.0075);  
      logs(-0.65, 2, 0.5, 0.05, 0.5, 80);
   }
}

// Draws the gun
void gun()
{
   int inc = 10;
   int x, y;
   
   // Grey coloring
   glColor3f(0.2,0.2,0.2);
   glPushMatrix();
   
   // Move the gun in the right direction
   glTranslated(hunter_x+direction_x/5,hunter_y+direction_y/5,hunter_z+direction_z/5);
   
   // Aim the gun in the right direction
   glRotated(-ph,1,0,0);
   glRotated(-th,0,Cos(ph),Sin(ph));
   
   // Draw gun barrel
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, gun_texture);
   glBegin(GL_QUAD_STRIP);
   for(y = 0; y <= 360; y += inc)
   {
      glNormal3f(Sin(y), Cos(y), 0);
      glTexCoord2f(y/120.0, 0); glVertex3f(Sin(y)/50, Cos(y)/50-0.04, -0.1);
      glTexCoord2f(y/120.0, 4); glVertex3f(Sin(y)/50, Cos(y)/50-0.04, 0.4);
   }
   glEnd();

   // Draw rounded gun end
   for (x=-1; x <= 90; x += inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (y=0; y <= 360; y += inc)
      {
         glTexCoord2f((y)/120.0, (90-x-inc)/45.0);
         glNormal3d(Sin(y)*Cos(x-inc), Cos(y)*Cos(x-inc), Sin(x-inc));
         glVertex3d(Sin(y)*Cos(x-inc)/50, Cos(y)*Cos(x-inc)/50-0.04, -Sin(x-inc)/50-0.1);
         
         glTexCoord2f((y)/120.0, (90-x)/45.0);
         glNormal3d(Sin(y)*Cos(x), Cos(y)*Cos(x), Sin(x));
         glVertex3d(Sin(y)*Cos(x)/50, Cos(y)*Cos(x)/50-0.04, -Sin(x)/50-0.1);
      }
      glEnd();
   }
   
   glDisable(GL_TEXTURE_2D);
   
   
   // Draw Sights
   glBegin(GL_QUADS);
     
   glVertex3f(-0.005, -0.04, 0.4);
   glVertex3f(-0.005, -0.005, 0.4);
   glVertex3f(0.005, -0.005, 0.4);
   glVertex3f(0.005, -0.04, 0.4);
   
   glColor3f(1,0.2,0.2);
   glVertex3f(-0.005, -0.005, 0.4);
   glVertex3f(-0.005, 0.00, 0.4);
   glVertex3f(0.005, 0.00, 0.4);
   glVertex3f(0.005, -0.005, 0.4);
   glEnd();
   
   glPopMatrix();
}

// The HUD
void hud()
{
   glWindowPos2i(5, window_height - 15);
   Print("Bullets: %i", bullets);
}

// Draws a log
void logs(double x, double y, double z, double radius, double height, double th)
{    
   glPushMatrix();

   glRotated(th,0,0,1); // rotate about z axis
   
   // Draw log body
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, log_texture);
   glBegin(GL_QUAD_STRIP);
   glColor3f(1.0, 1.0, 1.0);
   int i;
   for (i = 0; i <= 35; i++) 
   {     
         double angle = i*((1.0/30)*(2*M_PI));
         glNormal3d(x+cos(angle), y, z+sin(angle));
         glTexCoord2f(i/120.0, 0); glVertex3d(x+radius*cos(angle), y+height, z+radius*sin(angle)); // top
         glTexCoord2f(i/120.0, 4); glVertex3d(x+radius*cos(angle), y, z+radius*sin(angle)); // bottom
   }
        
   glEnd();
   glDisable(GL_TEXTURE_2D);
   
   glPopMatrix();
}

// Draws a moon
void moon(double x,double y,double z,double r, float color[3])
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
   glBindTexture(GL_TEXTURE_2D, moon_texture);
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

   glPopMatrix();
}

// Draws a rock scaled dx,dy,dz
void rock(double x, double y, double z, double dx, double dy, double dz)
{
   //  Draw the model
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(dx,dy,dz);
   glColor3f(1.0, 1.0, 1.0);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, rock_texture);
   glCallList(rock_obj);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/* 
 *  Draw sky box
 */
void Sky(double D)
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   // Sides
   glBindTexture(GL_TEXTURE_2D,activated?easter_sky[0]:sky[0]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.00,1); glVertex3f(-D,+D,-D);

   glTexCoord2f(0.25,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);

   glTexCoord2f(0.50,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);

   glTexCoord2f(0.75,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(1.00,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(1.00,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
   glEnd();

   // Top
   glBindTexture(GL_TEXTURE_2D,activated?easter_sky[1]:sky[1]);
   glBegin(GL_QUADS);
   glTexCoord2f(0.0,0); glVertex3f(+D,+D,-D);
   glTexCoord2f(0.5,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(0.5,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0.0,1); glVertex3f(-D,+D,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}

// Draws an astronaut
void spaceman(double x, double y, double z, double scale, double deg)
{
   //  Draw the model
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(scale, scale, scale);
   glRotated(deg, 0, 1, 0); // y-axis rotation
   glRotated(-25, 0, 0, 1); // z-axis rotation
   glColor3f(1,1,1);
   glCallList(spaceman_obj);
   glPopMatrix();
}

// Draws a spaceship
void spaceship(double x, double y, double z, double scale, double td, double pd)
{
   //  Draw the model
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(scale, scale, scale);
   glRotated(td, 0, 0, 1); // z-axis rotation
   glRotated(pd, 0, 1, 0); // y-axis rotation
   glColor3f(1,1,1);
   glCallList(easter_obj);
   glPopMatrix();
}

// Draws a tree
void tree(double x, double y, double z, double dx, double dy, double dz)
{
   //  Draw the model
   glPushMatrix();
   glTranslated(x,y,z);
   glScaled(dx,dy,dz);
   glColor3f(0.419608, 0.556863, 0.137255);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, tree_texture);
   glCallList(tree_obj);
   glDisable(GL_TEXTURE_2D);
   glPopMatrix();
}

/* GLUT Routines */

// Cleans up memory before exit
void cleanup()
{
   Mix_FreeMusic(bg_music);
   Mix_FreeMusic(gunshot);
   Mix_FreeMusic(emptymag);
   Mix_FreeMusic(easter_music);
   Mix_CloseAudio();
}

// OpenGL (GLUT) calls this routine to display the scene
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();
   
   //  Perspective - set eye position
   th = (mouse_x - (window_width/2.0))/(window_width/2.0)*60.0;
   ph = -(mouse_y - (window_height/2.0))/(window_width/2.0)*60.0;

   // Determine Direction of View
   direction_x = -Sin(th)*Cos(ph);
   direction_y = Sin(ph);
   direction_z = Cos(ph)*Cos(th);

   // Set Looking Position
   gluLookAt(hunter_x, hunter_y, hunter_z, direction_x + hunter_x,
             direction_y + hunter_y, direction_z + hunter_z, 0, 1, 0);

   //  Smooth shading
   glShadeModel(GL_SMOOTH);
   
   // Draw Sky
   Sky(3.5*dim);

   //  Translate intensity to color vectors
   float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
   float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
   float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
   //  Light position
   float Position[]  = {-3.85, 7.25, 9.5,1.0}; // in front of moon

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
   
   // Start Screen
   if(!started && cur_deer == 0)
   {
      // Instructions and tips w/ start screen
      glColor3f(1,1,0);
      glWindowPos2i(window_width/2-70,window_height*0.9);
      Print("Welcome to Hunting Frenzy!");
      glWindowPos2i(20,window_height*0.7);
      Print("Instructions:");
      glWindowPos2i(40,window_height*0.6);
      Print("1) To aim, move your mouse!");
      glWindowPos2i(40,window_height*0.55);
      Print("2) Take time to aim at your first target - the deer rests & grazes until you shoot.");
      glWindowPos2i(40,window_height*0.5);
      Print("3) To shoot, left click - be careful, you only have 5 bullets!");
      glWindowPos2i(40,window_height*0.45);
      Print("4) Turn up the sound! There's some cool sound effects we don't want you to miss!");
      glWindowPos2i(40,window_height*0.4);
      Print("5) Also, look for the hidden easter egg for a special bonus!");
      glWindowPos2i(40,window_height*0.35);
      Print("6) Ready to play? Press 3, 4, or 5 to select how many deer to hunt!");
      glColor3f(1,1,1);
   }
   // Gameplay
   else if(started == 1)
   {
      // Draw gun
      gun();

      // Draw easter object
      if(activated)
      {
         spaceship(-0.75, 4, 4, 0.02, -20, -45);
         easter_x += 0.1;
         if(easter_x < 4)
            spaceman(easter_x, 2, 4, 0.5, 20);
      }

      // Draw the terrain
      ground();

      // Draw the deer
      animate_deer(deers, num_deer);

      // Moon
      if(!activated)
      {
         float colors[3] = {0.9, 0.91, 0.98};
         moon(-4, 7.5, 10, 0.25, colors);
      }
   }
   // Game Over
   else
   {
      // Exit screen
      glColor3f(1,1,0);
      glWindowPos2i(window_width/2-65,window_height*0.6);
      Print("Thanks for Playing!");
      glWindowPos2i(window_width/2-50,window_height*0.5);
      int i, dead = 0;
      for(i = 0; i < num_deer; i++)
         if(deers[i].state == DEAD)
            dead++;
      Print("Deer Shot: %d", dead);
      glWindowPos2i(window_width/2.5,window_height*0.4);
      Print("Credit to: Willem Schreuder, Patrick Schubert (TGA Library),");
      glWindowPos2i(window_width/2.23,window_height*0.37);
      Print("Conrad Hougen (Collision Math)");
      glWindowPos2i(window_width/2.8,window_height*0.30);
      Print("Texture & Object Credit: Plant & Rock Objects - Nobiax, Spaceship & Astronaut Objects - NASA");

   }

   // Draw axes - no lighting from here on
   glDisable(GL_LIGHTING);
   glColor3f(1,1,1);

   // Display parameters
   hud();

   // Render the scene and make it visible
   ErrCheck("display");
   glFlush();
   glutSwapBuffers();
}

/*
 *  GLUT calls this toutine when there is nothing else to do
 */
void idle()
{
   // Play any waiting sounds
   if(bg_wait && bg_resume <= glutGet(GLUT_ELAPSED_TIME))
   {
      // Resume bg music
      Mix_PlayMusic(activated?easter_music:bg_music, -1);
      if(Mix_SetMusicPosition(music_pos/1000)==-1)
         printf("Mix_SetMusicPosition: %s\n", Mix_GetError());
      bg_wait = 0;
   }

   // Tell GLUT it is necessary to redisplay the scene
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

   else if(!started && ch == '3')
   {
      // Set up environment
      num_deer = 3;
      bullets = 5;
      started++;
   }
   else if(!started && ch == '4')
   {
      // Set up environment
      num_deer = 4;
      bullets = 5;
      started++;
   }
   else if(!started && ch == '5')
   {
      // Set up environment
      num_deer = 5;
      bullets = 5;
      started++;
   }

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
 *  GLUT calls this routine when the mouse is moved.
 */
void mouse(int x, int y)
{
   mouse_x = x;
   mouse_y = y;
}

/*
 *  GLUT calls this routine when the mouse is clicked.
 */
void mouseClick(int button, int state, int x, int y)
{
   // left click: shoot
   if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
   {
      // SOUNDS & HUD

      // Need bullets to shoot
      if(bullets > 0)
      {
         // Update the music position
         music_pos = (glutGet(GLUT_ELAPSED_TIME) - music_start)%(activated?EASTER_MUSIC_LENGTH:BG_MUSIC_LENGTH)-1000;

         // Gunshot sound
         Mix_PlayMusic(gunshot, 1);
         bg_resume = glutGet(GLUT_ELAPSED_TIME)+1000;
         bg_wait = 1;

         bullets--;

         // BULLET & SHOT AFTER-EFFECTS

         // Check for a kill
         // Find distance from deer
         deer_dist = sqrt(deers[cur_deer].x*deers[cur_deer].x+delta_y*delta_y+
                     (deers[cur_deer].z-hunter_z)*(deers[cur_deer].z-hunter_z));
         // Find the phi of the deer
         deer_ph = Degrees(asin(delta_y/deer_dist));
         // Find the theta of the deer
         deer_th = Degrees(atan((-1*deers[cur_deer].x+X_SHIFT)/(-1*hunter_z+deers[cur_deer].z)));

         // Calculate tolerances
         th_tol = TH_K/deer_dist;
         ph_tol = PH_K/deer_dist;

         /*// Print info to check against
         printf("deer_dist: %f\n", deer_dist);
         printf("deer_th+th_tol: %f\ndeer_th-th_tol: %f\nth: %f\n", deer_th+th_tol, deer_th-th_tol, th);
         printf("deer_ph+ph_tol: %f\ndeer_ph-ph_tol: %f\nph: %f\n", deer_ph+ph_tol, deer_ph-ph_tol, ph);*/

         // If they shot the moon, activiate the easter egg
         if(th <= 20.41 && th >= 18.21 && ph <= 32.61 && ph >= 30.79 && !activated)
         {
            bullets++;
            activated = 1;

            //  Play Background Sound(looping)
            music_start = glutGet(GLUT_ELAPSED_TIME);
            Mix_PlayMusic(easter_music,-1);
            bg_wait = 0;

            return;
         }

         // Determine whether or not it's a hit
         if((deer_th-th_tol < th && deer_th+th_tol > th) &&
            (deer_ph-ph_tol < ph && deer_ph+ph_tol > ph))
         {
            // Upon kill set leg_th to be 0;
            deers[cur_deer].front_leg_th = 0;
            deers[cur_deer].back_leg_th = 0;
            deers[cur_deer].state = DEAD;
         }


         // Make deer run
         int i;
         for(i = 0; i < num_deer; i++)
         {
            // Apply this to all alive deer
            if(deers[i].state != DEAD)
            {
               deers[i].state = RUNNING;
               deers[i].head_th = 0;
            }
         }
      }
      else
      {
         // Update the music position
         music_pos = (glutGet(GLUT_ELAPSED_TIME) - music_start)%(activated?EASTER_MUSIC_LENGTH:BG_MUSIC_LENGTH)-340;

         // Gunshot sound
         Mix_PlayMusic(emptymag, 1);
         bg_resume = glutGet(GLUT_ELAPSED_TIME)+340;
         bg_wait = 1;
      }
   }
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
   // Set global window values
   window_width = width;
   window_height = height;
   // Reset mouse position to middle of screen
   mouse_x = window_width/2;
   mouse_y = window_height/2;
   //  Set projection
   Project(fov,asp,dim);
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Update projection
   Project(fov,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
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
   glutPassiveMotionFunc(mouse);
   glutMouseFunc(mouseClick);
   glutCloseFunc(cleanup);

   // Disable mouse
   glutSetCursor(GLUT_CURSOR_NONE);

   // Make graphics smoother
   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

   // Load Sounds & Initialize audio
   if (Mix_OpenAudio(44100,AUDIO_S16SYS,2,4096)) Fatal("Cannot initialize audio\n");

   bg_music = Mix_LoadMUS("sounds/naturesounds.ogg");
   if (!bg_music) Fatal("Cannot load naturesounds.ogg\n");
   gunshot = Mix_LoadMUS("sounds/gunshot.ogg");
   if (!gunshot) Fatal("Cannot load gunshot.ogg\n");
   emptymag = Mix_LoadMUS("sounds/emptymag.ogg");
   if (!emptymag) Fatal("Cannot load emptymag.ogg\n");
   easter_music = Mix_LoadMUS("sounds/moon.ogg");
   if (!easter_music) Fatal("Cannot load moon.ogg\n");

   //  Play Background Sound(looping)
   music_start = glutGet(GLUT_ELAPSED_TIME);
   Mix_PlayMusic(bg_music,-1);

   //  Load textures
   deer_texture = LoadTexBMP("textures/deer_fur.bmp");
   ground_texture = LoadTexBMP("textures/grass2.bmp");
   moon_texture = LoadTexBMP("textures/moon.bmp");
   gun_texture = LoadTexBMP("textures/gun.bmp");
   log_texture = LoadTexBMP("textures/log.bmp");
   easter_texture = LoadTexBMP("textures/easter.bmp");

   sky[0] = LoadTexBMP("textures/sky0.bmp");
   sky[1] = LoadTexBMP("textures/sky1.bmp");
   easter_sky[0] = LoadTexBMP("textures/moon0.bmp");
   easter_sky[1] = LoadTexBMP("textures/moon1.bmp");

   // Load Objects & Textures
   rock_obj = LoadOBJ("objects/rocks/rock_02.obj");
   rock_texture = LoadTexBMP("objects/rocks/diffuse.bmp");
   bamboo_obj = LoadOBJ("objects/plants/bamboo/bamboo.obj");
   bamboo_texture = LoadTexTGA("objects/plants/bamboo/diffuse.tga", 1);
   tree_obj = LoadOBJ("objects/plants/hemp/hemp.obj");
   tree_texture = LoadTexBMP("textures/leaf.bmp");
   deer_body_obj = LoadOBJ("deerBody.obj");
   deer_head_obj = LoadOBJ("deerHead.obj");
   deer_front_legs_obj = LoadOBJ("deerFrontLegs.obj");
   deer_back_legs_obj = LoadOBJ("deerBackLegs.obj");
   spaceman_obj = LoadOBJ("EMU.obj");
   easter_obj = LoadOBJ("NASA_Shuttle.obj");

   // Load Deer Info
   deers[0].x = 0; deers[0].y = Y_COORD; deers[0].z = 0; deers[0].scale = 0.1; deers[0].dir = 220;
    deers[0].x_move = step*Sin(deers[0].dir); deers[0].z_move = step*Cos(deers[0].dir);
    deers[0].state = RESTING; deers[0].neg = 0; deers[0].front_leg_th = 0; deers[0].back_leg_th = 0;
    deers[0].head_th = 0; deers[0].rest_time = 0; deers[0].graze_count = 0; deers[0].graze_time = 0;
    deers[0].neg2 = 0;

   deers[1].x = 3.9; deers[1].y = Y_COORD; deers[1].z = 4.9; deers[1].scale = 0.1; deers[1].dir = 230;
    deers[1].x_move = step*Sin(deers[1].dir); deers[1].z_move = step*Cos(deers[1].dir);
    deers[1].state = RESTING; deers[1].neg = 0; deers[1].front_leg_th = 0; deers[1].back_leg_th = 0;
    deers[1].head_th = 0; deers[1].rest_time = 0; deers[1].graze_count = 0; deers[1].graze_time = 0;
    deers[1].neg2 = 0;

   deers[2].x = -3.9; deers[2].y = Y_COORD; deers[2].z = 4.9; deers[2].scale = 0.1; deers[2].dir = 140;
    deers[2].x_move = step*Sin(deers[2].dir); deers[2].z_move = step*Cos(deers[2].dir);
    deers[2].state = RESTING; deers[2].neg = 0; deers[2].front_leg_th = 0; deers[2].back_leg_th = 0;
    deers[2].head_th = 0; deers[2].rest_time = 0; deers[2].graze_count = 0; deers[2].graze_time = 0;
    deers[2].neg2 = 0;

   deers[3].x = 3.9; deers[3].y = Y_COORD; deers[3].z = 2.5; deers[3].scale = 0.1; deers[3].dir = 270;
    deers[3].x_move = step*Sin(deers[3].dir); deers[3].z_move = step*Cos(deers[3].dir);
    deers[3].state = RESTING; deers[3].neg = 0; deers[3].front_leg_th = 0; deers[3].back_leg_th = 0;
    deers[3].head_th = 0; deers[3].rest_time = 0; deers[3].graze_count = 0; deers[3].graze_time = 0;
    deers[3].neg2 = 0;

   deers[4].x = -3; deers[4].y = Y_COORD; deers[4].z = -1; deers[4].scale = 0.1; deers[4].dir = 10;
    deers[4].x_move = step*Sin(deers[4].dir); deers[4].z_move = step*Cos(deers[4].dir);
    deers[4].state = RESTING; deers[4].neg = 0; deers[4].front_leg_th = 0; deers[4].back_leg_th = 0;
    deers[4].head_th = 0; deers[4].rest_time = 0; deers[4].graze_count = 0; deers[4].graze_time = 0;
    deers[4].neg2 = 0;

   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
