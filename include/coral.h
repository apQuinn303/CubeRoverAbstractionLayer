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

/*If 1, trying to send a Motor Driver message while TX is unavailable will result
in returning -1. Else, it will hang.*/
#define CONFIG_I2C_FAIL_ON_TX_UNAVAILABLE 0
/*If 1, I2C messages will be sent using interrupts. Else, polling will be used.*/
#define CONFIG_I2C_USE_INTERRUPTS 1


#define MD_MESSAGE_LEN 3

/*Note: Must be volatile or else the compiler
  optimizes out the TX_UNAVAILABLE loop. */
volatile boolean i2cInUse;

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

int coral__sendMDMessage(uint8 addr, MDmessage_t* message);

void coral__receiveMDStatus(uint8 addr, MDmessage_t* status);

uint8 coral__parity(uint8 x);


#endif /* INCLUDE_CORAL_H_ */
