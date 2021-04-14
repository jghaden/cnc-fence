/**
  ******************************************************************************
  * @file    fence-terminal.ino
  * @author  Joshua Haden
  * @version V0.1.0
  * @date    12-APR-2021
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#include <Keypad.h>
#include <Key.h>
#include <LiquidCrystal_I2C.h>
#include "util.h"
#include <Wire.h>

void(*reset)(void) = 0;

volatile bool bJogMinus = false;
volatile bool bJogPlus = false;

char cSerialBuffer;
unsigned long nTime = 0;
unsigned long nLCDTime = 0;

void setup()
{
	pinMode(KEY_JOG_MINUS, INPUT);
	pinMode(KEY_JOG_PLUS, INPUT);
	pinMode(KEY_HOME, INPUT);

	///Serial.begin(115200);
	Serial1.begin(115200);

	// Configure LCD with I2C address, cols, and rows
	lcd.init();
	lcd.backlight();
	lcd.clear();
	// Upload customer characters to LCD to draw borders properly
	customCharacterSetup();

	// Initialize keypad
	keypad.setHoldTime(100);

	// Print menu to the LCD
	showMenu();
}

void loop()
{
	while (Serial1.available() > 0)
	{
		cSerialBuffer = Serial1.read();
		///Serial.print(cSerialBuffer);

		if (cSerialBuffer == 'R')
		{
			reset();
		}
	}

	if (millis() - nTime > KEY_HOLD_TIME)
	{
		//
		// button hold down
		//
		if (digitalRead(KEY_HOME) == LOW)
		{
			Serial1.print('H');
		}

		if (digitalRead(KEY_JOG_MINUS) == LOW && !bJogMinus)
		{
			bJogMinus = true;
			Serial1.print('X');
		}
		else if (digitalRead(KEY_JOG_PLUS) == LOW && !bJogPlus)
		{
			bJogPlus = true;
			Serial1.print('Y');
		}

		//
		// button release
		//
		if (digitalRead(KEY_JOG_MINUS) == HIGH && bJogMinus)
		{
			bJogMinus = false;
			Serial1.print('x');
		}
		else if (digitalRead(KEY_JOG_PLUS) == HIGH && bJogPlus)
		{
			bJogPlus = false;
			Serial1.print('y');
		}

		// Continuously call to handle keypad input
		keypadHandler();

		nTime = millis();
	}

	if (millis() - nLCDTime > 300)
	{
		warning(2, 0);
		warning(17, 0);

		if (nWarningIndex++ > 1)
		{
			nWarningIndex = 0;
		}

		nLCDTime = millis();
	}
}