#ifndef CWOLRDPART_H
#define CWOLRDPART_H

#include "commonvars.h"
#include "cworldparts.h"

//playfield positions are stored as int8_t (signed, neighbour checks go to -1)
static_assert((NrOfCols <= 127) && (NrOfRows <= 127), "playfield size does not fit in int8_t");
//pixel positions are stored as int16_t
static_assert((NrOfCols * TileWidth <= 32767) && (NrOfRows * TileHeight <= 32767), "pixel positions do not fit in int16_t");
//a speed is negated into the int8_t Xi / Yi, the viewport is moved by the same int8_t steps
static_assert((GameMoveSpeed <= 127) && (ViewportMove <= 127) && (PlayerAnimDelay <= 255), "move speed, viewport step or anim delay does not fit its type");

typedef struct CWorldParts CWorldParts;
typedef struct CWorldPart CWorldPart;
//fields are grouped by size to avoid padding. There is only one parts list, the global
//WorldParts, so no pointer back to it is kept. The move queue is kept there too, only the
//carried box ever has moves queued
struct CWorldPart {
	//The box this part carries and the player carrying it, as slots in the pool rather than
	//pointers: two bytes each instead of four, and dropping both pointers takes the part from
	//28 bytes to 22, which over MAXWORLDPARTS of them is about 2 KB. Read them through
	//CWorldPart_At and write them through CWorldPart_IndexOf, which keep NULL as NoWorldPart
	uint16_t AttachedPartIx;
	uint16_t PlayerIx;
	//pixel positions (max NrOfCols * TileWidth)
	int16_t X, Y;
	//where (in level pixels) and with which anim phase the part was last painted,
	//the board repaints the old and new square whenever these differ from X, Y, AnimPhase
	int16_t PrevDrawX;
	int16_t PrevDrawY;
	//playfield positions (0 .. NrOfCols-1 / NrOfRows-1, -1 = none)
	int8_t PlayFieldX, PlayFieldY;

	//Everything below is a small counter, an id or a speed, and each is given the bits its range
	//needs rather than a byte of its own. The pool holds one of these per part of a level
	//(MAXWORLDPARTS of them, which on a CHGame is the busiest level of the packs in the build),
	//so what one part costs is multiplied by a few hundred: this takes the part from 32 bytes to
	//28. The range of each is in the comment beside it and is checked below; a value that
	//outgrew its field would be cut silently, so a field is widened before it is given a larger
	//one anywhere
	//IDxxx, 0 .. IDRoofDownLeft (26)
	uint8_t Type : 5;
	//Groupxxx, 0 .. GroupNone (10)
	uint8_t Group : 4;
	//0 or GameMoveSpeed (2 or 3, see IMAGESET)
	uint8_t MoveSpeed : 2;
	//always 0, and kept because the move delay is counted against it
	uint8_t MoveDelay : 2;
	//counts up from -(TileWidth / MoveSpeed), so -16 at the slowest
	int8_t MoveDelayCounter : 6;
	//+- MoveSpeed while moving
	int8_t Xi : 3;
	int8_t Yi : 3;
	//AnimBaseLeft (0), AnimBaseRight (4), AnimBaseLeftJump (8) or AnimBaseRightJump (12)
	uint8_t AnimBase : 4;
	//0, 1 or 4
	uint8_t AnimPhases : 3;
	//0 .. AnimPhases, counted up until it equals AnimPhases so it holds that too
	uint8_t AnimCounter : 3;
	//0 or PlayerAnimDelay (4 or 5, see IMAGESET)
	uint8_t AnimDelay : 3;
	//0 .. AnimDelay
	uint8_t AnimDelayCounter : 3;
	//The two phases reach AnimBaseRightJump + AnimPhases - 1, which is 15, so four bits hold one
	uint8_t AnimPhase : 4;
	uint8_t PrevDrawAnimPhase : 4;
	uint8_t FirstArriveEventFired : 1;
	uint8_t IsMoving : 1;
	uint8_t NeedToMoveLeft : 1;
	uint8_t NeedToMoveRight : 1;
};

//What the fields above are sized for. A device header that raises any of these has to widen the
//field with it, which is why they are checked here rather than left to be noticed
static_assert(IDRoofDownLeft <= 31, "Type does not fit its five bits");
static_assert(GroupNone <= 15, "Group does not fit its four bits");
static_assert(GameMoveSpeed <= 3, "MoveSpeed does not fit its two bits, nor Xi / Yi theirs");
static_assert(TileWidth / GameMoveSpeed <= 31, "MoveDelayCounter does not fit its six bits");
static_assert(AnimBaseRightJump <= 15, "AnimBase does not fit its four bits");
static_assert(PlayerAnimDelay <= 7, "AnimDelay does not fit its three bits");
static_assert(AnimBaseRightJump + 4 - 1 <= 15, "AnimPhase does not fit its four bits");
static_assert(sizeof(struct CWorldPart) <= 22, "the part grew, the pool is MAXWORLDPARTS of it");

//every part lives in this pool (MAXWORLDPARTS parts), the positional grid in CWorldParts keeps its index.
//It is NULL until CWorldPart_PoolInit, CWorldParts_Create and CWorldParts_deinit take care of it
extern CWorldPart* WorldPartPool;
//an empty cell of the positional grid
#define NoWorldPart 0xFFFF
static_assert(MAXWORLDPARTS <= NoWorldPart, "pool indexes collide with NoWorldPart");
static inline uint16_t CWorldPart_PoolIndex(const CWorldPart* WorldPart)
{
	return (uint16_t)(WorldPart - WorldPartPool);
}

//the part a slot holds, and the slot a part sits in, with NULL standing in for NoWorldPart
static inline CWorldPart* CWorldPart_At(uint16_t index)
{
	return (index == NoWorldPart) ? NULL : &WorldPartPool[index];
}

static inline uint16_t CWorldPart_IndexOf(const CWorldPart* part)
{
	return part ? CWorldPart_PoolIndex(part) : NoWorldPart;
}

bool CWorldPart_PoolInit();
void CWorldPart_PoolDeinit();
void CWorldPart_free(CWorldPart* WorldPart);
CWorldPart* CWorldPart_create(const int8_t PlayFieldXin, const int8_t PlayFieldYin, const uint8_t Typein, const uint8_t GroupIn);
void CWorldPart_MoveQueClear(CWorldPart* self);
void CWorldPart_MoveQuePopBack(CWorldPart* self);
void CWorldPart_MoveQuePushBack(CWorldPart* self, SPoint point);
void CWorldPart_MoveQueInsert(CWorldPart* self, int8_t pos, SPoint point);
void CWorldPart_AddToMoveQue(CWorldPart* self, int8_t PlayFieldXIn, int8_t PlayFieldYIn);
bool CWorldPart_MovesInQue(CWorldPart* self);
void CWorldPart_AttachToPlayer(CWorldPart* self, CWorldPart* PlayerIn);
void CWorldPart_DeattachFromPlayer(CWorldPart* self, CWorldPart* PlayerIn);
void CWorldPart_SetAnimPhase(CWorldPart* self, uint8_t AnimPhaseIn);
bool CWorldPart_MoveTo(CWorldPart* self, const int8_t PlayFieldXin, const int8_t PlayFieldYin);
void CWorldPart_Event_ArrivedOnNewSpot(CWorldPart* self);
void CWorldPart_Event_BeforeDraw(CWorldPart* self);
void CWorldPart_Draw(CWorldPart* self);
void CWorldPart_Event_Moving(CWorldPart* self, int16_t ScreenPosX, int16_t ScreenPosY);
bool CWorldPart_SetPosition(CWorldPart* self, const int8_t PlayFieldXin, const int8_t PlayFieldYin);
bool CWorldPart_CanMoveTo(CWorldPart* self, const int8_t PlayFieldXin, const int8_t PlayFieldYin);
bool CWorldPart_Move(CWorldPart* self);
//the 16x16 RGB565_LE image of the part's current anim phase, NULL if it has none
//the sheet a tile type is drawn from, which is also what says whether that type is opaque
const uint8_t* CWorldPart_ImageForType(uint8_t type);
const uint8_t* CWorldPart_SpriteData(CWorldPart* self);
//which frame of its sheet the part shows, see CWorldPart_SpriteData
uint8_t CWorldPart_SpriteFrame(CWorldPart* self);

#endif