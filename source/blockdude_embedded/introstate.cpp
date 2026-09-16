#include "introstate.h"
#include "commonvars.h"
#include "gamefuncs.h"

void IntroInit(void)
{
	NeedRedraw = true;
}

void Intro(void)
{
	if (GameState == GSIntroInit)
	{
		IntroInit();
		GameState -= GSDiff;
	}
	framecounter++;

	if (((currButtons & BUTTON_A) && (!(prevButtons & BUTTON_A))) ||
		((currButtons & BUTTON_B) && (!(prevButtons & BUTTON_B))))
	{
		GameState = GSTitleScreenInit;
	}

	if (NeedRedraw)
	{
		NeedRedraw = false;
		switch (IntroScreenNr)
		{
		case 1:
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGIntro1);
			break;
		case 2:
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGIntro2);
			break;
		case 3:
			pushImageRLE(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, IMGIntro3);
			break;
		}
	}

	if (framecounter > FRAMERATE * 3)
	{
		framecounter = 0;
		IntroScreenNr++;
		NeedRedraw = true;
		if (IntroScreenNr > 3)
			GameState = GSTitleScreenInit;
	}
}