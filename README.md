# Blockdude Embedded Version
![DownloadCountTotal](https://img.shields.io/github/downloads/joyrider3774/blockdude_embedded/total?label=total%20downloads&style=plastic) ![DownloadCountLatest](https://img.shields.io/github/downloads/joyrider3774/blockdude_embedded/latest/total?style=plastic) ![LatestVersion](https://img.shields.io/github/v/tag/joyrider3774/blockdude_embedded?label=Latest%20version&style=plastic) ![License](https://img.shields.io/github/license/joyrider3774/blockdude_embedded?style=plastic)

Blokdude is a remake of the well known TI Caluculator game from Brandon Sterner as well as the blockman game from Soleau Software. 
This version of the game is based on the GP2X port of blockdude i made over a decade ago.

## Screenshots
The browser build, at twice the game's own 128x128:

| Title screen | In game |
| --- | --- |
| ![Blockdude title screen](metadata/screenshots/title.png) | ![Blockdude in game](metadata/screenshots/ingame.png) |

## Game Features:
- 21 levels from original blockman game by Soleau Software + 4 levels made by me, in 2 level packs
- 5 skins to choose from (the Gamebuino META build only has the default skin)
- Audio can be switched on or off
- Free view mode to look around levels that are bigger than the screen
- Autosaves progress per level pack, options and the chosen skin

## Playing the Game:
The aim of the game, in any level, is to move the player to the exit door. You will need to create a path to this exit door by picking up blocks and placing them on strategic locations so you can reach the exit door.
The player will automatically jump on higher level blocks or ground if you keep moving left or right in front of them. 
If the level is bigger than what the screen can display, press R to enter free view, this will allow you to pan around the level using the dpad. Press B or R again to go back to the player.

After you solved a level the next level of that level pack is unlocked.

## Controls 

| Button | Action |
| ------ | ------ |
| Dpad | Select menu's, options, level packs or levels to play. During gameplay move the player left or right, up picks up or drops a block. In free view pan around the level |
| A | Confirm in menu and level selector, pickup or drop a block in game mode |
| B | Back in options and level selector, ask to quit to the level selector in game, leave free view |
| L | Ask to restart the level |
| R | Enter or leave free view |
| (A) + Left + Down | Show or hide the debug info |

### Buttons
The game's buttons on every device:

| Device | D-pad | A | B | L | R |
| ------ | ----- | - | - | - | - |
| ESPboy | d-pad | ACT | ESC | LFT | RGT |
| Gamebuino META | d-pad | A | B | MENU | HOME |
| Adafruit PyBadge | d-pad | A | B | SELECT | START |
| Adafruit PyGamer | joystick | A | B | SELECT | START |
| Pimoroni PicoSystem | d-pad | A | B | Y | X |
| Pimoroni Explorer | A up, C down, B left, Y right | X | Z | BOOT + B | BOOT + Y |
| Pimoroni Tufty 2350 | UP up, DOWN down, A left, C right | B | HOME | HOME + A | HOME + C |
| TinyCircuits Thumby Color | d-pad | A | B | left bumper | right bumper |
| Playdate | d-pad | A | B | menu: restart | menu: free view |
| Libretro | d-pad | A | B | L | R |
| Game Boy Advance | d-pad | A | B | L | R |
| Nintendo DS | d-pad | A | B | L | R |
| Nintendo 3DS | d-pad or circle pad | A | B | L | R |
| Nintendo 64 | d-pad | A | B | L | R |
| PlayStation | d-pad | Cross | Circle | L1 | R1 |
| PlayStation Portable | d-pad or the analog stick | Cross | Circle | L | R |
| PlayStation Vita | d-pad or the left stick | Cross | Circle | L | R |
| Windows | arrow keys | X | C | S | D |
| MS-DOS | arrow keys | X | C | S | D |
| Browser | arrow keys | X | C | S | D |

On the Explorer BOOT is held as a shift, the direction pressed with it is not sent.

On the Tufty 2350 a tap of HOME is B when it is let go, held HOME is a shift like BOOT on the Explorer.

On the Gamebuino META holding HOME for a second goes back to its loader.

## Devices
Every [release](https://github.com/joyrider3774/blockdude_embedded/releases) has a build for every device. `releases/` is where a build of your own puts them, it is not part of the repository:

| Device | File | How to install |
| ------ | ---- | -------------- |
| [ESPboy](https://www.espboy.com/) | ESPboy_Blockdude.bin | flash it, the board is a LOLIN(WEMOS) D1 mini |
| [Gamebuino META](https://gamebuino.com/gamebuino-meta) | GamebuinoMeta_Blockdude.bin | copy it into a folder on the SD card, the .hex is for flashing it directly |
| [Adafruit PyBadge](https://www.adafruit.com/product/4200) | PyBadge_Blockdude.uf2 | double press reset and copy it onto the drive that appears |
| [Adafruit PyGamer](https://www.adafruit.com/product/4242) | PyGamer_Blockdude.uf2 | same as the PyBadge |
| [Pimoroni PicoSystem](https://shop.pimoroni.com/products/picosystem) | PicoSystem_Blockdude.uf2 | hold X while switching on and copy it onto the drive that appears |
| [Pimoroni Explorer](https://shop.pimoroni.com/products/explorer?variant=42092697845843) | Explorer_Blockdude.uf2 | hold BOOT while pressing RESET and copy it onto the drive that appears |
| [Pimoroni Tufty 2350](https://shop.pimoroni.com/products/tufty-2350?variant=55811986227579) | Tufty_Blockdude.uf2 | hold HOME while pressing RESET and copy it onto the drive that appears |
| [TinyCircuits Thumby Color](https://tinycircuits.com/products/thumby-color) | ThumbyColor_Blockdude.uf2 | put it into bootloader mode and copy it onto the RPI-RP2 drive that appears |
| [Playdate](https://play.date/) | Playdate_Blockdude.pdx.zip | unzip it and sideload Blockdude.pdx, the same pdx runs in the Playdate simulator |
| [Libretro / RetroArch](https://www.retroarch.com/) | Libretro_Blockdude.zip | copy blockdude_libretro.dll into RetroArch's cores folder and blockdude_libretro.info into its info folder, then Load Core and Start Core |
| [Game Boy Advance](https://en.wikipedia.org/wiki/Game_Boy_Advance) | GBA_Blockdude.gba | put it on a flash cart or open it in an emulator, the progress is saved in the cartridge's SRAM |
| [Nintendo DS](https://en.wikipedia.org/wiki/Nintendo_DS) | NDS_Blockdude.nds | put it on a flash card or open it in an emulator, the progress is saved next to it in Blockdude.sav |
| [Nintendo 3DS](https://en.wikipedia.org/wiki/Nintendo_3DS) | 3DS_Blockdude.3dsx | copy it into /3ds/ on the SD card and start it from the Homebrew Launcher, or open it in an emulator, the progress is saved in sdmc:/3ds/Blockdude/ |
| [Nintendo 64](https://en.wikipedia.org/wiki/Nintendo_64) | N64_Blockdude.z64 | put it on a flash cart or open it in an emulator, the progress is saved in the cartridge EEPROM |
| [PlayStation](https://en.wikipedia.org/wiki/PlayStation_(console)) | PSX_Blockdude.exe | open it in an emulator or send it to a console that runs unsigned code, the progress is not saved yet |
| [PlayStation Portable](https://en.wikipedia.org/wiki/PlayStation_Portable) | PSP_Blockdude.PBP | rename it to EBOOT.PBP and put it in ms0:/PSP/GAME/Blockdude/ on the memory stick, or open it in PPSSPP |
| [PlayStation Vita](https://en.wikipedia.org/wiki/PlayStation_Vita) | Vita_Blockdude.vpk | install it with VitaShell on a Vita with homebrew enabled, or open it in Vita3K |
| Windows | Windows_Blockdude.exe | runs on its own, the progress is saved next to it in Blockdude.sav |
| MS-DOS | DOS_Blockdude.zip | unzip BLOCKDUD.EXE onto a DOS machine or into DOSBox and run it, the progress is saved next to it in BLOCKDUD.SAV |
| Browser | Web_Blockdude.zip | upload it to an itch.io HTML project, or unzip it and open index.html from a web server, the progress is saved in the browser |

The Tufty 2350 has no speaker, the game is silent there. Holding RESET until the rear LEDs are dark puts it to sleep, a front button wakes it up again, with UP and DOWN held as well it goes into shipping mode instead.

The Thumby Color's display is 128x128, the game's own size, so it is shown 1:1 over the whole screen. That build has not been tried on the device itself yet.

The Playdate shows the black & white skin, scaled up in the middle of its display. It has no side buttons, L and R are the restart and free view entries of its system menu.

The Game Boy Advance shows the game scaled to 160x160 in the middle of its screen, with black bars at the sides. Scrolling around a level that is bigger than the screen is slow there: the game engine draws the whole screen again while the view moves.

On the Nintendo DS the game is scaled to 192x192 in the middle of the top screen, with black bars at the sides, and the bottom screen stays dark. What the game saves goes into Blockdude.sav on the card it was started from, so a card that libfat can not write to (or an emulator without one) plays the game but forgets it afterwards. Its tones are square waves played as a sample: the DS's own tone channels count their frequency in a 16 bit timer and can not go below about 256 Hz.

On the Nintendo 3DS the game is scaled to 240x240 in the middle of the top screen, with black bars at the sides, and the bottom screen stays dark. What the game saves goes into sdmc:/3ds/Blockdude/Blockdude.sav. Its tones play through the console's DSP when the DSP firmware has been dumped to the SD card (sdmc:/3ds/dspfirm.cdc), and through CSND when it has not: on hardware either one plays, in an emulator only the DSP one does.

On MS-DOS the game runs in VGA mode X, 320x240 in 256 colours, blown up to 240x240 in the middle of the screen with black bars at the sides. That mode rather than the usual 320x200 one because its pixels are square, where 320x200 is stretched over the same screen and would show the game a fifth too tall. The 256 colours are set to the RGB332 cube, which is exactly what the game's 8 bpp screen buffer holds, so a frame reaches the card without a colour being worked out. Its tones are a square wave on the PC speaker, the progress is saved next to the program in BLOCKDUD.SAV, and Escape quits. The program is 32 bit and carries the CWSDPMI host inside it, so it needs nothing beside it on the disk.

In a browser the game is drawn into a canvas of its own 128x128 pixels, which the page stretches to whatever room it is given while keeping it square and keeping the pixels sharp. The progress is saved in the browser's localStorage under the game's name, so a private window plays it but forgets it afterwards. The zip holds index.html, index.js and index.wasm and is what an itch.io HTML project takes as it is.

On the Nintendo 64 the game is drawn into memory in the colours the RDP takes and the RDP shows it scaled to 240x240 in the middle of its 320x240 screen, with black bars at the sides. Its tones are a square wave written into the buffers the sound hardware plays from. The progress is saved in the cartridge EEPROM, which the ROM says it has, so a cartridge or an emulator without one plays the game but forgets it afterwards.

On the PlayStation the game is drawn into memory in the colours the GPU takes, handed to it as a texture and shown scaled to 240x240 in the middle of its 320x240 screen, with black bars at the sides. Its tones are a square wave the SPU plays from a single looping block. The memory card is not written yet, so what the game saves is gone when the console is switched off.

On the PlayStation Portable the game is doubled to 256x256 in the middle of the display, and the high scores are saved next to the EBOOT.PBP in Blockdude.sav.

On the PlayStation Vita the game is blown up four times to 512x512 in the middle of the display, and the high scores are saved in ux0:data/Blockdude/Blockdude.sav.

`python tools/build_releases.py` builds all of them  
`python tools/convert_skins.py` turns the images in `assets/skins` and `assets/skins2` into the headers the game includes  
`python tools/convert_levels.py` does the same for the levels in `assets/levels`  
The Playdate build also needs the Playdate SDK, see `platforms/playdate/CMakeLists.txt`  
The libretro core needs libretro-common, see `platforms/libretro/CMakeLists.txt`  
The Game Boy Advance build needs devkitARM and libgba, see `platforms/gba/CMakeLists.txt`  
The Nintendo DS build needs devkitARM, libnds and calico, see `platforms/nds/CMakeLists.txt`  
The Nintendo 3DS build needs devkitARM and libctru, see `platforms/3ds/CMakeLists.txt`  
The PlayStation build needs PSn00bSDK, see `platforms/psx/CMakeLists.txt`  
The Nintendo 64 build needs the mips64-elf toolchain and libdragon, see `platforms/n64/CMakeLists.txt`  
The PSP build needs the pspdev toolchain, see `platforms/psp/CMakeLists.txt` (pspdev has no Windows build, so on Windows it is built from WSL)  
The Vita build needs VitaSDK, see `platforms/vita/CMakeLists.txt`  
The browser build needs Emscripten, see `platforms/web/CMakeLists.txt`  
The MS-DOS build needs DJGPP, see `platforms/dos/CMakeLists.txt`


## Credits
The Blockdude game idea was created by Soleau Software for the dos blockman game and Brandon Sterner for the Ti Calculator based game. I don't know who invented the idea first.

The Game uses 21 levels from the Soleau Software blockman dos game. At The time i had created the gp2x version i had gotten approval to use these levels

The game has 5 skins using tilesets created by different people / games
- Default Skin: Based on a Remake of the flatskin made for the gp2x version by Fusion Power
- Tech Skin: Based on a skin made for the gp2x version by fusion power
- Ti Skin: Based on the assets from the Ti calculator game by brandon Sterner 
- Kenney skin: Based on the 1 bit platformer pack https://kenney-assets.itch.io/1-bit-platformer-pack - https://www.kenney.nl
- Flat Skin: Based on the flash game created by Klas Kroon and Kris Katiesen, i had gotten permission at the time of creating the gp2x version to use these assets
