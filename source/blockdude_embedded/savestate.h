#ifndef SAVESTATE_H
#define SAVESTATE_H

void initSaveState(void);

uint8_t lastUnlockedLevel();
uint8_t levelUnlocked(uint8_t aLevel);
uint8_t isMusicOnSaveState(void);
uint8_t isSoundOnSaveState(void);
uint8_t isInvertedSaveState(void);
uint8_t skinSaveState(void);
void unlockLevel(uint8_t aLevel);
void setMusicOnSaveState(uint8_t value);
void setSoundOnSaveState(uint8_t value);
void setSkinSaveState(uint8_t value);
void setInvertedSaveState(uint8_t value);
uint8_t ShowPositionSaveState(void);
void setShowPositionSaveState(uint8_t value);
uint8_t ShowGridSaveState(void);
void setShowGridSaveState(uint8_t value);
void setFontScaleSaveState(float value);
float fontScaleSaveState(void);
#endif