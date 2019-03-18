/*
 * coral.c
 *
 *  Created on: Mar 17, 2019
 *      Author: AQaut
 */

#include "coral.h"
#include "gio.h"
#include "system.h"
#include "i2c.h"
#include "freeRTOS.h"
#include "os_semphr.h"
#include "os_queue.h"

void coral__sendMDMessage(uint8 addr, MDmessage_t* message)
{


    uint8 raw_message[3];

    //Generate message.
    raw_message[0] = message->checkSum;
    raw_message[1] = message->speedData;
    raw_message[2] = (0U | ((message->CounterClockwise) ? 64U : 0U)
                     | ((message->RequestMotorStartup) ? 16U : 0U)
                     | ((message->RequestMotorShutdown) ? 8U : 0U));

    //Set Parity Bit
    if(coral__parity(raw_message[0]) ^ coral__parity(raw_message[1]) ^ coral__parity(raw_message[2]))
    {
        raw_message[2] |= 1U;
    }

    i2cSetSlaveAdd(i2cREG1, (uint32)addr);
    i2cSetDirection(i2cREG1, I2C_TRANSMITTER);
    i2cSetMode(i2cREG1, I2C_MASTER);

    i2cSetCount(i2cREG1, MD_MESSAGE_LEN);

    i2cSetStop(i2cREG1); //Send a stop after the count reaches 0

    i2cSetStart(i2cREG1);

    xSemaphoreTake(i2cTransmitLock, 0);

    i2cSend(i2cREG1, MD_MESSAGE_LEN, raw_message);

}

void coral__receiveMDStatus(uint8 addr, MDmessage_t* status)
{
    /*message[2] = (0U | ((message->CounterClockwise) ? 64U : 0U)
                         | ((message->Error) ? 32U : 0U)
                         | ((message->RequestMotorStartup) ? 16U : 0U)
                         | ((message->RequestMotorShutdown) ? 8U : 0U)
                         | ((message->MotorDriverNotOK) ? 4U : 0U)
                         | ((message->MotorDriverOK) ? 2U : 0U));*/
}

void coral__setup(void)
{
    /* I2C Init as per GUI
     *  Mode = Master - Transmitter
     *  baud rate = 100KHz
     *  Bit Count = 8bit
     */
    i2cInit();

    i2cEnableNotification(i2cREG1, I2C_TX_INT | I2C_SCD_INT);

    i2cTransmitLock = xSemaphoreCreateMutex();


}


void coral__ledOn()
{
    //systemREG1->CLKCNTL |= ((uint32)(1U << 8U));

    gioInit();
    gioSetDirection(gioPORTB, 0x2);
    gioSetBit(gioPORTB,1,1);
    for(;;);
    return;
}


/*
 * Returns the parity of an individual char.
 */
uint8 coral__parity(uint8 x)
{
    uint8 par = 0;
    int i;
    for(i = 0; i < 8; i++)
    {
        par ^= x;
        x >>= 1;
    }

    return par & 0x01;
}


void i2cNotification(i2cBASE_t *i2c, uint32 flags)
{
    if(flags == (uint32)I2C_TX_INT)
    {
        xSemaphoreGive(i2cTransmitLock);
    }
    (void)1;
}
