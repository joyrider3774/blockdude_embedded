#ifndef commonvars_h
#define commonvars_h

#include <stdint.h>
#include "defines.h"
#include "Platform.h"

typedef struct CWorldPart CWorldPart;
typedef struct CWorldParts CWorldParts;

extern const uint8_t * IMGFloor, * IMGPlayer, * IMGBox, * IMGEmpty, * IMGExit, * IMGFloorLeft, * IMGFloorRight,
* IMGEarthGrassRight, * IMGEarthGrassLeft, * IMGEarthLeft, * IMGEarthRight, * IMGEarthMiddle, * IMGFloatingFloor, * IMGFloatingFloorLeft,
* IMGFloatingFloorRight, * IMGFloatingFloorMiddle, * IMGTower, * IMGStartTower, * IMGTowerShaft, * IMGRoof1, * IMGRoof2, * IMGRoofCornerLeft,
* IMGRoofCornerRight, * IMGRoofDownRight, * IMGRoofDownLeft, * IMGRoofCornerBoth;

extern const uint8_t * IMGBackground, *IMGIntro1, * IMGIntro2, * IMGIntro3, * IMGTitleScreen;
extern uint16_t ColorWhite, ColorBlack;
extern const char* LevelPacks[];
extern const char* skins[];
//sized to what they hold: skin 0..MAXSKINS-1, GameState at most GSStageSelect + GSDiff,
//levels at most InstalledLevelsDefaultGame. SelectedLevel, AskingQuestionID and
//CurrentLevelPackIndex go below 0 (-1 = none, or just before being wrapped / clamped)
static_assert((MAXSKINS <= 255) && (GSStageSelect + GSDiff <= 255) && (InstalledLevelsDefaultGame <= 127) && (MaxLevelPacks <= 127) && (qsRestartLevel <= 127), "globals do not fit their types");
extern uint8_t skin;
extern uint8_t GameState;
extern int8_t SelectedLevel;
extern bool NeedRedraw;
extern int8_t AskingQuestionID;
extern uint8_t titleStep, titleSelection;
extern uint8_t InstalledLevels;
extern int8_t CurrentLevelPackIndex;
extern uint8_t FoundLevelPacks;
extern bool AskingQuestion, FreeView, NeedToReloadGraphics;
extern CWorldParts* WorldParts;
extern uint8_t IntroScreenNr;
extern uint8_t framecounter;   //counts to FRAMERATE * 3 + 1 in the intro
//frame counters run for as long as the game does, menus repeat on (framecount - start) % MenuUpdateTicks
extern uint32_t framecount, frameUpStart, frameDownStart, frameLeftStart, frameRightStart;
extern uint8_t currButtons, prevButtons;

#endif