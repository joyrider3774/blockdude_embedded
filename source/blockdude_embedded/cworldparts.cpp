#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include "cworldparts.h"
//the strips hold pictures of the black & white skin too, and those are one bit a pixel
#include "onebitimage.h"
#include "cworldpart.h"
#include "commonvars.h"
#include "levels.h"
#include "gamefuncs.h"

//the buffers of the dirty band rendering further down, they come and go with the board
static void DrawBuffersInit();
static void DrawBuffersDeinit();

//A level is kept as three run length encoded planes rather than as the list of (type, x, y)
//triplets it is played as, see tools/convert_levels.py. Encoding that list as it stands gains
//nothing, since the byte after a type is a coordinate and the byte after that another, so almost
//nothing repeats; keeping the types together, then the x's, then the y's puts like next to like
//and takes 40% off.
//The three planes are read side by side, a stream each, so a level is still read a part at a time
//and none of it is held in ram
#define LEVEL_HEADER 6

typedef struct PlaneReader PlaneReader;
struct PlaneReader
{
	const uint8_t* pos;
	uint8_t left;            //how many bytes the run or the literal still owes
	uint8_t repeated;        //the byte a run repeats
	bool inRun;
};

static void PlaneReaderInit(PlaneReader* plane, const uint8_t* at)
{
	plane->pos = at;
	plane->left = 0;
	plane->repeated = 0;
	plane->inRun = false;
}

static uint8_t PlaneReaderNext(PlaneReader* plane)
{
	if (plane->left == 0)
	{
		//PLATFORM_READ_BYTE is pgm_read_byte on some of the devices, which may look at what it
		//is handed more than once, so the pointer is never stepped on inside it
		uint8_t control = PLATFORM_READ_BYTE(plane->pos);
		plane->pos++;
		if (control & 0x80)
		{
			plane->inRun = true;
			plane->left = (uint8_t)((control & 0x7F) + 1);
			plane->repeated = PLATFORM_READ_BYTE(plane->pos);
			plane->pos++;
		}
		else
		{
			plane->inRun = false;
			plane->left = (uint8_t)(control + 1);
		}
	}
	plane->left--;
	if (plane->inRun)
		return plane->repeated;
	uint8_t value = PLATFORM_READ_BYTE(plane->pos);
	plane->pos++;
	return value;
}

typedef struct LevelReader LevelReader;
struct LevelReader
{
	PlaneReader type, x, y;
	uint16_t left;           //parts still to come
};

//how many parts the level holds, and the three planes set at their first byte
static uint16_t LevelReaderInit(LevelReader* reader, const uint8_t* level)
{
	uint16_t parts = (uint16_t)(PLATFORM_READ_BYTE(level) | (PLATFORM_READ_BYTE(level + 1) << 8));
	uint16_t xAt = (uint16_t)(PLATFORM_READ_BYTE(level + 2) | (PLATFORM_READ_BYTE(level + 3) << 8));
	uint16_t yAt = (uint16_t)(PLATFORM_READ_BYTE(level + 4) | (PLATFORM_READ_BYTE(level + 5) << 8));
	PlaneReaderInit(&reader->type, level + LEVEL_HEADER);
	PlaneReaderInit(&reader->x, level + xAt);
	PlaneReaderInit(&reader->y, level + yAt);
	reader->left = parts;
	return parts;
}

//gives the next part of the level, false once there are none left
static bool LevelReaderNext(LevelReader* reader, uint8_t* type, uint8_t* x, uint8_t* y)
{
	if (reader->left == 0)
		return false;
	reader->left--;
	*type = PlaneReaderNext(&reader->type);
	*x = PlaneReaderNext(&reader->x);
	*y = PlaneReaderNext(&reader->y);
	return true;
}

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
		Result->Exit = NoWorldPart;
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
	self->Exit = NoWorldPart;
	//how many parts each group holds is still counted per group, see CWorldParts_GroupCount
	for (uint8_t i = 0; i < NrOfGroups; i++)
		self->PositionalItemsCount[i] = 0;
	//the grid holds one part a tile whatever group it belongs to, see PositionalItems
	for (uint8_t y = 0; y < NrOfRows; y++)
		for (uint8_t x = 0; x < NrOfCols; x++)
			self->PositionalItems[y][x] = NoWorldPart;
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
					if (self->Exit == CWorldPart_PoolIndex(self->Items[Teller1]))
						self->Exit = NoWorldPart;
					//only when the tile still names the part being removed, see PositionalItems
					else if (self->PositionalItems[self->Items[Teller1]->PlayFieldY][self->Items[Teller1]->PlayFieldX] ==
					         CWorldPart_PoolIndex(self->Items[Teller1]))
						self->PositionalItems[self->Items[Teller1]->PlayFieldY][self->Items[Teller1]->PlayFieldX] = NoWorldPart;
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
					if (self->Exit == CWorldPart_PoolIndex(self->Items[Teller1]))
						self->Exit = NoWorldPart;
					//only when the tile still names the part being removed, see PositionalItems
					else if (self->PositionalItems[self->Items[Teller1]->PlayFieldY][self->Items[Teller1]->PlayFieldX] ==
					         CWorldPart_PoolIndex(self->Items[Teller1]))
						self->PositionalItems[self->Items[Teller1]->PlayFieldY][self->Items[Teller1]->PlayFieldX] = NoWorldPart;
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
			//the exit is held on its own and not in the grid, see Exit
			if (WorldPart->Group == GroupExit)
				self->Exit = CWorldPart_PoolIndex(WorldPart);
			else
				self->PositionalItems[WorldPart->PlayFieldY][WorldPart->PlayFieldX] = CWorldPart_PoolIndex(WorldPart);
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
	//temporary: says what the load was asked for and what it found
	LevelReader reader;
	if (level)
	{
		LevelReaderInit(&reader, level);
		while (LevelReaderNext(&reader, &Type, &X, &Y))
		{
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
			
		//read again from the start, the planes give their parts once each
		LevelReaderInit(&reader, level);
		while (LevelReaderNext(&reader, &Type, &X, &Y))
		{
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
#if PADLEVELROWS
		//the 15 row levels, with 16x16 tiles every level is taller than the screen)
		for (Y = maxy - miny + 1; (Y < NrOfRowsVisible) && (Y < NrOfRows); Y++)
			//X is a byte, the NrOfCols bound also keeps this loop from never ending
			for (X = 0; (X <= maxx - minx) && (X < NrOfCols); X++)
				CWorldParts_Add(self, CWorldPart_create(X, Y, IDEarthMiddle, GroupFloor));
#endif

		self->DisableSorting = false;
		CWorldParts_Sort(self);
		CWorldParts_LimitVPLevel(self);
		CWorldParts_CenterVPOnPlayer(self);
	}

#if CHGAME_TIMING
	//what the board costs and what the load managed to build, see CHGAME_TIMING in Platform.h
	Platform_Log("load: part %d, board %d, pool %d x %d | pack %d level %d of %d, data %s, %d parts, pool %s, %" PRIu32 " free\n",
	             (int)sizeof(CWorldPart), (int)sizeof(CWorldParts), (int)MAXWORLDPARTS,
	             (int)sizeof(CWorldPart),
	             (int)levelPack, (int)Level, (int)LEVELPACKCOUNT, level ? "there" : "NULL",
	             (int)self->ItemCount, WorldPartPool ? "there" : "NULL", Platform_FreeHeap());
#endif
}

#if CHGAME_TIMING
//what the per part logic costs a frame, beside what the drawing costs, see CHGAME_TIMING
static uint32_t dbgMoveUs = 0, dbgMoveCalls = 0;
#endif

bool CWorldParts_Move(CWorldParts* self)
{
#if CHGAME_TIMING
	const uint32_t tMove = Platform_Micros();
	dbgMoveCalls++;
#endif
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
#if CHGAME_TIMING
	dbgMoveUs += Platform_Micros() - tMove;
#endif
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
//Which strips each part reaches, worked out once a frame instead of once a strip. The strip loop
//used to ask every part of the level whether it belonged in the strip it was composing, which for a
//full screen repaint is sixteen passes over the whole list, each one reading a Y out of a part that
//sits wherever the heap put it. One byte a part says it here instead: 0xFF for a part that is not
//drawn at all this frame, otherwise the first cell row it reaches with the top bit set when it
//reaches the row under that as well
static_assert(CELLSY <= 128, "the strip a part starts in has to fit in seven bits");
#define PARTSTRIP_NONE 0xFF
static uint8_t* partStrip = NULL;
#if CHGAME_TIMING
//what a frame actually draws and pushes, see CHGAME_TIMING in Platform.h
static uint32_t dbgStrips = 0, dbgSprites = 0, dbgFrames = 0, dbgParts = 0;
#endif

static uint16_t* bandBuf = NULL; //the strip being composed, WINDOW_WIDTH * BANDHEIGHT pixels
static int16_t bandX0, bandY0, bandW;               //where that strip sits on screen
static int16_t lastMinScreenX = -30000, lastMinScreenY = -30000;

//The background is run length encoded (see pushImageRLE). To start decoding in the
//middle of it, this keeps for every screen row the control byte the row starts in
//and how many pixels of that control belong to the rows above it.
//an encoded background is at most 3 bytes per pixel (a one pixel run every time),
//so the offsets need 32 bits once the screen is bigger than about 147x147
#if WINDOW_WIDTH * WINDOW_HEIGHT * 3 < 65536
//Only for a build that can still be asked for an RGB565 skin, see ONEBITONLY: a one bit only
//build never reads these and they are the width of the screen twice over
#if !ONEBITONLY
typedef uint16_t BgOffset;
#else
typedef uint32_t BgOffset;
#endif
static BgOffset* bgRowOffset = NULL; //WINDOW_HEIGHT rows
static uint8_t* bgRowUsed = NULL;    //WINDOW_HEIGHT rows
static const uint8_t* bgIndexed = NULL;
#endif

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
	//Four bytes at a time where both sides sit on an address that allows it: a 32 bit machine
	//then moves two pixels per load and per store. A 16 bit read needs an even address (a core
	//like the Cortex-M0+ faults on an odd one) and the pixels of an encoded row sit wherever the
	//control bytes leave them, so the other two ways are there for the rows that are not placed
	//as well
	if ((((uintptr_t)src | (uintptr_t)dst) & 3) == 0)
	{
		const uint32_t* s = (const uint32_t*)src;
		uint32_t* d = (uint32_t*)dst;
		const int16_t pairs = count >> 1;
		for (int16_t i = 0; i < pairs; i++)
			d[i] = s[i];
		//an odd last pixel of the run
		if (count & 1)
			dst[count - 1] = ((const uint16_t*)src)[count - 1];
	}
	else if (((uintptr_t)src & 1) == 0)
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
	//two pixels at a time where the addresses allow it: a pair that holds no transparent pixel
	//is one load and one store, which is most of a sprite
	if ((((uintptr_t)src | (uintptr_t)dst) & 3) == 0)
	{
		const uint32_t* s = (const uint32_t*)src;
		uint32_t* d = (uint32_t*)dst;
		const int16_t pairs = count >> 1;
		for (int16_t i = 0; i < pairs; i++)
		{
			const uint32_t two = s[i];
			//magenta is the transparent key, 0xF81F in RGB565
			const uint16_t low = (uint16_t)two, high = (uint16_t)(two >> 16);
			if ((low != 0xF81F) && (high != 0xF81F))
				d[i] = two;
			else
			{
				if (low != 0xF81F)
					dst[i * 2] = low;
				if (high != 0xF81F)
					dst[i * 2 + 1] = high;
			}
		}
		if (count & 1)
		{
			const uint16_t last = ((const uint16_t*)src)[count - 1];
			if (last != 0xF81F)
				dst[count - 1] = last;
		}
		return;
	}
	if (((uintptr_t)src & 1) == 0)
	{
		const uint16_t* s = (const uint16_t*)src;
		for (int16_t i = 0; i < count; i++)
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
	free(partStrip);
	partStrip = NULL;
#if !ONEBITONLY
	free(bgRowOffset);
	bgRowOffset = NULL;
	free(bgRowUsed);
	bgRowUsed = NULL;
	bgIndexed = NULL;
#endif
#endif
}

static void DrawBuffersInit()
{
#if SCREENBUFFER == 0
	if (cellDirty)
		return;
	cellDirty = (CellRow*)calloc(CELLSY, sizeof(CellRow));
	bandBuf = (uint16_t*)malloc(WINDOW_WIDTH * BANDHEIGHT * sizeof(uint16_t));
	//partStrip only saves the strip loop work, so it is not one of the ones below that have to be there
	partStrip = (uint8_t*)malloc(MAXWORLDPARTS);
#if !ONEBITONLY
	bgRowOffset = (BgOffset*)malloc(WINDOW_HEIGHT * sizeof(BgOffset));
	bgRowUsed = (uint8_t*)malloc(WINDOW_HEIGHT * sizeof(uint8_t));
#endif
	if (!cellDirty || !bandBuf
#if !ONEBITONLY
	    || !bgRowOffset || !bgRowUsed
#endif
	   )
	{
		//without these the board is not painted at all, so it says so rather than leaving a
		//blank screen and nothing to go on
		Platform_Log("DrawBuffersInit: out of heap, %" PRIu32 " free\n", Platform_FreeHeap());
		DrawBuffersDeinit();
		return;
	}
	//the background index is gone with the old buffers, and the first draw has to paint everything
#if !ONEBITONLY
	bgIndexed = NULL;
#endif
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
#if !ONEBITONLY
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
#endif

//the background is a full screen image so it lines up with the strip
#if ONEBITIMAGES
//The same, for a background packed one bit a pixel. There is no index to build: every row of such
//the reader gives the strip its rows one after another, having passed over the ones above it
//This stands in for the index the run length encoded background builds, see IndexBackground: a
//plane packed one bit a pixel cannot be entered in the middle, since a row of it may be told to
//repeat the row above. What is kept instead is the reader itself, carried from one strip to the
//next, and the row it stands at. The picture does not scroll, so that row is simply a screen row
//and the strips of a frame are composed from the top down: only the rows between the strip that
//was done last and this one have to be passed over. Starting from the top for every strip meant
//decoding the whole picture once per strip, eight and a half times over for a full screen repaint
static OneBitReader bgPlane;
static const uint8_t* bgPlaneFor = NULL;             //the picture it was started on
static int16_t bgPlaneAt = -1;                       //the screen row it stands at, -1 when unset
//The row it read last, which is also the row it decodes the next one into. A plane packed as rows
//may say the next row is this one again, so the two cannot be separate buffers
static uint8_t bgPlaneRow[(WINDOW_WIDTH + 7) / 8];

static PLATFORM_HOT_CODE void BandBackgroundOneBit()
{
	const int stride = (WINDOW_WIDTH + 7) / 8;
	//A strip above the one that was done last cannot be reached by going on, so the picture is
	//taken from the top again. That is every first strip of a frame, and the picture changing
	if ((bgPlaneAt < 0) || (bgPlaneAt > bandY0) || (bgPlaneFor != IMGBackground))
	{
		OneBitReaderInit(&bgPlane, IMGBackground + ONEBIT_HEADER, OneBitFlags(IMGBackground), false);
		bgPlaneFor = IMGBackground;
		bgPlaneAt = 0;
	}
	OneBitReaderSkip(&bgPlane, bandY0 - bgPlaneAt, stride, bgPlaneRow);
	bgPlaneAt = (int16_t)(bandY0 + BANDHEIGHT);
	for (int16_t r = 0; r < BANDHEIGHT; r++)
	{
		OneBitReaderRow(&bgPlane, bgPlaneRow, stride);
		uint16_t* drow = &bandBuf[r * bandW];
		//this game keeps no record of what the sprites cover, so the whole strip is painted
		for (int16_t x = 0; x < bandW; x++)
			drow[x] = OneBitAt(bgPlaneRow, bandX0 + x) ? ONEBIT_SET : ONEBIT_CLEAR;
	}
}
#endif

static void BandBackground()
{
	if (!IMGBackground)
	{
		for (uint16_t i = 0; i < bandW * BANDHEIGHT; i++)
			bandBuf[i] = ColorWhite;
		return;
	}
	//a new skin brings a new background
#if ONEBITIMAGES
	if (skinImagesOneBit)
	{
		BandBackgroundOneBit();
		return;
	}
#endif
#if !ONEBITONLY
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
#endif
}

//blit a 16x16 sprite that sits at screen position sx,sy, clipped to the strip
#if ONEBITIMAGES
//The same, for a sprite packed one bit a pixel. The frames of a sheet are stacked down it, so the
//rows the strip wants are that many tiles further down, and the rows before them are passed over
//without their pixels being looked at
static PLATFORM_HOT_CODE void BandSpriteOneBit(int16_t sx, int16_t sy, const uint8_t* image, uint8_t frame,
                             bool keyed, int16_t r0, int16_t r1, int16_t c0, int16_t c1)
{
	const int stride = (OneBitWidth(image) + 7) / 8;
	const int flags = OneBitFlags(image);
	const int maskAt = OneBitMaskAt(image);
	const bool useMask = keyed && (maskAt != 0);
	uint8_t rowPixels[ONEBIT_MAX_STRIDE];
	uint8_t rowMask[ONEBIT_MAX_STRIDE];
	const int first = frame * TileHeight + r0;
	OneBitReader plane;
	OneBitReaderInit(&plane, image + ONEBIT_HEADER, flags, false);
	OneBitReaderSkip(&plane, first, stride, rowPixels);
	OneBitReader maskPlane;
	if (useMask)
	{
		OneBitReaderInit(&maskPlane, image + maskAt, flags, true);
		OneBitReaderSkip(&maskPlane, first, stride, rowMask);
	}
	for (int16_t r = r0; r < r1; r++)
	{
		OneBitReaderRow(&plane, rowPixels, stride);
		if (useMask)
			OneBitReaderRow(&maskPlane, rowMask, stride);
		uint16_t* drow = &bandBuf[(sy + r - bandY0) * bandW + (sx + c0 - bandX0)];
		for (int16_t c = c0; c < c1; c++)
		{
			//a clear mask bit is a pixel the sprite does not cover
			if (useMask && !OneBitAt(rowMask, c))
				continue;
			drow[c - c0] = OneBitAt(rowPixels, c) ? ONEBIT_SET : ONEBIT_CLEAR;
		}
	}
}
#endif

static void BandSprite(int16_t sx, int16_t sy, const uint8_t* image, uint8_t frame)
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
#if ONEBITIMAGES
	if (skinImagesOneBit)
	{
		BandSpriteOneBit(sx, sy, image, frame, true, r0, r1, c0, c1);
		return;
	}
#endif
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

#if CHGAME_TIMING
	dbgFrames++;
	dbgParts = self->ItemCount;
	if (dbgFrames >= 60)
	{
		Platform_Log("DrawBoard: %" PRIu32 " frames, %" PRIu32 " parts, %" PRIu32 " sprites, %" PRIu32 " strips, move %" PRIu32 " us a call, strip index %s\n",
		             dbgFrames, dbgParts, dbgSprites, dbgStrips,
		             dbgMoveCalls ? (dbgMoveUs / dbgMoveCalls) : 0,
		             partStrip ? "there" : "none");
		dbgFrames = dbgSprites = dbgStrips = 0;
		dbgMoveUs = dbgMoveCalls = 0;
	}
#endif

	//Which strips each part reaches, see partStrip. This is the work the strip loop below used to
	//do again for every strip: reading a part's Y out of wherever the heap put it
	for (uint16_t Teller = 0; partStrip && (Teller < self->ItemCount); Teller++)
	{
		CWorldPart* Part = self->Items[Teller];
		const int16_t sy = Part->Y - msy;
		if ((sy + TileHeight <= 0) || (sy >= WINDOW_HEIGHT) || (Part == self->IgnorePart))
		{
			partStrip[Teller] = PARTSTRIP_NONE;
			continue;
		}
		//a part hanging off the top of the screen reaches row 0 and no other
		const int16_t r0 = (sy < 0) ? 0 : (int16_t)(sy / TileHeight);
		int16_t r1 = (int16_t)((sy + TileHeight - 1) / TileHeight);
		if (r1 >= CELLSY)
			r1 = CELLSY - 1;
		partStrip[Teller] = (uint8_t)(r0 | ((r1 > r0) ? 0x80 : 0));
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

#if CHGAME_TIMING
		uint32_t tSection = Platform_Micros();
#endif
		BandBackground();
#if CHGAME_TIMING
		bandBgUs += Platform_Micros() - tSection;
		tSection = Platform_Micros();
#endif

		//the parts, in the order the list is sorted in so layering is kept
		for (uint16_t Teller = 0; Teller < self->ItemCount; Teller++)
		{
			CWorldPart* Part = self->Items[Teller];
			if (partStrip)
			{
				//the row this part starts in, and whether it reaches the one after it
				const uint8_t strip = partStrip[Teller];
				const uint8_t first = (uint8_t)(strip & 0x7F);
				if ((strip == PARTSTRIP_NONE) ||
				    ((first != (uint8_t)cy) && (!(strip & 0x80) || (first + 1 != cy))))
					continue;
			}
			else
			{
				const int16_t sy = Part->Y - msy;
				if ((sy + TileHeight <= bandY0) || (sy >= bandY0 + BANDHEIGHT) ||
				    (Part == self->IgnorePart))
					continue;
			}
#if CHGAME_TIMING
			dbgSprites++;
#endif
			BandSprite(Part->X - msx, Part->Y - msy, CWorldPart_SpriteData(Part), CWorldPart_SpriteFrame(Part));
		}
#if CHGAME_TIMING
		//the floor and the parts, which is everything drawn over the background
		bandSpriteUs += Platform_Micros() - tSection;
#endif

#if LOVYANGFX
		//true: bandBuf holds plain RGB565, the library puts it in display order.
		//LovyanGFX's pushPixels would open a transaction of its own every call
		SCREEN.writePixels((const uint16_t*)bandBuf, bandW * BANDHEIGHT, true);
#else
		SCREEN.pushPixels(bandBuf, bandW * BANDHEIGHT);
#endif
		painted = true;
#if CHGAME_TIMING
		dbgStrips++;
#endif
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

	//The exit first, the way the old grid per group reached it first. It is not in the grid, and a
	//part standing on its tile is in the grid, so both are still answered for
	if ((self->Exit != NoWorldPart) &&
	    (WorldPartPool[self->Exit].PlayFieldX == PlayFieldXin) &&
	    (WorldPartPool[self->Exit].PlayFieldY == PlayFieldYin) &&
	    (&WorldPartPool[self->Exit] != self->IgnorePart))
		return &WorldPartPool[self->Exit];
	//one part a tile, see PositionalItems
	const uint16_t Index = self->PositionalItems[PlayFieldYin][PlayFieldXin];
	if ((Index != NoWorldPart) && (&WorldPartPool[Index] != self->IgnorePart))
		return &WorldPartPool[Index];

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

	//the exit first, see CWorldParts_PartAtPosition
	if ((self->Exit != NoWorldPart) &&
	    (WorldPartPool[self->Exit].PlayFieldX == PlayFieldXin) &&
	    (WorldPartPool[self->Exit].PlayFieldY == PlayFieldYin) &&
	    (&WorldPartPool[self->Exit] != self->IgnorePart))
		return WorldPartPool[self->Exit].Type;
	//one part a tile, see PositionalItems
	const uint16_t Index = self->PositionalItems[PlayFieldYin][PlayFieldXin];
	if ((Index != NoWorldPart) && (&WorldPartPool[Index] != self->IgnorePart))
		return WorldPartPool[Index].Type;

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
