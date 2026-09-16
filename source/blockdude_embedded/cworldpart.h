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
	CWorldPart* AttachedPart;
	CWorldPart *Player;
	//pixel positions (max NrOfCols * TileWidth)
	int16_t X, Y;
	//where (in level pixels) and with which anim phase the part was last painted,
	//the board repaints the old and new square whenever these differ from X, Y, AnimPhase
	int16_t PrevDrawX;
	int16_t PrevDrawY;
	//playfield positions (0 .. NrOfCols-1 / NrOfRows-1, -1 = none)
	int8_t PlayFieldX, PlayFieldY;
	//speeds / increments are +- GameMoveSpeed, delay counter goes negative (-TileWidth / MoveSpeed)
	int8_t MoveSpeed, MoveDelay, MoveDelayCounter, Xi, Yi;
	//IDxxx (max 26) and Groupxxx (max 10)
	uint8_t Type, Group;
	//anim phases max AnimBaseRightJump + AnimPhases - 1 (15)
	uint8_t AnimPhase;
	uint8_t PrevDrawAnimPhase;
	uint8_t AnimCounter, AnimBase, AnimDelay, AnimDelayCounter, AnimPhases;
	bool FirstArriveEventFired;
	bool IsMoving;
	bool NeedToMoveLeft;
	bool NeedToMoveRight;
};

//every part lives in this pool, the positional grid in CWorldParts keeps its index
extern CWorldPart WorldPartPool[MAXWORLDPARTS];
//an empty cell of the positional grid
#define NoWorldPart 0xFFFF
static_assert(MAXWORLDPARTS <= NoWorldPart, "pool indexes collide with NoWorldPart");
static inline uint16_t CWorldPart_PoolIndex(const CWorldPart* WorldPart)
{
	return (uint16_t)(WorldPart - WorldPartPool);
}

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
const uint8_t* CWorldPart_SpriteData(CWorldPart* self);

#endif