#ifndef _UTIL_h
#define _UTIL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <Adafruit_Keypad.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define LCD_COLS 20
#define LCD_ROWS 4

#define MODE_TARGET 0
#define MODE_JOG	1
#define MODE_CONFIG 2

extern byte CornerTL[], CornerTR[], CornerBL[], CornerBR[], LineHT[], LineHB[], LineV[], LetG[];
extern bool bEditMode, bSetDenominator, bSetFenceDepthValue, bSetSpeedValue, bSetTargetValue, bSetThreadsPerInchValue;
extern int nBufferIndex, nKeypadBuffer, nPageMode, nSerialBuffer;
extern unsigned long nHoldTime;
extern float fFenceDepth, fSpeedValue ,fTargetValue, fThreadsPerInchValue;
extern char cValueBufferNumerator[8], cValueBufferDenominator[8];
extern Adafruit_Keypad keypad;
extern LiquidCrystal_I2C lcd;

void alignCenter(const char s[], uint8_t row);
void alignRight(const char s[], uint8_t row, uint8_t offset_x = 0);
void clearRow(uint8_t row);
void clearPartialRow(uint8_t x1, uint8_t x2, uint8_t row);
void customCharSetup();
void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void defaultMode();
void editMode();
void keypadHandler();
void loadEEPROM();
void showMenu();

uint8_t getLength(const char s[]);

#endif

