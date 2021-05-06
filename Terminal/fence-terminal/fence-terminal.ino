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

void setup()
{
	pinMode(KEY_JOG_MINUS, INPUT);
	pinMode(KEY_JOG_PLUS, INPUT);
	pinMode(KEY_HOME, INPUT);

	Serial.begin(115200);
	Serial1.begin(115200);
	Serial1.setTimeout(25);

	// Configure LCD with I2C address, cols, and rows
	lcd.init();
	lcd.backlight();
	lcd.clear();
	// Upload customer characters to LCD to draw borders properly
	customCharacterSetup();

	// Initialize keypad
	keypad.setHoldTime(2000);

	// Print menu to the LCD
	showMenu();
}

void loop()
{
	commandHandler();

	if (!bEStop)
	{
		buttonHandler();

		if ((millis() - nLCDTime) > 300 && !bHomed && !bConfigMode)
		{
			showWarning(2, 0);
			alignCenter(" Needs Homin\7 ", 0);
			showWarning(17, 0);
		
			if (nWarningIndex++ > 1)
			{
				nWarningIndex = 0;
			}
		
			nLCDTime = millis();
		}
	}
	else if(bEStop && ((millis() - nLCDTime) > 300))
	{
		showWarning(4, 1);
		showWarning(5, 1);
		showWarning(13, 1);
		showWarning(14, 1);

		if (nWarningIndex++ > 1)
		{
			nWarningIndex = 0;
		}

		nLCDTime = millis();
	}
}
