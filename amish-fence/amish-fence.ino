#include "util.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Keypad.h"
#include "keypad_config.h"

LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, LCD_COLS, LCD_ROWS);
Adafruit_Keypad keypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

bool bEditMode = false;
bool bSetFenceDepthValue = false;
bool bSetSpeedValue = false;
bool bSetTargetValue = false;
bool bSetThreadsPerInchValue = false;

int nHoldTime = 0;
int nPageMode = MODE_TARGET;
int nSerialBuffer = 0;
int nKeypadBuffer = 0;

int nBufferIndex = 0;

float fFenceDepth = 30.0f;
float fSpeedValue = 0.875f;
float fTargetValue = 0.0f;
float fThreadsPerInchValue = 20.0f;

char cValueBuffer[8] = { 0 };

void setup()
{
	Serial.begin(9600);

	lcd.init();
	lcd.backlight();
	lcd.clear();
	customCharSetup(lcd);

	keypad.begin();

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

		if (e.bit.EVENT == KEY_JUST_PRESSED)
		{
			if (bEditMode)
			{
				if (nBufferIndex < 6)
				{
					if ((nKeypadBuffer - 48) >= 0 && (nKeypadBuffer - 48 <= 9))
					{
						lcd.write(nKeypadBuffer);
						cValueBuffer[nBufferIndex++] = nKeypadBuffer;
					}
					else if (nKeypadBuffer == '*')
					{
						lcd.write('.');
						cValueBuffer[nBufferIndex++] = '.';
					}
				}

				if (nKeypadBuffer == 'A')
				{
					if (bSetTargetValue)
					{
						bSetTargetValue = false;
						fTargetValue = atof(cValueBuffer);

						if (fTargetValue > fFenceDepth)
						{
							fTargetValue = fFenceDepth;
						}

						bEditMode = false;
						showMenu();
					}
					else if (bSetThreadsPerInchValue)
					{
						bSetThreadsPerInchValue = false;
						fThreadsPerInchValue = atof(cValueBuffer);

						if (fThreadsPerInchValue > 80)
						{
							fThreadsPerInchValue = 80;
						}

						bEditMode = false;
						showMenu();
					}					
				}
				else if (nKeypadBuffer == 'B')
				{
					if (bSetFenceDepthValue)
					{
						bSetFenceDepthValue = false;
						fFenceDepth = atof(cValueBuffer);

						if (fFenceDepth > 300.0f)
						{
							fFenceDepth = 300.0f;
						}

						bEditMode = false;
						showMenu();
					}
					else if (bSetSpeedValue)
					{
						bSetSpeedValue = false;
						fSpeedValue = atof(cValueBuffer);

						if (fSpeedValue > 3.0f)
						{
							fSpeedValue = 3.0f;
						}

						bEditMode = false;
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
							memset(cValueBuffer, 0, sizeof(cValueBuffer));
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
							memset(cValueBuffer, 0, sizeof(cValueBuffer));
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
							memset(cValueBuffer, 0, sizeof(cValueBuffer));
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
							memset(cValueBuffer, 0, sizeof(cValueBuffer));
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

							showMenu();
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

							showMenu();
						}
						break;
				}
			}
		}
		else if (e.bit.EVENT == KEY_JUST_RELEASED)
		{
			//Serial.println(" up");
		}

		delay(50);
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

		alignRight(lcd, " Confi\7 mode ", 3, 2);
	}
}