/**
  ******************************************************************************
  * @file    cnc-fence.ino
  * @author  Joshua Haden
  * @version V0.1.0
  * @date    26-MAR-2021
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#include "Adafruit_Keypad.h"
#include <eeprom.h>
#include <LiquidCrystal_I2C.h>
#include "util.h"
#include <Wire.h>

void setup()
{
	// Configure LCD with I2C address, cols, and rows
	lcd.init();
	lcd.backlight();
	lcd.clear();
	// Upload customer characters to LCD to draw borders properly
	customCharacterSetup();

	// Load config data (fence depth, TPI) out of reset
	loadEEPROM();

	// Initialize keypad
	keypad.begin();

	// Print menu to the LCD
	showMenu();
}

void loop()
{
	keypad.tick();	

	keypadHandler();
}
