#ifndef _UTIL_h
#define _UTIL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <LiquidCrystal_I2C.h>

#define LCD_COLS 20
#define LCD_ROWS 4

#define MODE_TARGET 0
#define MODE_JOG	1
#define MODE_CONFIG 2

extern byte CornerTL[], CornerTR[], CornerBL[], CornerBR[], LineHT[], LineHB[], LineV[], LetG[];

void alignCenter(LiquidCrystal_I2C &lcd, const char s[], uint8_t row);
void alignRight(LiquidCrystal_I2C &lcd, const char s[], uint8_t row, uint8_t offset_x = 0);
void clearRow(LiquidCrystal_I2C &lcd, uint8_t row);
void clearPartialRow(LiquidCrystal_I2C &lcd, uint8_t x1, uint8_t x2, uint8_t row);
void customCharSetup(LiquidCrystal_I2C &lcd);
void drawBox(LiquidCrystal_I2C &lcd, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

uint8_t getLength(const char s[]);

#endif

