#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "cworldparts.h"
#include "cworldpart.h"
#include "commonvars.h"
#include "levels.h"
#include "gamefuncs.h"

//the buffers of the dirty band rendering further down, they come and go with the board
static void DrawBuffersInit();
static void DrawBuffersDeinit();

CWorldParts* CWorldParts_Create()
{
	CWorldParts* Result = (CWorldParts*) malloc(sizeof(CWorldParts));
	if (Result)
	{
		//when it can not be allocated the board still works, it just gets no parts
		CWorldPart_PoolInit();
		//same for these, without them the board is just not painted
		DrawBuffersInit();
		CWorldParts_ClearPositionalItems(Result);
		Result->ItemCount = 0;
		Result->Player = NULL;
		Result->IgnorePart = NULL;
		Result->DisableSorting = false;
		Result->AttchedBoxQuedOrMoving = false;
		Result->NumPartsMoving = 0;
		Result->NumPartsMovingQueued = 0;
		Result->NumBoxesAttachedToPlayer = 0;
		Result->NumPartsAttachedToPlayer = 0;
		Result->MoveQueOwner = NoWorldPart;
		Result->MoveQueBack = -1;
		Result->MoveQueFront = -1;
		Result->ViewPort = CViewPort_Create(0, 0, NrOfColsVisible -1, NrOfRowsVisible - 1, 0, 0, NrOfCols - 1, NrOfRows - 1);
	}
	return Result;
}

void CWorldParts_FindPlayer(CWorldParts* self)
{
	for (uint16_t i = 0; i < self->ItemCount; i++)
	{
		if ((self->Items[i]->Group == GroupPlayer) && (self->Items[i] != self->IgnorePart))
		{
			self->Player = self->Items[i];
			break;
		}
	}
}

void CWorldParts_ClearPositionalItems(CWorldParts* self)
{
	for (uint8_t i = 0; i < NrOfGroups; i++)
	{
		self->PositionalItemsCount[i] = 0;
		for (uint8_t y = 0; y < NrOfRows; y++)
			for (uint8_t x = 0; x < NrOfCols; x++)
				self->PositionalItems[i][y][x] = NoWorldPart;
	}
}

void CWorldParts_CenterVPOnPlayer(CWorldParts* self)
{
	if (self->Player == NULL)
		CWorldParts_FindPlayer(self);

	//The window is NrOfColsVisible by NrOfRowsVisible tiles. Asking for
	//centre - half .. centre + half spans one tile too many, because SetViewPort
	//adds another one to the max, and MaxScreenX / MaxScreenY then sit a whole tile
	//past the right / bottom edge of the screen. CViewPort_Move compares those
	//against the level limit, so scrolling stopped a tile early and the last column
	//and row of a level could never be brought into view
	//a player position minus half the visible tiles, that count grows with the resolution
	int16_t VPX, VPY;
	if (self->Player)
	{
		VPX = self->Player->PlayFieldX - ((NrOfColsVisible) >> 1);
		VPY = self->Player->PlayFieldY - ((NrOfRowsVisible) >> 1);
	}
	else
	{
		VPX = (NrOfCols >> 1) - ((NrOfColsVisible) >> 1);
		VPY = (NrOfRows >> 1) - ((NrOfRowsVisible) >> 1);
	}
	//this used to go through the global WorldParts instead of self
	CViewPort_SetViewPort(self->ViewPort, VPX, VPY, VPX + NrOfColsVisible - 1, VPY + NrOfRowsVisible - 1);
	//the whole view just jumped to a new place
	CWorldParts_MarkAllDirty();
}

void CWorldParts_LimitVPLevel(CWorldParts* self)
{
	int8_t MinX = NrOfCols, MinY = NrOfRows, MaxX = -1, MaxY = -1;
	for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
	{
		if (self->Items[Teller]->PlayFieldX < MinX)
			MinX = self->Items[Teller]->PlayFieldX;
		if (self->Items[Teller]->PlayFieldY < MinY)
			MinY = self->Items[Teller]->PlayFieldY;
		if (self->Items[Teller]->PlayFieldX > MaxX)
			MaxX = self->Items[Teller]->PlayFieldX;
		if (self->Items[Teller]->PlayFieldY > MaxY)
			MaxY = self->Items[Teller]->PlayFieldY;
	}
	CViewPort_SetVPLimit(self->ViewPort, MinX, MinY, MaxX, MaxY);
	CWorldParts_CenterVPOnPlayer(self);
}

void CWorldParts_RemoveAll(CWorldParts* self)
{
	for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
	{
		if (self->Items[Teller])
		{
			CWorldPart_free(self->Items[Teller]);
			self->Items[Teller] = NULL;
		}
	}
	CWorldParts_ClearPositionalItems(self);
	self->Player = NULL;
	self->ItemCount = 0;
	//every part on screen is gone
	CWorldParts_MarkAllDirty();
}

//the square a part was last painted on, it has to be painted again once the part is gone
static void MarkPrevDrawDirty(CWorldParts* self, CWorldPart* Part)
{
	CWorldParts_MarkDirty(Part->PrevDrawX - self->ViewPort->MinScreenX,
	                      Part->PrevDrawY - self->ViewPort->MinScreenY,
	                      TileWidth, TileHeight);
}

void CWorldParts_Remove(CWorldParts* self, int8_t PlayFieldXin, int8_t PlayFieldYin)
{
	for (uint16_t Teller1 = 0; Teller1 < self->ItemCount; Teller1++)
	{
		if (self->Items[Teller1] && (self->Items[Teller1] != self->IgnorePart))
		{
			if ((self->Items[Teller1]->PlayFieldX == PlayFieldXin) && (self->Items[Teller1]->PlayFieldY == PlayFieldYin))
			{

				if (self->Items[Teller1]->Group == GroupPlayer)
				{
					self->Player = NULL;
				}
				
				if (self->Items[Teller1]->Group != GroupNone)
				{
					self->PositionalItemsCount[self->Items[Teller1]->Group]--;
					self->PositionalItems[self->Items[Teller1]->Group][self->Items[Teller1]->PlayFieldY][self->Items[Teller1]->PlayFieldX] = NoWorldPart;
				}

				MarkPrevDrawDirty(self, self->Items[Teller1]);

				//now clear memory & remove for item list
				CWorldPart_free(self->Items[Teller1]);
				self->Items[Teller1] = NULL;
				for (uint16_t Teller2 = Teller1; Teller2 + 1 < self->ItemCount; Teller2++)
					self->Items[Teller2] = self->Items[Teller2 + 1];
				self->ItemCount--;
				Teller1--;
			}
		}
	}
}

void CWorldParts_RemoveType(CWorldParts* self, uint8_t Type)
{
	for (uint16_t Teller1 = 0; Teller1 < self->ItemCount; Teller1++)
	{
		if (self->Items[Teller1] && (self->Items[Teller1] != self->IgnorePart))
		{
			if (self->Items[Teller1]->Type == Type)
			{
				if (Type == IDPlayer)
				{
					self->Items[Teller1]->Player = NULL;
					self->Player = NULL;
				}
				
				if (self->Items[Teller1]->Group != GroupNone)
				{
					self->PositionalItemsCount[self->Items[Teller1]->Group]--;
					self->PositionalItems[self->Items[Teller1]->Group][self->Items[Teller1]->PlayFieldY][self->Items[Teller1]->PlayFieldX] = NoWorldPart;
				}

				MarkPrevDrawDirty(self, self->Items[Teller1]);

				//now clear memory & remove for item list
				CWorldPart_free(self->Items[Teller1]);
				self->Items[Teller1] = NULL;
				for (uint16_t Teller2 = Teller1; Teller2 + 1 < self->ItemCount; Teller2++)
					self->Items[Teller2] = self->Items[Teller2 + 1];
				self->ItemCount--;
				Teller1--;
			}
		}
	}
}



void CWorldParts_Sort(CWorldParts* self)
{
	uint16_t Teller2;
	uint8_t Group;
	int8_t Y;
	CWorldPart* Part;
	if (!self->DisableSorting)
	{
		for (uint16_t Teller1 = 1; Teller1 < self->ItemCount; Teller1++)
		{
			Group = self->Items[Teller1]->Group;
			Y = self->Items[Teller1]->PlayFieldY;
			Part = self->Items[Teller1];
			Teller2 = Teller1;
			//need to sort on group for drawing but also on playfieldY for same Z so that 1st item is the highest one, otherwise blocks don't fall at same time
			while ((Teller2 > 0) && ((self->Items[Teller2 - 1]->Group > Group) || ((self->Items[Teller2 - 1]->Group == Group) && (self->Items[Teller2-1]->PlayFieldY < Y))))
			{
				self->Items[Teller2] = self->Items[Teller2 - 1];
				Teller2--;
			}
			self->Items[Teller2] = Part;
		}
	}

}

void CWorldParts_Add(CWorldParts* self, CWorldPart* WorldPart)
{
	//CWorldPart_create returns NULL when the heap is full, every call site passes
	//its result straight in here. Adding that would fault on the next field access
	if (!WorldPart)
		return;
	if (self->ItemCount < MAXWORLDPARTS)
	{
		if (WorldPart->Type == IDPlayer)
			self->Player = WorldPart;
		//no need to mark it dirty, it has never been painted so the board picks it up by itself
		self->Items[self->ItemCount] = WorldPart;
		if (WorldPart->Group != GroupNone)
		{
			self->PositionalItems[WorldPart->Group][WorldPart->PlayFieldY][WorldPart->PlayFieldX] = CWorldPart_PoolIndex(WorldPart);
			self->PositionalItemsCount[WorldPart->Group]++;
		}
		self->ItemCount++;
		CWorldParts_Sort(self);
	}
}

void CWorldParts_Load(CWorldParts* self, uint8_t levelPack, uint8_t Level)
{
	//.lev coordinates are bytes and the minimum is subtracted before use, so every
	//position stays in 0..255
	uint8_t X, Y;
	uint8_t Type;
	CWorldParts_RemoveAll(self);
	uint8_t minx = 255;
	uint8_t miny = 255;
	uint8_t maxx = 0;
	uint8_t maxy = 0;
	const uint8_t * level = level_data_files[levelPack][Level];
	if (level)
	{
		while (PLATFORM_READ_BYTE(level) != 0xff)
		{
			Type = PLATFORM_READ_BYTE(level++);
			X = PLATFORM_READ_BYTE(level++);
			Y = PLATFORM_READ_BYTE(level++);
			if (X < minx)
				minx = X;
			if (Y < miny)
				miny = Y;
			if (X > maxx)
				maxx = X;
			if (Y > maxy)
				maxy = Y;
		}
	}
	level = level_data_files[levelPack][Level];
	if (level)
	{
		self->DisableSorting = true;
			
		while (PLATFORM_READ_BYTE(level) != 0xff)
		{	
			Type = PLATFORM_READ_BYTE(level++);		
			X = PLATFORM_READ_BYTE(level++);
			Y = PLATFORM_READ_BYTE(level++);
			X -= minx;
			Y -= miny;
			switch (Type)
			{
			case IDEmpty:
				CWorldParts_Add(self, CWorldPart_create(X, Y, Type, GroupNone));
				break;
			case IDBox:
				CWorldParts_Add(self, CWorldPart_create(X, Y, Type, GroupBox));
				break;
			case IDPlayer:
				CWorldParts_Add(self, CWorldPart_create(X, Y, Type, GroupPlayer));
				break;
			case IDExit:
				CWorldParts_Add(self, CWorldPart_create(X, Y, Type, GroupExit));
				break;
			case IDEarthGrassLeft:
			case IDEarthGrassRight:
			case IDEarthLeft:
			case IDEarthMiddle:
			case IDEarthRight:
			case IDFloatingFloor:
			case IDFloatingFloorLeft:
			case IDFloatingFloorMiddle:
			case IDFloatingFloorRight:
			case IDFloorLeft:
			case IDFloorRight:
			case IDTower:
			case IDStartTower:
			case IDTowerShaft:
			case IDRoof1:
			case IDRoof2:
			case IDRoofCornerLeft:
			case IDRoofCornerRight:
			case IDRoofCornerBoth:
			case IDRoofDownRight:
			case IDRoofDownLeft:
			case IDFloor:
				CWorldParts_Add(self, CWorldPart_create(X, Y, Type, GroupFloor));
				break;

			}
		}

		//a level less tall than the screen leaves an empty strip below it, fill it with
		//earth under the whole width of the level (with 8x8 tiles that is one row for
		//the 15 row levels, with 16x16 tiles every level is taller than the screen)
		for (Y = maxy - miny + 1; (Y < NrOfRowsVisible) && (Y < NrOfRows); Y++)
			//X is a byte, the NrOfCols bound also keeps this loop from never ending
			for (X = 0; (X <= maxx - minx) && (X < NrOfCols); X++)
				CWorldParts_Add(self, CWorldPart_create(X, Y, IDEarthMiddle, GroupFloor));

		self->DisableSorting = false;
		CWorldParts_Sort(self);
		CWorldParts_LimitVPLevel(self);
		CWorldParts_CenterVPOnPlayer(self);
	}

}

bool CWorldParts_Move(CWorldParts* self)
{
	bool result = false;
	self->NumPartsMoving = 0;
	self->NumPartsMovingQueued = 0;
	self->NumPartsAttachedToPlayer = 0 ;
	self->NumBoxesAttachedToPlayer = 0;
	self->AttchedBoxQuedOrMoving = false;
	for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
	{
		result |= CWorldPart_Move(self->Items[Teller]);

		if (WorldParts->Items[Teller]->IsMoving)
		{
			self->NumPartsMoving++;
		}
		
		if (WorldParts->Items[Teller]->Player)
		{
			self->NumPartsAttachedToPlayer++;

			if (WorldParts->Items[Teller]->Type == IDBox)
			{			
				self->NumBoxesAttachedToPlayer++;
				self->AttchedBoxQuedOrMoving = CWorldPart_MovesInQue(WorldParts->Items[Teller]) || WorldParts->Items[Teller]->IsMoving;
			}
		}

		if (CWorldPart_MovesInQue(WorldParts->Items[Teller]))
			self->NumPartsMovingQueued++;
			
	}
	return result;
}

// ===========================================================================
// Dirty band board rendering (same approach as blips_espboy)
//
// The screen is tracked as a grid of tile sized cells, but repainting is done a
// row of cells at a time: the dirty cells of a row are composed together
// (background first, then the parts in list order) in one buffer and sent in a
// single transfer. Standing still costs nothing, walking repaints the rows the
// player covers and a scroll repaints the whole screen. Nothing half drawn ever
// reaches the display, so there is no flicker either way.
//
// Parts do not flag themselves dirty. Every frame the board compares each part's
// position and anim phase with what it last painted (PrevDrawX, PrevDrawY,
// PrevDrawAnimPhase) and repaints the old and the new square when they differ.
// ===========================================================================

#define CELLSX (WINDOW_WIDTH / TileWidth)
#define CELLSY (WINDOW_HEIGHT / TileHeight)
//one bit per cell of a row, the smallest type that has a bit for every cell the
//screen is wide (16 at 128 px wide with 8x8 tiles, 20 at 320 px with 16x16 tiles)
#if CELLSX <= 8
typedef uint8_t CellRow;
#elif CELLSX <= 16
typedef uint16_t CellRow;
#elif CELLSX <= 32
typedef uint32_t CellRow;
#else
typedef uint64_t CellRow;
#endif
static_assert(CELLSX <= 64, "cellDirty keeps one bit per cell of a row in at most a uint64_t");

//The buffers below are allocated with the board (DrawBuffersInit) and not reserved as
//globals, so nothing that runs instead of the game (the web app store) pays for them.
//When one of them could not be allocated they are all NULL and the board is not painted
static CellRow* cellDirty = NULL; //CELLSY rows
#if SCREENBUFFER == 0
//A whole cell row at a time. Half rows cost twice the work for every sprite: a part is
//TileHeight tall, so it lands in both halves and its pixels are walked for each of them.
//A full row is one window, one push and every part handled once
#define BANDHEIGHT (TileHeight)
static uint16_t* bandBuf = NULL; //the strip being composed, WINDOW_WIDTH * BANDHEIGHT pixels
static int16_t bandX0, bandY0, bandW;               //where that strip sits on screen
static int16_t lastMinScreenX = -30000, lastMinScreenY = -30000;

//The background is run length encoded (see pushImageRLE). To start decoding in the
//middle of it, this keeps for every screen row the control byte the row starts in
//and how many pixels of that control belong to the rows above it.
//an encoded background is at most 3 bytes per pixel (a one pixel run every time),
//so the offsets need 32 bits once the screen is bigger than about 147x147
#if WINDOW_WIDTH * WINDOW_HEIGHT * 3 < 65536
typedef uint16_t BgOffset;
#else
typedef uint32_t BgOffset;
#endif
static BgOffset* bgRowOffset = NULL; //WINDOW_HEIGHT rows
static uint8_t* bgRowUsed = NULL;    //WINDOW_HEIGHT rows
static const uint8_t* bgIndexed = NULL;

//pixels are little endian RGB565, read per byte as the images are uint8_t arrays
static inline uint16_t ReadPixel(const uint8_t* p)
{
	return PLATFORM_READ_BYTE(p) | (PLATFORM_READ_BYTE(p + 1) << 8);
}

//count pixels of an image into the strip. Runs here are a few pixels at a time, a sprite
//row is 8 of them: where flash is plain memory that is a short copy of 16 bit values, and
//calling memcpy for it costs more than the copy itself
static inline void BandCopy(uint16_t* dst, const uint8_t* src, int16_t count)
{
#if PLATFORM_DIRECT_FLASH
	//A 16 bit read needs an even address: a core like the Cortex-M0+ faults on an odd one.
	//The pixels of an encoded row sit wherever the control bytes leave them, so half of the
	//time they are odd and the two bytes are put together by hand
	if (((uintptr_t)src & 1) == 0)
	{
		const uint16_t* s = (const uint16_t*)src;
		for (int16_t i = 0; i < count; i++)
			dst[i] = s[i];
	}
	else
	{
		for (int16_t i = 0; i < count; i++, src += 2)
			dst[i] = (uint16_t)(src[0] | (src[1] << 8));
	}
#else
	PLATFORM_READ_BYTES((uint8_t*)dst, src, count * sizeof(uint16_t));
#endif
}

//the same, but leaving the pixels of the image that carry the transparent key
static inline void BandCopyKeyed(uint16_t* dst, const uint8_t* src, int16_t count)
{
#if PLATFORM_DIRECT_FLASH
	if (((uintptr_t)src & 1) == 0)
	{
		const uint16_t* s = (const uint16_t*)src;
		for (int16_t i = 0; i < count; i++)
			//magenta is the transparent key, 0xF81F in RGB565
			if (s[i] != 0xF81F)
				dst[i] = s[i];
		return;
	}
	for (int16_t i = 0; i < count; i++, src += 2)
	{
		const uint16_t col = (uint16_t)(src[0] | (src[1] << 8));
		if (col != 0xF81F)
			dst[i] = col;
	}
#else
	//reading this a pixel at a time would be a flash read each, the row is copied first
	uint16_t row[TileWidth];
	PLATFORM_READ_BYTES((uint8_t*)row, src, count * sizeof(uint16_t));
	for (int16_t i = 0; i < count; i++)
		if (row[i] != 0xF81F)
			dst[i] = row[i];
#endif
}
#endif

static void DrawBuffersDeinit()
{
	free(cellDirty);
	cellDirty = NULL;
#if SCREENBUFFER == 0
	free(bandBuf);
	bandBuf = NULL;
	free(bgRowOffset);
	bgRowOffset = NULL;
	free(bgRowUsed);
	bgRowUsed = NULL;
	bgIndexed = NULL;
#endif
}

static void DrawBuffersInit()
{
#if SCREENBUFFER == 0
	if (cellDirty)
		return;
	cellDirty = (CellRow*)calloc(CELLSY, sizeof(CellRow));
	bandBuf = (uint16_t*)malloc(WINDOW_WIDTH * BANDHEIGHT * sizeof(uint16_t));
	bgRowOffset = (BgOffset*)malloc(WINDOW_HEIGHT * sizeof(BgOffset));
	bgRowUsed = (uint8_t*)malloc(WINDOW_HEIGHT * sizeof(uint8_t));
	if (!cellDirty || !bandBuf || !bgRowOffset || !bgRowUsed)
	{
		DrawBuffersDeinit();
		return;
	}
	//the background index is gone with the old buffers, and the first draw has to paint everything
	bgIndexed = NULL;
	lastMinScreenX = -30000;
	lastMinScreenY = -30000;
#endif
	//with a screen buffer the whole board is drawn every frame, nothing is tracked
}

//marking is called from all over the world part code, with a buffer it is unused but harmless

void CWorldParts_MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h)
{
	if (!cellDirty || (w <= 0) || (h <= 0))
		return;
	int16_t x1 = x + w - 1;
	int16_t y1 = y + h - 1;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x1 > WINDOW_WIDTH - 1) x1 = WINDOW_WIDTH - 1;
	if (y1 > WINDOW_HEIGHT - 1) y1 = WINDOW_HEIGHT - 1;
	if ((x > x1) || (y > y1))
		return;
	for (int16_t cy = y / TileHeight; cy <= y1 / TileHeight; cy++)
		for (int16_t cx = x / TileWidth; cx <= x1 / TileWidth; cx++)
			cellDirty[cy] |= (CellRow)((CellRow)1 << cx);
}

void CWorldParts_MarkAllDirty()
{
	if (!cellDirty)
		return;
	for (int16_t cy = 0; cy < CELLSY; cy++)
		//every bit up to CELLSX, shifting by the full width of the type is undefined
		cellDirty[cy] = (CellRow)((CellRow)~(CellRow)0 >> (sizeof(CellRow) * 8 - CELLSX));
}

#if SCREENBUFFER == 0
static void IndexBackground()
{
	const uint8_t* data = IMGBackground;
	uint32_t pixel = 0; //first pixel the current control covers
	int16_t row = 0;
	while (row < WINDOW_HEIGHT)
	{
		uint8_t control = PLATFORM_READ_BYTE(data);
		uint32_t count = (control & 0x7F) + 1;
		//every row that starts inside this control
		while ((row < WINDOW_HEIGHT) && ((uint32_t)row * WINDOW_WIDTH < pixel + count))
		{
			bgRowOffset[row] = (BgOffset)(data - IMGBackground);
			bgRowUsed[row] = (uint8_t)((uint32_t)row * WINDOW_WIDTH - pixel);
			row++;
		}
		pixel += count;
		data += (control & 0x80) ? 3 : 1 + count * 2;
	}
	bgIndexed = IMGBackground;
}

//the background is a full screen image so it lines up with the strip
static void BandBackground()
{
	if (!IMGBackground)
	{
		for (uint16_t i = 0; i < bandW * BANDHEIGHT; i++)
			bandBuf[i] = ColorWhite;
		return;
	}
	//a new skin brings a new background
	if (bgIndexed != IMGBackground)
		IndexBackground();

	for (int16_t r = 0; r < BANDHEIGHT; r++)
	{
		const uint8_t* data = IMGBackground + bgRowOffset[bandY0 + r];
		//used / count / avail stay in 1..128, a control never covers more pixels than that.
		//skip and left are screen positions / widths, so they grow with WINDOW_WIDTH
		uint8_t used = bgRowUsed[bandY0 + r]; //pixels of this control that lie before the row
		uint16_t skip = bandX0;               //pixels of the row left of the strip
		uint16_t left = bandW;
		uint16_t* drow = &bandBuf[r * bandW];
		while (left > 0)
		{
			uint8_t control = PLATFORM_READ_BYTE(data);
			uint8_t count = (control & 0x7F) + 1;
			bool run = (control & 0x80) != 0;
			uint8_t avail = count - used;
			if (skip >= avail)
				skip -= avail;
			else
			{
				used += skip;
				avail -= skip;
				skip = 0;
				if (avail > left)
					avail = left;
				if (run)
				{
					uint16_t col = ReadPixel(data + 1);
					for (uint8_t i = 0; i < avail; i++)
						*drow++ = col;
				}
				else
				{
					//the pixels are little endian RGB565 like the strip, copied as they are
					BandCopy(drow, data + 1 + used * 2, avail);
					drow += avail;
				}
				left -= avail;
			}
			used = 0;
			data += run ? 3 : 1 + count * 2;
		}
	}
}

//blit a 16x16 sprite that sits at screen position sx,sy, clipped to the strip
static void BandSprite(int16_t sx, int16_t sy, const uint8_t* image)
{
	if (!image)
		return;
	//cheap reject before touching the pixels
	if ((sy + TileHeight <= bandY0) || (sy >= bandY0 + BANDHEIGHT) ||
		(sx + TileWidth <= bandX0) || (sx >= bandX0 + bandW))
		return;

	//only the rows and columns of the sprite that fall inside the strip
	int16_t r0 = bandY0 - sy, r1 = bandY0 + BANDHEIGHT - sy;
	int16_t c0 = bandX0 - sx, c1 = bandX0 + bandW - sx;
	if (r0 < 0) r0 = 0;
	if (r1 > TileHeight) r1 = TileHeight;
	if (c0 < 0) c0 = 0;
	if (c1 > TileWidth) c1 = TileWidth;
	//a visible row at a time, the transparent pixels of the sprite are left as they are
	for (int16_t r = r0; r < r1; r++)
	{
		uint16_t* drow = &bandBuf[(sy + r - bandY0) * bandW + (sx + c0 - bandX0)];
		const uint8_t* src = image + (r * TileWidth + c0) * sizeof(uint16_t);
		BandCopyKeyed(drow, src, c1 - c0);
	}
}

#endif

//is this part in view, same test the old full screen draw used
static bool PartVisible(CWorldParts* self, CWorldPart* Part)
{
	return (Part != self->IgnorePart) &&
		(Part->PlayFieldX >= self->ViewPort->VPMinX) && (Part->PlayFieldX - 1 <= self->ViewPort->VPMaxX) &&
		(Part->PlayFieldY >= self->ViewPort->VPMinY) && (Part->PlayFieldY - 1 <= self->ViewPort->VPMaxY);
}

//the parts in view, in the order the list is sorted in so layering is kept
void CWorldParts_Draw(CWorldParts* self)
{
	for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
		if (PartVisible(self, self->Items[Teller]))
			CWorldPart_Draw(self->Items[Teller]);
}

#if SCREENBUFFER
//With a buffer the whole frame is drawn off screen and sent in one go, so there is
//nothing to track: the board is simply drawn again every frame, which is what the
//game did before any of the dirty cell work.
bool CWorldParts_DrawBoard(CWorldParts* self)
{
	if (IMGBackground)
		pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGBackground);
	else
		GFX.fillRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, ColorWhite);

	CWorldParts_Draw(self);

	//every pixel of the board was drawn, anything that sits on top has to follow
	return true;
}
#else
bool CWorldParts_DrawBoard(CWorldParts* self)
{
	bool painted = false;
	//the buffers could not be allocated
	if (!cellDirty)
		return painted;
	//read once, these are used for every part of every strip
	const int16_t msx = self->ViewPort->MinScreenX;
	const int16_t msy = self->ViewPort->MinScreenY;

	//the viewport scrolled, so every pixel on screen moved
	if ((msx != lastMinScreenX) || (msy != lastMinScreenY))
	{
		lastMinScreenX = msx;
		lastMinScreenY = msy;
		CWorldParts_MarkAllDirty();
	}

	for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
	{
		CWorldPart* Part = self->Items[Teller];
		//animation advances once per frame for the parts in view
		if (PartVisible(self, Part))
			CWorldPart_Event_BeforeDraw(Part);
		//anything that moved or changed its image repaints where it was and where it is
		if ((Part->X != Part->PrevDrawX) || (Part->Y != Part->PrevDrawY) || (Part->AnimPhase != Part->PrevDrawAnimPhase))
		{
			CWorldParts_MarkDirty(Part->PrevDrawX - msx, Part->PrevDrawY - msy, TileWidth, TileHeight);
			CWorldParts_MarkDirty(Part->X - msx, Part->Y - msy, TileWidth, TileHeight);
			Part->PrevDrawX = Part->X;
			Part->PrevDrawY = Part->Y;
			Part->PrevDrawAnimPhase = Part->AnimPhase;
		}
	}

	//compose and push one strip per row that has dirty cells
	for (int16_t cy = 0; cy < CELLSY; cy++)
	{
		if (!cellDirty[cy])
			continue;

		if (!painted)
			SCREEN.startWrite();

		//the run from the first to the last dirty cell of the row goes out as one
		int16_t first = -1, last = -1;
		for (int16_t cx = 0; cx < CELLSX; cx++)
			if (cellDirty[cy] & ((CellRow)1 << cx))
			{
				if (first < 0)
					first = cx;
				last = cx;
			}
		cellDirty[cy] = 0;

		bandX0 = first * TileWidth;
		bandW = (last - first + 1) * TileWidth;

		//one window for the whole row, the halves are streamed into it in order
		SCREEN.setAddrWindow(bandX0, cy * TileHeight, bandW, TileHeight);

		bandY0 = cy * TileHeight;

		BandBackground();

		//the parts, in the order the list is sorted in so layering is kept
		for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
		{
			CWorldPart* Part = self->Items[Teller];
			int16_t sy = Part->Y - msy;
			if ((sy + TileHeight <= bandY0) || (sy >= bandY0 + BANDHEIGHT) || (Part == self->IgnorePart))
				continue;
			BandSprite(Part->X - msx, sy, CWorldPart_SpriteData(Part));
		}

#if LOVYANGFX
		//true: bandBuf holds plain RGB565, the library puts it in display order.
		//LovyanGFX's pushPixels would open a transaction of its own every call
		SCREEN.writePixels((const uint16_t*)bandBuf, bandW * BANDHEIGHT, true);
#else
		SCREEN.pushPixels(bandBuf, bandW * BANDHEIGHT);
#endif
		painted = true;
	}

	if (painted)
		SCREEN.endWrite();

	return painted;
}
#endif

CWorldPart* CWorldParts_PartAtPosition(CWorldParts* self, int8_t PlayFieldXin, int8_t PlayFieldYin)
{
	if ((PlayFieldYin < 0) || (PlayFieldYin >= NrOfRows) || (PlayFieldXin < 0) || (PlayFieldXin >= NrOfCols))
		return NULL;

	for (uint8_t Teller = 0; Teller < NrOfGroups; Teller++)
	{
		uint16_t Index = self->PositionalItems[Teller][PlayFieldYin][PlayFieldXin];
		if ((Index != NoWorldPart) && (&WorldPartPool[Index] != self->IgnorePart))
			return &WorldPartPool[Index];
	}

	return NULL;
}

uint16_t CWorldParts_GroupCount(CWorldParts* self, uint8_t GroupIn)
{
	if (GroupIn == GroupNone)
		return 0;

	return (self->PositionalItemsCount[GroupIn]);
}


uint8_t CWorldParts_TypeAtPosition(CWorldParts* self, int8_t PlayFieldXin, int8_t PlayFieldYin)
{
	if ((PlayFieldYin < 0) || (PlayFieldYin >= NrOfRows) || (PlayFieldXin < 0) || (PlayFieldXin >= NrOfCols))
		return 0;

	for(uint8_t Teller = 0; Teller < NrOfGroups; Teller++)
	{
		uint16_t Index = self->PositionalItems[Teller][PlayFieldYin][PlayFieldXin];
		if ((Index != NoWorldPart) && (&WorldPartPool[Index] != self->IgnorePart))
			return WorldPartPool[Index].Type;
	}

	return 0;
}

void CWorldParts_deinit(CWorldParts* self)
{
	if(!self)
		return;
	CWorldParts_RemoveAll(self);
	CWorldPart_PoolDeinit();
	DrawBuffersDeinit();
	CViewPort_deinit(self->ViewPort);
	free(self);
	self = NULL;
}
