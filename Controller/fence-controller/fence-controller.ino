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

volatile bool bFenceHome = false;
volatile bool bFenceEnd = false;
volatile bool bHoming = false;
volatile bool bJogMinus = false;
volatile bool bJogPlus = false;
volatile bool bProxHome = false;
volatile bool bProxEnd = false;

char cSerialBuffer;
unsigned long nHomingTime = 0;

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
	Serial.begin(115200);
	Serial1.begin(115200);

	// Load config data (fence depth, TPI) out of reset
	loadEEPROM();
}

void loop()
{
	// Reset when coming out of EStopISR
	if (bEStop && digitalRead(3) == HIGH)
	{
		Serial1.print('R');
		reset();
	}
	else if (!bEStop)
	{
		while (Serial1.available() > 0)
		{
			cSerialBuffer = Serial1.read();
			Serial.print(cSerialBuffer);

			if (!bHoming)
			{
				switch (cSerialBuffer)
				{
					case 'H':
						if (digitalRead(PROX1_HOME) == HIGH)
						{
							bFenceHome = false;
							bHoming = true;
							bJogMinus = false;
							bJogPlus = false;
							nHomingTime = 0;
							setDir(JOG_MINUS);
						}
						break;
					case 'X':
						bJogMinus = true;
						setDir(JOG_MINUS);
						break;
					case 'Y':
						bJogPlus = true;
						setDir(JOG_PLUS);
						break;
					case 'x':
						bJogMinus = false;
						break;
					case 'y':
						bJogPlus = false;
						break;
				}
			}
		}
		
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
		// Homing sequence
		else if (!bFenceHome && bHoming)
		{
			jog();
		}
	}
}

ISR(PCINT1_vect)
{
	///if((PINJ & ~(1 << PINJ1)) && !bProxHome)
	if (digitalRead(PROX1_HOME) == LOW)
	{
		bFenceHome = true;
		bHoming = false;
		bProxHome = true;
		///Serial.println("Home");
	}
	else
	{
		bProxHome = false;
	}
	
	///if((PINJ & ~(PINJ0)) && !bProxEnd)
	if (digitalRead(PROX2_END) == LOW)
	{
		bFenceHome = true;
		bFenceEnd = true;
		bHoming = false;
		bProxEnd = true;
		///Serial.println("End");
	}
	else
	{
		bProxEnd = false;
	}
}