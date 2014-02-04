#ifndef CSCIx229
#define CSCIx229

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef USEGLEW
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))

void Print(const char* format , ...);
void Fatal(const char* format , ...);
unsigned int LoadTexBMP(const char* file);
void Project(double fov,double asp,double dim);
void ErrCheck(const char* where);
int  LoadOBJ(const char* file);


/*
 * Nehe Lesson 33 Code (ported to Linux//GLX by Patrick Schubert 2003
 * with help from the lesson 1 basecode for Linux/GLX by Mihael Vrbanec)
 */

typedef struct
{
	unsigned char	* imageData;									/* Image Data (Up To 32 Bits) */
	unsigned int	bpp;											/* Image Color Depth In Bits Per Pixel */
	unsigned int	width;											/* Image Width */
	unsigned int	height;											/* Image Height */
	unsigned int	texID;											/* Texture ID Used To Select A Texture */
	unsigned int	type;											/* Image Type (GL_RGB, GL_RGBA) */
} Texture;

typedef struct
{
	unsigned char Header[12];									/* TGA File Header */
} TGAHeader;

typedef struct
{
	unsigned char		header[6];								/* First 6 Useful Bytes From The Header */
	unsigned int		bytesPerPixel;							/* Holds Number Of Bytes Per Pixel Used In The TGA File */
	unsigned int		imageSize;								/* Used To Store The Image Size When Setting Aside Ram */
	unsigned int		temp;									/* Temporary Variable */
	unsigned int		type;
	unsigned int		Height;									/* Height of Image */
	unsigned int		Width;									/* Width ofImage */
	unsigned int		Bpp;									/* Bits Per Pixel */
} TGA;

int LoadTGA(Texture * texture,const char * filename);
int LoadUncompressedTGA(Texture *,const char *,FILE *);	/* Load an Uncompressed file */
int LoadCompressedTGA(Texture *,const char *,FILE *);		/* Load a Compressed file */

/* End 3rd Party Code */


unsigned int LoadTexTGA(const char* file, int mipmap);


#endif