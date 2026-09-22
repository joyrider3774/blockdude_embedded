#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "commonvars.h"
#include "cworldpart.h"
#include "sound.h"
#include "gamefuncs.h"

//Parts come out of one pool instead of a malloc each. That drops the 8 bytes of
//heap overhead every single part would otherwise cost and it can not fragment the
//heap. A free slot is marked by Type 0, no real part ever has that id (1..26).
//The busiest bundled level needs 340 parts; when the pool is full this returns
//NULL and CWorldParts_Add drops the part instead of taking the board down.
//The pool itself is allocated with the board (CWorldParts_Create) and not reserved
//as a global, so nothing that runs instead of the game (the web app store) pays for it
CWorldPart* WorldPartPool = NULL;
//the pool is searched from WorldPartPoolNext onwards, so an index reaches MAXWORLDPARTS * 2
static_assert(MAXWORLDPARTS * 2 <= 65535, "pool indexes do not fit in uint16_t");
static uint16_t WorldPartPoolNext = 0;

bool CWorldPart_PoolInit()
{
	if (WorldPartPool)
		return true;
	WorldPartPool = (CWorldPart*)malloc(MAXWORLDPARTS * sizeof(CWorldPart));
	if (!WorldPartPool)
		return false;
	for (uint16_t Teller = 0; Teller < MAXWORLDPARTS; Teller++)
		WorldPartPool[Teller].Type = 0;
	WorldPartPoolNext = 0;
	return true;
}

void CWorldPart_PoolDeinit()
{
	free(WorldPartPool);
	WorldPartPool = NULL;
	WorldPartPoolNext = 0;
}

static CWorldPart* CWorldPart_PoolAlloc()
{
	//the pool could not be allocated, every part is dropped
	if (!WorldPartPool)
		return NULL;
	for (uint16_t Teller = 0; Teller < MAXWORLDPARTS; Teller++)
	{
		//start looking where the last one was taken, loading a whole level stays
		//linear that way instead of rescanning the pool from the front every time
		uint16_t Index = WorldPartPoolNext + Teller;
		if (Index >= MAXWORLDPARTS)
			Index -= MAXWORLDPARTS;
		if (WorldPartPool[Index].Type == 0)
		{
			WorldPartPoolNext = Index + 1;
			if (WorldPartPoolNext >= MAXWORLDPARTS)
				WorldPartPoolNext = 0;
			return &WorldPartPool[Index];
		}
	}
	return NULL;
}

//hands the slot back to the pool, Type 0 marks it free again
void CWorldPart_free(CWorldPart* WorldPart)
{
	if (WorldPart)
	{
		//its pool slot can be handed to another part, which must not inherit the queue
		if (WorldParts && (WorldParts->MoveQueOwner == CWorldPart_PoolIndex(WorldPart)))
		{
			WorldParts->MoveQueBack = -1;
			WorldParts->MoveQueFront = -1;
			WorldParts->MoveQueOwner = NoWorldPart;
		}
		WorldPart->Type = 0;
	}
}

CWorldPart* CWorldPart_create(const int8_t PlayFieldXin, const int8_t PlayFieldYin, const uint8_t Typein, const uint8_t GroupIn)
{
	CWorldPart* Result = CWorldPart_PoolAlloc();
	if (Result)
	{
		Result->Player = NULL;
		Result->AttachedPart = NULL;
		Result->NeedToMoveLeft = false;
		Result->NeedToMoveRight = false;
		Result->AnimCounter = 0;
		Result->AnimBase = 0;
		Result->AnimDelay = 0;
		Result->AnimDelayCounter = 0;
		Result->AnimPhases = 0;
		Result->PlayFieldX = PlayFieldXin;
		Result->PlayFieldY = PlayFieldYin;
		Result->Xi = 0;
		Result->Yi = 0;
		Result->X = PlayFieldXin * TileWidth;
		Result->Y = PlayFieldYin * TileHeight;
		Result->Type = Typein;
		Result->MoveDelay = 0;
		Result->MoveDelayCounter = 0;
		Result->IsMoving = false;
		Result->MoveSpeed = 0;
		Result->AnimPhase = 0;
		Result->PrevDrawAnimPhase = 0;
		Result->FirstArriveEventFired = false;
		Result->Group = GroupIn;
		//never painted, so the board sees it as changed and paints it (the old square is off screen)
		Result->PrevDrawX = -TileWidth;
		Result->PrevDrawY = -TileHeight;


		if (Typein == IDPlayer)
		{
			Result->AnimBase = AnimBaseLeft;
			Result->AnimPhases = 4;
			Result->AnimCounter = 1;
			Result->AnimDelay = PlayerAnimDelay;
			Result->MoveSpeed = GameMoveSpeed;
			Result->AnimDelayCounter = 0;
		}

		if (Typein == IDBox)
		{
			Result->MoveSpeed = GameMoveSpeed;
		}
	}
	return Result;
}



//the board has one move queue (see CWorldParts), these only act on it for the part that owns it
static inline bool CWorldPart_OwnsMoveQue(CWorldPart* self)
{
	return WorldParts->MoveQueOwner == CWorldPart_PoolIndex(self);
}

void CWorldPart_MoveQueClear(CWorldPart* self)
{
	if (!CWorldPart_OwnsMoveQue(self))
		return;
	WorldParts->MoveQueBack = -1;
	WorldParts->MoveQueFront = -1;
	WorldParts->MoveQueOwner = NoWorldPart;
}

void CWorldPart_MoveQuePopBack(CWorldPart* self)
{
	if (!CWorldPart_OwnsMoveQue(self))
		return;
	if (WorldParts->MoveQueBack > -1)
		WorldParts->MoveQueBack--;

	if (WorldParts->MoveQueBack == -1)
	{
		WorldParts->MoveQueFront = -1;
		WorldParts->MoveQueOwner = NoWorldPart;
	}
}

void CWorldPart_MoveQuePushBack(CWorldPart* self, SPoint point)
{
	//queued moves of another part are still waiting, they are never mixed
	if ((WorldParts->MoveQueBack > -1) && !CWorldPart_OwnsMoveQue(self))
		return;
	if (WorldParts->MoveQueBack < MOVEQUESIZE - 1)
	{
		WorldParts->MoveQueOwner = CWorldPart_PoolIndex(self);
		if (WorldParts->MoveQueFront == -1)
			WorldParts->MoveQueFront = 0;
		WorldParts->MoveQueBack++;
		WorldParts->MoveQue[WorldParts->MoveQueBack] = point;
	}
}

void CWorldPart_MoveQueInsert(CWorldPart* self, int8_t pos, SPoint point)
{
	if ((WorldParts->MoveQueBack > -1) && !CWorldPart_OwnsMoveQue(self))
		return;
	if ((WorldParts->MoveQueBack < MOVEQUESIZE - 1) && (pos <= WorldParts->MoveQueBack))
	{
		WorldParts->MoveQueOwner = CWorldPart_PoolIndex(self);
		if (WorldParts->MoveQueFront == -1)
			WorldParts->MoveQueFront = 0;
		WorldParts->MoveQueBack++;

		for (int8_t i = WorldParts->MoveQueBack; i > pos; i--)
		{
			WorldParts->MoveQue[i] = WorldParts->MoveQue[i - 1];
		}

		WorldParts->MoveQue[pos] = point;
	}
}

void CWorldPart_AddToMoveQue(CWorldPart* self, int8_t PlayFieldXIn, int8_t PlayFieldYIn)
{
	SPoint Temp = { .X = (int8_t)PlayFieldXIn, .Y = (int8_t)PlayFieldYIn };

	if (!CWorldPart_MovesInQue(self))
		CWorldPart_MoveQuePushBack(self, Temp);
	else
		CWorldPart_MoveQueInsert(self, WorldParts->MoveQueFront, Temp);
}


bool CWorldPart_MovesInQue(CWorldPart* self)
{
	return CWorldPart_OwnsMoveQue(self) && (WorldParts->MoveQueBack > -1);
}

void CWorldPart_AttachToPlayer(CWorldPart* self, CWorldPart* PlayerIn)
{
	PlayerIn->AttachedPart = self;
	self->Player = PlayerIn;
}

void CWorldPart_DeattachFromPlayer(CWorldPart* self, CWorldPart* PlayerIn)
{
	PlayerIn->AttachedPart = NULL;
	self->Player = NULL;
}

void CWorldPart_SetAnimPhase(CWorldPart* self, uint8_t AnimPhaseIn)
{
	self->AnimPhase = AnimPhaseIn;
}


bool CWorldPart_MoveTo(CWorldPart* self, const int8_t PlayFieldXin, const int8_t PlayFieldYin)
{
	bool Result = false;
	if (!self->IsMoving && ((PlayFieldXin != self->PlayFieldX) || (PlayFieldYin != self->PlayFieldY)))
	{
		if (self->Type == IDPlayer)
		{
			if (CWorldPart_CanMoveTo(self, PlayFieldXin, PlayFieldYin))
			{
				Result = true;
				if (WorldParts && (self != WorldParts->IgnorePart) && (self->Group != GroupNone))
				{
					if ((self->PlayFieldX >= 0) && (self->PlayFieldY >= 0))
					{
						WorldParts->PositionalItems[self->Group][self->PlayFieldY][self->PlayFieldX] = NoWorldPart;
					}
					WorldParts->PositionalItems[self->Group][PlayFieldYin][PlayFieldXin] = CWorldPart_PoolIndex(self);
				}

				self->PlayFieldX = PlayFieldXin;
				self->PlayFieldY = PlayFieldYin;
				self->IsMoving = true;
				//Right
				if (self->X < self->PlayFieldX * TileWidth)
				{
					//this comes from a jump and falling down
					if (!self->NeedToMoveRight)
					{
						CWorldPart* Tmp = CWorldParts_PartAtPosition(WorldParts, PlayFieldXin, PlayFieldYin + 1);
						if (Tmp != NULL)
						{
							if ((Tmp->Group == GroupFloor) || (Tmp->Group == GroupBox))
								playWalkSound();
						}
					}

					self->Xi = self->MoveSpeed;
					self->AnimBase = AnimBaseRight;

					if (WorldParts)
					{
						//check for a box on top of the player.
						CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX - 1, self->PlayFieldY - 1);
						if(Part)
						{
							if (Part->Group == GroupBox)
							{
								//if one is found move it to the players X position
								CWorldPart_MoveTo(Part, self->PlayFieldX, self->PlayFieldY - 1);
								//AnimBase = 6;
							}
						}
					}
				}
				//left
				if (self->X > self->PlayFieldX * TileWidth)
				{
					if (!self->NeedToMoveLeft)
					{
						CWorldPart* Tmp = CWorldParts_PartAtPosition(WorldParts, PlayFieldXin, PlayFieldYin + 1);
						if (Tmp)
						{
							if ((Tmp->Group == GroupFloor) || (Tmp->Group == GroupBox))
								playWalkSound();
						}
					}

					self->Xi = -self->MoveSpeed;
					self->AnimBase = AnimBaseLeft;

					if (WorldParts)
					{
						//check for a box on top of the player.
						CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX + 1, self->PlayFieldY - 1);
						if (Part)
						{
							if (Part->Group == GroupBox)
							{
								//if one is found move it to the players X position
								CWorldPart_MoveTo(Part, self->PlayFieldX, self->PlayFieldY - 1);
								//AnimBase = 6;
							}
						}
					}
				}

				//up
				if (self->Y > self->PlayFieldY * TileHeight)
				{
					playJumpSound();
					self->Yi = -self->MoveSpeed;
					//animbase = 0 is start of animation for going left, 2 is start for going right
					if (self->AnimBase == AnimBaseLeft)
					{
						self->NeedToMoveLeft = true;
						self->AnimBase = AnimBaseLeftJump;
					}
					else
						if (self->AnimBase == AnimBaseRight)
						{
							self->AnimBase = AnimBaseRightJump;
							self->NeedToMoveRight = true;
						}

					if (WorldParts)
					{
						//check for a box on top of the player.
						CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX, self->PlayFieldY);
						if (Part)
						{
							if (Part->Group == GroupBox)
							{
								//if one is found move it to the players X position
								CWorldPart_MoveTo(Part, self->PlayFieldX, self->PlayFieldY - 1);
								//AnimBase = 6;
							}
						}
					}
				}
				//down
				if (self->Y < self->PlayFieldY * TileHeight)
				{
					playFallSound();
					self->Yi = self->MoveSpeed;
					if (WorldParts)
					{

						//check for a box on top of the player.
						CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX, self->PlayFieldY - 2);
						if (Part)
						{
							if (Part->Group == GroupBox)
							{
								//if one is found move it to the players X position
								CWorldPart_MoveTo(Part, self->PlayFieldX, self->PlayFieldY - 1);
								//AnimBase = 6;
							}
						}
					}
				}
			}
			else
			{
				//Right
				if ((self->AnimBase != AnimBaseRight) && (PlayFieldXin > self->PlayFieldX))
				{
					self->MoveDelayCounter = -TileWidth / self->MoveSpeed;
					self->IsMoving = true;
					Result = true;
					self->AnimBase = AnimBaseRight;
					self->AnimCounter = 0;
				}
				//Left
				if ((self->AnimBase != AnimBaseLeft) && (PlayFieldXin < self->PlayFieldX))
				{
					self->MoveDelayCounter = -TileWidth / self->MoveSpeed;
					self->IsMoving = true;
					Result = true;
					self->AnimBase = AnimBaseLeft;
					self->AnimCounter = 0;
				}
				if (self->AnimPhase != self->AnimBase)
				{
					Result = true;
					self->AnimPhase = self->AnimBase;
				}
			}

		}
		else
		{
			if (self->Group == GroupBox)
				if (CWorldPart_CanMoveTo(self, PlayFieldXin, PlayFieldYin))
				{
					Result = true;
					if (WorldParts && (self != WorldParts->IgnorePart) && (self->Group != GroupNone))
					{
						if ((self->PlayFieldX >= 0) && (self->PlayFieldY >= 0))
						{
							WorldParts->PositionalItems[self->Group][self->PlayFieldY][self->PlayFieldX] = NoWorldPart;
						}
						WorldParts->PositionalItems[self->Group][PlayFieldYin][PlayFieldXin] = CWorldPart_PoolIndex(self);
					}
					self->PlayFieldX = PlayFieldXin;
					self->PlayFieldY = PlayFieldYin;
					if (self->X < self->PlayFieldX * TileWidth)
						self->Xi = self->MoveSpeed;
					if (self->X > self->PlayFieldX * TileWidth)
						self->Xi = -self->MoveSpeed;
					if (self->Y > self->PlayFieldY * TileHeight)
						self->Yi = -self->MoveSpeed;
					if (self->Y < self->PlayFieldY * TileHeight)
						self->Yi = self->MoveSpeed;
					self->IsMoving = true;
				}
		}
	}
	return Result;
}

void CWorldPart_Event_ArrivedOnNewSpot(CWorldPart* self)
{
	switch (self->Type)
	{
		case IDPlayer:
		{
			//check for floor so we can drop down if not found
			bool FloorFound = false;

			//in case the block was attached to a player but blocked by wall the player moved under
			if (self->AttachedPart) 
			{
				if(self->AttachedPart->Group == GroupBox)
				{
					CWorldPart_Event_ArrivedOnNewSpot(self->AttachedPart);
				}
			}

			if (self->PlayFieldY == NrOfRows - 1)
			{
				self->AnimPhases = 4;
				FloorFound = true;
			}

			if (!FloorFound)
			{
				CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX, self->PlayFieldY + 1);
				if (Part)
				{
					if ((Part->Group == GroupFloor) || (Part->Group == GroupBox))
					{
						self->AnimPhases = 4;
						FloorFound = true;
					}
				}
			}

			if (!FloorFound)
			{
				CWorldPart* Part2 = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX, self->PlayFieldY);
				if (Part2)
				{
					//to catch floating exits
					if (Part2->Group == GroupExit)
					{
						self->AnimPhases = 4;
						FloorFound = true;
					}
				}
			}

			if (self->NeedToMoveLeft)
			{
				CWorldPart_MoveTo(self, self->PlayFieldX - 1, self->PlayFieldY);
				self->NeedToMoveLeft = false;
			}

			if (self->NeedToMoveRight)
			{
				CWorldPart_MoveTo(self, self->PlayFieldX + 1, self->PlayFieldY);
				self->NeedToMoveRight = false;
			}

			if (!FloorFound && WorldParts)
			{
				self->AnimCounter = 0;
				self->AnimPhases = 1;
				CWorldPart_MoveTo(self, self->PlayFieldX, self->PlayFieldY + 1);
			}

			break;
		}
		case IDBox:
		{
			bool PlayerBelow = false;
			bool SomethingBelow = (self->PlayFieldY == NrOfRows - 1);

			CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX, self->PlayFieldY + 1);
			if (Part)
			{
				if (Part->Type == IDPlayer)
				{
					SomethingBelow = true;
					PlayerBelow = true;
				}

				if ((Part->Group == GroupFloor) || (Part->Group == GroupBox) || (Part->Group == GroupPlayer) || (Part->Group == GroupExit))
				{
					SomethingBelow = true;
				}
			}
			
			if (PlayerBelow)
				CWorldPart_AttachToPlayer(self, WorldParts->Player);
			else
				if ((self->Player) && !CWorldPart_MovesInQue(self))
					CWorldPart_DeattachFromPlayer(self, self->Player);

			//if nothing is below and there are no moves in the queue move it down
			if ((!SomethingBelow) && (!CWorldPart_MovesInQue(self)))
			{
				if (self->Player)
				{
					/*if (!self->Player->IsMoving)
					{
						self->AttachedToPlayer = false;
						self->Player = NULL;
					}*/
					//CWorldPart_Event_ArrivedOnNewSpot(self);
				}
				/*if ((!CWorldPart_CanMoveTo(self, self->PlayFieldX, self->PlayFieldY + 2)) && (self->PlayFieldY < NrOfRows - 1))
				{
					playDropSound();
				}*/
				CWorldPart_MoveTo(self, self->PlayFieldX, self->PlayFieldY + 1);
			}

			
			break;
		}
	}
}

void CWorldPart_Event_BeforeDraw(CWorldPart* self)
{
	if (self->IsMoving)
	{
		self->AnimPhase = self->AnimBase + self->AnimCounter;
		//parts without animation (AnimDelay 0) would otherwise wrap the uint8_t counter back to 0 and advance AnimCounter
		if (self->AnimDelay > 0)
		{
			self->AnimDelayCounter++;
			if (self->AnimDelayCounter == self->AnimDelay)
			{
				self->AnimDelayCounter = 0;
				self->AnimCounter++;
				if (self->AnimCounter == self->AnimPhases)
					self->AnimCounter = 0;
			}
		}
	}
	else
	{
		//i commented it normally we should go to animbase
		//	but the walking animation shows weird if do this
		// because there is a single frame where ismoving is false
		// when the player arrived on a new spot and this made the animation look weird
		// side effect is that walking animation remain visible but thats not a big issue
		// people could think it's as designed
		//if (self->AnimPhase != self->AnimBase)
		//{
		//	//self->AnimPhase = self->AnimBase;
		//	self->AnimDelayCounter = 0;
		//}
	}
}

void CWorldPart_Event_Moving(CWorldPart* self, int16_t ScreenPosX, int16_t ScreenPosY)
{
	//the board notices the viewport scrolled and repaints everything by itself
	if (self->Type == IDPlayer)
	{
		if (WorldParts)
		{
			if ((ScreenPosX > (WorldParts->ViewPort->MaxScreenX) - HALFWINDOWWIDTH) && (self->Xi > 0))
				CViewPort_Move(WorldParts->ViewPort, self->Xi, self->Yi);
			if ((ScreenPosX < (WorldParts->ViewPort->MaxScreenX) - HALFWINDOWWIDTH) && (self->Xi < 0))
				CViewPort_Move(WorldParts->ViewPort, self->Xi, self->Yi);
			if ((ScreenPosY > (WorldParts->ViewPort->MaxScreenY) - HALFWINDOWHEIGHT) && (self->Yi > 0))
				CViewPort_Move(WorldParts->ViewPort, self->Xi, self->Yi);
			if ((ScreenPosY < (WorldParts->ViewPort->MaxScreenY) - HALFWINDOWHEIGHT) && (self->Yi < 0))
				CViewPort_Move(WorldParts->ViewPort, self->Xi, self->Yi);
		}
	}
}

bool CWorldPart_SetPosition(CWorldPart* self, const int8_t PlayFieldXin, const int8_t PlayFieldYin)
{
	bool Result = false;
	if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
	{
		if ((self->PlayFieldX != PlayFieldXin) || (self->PlayFieldY != PlayFieldYin))
		{
			if (WorldParts && (self != WorldParts->IgnorePart) && (self->Group != GroupNone))
			{
				if ((self->PlayFieldX >= 0) && (self->PlayFieldY >= 0))
				{
					WorldParts->PositionalItems[self->Group][self->PlayFieldY][self->PlayFieldX] = NoWorldPart;
				}
				WorldParts->PositionalItems[self->Group][PlayFieldYin][PlayFieldXin] = CWorldPart_PoolIndex(self);
			}
			self->PlayFieldX = PlayFieldXin;
			self->PlayFieldY = PlayFieldYin;
			self->X = PlayFieldXin * TileWidth;
			self->Y = PlayFieldYin * TileHeight;
			//Event_ArrivedOnNewSpot();
			//no need to mark it dirty, the board sees X / Y changed
			Result = true;
		}
	}
	return Result;
}

bool CWorldPart_CanMoveTo(CWorldPart* self, const int8_t PlayFieldXin, const int8_t PlayFieldYin)
{
	switch (self->Type)
	{
		case IDPlayer:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					//if we move up
					if (PlayFieldYin - self->PlayFieldY < 0)
					{
						//we're facing left
						if (self->AnimBase == AnimBaseLeft)
						{
							//part left from player
							CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX - 1, self->PlayFieldY);
							if (Part)
							{
								//its an exit (can't jump onto it)
								if (Part->Group == GroupExit)
									return false;

								//it's a box
								if (Part->Group == GroupBox)
								{
									//check part below it
									CWorldPart* Part2 = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX - 1, self->PlayFieldY + 1);
									if (Part2)
									{
										//it's not a floor kind so can't jump onto it
										if (!((Part2->Group == GroupFloor) || (Part2->Group == GroupBox) || (Part2->Group == GroupExit)))
										{
											return false;
										}
									}
									else
										return false;

								}

								//check part on final position
								CWorldPart* Part3 = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX - 1, PlayFieldYin);
								if (Part3)
								{
									//it's something else than exit
									if (Part3->Group != GroupExit)
									{
										return false;
									}
								}
							}
							else
								//nothing on the left
								return false;
						}
						else
						{
							//we're facing right
							if (self->AnimBase == AnimBaseRight)
							{
								//check part on right of player
								CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX + 1, self->PlayFieldY);
								if (Part)
								{
									//its an exit can't jump onto that
									if (Part->Group == GroupExit)
										return false;

									//it's a box
									if (Part->Group == GroupBox)
									{
										//check part below it
										CWorldPart* Part2 = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX + 1, self->PlayFieldY + 1);
										if (Part2)
										{
											//it's not a floor kind so can't jump onto it
											if (!((Part2->Group == GroupFloor) || (Part2->Group == GroupBox) || (Part2->Group == GroupExit)))
											{
												return false;
											}

										}
										else
											return false;
									}

									//check part on final position
									CWorldPart* Part3 = CWorldParts_PartAtPosition(WorldParts, self->PlayFieldX + 1, PlayFieldYin);
									if (Part3)
									{
										//anything but exit and we can't jump onto it
										if (Part3->Group != GroupExit)
										{
											return false;
										}
									}
								}
								else
									//nothing on the right
									return false;
							}
						}
					}

					CWorldPart* Part4 = CWorldParts_PartAtPosition(WorldParts, PlayFieldXin, PlayFieldYin);
					if (Part4)
					{
						if (Part4->Group == GroupFloor)
						{
							return false;
						}
						else
						{
							if (Part4->Group == GroupBox)
							{
								//we're moving horizontal or falling down so we can't move into the box
								if (PlayFieldYin - self->PlayFieldY >= 0)
								{
									return false;
								}
								//were moving up
								//check to see if there are no stackboxes or other things on top of the player blocking the jump
								else
								{
									if ((self->AnimBase == AnimBaseLeft) || (self->AnimBase == AnimBaseRight))
									{
										CWorldPart* Part5 = CWorldParts_PartAtPosition(WorldParts, PlayFieldXin, PlayFieldYin - 1);
										if (Part5)
										{
											if ((Part5->Group == GroupFloor) || (Part5->Group == GroupBox) || (Part5->Group == GroupExit))
											{
												return false;
											}
										}
									}
								}
							}
						}
					}

					//check to see if our attached box has a queue if so we can't move either
					if (self->AttachedPart)
					{
						if (self->AttachedPart->Group == GroupBox)
						{
							if (CWorldPart_MovesInQue(self->AttachedPart))
							{
								return false;
							}
						}
					}
				}
				else
					return false;
			}
			else
				return false;

			//if we want to move up
			if (PlayFieldYin - self->PlayFieldY < 0)
				//only allow jump if we are not on top position
				return self->PlayFieldY - 1 > 0;
			return true;
			break;
		}
		case IDBox:
		{
			if ((PlayFieldXin >= 0) && (PlayFieldXin < NrOfCols) && (PlayFieldYin >= 0) && (PlayFieldYin < NrOfRows))
			{
				if (WorldParts)
				{
					CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, PlayFieldXin, PlayFieldYin);
					if (Part)
					{
						if ((Part->Group == GroupFloor) || (Part->Group == GroupBox) || (Part->Group == GroupExit))
						{
							return false;
						}
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
			return true;
			break;
		}
		default:
			return false;
			break;
	}
}

bool CWorldPart_Move(CWorldPart* self)
{
	bool Result = false;
	//floors & exits don't move
	if ((self->Group == GroupFloor) || (self->Group == GroupExit))
	{
		if (!self->FirstArriveEventFired)
		{
			self->FirstArriveEventFired = true;
			Result = true;
		}
	}
	else
	{
		if (!self->FirstArriveEventFired)
		{
			CWorldPart_Event_ArrivedOnNewSpot(self);
			self->FirstArriveEventFired = true;
			Result = true;
		}

		if (self->IsMoving)
		{
			//>= and not ==: the counter only ever counts up to MoveDelay, so the two are the same
			//for every value it legitimately takes. Should it ever start out past MoveDelay, == would
			//wait for it to wrap the whole int8_t range, 256 frames of standing still with the walk
			//animation running, where >= simply steps on the next frame
			if (self->MoveDelayCounter >= self->MoveDelay)
			{
				Result = true;
				self->X += self->Xi;
				self->Y += self->Yi;
				CWorldPart_Event_Moving(self, self->X, self->Y);
				if ((abs(self->X - (self->PlayFieldX * TileWidth)) < GameMoveSpeed) && (abs(self->Y - (self->PlayFieldY * TileHeight)) < GameMoveSpeed))
				{
					self->IsMoving = false;
					self->Xi = 0;
					self->Yi = 0;
					self->X = self->PlayFieldX * TileWidth;
					self->Y = self->PlayFieldY * TileHeight;
					CWorldPart_Event_ArrivedOnNewSpot(self);
				}

				self->MoveDelayCounter = -1;
			}
			self->MoveDelayCounter++;
		}
		else
		{
			if (CWorldPart_MovesInQue(self))
			{
				SPoint Next = WorldParts->MoveQue[WorldParts->MoveQueBack];
				if (CWorldPart_CanMoveTo(self, Next.X, Next.Y))
				{
					CWorldPart_MoveTo(self, Next.X, Next.Y);
					CWorldPart_MoveQuePopBack(self);
					Result = true;
				}
				else
				{
					if (self->Player)
					{
						CWorldPart_DeattachFromPlayer(self, self->Player);
					}
					CWorldPart_MoveQueClear(self);
				}
			}
		}
	}
	return Result;
}

const uint8_t* CWorldPart_SpriteData(CWorldPart* self)
{
	const uint8_t * Img = NULL;

	switch (self->Type)
	{
	case IDEmpty:
		Img = IMGEmpty;
		break;
	case IDBox:
		Img = IMGBox;
		break;
	case IDPlayer:
		Img = IMGPlayer;
		break;
	case IDFloor:
		Img = IMGFloor;
		break;
	case IDExit:
		Img = IMGExit;
		break;
	case IDEarthGrassLeft:
		Img = IMGEarthGrassLeft;
		break;
	case IDEarthGrassRight:
		Img = IMGEarthGrassRight;
		break;
	case IDEarthLeft:
		Img = IMGEarthLeft;
		break;
	case IDEarthMiddle:
		Img = IMGEarthMiddle;
		break;
	case IDEarthRight:
		Img = IMGEarthRight;
		break;
	case IDFloatingFloor:
		Img = IMGFloatingFloor;
		break;
	case IDFloatingFloorLeft:
		Img = IMGFloatingFloorLeft;
		break;
	case IDFloatingFloorMiddle:
		Img = IMGFloatingFloorMiddle;
		break;
	case IDFloatingFloorRight:
		Img = IMGFloatingFloorRight;
		break;
	case IDFloorLeft:
		Img = IMGFloorLeft;
		break;
	case IDFloorRight:
		Img = IMGFloorRight;
		break;
	case IDTower:
		Img = IMGTower;
		break;
	case IDStartTower:
		Img = IMGStartTower;
		break;
	case IDTowerShaft:
		Img = IMGTowerShaft;
		break;
	case IDRoof1:
		Img = IMGRoof1;
		break;
	case IDRoof2:
		Img = IMGRoof2;
		break;
	case IDRoofCornerLeft:
		Img = IMGRoofCornerLeft;
		break;
	case IDRoofCornerRight:
		Img = IMGRoofCornerRight;
		break;
	case IDRoofCornerBoth:
		Img = IMGRoofCornerBoth;
		break;
	case IDRoofDownRight:
		Img = IMGRoofDownRight;
		break;
	case IDRoofDownLeft:
		Img = IMGRoofDownLeft;
		break;
	}

	if (!Img)
		return NULL;
	return Img + self->AnimPhase * TileWidth * TileHeight * sizeof(uint16_t);
}

void CWorldPart_Draw(CWorldPart* self)
{
	CWorldPart_Event_BeforeDraw(self);
	DrawImageTransparent(self->X - WorldParts->ViewPort->MinScreenX, self->Y - WorldParts->ViewPort->MinScreenY,
	                     TileWidth, TileHeight, CWorldPart_SpriteData(self));
}
