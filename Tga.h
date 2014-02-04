/*
 * Nehe Lesson 33 Code (ported to Linux//GLX by Patrick Schubert 2003
 * with help from the lesson 1 basecode for Linux/GLX by Mihael Vrbanec)
 */

#ifndef __TGA_H__
#define __TGA_H__

#include "Texture.h"
#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/keysym.h>
#include <stdio.h>

typedef struct
{
	unsigned byte Header[12];									/* TGA File Header */
} TGAHeader;

typedef struct
{
	unsigned byte		header[6];								/* First 6 Useful Bytes From The Header */
	unsigned int		bytesPerPixel;							/* Holds Number Of Bytes Per Pixel Used In The TGA File */
	unsigned int		imageSize;								/* Used To Store The Image Size When Setting Aside Ram */
	unsigned int		temp;									/* Temporary Variable */
	unsigned int		type;
	unsigned int		Height;									/* Height of Image */
	unsigned int		Width;									/* Width ofImage */
	unsigned int		Bpp;									/* Bits Per Pixel */
} TGA;


TGAHeader tgaheader;									/* TGA header */
TGA tga;												/* TGA image data */


unsigned byte uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	/* Uncompressed TGA Header */
unsigned byte cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};	/* Compressed TGA Header */
int LoadUncompressedTGA(Texture *,const char *,FILE *);	/* Load an Uncompressed file */
int LoadCompressedTGA(Texture *,const char *,FILE *);		/* Load a Compressed file */

#endif






