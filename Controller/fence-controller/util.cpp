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

volatile bool bEStop = false;

float fFenceDepth;
float fSpeedValue;
float fTargetValue = 0.0f;
float fThreadsPerInchValue;

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
