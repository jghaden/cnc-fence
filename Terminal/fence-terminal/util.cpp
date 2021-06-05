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

volatile bool bConfigMode             = false;
volatile bool bEditMode               = false;
volatile bool bEStop                  = false;
volatile bool bGoTarget               = false;
volatile bool bHome                   = false;
volatile bool bHomed                  = false;
volatile bool bHoming                = false;
volatile bool bJogMinus               = false;
volatile bool bJogPlus                = false;
volatile bool bSerialParams           = false;
volatile bool bSetDenominator         = false;
volatile bool bSetFenceDepthValue     = false;
volatile bool bSetSummation           = false;
volatile bool bSetSpeedValue          = false;
volatile bool bSetSpeedMultValue      = false;
volatile bool bSetStepsValue          = false;
volatile bool bSetTargetValue         = false;
volatile bool bSetThreadsPerInchValue = false;
volatile bool bTargetMode             = false;

char cBuf[32];
char cSerialBuffer;

uint8_t nBufferIndex     = 0;
uint8_t nHoldKey         = 0;
uint8_t nKeypadBuffer    = 0;
uint8_t nKeypadBufferOld = 0;
uint8_t nPageMode        = PAGE_TARGET;
uint8_t nSerialBuffer    = 0;
uint8_t nSpeedValue      = 1;
uint8_t nSpeedMultValue  = 1;
uint8_t nWarningIndex    = 0;

int nStepsValue = 800;

float fPositionValue       = 0.0f;
float fFenceDepth          = FENCE_DEPTH;
float fTargetValue         = 0.0f;
float fThreadsPerInchValue = FENCE_TPI;

unsigned long nTime     = 0;
unsigned long nLCDTime  = 0;
unsigned long nHoldTime = 0;

char cEditValueBuffer[32] = { 0 };

String sSerialBuffer;

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
		if (digitalRead(KEY_HOME) == LOW && !bHome && !bEditMode)
		{
			Serial1.print('H');
			bHome = true;
			bHomed = false;
			bHoming = true;
			showMenu();
		}

		if (bHomed)
		{
			if (digitalRead(KEY_JOG_MINUS) == LOW && !bJogMinus &&!bJogPlus && !bGoTarget && !bEditMode && !bTargetMode)
			{
				bJogMinus = true;
				Serial1.print('X');
			}
			else if (digitalRead(KEY_JOG_PLUS) == LOW && !bJogPlus && !bJogMinus && !bGoTarget && !bEditMode && !bTargetMode)
			{
				bJogPlus = true;
				Serial1.print('Y');
			}
			else if (digitalRead(KEY_GO) == LOW && !bGoTarget && !bJogPlus && !bJogMinus && !bEditMode)
			{
				bGoTarget = true;
				bTargetMode = true;

				updateSpeed();

				delay(120);

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
		}

		// Continuously call to handle keypad input
		keypadHandler();
		
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
		sSerialBuffer = Serial1.readString();
		cSerialBuffer = sSerialBuffer[0];

		bSerialParams = (sSerialBuffer[1] == ':') ? true : false;

		Serial.println(sSerialBuffer);

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
		else if (cSerialBuffer == 'G')
		{
			bTargetMode = false;
		}
		else if (cSerialBuffer == 'H')
		{
			bHome = false;
			bHomed = true;
			bHoming = false;
			nPageMode = PAGE_TARGET;
			fPositionValue = FENCE_OFFSET;
			showMenu();
		}
		else if (cSerialBuffer == 'P')
		{
			sSerialBuffer[0] = '0';
			sSerialBuffer[1] = '0';

			fPositionValue = atof(sSerialBuffer.c_str());

			clearRowPartial(8, 16, 1);
			lcd.setCursor(8, 1);
			lcd.print(fPositionValue, 3);
			lcd.print('\"');

			if (fPositionValue == fTargetValue)
			{
				bTargetMode = false;
			}
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
		case 'B':
			if (nPageMode == PAGE_TARGET && bHomed)
			{
				clearRow(2);
				lcd.setCursor(0, 2);
				lcd.print("Tar\7et: ");

				bSetTargetValue = true;
				editMode(EDIT_MODE_PRE);
			}
			else if (nPageMode == PAGE_CONFIG_0)
			{
				clearRow(1);
				lcd.setCursor(3, 1);
				lcd.print("TPI: ");

				bSetThreadsPerInchValue = true;
				editMode(EDIT_MODE_PRE);
			}
			else if (nPageMode == PAGE_CONFIG_1)
			{
				clearRow(1);
				lcd.setCursor(1, 1);
				lcd.print("Steps: ");

				bSetStepsValue = true;
				editMode(EDIT_MODE_PRE);
			}
			break;
		case 'C':
			if (nPageMode == PAGE_CONFIG_0)
			{
				clearRow(2);
				lcd.setCursor(1, 2);
				lcd.print("Depth: ");

				bSetFenceDepthValue = true;
				editMode(EDIT_MODE_PRE);
			}
			else if (nPageMode == PAGE_CONFIG_1)
			{
				clearRow(2);
				lcd.setCursor(1, 2);
				lcd.print("Speed: ");

				bSetSpeedMultValue = true;
				editMode(EDIT_MODE_PRE);
			}
			else if (nPageMode == PAGE_TARGET && bHomed)
			{
				clearRowPartial(8, 15, 3);

				lcd.setCursor(1, 3);

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

				updateSpeed();
			}
			break;
		case 'D':
			if(bConfigMode)
			{
				nPageMode = (nPageMode == PAGE_CONFIG_0) ? PAGE_CONFIG_1 : PAGE_CONFIG_0;

				showMenu();
			}
			break;
		case '#':
			if (nKeypadBufferOld == '*' && !bConfigMode && !bHomed && !bHoming)
			{
				bConfigMode = true;

				nPageMode = PAGE_CONFIG_0;
				showMenu();
			}
			break;
	}

	if (nKeypadBuffer)
	{
		nKeypadBufferOld = nKeypadBuffer;
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

			memset(cEditValueBuffer, 0, 32);
			break;
		case EDIT_MODE_CUR:
			if (nBufferIndex < 12)
			{
				if ((nKeypadBuffer - 48) >= 0 && (nKeypadBuffer - 48 <= 9))
				{
					lcd.write(nKeypadBuffer);

					cEditValueBuffer[nBufferIndex++] = nKeypadBuffer;
				}
				else if (nKeypadBuffer == '*')
				{
					cEditValueBuffer[nBufferIndex++] = '.';

					lcd.write('.');
				}
				if ((nKeypadBuffer == 'A') && !bSetSummation && (nBufferIndex != 0))
				{
					cEditValueBuffer[nBufferIndex++] = '+';
					bSetSummation = true;

					lcd.write('+');
				}
				else if ((nKeypadBuffer == 'D') && !bSetDenominator && (nBufferIndex != 0))
				{
					cEditValueBuffer[nBufferIndex++] = '/';
					bSetDenominator = true;

					lcd.write('/');
				}
			}

			if (nKeypadBuffer == '#')
			{
				if (bSetTargetValue)
				{
					bSetTargetValue = false;
					fTargetValue = editModeParser();

					if (fTargetValue > fFenceDepth)
					{
						fTargetValue = fFenceDepth;
					}
				}
				else if (bSetThreadsPerInchValue)
				{
					bSetThreadsPerInchValue = false;
					fThreadsPerInchValue = editModeParser();

					if (fThreadsPerInchValue > 20)
					{
						fThreadsPerInchValue = 20;
					}

					memset(cBuf, 0, 32);

					cBuf[0] = 'T';
					cBuf[1] = ':';
					strcat(cBuf, String(fThreadsPerInchValue, 5).c_str());

					Serial1.print(cBuf);
				}
				else if (bSetFenceDepthValue)
				{
					bSetFenceDepthValue = false;
					fFenceDepth = editModeParser();

					if (fFenceDepth > 48.0f)
					{
						fFenceDepth = 48.0f;
					}

					memset(cBuf, 0, 32);

					cBuf[0] = 'D';
					cBuf[1] = ':';
					strcat(cBuf, String(fFenceDepth, 5).c_str());

					Serial1.print(cBuf);
				}
				else if (bSetStepsValue)
				{
					bSetStepsValue = false;
					nStepsValue = (int)round(editModeParser());

					if (nStepsValue > 51200)
					{
						nStepsValue = 51200;
					}

					memset(cBuf, 0, 32);

					cBuf[0] = 'N';
					cBuf[1] = ':';
					strcat(cBuf, String(nStepsValue).c_str());

					Serial1.print(cBuf);
				}
				else if (bSetSpeedMultValue)
				{
					bSetSpeedMultValue = false;
					nSpeedMultValue = (int)round(editModeParser());

					if (nSpeedMultValue > 4)
					{
						nSpeedMultValue = 4;
					}

					memset(cBuf, 0, 32);

					cBuf[0] = 'M';
					cBuf[1] = ':';
					strcat(cBuf, String(nSpeedMultValue).c_str());

					Serial1.print(cBuf);
				}

				editMode(EDIT_MODE_POST);
			}
			break;
		case EDIT_MODE_POST:
			bEditMode = false;
			bSetDenominator = false;
			bSetSummation = false;
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
		if ((bEditMode && !bJogPlus && !bJogMinus && !bGoTarget && bHomed) || (bEditMode && bConfigMode))
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

	if (nPageMode != PAGE_TARGET)
	{
		drawWindow(0, 0, LCD_COLS, LCD_ROWS);
	}

	switch (nPageMode)
	{
		case PAGE_TARGET:
			lcd.setCursor(0, 1);
			lcd.print("   Pos: ");
			lcd.print(fPositionValue, 3);
			lcd.print("\"");

			lcd.setCursor(0, 2);
			lcd.print("Tar\7et: ");
			lcd.print(fTargetValue, 3);
			lcd.print("\"");

			lcd.setCursor(1, 3);
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
		case PAGE_CONFIG_0:
			lcd.setCursor(3, 1);
			lcd.print("TPI: ");
			lcd.print(fThreadsPerInchValue, 3);

			lcd.setCursor(1, 2);
			lcd.print("Depth: ");
			lcd.print(fFenceDepth, 3);
			lcd.print("\"");

			alignRight(" Confi\7 1/2", 3, 2);
			break;
		case PAGE_CONFIG_1:
			lcd.setCursor(1, 1);
			lcd.print("Steps: ");
			lcd.print(nStepsValue);

			lcd.setCursor(1, 2);
			lcd.print("Speed: ");
			lcd.print(nSpeedMultValue);

			alignRight(" Confi\7 2/2", 3, 2);
			break;
		case PAGE_ESTOP:
			alignCenter("! ESTOP !", 1);
			alignCenter("Reset power", 2);
			break;
	}
}

// Prints an alternating loading symbol on the LCD
void showWarning(uint8_t x, uint8_t y)
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

void updateSpeed()
{
	char cBuf[32] = { 'S', ':' };

	strcat(cBuf, String(nSpeedValue).c_str());

	Serial1.print(cBuf);
}

// Return index of character in a string
int getIndex(const char s[], char delimeter)
{
	int i = 0;

	while (i < getLength(s))
	{
		if (s[i] != delimeter)
		{
			i = -1;
		}
		else
		{
			return i;
		}

		i++;
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

// Return calculated float for edit mode
float editModeParser()
{
	bool bAddition      = false;
	bool bDivision      = false;
	bool bAdditionEarly = false;
	bool bDivisionEarly = false;

	float fResult    = 0.0f;

	String sBuffer0;
	String sBuffer1;
	String sBuffer2;

	int nAddIndex = String(cEditValueBuffer).indexOf('+');
	int nDivIndex = String(cEditValueBuffer).indexOf('/');

	if (nAddIndex != -1 && nDivIndex != -1)
	{
		// AD
		if (nAddIndex < nDivIndex)
		{
			bAddition = true;
			bAdditionEarly = true;
			bDivision = true;

			sBuffer0 = String(cEditValueBuffer).substring(0, nAddIndex);
			sBuffer1 = String(cEditValueBuffer).substring(nAddIndex + 1, nDivIndex);
			sBuffer2 = String(cEditValueBuffer).substring(nDivIndex + 1, String(cEditValueBuffer).length());

			fResult = (atof(sBuffer1.c_str()) / atof(sBuffer2.c_str())) + atof(sBuffer0.c_str());
		}
		// DA
		else if (nAddIndex > nDivIndex)
		{
			bAddition = true;
			bDivision = true;
			bDivisionEarly = true;

			sBuffer0 = String(cEditValueBuffer).substring(0, nDivIndex);
			sBuffer1 = String(cEditValueBuffer).substring(nDivIndex + 1, nAddIndex);
			sBuffer2 = String(cEditValueBuffer).substring(nAddIndex + 1, String(cEditValueBuffer).length());

			fResult = (atof(sBuffer0.c_str()) / atof(sBuffer1.c_str()) + atof(sBuffer2.c_str()));
		}
	}
	// A
	else if (nAddIndex >= 0 && nDivIndex == -1)
	{
		bAddition = true;

		sBuffer0 = String(cEditValueBuffer).substring(0, nAddIndex);
		sBuffer1 = String(cEditValueBuffer).substring(nAddIndex, String(cEditValueBuffer).length());

		fResult = atof(sBuffer0.c_str()) + atof(sBuffer1.c_str());
	}
	// D
	else if (nDivIndex >= 0 && nAddIndex == -1)
	{
		bDivision = true;

		sBuffer0 = String(cEditValueBuffer).substring(0, nDivIndex);
		sBuffer1 = String(cEditValueBuffer).substring(nDivIndex + 1, String(cEditValueBuffer).length());

		fResult = atof(sBuffer0.c_str()) / atof(sBuffer1.c_str());
	}
	// N
	else
	{
		sBuffer0 = String(cEditValueBuffer);

		fResult = atof(sBuffer0.c_str());
	}

	return fResult;
}