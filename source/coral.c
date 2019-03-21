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
#include "pinmux.h"
#include "sys_core.h"

int coral__sendMDMessage(uint8 addr, MDmessage_t* message)
{
#if CONFIG_I2C_USE_INTERRUPTS
    #if CONFIG_I2C_FAIL_ON_TX_UNAVAILABLE
        if(i2cInUse == true) return -1;
    #else
        while(i2cInUse){};
    #endif
#endif



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

    //Configure options.
    i2cSetSlaveAdd(i2cREG1, (uint32)addr);

    i2cSetDirection(i2cREG1, I2C_TRANSMITTER);
    i2cSetMode(i2cREG1, I2C_MASTER);

    i2cSetCount(i2cREG1, MD_MESSAGE_LEN);

    i2cSetStop(i2cREG1); //Send a stop after the count reaches 0

    i2cSetStart(i2cREG1);

#if CONFIG_I2C_USE_INTERRUPTS
    i2cInUse = true;
#endif

    i2cSend(i2cREG1, MD_MESSAGE_LEN, raw_message);


#if !CONFIG_I2C_USE_INTERRUPTS
    // Wait until Bus Busy is cleared
     while(i2cIsBusBusy(i2cREG1) == true);

    // Wait until Stop is detected
    while(i2cIsStopDetected(i2cREG1) == 0);

    // Clear the Stop condition
    i2cClearSCD(i2cREG1);



#endif



    return 0;
}

int coral__receiveMDStatus(uint8 addr, MDmessage_t* status)
{
    //Make sure the master is ready before continuing.
    while(i2cIsMasterReady(i2cREG1) != true);

    uint8 raw_message[3];

    //i2cREG1->DIR = 0U;

    /* Configure address of Slave to talk to */
     i2cSetSlaveAdd(i2cREG1, (uint32)addr);

     /* Set direction to receiver */
     i2cSetDirection(i2cREG1, I2C_RECEIVER);


     i2cSetCount(i2cREG1, MD_MESSAGE_LEN);

     /* Set mode as Master */
     i2cSetMode(i2cREG1, I2C_MASTER);

     /* Set Stop after programmed Count */
     i2cSetStop(i2cREG1);

     /* Transmit Start Condition */
     i2cSetStart(i2cREG1);


     i2cReceive(i2cREG1, MD_MESSAGE_LEN, raw_message);

     /* Wait until Bus Busy is cleared */
     while(i2cIsBusBusy(i2cREG1) == true);

     /* Wait until Stop is detected */
     while(i2cIsStopDetected(i2cREG1) == 0);

     /* Clear the Stop condition */
     i2cClearSCD(i2cREG1);

     //Wait for Master to be ready before returning.
     //while(i2cIsMasterReady(i2cREG1) != true);

     /*uint8 george = (coral__parity(raw_message[0])
             ^ coral__parity(raw_message[1])
             ^ coral__parity(raw_message[2]));*/
     if((raw_message[0] == 'A') &&
             ((coral__parity(raw_message[0])
                     ^ coral__parity(raw_message[1])
                     ^ coral__parity(raw_message[2])) == 0U))
     {

         status->checkSum = raw_message[0];

         status->speedData = raw_message[1];

         status->CounterClockwise = (raw_message[2] & 64U) ? true : false;
         status->Error = (raw_message[2] & 32U)  ? true : false;
         status->MotorDriverNotOK = (raw_message[2] & 4U) ? true : false;
         status->MotorDriverOK = (raw_message[2] & 2U)  ? true : false;

         status->RequestMotorShutdown = false;
         status->RequestMotorStartup = false;


         return 0;
     }
     else
     {
         //Invalid status received.
         return -1;
     }
}

void coral__setup(void)
{
    //Make sure I2C pins act like I2C pins.
    muxInit();

    /* I2C Init as per GUI
     *  Mode = Master - Transmitter
     *  baud rate = 100KHz
     *  Bit Count = 8bit
     */
    i2cInit();

    _enable_interrupt_();

#if CONFIG_I2C_USE_INTERRUPTS
    i2cEnableNotification(i2cREG1, I2C_TX_INT | I2C_SCD_INT);

    i2cInUse = false;
#endif

}


void coral__ledOn()
{
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
#if CONFIG_I2C_USE_INTERRUPTS
    //When we reach the stop condition after a successful transmission...
    if(flags == (uint32)I2C_SCD_INT)
    {
        //We are allowed to send new messages again.
        i2cInUse = false;
    }
#endif
}
