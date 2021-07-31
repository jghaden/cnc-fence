// util.h

#ifndef _util_h
#define _util_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Key.h>

// Used by showMenu() to know what page to print to the LCD
#define PAGE_TARGET   0
#define PAGE_CONFIG_0 1
#define PAGE_CONFIG_1 2
#define PAGE_ESTOP    3

// EEPROM addresses
///#define EEPROM_FENCE_DEPTH 0xC0
///#define EEPROM_SPEED       0xC1
///#define EEPROM_TPI         0xC4

#define EDIT_MODE_PRE  0
#define EDIT_MODE_CUR  1
#define EDIT_MODE_POST 2

// Define width and height of LCD
#define LCD_COLS 20
#define LCD_ROWS 4

#define KEY_HOLD_TIME  50 // ms
#define KEY_JOG_MINUS  8
#define KEY_JOG_PLUS   9
#define KEY_HOME       7
#define KEY_GO         6

#define FENCE_TPI    20
#define FENCE_DEPTH  48
#define FENCE_OFFSET 0.056

extern byte CornerTL[], CornerTR[], CornerBL[], CornerBR[], LineH[], LineV[], BlockPartial[], BlockFull[], LetG[];
extern volatile bool bConfigMode, bEditMode, bEStop, bGoTarget, bHome, bHomed, bHoming, bJogMinus, bJogPlus, bSerialParams, bSetFenceDepthValue, bSetSpeedValue, bSetSpeedMultValue, bSetStepsValue, bSetTargetValue, bSetThreadsPerInchValue, bTargetMode;
extern uint8_t nBufferIndex, nEditMode, nHoldKey, nKeypadBuffer, nKeypadBufferOld, nPageMode, nSerialBuffer, nSpeedValue, nSpeedMultValue, nWarningIndex;
extern int nStepsValue;
extern unsigned long nHoldTime, nTime, nLCDTime;
extern float fPositionValue, fFenceDepth, fTargetValue, fThreadsPerInchValue;
extern char cBuf[32], cSerialBuffer, cEditValueBuffer[32];
extern String sSerialBuffer;

extern Keypad keypad;
extern LiquidCrystal_I2C lcd;

void alignCenter(const char s[], uint8_t row);
void alignRight(const char s[], uint8_t row, uint8_t offset_x = 0);
void buttonHandler();
void clearRow(uint8_t row);
void clearRowPartial(uint8_t x1, uint8_t x2, uint8_t row);
void commandHandler();
void customCharacterSetup();
void defaultMode();
void drawWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void editMode(uint8_t nEditMode = EDIT_MODE_CUR);
void keypadHandler();
void showMenu();
void showWarning(uint8_t x, uint8_t y);
void updateSpeed();

int getIndex(const char s[], char delimeter);
uint8_t getLength(const char s[]);
float editModeParser();

String RemoveZeros(String s);

#endif
