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

uint8_t nDirState = LOW;

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
	///digitalWrite(PUL1, LOW);
	///digitalWrite(PUL2, LOW);
	PING &= ~(1 << PING0); // Set PUL1 low
	PINA &= ~(1 << PINA3); // Set PUL2 low
	delayMicroseconds(DELAY_US / 2);
	///digitalWrite(PUL1, HIGH);
	///digitalWrite(PUL2, HIGH);
	PING |= (1 << PING0); // Set PUL1 high
	PINA |= (1 << PINA3); // Set PUL2 high
	delayMicroseconds(DELAY_US / 2);
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

// 
void setDir(uint8_t dir)
{
	if (dir == JOG_MINUS)
	{
		nDirState = JOG_MINUS;
		PINL &= ~(PINL4); // Set DIR1 low
		PINA &= ~(PINA7); // Set DIR2 low
	}
	else if (dir == JOG_PLUS)
	{
		nDirState = JOG_PLUS;
		PINL ^= (1 << PINL4); // Set DIR1 high
		PINA ^= (1 << PINA7); // Set DIR2 high
	}

	delayMicroseconds(20);
}
