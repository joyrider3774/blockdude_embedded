#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gamestate.h"
#include "savestate.h"
#include "commonvars.h"
#include "cworldparts.h"
#include "cworldpart.h"
#include "sound.h"
#include "gamefuncs.h"


bool StageDone(CWorldPart* Player)
{
	//this works because player is on a higher group (it is handled & found last so the exit is found first if we are at that position)
	CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, Player->PlayFieldX, Player->PlayFieldY);
	if (Part)
	{
		if (Part->Type == IDExit)
		{
			return true;
		}
	}
	return false;
}


void GameInit(void)
{
	FreeView = false;
	NeedRedraw = true;
	CWorldParts_FindPlayer(WorldParts);
	CWorldParts_LimitVPLevel(WorldParts);
	//whatever the previous state left on screen has to go
	CWorldParts_MarkAllDirty();
}

void Game(void)
{
	if (GameState == GSGameInit)
	{
		GameInit();
		GameState -= GSDiff;
	}

	if (!FreeView && !AskingQuestion && ((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B))))
	{
		playMenuBackSound();
		AskQuestion(qsQuitPlaying, "Quit playing the\ncurrent level and\nreturn to the level\nselector?\n\n(A) Quit (B) Cancel");
	}

	//restart
	if (!AskingQuestion && (currButtons & BUTTON_L) && (!(prevButtons & BUTTON_L)))
	{
		playMenuSelectSound();
		AskQuestion(qsRestartLevel, "You are about to\nrestart this level\nAre you sure you\nwant to restart?\n\n(A)Restart (B)Cancel");
	}

	//freeview
	if (!FreeView && !AskingQuestion && (currButtons & BUTTON_R) && (!(prevButtons & BUTTON_R)))
	{
		FreeView = true;
		NeedRedraw = true;
		prevButtons = currButtons;
		playMenuSelectSound();
	}

	if (FreeView && !AskingQuestion)
	{
		if (((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B))) ||
			((currButtons & BUTTON_R) && (!(prevButtons & BUTTON_R))))
		{
			FreeView = false;
			CWorldParts_CenterVPOnPlayer(WorldParts);
			NeedRedraw = true;
			playMenuBackSound();
		}
	}
	else
	{
		if (WorldParts->Player)
		{
			if (!AskingQuestion && !WorldParts->Player->IsMoving && !WorldParts->AttchedBoxQuedOrMoving)
			{
				//pickup 
				if (((currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A))) || ((currButtons & BUTTON_UP) && (!(prevButtons & BUTTON_UP))))
				{
					//dropping a block
					//if there is a block on top of the player and were facing left				
					if (WorldParts->NumBoxesAttachedToPlayer > 0)
					{
						CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY - 1);
						if (Part)
						{
							if (Part->Group == GroupBox)
							{
								if (WorldParts->Player->AnimBase == AnimBaseLeft)
								{
									// and it can move to the left
									if ((CWorldPart_CanMoveTo(Part, Part->PlayFieldX - 1, Part->PlayFieldY)))
									{
										//move it to the left
										CWorldPart_AddToMoveQue(Part, Part->PlayFieldX - 1, Part->PlayFieldY);
										CWorldPart_AddToMoveQue(Part, Part->PlayFieldX - 1, Part->PlayFieldY + 1);
										//detaching is set automatically otherwise it would be set too early while the block is still detaching
										//CWorldPart_DeattachFromPlayer(WorldParts->Items[teller]);
										//NeedRedraw = true;
										playDropSound();
									}
								}
								else
								{
									// if there is block on top of theplayer were facing right
									if (WorldParts->Player->AnimBase == AnimBaseRight)
									{
										// and it can move to the left
										if ((CWorldPart_CanMoveTo(Part, Part->PlayFieldX + 1, Part->PlayFieldY)))
										{
											//move it to right
											CWorldPart_AddToMoveQue(Part, Part->PlayFieldX + 1, Part->PlayFieldY);
											CWorldPart_AddToMoveQue(Part, Part->PlayFieldX + 1, Part->PlayFieldY + 1);
											//detaching is set automatically otherwise it would be set too early while the block is still detaching
											//CWorldPart_DeattachFromPlayer(WorldParts->Items[teller]);
											//NeedRedraw = true;
											playDropSound();
										}

									}
								}
							}
						}
					}
					else
					{
						if (WorldParts->NumBoxesAttachedToPlayer == 0)
						{
							bool FloorFound = false;
							//picking up a block
							//if there is a block left to the player and were facing left
							if (WorldParts->Player->AnimBase == AnimBaseLeft)
							{
								CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX - 1, WorldParts->Player->PlayFieldY);
								if (Part)
								{
									if (Part->Group == GroupBox)
									{
										//see if there is a floor or block beneath the block
										CWorldPart* Part2 = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX - 1, WorldParts->Player->PlayFieldY + 1);
										if (Part2)
										{
											if ((Part2->Group == GroupFloor) || (Part2->Group == GroupBox) || (Part2->Group == GroupExit))
											{
												FloorFound = true;
											}
										}

										if (Part->PlayFieldY == NrOfRows - 1)
										{
											FloorFound = true;
										}

										if (FloorFound)
										{
											//if there was see if there is space above the block and above the player
											if ((CWorldPart_CanMoveTo(Part, Part->PlayFieldX, Part->PlayFieldY - 1)) &&
												(CWorldPart_CanMoveTo(Part, Part->PlayFieldX + 1, Part->PlayFieldY - 1)))
											{
												//Attach the block to the player & move the block
												CWorldPart_AttachToPlayer(Part, WorldParts->Player);
												CWorldPart_AddToMoveQue(Part, Part->PlayFieldX, Part->PlayFieldY - 1);
												CWorldPart_AddToMoveQue(Part, Part->PlayFieldX + 1, Part->PlayFieldY - 1);
												//NeedRedraw = true;
												playPickupSound();
											}
										}
									}
								}
							}
							else
							{
								if (WorldParts->Player->AnimBase == AnimBaseRight)
								{
									CWorldPart* Part = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX + 1, WorldParts->Player->PlayFieldY);
									if (Part)
									{
										if (Part->Group == GroupBox)
										{
											//see if there is a floor or block beneath the block
											CWorldPart* Part2 = CWorldParts_PartAtPosition(WorldParts, WorldParts->Player->PlayFieldX + 1, WorldParts->Player->PlayFieldY + 1);
											if (Part2)
											{
												if ((Part2->Group == GroupFloor) || (Part2->Group == GroupBox) || (Part2->Group == GroupExit))
												{
													FloorFound = true;
												}
											}


											if (Part->PlayFieldY == NrOfRows - 1)
											{
												FloorFound = true;
											}

											if (FloorFound)
											{
												//if there was see if there is space above the block and above the player
												if (CWorldPart_CanMoveTo(Part, Part->PlayFieldX, Part->PlayFieldY - 1) &&
													CWorldPart_CanMoveTo(Part, Part->PlayFieldX - 1, Part->PlayFieldY - 1))
												{
													//Attach the block to the player & move the block
													CWorldPart_AttachToPlayer(Part, WorldParts->Player);
													CWorldPart_AddToMoveQue(Part, Part->PlayFieldX, Part->PlayFieldY - 1);
													CWorldPart_AddToMoveQue(Part, Part->PlayFieldX - 1, Part->PlayFieldY - 1);
													//NeedRedraw = true;
													playPickupSound();
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//the board notices the viewport scrolled and repaints everything by itself
	if (FreeView)
	{
		if (!AskingQuestion)
		{
			if (currButtons & BUTTON_LEFT)
				CViewPort_Move(WorldParts->ViewPort, -ViewportMove, 0);
			if (currButtons & BUTTON_RIGHT)
				CViewPort_Move(WorldParts->ViewPort, ViewportMove, 0);
			if (currButtons & BUTTON_UP)
				CViewPort_Move(WorldParts->ViewPort, 0, -ViewportMove);
			if (currButtons & BUTTON_DOWN)
				CViewPort_Move(WorldParts->ViewPort, 0, ViewportMove);
		}
	}
	else
	{
		//need to have a input delay, its too taxing otherwise
		framecounter++;
		if (framecounter >= FrameDelayInput)
		{
			framecounter = 0;
			if (WorldParts->Player)
			{
				if (!AskingQuestion && !WorldParts->Player->IsMoving && !WorldParts->AttchedBoxQuedOrMoving)
				{
					if (currButtons & BUTTON_LEFT)
					{
						if (CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX - 1, WorldParts->Player->PlayFieldY))
						{
							NeedRedraw = true;
						}
						else
						{
							//move up
							NeedRedraw |= CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY - 1);
						}
					}

					if (currButtons & BUTTON_RIGHT)
					{
						if (CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX + 1, WorldParts->Player->PlayFieldY))
						{
							NeedRedraw = true;
						}
						else
						{
							//move up
							NeedRedraw |= CWorldPart_MoveTo(WorldParts->Player, WorldParts->Player->PlayFieldX, WorldParts->Player->PlayFieldY - 1);
						}
					}
				}
			}
		}
	}


	if (!AskingQuestion)
	{
		CWorldParts_Move(WorldParts);

		if (NeedToReloadGraphics)
		{
			LoadGraphics();
			NeedToReloadGraphics = false;
			//every image changed
			CWorldParts_MarkAllDirty();
		}

		//only paints what changed, so it can run every frame
		bool boardPainted = CWorldParts_DrawBoard(WorldParts);
		//the top bar sits over the board, so it is repainted whenever the board was
		if (FreeView && (boardPainted || NeedRedraw))
		{
			GFX.fillRect(0, 0, WINDOW_WIDTH, 12, ColorWhite);
			GFX.drawRect(0, 11, WINDOW_WIDTH, 1, ColorBlack);
			tftPrint(2,2, "dpad:Move B:exit", ColorBlack, ColorBlack,1);
		}
		NeedRedraw = false;
	}

	if (WorldParts->Player)
	{
		if (!AskingQuestion && !WorldParts->Player->IsMoving && StageDone(WorldParts->Player))
		{
			//to one extra move & draw to make sure boxes are on final spot
			CWorldParts_Move(WorldParts);
			CWorldParts_DrawBoard(WorldParts);
			playLevelDoneSound();
			char Text[100];
			if ((SelectedLevel == lastUnlockedLevel()))
			{
				
				if (lastUnlockedLevel() < InstalledLevels)
				{
					snprintf(Text, sizeof(Text), "You Solved Lvl %d/%d\nNext level has\nbeen unlocked!\n\n(A) Continue", SelectedLevel, InstalledLevels);
					AskQuestion(qsSolvedNotLastLevel, Text);
				}
				else
				{
					snprintf(Text, sizeof(Text), "You Solved Lvl %d/%d\nAll levels are\nnow finished!\n\n(A) Continue", SelectedLevel, InstalledLevels);
					AskQuestion(qsSolvedLastLevel, Text);
				}
			}
			else
			{
				snprintf(Text, sizeof(Text), "You Solved Lvl %d/%d\n\n(A) Continue", SelectedLevel, InstalledLevels);
				AskQuestion(qsSolvedLevel, Text);
			}
		}
	}

	int8_t id;
	bool answer;
	//simple confirm messages
	if ((AskingQuestionID == qsSolvedNotLastLevel) || (AskingQuestionID == qsSolvedLastLevel) || (AskingQuestionID == qsSolvedLevel))
	{
		if (AskQuestionUpdate(&id, &answer, true))
		{
			if ((id == qsSolvedNotLastLevel) && answer)
			{
				SelectedLevel++;
				unlockLevel(SelectedLevel);
				GameState = GSStageSelectInit;
			}

			if ((id == qsSolvedLastLevel) && answer)
			{
				GameState = GSTitleScreenInit;
			}

			if ((id == qsSolvedLevel) && answer)
			{
				GameState = GSStageSelectInit;
			}
		}
	}

	//Yes / No Questions
	if (AskingQuestionID == qsRestartLevel)
	{
		if (AskQuestionUpdate(&id, &answer, false))
		{
			if (id == qsRestartLevel)
			{
				if (answer)
				{
					LoadSelectedLevel();
					CWorldParts_MarkAllDirty();
					FreeView = false;
				}
				//either way the question was drawn over the board and the free view bar
				NeedRedraw = true;
			}
		}
	}

	if (AskingQuestionID == qsQuitPlaying)
	{
		if (AskQuestionUpdate(&id, &answer, false))
		{
			if (id == qsQuitPlaying)
			{
				//keep playing needs no repaint here, closing the question marked the board dirty
				if (answer)
				{
					GameState = GSStageSelectInit;
				}
			}
		}
	}
}