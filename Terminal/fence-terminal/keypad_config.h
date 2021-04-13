/**
  ******************************************************************************
  * @file    keypad_config.h
  * @author  Joshua Haden
  * @version V0.1.0
  * @date    26-MAR-2021
  * @brief   Configure keypad pins and output data
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#ifndef __KEYPAD_CONFIG_H__
#define __KEYPAD_CONFIG_H__

#if defined(ARDUINO_AVR_MICRO)
	#define C1    10
	#define C2    16
	#define C3    14
	#define C4    15
	#define R1    18
	#define R2    19
	#define R3    20
	#define R4    21
#elif defined(ARDUINO_AVR_MEGA2560)
	#define C1    22
	#define C2    24
	#define C3    26
	#define C4    28
	#define R1    30
	#define R2    32
	#define R3    34
	#define R4    36
#endif

const byte ROWS = 4; // rows
const byte COLS = 4; // columns

// define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = { {'1', '2', '3', 'A'},
						  {'4', '5', '6', 'B'},
						  {'7', '8', '9', 'C'},
						  {'*', '0', '#', 'D'} };

byte colPins[COLS] = { C1, C2, C3, C4 }; // connect to the column pinouts of the keypad
byte rowPins[ROWS] = { R1, R2, R3, R4 }; // connect to the row pinouts of the keypad

#endif