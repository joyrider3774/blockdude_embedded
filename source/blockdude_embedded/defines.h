#ifndef defines_h
#define defines_h

//the device comes first: the display library, SCREENBUFFER and IMAGESET are device settings, see
//PlatformESPboy.h / PlatformSDL.h
#include "PlatformDevice.h"

#define FRAMERATE 30
//1 = every frame waits until 1/FRAMERATE of a second has passed, 0 = a frame starts as soon
//as the last one is done, to see how fast the game can go. Movement, animation, input
//repeat and music all count frames, so without the lock they run faster as well.
//A build can set it itself
#ifndef FPSLOCK
#define FPSLOCK 1
#endif
//1 = the debug header (frame rate, free heap and stack) is always shown, Up + Down does not
//hide it. 0 = it starts hidden and Up + Down shows and hides it. A build can set it itself
#ifndef FORCEDEBUG
#define FORCEDEBUG 0
#endif
//GameMoveSpeed and PlayerAnimDelay depend on the tile size, see IMAGESET below
#define FrameDelayInput 1			
#define FrameDelayInputLevelEditor 3
//a held direction in the menus acts again every this many frames
#define MenuUpdateTicks 10
//frames between repeats while a direction is held in the level selector
#define LevelSelectUpdateTicks 5
#define ViewportMove 3					//dec if fps increases

#define MAXSKINS 5
//Kenney's skin is white on black, the only one a 1 bpp buffer can show
#define SKINBLACKWHITE 4

//FORCESKIN: -1 = every skin is built in and can be picked in the options, n = only skin n
//(0 Default, 1 Tech, 2 Flat, 3 Ti-83, 4 Kenney) is built in and always used, which saves the
//flash of the others on a small device. A 1 bpp buffer can only show SKINBLACKWHITE, so it
//forces that one. Set by the device header or the build
#if SCREENBUFFER == 1
  #if defined(FORCESKIN) && (FORCESKIN >= 0) && (FORCESKIN != SKINBLACKWHITE)
  #error "a 1 bpp buffer can only show the black & white skin, FORCESKIN has to be -1 or 4"
  #endif
  #undef FORCESKIN
  #define FORCESKIN SKINBLACKWHITE
#elif !defined(FORCESKIN)
  #define FORCESKIN -1
#endif
#if (FORCESKIN < -1) || (FORCESKIN >= MAXSKINS)
#error "FORCESKIN has to be -1 or a skin number below MAXSKINS"
#endif
//1 when the images of skin n are part of the build
#define SKINBUILT(n) ((FORCESKIN < 0) || (FORCESKIN == (n)))
#define WINDOW_WIDTH 128
#define WINDOW_HEIGHT 128
#define HALFWINDOWWIDTH 64
#define HALFWINDOWHEIGHT 64

//skin headers to build with: 1 = images (16x16 tiles), 2 = images2 (same skins at half size, 8x8 tiles)
//set by the device header (PlatformESPboy.h / PlatformSDL.h) or by the build
#ifndef IMAGESET
#error "the device header has to define IMAGESET"
#endif

//move speed is in pixels per frame, so smaller tiles need a lower speed to keep the
//same pace: 16 / 3 and 8 / 2 both take 4 to 5 frames per tile. The anim delay (frames
//per anim phase) follows it so the walk cycle still advances about once per tile
#if IMAGESET == 2
#define IMAGES_DIR images2
#define TileWidth 8
#define TileHeight 8
#define GameMoveSpeed 2					//dec if fps increases
#define PlayerAnimDelay 4				//inc if fps increases
#else
#define IMAGES_DIR images
#define TileWidth 16
#define TileHeight 16
#define GameMoveSpeed 3					//dec if fps increases
#define PlayerAnimDelay 5				//inc if fps increases
#endif

//path of a skin header in the selected folder: #include SKIN_IMAGE(Default/box_table_16_16_RGB565_LE.h)
#define SKIN_IMAGE_STR(x) #x
#define SKIN_IMAGE_PATH(dir, file) SKIN_IMAGE_STR(dir/file)
#define SKIN_IMAGE(file) SKIN_IMAGE_PATH(IMAGES_DIR, file)

#define NrOfRowsVisible (WINDOW_HEIGHT / TileHeight)
#define NrOfColsVisible (WINDOW_WIDTH / TileWidth) 
#define InstalledLevelsDefaultGame 21
#define NrOfRows 23
#define NrOfCols 28
//size of the fixed part pool in cworldpart.cpp. The grid could in principle hold
//NrOfRows * NrOfCols (644) parts, but the busiest of the 25 bundled levels needs
//340, and reserving the whole grid would cost 15k of ram for slots nothing uses.
//This leaves 44 spare. Add a denser level and CWorldParts_Add drops what does not
//fit, so raise this if a level ever comes up short of tiles
#define MAXWORLDPARTS 384
#define MaxLevelPacks 2
#define MaxLenLevelPackName 6

#define IDEmpty 1
#define IDPlayer 2
#define IDBox 3
#define IDFloor 4
#define IDExit 5
#define IDEarthGrassLeft 6
#define IDEarthGrassRight 7
#define IDEarthLeft 8
#define IDEarthMiddle 9
#define IDEarthRight 10
#define IDFloatingFloor 11
#define IDFloatingFloorLeft 12
#define IDFloatingFloorMiddle 13
#define IDFloatingFloorRight 14
#define IDFloorLeft 15
#define IDFloorRight 16
#define IDTower 17
#define IDStartTower 18
#define IDTowerShaft 19
#define IDRoof1 20
#define IDRoof2 21
#define IDRoofCornerLeft 22
#define IDRoofCornerRight 23
#define IDRoofCornerBoth 24
#define IDRoofDownRight 25
#define IDRoofDownLeft 26

//these groups also define drawing order for all dirty items as well as logic
#define GroupNone 10 //special used in level editor
#define GroupExit 0
#define GroupFloor 1
#define GroupBox 2
#define GroupPlayer 3 //has to be drawn and moved last for logic to work
#define NrOfGroups 4 //group none is ignored


#define AnimBaseLeft 0
#define AnimBaseRight 4
#define AnimBaseLeftJump 8
#define AnimBaseRightJump 12

#define errNoError 0
#define errNoPlayer 1
#define errNoExit 2
#define errBlocksPlayerNotOnAFloor 3
#define errBlocksOnPlayerNotOne 4

#define mmStartGame 0
#define mmPack 1
#define mmOptions 2
#define mmCredits 3
#define mmCount 4

#define opMusicSound 0
#define opSkin 1
#define opCount 2

#define tsMainMenu 0
#define tsOptions 1
#define tsCredits 2

#define GSDiff 50

#define GSTitleScreen 0
#define GSIntro 1
#define GSGame 2
#define GSStageClear 3
#define GSStageSelect 4

#define GSIntroInit GSIntro + GSDiff
#define GSGameInit GSGame + GSDiff
#define GSStageClearInit GSStageClear + GSDiff
#define GSTitleScreenInit GSTitleScreen + GSDiff
#define GSStageSelectInit GSStageSelect + GSDiff

#define qsErrPlayer 1
#define qsErrExit 2
#define qsErrBlocksOrPlayerNotOnAFloor 3
#define qsErrBlocksOnPlayerNotOne 4
#define qsNotSaved 5
#define qsNotUnlocked 6
#define qsSolvedNotLastLevel 7
#define qsSolvedLastLevel 8
#define qsSolvedLevel 9
#define qsQuitPlaying 10
#define qsOptimizePack 11
#define qsAllDone 12
#define qsDelPack 13
#define qsClearLevel 14
#define qsCredits 15
#define qsRestartLevel 16

#endif
