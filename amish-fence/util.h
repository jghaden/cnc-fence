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

extern byte CornerTL[], CornerTR[], CornerBL[], CornerBR[], LineHT[], LineHB[], LineV[], LetG[];

void centerText(LiquidCrystal_I2C &lcd, const char s[], uint8_t row);
void customCharSetup(LiquidCrystal_I2C &lcd);
void drawBox(LiquidCrystal_I2C &lcd, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

uint8_t getLength(const char s[]);

#endif

