#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "commonvars.h"
#include "sound.h"
#include "savestate.h"
#include "gamefuncs.h"
#include "cviewport.h"
#include "cworldpart.h"
#include "cworldparts.h"
//only the skins FORCESKIN leaves in are part of the build (a 1 bpp buffer forces the black & white
//Kenney skin). The Flat skin (2) uses Default images as well
#if SKINBUILT(0) || SKINBUILT(2)
#include SKIN_IMAGE(Default/background_RLE565.h)
#include SKIN_IMAGE(Default/box_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/door_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/earthgrassleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/earthgrassright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/earthleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/earthmiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/earthright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floatingfloorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floatingfloormiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floatingfloorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floatingfloor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/floor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/intro1_RLE565.h)
#include SKIN_IMAGE(Default/intro2_RLE565.h)
#include SKIN_IMAGE(Default/intro3_RLE565.h)
#include SKIN_IMAGE(Default/player_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roof1_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roof2_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roofcornerboth_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roofcornerleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roofcornerright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roofdownleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/roofdownright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/starttower_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/titlescreen_RLE565.h)
#include SKIN_IMAGE(Default/towershaft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Default/tower_table_16_16_RGB565_LE.h)
#endif
#if SKINBUILT(2)
//the Flat skin shares its background, intro screens, player, box, door and floating floor
//images with the Default skin, those are taken from Default and not included twice
#include SKIN_IMAGE(Flat/earthgrassleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/earthgrassright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/earthleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/earthmiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/earthright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/floorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/floorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/floor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roof1_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roof2_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roofcornerboth_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roofcornerleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roofcornerright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roofdownleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/roofdownright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/starttower_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/titlescreen_RLE565.h)
#include SKIN_IMAGE(Flat/towershaft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Flat/tower_table_16_16_RGB565_LE.h)
#endif
#if SKINBUILT(4)
#include SKIN_IMAGE(Kenney/background_RLE565.h)
#include SKIN_IMAGE(Kenney/box_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/door_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/earthgrassleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/earthgrassright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/earthleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/earthmiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/earthright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floatingfloorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floatingfloormiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floatingfloorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floatingfloor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/floor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/intro1_RLE565.h)
#include SKIN_IMAGE(Kenney/intro2_RLE565.h)
#include SKIN_IMAGE(Kenney/intro3_RLE565.h)
#include SKIN_IMAGE(Kenney/player_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roof1_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roof2_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roofcornerboth_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roofcornerleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roofcornerright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roofdownleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/roofdownright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/starttower_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/titlescreen_RLE565.h)
#include SKIN_IMAGE(Kenney/towershaft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Kenney/tower_table_16_16_RGB565_LE.h)
#endif
#if SKINBUILT(1)
#include SKIN_IMAGE(Tech/background_RLE565.h)
#include SKIN_IMAGE(Tech/box_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/door_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/earthgrassleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/earthgrassright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/earthleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/earthmiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/earthright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floatingfloorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floatingfloormiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floatingfloorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floatingfloor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/floor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/intro1_RLE565.h)
#include SKIN_IMAGE(Tech/intro2_RLE565.h)
#include SKIN_IMAGE(Tech/intro3_RLE565.h)
#include SKIN_IMAGE(Tech/player_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roof1_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roof2_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roofcornerboth_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roofcornerleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roofcornerright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roofdownleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/roofdownright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/starttower_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/titlescreen_RLE565.h)
#include SKIN_IMAGE(Tech/towershaft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Tech/tower_table_16_16_RGB565_LE.h)
#endif
#if SKINBUILT(3)
#include SKIN_IMAGE(Ti-83/background_RLE565.h)
#include SKIN_IMAGE(Ti-83/box_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/door_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/earthgrassleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/earthgrassright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/earthleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/earthmiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/earthright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floatingfloorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floatingfloormiddle_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floatingfloorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floatingfloor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floorleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floorright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/floor_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/intro1_RLE565.h)
#include SKIN_IMAGE(Ti-83/intro2_RLE565.h)
#include SKIN_IMAGE(Ti-83/intro3_RLE565.h)
#include SKIN_IMAGE(Ti-83/player_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roof1_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roof2_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roofcornerboth_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roofcornerleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roofcornerright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roofdownleft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/roofdownright_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/starttower_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/titlescreen_RLE565.h)
#include SKIN_IMAGE(Ti-83/towershaft_table_16_16_RGB565_LE.h)
#include SKIN_IMAGE(Ti-83/tower_table_16_16_RGB565_LE.h)
#endif

//multi line text straight to the screen. tft.drawChar already matches what the
//framebuffer version did per character, a 6x8 cell with the background only
//painted when it differs from the text colour, so only the line breaks are
//handled here. Advances match the old code, 6 pixels per char and 9 per line
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size)
{
	int16_t cursorX = x;
	int16_t cursorY = y;
	if (!str)
		return;
#if LOVYANGFX
	//LovyanGFX's drawChar that takes the colours hands them to the font the other way
	//round, set them as the text colour instead. Its default font is the same 6x8 GLCD
	//font and a background equal to the text colour is left out here as well
	GFX.setTextColor(color, bg);
	GFX.setTextSize(size);
#endif
#if SCREENBUFFER == 0
	//Straight to the display every character would be a write transaction of its own, and
	//the chip select sits on the I/O expander: that is I2C traffic per character. One
	//transaction for the whole text instead. Into a buffer nothing is sent, so nothing to do
	SCREEN.startWrite();
#endif
	while (*str)
	{
		if (*str == '\n')
		{
			cursorY += 9 * size;
			cursorX = x;
			str++;
			continue;
		}
#if LOVYANGFX
		GFX.drawChar((uint8_t)*str, cursorX, cursorY);
#else
		GFX.drawChar(cursorX, cursorY, *str, color, bg, size);
#endif
		cursorX += 6 * size;
		str++;
	}
#if SCREENBUFFER == 0
	SCREEN.endWrite();
#endif
}

#if SCREENBUFFER
//The board is drawn again every frame with a buffer, every tile of it, so this is kept
//lean: the image is clipped once, every visible row comes out of flash in one copy and
//is written through a row pointer, nothing is worked out per pixel
void DrawImageToBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, bool transparent)
{
	if (!image)
		return;
	int16_t c0 = (x < 0) ? -x : 0;
	int16_t c1 = (x + w > WINDOW_WIDTH) ? WINDOW_WIDTH - x : w;
	int16_t r0 = (y < 0) ? -y : 0;
	int16_t r1 = (y + h > WINDOW_HEIGHT) ? WINDOW_HEIGHT - y : h;
	if ((c0 >= c1) || (r0 >= r1))
		return;
	void* dst = SCREENBUFFER_PIXELS();
	if (!dst)
		return;
	const int16_t cols = c1 - c0;
	const int16_t sx = x + c0;
	//the images are uint8_t arrays of little endian RGB565, like both devices, so the
	//bytes can be copied straight into it
	uint16_t row[WINDOW_WIDTH];
	for (int16_t r = r0; r < r1; r++)
	{
		const int16_t sy = y + r;
		PLATFORM_READ_BYTES((uint8_t*)row, image + (r * w + c0) * sizeof(uint16_t), cols * sizeof(uint16_t));
  #if SCREENBUFFER == 16
		uint16_t* d = &((uint16_t*)dst)[sy * WINDOW_WIDTH + sx];
		for (int16_t c = 0; c < cols; c++)
		{
			uint16_t color = row[c];
			//magenta is the transparent key
			if (!transparent || (color != 0xF81F))
				d[c] = (uint16_t)((color >> 8) | (color << 8));
		}
  #elif SCREENBUFFER == 8
		uint8_t* d = &((uint8_t*)dst)[sy * WINDOW_WIDTH + sx];
		for (int16_t c = 0; c < cols; c++)
		{
			uint16_t color = row[c];
			if (!transparent || (color != 0xF81F))
				d[c] = (uint8_t)(((color & 0xE000) >> 8) | ((color & 0x0700) >> 6) | ((color & 0x0018) >> 3));
		}
  #else
		for (int16_t c = 0; c < cols; c++)
		{
			uint16_t color = row[c];
			if (!transparent || (color != 0xF81F))
				SetBufferBit((uint8_t*)dst, sx + c, sy, color);
		}
  #endif
	}
}
#endif

//Draws one of the magenta keyed sprites. The display can do the keying itself, but
//TFT_eSprite has no pushImage that skips a transparent colour (and its 1 bpp pushImage
//expects 1 bpp image data), so with a buffer the pixels are written into it directly,
//in whatever form that buffer keeps them. LovyanGFX reads image data through plain
//pointers, but PROGMEM on the ESP8266 is flash that only takes 32 bit reads, so with
//that library even the display gets its pixels from here.
void DrawImageTransparent(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image)
{
	if (!image)
		return;
#if (SCREENBUFFER == 0) && !LOVYANGFX
	GFX.pushImage(x, y, w, h, (const uint16_t*)image, 0xF81F);
#elif SCREENBUFFER == 0
	//straight to the display, every run of opaque pixels on a row goes out as one
	uint16_t line[WINDOW_WIDTH];
	SCREEN.startWrite();
	for (int16_t r = 0; r < h; r++)
	{
		int16_t sy = y + r;
		if ((sy < 0) || (sy >= WINDOW_HEIGHT))
			continue;
		//the images are uint8_t arrays, so the pixels are read a byte at a time
		const uint8_t* src = image + r * w * sizeof(uint16_t);
#if PLATFORM_DIRECT_FLASH
		//flash is plain memory here: an evenly placed row is read 16 bits at a time, a read
		//of an odd address faults on a core like the Cortex-M0+
		const uint16_t* srow = (((uintptr_t)src & 1) == 0) ? (const uint16_t*)src : NULL;
#endif
		int16_t runX = 0, runLen = 0;
		for (int16_t c = 0; c <= w; c++)
		{
			int16_t sx = x + c;
			uint16_t color = 0xF81F;
			if ((c < w) && (sx >= 0) && (sx < WINDOW_WIDTH))
#if PLATFORM_DIRECT_FLASH
				color = srow ? srow[c] : (uint16_t)(PLATFORM_READ_BYTE(src + c * 2) | (PLATFORM_READ_BYTE(src + c * 2 + 1) << 8));
#else
				color = PLATFORM_READ_BYTE(src + c * 2) | (PLATFORM_READ_BYTE(src + c * 2 + 1) << 8);
#endif
			//magenta is the transparent key, it (and the end of the row) closes a run
			if (color != 0xF81F)
			{
				if (runLen == 0)
					runX = sx;
				line[runLen++] = color;
			}
			else if (runLen > 0)
			{
				SCREEN.setAddrWindow(runX, sy, runLen, 1);
				//true: the values are plain RGB565, the library puts them in display order
				SCREEN.writePixels(line, runLen, true);
				runLen = 0;
			}
		}
	}
	SCREEN.endWrite();
#else
	DrawImageToBuffer(x, y, w, h, image, true);
#endif
}

//draws a run length encoded RGB565 image made by tools/png2rle565.py straight from
//flash, the raw full screen images of all skins do not fit in the flash that can be
//mapped for code. A control byte with the top bit set is a run of (c & 0x7F) + 1 times
//the pixel after it, otherwise c + 1 literal pixels follow. No clipping is done.
//The data is read here with PLATFORM_READ_BYTE and PLATFORM_READ_BYTES: LovyanGFX reads
//image data through plain pointers, but PROGMEM on the ESP8266 is flash that only takes
//32 bit reads, so none of its image functions may be handed the data
void pushImageRLE(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data)
{
#if SCREENBUFFER
	//decoded straight into the buffer instead of streamed to the display. The pixels of a
	//control go in a row at a time: the part of the row that is on screen is worked out once,
	//literal pixels come out of flash in one copy and the colour of a run is converted once
	if ((w <= 0) || (h <= 0))
		return;
	void* dst = SCREENBUFFER_PIXELS();
	if (!dst)
		return;
	//a control covers at most 128 pixels
	uint16_t pixels[128];
	uint32_t left = (uint32_t)w * h;
	int16_t cx = 0, cy = 0;
	while (left > 0)
	{
		uint8_t control = PLATFORM_READ_BYTE(data++);
		uint16_t count = (control & 0x7F) + 1;
		if (count > left)
			count = (uint16_t)left;
		bool run = (control & 0x80) != 0;
		uint16_t color = 0;
		if (run)
		{
			color = PLATFORM_READ_BYTE(data) | (PLATFORM_READ_BYTE(data + 1) << 8);
			data += 2;
		}
		else
		{
			//little endian RGB565 like both devices, so the bytes can be copied straight in
			PLATFORM_READ_BYTES((uint8_t*)pixels, data, count * sizeof(uint16_t));
			data += count * sizeof(uint16_t);
		}
		left -= count;
  #if SCREENBUFFER == 16
		//a 16 bpp sprite keeps its pixels byte swapped
		const uint16_t value = (uint16_t)((color >> 8) | (color << 8));
  #elif SCREENBUFFER == 8
		//RGB332, the same conversion both libraries apply to everything else
		const uint8_t value = (uint8_t)(((color & 0xE000) >> 8) | ((color & 0x0700) >> 6) | ((color & 0x0018) >> 3));
  #endif
		for (uint16_t done = 0; done < count; )
		{
			//the pixels of the control that are left on this row
			uint16_t n = w - cx;
			if (n > count - done)
				n = count - done;
			const int16_t sx = x + cx, sy = y + cy;
			//the columns of those that are on screen
			const int16_t c0 = (sx < 0) ? -sx : 0;
			const int16_t c1 = (sx + n > WINDOW_WIDTH) ? WINDOW_WIDTH - sx : n;
			if ((sy >= 0) && (sy < WINDOW_HEIGHT) && (c0 < c1))
			{
				const uint16_t* src = &pixels[done];
  #if SCREENBUFFER == 16
				uint16_t* d = &((uint16_t*)dst)[sy * WINDOW_WIDTH];
				if (run)
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = value;
				else
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = (uint16_t)((src[c] >> 8) | (src[c] << 8));
  #elif SCREENBUFFER == 8
				uint8_t* d = &((uint8_t*)dst)[sy * WINDOW_WIDTH];
				if (run)
					memset(&d[sx + c0], value, c1 - c0);
				else
					for (int16_t c = c0; c < c1; c++)
						d[sx + c] = (uint8_t)(((src[c] & 0xE000) >> 8) | ((src[c] & 0x0700) >> 6) | ((src[c] & 0x0018) >> 3));
  #else
				for (int16_t c = c0; c < c1; c++)
					SetBufferBit((uint8_t*)dst, sx + c, sy, run ? color : src[c]);
  #endif
			}
			done += n;
			cx += n;
			if (cx == w)
			{
				cx = 0;
				cy++;
			}
		}
	}
#else
	uint16_t buffer[128];
	uint32_t left = (uint32_t)w * h;
	//the chip select sits on the I/O expander, so the whole image goes out in one
	//transaction. LovyanGFX's pushBlock and pushPixels open one of their own per call,
	//there the write variants are used inside this one
	SCREEN.startWrite();
	SCREEN.setAddrWindow(x, y, w, h);
	while (left > 0)
	{
		uint8_t control = PLATFORM_READ_BYTE(data++);
		uint32_t count = (control & 0x7F) + 1;
		if (count > left)
			count = left;
		if (control & 0x80)
		{
			uint16_t color = PLATFORM_READ_BYTE(data) | (PLATFORM_READ_BYTE(data + 1) << 8);
#if LOVYANGFX
			//a uint16_t colour is taken as plain RGB565
			SCREEN.writeColor(color, count);
#else
			SCREEN.pushBlock(color, count);
#endif
			data += 2;
		}
		else
		{
			//all literal pixels in one copy, little endian RGB565 like both devices
			PLATFORM_READ_BYTES((uint8_t*)buffer, data, count * sizeof(uint16_t));
			data += count * sizeof(uint16_t);
#if LOVYANGFX
			//true: the values are plain RGB565, the library puts them in display order
			SCREEN.writePixels(buffer, count, true);
#else
			SCREEN.pushPixels(buffer, count);
#endif
		}
		left -= count;
	}
	SCREEN.endWrite();
#endif
}

//length of the longest line in Text, the count restarts on every newline so a
//multi line message reports the width it actually needs and not its total length
uint8_t MaxLineLen(const char* Text)
{
	uint8_t Result = 0;
	uint8_t Len = 0;
	if (!Text)
		return 0;
	while (*Text)
	{
		if (*Text == '\n')
			Len = 0;
		else
		{
			Len++;
			if (Len > Result)
				Result = Len;
		}
		Text++;
	}
	return Result;
}

void UnLoadGraphics()
{
}

//the skin in use: the one FORCESKIN builds in (a 1 bpp buffer forces the black & white one), or
//the one picked in the options
uint8_t CurrentSkin(void)
{
#if FORCESKIN >= 0
	return FORCESKIN;
#else
	return skin;
#endif
}

void LoadGraphics(void)
{
	UnLoadGraphics();
	switch (CurrentSkin())
	{
#if SKINBUILT(0)
		//Default
		case 0:
			IMGBackground = default_background_rle;
			IMGIntro1 = default_intro1_rle;
			IMGIntro2 = default_intro2_rle;
			IMGIntro3 = default_intro3_rle;
			IMGTitleScreen = default_titlescreen_rle;
			IMGRoof1 = default_roof1_table_16_16_data;
			IMGRoof2 = default_roof2_table_16_16_data;
			IMGRoofCornerLeft = default_roofcornerleft_table_16_16_data;
			IMGRoofCornerRight = default_roofcornerright_table_16_16_data;
			IMGRoofDownRight = default_roofdownright_table_16_16_data;
			IMGRoofDownLeft = default_roofdownleft_table_16_16_data;
			IMGRoofCornerBoth = default_roofcornerboth_table_16_16_data;
			IMGExit = default_door_table_16_16_data;
			IMGFloor = default_floor_table_16_16_data;
			IMGBox = default_box_table_16_16_data;
			IMGPlayer = default_player_table_16_16_data;
			IMGFloorLeft = default_floorleft_table_16_16_data;
			IMGFloorRight = default_floorright_table_16_16_data;
			IMGEarthGrassRight = default_earthgrassright_table_16_16_data;
			IMGEarthGrassLeft = default_earthgrassleft_table_16_16_data;
			IMGEarthLeft = default_earthleft_table_16_16_data;
			IMGEarthRight = default_earthright_table_16_16_data;
			IMGEarthMiddle = default_earthmiddle_table_16_16_data;
			IMGFloatingFloor = default_floatingfloor_table_16_16_data;
			IMGFloatingFloorLeft = default_floatingfloorleft_table_16_16_data;
			IMGFloatingFloorRight = default_floatingfloorright_table_16_16_data;
			IMGFloatingFloorMiddle = default_floatingfloormiddle_table_16_16_data;
			IMGTower = default_tower_table_16_16_data;
			IMGStartTower = default_starttower_table_16_16_data;
			IMGTowerShaft = default_towershaft_table_16_16_data;
			ColorBlack = SCREEN.color565(107,128,128);
			ColorWhite = SCREEN.color565(255,255,255);
			break;
#endif
#if SKINBUILT(1)
		//Tech
		case 1:
			IMGBackground = tech_background_rle;
			IMGIntro1 = tech_intro1_rle;
			IMGIntro2 = tech_intro2_rle;
			IMGIntro3 = tech_intro3_rle;
			IMGTitleScreen = tech_titlescreen_rle;
			IMGRoof1 = tech_roof1_table_16_16_data;
			IMGRoof2 = tech_roof2_table_16_16_data;
			IMGRoofCornerLeft = tech_roofcornerleft_table_16_16_data;
			IMGRoofCornerRight = tech_roofcornerright_table_16_16_data;
			IMGRoofDownRight = tech_roofdownright_table_16_16_data;
			IMGRoofDownLeft = tech_roofdownleft_table_16_16_data;
			IMGRoofCornerBoth = tech_roofcornerboth_table_16_16_data;
			IMGExit = tech_door_table_16_16_data;
			IMGFloor = tech_floor_table_16_16_data;
			IMGBox = tech_box_table_16_16_data;
			IMGPlayer = tech_player_table_16_16_data;
			IMGFloorLeft = tech_floorleft_table_16_16_data;
			IMGFloorRight = tech_floorright_table_16_16_data;
			IMGEarthGrassRight = tech_earthgrassright_table_16_16_data;
			IMGEarthGrassLeft = tech_earthgrassleft_table_16_16_data;
			IMGEarthLeft = tech_earthleft_table_16_16_data;
			IMGEarthRight = tech_earthright_table_16_16_data;
			IMGEarthMiddle = tech_earthmiddle_table_16_16_data;
			IMGFloatingFloor = tech_floatingfloor_table_16_16_data;
			IMGFloatingFloorLeft = tech_floatingfloorleft_table_16_16_data;
			IMGFloatingFloorRight = tech_floatingfloorright_table_16_16_data;
			IMGFloatingFloorMiddle = tech_floatingfloormiddle_table_16_16_data;
			IMGTower = tech_tower_table_16_16_data;
			IMGStartTower = tech_starttower_table_16_16_data;
			IMGTowerShaft = tech_towershaft_table_16_16_data;
			ColorBlack = SCREEN.color565(6,6,72);
			ColorWhite = SCREEN.color565(255,255,255);
			
			break;
#endif
#if SKINBUILT(2)
		//flat
		case 2:
			IMGBackground = default_background_rle;
			IMGIntro1 = default_intro1_rle;
			IMGIntro2 = default_intro2_rle;
			IMGIntro3 = default_intro3_rle;
			IMGTitleScreen = flat_titlescreen_rle;
			IMGRoof1 = flat_roof1_table_16_16_data;
			IMGRoof2 = flat_roof2_table_16_16_data;
			IMGRoofCornerLeft = flat_roofcornerleft_table_16_16_data;
			IMGRoofCornerRight = flat_roofcornerright_table_16_16_data;
			IMGRoofDownRight = flat_roofdownright_table_16_16_data;
			IMGRoofDownLeft = flat_roofdownleft_table_16_16_data;
			IMGRoofCornerBoth = flat_roofcornerboth_table_16_16_data;
			IMGExit = default_door_table_16_16_data;
			IMGFloor = flat_floor_table_16_16_data;
			IMGBox = default_box_table_16_16_data;
			IMGPlayer = default_player_table_16_16_data;
			IMGFloorLeft = flat_floorleft_table_16_16_data;
			IMGFloorRight = flat_floorright_table_16_16_data;
			IMGEarthGrassRight = flat_earthgrassright_table_16_16_data;
			IMGEarthGrassLeft = flat_earthgrassleft_table_16_16_data;
			IMGEarthLeft = flat_earthleft_table_16_16_data;
			IMGEarthRight = flat_earthright_table_16_16_data;
			IMGEarthMiddle = flat_earthmiddle_table_16_16_data;
			IMGFloatingFloor = default_floatingfloor_table_16_16_data;
			IMGFloatingFloorLeft = default_floatingfloorleft_table_16_16_data;
			IMGFloatingFloorRight = default_floatingfloorright_table_16_16_data;
			IMGFloatingFloorMiddle = default_floatingfloormiddle_table_16_16_data;
			IMGTower = flat_tower_table_16_16_data;
			IMGStartTower = flat_starttower_table_16_16_data;
			IMGTowerShaft = flat_towershaft_table_16_16_data;
			ColorBlack = SCREEN.color565(107,128,128);
			ColorWhite = SCREEN.color565(255,255,255);
			
			break;
#endif
#if SKINBUILT(3)
		//ti-83
		case 3:
			IMGBackground = ti83_background_rle;
			IMGIntro1 = ti83_intro1_rle;
			IMGIntro2 = ti83_intro2_rle;
			IMGIntro3 = ti83_intro3_rle;
			IMGTitleScreen = ti83_titlescreen_rle;
			IMGRoof1 = ti83_roof1_table_16_16_data;
			IMGRoof2 = ti83_roof2_table_16_16_data;
			IMGRoofCornerLeft = ti83_roofcornerleft_table_16_16_data;
			IMGRoofCornerRight = ti83_roofcornerright_table_16_16_data;
			IMGRoofDownRight = ti83_roofdownright_table_16_16_data;
			IMGRoofDownLeft = ti83_roofdownleft_table_16_16_data;
			IMGRoofCornerBoth = ti83_roofcornerboth_table_16_16_data;
			IMGExit = ti83_door_table_16_16_data;
			IMGFloor = ti83_floor_table_16_16_data;
			IMGBox = ti83_box_table_16_16_data;
			IMGPlayer = ti83_player_table_16_16_data;
			IMGFloorLeft = ti83_floorleft_table_16_16_data;
			IMGFloorRight = ti83_floorright_table_16_16_data;
			IMGEarthGrassRight = ti83_earthgrassright_table_16_16_data;
			IMGEarthGrassLeft = ti83_earthgrassleft_table_16_16_data;
			IMGEarthLeft = ti83_earthleft_table_16_16_data;
			IMGEarthRight = ti83_earthright_table_16_16_data;
			IMGEarthMiddle = ti83_earthmiddle_table_16_16_data;
			IMGFloatingFloor = ti83_floatingfloor_table_16_16_data;
			IMGFloatingFloorLeft = ti83_floatingfloorleft_table_16_16_data;
			IMGFloatingFloorRight = ti83_floatingfloorright_table_16_16_data;
			IMGFloatingFloorMiddle = ti83_floatingfloormiddle_table_16_16_data;
			IMGTower = ti83_tower_table_16_16_data;
			IMGStartTower = ti83_starttower_table_16_16_data;
			IMGTowerShaft = ti83_towershaft_table_16_16_data;
			ColorWhite = SCREEN.color565(204,224,207);
			ColorBlack = SCREEN.color565(79,80,67);
			
			break;
#endif
#if SKINBUILT(4)
		//kenney
		case 4:
			IMGBackground = kenney_background_rle;
			IMGIntro1 = kenney_intro1_rle;
			IMGIntro2 = kenney_intro2_rle;
			IMGIntro3 = kenney_intro3_rle;
			IMGTitleScreen = kenney_titlescreen_rle;
			IMGRoof1 = kenney_roof1_table_16_16_data;
			IMGRoof2 = kenney_roof2_table_16_16_data;
			IMGRoofCornerLeft = kenney_roofcornerleft_table_16_16_data;
			IMGRoofCornerRight = kenney_roofcornerright_table_16_16_data;
			IMGRoofDownRight = kenney_roofdownright_table_16_16_data;
			IMGRoofDownLeft = kenney_roofdownleft_table_16_16_data;
			IMGRoofCornerBoth = kenney_roofcornerboth_table_16_16_data;
			IMGExit = kenney_door_table_16_16_data;
			IMGFloor = kenney_floor_table_16_16_data;
			IMGBox = kenney_box_table_16_16_data;
			IMGPlayer = kenney_player_table_16_16_data;
			IMGFloorLeft = kenney_floorleft_table_16_16_data;
			IMGFloorRight = kenney_floorright_table_16_16_data;
			IMGEarthGrassRight = kenney_earthgrassright_table_16_16_data;
			IMGEarthGrassLeft = kenney_earthgrassleft_table_16_16_data;
			IMGEarthLeft = kenney_earthleft_table_16_16_data;
			IMGEarthRight = kenney_earthright_table_16_16_data;
			IMGEarthMiddle = kenney_earthmiddle_table_16_16_data;
			IMGFloatingFloor = kenney_floatingfloor_table_16_16_data;
			IMGFloatingFloorLeft = kenney_floatingfloorleft_table_16_16_data;
			IMGFloatingFloorRight = kenney_floatingfloorright_table_16_16_data;
			IMGFloatingFloorMiddle = kenney_floatingfloormiddle_table_16_16_data;
			IMGTower = kenney_tower_table_16_16_data;
			IMGStartTower = kenney_starttower_table_16_16_data;
			IMGTowerShaft = kenney_towershaft_table_16_16_data;
			ColorBlack = SCREEN.color565(0,0,0);
			ColorWhite = SCREEN.color565(255,255,255);
			
			break;
#endif
	}
}

void LoadFonts(void)
{

}

void FindLevelPacks(void)
{
	FoundLevelPacks = 2;
}


void FindLevels(void)
{
	switch(CurrentLevelPackIndex)
	{
		case 0:
			InstalledLevels = 21;
			break;
		case 1:
			InstalledLevels = 4;
			break;

	}
}

bool AskQuestionUpdate(int8_t* Id, bool* Answer, bool MustBeAButton)
{
	*Answer = false;
	*Id = AskingQuestionID;

	if (AskingQuestionID > -1)
	{
		if (((currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A))))
		{
			*Answer = true;
			AskingQuestion = false;
			NeedRedraw = true;
			//the question box was drawn over the board
			CWorldParts_MarkAllDirty();
			AskingQuestionID = -1;
			prevButtons = currButtons;
			playMenuSelectSound();
			return true;
		}

		if (!MustBeAButton && (((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B)))))
		{
			*Answer = false;
			AskingQuestion = false;
			NeedRedraw = true;
			//the question box was drawn over the board
			CWorldParts_MarkAllDirty();
			AskingQuestionID = -1;
			prevButtons = currButtons;
			playMenuBackSound();
			return true;
		}
	}
	return false;
}

void AskQuestion(int8_t Id, const char* Msg)
{
	uint8_t fh = 8;
	uint8_t count = 0;
	const char* s = Msg;
	while (*s) {
		if (*s == '\n')
			count++;
		s++;
	}
	int16_t MsgHeight = (count + 2) * fh;
	int16_t MsgWidth = MaxLineLen(Msg)*6;
	int16_t MsgX = (WINDOW_WIDTH - MsgWidth) >> 1;
	int16_t MsgY = (WINDOW_HEIGHT - MsgHeight) >> 1;
	GFX.fillRect(MsgX - 5, MsgY - 5, MsgWidth + 10, MsgHeight + 10, ColorWhite);
	GFX.drawRect(MsgX - 5, MsgY - 5, MsgWidth + 10, MsgHeight + 10, ColorBlack);
	GFX.drawRect(MsgX - 3, MsgY - 3, MsgWidth + 6, MsgHeight + 6, ColorBlack);
	tftPrint(MsgX, MsgY, Msg, ColorBlack, ColorBlack, 1);

	AskingQuestionID = Id;
	AskingQuestion = (Id > -1);
	prevButtons = currButtons;
}


void LoadSelectedLevel(void)
{
	if ((SelectedLevel > 0) && (SelectedLevel <= InstalledLevels))
	{
		{
			CWorldParts_Load(WorldParts, CurrentLevelPackIndex, SelectedLevel-1);
		}
		
	}
}

void PlayLevelIfNoErrorsFound()
{
	uint8_t errType;
	if (!LevelErrorsFound(&errType))
	{
		GameState = GSGameInit;
	}
	else
	{
		if (errType == errNoPlayer)
		{
			AskQuestion(qsErrPlayer, "Can not play this level because there\nis no player in the level! Please add\na Player and try again.\n\nPress (A) to continue");
		}
		else if (errType == errNoExit)
		{
			AskQuestion(qsErrExit, "Can not play this level because there\nis no exit in the level! Please add an\nexit and try again.\n\nPress (A) to continue");
		}
		else if (errType == errBlocksPlayerNotOnAFloor)
		{
			AskQuestion(qsErrBlocksOrPlayerNotOnAFloor, "Can not play this level because there\nare boxes, players or exits not on a\nfloor!\nPlease correct this and and try again.\n\nPress (A) to continue");
		}
		else if (errType == errBlocksOnPlayerNotOne)
		{
			AskQuestion(qsErrBlocksOrPlayerNotOnAFloor, "Can not play this level because the\nplayer is carrying more than one box!\nPlease correct this and and try again.\n\nPress (A) to continue");
		}
	}
}

bool LevelErrorsFound(uint8_t* ErrorType)
{
	//there is only one player, the other counts can reach the number of parts
	uint8_t NumPlayer = 0, NumPlayerNotFloor = 0, NumBlocksOnPlayerNotOne = 0;
	uint16_t NumExit = 0, NumBlocksNotFloor = 0, NumExitsNotFloor = 0;
	CWorldPart* Part;

	*ErrorType = errNoError;

	if(WorldParts->Player)
	{
		NumPlayer++;
		Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY + 1);
		if (Part == NULL)
			NumPlayerNotFloor++;

		Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY - 1);
		if (Part != NULL)
		{
			if (Part->Group == GroupBox)
			{
				Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY - 2);
				if (Part != NULL)
				{
					if (Part->Group == GroupBox)
						NumBlocksOnPlayerNotOne++;
				}
			}
		}
	}
	
	for (uint16_t Teller = 0; Teller < WorldParts->ItemCount; Teller++)
	{

		if (WorldParts->Items[Teller]->Type == IDExit)
			NumExit++;

		if (WorldParts->Items[Teller]->Group == GroupBox)
		{
			Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Items[Teller]->PlayFieldX, WorldParts->Items[Teller]->PlayFieldY + 1);
			if (Part == NULL)
				NumBlocksNotFloor++;
		}
		
		if (WorldParts->Items[Teller]->Group == GroupExit)
		{
			Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Items[Teller]->PlayFieldX, WorldParts->Items[Teller]->PlayFieldY + 1);
			if (Part == NULL)
				NumExitsNotFloor++;
		}
	}

	if (NumPlayer == 0)
	{

		*ErrorType = errNoPlayer;
		return true;
	}

	else if (NumExit == 0)
	{
		*ErrorType = errNoExit;
		return true;
	}

	else if ((NumBlocksNotFloor > 0) || (NumPlayerNotFloor > 0) || (NumExitsNotFloor > 0))
	{
		*ErrorType = errBlocksPlayerNotOnAFloor;
		return true;
	}

	else if (NumBlocksOnPlayerNotOne > 0)
	{
		*ErrorType = errBlocksOnPlayerNotOne;
		return true;
	}

	return false;
}