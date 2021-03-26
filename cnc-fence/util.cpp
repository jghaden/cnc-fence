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

int nBufferIndex = 0;
int nKeypadBuffer = 0;
int nPageMode = MODE_TARGET;
int nSerialBuffer = 0;

unsigned long nHoldTime = 0;

float fFenceDepth;
float fSpeedValue = 1.25f;
float fTargetValue = 0.0f;
float fThreadsPerInchValue;

char cValueBufferNumerator[8] = { 0 };
char cValueBufferDenominator[8] = { 0 };

Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);

void alignCenter(const char s[], uint8_t row)
{
	lcd.setCursor((LCD_COLS / 2) - getLength(s) + (getLength(s) / 2), row);
	lcd.print(s);
}

void alignRight(const char s[], uint8_t row, uint8_t offset_x = 0)
{
	lcd.setCursor(LCD_COLS - getLength(s) - offset_x, row);
	lcd.print(s);
}

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

void clearPartialRow(uint8_t x1, uint8_t x2, uint8_t row)
{
	lcd.setCursor(x1, row);

	int i = x1;

	while (i < x2)
	{
		lcd.write(' ');
		i++;
	}
}

void customCharSetup()
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

void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
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

void editMode()
{
	if (nBufferIndex == 0 && !bSetDenominator)
	{
		memset(cValueBufferNumerator, 0, sizeof(cValueBufferNumerator));
		memset(cValueBufferDenominator, 0, sizeof(cValueBufferDenominator));
		cValueBufferDenominator[0] = '1';
	}

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

			bEditMode = false;
			bSetDenominator = false;
			showMenu();
		}
		else if (bSetThreadsPerInchValue)
		{
			bSetThreadsPerInchValue = false;
			fThreadsPerInchValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

			if (fThreadsPerInchValue > 80)
			{
				fThreadsPerInchValue = 80;
			}

			EEPROM.write(0xC4, fThreadsPerInchValue);

			bEditMode = false;
			bSetDenominator = false;
			showMenu();
		}
	}
	else if (nKeypadBuffer == 'B')
	{
		if (bSetFenceDepthValue)
		{
			bSetFenceDepthValue = false;
			fFenceDepth = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

			if (fFenceDepth > 300.0f)
			{
				fFenceDepth = 300.0f;
			}

			EEPROM.write(0xC0, fFenceDepth);

			bEditMode = false;
			bSetDenominator = false;
			showMenu();
		}
		else if (bSetSpeedValue)
		{
			bSetSpeedValue = false;
			fSpeedValue = atof(cValueBufferNumerator) / atof(cValueBufferDenominator);

			if (fSpeedValue > 3.0f)
			{
				fSpeedValue = 3.0f;
			}

			bEditMode = false;
			bSetDenominator = false;
			showMenu();
		}
	}
}

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
			if (nPageMode == MODE_TARGET)
			{
				clearRow(1);
				lcd.setCursor(1, 1);
				lcd.print("Tar\7et: ");
				lcd.setCursor(9, 1);

				nBufferIndex = 0;
				bEditMode = true;
				bSetTargetValue = true;
			}
			else if (nPageMode == MODE_CONFIG)
			{
				clearRow(1);
				lcd.setCursor(1, 1);
				lcd.print("TPI: ");
				lcd.setCursor(6, 1);

				nBufferIndex = 0;
				bEditMode = true;
				bSetThreadsPerInchValue = true;
			}

			break;
		case 'B':
			if (nPageMode == MODE_TARGET || nPageMode == MODE_JOG)
			{
				clearRow(2);
				lcd.setCursor(1, 2);
				lcd.print("Speed: ");
				lcd.setCursor(8, 2);

				nBufferIndex = 0;
				bEditMode = true;
				bSetSpeedValue = true;
			}
			else if (nPageMode == MODE_CONFIG)
			{
				clearRow(2);
				lcd.setCursor(1, 2);
				lcd.print("Depth: ");
				lcd.setCursor(8, 2);

				nBufferIndex = 0;
				bEditMode = true;
				bSetFenceDepthValue = true;
			}

			break;
		case 'C':
			nPageMode++;

			if (nPageMode > MODE_CONFIG)
			{
				nPageMode = 0;
			}

			showMenu();
			break;
		case 'D': break;
		case '*':
			if (nPageMode == MODE_JOG)
			{
				fTargetValue -= (fSpeedValue / 60);

				if (fTargetValue < 0)
				{
					fTargetValue = 0;
				}

				clearPartialRow(11, 18, 1);
				lcd.setCursor(11, 1);
				lcd.print(fTargetValue, 3);
				lcd.write('\"');
			}
			break;
		case '#':
			if (nPageMode == MODE_JOG)
			{
				fTargetValue += (fSpeedValue / 60);

				if (fTargetValue > fFenceDepth)
				{
					fTargetValue = fFenceDepth;
				}

				clearPartialRow(11, 18, 1);
				lcd.setCursor(11, 1);
				lcd.print(fTargetValue, 3);
				lcd.write('\"');
			}
			break;
	}
}

void keypadHandler()
{
	while (keypad.available())
	{
		keypadEvent e = keypad.read();
		nKeypadBuffer = e.bit.KEY;

		if (e.bit.EVENT == KEY_JUST_PRESSED && (millis() - nHoldTime > 200))
		{
			if (bEditMode)
			{
				editMode();
			}
			else
			{
				defaultMode();
			}

			nHoldTime = millis();
		}
		else if (e.bit.EVENT == KEY_JUST_RELEASED)
		{
			//Serial.println(" up");
		}
	}
}

void loadEEPROM()
{
	fFenceDepth = EEPROM.read(0xC0);
	fThreadsPerInchValue = EEPROM.read(0xC4);

	if (isnan(fFenceDepth))
	{
		fFenceDepth = 30.0f;
	}

	if (isnan(fThreadsPerInchValue))
	{
		fThreadsPerInchValue = 20.0f;
	}
}

void showMenu()
{
	lcd.clear();

	if (nPageMode == MODE_TARGET)
	{
		lcd.setCursor(1, 1);
		lcd.print("Tar\7et: ");
		lcd.print(fTargetValue, 3);
		lcd.print("\"");

		lcd.setCursor(1, 2);
		lcd.print(" Speed: ");
		lcd.print(fSpeedValue, 3);
		lcd.print(" in/s");
	}
	else if (nPageMode == MODE_JOG)
	{
		drawBox(0, 0, LCD_COLS, LCD_ROWS);
		lcd.setCursor(1, 1);
		lcd.print("Position: ");
		lcd.print(fTargetValue, 3);
		lcd.print("\"");

		lcd.setCursor(1, 2);
		lcd.print("Speed: ");
		lcd.print(fSpeedValue, 3);
		lcd.print(" in/s");

		alignRight(" Jo\7 mode ", 3, 2);
	}
	else if (nPageMode == MODE_CONFIG)
	{
		drawBox(0, 0, LCD_COLS, LCD_ROWS);
		lcd.setCursor(1, 1);
		lcd.print("  TPI: ");
		lcd.print(fThreadsPerInchValue, 3);

		lcd.setCursor(1, 2);
		lcd.print("Depth: ");
		lcd.print(fFenceDepth, 3);
		lcd.print("\"");

		/*lcd.setCursor(2, 0);
		lcd.print(" Locked ");*/
		alignRight(" Confi\7 mode ", 3, 2);
	}
}

uint8_t getLength(const char s[])
{
	size_t i = 0;

	while (s[i] != '\0')
	{
		i++;
	}

	return i;
}