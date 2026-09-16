#ifndef game_h
#define game_h

#include <stdbool.h>
void tftPrint(int16_t x, int16_t y, const char* str, uint16_t color, uint16_t bg, uint8_t size);
//draws a tile image, magenta pixels are left out
void DrawImageTransparent(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image);
//only exists with a screen buffer: draws an image into it, transparent skips magenta pixels
void DrawImageToBuffer(int16_t x, int16_t y, int16_t w, int16_t h, const uint8_t* image, bool transparent);
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
void LoadGraphics(void);
uint8_t CurrentSkin(void);
void UnLoadGraphics();
#endif