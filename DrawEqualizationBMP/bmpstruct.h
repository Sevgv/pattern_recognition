#pragma once
#ifndef BMPSTRUCT_H_INCLUDED
#define BMPSTRUCT_H_INCLUDED
#include <cstdio>


typedef struct
{
	unsigned int    bfType;
	unsigned long   bfSize;
	unsigned int    bfReserved1;
	unsigned int    bfReserved2;
	unsigned long   bfOffBits;
} BMPFILEHEADER;

typedef struct
{
	unsigned int    biSize;
	int             biWidth;
	int             biHeight;
	unsigned short  biPlanes;
	unsigned short  biBitCount;
	unsigned int    biCompression;
	unsigned int    biSizeImage;
	int             biXPelsPerMeter;
	int             biYPelsPerMeter;
	unsigned int    biClrUsed;
	unsigned int    biClrImportant;
} BMPINFOHEADER;

typedef struct
{
	int   rgbBlue;
	int   rgbGreen;
	int   rgbRed;
	int   rgbReserved;
} RGBQ;


static unsigned short read_u16(FILE *fp);
static unsigned int   read_u32(FILE *fp);
static int            read_s32(FILE *fp);

#endif // BMPSTRUCT_H_INCLUDEDs
