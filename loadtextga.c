#include "CSCIx229.h"

unsigned int LoadTexTGA(const char* file, int mipmap)
{
	Texture tex;

	LoadTGA(&tex, file); // load TGA


	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &tex.texID);				/* Create The Texture */
	glBindTexture(GL_TEXTURE_2D, tex.texID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	if(mipmap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,tex.width,tex.height,GL_BGRA,GL_UNSIGNED_BYTE,tex.imageData);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, tex.width, tex.height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.imageData);
	}

	free(tex.imageData); // free image

	return tex.texID;
}