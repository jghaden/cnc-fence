// This file contains predefined setup for various Adafruit Matrix Keypads.
#ifndef __KEYPAD_CONFIG_H__
#define __KEYPAD_CONFIG_H__

#define KEYPAD_PID3844
#define R1    38
#define R2    40
#define R3    42
#define R4    44
#define C1    30
#define C2    34
#define C3    32
#define C4    36

#if defined(KEYPAD_PID3844)
const byte ROWS = 4; // rows
const byte COLS = 4; // columns
// define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = { {'1', '2', '3', 'A'},
						 {'4', '5', '6', 'B'},
						 {'7', '8', '9', 'C'},
						 {'*', '0', '#', 'D'} };
byte rowPins[ROWS] = { R1, R2, R3,
					  R4 }; // connect to the row pinouts of the keypad
byte colPins[COLS] = { C1, C2, C3,
					  C4 }; // connect to the column pinouts of the keypad
#endif

#endif