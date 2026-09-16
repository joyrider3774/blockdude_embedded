#ifndef CWORLDPARTS_H
#define CWORLDPARTS_H

#include <stdint.h>
#include "commonvars.h"
#include "cviewport.h"


typedef struct CWorldPart CWorldPart;
typedef struct CWorldParts CWorldParts;

//ItemCount goes up to MAXWORLDPARTS (the size of Items), the positional grid is NrOfRows * NrOfCols big
static_assert(MAXWORLDPARTS <= 65535, "ItemCount does not fit in uint16_t");
static_assert(NrOfRows * NrOfCols <= 65535, "tile indexes do not fit in uint16_t");

//a box being picked up or dropped queues 2 moves (up then over, or over then down)
#define MOVEQUESIZE 4
static_assert(MOVEQUESIZE <= 127, "MOVEQUESIZE does not fit in int8_t");

typedef struct SPoint SPoint;
struct SPoint {
    int8_t X, Y;
};

struct CWorldParts {
	CViewPort* ViewPort;
	//every part comes out of the pool, so there are never more than MAXWORLDPARTS
	CWorldPart* Items[MAXWORLDPARTS];
	//pool index (WorldPartPool) of the part of a group on a tile, NoWorldPart when there is none.
	//Half the size of a pointer grid and still a direct lookup
	uint16_t PositionalItems[NrOfGroups][NrOfRows][NrOfCols];
	CWorldPart* Player;
	CWorldPart* IgnorePart;
	//Only boxes queue moves, when picked up or dropped, and only the one carried box at a
	//time, so there is one queue for the whole board instead of one in every part
	SPoint MoveQue[MOVEQUESIZE];
	uint16_t MoveQueOwner;   //pool index of the box the queue belongs to, NoWorldPart when empty
	int8_t MoveQueBack;      //-1 = empty queue, max MOVEQUESIZE-1
	int8_t MoveQueFront;
	uint16_t PositionalItemsCount[NrOfGroups];
	uint16_t ItemCount;
	uint16_t NumPartsMoving;
	uint16_t NumPartsMovingQueued;
	uint16_t NumPartsAttachedToPlayer;
	uint16_t NumBoxesAttachedToPlayer;
	bool DisableSorting;
	bool AttchedBoxQuedOrMoving;
};

CWorldParts* CWorldParts_Create();
void CWorldParts_ClearPositionalItems(CWorldParts* self);
void CWorldParts_CenterVPOnPlayer(CWorldParts* self);
void CWorldParts_LimitVPLevel(CWorldParts* self);
void CWorldParts_RemoveAll(CWorldParts* self);
void CWorldParts_Remove(CWorldParts* self, int8_t PlayFieldXin, int8_t PlayFieldYin);
void CWorldParts_RemoveType(CWorldParts* self, uint8_t Type);
void CWorldParts_Sort(CWorldParts* self);
void CWorldParts_Add(CWorldParts* self, CWorldPart* WorldPart);
void CWorldParts_Load(CWorldParts* self, uint8_t levelPack, uint8_t Level);
bool CWorldParts_SavePositional(CWorldParts* self, char* Filename);
bool CWorldParts_Save(CWorldParts* self, char* Filename);
bool CWorldParts_Move(CWorldParts* self);
//repaints only the screen cells whose contents changed, returns true if anything was painted
bool CWorldParts_DrawBoard(CWorldParts* self);
//draws every part in view, used for the full board draw with a screen buffer
void CWorldParts_Draw(CWorldParts* self);
//screen rectangle (in pixels) that has to be repainted on the next CWorldParts_DrawBoard
void CWorldParts_MarkDirty(int16_t x, int16_t y, int16_t w, int16_t h);
//something was drawn over the board (or it jumped), repaint all of it
void CWorldParts_MarkAllDirty();
void CWorldParts_FindPlayer(CWorldParts* self);
void CWorldParts_deinit(CWorldParts* self);
uint16_t CWorldParts_GroupCount(CWorldParts* self, uint8_t GroupIn);
CWorldPart* CWorldParts_PartAtPosition(CWorldParts* self, int8_t PlayFieldXin, int8_t PlayFieldYin);
uint8_t CWorldParts_TypeAtPosition(CWorldParts* self, int8_t PlayFieldXin, int8_t PlayFieldYin);

#endif