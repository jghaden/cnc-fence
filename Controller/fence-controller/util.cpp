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

volatile bool bEStop        = false;
volatile bool bFenceHome    = false;
volatile bool bFenceEnd     = false;
volatile bool bHoming       = false;
volatile bool bJogMinus     = false;
volatile bool bJogPlus      = false;
volatile bool bProxHome     = false;
volatile bool bProxEnd      = false;
volatile bool bSerialParams = false;
volatile bool bTargetMode   = false;

volatile char cSerialBuffer;
volatile char cSerialBufferOld;
char cBuf[32];

volatile uint8_t nDirState           = LOW;
volatile uint8_t nSpeedValue         = 1;
volatile uint8_t nSpeedMultValue     = 1;
volatile uint8_t nSpeedTempValue     = 1;

volatile uint32_t nStepsValue        = CONF_STEPS;

volatile float fPositionValue        = 0.0f;
volatile float fFenceDepth           = CONF_DEPTH;
volatile float fTargetValue          = 0.0f;
volatile float fThreadsPerInchValue  = CONF_TPI;

float fEEPROMBuffer[1];

volatile unsigned long t0            = 0;
volatile unsigned long t1            = 0;

String sSerialBuffer;

void commandHandler()
{
	while (Serial1.available() > 0)
	{
		sSerialBuffer = Serial1.readString();
		cSerialBuffer = sSerialBuffer[0];

		bSerialParams = (sSerialBuffer[1] == ':') ? true : false;

		if (!bHoming)
		{
			switch (cSerialBuffer)
			{
				case 'D':
					sSerialBuffer[0] = '0';
					sSerialBuffer[1] = '0';

					fFenceDepth = atof(sSerialBuffer.c_str());

					EEPROM.write(EEPROM_FENCE_DEPTH, fFenceDepth);
					break;
				case 'G':
					sSerialBuffer[0] = '0';
					sSerialBuffer[1] = '0';

					fTargetValue = atof(sSerialBuffer.c_str());

					if ((fPositionValue - fTargetValue) < 0)
					{
						setDir(JOG_PLUS);
					}
					else
					{
						setDir(JOG_MINUS);
					}

					bTargetMode = true;

					jog(roundf(fabs(jogToIN(fTargetValue) - jogToIN(fPositionValue))));

					delay(120);

					memset(cBuf, 0, 32);

					cBuf[0] = 'P';
					cBuf[1] = ':';
					strcat(cBuf, String(fPositionValue, 5).c_str());

					Serial1.print(cBuf);

					delay(120);

					Serial1.print('G');
					break;
				case 'H':
					homing();

					fPositionValue = 0;

					delay(100);

					memset(cBuf, 0, 32);

					cBuf[0] = 'P';
					cBuf[1] = ':';
					strcat(cBuf, String(fPositionValue, 5).c_str());

					delay(120);

					Serial1.print("H");
					break;
				case 'M':
					sSerialBuffer[0] = '0';
					sSerialBuffer[1] = '0';

					nSpeedMultValue = atoi(sSerialBuffer.c_str());

					EEPROM.write(EEPROM_SPEED, nSpeedMultValue);
					break;
				case 'N':
					sSerialBuffer[0] = '0';
					sSerialBuffer[1] = '0';

					nStepsValue = atoi(sSerialBuffer.c_str());

					///EEPROM.write(EEPROM_STEPS, nStepsValue); // Needs union of 4 bytes in EEPROM for uint32_t
					break;
				case 'S':
					sSerialBuffer[0] = '0';
					sSerialBuffer[1] = '0';

					nSpeedValue = atoi(sSerialBuffer.c_str());
					break;
				case 'T':
					sSerialBuffer[0] = '0';
					sSerialBuffer[1] = '0';

					///fThreadsPerInchValue = atof(sSerialBuffer.c_str());

					///eeprom_write_float((void*)fEEPROMBuffer, fThreadsPerInchValue);
					///eeprom_write_block((const void*)EEPROM_TPI, (void*)fEEPROMBuffer, 4);
					///EEPROM.write(EEPROM_TPI, fThreadsPerInchValue);
					break;
				case 'X':
					if (cSerialBufferOld != 'Y')
					{
						bJogMinus = true;
						setDir(JOG_MINUS);
					}
					break;
				case 'Y':
					if (cSerialBufferOld != 'X')
					{
						bJogPlus = true;
						setDir(JOG_PLUS);
					}
					break;
				case 'x':
					bJogMinus = false;
					break;
				case 'y':
					bJogPlus = false;
					break;
			}

			cSerialBufferOld = cSerialBuffer;
		}
	}
}

void EStopISR()
{
	cli();  // Stop all other interrupts (Default = on)

	if (!bEStop)
	{
		bEStop = true;

		Serial1.print('E');
	}

	sei(); // Re-enable interrupts
}

void homing()
{
	bFenceHome = false;
	bHoming = true;
	bJogMinus = false;
	bJogPlus = false;

	nSpeedTempValue = nSpeedValue;
	nSpeedValue = 4;

	/* Homing sequence START */
	setDir(JOG_PLUS);
	// Move away from home sensor
	jog(jogToIN(0.125f));

	// Keep moving away until clear of sensor
	while (((digitalRead(PROX1_HOME) == LOW) || bProxHome) && ((digitalRead(PROX2_END) == HIGH ) || !bProxEnd))
	{
		jog();
	}

	// Move a bit more
	jog(jogToIN(0.125f));

	setDir(JOG_MINUS);

	// Move towards home sensor until detected
	while (((digitalRead(PROX1_HOME) == HIGH) || !bProxHome))
	{
		jog();
	}

	setDir(JOG_PLUS);

	// Move until clear of home sensor again
	while (((digitalRead(PROX1_HOME) == LOW) || bProxHome) && ((digitalRead(PROX2_END) == HIGH) || !bProxEnd))
	{
		jog();
	}

	nSpeedValue = 1;

	// Move away from home sensor again
	jog(jogToIN(0.0625));

	setDir(JOG_MINUS);

	// Move towards home sensor until detected again
	while (((digitalRead(PROX1_HOME) == HIGH) || !bProxHome))
	{
		jog();
	}

	bHoming = false;
	bFenceHome = true;
	/* Homing sequence END */

	nSpeedValue = nSpeedTempValue;

	fPositionValue = 0;
}

void jog(uint32_t steps = 1)
{
	for (uint32_t i = 0; i < steps; i++)
	{
		PING &= ~(1 << PING0); // Set PUL1 low
		PINA &= ~(1 << PINA3); // Set PUL2 low
		delayMicroseconds((DELAY_US / 2) / (nSpeedValue * nSpeedMultValue));
		PING |= (1 << PING0); // Set PUL1 high
		PINA |= (1 << PINA3); // Set PUL2 high
		delayMicroseconds((DELAY_US / 2) / (nSpeedValue * nSpeedMultValue));

		if (nDirState == JOG_MINUS)
		{
			fPositionValue -= (float)(1.0f / inToSteps());

			if (bProxHome)
			{
				break;
			}
		}
		else if (nDirState == JOG_PLUS)
		{
			fPositionValue += (float)(1.0f / inToSteps());

			if (bProxEnd)
			{
				break;
			}
		}
		
		if ((millis() - t0 ) > 100 && !bHoming)
		{
			commandHandler();

			t0 = millis();

			char cBuf[32] = { 'P', ':' };

			strcat(cBuf, String(fPositionValue, 5).c_str());

			Serial1.print(cBuf);
		}
	}
}

// Load config data from EEPROM (4 KB) to set fence depth and TPI out of reset
void loadEEPROM()
{
	nStepsValue = EEPROM.read(EEPROM_STEPS);
	nSpeedMultValue = EEPROM.read(EEPROM_SPEED);
	fFenceDepth = EEPROM.read(EEPROM_FENCE_DEPTH);

	eeprom_read_block((void*)fEEPROMBuffer, (const void*)EEPROM_TPI, 4);
	fThreadsPerInchValue = fEEPROMBuffer[1];

	if (isnan(nStepsValue))
	{
		nStepsValue = CONF_STEPS;
	}

	if (isnan(nSpeedMultValue))
	{
		nSpeedMultValue = CONF_SPEED;
	}

	if (isnan(fFenceDepth))
	{
		fFenceDepth = CONF_DEPTH;
	}

	if (isnan(fThreadsPerInchValue) || fThreadsPerInchValue <= 0)
	{
		fThreadsPerInchValue = CONF_TPI;
	}

	///Serial.print("  TPI: ");
	///Serial.println(fThreadsPerInchValue, 3);
	///Serial.print("Depth: ");
	///Serial.println(fFenceDepth, 3);
	///Serial.print("Speed: ");
	///Serial.println(nSpeedMultValue);
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

uint32_t jogToIN(float in)
{
	return (uint32_t)(CONF_STEPS * fThreadsPerInchValue * in);
}

uint32_t inToSteps()
{
	return (CONF_STEPS * fThreadsPerInchValue);
}