
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bmp.h"

struct TILESET {
	int n;
	uint8_t *data;
	uint32_t *hash;
};

struct MAP {
	int w;
	int h;
	uint16_t *data;
};

uint32_t hashTile (uint8_t *a);

void initTileset (struct TILESET *ts) {
	ts->n = 1; // has empty tile
	ts->data = malloc(64*1024); // 1024 tiles * 64 px/tile * 1 byte/px
	ts->hash = malloc(sizeof(uint32_t)*1024);

	memset(ts->data, 0, 64);
	ts->hash[0] = hashTile(ts->data); // should be 0
}

void freeTileset (struct TILESET *ts) {
	free(ts->data);
	free(ts->hash);
	free(ts);
}

/* a
 0  1  2  3  4  5  6  7
 8  9 10 11 12 13 14 15
16 17 18 19 20 21 22 23
24 25 26 27 28 29 30 31
32 33 34 35 36 37 38 39
40 41 42 43 44 45 46 47
48 49 50 51 52 53 54 55
56 57 58 59 60 61 62 63
*/

/* b
 0  1  2  3
 4  5  6  7
 8  9 10 11
12 13 14 15
*/

uint32_t hashTile (uint8_t *a) {
	// a non-matching hash will tell tiles are different (100%)
	// a matching hash indicates tiles are likely the same (<100%)

	uint8_t b[16];

	b[ 0] = a[ 0]^a[ 7]^a[56]^a[63];
	b[ 1] = a[ 1]^a[ 6]^a[57]^a[62];
	b[ 2] = a[ 2]^a[ 5]^a[58]^a[61];
	b[ 3] = a[ 3]^a[ 4]^a[59]^a[60];

	b[ 4] = a[ 8]^a[15]^a[48]^a[55];
	b[ 5] = a[ 9]^a[14]^a[49]^a[54];
	b[ 6] = a[10]^a[13]^a[50]^a[53];
	b[ 7] = a[11]^a[12]^a[51]^a[52];

	b[ 8] = a[16]^a[23]^a[40]^a[47];
	b[ 9] = a[17]^a[22]^a[41]^a[46];
	b[10] = a[18]^a[21]^a[42]^a[45];
	b[11] = a[19]^a[20]^a[43]^a[44];

	b[12] = a[24]^a[31]^a[32]^a[39];
	b[13] = a[25]^a[30]^a[33]^a[38];
	b[14] = a[26]^a[29]^a[34]^a[37];
	b[15] = a[27]^a[28]^a[35]^a[36];

	uint32_t c[4];

	c[0] = b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24);
	c[1] = b[4] | (b[5]<<8) | (b[6]<<16) | (b[7]<<24);
	c[2] = b[8] | (b[9]<<8) | (b[10]<<16) | (b[11]<<24);
	c[3] = b[12] | (b[13]<<8) | (b[14]<<16) | (b[15]<<24);
	
	c[1] = (c[1]<<4) | (c[1]>>28); // rotate left 4
	c[3] = (c[3]<<4) | (c[3]>>28); // rotate left 4
	
	return c[0] ^ c[1] ^ c[2] ^ c[3];
}

int compareTile (uint8_t *a, uint8_t *b) {
	uint8_t c[64];
	uint8_t d;

	// no flip
	d = 0;
	for (int i=0; i<64; i++) { c[i] = a[i] ^ b[i]; }
	for (int i=0; i<64; i++) { d = d | c[i]; }
	if (d == 0) { return 0; }

	// h flip
	d = 0;
	for (int i=0; i<64; i++) {
		int j = (i&~7) + 7 - (i&7);
		c[i] = a[i] ^ b[j];
	}
	for (int i=0; i<64; i++) { d = d | c[i]; }
	if (d == 0) { return 1; }

	// v flip
	d = 0;
	for (int i=0; i<64; i++) {
		int j = ((63-i)&~7) + (i&7);
		c[i] = a[i] ^ b[j];
	}
	for (int i=0; i<64; i++) { d = d | c[i]; }
	if (d == 0) { return 2; }

	// hv flip
	d = 0;
	for (int i=0; i<64; i++) {
		c[i] = a[i] ^ b[63-i];
	}
	for (int i=0; i<64; i++) { d = d | c[i]; }
	if (d == 0) { return 3; }

	return -1;
}

uint32_t addTile (struct TILESET *ts, uint8_t *a) {
	uint32_t hash;
	uint8_t *b;
	uint32_t n;

	int flip;

	hash = hashTile(a);
	n = ts->n;

	// check if hash exists
	for (int i=0; i<n; i++) {
		if (hash == ts->hash[i]) {
			b = ts->data + i*64;
			flip = compareTile(a, b);
			if (flip != -1) {
				// tile already exists
				return (flip<<10) | i;
			}
		}
	}

	// new tile
	b = ts->data + n*64;
	for (int i=0; i<64; i++) {
		b[i] = a[i];
	}
	ts->hash[n] = hash;
	ts->n++;
	
	return n;
}

void initMap (struct MAP *m, struct BMP *b) {
	m->w = b->width/8;
	m->h = b->height/8;
	
	m->data = malloc(sizeof(uint16_t) * m->w * m->h);
}

void freeMap (struct MAP *m) {
	free(m->data);
	free(m);
}

void readTile (struct BMP *b, int y, int x, uint8_t *data) {
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			data[i*8+j] = b->image[(i+y)*b->width + j+x];
		}
	}
}

void clearTilePal (uint8_t *data) {
	for (int i=0; i<64; i++) {
		data[i] = data[i] & 15;
	}
}

void bmpToTileMap (struct BMP *b, struct TILESET *ts, struct MAP *m) {
	uint8_t *a;
	a = malloc(sizeof(uint8_t)*64);

	uint32_t p, n;

	for (int i=0; i<m->h; i++) {
		for (int j=0; j<m->w; j++) {
			readTile(b, i*8, j*8, a);

			// use palette of top left pixel
			p = (a[0]>>4)&15;
			clearTilePal(a);

			n = addTile(ts, a);

			m->data[i*m->w + j] = (p<<12) | n;
		}
	}

	free(a);
}

void printMap (struct MAP *m) {
	
	for (int i=0; i<m->h; i++) {
		for (int j=0; j<m->w; j++) {
			printf("%04x ",m->data[i*m->w + j]);
		}
		printf("\n");
	}
}

void writeMap (struct MAP *m, FILE *f) {
	int n;
	n = m->h * m->w;

	for (int i=0; i<n; i++) {
		uint32_t c;
		c = m->data[i];
		putc(c&255, f);
		putc(c>>8, f);
	}
}

void writeTileset (struct TILESET *ts, FILE *f) {
	int n;
	n = ts->n * 64;
	
	for (int i=0; i<n; i+=2) {
		uint32_t c;
		c = ts->data[i] | (ts->data[i+1]<<4);
		putc(c, f);
	}
}

int main (int argc, char **argv) {

	if (argc < 4) {
		printf(
			"not enough arguments!\n"
			"usage: G1 in.bmp outTile.bin outMap.bin\n"
		);

		return 0;
	}

	FILE *fb, *ft, *fm;
	fb = fopen(argv[1], "rb");
	ft = fopen(argv[2], "wb");
	fm = fopen(argv[3], "wb");

	struct BMP *b;
	b = bmpRead(fb);
	fclose(fb);

	//if (b->vflip) printf("warning: image is flipped!\n");
	if (b->vflip) bmpFlip(b);

	struct MAP *m;
	m = malloc(sizeof(struct MAP));
	initMap(m, b);

	struct TILESET *ts;
	ts = malloc(sizeof(struct TILESET));
	initTileset(ts);

	bmpToTileMap(b,ts,m);
	// printMap(m);

	printf("Tiles used: %d\n", ts->n);

	writeTileset(ts,ft);
	fclose(ft);

	writeMap(m,fm);
	fclose(fm);

	freeMap(m);
	freeTileset(ts);

	return 0;
}
