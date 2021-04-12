/**
  ******************************************************************************
  * @file    fence-terminal.ino
  * @author  Joshua Haden
  * @version V0.1.0
  * @date    12-APR-2021
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#define KEY_HOLD 50
#define JOG_MINUS 8
#define JOG_PLUS  9
#define HOME      7

void(*reset)(void) = 0;

volatile bool bJogMinus = false;
volatile bool bJogPlus = false;

char cSerialBuffer;
unsigned long nTime = 0;

void setup()
{
	Serial.begin(115200);
	Serial1.begin(115200);

	Serial.println("Starting...");

	pinMode(JOG_MINUS, INPUT);
	pinMode(JOG_PLUS, INPUT);
	pinMode(HOME, INPUT);
}

void loop()
{
	while (Serial1.available() > 0)
	{
		cSerialBuffer = Serial1.read();
		Serial.print(cSerialBuffer);

		if (cSerialBuffer == 'R')
		{
			reset();
		}
	}

	if (millis() - nTime > KEY_HOLD)
	{
		//
		// button hold down
		//
		if (digitalRead(HOME) == LOW)
		{
			Serial1.print('H');
		}

		if (digitalRead(JOG_MINUS) == LOW && !bJogMinus)
		{
			bJogMinus = true;
			Serial1.print('X');
		}
		else if (digitalRead(JOG_PLUS) == LOW && !bJogPlus)
		{
			bJogPlus = true;
			Serial1.print('Y');
		}

		//
		// button release
		//
		if (digitalRead(JOG_MINUS) == HIGH && bJogMinus)
		{
			bJogMinus = false;
			Serial1.print('x');
		}
		else if (digitalRead(JOG_PLUS) == HIGH && bJogPlus)
		{
			bJogPlus = false;
			Serial1.print('y');
		}

		nTime = millis();
	}
}