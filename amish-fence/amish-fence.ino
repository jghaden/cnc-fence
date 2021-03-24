#include "util.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Keypad.h"

#define KEYPAD_PID3844
#define R1    38
#define R2    40
#define R3    42
#define R4    44
#define C1    30
#define C2    34
#define C3    32
#define C4    36
// leave this import after the above configuration
#include "keypad_config.h"

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int serialBuffer = 0;

void setup()
{
	Serial.begin(9600);

	lcd.init();
	lcd.backlight();
	lcd.clear();

	customCharSetup(lcd);

	keypad.begin();

	lcd.setCursor(1, 1);
	lcd.print("Tar");
	lcd.write(7);
	lcd.print("et: 6.500\"");
	lcd.setCursor(1, 2);
	lcd.print("Speed: 200 RPM");

	//drawBox(lcd, 0, 3, LCD_COLS, 2);			// inverted box on top and bottom row
	//drawBox(lcd, 0, 0, LCD_COLS , LCD_ROWS);	// full screen box
	drawBox(lcd, 17, 1, 3, 3);
}

void loop()
{
	keypad.tick();

	if (Serial.available() > 0)
	{
		serialBuffer = Serial.read();

		lcd.setCursor(18, 2);
		lcd.write(serialBuffer);
		Serial.println(serialBuffer);

		if (serialBuffer == '~')
		{
			lcd.clear();
			drawBox(lcd, 0, 0, LCD_COLS, LCD_ROWS);	// full screen box
			centerText(lcd, "debug", 1);
			centerText(lcd, "mode", 2);
		}
	}

	while (keypad.available())
	{
		lcd.setCursor(18, 2);

		keypadEvent e = keypad.read();
		Serial.print(e.bit.KEY);

		if (e.bit.EVENT == KEY_JUST_PRESSED)
		{
			Serial.println(" down");
			lcd.write(e.bit.KEY);
		}
		else if (e.bit.EVENT == KEY_JUST_RELEASED)
		{
			Serial.println(" up");
			lcd.print(" ");
		}
	}
}