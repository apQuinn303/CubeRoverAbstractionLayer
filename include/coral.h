/*
 * coral.h
 *
 *  Created on: Mar 17, 2019
 *      Author: AQaut
 */

#ifndef INCLUDE_CORAL_H_
#define INCLUDE_CORAL_H_

#include "hal_stdtypes.h"
#include "freeRTOS.h"
#include "os_semphr.h"

#define MD_MESSAGE_LEN 3

SemaphoreHandle_t i2cTransmitLock;

typedef struct MotorDriverMessage
{
   uint8 checkSum;
   uint8 speedData;
   bool MotorDriverOK;
   bool MotorDriverNotOK;
   bool RequestMotorShutdown;
   bool RequestMotorStartup;
   bool Error;
   bool CounterClockwise;
} MDmessage_t;



void coral__ledOn(void);

void coral__setup(void);

void coral__sendMDMessage(uint8 addr, MDmessage_t* message);

void coral__receiveMDStatus(uint8 addr, MDmessage_t* status);

uint8 coral__parity(uint8 x);


#endif /* INCLUDE_CORAL_H_ */
