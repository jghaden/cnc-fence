#include "util.h"
#include "keypad_config.h"

byte CornerTL[] = {
	0b00000,
	0b00000,
	0b00000,
	0b00111,
	0b00100,
	0b00100,
	0b00100,
	0b00100
};

byte CornerTR[] = {
	0b00000,
	0b00000,
	0b00000,
	0b11100,
	0b00100,
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

byte LineH[] = {
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b00000,
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

byte BlockPartial[] = {
	0b00000,
	0b00000,
	0b00000,
	0b11111,
	0b11111,
	0b00000,
	0b00000,
	0b00000
};

byte BlockFull[] = {
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111
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
bool bSetDenominator = false;
bool bSetFenceDepthValue = false;
bool bSetSpeedValue = false;
bool bSetTargetValue = false;
bool bSetThreadsPerInchValue = false;

volatile bool bEStop = false;
volatile bool bHomed = false;
volatile bool bJogMinus = false;
volatile bool bJogPlus = false;
volatile bool bGoTarget = false;

char cSerialBuffer;

uint8_t nBufferIndex = 0;
uint8_t nHoldKey = 0;
uint8_t nKeypadBuffer = 0;
uint8_t nPageMode = PAGE_TARGET;
uint8_t nSerialBuffer = 0;
uint8_t nSpeedValue = 1;
uint8_t nWarningIndex = 0;

float fFenceDepth = FENCE_DEPTH;
float fTargetValue = 0.0f;
float fThreadsPerInchValue = FENCE_TPI;

unsigned long nTime = 0;
unsigned long nLCDTime = 0;
unsigned long nHoldTime = 0;

char cValueBufferNumerator[8] = { 0 };
char cValueBufferDenominator[8] = { 0 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);

void(*reset)(void) = 0;

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

void buttonHandler()
{
	if ((millis() - nTime) > KEY_HOLD_TIME)
	{
		//
		// button hold down
		//
		if (digitalRead(KEY_HOME) == LOW)
		{
			Serial1.print('H');
			bHomed = false;
			showMenu();
		}

		if (bHomed)
		{
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
			else if (digitalRead(KEY_GO) == LOW && !bGoTarget)
			{
				bGoTarget = true;

				char cBuf[32] = { 'G', ':' };

				strcat(cBuf, String(fTargetValue, 3).c_str());

				Serial1.print(cBuf);
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
			else if (digitalRead(KEY_GO) == HIGH && bGoTarget)
			{
				bGoTarget = false;
			}

			// Continuously call to handle keypad input
			keypadHandler();
		}
		
		nTime = millis();
	}
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

void commandHandler()
{
	while (Serial1.available() > 0)
	{
		cSerialBuffer = Serial1.read();
		///Serial.write(cSerialBuffer);

		if (cSerialBuffer == 'R')
		{
			bEStop = false;
			reset();
		}

		if (cSerialBuffer == 'E')
		{
			bEStop = true;
			nPageMode = PAGE_ESTOP;
			showMenu();
		}
		else if (cSerialBuffer == 'H')
		{
			bHomed = true;
			nPageMode = PAGE_TARGET;
			showMenu();
		}
	}
}

// Encodes custom characters to the first 8 characters of the LCD character set
void customCharacterSetup()
{
	lcd.createChar(0, CornerTL);
	lcd.createChar(1, CornerTR);
	lcd.createChar(2, CornerBL);
	lcd.createChar(3, CornerBR);
	lcd.createChar(4, LineV);
	lcd.createChar(5, BlockPartial);
	lcd.createChar(6, BlockFull);
	lcd.createChar(7, LetG);
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
			break;
		case 'B':
			if (nPageMode == PAGE_TARGET)
			{
				clearRow(1);
				lcd.setCursor(0, 1);
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
		case 'C':
			if (nPageMode == PAGE_TARGET)
			{
				clearRowPartial(8, 15, 2);

				lcd.setCursor(1, 2);

				lcd.print("Speed: ");

				if (++nSpeedValue > 5)
				{
					nSpeedValue = 1;
				}

				for (size_t i = 0; i < 5; i++)
				{
					if (i < nSpeedValue)
					{
						lcd.write(6);
					}
					else
					{
						lcd.write(5);
					}
				}

				Serial1.print('S');
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
		case 'D':
			nPageMode++;
			showMenu();
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
		if (nBufferIndex < 6)
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
			if (nKeypadBuffer == 'A')
			{
				lcd.write('+');
			}
			else if (nKeypadBuffer == 'D')
			{
				lcd.write('/');

				nBufferIndex = 0;
				bSetDenominator = true;
			}
		}

		if (nKeypadBuffer == '#')
		{
			if (bSetTargetValue)
			{
				bSetTargetValue = false;
				fTargetValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

				if (fTargetValue > fFenceDepth)
				{
					fTargetValue = fFenceDepth;
				}
			}
			else if (bSetThreadsPerInchValue)
			{
				bSetThreadsPerInchValue = false;
				fThreadsPerInchValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

				if (fThreadsPerInchValue > 20)
				{
					fThreadsPerInchValue = 20;
				}

				///EEPROM.write(EEPROM_TPI, fThreadsPerInchValue);
			}
			else if (bSetFenceDepthValue)
			{
				bSetFenceDepthValue = false;
				fFenceDepth = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

				if (fFenceDepth > 240.0f)
				{
					fFenceDepth = 240.0f;
				}

				///EEPROM.write(EEPROM_FENCE_DEPTH, fFenceDepth);
			}

			editMode(EDIT_MODE_POST);
		}
		break;
	case EDIT_MODE_POST:
		bEditMode = false;
		bSetDenominator = false;
		showMenu();
		break;
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
				lcd.write('-');
			}
			else if ((_x > x || _x < x) && _y == h + y - 1)
			{
				lcd.write('-');
			}
			else if ((_x == x || _x == w + x - 1) && (_y > y || _y < h + y - 1))
			{
				lcd.write(4);
			}
		}
	}
}

// Print menu to LCD based on current mode
void showMenu()
{
	lcd.blink_off();
	lcd.clear();

	if (!bHomed)
	{
		alignCenter(" Needs Homin\7 ", 0);
	}

	if (nPageMode != PAGE_TARGET)
	{
		drawWindow(0, 0, LCD_COLS, LCD_ROWS);
	}

	switch (nPageMode)
	{
		case PAGE_TARGET:
			lcd.setCursor(0, 1);
			lcd.print("Tar\7et: ");
			lcd.print(fTargetValue, 3);
			lcd.print("\"");

			lcd.setCursor(1, 2);
			lcd.print("Speed: ");
			
			for (size_t i = 0; i < 5; i++)
			{
				if (i < nSpeedValue)
				{
					lcd.write(6);
				}
				else
				{
					lcd.write(5);
				}
			}
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
		case PAGE_ESTOP:
			alignCenter("! E-Stop !", 1);
			alignCenter("Will reset", 2);
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

// Prints an alternating loading symbol on the LCD
void warning(uint8_t x, uint8_t y)
{
	lcd.setCursor(x, y);

	switch (nWarningIndex)
	{
	case 0:
		lcd.write(' ');
		break;
	case 1:
		lcd.write('!');
		break;
	}
}