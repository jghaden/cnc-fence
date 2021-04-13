/**
  ******************************************************************************
  * @file    util.h
  * @author  Joshua Haden
  * @version V0.1.0
  * @date    26-MAR-2021
  * @brief   Header for util.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
#ifndef _UTIL_h
#define _UTIL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <EEPROM.h>

// EEPROM addresses
#define EEPROM_FENCE_DEPTH 0xC0
#define EEPROM_SPEED       0xC1
#define EEPROM_TPI         0xC4

// Jog states
#define JOG_MINUS LOW  // Backwards (CCW)
#define JOG_PLUS  HIGH // Forwards   (CW)

#define DELAY_US 2500

// EStop pin
#define ESTOP 3

// Prxomity switch pins
#define PROX1_HOME 14 // Home
#define PROX2_END  15 // End

// PUL/DIR pins for motor drivers
#define PUL1 41
#define PUL2 25
#define DIR1 45
#define DIR2 29

extern volatile bool bEStop;
extern float fFenceDepth, fSpeedValue, fTargetValue, fThreadsPerInchValue;

void EStopISR();
void jog();
void loadEEPROM();

#endif
