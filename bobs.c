#include "bobs.h"

#include "podfather.h"
#include "fb.h"
#include "sprite.h"
#include "strings.h"
#include "text_printer.h"
#include <math.h>
#include <stdio.h>

static int sine[256];

#ifdef __IPOD__
#define SCREEN_COUNT 12
#else
#define SCREEN_COUNT 48
#endif

static unsigned char* greets[] = {"greetings", "to", "", "leachbj", "courtc", "aegray", "all other", "ipodlinux", "devs", "", "sundown", "orgs", "ate bit", "icabod", "laesq", "equinox", "fairlight", "dvs", "noogz", "neseven", "meteorik", "tyranid", "rawhed", "zeroteam", "threesc", "mbmaniax", "jeffie", "megus", "kthreel", "poi", "hardwave", "", "and all at", "sundown!", NULL};
bitmap *greets_bmp;

static unsigned char screens[SCREEN_COUNT][LCD_HEIGHT * LCD_WIDTH];
static unsigned char pixels[LCD_HEIGHT * LCD_WIDTH];

static unsigned char ball_pixels[] = {
	0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,
	0,0,0,3,3,2,2,2,2,2,3,3,0,0,0,0,
	0,0,3,2,2,1,1,2,1,2,2,3,3,0,0,0,
	0,3,2,1,1,1,2,1,2,2,2,2,3,3,0,0,
	0,3,2,1,1,1,1,1,1,2,1,2,2,3,0,0,
	3,2,1,1,1,1,2,1,2,2,2,2,3,3,3,0,
	3,2,1,1,1,1,1,1,1,2,1,2,2,3,3,0,
	3,2,2,1,2,1,2,1,2,2,2,2,3,3,3,0,
	3,2,1,2,1,1,1,2,1,2,2,2,2,3,3,0,
	3,2,2,2,2,2,2,2,2,2,2,3,3,3,3,0,
	0,3,2,2,1,2,1,2,2,2,2,3,2,3,0,0,
	0,3,3,2,2,2,2,2,2,3,3,3,3,3,0,0,
	0,0,3,3,2,3,2,3,2,3,2,3,3,0,0,0,
	0,0,0,3,3,3,3,3,3,3,3,3,0,0,0,0,
	0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static unsigned char ball_mask[] = {
	0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
	0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
	0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
	0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

static bitmap ball = {
	ball_pixels,
	ball_mask,
	16,
	16
};

void bobs_init() {
	int i;

	for (i=0; i<256; i++) {
		sine[i] = (int)(0xffff * sin(i * M_PI / 128));
	}
	
	for (i=0; i<SCREEN_COUNT; i++) {
		/* clear all screens */
		memset(screens[i], 0, LCD_WIDTH * LCD_HEIGHT);
	}
	
	greets_bmp = render_text(greets);
}

void bobs_frame(long time) {
	int x = (sine[((time * 30 / 1200) + 0x40) & 0xff] * (LCD_WIDTH * 2 / 3)) >> 17;
	int y = (sine[((time * 37 / 1200) + 0x40) & 0xff] * (LCD_HEIGHT * 2 / 3)) >> 17;
	int greet_y;
	static int screen_num = 0;
	
	screen_num = (screen_num + 1) % SCREEN_COUNT;
	centre_sprite(screens[screen_num], &ball, x + LCD_WIDTH / 2, y + LCD_HEIGHT / 2);
	
	/* copy shadow screen to main screen */
	memcpy(pixels, screens[screen_num], LCD_WIDTH * LCD_HEIGHT);
	
	/* overlay greetings */
	if (time > PATTERN * 2) {
		greet_y = LCD_HEIGHT - ((time - PATTERN * 2) >> 5);
		put_sprite(pixels, greets_bmp, (LCD_WIDTH - greets_bmp->width) / 2, greet_y);
	}
	
	fb_write(pixels);
}
