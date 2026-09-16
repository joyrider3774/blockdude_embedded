#include <stdint.h>
#include <stddef.h>
#include "commonvars.h"

const uint8_t* IMGFloor = NULL, * IMGPlayer = NULL, * IMGBox = NULL, * IMGEmpty = NULL, * IMGExit = NULL, * IMGFloorLeft = NULL, * IMGFloorRight = NULL,
    * IMGEarthGrassRight = NULL, * IMGEarthGrassLeft = NULL, * IMGEarthLeft = NULL, * IMGEarthRight = NULL, * IMGEarthMiddle = NULL, * IMGFloatingFloor = NULL,
    * IMGFloatingFloorLeft = NULL, * IMGFloatingFloorRight = NULL, * IMGFloatingFloorMiddle = NULL, * IMGTower = NULL, * IMGStartTower = NULL, * IMGTowerShaft = NULL,
    * IMGRoof1 = NULL, * IMGRoof2 = NULL, * IMGRoofCornerLeft = NULL,* IMGRoofCornerRight = NULL, * IMGRoofDownRight = NULL, * IMGRoofDownLeft = NULL,
    * IMGRoofCornerBoth = NULL;
const uint8_t *IMGBackground = NULL, * IMGIntro1 = NULL, *IMGIntro2 = NULL, *IMGIntro3 = NULL, *IMGTitleScreen = NULL;

uint16_t ColorWhite, ColorBlack;
const char* skins[] = { "Default", "Tech", "Flat", "Ti-83", "Kenney"};
const char* LevelPacks[] = {"Blockman", "Davy"};

uint8_t GameState = GSIntroInit;
int8_t SelectedLevel = 0;
bool NeedRedraw = true;
int8_t AskingQuestionID = -1;
uint8_t titleStep = tsMainMenu, titleSelection = 0;
uint8_t InstalledLevels = 0;
int8_t CurrentLevelPackIndex = 0;
uint8_t FoundLevelPacks = 0;
bool AskingQuestion = false, FreeView = false, NeedToReloadGraphics = false;
uint8_t IntroScreenNr = 1;
uint8_t framecounter = 0;
uint32_t framecount = 0, frameUpStart = 0, frameDownStart = 0, frameLeftStart = 0, frameRightStart = 0;
CWorldParts* WorldParts;
uint8_t currButtons, prevButtons;
