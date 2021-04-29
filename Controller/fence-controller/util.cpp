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
volatile bool bFenceHome = false;
volatile bool bFenceEnd = false;
volatile bool bHoming = false;
volatile bool bJogMinus = false;
volatile bool bJogPlus = false;
volatile bool bProxHome = false;
volatile bool bProxEnd = false;

char cSerialBuffer;

uint8_t nDirState = LOW;
uint8_t nSpeedValue = 1;
uint8_t nSpeedTempValue = 1;

float fFenceDepth;
float fTargetValue = 0.0f;
float fThreadsPerInchValue;

unsigned long nHomingTime = 0;

void EStopISR()
{
	cli();  // Stop all other interrupts (Default = on)

	if (!bEStop)
	{
		bEStop = true;
	}

	sei(); // Re-enable interrupts
}

void jog(uint8_t steps = 1)
{
	for (uint8_t i = 0; i < steps; i++)
	{
		PING &= ~(1 << PING0); // Set PUL1 low
		PINA &= ~(1 << PINA3); // Set PUL2 low
		delayMicroseconds((DELAY_US / 2) / nSpeedValue);
		PING |= (1 << PING0); // Set PUL1 high
		PINA |= (1 << PINA3); // Set PUL2 high
		delayMicroseconds((DELAY_US / 2) / nSpeedValue);
	}
}

// Load config data from EEPROM (4 KB) to set fence depth and TPI out of reset
void loadEEPROM()
{
	fFenceDepth = EEPROM.read(EEPROM_FENCE_DEPTH);
	fThreadsPerInchValue = EEPROM.read(EEPROM_TPI);

	if (isnan(fFenceDepth))
	{
		fFenceDepth = 30.0f;
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
		PINL ^= (1 << PINL4); // Set DIR1 high
		PINA ^= (1 << PINA7); // Set DIR2 high
	}
	else if (dir == JOG_PLUS)
	{
		nDirState = JOG_PLUS;
		PINL &= ~(PINL4); // Set DIR1 low
		PINA &= ~(PINA7); // Set DIR2 low
	}

	delayMicroseconds(20);
}
