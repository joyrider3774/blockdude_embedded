#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "commonvars.h"
#include "savestate.h"

// ===========================================================================
// Saved data, kept in the platform's save storage
//
// Everything is one record, stamped with a magic value and a version and ended
// by a CRC over the bytes before it. That is what tells a never written sector,
// an older layout or a half finished write apart from real data. Anything that
// does not check out is replaced by the defaults rather than trusted.
// ===========================================================================

#define SAVE_MAGIC        0x4442  //"BD"
#define SAVE_VERSION      1
#define STORE_SAVE_ADDR   0

typedef struct SaveRecord SaveRecord;
struct SaveRecord
{
	uint16_t magic;
	uint8_t  version;
	uint8_t  musicOn;
	uint8_t  soundOn;
	uint8_t  skin;
	uint8_t  inverted;
	uint8_t  editorShowGrid;
	uint8_t  editorShowPosition;
	uint8_t  fontScale;                   //in percent, 100 .. 150
	uint8_t  levelLocks[MaxLevelPacks];   //last unlocked level of every level pack
	uint16_t crc;                         //covers every byte before it
};

#define STORE_TOTAL (STORE_SAVE_ADDR + sizeof(SaveRecord))

//the record has to fit in what the platform stores
static_assert(STORE_TOTAL <= PLATFORM_STORAGE_SIZE, "the saved record does not fit in PLATFORM_STORAGE_SIZE");
//record addresses, lengths and the loops over them are uint8_t
static_assert(STORE_TOTAL <= 255, "store offsets do not fit in uint8_t");
//unlocked levels are kept as uint8_t
static_assert(InstalledLevelsDefaultGame <= 255, "unlocked levels do not fit in uint8_t");

uint8_t levelLocks[MaxLevelPacks];
uint8_t musicOn;
uint8_t soundOn;
uint8_t skin;
uint8_t inverted;
uint8_t editorshowposition;
uint8_t editorshowgrid;
float fontScale;

//CRC16 CCITT, small and more than enough to spot a corrupted record
static uint16_t SaveCrc(const uint8_t* data, uint8_t len)
{
	uint16_t crc = 0xFFFF;
	while (len--)
	{
		crc ^= (uint16_t)(*data++) << 8;
		for (uint8_t bit = 0; bit < 8; bit++)
			crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
	}
	return crc;
}

static void defaultSaveState(void)
{
	for (uint8_t i = 0; i < MaxLevelPacks; i++)
		levelLocks[i] = 1;
	musicOn = 1;
	soundOn = 1;
	skin = 0;
	inverted = 0;
	editorshowposition = 1;
	editorshowgrid = 1;
	fontScale = 1.0f;
}

//true when the storage held a record we can trust, the values are only taken over then
static bool loadSaveState(void)
{
	SaveRecord rec;
	uint8_t* bytes = (uint8_t*)&rec;
	Platform_StorageRead(STORE_SAVE_ADDR, bytes, sizeof(SaveRecord));

	if ((rec.magic != SAVE_MAGIC) || (rec.version != SAVE_VERSION) ||
		(rec.crc != SaveCrc(bytes, offsetof(SaveRecord, crc))))
		return false;

	musicOn = rec.musicOn;
	soundOn = rec.soundOn;
	skin = rec.skin;
	inverted = rec.inverted;
	editorshowgrid = rec.editorShowGrid;
	editorshowposition = rec.editorShowPosition;
	fontScale = rec.fontScale / 100.0f;
	for (uint8_t i = 0; i < MaxLevelPacks; i++)
		levelLocks[i] = rec.levelLocks[i];
	return true;
}

void saveSaveState(void)
{
	SaveRecord rec;
	//padding (if any) has to be the same every time, it is part of the crc
	memset(&rec, 0, sizeof(rec));
	rec.magic = SAVE_MAGIC;
	rec.version = SAVE_VERSION;
	rec.musicOn = (uint8_t)musicOn;
	rec.soundOn = (uint8_t)soundOn;
	rec.skin = (uint8_t)skin;
	rec.inverted = (uint8_t)inverted;
	rec.editorShowGrid = (uint8_t)editorshowgrid;
	rec.editorShowPosition = (uint8_t)editorshowposition;
	rec.fontScale = (uint8_t)(fontScale * 100.0f + 0.5f);
	for (uint8_t i = 0; i < MaxLevelPacks; i++)
		rec.levelLocks[i] = (uint8_t)levelLocks[i];
	rec.crc = SaveCrc((const uint8_t*)&rec, offsetof(SaveRecord, crc));

	//saving an unchanged record costs no write at all
	Platform_StorageWrite(STORE_SAVE_ADDR, (const uint8_t*)&rec, sizeof(SaveRecord));
}

//puts anything out of range back to its default, true if something had to change
static bool validateSaveState(void)
{
	bool changed = false;

	if (soundOn > 1)
	{
		changed = true;
		soundOn = 1;
	}

	if (musicOn > 1)
	{
		changed = true;
		musicOn = 1;
	}

	if (skin >= MAXSKINS)
	{
		changed = true;
		skin = 0;
	}

	if (inverted > 1)
	{
		changed = true;
		inverted = 0;
	}

	if (editorshowposition > 1)
	{
		changed = true;
		editorshowposition = 1;
	}

	if (editorshowgrid > 1)
	{
		changed = true;
		editorshowgrid = 1;
	}

	//no pack has more levels than the default game
	for (uint8_t i = 0; i < MaxLevelPacks; i++)
	{
		if ((levelLocks[i] < 1) || (levelLocks[i] > InstalledLevelsDefaultGame))
		{
			changed = true;
			levelLocks[i] = 1;
		}
	}

	if ((fontScale < 1.0f) || (fontScale > 1.5f))
	{
		changed = true;
		fontScale = 1.0f;
	}

	return changed;
}

void initSaveState(void)
{
	//never written, written by an older layout or damaged, start clean so the
	//next save has something valid to build on
	if (!loadSaveState())
	{
		defaultSaveState();
		saveSaveState();
	}
	else if (validateSaveState())
		saveSaveState();
}

//level progress is kept per level pack, this is the one currently selected
static uint8_t currentPack(void)
{
	if ((CurrentLevelPackIndex < 0) || (CurrentLevelPackIndex >= MaxLevelPacks))
		return 0;
	return CurrentLevelPackIndex;
}

void setSkinSaveState(uint8_t value)
{
	skin = value;
	saveSaveState();
}

uint8_t skinSaveState(void)
{
	return skin;
}

void setInvertedSaveState(uint8_t value)
{
	inverted = value;
	saveSaveState();
}

uint8_t isInvertedSaveState(void)
{
	return inverted;
}

void setFontScaleSaveState(float value)
{
	fontScale = value;
	saveSaveState();
}

float fontScaleSaveState(void)
{
	return fontScale;
}


void setShowGridSaveState(uint8_t value)
{
	editorshowgrid = value;
	saveSaveState();
}

uint8_t ShowGridSaveState(void)
{
	return editorshowgrid;
}

void setShowPositionSaveState(uint8_t value)
{
	editorshowposition = value;
	saveSaveState();
}

uint8_t ShowPositionSaveState(void)
{
	return editorshowposition;
}

void setMusicOnSaveState(uint8_t value)
{
	musicOn = value;
	saveSaveState();
}

uint8_t isMusicOnSaveState(void)
{
	return musicOn;
}

void setSoundOnSaveState(uint8_t value)
{
	soundOn = value;
	saveSaveState();
}

uint8_t isSoundOnSaveState(void)
{
	return soundOn;
}

void unlockLevel(uint8_t aLevel)
{
	uint8_t pack = currentPack();
	if ((aLevel > levelLocks[pack]) && (aLevel <= InstalledLevels) && (aLevel <= InstalledLevelsDefaultGame))
	{
		levelLocks[pack] = aLevel;
		saveSaveState();
	}
}

uint8_t levelUnlocked(uint8_t aLevel)
{
	return (lastUnlockedLevel() >= aLevel);
}

uint8_t lastUnlockedLevel()
{
	uint8_t result = levelLocks[currentPack()];
	//result = InstalledLevels;
	//never point past the end of the selected pack
	if ((InstalledLevels > 0) && (result > InstalledLevels))
		result = InstalledLevels;
	return result;
}
