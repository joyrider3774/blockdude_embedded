#ifndef game_h
#define game_h

#include <stdbool.h>
//for PLATFORM_FAST_CODE, which marks the calls that run once for every pixel
#include "Platform.h"
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size);

//1 for a tile type whose sheet covers every pixel of its tile, worked out when a skin is
//loaded, see RefreshOpaque in GameFuncs.cpp. A strip copies such a sprite straight in and
//leaves the background under it out
extern bool partOpaque[];
static inline bool PartOpaqueType(uint8_t type) { return partOpaque[type]; }
//draws a tile image, magenta pixels are left out
PLATFORM_FAST_CODE void DrawImageTransparent(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image);
//only exists with a screen buffer: draws an image into it, transparent skips magenta pixels
PLATFORM_FAST_CODE void DrawImageToBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, bool transparent);
PLATFORM_FAST_CODE //draws one frame of a sprite sheet, see CWorldPart_SpriteData
void DrawSpriteFrame(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, uint8_t frame);
void pushImageRLE(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* data);
uint8_t MaxLineLen(const char* Text);
bool LevelErrorsFound(uint8_t* ErrorType);
void PlayLevelIfNoErrorsFound();
void LoadSelectedLevel(void);
void AskQuestion(int8_t Id, const char* Msg);
bool AskQuestionUpdate(int8_t* Id, bool* Answer, bool MustBeAButton);
void FindLevels(void);
void FindLevelPacks(void);
void LoadFonts(void);
//1 while the skin in use keeps its pictures one bit a pixel, see onebitimage.h
extern bool skinImagesOneBit;
void LoadGraphics(void);
uint8_t CurrentSkin(void);
void UnLoadGraphics();
#endif