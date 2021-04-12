/**
  ******************************************************************************
  * @file    util.cpp
  * @author  Joshua Haden
  * @version V0.1.0
  * @date    26-MAR-2021
  * @brief   Manages functionality regarding the LCD and keypad
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#include "util.h"
#include "keypad_config.h"

byte CornerTL[] = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b00111,
	0b00100,
	0b00100,
	0b00100
};

byte CornerTR[] = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11100,
	0b00100,
	0b00100,
	0b00100
};

byte CornerBL[] = {
	0b00100,
	0b00100,
	0b00100,
	0b00111,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

byte CornerBR[] = {
	0b00100,
	0b00100,
	0b00100,
	0b11100,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

byte LineHB[] = {
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b00000,
	0b00000,
	0b00000,
	0b00000
};

byte LineHT[] = {
	0b00000,
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b00000,
	0b00000,
	0b00000
};

byte LineV[] = {
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100
};

// Custom lower case 'g' character, 'g' in the default LCD character set looks bad
byte LetG[] = {
	0b00000,
	0b00000,
	0b01111,
	0b10001,
	0b10001,
	0b01111,
	0b00001,
	0b01110
};

bool bEditMode = false;
volatile bool bEStop = false;
bool bSetDenominator = false;
bool bSetFenceDepthValue = false;
bool bSetSpeedValue = false;
bool bSetTargetValue = false;
bool bSetThreadsPerInchValue = false;

int nBufferIndex = 0;
int nHoldKey = 0;
int nKeypadBuffer = 0;
int nPageMode = PAGE_TARGET;
int nSerialBuffer = 0;

unsigned long nHoldTime = 0;

float fFenceDepth;
float fSpeedValue;
float fTargetValue = 0.0f;
float fThreadsPerInchValue;

char cValueBufferNumerator[8] = { 0 };
char cValueBufferDenominator[8] = { 0 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);

// Prints a string to the center of the LCD
void alignCenter(const char s[], uint8_t row)
{
	lcd.setCursor((LCD_COLS / 2) - getLength(s) + (getLength(s) / 2), row);
	lcd.print(s);
}

// Prints a string from the right side of the LCD
void alignRight(const char s[], uint8_t row, uint8_t offset_x = 0)
{
	lcd.setCursor(LCD_COLS - getLength(s) - offset_x, row);
	lcd.print(s);
}

// Clears row on the LCD without resetting the whole screen
void clearRow(uint8_t row)
{
	lcd.setCursor(1, row);

	int i = 0;
	
	while (i < LCD_COLS - 2)
	{
		lcd.write(' ');
		i++;
	}
}

// Clears a portion of a row on the LCD without updating the rest of the row
void clearRowPartial(uint8_t x1, uint8_t x2, uint8_t row)
{
	lcd.setCursor(x1, row);

	int i = x1;

	while (i < x2)
	{
		lcd.write(' ');
		i++;
	}
}

// Encodes custom characters to the first 8 characters of the LCD character set
void customCharacterSetup()
{
	lcd.createChar(0, CornerTL);
	lcd.createChar(1, CornerTR);
	lcd.createChar(2, CornerBL);
	lcd.createChar(3, CornerBR);
	lcd.createChar(4, LineHT);
	lcd.createChar(5, LineHB);
	lcd.createChar(6, LineV);
	lcd.createChar(7, LetG);
}

// Print a window at a specific point on the LCD with a width and height
void drawWindow(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	for (uint8_t _y = y; _y < h + y; _y++)
	{
		for (uint8_t _x = x; _x < w + x; _x++)
		{
			lcd.setCursor(_x, _y);

			if (_x == x && _y == y)
			{
				lcd.write(0);
			}
			else if (_x == w + x - 1 && _y == y)
			{
				lcd.write(1);
			}
			else if (_x == x && _y == h + y - 1)
			{
				lcd.write(2);
			}
			else if (_x == w + x - 1 && _y == h + y - 1)
			{
				lcd.write(3);
			}
			else if ((_x > x || _x < x) && _y == y)
			{
				lcd.write(4);
			}
			else if ((_x > x || _x < x) && _y == h + y - 1)
			{
				lcd.write(5);
			}
			else if ((_x == x || _x == w + x - 1) && (_y > y || _y < h + y - 1))
			{
				lcd.write(6);
			}
		}
	}
}

// Handle keypresses when not using special modes
void defaultMode()
{
	switch (nKeypadBuffer)
	{
		case '0': break;
		case '1': break;
		case '2': break;
		case '3': break;	
		case '4': break;
		case '5': break;
		case '6': break;
		case '7': break;
		case '8': break;
		case '9': break;
		case 'A':
			if (nPageMode == PAGE_TARGET)
			{				
				clearRow(1);
				lcd.setCursor(1, 1);
				lcd.print("Tar\7et: ");

				bSetTargetValue = true;				
				editMode(EDIT_MODE_PRE);
			}
			else if (nPageMode == PAGE_CONFIG)
			{
				clearRow(1);
				lcd.setCursor(3, 1);
				lcd.print("TPI: ");
				
				bSetThreadsPerInchValue = true;
				editMode(EDIT_MODE_PRE);
			}

			break;
		case 'B':
			if (nPageMode == PAGE_TARGET || nPageMode == PAGE_JOG)
			{
				clearRow(2);

				if (nPageMode == PAGE_TARGET)
				{
					lcd.setCursor(2, 2);
				}
				else
				{
					lcd.setCursor(1, 2);
				}

				lcd.print("Speed: ");

				bSetSpeedValue = true;
				editMode(EDIT_MODE_PRE);
			}
			else if (nPageMode == PAGE_CONFIG)
			{
				clearRow(2);
				lcd.setCursor(1, 2);
				lcd.print("Depth: ");

				bSetFenceDepthValue = true;
				editMode(EDIT_MODE_PRE);
			}

			break;
		case 'C':
			lcd.print("page");
			nPageMode++;

			if (nPageMode > PAGE_CONFIG)
			{
				nPageMode = 0;
			}

			showMenu();
			break;
		case 'D':
			if (fTargetValue > 0 && (nPageMode != PAGE_CONFIG || nPageMode != PAGE_SYSTEM))
			{
				fTargetValue = 0;

				showMenu();
			}

			break;
	}
}

// Handle keypresses while in edit mode differently than default mode
void editMode(uint8_t nEditMode = EDIT_MODE_CUR)
{
	switch (nEditMode)
	{
		case EDIT_MODE_PRE:
			lcd.blink_on();
			nBufferIndex = 0;
			bEditMode = true;

			memset(cValueBufferNumerator, 0, sizeof(cValueBufferNumerator));
			memset(cValueBufferDenominator, 0, sizeof(cValueBufferDenominator));
			cValueBufferDenominator[0] = '1';
			break;
		case EDIT_MODE_CUR:
			if (nBufferIndex < 5)
			{
				if ((nKeypadBuffer - 48) >= 0 && (nKeypadBuffer - 48 <= 9))
				{
					lcd.write(nKeypadBuffer);

					if (bSetDenominator)
					{
						cValueBufferDenominator[nBufferIndex++] = nKeypadBuffer;
					}
					else
					{
						cValueBufferNumerator[nBufferIndex++] = nKeypadBuffer;
					}
				}
				else if (nKeypadBuffer == '*')
				{
					lcd.write('.');

					if (bSetDenominator)
					{
						cValueBufferDenominator[nBufferIndex++] = '.';
					}
					else
					{
						cValueBufferNumerator[nBufferIndex++] = '.';
					}
				}
				else if (nKeypadBuffer == 'D')
				{
					lcd.write('/');

					nBufferIndex = 0;
					bSetDenominator = true;
				}
			}

			if (nKeypadBuffer == 'A')
			{
				if (bSetTargetValue)
				{
					bSetTargetValue = false;
					fTargetValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

					if (fTargetValue > fFenceDepth)
					{
						fTargetValue = fFenceDepth;
					}

					editMode(EDIT_MODE_POST);
				}
				else if (bSetThreadsPerInchValue)
				{
					bSetThreadsPerInchValue = false;
					fThreadsPerInchValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

					if (fThreadsPerInchValue > 80)
					{
						fThreadsPerInchValue = 80;
					}

					EEPROM.write(EEPROM_TPI, fThreadsPerInchValue);

					editMode(EDIT_MODE_POST);
				}
			}
			else if (nKeypadBuffer == 'B')
			{
				if (bSetFenceDepthValue)
				{
					bSetFenceDepthValue = false;
					fFenceDepth = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

					if (fFenceDepth > 500.0f)
					{
						fFenceDepth = 500.0f;
					}

					EEPROM.write(EEPROM_FENCE_DEPTH, fFenceDepth);

					editMode(EDIT_MODE_POST);
				}
				else if (bSetSpeedValue)
				{
					bSetSpeedValue = false;
					fSpeedValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

					if (fSpeedValue > 5.0f)
					{
						fSpeedValue = 5.0f;
					}
					EEPROM.write(EEPROM_SPEED, fSpeedValue);


					editMode(EDIT_MODE_POST);
				}
			}
			break;
		case EDIT_MODE_POST:
			bEditMode = false;
			bSetDenominator = false;
			showMenu();
			break;
	}
}

void EStopISR()
{
	cli();  // Stop all other interrupts (Default = on)

	if (!bEStop)
	{
		bEStop = true;
	}
}

void jog()
{
	digitalWrite(PUL1, HIGH);
	digitalWrite(PUL2, HIGH);
	delayMicroseconds(DELAY_US);
	digitalWrite(PUL1, LOW);
	digitalWrite(PUL2, LOW);
}

void jogMode(uint8_t dir)
{
	if (dir == JOG_MINUS)
	{
		fTargetValue -= (fSpeedValue / 8);

		if (fTargetValue < 0)
		{
			fTargetValue = 0;
		}

		clearRowPartial(11, 18, 1);
		lcd.setCursor(11, 1);
		lcd.print(fTargetValue, 3);
		lcd.write('\"');
	}
	else if (dir == JOG_PLUS)
	{
		fTargetValue += (fSpeedValue / 8);

		if (fTargetValue > fFenceDepth)
		{
			fTargetValue = fFenceDepth;
		}

		clearRowPartial(11, 18, 1);
		lcd.setCursor(11, 1);
		lcd.print(fTargetValue, 3);
		lcd.write('\"');
	}
}

void keypadHandler()
{
	nKeypadBuffer = keypad.getKey();

	if (nKeypadBuffer)
	{
		nHoldKey = nKeypadBuffer;
	}

	if (keypad.getState() == PRESSED)
	{
		if (bEditMode)
		{
			editMode();
		}
		else
		{
			defaultMode();
		}
	}
}

// Load config data from EEPROM (4 KB) to set fence depth and TPI out of reset
void loadEEPROM()
{
	fFenceDepth = EEPROM.read(EEPROM_FENCE_DEPTH);
	fSpeedValue = EEPROM.read(EEPROM_SPEED);
	fThreadsPerInchValue = EEPROM.read(EEPROM_TPI);

	if (isnan(fFenceDepth))
	{
		fFenceDepth = 30.0f;
	}

	if (isnan(fSpeedValue))
	{
		fSpeedValue = 1.0f;
	}

	if (isnan(fThreadsPerInchValue))
	{
		fThreadsPerInchValue = 24.0f;
	}
}

// Print menu to LCD based on current mode
void showMenu()
{
	lcd.blink_off();
	lcd.clear();

	if (nPageMode != PAGE_TARGET)
	{
		drawWindow(0, 0, LCD_COLS, LCD_ROWS);
	}

	switch (nPageMode)
	{
		case PAGE_TARGET:
			lcd.setCursor(1, 1);
			lcd.print("Tar\7et: ");
			lcd.print(fTargetValue, 3);
			lcd.print("\"");

			lcd.setCursor(2, 2);
			lcd.print("Speed: ");
			lcd.print(fSpeedValue, 3);
			lcd.print(" in/s");
			break;
		case PAGE_JOG:
			lcd.setCursor(1, 1);
			lcd.print("Position: ");
			lcd.print(fTargetValue, 3);
			lcd.print("\"");

			lcd.setCursor(1, 2);
			lcd.print("Speed: ");
			lcd.print(fSpeedValue, 3);
			lcd.print(" in/s");

			alignRight(" Jo\7 ", 3, 2);
			break;
		case PAGE_CONFIG:
			lcd.setCursor(3, 1);
			lcd.print("TPI: ");
			lcd.print(fThreadsPerInchValue, 3);

			lcd.setCursor(1, 2);
			lcd.print("Depth: ");
			lcd.print(fFenceDepth, 3);
			lcd.print("\"");

			alignRight(" Confi\7 ", 3, 2);
			break;
	}
}

// Return length of string to use for text alignment functions
uint8_t getLength(const char s[])
{
	size_t i = 0;

	while (s[i] != '\0')
	{
		i++;
	}

	return i;
}