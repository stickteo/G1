// bmp.h
// teod 2022-09-03

#ifndef BMP_H
#define BMP_H

struct BMP {
	int width;
	int height;
	int bitCount;
	char *image;
	int *palette;
	int vflip;
};

void bmpWrite (FILE *, struct BMP *);

struct BMP * bmpRead(FILE *f);

void bmpFlip (struct BMP *b);

#endif
