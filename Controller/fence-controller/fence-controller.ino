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
uint8_t nDirState = LOW;
unsigned long nHomingTime = 0;

void setDir(uint8_t dir)
{
	if (dir == JOG_MINUS)
	{
		nDirState = JOG_MINUS;
		///digitalWrite(DIR1, JOG_MINUS);
		///digitalWrite(DIR2, JOG_MINUS);
		PINL &= ~(PINL4);
		PINA &= ~(PINA7);
	}
	else if (dir == JOG_PLUS)
	{
		nDirState = JOG_PLUS;
		///digitalWrite(DIR1, JOG_PLUS);
		///digitalWrite(DIR2, JOG_PLUS);
		PINL ^= (1 << 4);
		PINA ^= (1 << 7);
	}

	delayMicroseconds(20);
}

void setup()
{
	// Setup INT4 pin for E-Stop switch
	pinMode(ESTOP, INPUT);
	attachInterrupt(digitalPinToInterrupt(ESTOP), EStopISR, LOW);

	// Setup proximity switch pins
	pinMode(PROX1_HOME, INPUT);
	pinMode(PROX2_END, INPUT);
	// Configure PCINT for proximity switches
	// D15 (PCINT9)
	// D14 (PCINT10)
	PCICR  |= (1 << PCIE1);	// Enable PCMSK1
	PCMSK1 |= 0b00000110;	// D15 & D14 interrupt

	// Setup PUL/DIR pins for motors
	pinMode(PUL1, OUTPUT);
	pinMode(PUL2, OUTPUT);
	pinMode(DIR1, OUTPUT);
	pinMode(DIR2, OUTPUT);
	digitalWrite(DIR1, LOW);
	digitalWrite(DIR2, LOW);

	pinMode(52, INPUT);

	// Initialize serial port (115200-8-N-1)
	Serial.begin(115200);
	Serial1.begin(115200);

	// Load config data (fence depth, TPI) out of reset
	loadEEPROM();
}

unsigned long t0 = 0;
unsigned long t1 = 0;

void loop()
{
	t0 = micros();

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

		///if (digitalRead(52) == HIGH)
		///{
		///	reset();
		///}
	}

	Serial.println(micros() - t0);
}

ISR(PCINT1_vect)
{
	///bProxHome = (digitalRead(PROX1_HOME) == LOW) ? true : false;
	///bProxEnd  = (digitalRead(PROX2_END)  == LOW) ? true : false;

	if (digitalRead(PROX1_HOME) == LOW)
	///if((PINJ & PINJ1) && !bProxHome)
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
	
	if (digitalRead(PROX2_END) == LOW)
	///if((PINJ & PINJ0) && !bProxEnd)
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