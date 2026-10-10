#ifndef defines_h
#define defines_h

//the device comes first: the display library, SCREENBUFFER and IMAGESET are device settings, see
//PlatformESPboy.h / PlatformSDL.h
#include "PlatformDevice.h"

//1 = the art is read from a card while the game runs and none of it is in flash, see
//cardimages.h. It needs a device that can read one (PLATFORM_HAS_CARD in Platform.h) and the
//card file tools/mkcard.py writes. Every skin is then on the card in full RGB565 and the game
//can be asked for any of them, which is what flash could never hold: this game has five skins
#ifndef CARDIMAGES
#define CARDIMAGES 0
#endif

//How much RAM a card build keeps its art in. A picture small enough to be worth it is read once
//and kept here, so drawing it again is a copy; a full screen one is read a row or a strip at a
//time and never kept. 0 is no arena at all. See the arena in cardimages.cpp
//How much of a level's plane is held while it is read off the card. A level is three planes
//walked together, so three of these are live at once; a plane is a few dozen bytes
#ifndef CARD_LEVEL_CHUNK
#define CARD_LEVEL_CHUNK 32
#endif

#ifndef CARDARENA
#define CARDARENA 2048
#endif

//1 = the full screen background is drawn as one plain colour instead of as a picture, for a
//skin whose background is one colour a row. Off here: of this game's five skins only Kenney
//and Ti-83 are like that, the Default, Tech and Flat ones have a real picture behind the board
#ifndef FLATBACKGROUND
#define FLATBACKGROUND 0
#endif

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
//1 = the colours of an image are spread over the ones the buffer can hold, so that a shade it
//has no colour for is a pattern of the two it does instead of the nearer of them. 0 = every
//colour becomes the nearest one there is, which shows as bands across anything that shades.
//An 8 bpp buffer is RGB332 and drops 2 bits of red, 3 of green and 3 of blue, and a 1 bpp buffer
//keeps only black and white, so both have something to spread. A 16 bpp buffer holds every colour
//of the image as it is and is left alone. A build can set this itself, see DitherSpread in
//Platform.h
#ifndef DITHERING
#define DITHERING 0
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
//flash of the others on a small device. A 1 bpp buffer has only two colours to show, so the
//black & white skin is the one it takes on its own. A build can still ask it for another one,
//whose shades then go through the brightness rule in SetBufferBit, and with DITHERING come out
//as a pattern of the two colours rather than as the nearer of them. Set by the device header or the build
//A card build names no skin either: every one of them is on the card in full RGB565 and the
//game is asked for one while it runs, see CardImages_UseSkin
#if !defined(FORCESKIN)
  #if (SCREENBUFFER == 1) && !CARDIMAGES
  #define FORCESKIN SKINBLACKWHITE
  #else
  #define FORCESKIN -1
  #endif
#endif
#if (FORCESKIN < -1) || (FORCESKIN >= MAXSKINS)
#error "FORCESKIN has to be -1 or a skin number below MAXSKINS"
#endif
//1 when the images of skin n are part of the build
#define SKINBUILT(n) ((FORCESKIN < 0) || (FORCESKIN == (n)))

//1 when the black & white skin is in the build, whose pictures are packed one bit a pixel
//by tools/onebit.py and drawn by the routines in onebitimage.cpp rather than as RGB565. It
//shows two colours, and keeping each of them in sixteen bits costs both flash and the work
//of writing a colour per pixel. Every skin can be in the build here and picked in the
//options, so which kind a picture is cannot be known at build time: skinImagesOneBit says
//A card build has none of them: every skin is on the card in full RGB565, so there is no
//reduced form to read and nothing of the one bit paths is built
#define ONEBITIMAGES (!CARDIMAGES && SKINBUILT(SKINBLACKWHITE))

//1 when the black & white skin is the only one in the build. Every picture is then one bit a pixel
//and the paths that read RGB565 are dead: a build that is only ever going to draw one bit pictures
//need not carry the index the run length encoded background is read through, which is a row table
//the width of the screen
#define ONEBITONLY (ONEBITIMAGES && (FORCESKIN == SKINBLACKWHITE))
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
//The pool of world parts, the largest single thing the game asks the heap for. The device header
//may ask for fewer, which a device with little ram has to: it can size itself from
//LEVELPACKMAXPARTS, the busiest level of the packs it ships, which is checked below where that
//is known
#ifndef MAXWORLDPARTS
#define MAXWORLDPARTS 384
#endif
#define MaxLevelPacks 2

//1 = the rows between the bottom of a level and the bottom of the screen are filled with earth,
//so a level shorter than the display has ground under it rather than background. 0 = they are left
//alone. Every filled tile is a world part out of the pool, which on a small device is ram it has
//not got, see CWorldParts_Load and LP_FILL below. A build can set it itself
#ifndef PADLEVELROWS
#define PADLEVELROWS 1
#endif

//>>> written by tools/convert_levels.py from assets/levels, do not edit by hand
//LEVELPACKS: the level packs that are built in, an LP_ bit each (the size is what the
//pack takes in flash). All of them unless the device header or the build picks fewer; a
//pack that is left out takes no flash and is not offered in the game
#define LP_blockman (1ul <<  0)    //21 levels,   9609 bytes
#define LP_davy     (1ul <<  1)    // 4 levels,   1643 bytes
#define LP_ALL ((1ul << 2) - 1)
#ifndef LEVELPACKS
#define LEVELPACKS LP_ALL
#endif
#if (LEVELPACKS & LP_ALL) == 0
#error "LEVELPACKS has to leave at least one level pack in"
#endif
//how many that comes to, which is how many the game offers
#define LEVELPACKCOUNT (((LEVELPACKS & LP_blockman) != 0) + ((LEVELPACKS & LP_davy) != 0))

//FIRSTLEVELPERPACK and MAXLEVELSPERPACK: the run of levels a pack keeps, for a device
//that has not the flash for a whole one. The levels of a pack are in the order they are
//played, so a build takes MAXLEVELSPERPACK of them starting at FIRSTLEVELPERPACK and the
//rest are left out; 0 levels means all of them from the first one on. Several builds with
//runs that follow one another cover a whole pack between them
#ifndef FIRSTLEVELPERPACK
#define FIRSTLEVELPERPACK 0
#endif
#ifndef MAXLEVELSPERPACK
#define MAXLEVELSPERPACK 0
#endif
//A pack may be given a run of its own, and takes the one above when it is not. That is what
//lets one build hold the end of a long pack and the whole of a short one
#ifndef FIRSTLEVEL_blockman
#define FIRSTLEVEL_blockman FIRSTLEVELPERPACK
#endif
#ifndef MAXLEVELS_blockman
#define MAXLEVELS_blockman MAXLEVELSPERPACK
#endif
#ifndef FIRSTLEVEL_davy
#define FIRSTLEVEL_davy FIRSTLEVELPERPACK
#endif
#ifndef MAXLEVELS_davy
#define MAXLEVELS_davy MAXLEVELSPERPACK
#endif
//1 while level n of a pack is in the build, counted from 0
#define LEVELBUILT_AT(n, first, most) (((n) >= (first)) && \
                                      (((most) == 0) || ((n) < (first) + (most))))
//of a pack of n levels, how many are kept
#define LEVELSKEPT_AT(n, first, most) (((n) <= (first)) ? 0 : \
                                      ((((most) == 0) || ((n) - (first) <= (most))) \
                                       ? (n) - (first) : (most)))
//the longest of the runs, which is what the lookup table in levels.h is sized by. It is
//written after the packs below, since it is the packs it is the largest of
//A pack the run leaves nothing of would be offered with no levels in it, so this says
//so at build time instead: a build starting past the end of a pack leaves it out too
#define LEVELBUILT_blockman(n) LEVELBUILT_AT(n, FIRSTLEVEL_blockman, MAXLEVELS_blockman)
#define LEVELSKEPT_blockman LEVELSKEPT_AT(21, FIRSTLEVEL_blockman, MAXLEVELS_blockman)
#if (LEVELPACKS & LP_blockman) && (LEVELSKEPT_blockman == 0)
#error "the run leaves nothing of blockman, take that pack out of LEVELPACKS"
#endif
#define LEVELBUILT_davy(n) LEVELBUILT_AT(n, FIRSTLEVEL_davy, MAXLEVELS_davy)
#define LEVELSKEPT_davy LEVELSKEPT_AT(4, FIRSTLEVEL_davy, MAXLEVELS_davy)
#if (LEVELPACKS & LP_davy) && (LEVELSKEPT_davy == 0)
#error "the run leaves nothing of davy, take that pack out of LEVELPACKS"
#endif
//the longest run of all, which is how wide the lookup table in levels.h has to be
#define LEVELSPERPACK ((LEVELSKEPT_blockman) > LEVELSKEPT_davy ? (LEVELSKEPT_blockman) : LEVELSKEPT_davy)

//The busiest level each pack has, counted the same way encode_level does. The pool of world
//parts is the largest single thing the game asks the heap for and no level fills the grid, so
//a build wants no more slots than the packs it holds can fill, see MAXWORLDPARTS
//A level is padded with earth from its bottom row down to the bottom of the screen, see
//CWorldParts_Load, so the parts it makes depend on how many rows the device shows. This is
//that padding for a level whose bounding box is w by h tiles
#define LP_ROWSSHOWN ((NrOfRowsVisible < NrOfRows) ? NrOfRowsVisible : NrOfRows)
#define LP_FILL(w, h) ((PADLEVELROWS && (LP_ROWSSHOWN > (h))) \
                       ? ((LP_ROWSSHOWN - (h)) * (((w) < NrOfCols) ? (w) : NrOfCols)) : 0)
//the entries of each level plus that padding, the largest of them per pack
#define LP_PARTS_blockman LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(0, 266 + LP_FILL(25, 15)), 276 + LP_FILL(25, 15)), 297 + LP_FILL(25, 15)), 238 + LP_FILL(25, 15)), 240 + LP_FILL(25, 15)), 266 + LP_FILL(25, 15)), 242 + LP_FILL(25, 15)), 183 + LP_FILL(27, 17)), 269 + LP_FILL(25, 16)), 253 + LP_FILL(27, 19)), 301 + LP_FILL(28, 20)), 230 + LP_FILL(25, 19)), 203 + LP_FILL(28, 19)), 221 + LP_FILL(27, 19)), 246 + LP_FILL(25, 18)), 234 + LP_FILL(25, 19)), 256 + LP_FILL(27, 19)), 216 + LP_FILL(25, 19)), 261 + LP_FILL(28, 19)), 214 + LP_FILL(25, 17)), 230 + LP_FILL(28, 19))
#define LP_PARTS_davy     LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(LP_PARTS_MAX(0, 315 + LP_FILL(25, 15)), 181 + LP_FILL(25, 15)), 192 + LP_FILL(27, 23)), 302 + LP_FILL(25, 17))

//How many parts the busiest level of the packs this build holds has. A pack that is left out
//counts for nothing. LP_PARTS_MAX is a function and not a macro on purpose: a macro naming
//its first argument twice doubles the text at every step, which with many packs puts the
//compiler out of memory. constexpr keeps it usable where a constant is wanted
static inline constexpr int LP_PARTS_MAX(int a, int b) { return (a > b) ? a : b; }
#define LP_PARTS_OF(p) (((LEVELPACKS & LP_##p) != 0) ? LP_PARTS_##p : 0)
#define LEVELPACKMAXPARTS LP_PARTS_MAX(LP_PARTS_MAX(0, LP_PARTS_OF(blockman)), LP_PARTS_OF(davy))
//<<<

//The pool has to take the busiest level of the packs this build ships
static_assert(MAXWORLDPARTS >= LEVELPACKMAXPARTS, "a level of a pack in this build would not fit the pool");
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
//AnimPhase and PrevDrawAnimPhase are four bits each in CWorldPart, so the highest phase a part
//can reach has to stay inside them. 4 is the most any part sets AnimPhases to
static_assert(AnimBaseRightJump + 4 - 1 <= 15, "an anim phase would not fit its four bits");

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
