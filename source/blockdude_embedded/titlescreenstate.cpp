#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "titlescreenstate.h"
#include "commonvars.h"
#include "sound.h"
#include "gamefuncs.h"
#include "savestate.h"

void TitleScreenInit(void)
{
	NeedRedraw = true;	
}

void TitleScreen()
{
	if (GameState == GSTitleScreenInit)
	{
		TitleScreenInit();
		GameState -= GSDiff;
	}

	if (!AskingQuestion)
	{
		if ((currButtons & BUTTON_LEFT))
		{
			if (!(prevButtons & BUTTON_LEFT))
				frameLeftStart = framecount;
			if ((framecount - frameLeftStart) % MenuUpdateTicks == 0)
			{
				switch (titleStep)
				{
				case tsMainMenu:
					switch (titleSelection)
					{
					case mmPack:
						CurrentLevelPackIndex--;
						if (CurrentLevelPackIndex < 0)
							CurrentLevelPackIndex = FoundLevelPacks - 1;
						playMenuSelectSound();
						NeedRedraw = true;
						break;
					default:
						break;
					}
					break;
				case tsOptions:
					switch (titleSelection)
					{
					case opMusicSound:
						playMenuSelectSound();
						if (isSoundOn())
						{
							setMusicOn(true);
							setMusicOnSaveState(true);
							setSoundOn(false);
							setSoundOnSaveState(false);
						}
						else
						{
							setMusicOn(true);
							setMusicOnSaveState(true);
							setSoundOn(true);
							setSoundOnSaveState(true);
						}
						NeedRedraw = true;
	    				break;
					case opSkin:
#if FORCESKIN >= 0
						//only one skin is built in, FORCESKIN (a 1 bpp buffer forces the black & white one)
						playErrorSound();
						break;
#endif
						playMenuSelectSound();
						int8_t i = skinSaveState();
						i--;
						if (i == -1)
							i = MAXSKINS - 1;
						setSkinSaveState(i);
						LoadGraphics();
						NeedRedraw = true;
						break;
					}
					break;
				default:
					break;
				}
			}
		}

		if ((currButtons & BUTTON_RIGHT))
		{
			if (!(prevButtons & BUTTON_RIGHT))
				frameRightStart = framecount;
			if ((framecount - frameRightStart) % MenuUpdateTicks == 0)
			{
				switch (titleStep)
				{
				case tsMainMenu:
					switch (titleSelection)
					{
					case mmPack:
						CurrentLevelPackIndex++;
						if (CurrentLevelPackIndex == FoundLevelPacks)
							CurrentLevelPackIndex = 0;
						playMenuSelectSound();
						NeedRedraw = true;
						break;
					default:
						break;
					}
					break;
				case tsOptions:
					switch (titleSelection)
					{
					case opMusicSound:
						playMenuSelectSound();
						if (isSoundOn())
						{
							setMusicOn(true);
							setMusicOnSaveState(true);
							setSoundOn(false);
							setSoundOnSaveState(false);
						}
						else
						{
							setMusicOn(true);
							setMusicOnSaveState(true);
							setSoundOn(true);
							setSoundOnSaveState(true);
						}
						NeedRedraw = true;
						break;
					case opSkin:
#if FORCESKIN >= 0
						//only one skin is built in, FORCESKIN (a 1 bpp buffer forces the black & white one)
						playErrorSound();
						break;
#endif
						playMenuSelectSound();
						int8_t i = skinSaveState();
						i++;
						if (i == MAXSKINS)
							i = 0;
						setSkinSaveState(i);
						LoadGraphics();
						NeedRedraw = true;
						break;
					}
					break;
				default:
					break;
				}
			}
		}


		if ((currButtons & BUTTON_DOWN))
		{
			if (!(prevButtons & BUTTON_DOWN))
				frameDownStart = framecount;
			if ((framecount - frameDownStart) % MenuUpdateTicks == 0)
			{
				switch (titleStep)
				{
				case tsMainMenu:
					if (titleSelection < mmCount - 1)
					{
						titleSelection++;
						playMenuSound();
						NeedRedraw = true;
					}
					break;
				case tsOptions:
					if (titleSelection < opCount - 1)
					{
						titleSelection++;
						playMenuSound();
						NeedRedraw = true;
					}
					break;			
				}
			}
		}

		if ((currButtons & BUTTON_UP))
		{
			if (!(prevButtons & BUTTON_UP))
				frameUpStart = framecount;
			if ((framecount - frameUpStart) % MenuUpdateTicks == 0)
			{
				switch (titleStep)
				{
				case tsMainMenu:
				case tsOptions:
					if (titleSelection > 0)
					{
						titleSelection--;
						playMenuSound();
						NeedRedraw = true;
					}
					break;
				}
			}
		}

		if ((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B)))
		{
			switch (titleStep)
			{
			case tsOptions:
				titleStep = tsMainMenu;
				titleSelection = mmOptions;
				playMenuBackSound();
				NeedRedraw = true;
				break;
			case tsCredits:
				break;
			}
		}

		if ((currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A)))
		{
			switch (titleStep)
			{
			case tsMainMenu:
				switch (titleSelection)
				{				
				case mmStartGame:
					FindLevels();
					if (InstalledLevels > 0)
					{
						SelectedLevel = lastUnlockedLevel();
						GameState = GSStageSelectInit;
						playMenuSelectSound();
					}
					break;
				case mmPack:
					CurrentLevelPackIndex++;
					if (CurrentLevelPackIndex == FoundLevelPacks)
						CurrentLevelPackIndex = 0;
					playMenuSelectSound();
					NeedRedraw = true;
					break;
				case mmCredits:
					playMenuSelectSound();
					AskQuestion(qsCredits, "Created by:\nWillems Davy\nTech & Default skin:\nFusion Power\nFlat skin:\nKlas Kroon &\nKris Katiesen\nKenney's skin:\nKenney\nLevels from blockman\nby Soleau Software");
					break;
				case mmOptions:
					titleStep = tsOptions;
					titleSelection = opMusicSound;
					playMenuSelectSound();
					NeedRedraw = true;
					break;
				}
				break;
			case tsOptions:
				switch (titleSelection)
				{
				case opMusicSound:
					playMenuSelectSound();
					if (isSoundOn())
					{
						setMusicOn(true);
						setMusicOnSaveState(true);
						setSoundOn(false);
						setSoundOnSaveState(false);
					}
					else
					{
						setMusicOn(true);
						setMusicOnSaveState(true);
						setSoundOn(true);
						setSoundOnSaveState(true);
					}

					NeedRedraw = true;
					break;
				case opSkin:
#if FORCESKIN >= 0
					//only one skin is built in, FORCESKIN (a 1 bpp buffer forces the black & white one)
					playErrorSound();
					break;
#endif
					playMenuSelectSound();
					int8_t i = skinSaveState();
					i++;
					if (i == MAXSKINS)
						i = 0;
					setSkinSaveState(i);
					LoadGraphics();
					NeedRedraw = true;
					break;
				}
				break;
			case tsCredits:
				break;	
			}		
		}
	}

	if (NeedRedraw)
	{
		NeedRedraw = false;
		pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGTitleScreen);
		char Text[100];
		switch (titleStep)
		{
		case tsMainMenu:
			snprintf(Text, sizeof(Text), "Start Game\n<%s>\nOptions\nCredits\n", LevelPacks[CurrentLevelPackIndex]);
			GFX.fillRect(9,35,111,56, ColorWhite);
			GFX.drawRect(9,35,111,56, ColorBlack);
			tftPrint(40,40,Text, ColorBlack, ColorBlack, 1);
			switch (titleSelection)
			{
			case mmStartGame:
				snprintf(Text, sizeof(Text), ">");
				break;
			case mmPack:
				snprintf(Text, sizeof(Text), "\n>");
				break;
     		case mmOptions:
				snprintf(Text, sizeof(Text), "\n\n>");
				break;
			case mmCredits:
				snprintf(Text, sizeof(Text), "\n\n\n>");
				break;
			}
			tftPrint(30, 40, Text, ColorBlack, ColorBlack,1);
			break;
		case tsCredits:
			break;
		case tsOptions:
			GFX.fillRect(9,35,111,56, ColorWhite);
			GFX.drawRect(9,35,111,56, ColorBlack);
			snprintf(Text, sizeof(Text), "Audio %s", isSoundOn() ? "On":"Off");
			tftPrint(40, 40, Text, ColorBlack, ColorBlack,1);
			
			snprintf(Text, sizeof(Text), "%s skin", skins[CurrentSkin()]);
			tftPrint(40, 50, Text, ColorBlack, ColorBlack,1);

			switch (titleSelection)
			{
			case opMusicSound:
				tftPrint(30, 40, ">", ColorBlack, ColorBlack,1);
				break;
			case opSkin:
				tftPrint(30, 50, ">", ColorBlack, ColorBlack,1);
				break;
			}
			break;

		}
	}

	int8_t id = -1;
	bool answered = false;

	id = -1;
	answered = false;
	if (AskQuestionUpdate(&id, &answered, AskingQuestionID != qsCredits))
	{
	}
}
