/**
  ******************************************************************************
  * @file    fence-controller.ino
  * @author  Joshua Haden
  * @version V0.1.1
  * @date    12-APR-2021
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#include <eeprom.h>
#include "util.h"

void(*reset)(void) = 0;

void setup()
{
	// Setup proximity switch pins
	// D15 (PCINT9)
	// D14 (PCINT10)
	DDRJ &= 0b11111100;     // Set D15 & D15 as inputs
	// Configure PCINT for proximity switches
	PCICR  |= (1 << PCIE1);	// Enable PCMSK1
	PCMSK1 |= 0b00000110;	// D15 & D14 interrupt

	// Setup INT4 pin for E-Stop switch
	DDRE &= ~(1 << PINE4); // Set ESTOP as input
	attachInterrupt(digitalPinToInterrupt(ESTOP), EStopISR, LOW);

	// Setup PUL/DIR pins for motors
	DDRG |= (1 << PING0);  // Set PUL1 as output
	DDRA |= (1 << PINA3);  // Set PUL2 as output
	DDRL |= (1 << PINL4);  // Set DIR1 as output
	DDRA |= (1 << PINA7);  // Set DIR2 as output
	PING &= ~(1 << PING0); // Set PUL1 low
	PINA &= ~(1 << PINA3); // Set PUL2 low
	PINL &= ~(PINL4);      // Set DIR1 low
	PINA &= ~(PINA7);      // Set DIR2 low

	// Initialize serial port (115200-8-N-1)
	///Serial.begin(115200);
	Serial1.begin(115200);
	Serial1.setTimeout(50);

	// Load config data (fence depth, TPI) out of reset
	loadEEPROM();
}

void loop()
{
	// Reset when coming out of EStopISR
	if (bEStop && (digitalRead(ESTOP) == HIGH))
	{
		bEStop = false;

		Serial1.print('R');
		reset();
	}
	else if (!bEStop)
	{
		commandHandler();
		
		// Jog only if homed
		if ((bJogMinus ^ bJogPlus) && bFenceHome)
		{
			if (!bProxHome && !bProxEnd)
			{
				jog();
			}
			else if (bProxHome && (nDirState == JOG_PLUS))
			{
				jog();
			}
			else if (bProxEnd && (nDirState == JOG_MINUS))
			{
				jog();
			}
		}
	}
}

ISR(PCINT1_vect)
{
	if (digitalRead(PROX1_HOME) == LOW)
	{
		bProxHome = true;

		delay(120);

		memset(cBuf, 0, 32);

		cBuf[0] = 'P';
		cBuf[1] = ':';
		strcat(cBuf, String(fPositionValue, 5).c_str());

		Serial1.print(cBuf);
	}
	else
	{
		bProxHome = false;
	}
	
	if (digitalRead(PROX2_END) == LOW)
	{
		bFenceHome = true;
		bFenceEnd = true;
		bHoming = false;
		bProxEnd = true;
	}
	else
	{
		bProxEnd = false;
	}
}
