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
#include <Keypad.h>
#include <Key.h>
#include <eeprom.h>
#include <LiquidCrystal_I2C.h>
#include "util.h"
#include <Wire.h>

void setup()
{
	// Initialize serial port (9600-8-N-1)
	Serial.begin(9600);

	// Configure LCD with I2C address, cols, and rows
	lcd.init();
	lcd.backlight();
	lcd.clear();
	// Upload customer characters to LCD to draw borders properly
	customCharacterSetup();

	// Load config data (fence depth, TPI) out of reset
	loadEEPROM();

	// Initialize keypad
	keypad.setHoldTime(100);

	// Setup INT4 pin for E-Stop switch
	pinMode(2, INPUT);
	attachInterrupt(digitalPinToInterrupt(2), EStopISR, LOW);

	// Print menu to the LCD
	showMenu();
}

void loop()
{
	// Prints a '0' and a '1' in top left corner repeatedly as a visual cue
	// on the LCD that indicates wether the E-Stop switch is active or not
	lcd.home();
	lcd.print(0);
	lcd.home();
	lcd.print(1);

	// Continuously call to handle keypad input
	keypadHandler();
}
