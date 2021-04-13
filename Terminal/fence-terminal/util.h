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
#define PAGE_TARGET 0
#define PAGE_JOG	1
#define PAGE_CONFIG 2
#define PAGE_SYSTEM 3

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

#define KEY_HOLD 50
#define JOG_MINUS 8
#define JOG_PLUS  9
#define HOME      7

extern byte CornerTL[], CornerTR[], CornerBL[], CornerBR[], LineHT[], LineHB[], LineV[], LetG[];
extern bool bEditMode, bSetDenominator, bSetFenceDepthValue, bSetSpeedValue, bSetTargetValue, bSetThreadsPerInchValue;
extern volatile bool bEStop;
extern int nBufferIndex, nEditMode, nHoldKey, nKeypadBuffer, nPageMode, nSerialBuffer;
extern unsigned long nHoldTime;
extern float fFenceDepth, fSpeedValue, fTargetValue, fThreadsPerInchValue;
extern char cValueBufferNumerator[8], cValueBufferDenominator[8];

extern Keypad keypad;
extern LiquidCrystal_I2C lcd;

void alignCenter(const char s[], uint8_t row);
void alignRight(const char s[], uint8_t row, uint8_t offset_x = 0);
void clearRow(uint8_t row);
void clearRowPartial(uint8_t x1, uint8_t x2, uint8_t row);
void customCharacterSetup();
void defaultMode();
void drawWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void editMode(uint8_t nEditMode = EDIT_MODE_CUR);
void jogMode(uint8_t dir);
void keypadHandler();
void showMenu();

uint8_t getLength(const char s[]);

#endif
