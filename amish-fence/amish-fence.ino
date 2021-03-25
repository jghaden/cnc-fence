#include "util.h"
#include <eeprom.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Keypad.h"
#include "keypad_config.h"

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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

void setup()
{
	Serial.begin(9600);

	lcd.init();
	lcd.backlight();
	lcd.clear();
	customCharSetup(lcd);

	keypad.begin();

	loadEEPROM();

	showMenu();
}

void loop()
{
	keypad.tick();	

	if (Serial.available() > 0)
	{
		nSerialBuffer = Serial.read();
		Serial.println(nSerialBuffer);

		if (nSerialBuffer == '~')
		{
			lcd.clear();
			drawBox(lcd, 0, 0, LCD_COLS, LCD_ROWS);	// full screen box
			alignCenter(lcd, "debug", 1);
			alignCenter(lcd, "mode", 2);
		}
	}

	while (keypad.available())
	{
		keypadEvent e = keypad.read();
		nKeypadBuffer = e.bit.KEY;

		if (e.bit.EVENT == KEY_JUST_PRESSED && (millis() - nHoldTime > 200))
		{
			if (bEditMode)
			{
				if (nBufferIndex == 0 && !bSetDenominator)
				{
					memset(cValueBufferNumerator, 0, sizeof(cValueBufferNumerator));
					memset(cValueBufferDenominator, 0, sizeof(cValueBufferDenominator));
					cValueBufferDenominator[0] = '1';
				}

				if (nBufferIndex < 10)
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
					else if (nKeypadBuffer == '#')
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
			else
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
							clearRow(lcd, 1);
							lcd.setCursor(1, 1);
							lcd.print("Tar\7et: ");
							lcd.setCursor(9, 1);

							nBufferIndex = 0;
							bEditMode = true;
							bSetTargetValue = true;
						}
						else if (nPageMode == MODE_CONFIG)
						{
							clearRow(lcd, 1);
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
							clearRow(lcd, 2);							
							lcd.setCursor(1, 2);
							lcd.print("Speed: ");
							lcd.setCursor(8, 2);

							nBufferIndex = 0;
							bEditMode = true;
							bSetSpeedValue = true;
						}
						else if (nPageMode == MODE_CONFIG)
						{
							clearRow(lcd, 2);
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

							clearPartialRow(lcd, 11, 18, 1);
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

							clearPartialRow(lcd, 11, 18, 1);
							lcd.setCursor(11, 1);
							lcd.print(fTargetValue, 3);
							lcd.write('\"');
						}
						break;
				}
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
		drawBox(lcd, 0, 0, LCD_COLS, LCD_ROWS);
		lcd.setCursor(1, 1);
		lcd.print("Position: ");
		lcd.print(fTargetValue, 3);
		lcd.print("\"");

		lcd.setCursor(1, 2);
		lcd.print("Speed: ");
		lcd.print(fSpeedValue, 3);
		lcd.print(" in/s");

		alignRight(lcd, " Jo\7 mode ", 3, 2);
	}
	else if (nPageMode == MODE_CONFIG)
	{
		drawBox(lcd, 0, 0, LCD_COLS, LCD_ROWS);
		lcd.setCursor(1, 1);
		lcd.print("  TPI: ");
		lcd.print(fThreadsPerInchValue, 3);

		lcd.setCursor(1, 2);
		lcd.print("Depth: ");
		lcd.print(fFenceDepth, 3);
		lcd.print("\"");

		/*lcd.setCursor(2, 0);
		lcd.print(" Locked ");*/
		alignRight(lcd, " Confi\7 mode ", 3, 2);
	}
}