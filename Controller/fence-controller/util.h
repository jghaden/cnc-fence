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
#include <math.h>

// EEPROM addresses
#define EEPROM_FENCE_DEPTH 0xD4
#define EEPROM_SPEED       0xC1
#define EEPROM_TPI         0xC4
#define EEPROM_STEPS       0xC8

// Jog states
#define JOG_MINUS LOW  // Backwards (CCW)
#define JOG_PLUS  HIGH // Forwards   (CW)

#define DELAY_US 2500

// EStop pin
#define ESTOP 3

// Prxomity switch pins
#define PROX1_HOME 15 // Home
#define PROX2_END  14 // End

// PUL/DIR pins for motor drivers
#define PUL1 41 // PG0
#define PUL2 25 // PA3
#define DIR1 45 // PL4
#define DIR2 29 // PA7

#define CONF_STEPS  800
#define CONF_SPEED  1
#define CONF_TPI    5.08f  // TPI for full machine
#define CONF_DEPTH  48     // inches
#define CONF_OFFSET 0.056f // inches

#define IN_TO_STEP(x) (CONF_STEPS * CONF_TPI * x)
#define STEP_TO_IN(x) (x / (CONF_STEPS * CONF_TPI))

extern volatile bool bEStop, bFenceHome, bFenceEnd, bHoming, bJogMinus, bJogPlus, bProxHome, bProxEnd, bSerialParams, bTargetMode;
extern volatile char cSerialBuffer, cSerialBufferOld;
extern char cBuf[32];
extern volatile uint8_t nDirState, nSpeedValue, nSpeedMultValue, nSpeedTempValue;
extern volatile uint32_t nStepsValue;
extern volatile float fPositionValue, fFenceDepth, fTargetValue, fThreadsPerInchValue;
extern float fEEPROMBuffer[1];
extern volatile unsigned long nTime;
extern String sSerialBuffer;

void commandHandler();
void EStopISR();
void homing();
void jog(uint32_t steps = 1);
void loadEEPROM();
void setDir(uint8_t dir);
void targetMode(float in);

#endif
