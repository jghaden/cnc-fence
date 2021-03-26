#include "Adafruit_Keypad.h"
#include <eeprom.h>
#include <LiquidCrystal_I2C.h>
#include "util.h"
#include <Wire.h>

void setup()
{
	lcd.init();
	lcd.backlight();
	lcd.clear();
	customCharSetup();

	loadEEPROM();

	keypad.begin();

	showMenu();
}

void loop()
{
	keypad.tick();	

	keypadHandler();
}
