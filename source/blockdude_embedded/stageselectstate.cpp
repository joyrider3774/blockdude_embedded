#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stageselectstate.h"
#include "commonvars.h"
#include "gamefuncs.h"
#include "cworldparts.h"
#include "sound.h"
#include "savestate.h"

void StageSelectInit()
{
	//coming back from a game or a menu, repaint the whole board
	CWorldParts_MarkAllDirty();
	LoadSelectedLevel();
	NeedRedraw = true;
}

void StageSelect()
{
	if (GameState == GSStageSelectInit)
	{
		StageSelectInit();
		GameState -= GSDiff;
	}


	if (!AskingQuestion && (currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B)))
	{
		GameState = GSTitleScreenInit;
		CWorldParts_RemoveAll(WorldParts);
		playMenuBackSound();
	}
	else
	if (!AskingQuestion && (currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A)))
	{
		playMenuSelectSound();		
		if (SelectedLevel <= lastUnlockedLevel())
			GameState = GSGameInit;
		else
		{
			//82 bytes with real level numbers, 104 with the widest ints
			char Text[128];
			snprintf(Text, sizeof(Text), "Level Has not been\nunlocked yet!\nPlay last unlocked\nlevel %d/%d?\n\n(A) Play (B) Cancel", lastUnlockedLevel(), InstalledLevels);
			AskQuestion(qsNotUnlocked, Text);
		}

	}

	if (!AskingQuestion && (currButtons & BUTTON_LEFT))
	{
		if (!(prevButtons & BUTTON_LEFT))
			frameLeftStart = framecount;
		//held at the first level it would otherwise reload the same level over and over
		if (((framecount - frameLeftStart) % LevelSelectUpdateTicks == 0) && (SelectedLevel > 1))
		{
			playMenuSound();
			NeedRedraw = true;
			SelectedLevel--;
			LoadSelectedLevel();
		}
	}

	if (!AskingQuestion && (currButtons & BUTTON_RIGHT))
	{
		if (!(prevButtons & BUTTON_RIGHT))
			frameRightStart = framecount;
		if (((framecount - frameRightStart) % LevelSelectUpdateTicks == 0) && (SelectedLevel < InstalledLevels))
		{
			playMenuSound();
			NeedRedraw = true;
			SelectedLevel++;
			LoadSelectedLevel();
		}
	}

	//not when leaving this screen, the next one paints its own
	if (!AskingQuestion && (GameState == GSStageSelect))
	{
		bool boardPainted = CWorldParts_DrawBoard(WorldParts);
		//the top bar sits over the board, so it is repainted whenever the board was
		if (boardPainted || NeedRedraw)
		{
			char Text[100];
			GFX.fillRect(0, 0, WINDOW_WIDTH, 12, ColorWhite);
			GFX.drawRect(0, 11, WINDOW_WIDTH, 1, ColorBlack);
			if (SelectedLevel <= lastUnlockedLevel())
				snprintf(Text, sizeof(Text), "Lvl: %d/%d A:Play", SelectedLevel, InstalledLevels);
			else
				snprintf(Text, sizeof(Text), "Lvl: %d/%d Locked!", SelectedLevel, InstalledLevels);

			tftPrint(2,2,Text, ColorBlack, ColorBlack, 1);
		}
		NeedRedraw = false;
	}

	int8_t id = -1;
	bool answer = false;
	if (AskQuestionUpdate(&id, &answer, false))
	{
		if (id == qsNotUnlocked)
		{
			//closing the question already marked the board dirty
			if (answer)
			{
				SelectedLevel = lastUnlockedLevel();
				LoadSelectedLevel();
				GameState = GSGameInit;
			}
		}
	}
}
