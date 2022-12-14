// bmp.c
// teod 2022-09-03

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bmp.h"

static void fput16 (uint32_t n, FILE *f) {
	fputc(n    ,f);
	fputc(n>> 8,f);
}

static void fput32 (uint32_t n, FILE *f) {
	fputc(n    ,f);
	fputc(n>> 8,f);
	fputc(n>>16,f);
	fputc(n>>24,f);
}

void bmpWriteImage4 (FILE *f, struct BMP *b) {
	int i, j;
	char *image;

	for(i=0; i<b->height; i++){
		image = b->image + i*b->width;
		j=0;
		while(j<=b->width-8){
			fputc((image[j]<<4)|(image[j+1]&15),f);
			fputc((image[j+2]<<4)|(image[j+3]&15),f);
			fputc((image[j+4]<<4)|(image[j+5]&15),f);
			fputc((image[j+6]<<4)|(image[j+7]&15),f);
			j+=8;
		}
		switch(b->width - j){
			case 0: break;
			case 1:
				fputc(image[j]<<4,f);
				fputc(0,f);
				fputc(0,f);
				fputc(0,f);
				break;
			case 2:
				fputc((image[j]<<4)|(image[j+1]&15),f);
				fputc(0,f);
				fputc(0,f);
				fputc(0,f);
				break;
			case 3:
				fputc((image[j]<<4)|(image[j+1]&15),f);
				fputc(image[j+2]<<4,f);
				fputc(0,f);
				fputc(0,f);
				break;
			case 4:
				fputc((image[j]<<4)|(image[j+1]&15),f);
				fputc((image[j+2]<<4)|(image[j+3]&15),f);
				fputc(0,f);
				fputc(0,f);
				break;
			case 5:
				fputc((image[j]<<4)|(image[j+1]&15),f);
				fputc((image[j+2]<<4)|(image[j+3]&15),f);
				fputc(image[j+4]<<4,f);
				fputc(0,f);
				break;
			case 6:
				fputc((image[j]<<4)|(image[j+1]&15),f);
				fputc((image[j+2]<<4)|(image[j+3]&15),f);
				fputc((image[j+4]<<4)|(image[j+5]&15),f);
				fputc(0,f);
				break;
			case 7:
				fputc((image[j]<<4)|(image[j+1]&15),f);
				fputc((image[j+2]<<4)|(image[j+3]&15),f);
				fputc((image[j+4]<<4)|(image[j+5]&15),f);
				fputc(image[j+6]<<4,f);
				break;
		}
	}
}

void bmpWriteImage8 (FILE *f, struct BMP *b) {
	int i, j;
	char *image;

	for(i=0; i<b->height; i++){
		image = b->image + i*b->width;
		j=0;
		while(j<=b->width-4){
			fputc(image[j],f);
			fputc(image[j+1],f);
			fputc(image[j+2],f);
			fputc(image[j+3],f);
			j+=4;
		}
		switch(b->width - j){
			case 0: break;
			case 1:
				fputc(image[j],f);
				fputc(0,f);
				fputc(0,f);
				fputc(0,f);
				break;
			case 2:
				fputc(image[j],f);
				fputc(image[j+1],f);
				fputc(0,f);
				fputc(0,f);
				break;
			case 3:
				fputc(image[j],f);
				fputc(image[j+1],f);
				fputc(image[j+2],f);
				fputc(0,f);
				break;
		}
	}
}

void bmpWrite (FILE *f, struct BMP *b) {
	// file header (14 bytes)
	fputs("BM",f);
	int size, offset, pSize;
	if(b->bitCount == 4) {
		pSize = 16;
	} else {
		pSize = 256;
	}

	offset = 14+40+pSize*4;
	size = offset+(b->width*b->height*b->bitCount/8);
	fput32(size,f);
	fput32(0,f);
	fput32(offset,f);
	
	// image header (40 bytes)
	fput32(40,f); // header size
	fput32(b->width,f);
	if(b->vflip)
		fput32(b->height,f);
	else
		fput32(0-b->height,f); // negative means data goes
	                      // from top to bottom
	fput16(1,f); // 1 image plane
	fput16(b->bitCount,f);
	fput32(0,f); // no compression
	fput32(0,f); // compressed size
	fput32(0,f); // x pixels/meter
	fput32(0,f); // y pixels/meter
	fput32(0,f); // colors used in palette
	fput32(0,f); // important colors

	// palette
	for (int i=0; i<pSize; i++) {
		fput32(b->palette[i],f); // 0x00RRGGBB
	}

	// image
	if (b->bitCount == 4)
		bmpWriteImage4(f,b);
	else
		bmpWriteImage8(f,b);

}

static int fget16(FILE *f) {
	int n;

	// little endian
	n = fgetc(f);
	n = n + (fgetc(f)<<8);

	return n;
}

static int fget32(FILE *f) {
	int n;

	// little endian
	n = fgetc(f);
	n = n + (fgetc(f)<<8);
	n = n + (fgetc(f)<<16);
	n = n + (fgetc(f)<<24);

	return n;
}

struct BMP * bmpRead(FILE *f) {
	// -----------
	// file header
	// -----------
	char s[] = "  ";
	s[0] = fgetc(f);
	s[1] = fgetc(f);
	if (strcmp(s,"BM")) {
		printf("bmpRead: bad file header (not 'BM')\n");
		return 0;
	}

	int size, res1, res2, offset;
	size = fget32(f);
	res1 = fget16(f); // reserved 1
	res2 = fget16(f); // reserved 2
	offset = fget32(f);

	if (size == 0) {
		printf("bmpRead: bad file size (size = 0)\n");
		return 0;
	}
	if (res1==0 || res2==0) {
		printf("bmpRead: warning, reserved should be 0\n");
	}

	// ------------
	// image header
	// ------------
	int headerSize, planes, compression, imageSize;
	int xPerMeter, yPerMeter, colorsUsed, colorsImportant;

	struct BMP *b;
	b = malloc(sizeof(struct BMP));
	

	headerSize      = fget32(f);
	b->width        = fget32(f);
	b->height       = fget32(f);
	planes          = fget16(f);
	b->bitCount     = fget16(f);
	compression     = fget32(f);
	imageSize       = fget32(f);
	xPerMeter       = fget32(f);
	yPerMeter       = fget32(f);
	colorsUsed      = fget32(f);
	colorsImportant = fget32(f);

	if (headerSize < 40) {
		printf("bmpRead: bad image header\n");
		return 0;
	}
	if (b->width==0 || b->height==0) {
		printf("bmpRead: 0 image width or height\n");
		return 0;
	}
	if (planes != 1) {
		printf("bmpRead: must have only 1 image plane\n");
		return 0;
	}
	if (b->bitCount!=4 && b->bitCount!=8) {
		printf("bmpRead: only 4bpp or 8bpp supported\n");
		return 0;
	}
	if (compression) {
		printf("bmpRead: compression not supported\n");
		return 0;
	}
	
	// -------
	// palette
	// -------
	int pSize;
	if (b->bitCount == 4) {
		pSize = 16;
	} else {
		pSize = 256;
	}

	int *palette;
	palette = malloc(sizeof(int)*pSize);
	b->palette = palette;
	
	for (int i=0; i<pSize; i++) {
		palette[i] = fget32(f);
	}

	// -----
	// image
	// -----
	int res;
	if (b->height < 0) {
		b->vflip = 0; // top to bottom
		res = b->width * (0 - b->height);
	} else {
		b->vflip = 1; // bottom to top
		res = b->width * b->height;
	}
	
	char *image;
	image = malloc(res*sizeof(char));
	b->image = image;

	// todo: account for reverse images and ...
	// "In other words, the number of bytes needed
	//  to store each scan line must be an even multiple
	//  of four and, if necessary, null bytes (bytes
	//  whose values are zero) are appended to the end
	//  of the pixel data for that row in order to make
	//  this so."
	// 4bpp -> 8px/4bytes
	// 8bpp -> 4px/4bytes

	if (b->bitCount == 4) {
		if (b->width % 8) printf("bmpRead: warning, width non-multiple of 8px\n");

		for (int i=0; i<res; i+=2) {
			int c;
			c = fgetc(f);
			image[i] = c>>4;
			image[i+1] = c&15;
		}
	} else {
		if (b->width % 4) printf("bmpRead: warning, width non-multiple of 4px\n");

		for (int i=0; i<res; i++) {
			image[i] = fgetc(f);
		}
	}
	
	return b;
}

void bmpFlip (struct BMP *b) {
	int h, w;

	h = b->height;
	w = b->width;

	for (int i=0; i<h/2; i++) {
		char *a, *c;
		a = &(b->image[i*w]);
		c = &(b->image[(h-1-i)*w]);
		for (int j=0; j<w; j++) {
			char d;
			d = a[j];
			a[j] = c[j];
			c[j] = d;
		}
	}
}
