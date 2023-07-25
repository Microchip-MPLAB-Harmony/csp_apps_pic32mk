/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

#define I2C2_SLAVE_ADDR                 0x56
#define I2C4_SLAVE_ADDR                 0x54
#define ENABLE_PEC()                    false
#define CMD_INDEX                       0
#define DATA1_INDEX                     1
#define DATA2_INDEX                     2
#define NUM_BYTES_INDEX                 1

#define LED_ON()                       LED1_Set()
#define LED_OFF()                      LED1_Clear()

typedef enum
{
    HOST_STATE_SEND_BYTE = 0,
    HOST_STATE_WRITE_BYTE,
    HOST_STATE_WRITE_WORD,
    HOST_STATE_WRITE_BLOCK,
    HOST_STATE_RECEIVE_BYTE,
    HOST_STATE_READ_BYTE,
    HOST_STATE_READ_WORD,
    HOST_STATE_READ_BLOCK,
    HOST_STATE_WRITE_READ_BLOCK,
    HOST_STATE_PROCESS_CALL,
    HOST_STATE_WAIT_FOR_TRANSFER_DONE,
    HOST_STATE_ERROR,
    HOST_STATE_SUCCESS,
}HOST_STATE;

typedef enum
{
    HOST_CMD_WRITE_BYTE,
    HOST_CMD_WRITE_WORD,
    HOST_CMD_WRITE_BLOCK,
    HOST_CMD_READ_BYTE,
    HOST_CMD_READ_WORD,
    HOST_CMD_READ_BLOCK,
    HOST_CMD_WRITE_READ_BLOCK,
    HOST_CMD_PROCESS_CALL,
}HOST_CMD;

typedef struct
{
    bool transferDone;
    bool blockRead;
    uint8_t wrBuffer[258];
    uint8_t rdBuffer[257];
    uint32_t nBytesRead;
    uint32_t nBytesRequested;
    HOST_STATE state;
    HOST_STATE prevState;
    HOST_STATE nextState;
}APP_I2C_MASTER;

typedef struct
{
    bool transferInProgress;
    uint8_t rxBuffer[257];
    uint8_t txBuffer[258];
    uint32_t rxIndex;
    uint32_t txIndex;
    uint32_t nDataBytes;
    HOST_CMD command;
}APP_I2C_SLAVE;

static volatile APP_I2C_MASTER i2c4_Master;
static volatile APP_I2C_MASTER i2c2_Master;

static volatile APP_I2C_SLAVE i2c4_Slave;
static volatile APP_I2C_SLAVE i2c2_Slave;

bool i2c4_SlaveEventHandler (I2C_SLAVE_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    switch (event)
    {
        case I2C_SLAVE_TRANSFER_EVENT_ADDR_MATCH:
            i2c4_Slave.rxIndex = i2c4_Slave.txIndex = 0;

            if (i2c4_Slave.transferInProgress == true)
            {
                /* This means this is a repeated start condition */
                i2c4_Slave.command = i2c4_Slave.rxBuffer[CMD_INDEX];
                if (i2c4_Slave.command == HOST_CMD_WRITE_READ_BLOCK)
                {
                    i2c4_Slave.nDataBytes = i2c4_Slave.rxBuffer[NUM_BYTES_INDEX];
                    i2c4_Slave.txBuffer[0] = i2c4_Slave.nDataBytes;
                    memcpy((void*)&i2c4_Slave.txBuffer[1], (void*)&i2c4_Slave.rxBuffer[NUM_BYTES_INDEX + 1], i2c4_Slave.nDataBytes);
                    /* Slave must transmit the number of data bytes back to the master */
                    i2c4_Slave.nDataBytes += 1;
                }
                else if (i2c4_Slave.command == HOST_CMD_PROCESS_CALL)
                {
                    i2c4_Slave.txBuffer[0] = i2c4_Slave.rxBuffer[DATA1_INDEX];
                    i2c4_Slave.txBuffer[1] = i2c4_Slave.rxBuffer[DATA2_INDEX];
                    i2c4_Slave.nDataBytes = 2;
                }
            }
            i2c4_Slave.transferInProgress = true;
            break;

        case I2C_SLAVE_TRANSFER_EVENT_RX_READY:
            i2c4_Slave.rxBuffer[i2c4_Slave.rxIndex++] = I2C4_SlaveReadByte();
            break;

        case I2C_SLAVE_TRANSFER_EVENT_TX_READY:
            if (i2c4_Slave.txIndex < i2c4_Slave.nDataBytes)
            {
                I2C4_SlaveWriteByte(i2c4_Slave.txBuffer[i2c4_Slave.txIndex++]);
            }
            else
            {
                /* All the data bytes are transmitted out, transmit PEC if enabled */
                if (ENABLE_PEC())
                {
                    I2C4_SlaveWriteByte(I2C4_SlaveCRCGet());
                    i2c4_Slave.txIndex++;
                }
            }
            break;

        case I2C_SLAVE_TRANSFER_EVENT_STOP_BIT_RECEIVED:
            if (i2c4_Slave.rxIndex > 0)
            {
                uint32_t readIndex = 0;
                
                if ((i2c4_Slave.rxIndex == 1) || ((ENABLE_PEC() && i2c4_Slave.rxIndex == 2)))
                {
                    /* This is a Send Byte packet */
                    i2c4_Slave.txBuffer[0] = i2c4_Slave.rxBuffer[readIndex++];
                    i2c4_Slave.nDataBytes = i2c4_Slave.rxIndex - (ENABLE_PEC()? 1: 0);
                }
                else
                {
                    /* Parse the data written by master */
                    i2c4_Slave.command = i2c4_Slave.rxBuffer[readIndex++];

                    if (i2c4_Slave.command == HOST_CMD_WRITE_BYTE)
                    {
                        i2c4_Slave.txBuffer[0] = i2c4_Slave.rxBuffer[readIndex++];
                        i2c4_Slave.nDataBytes = i2c4_Slave.rxIndex - 1 - (ENABLE_PEC()? 1: 0);
                    }
                    else if (i2c4_Slave.command == HOST_CMD_WRITE_WORD || i2c4_Slave.command == HOST_CMD_PROCESS_CALL)
                    {
                        i2c4_Slave.txBuffer[0] = i2c4_Slave.rxBuffer[readIndex++];
                        i2c4_Slave.txBuffer[1] = i2c4_Slave.rxBuffer[readIndex++];
                        i2c4_Slave.nDataBytes = i2c4_Slave.rxIndex - 1 - (ENABLE_PEC()? 1: 0);
                    }
                    else if (i2c4_Slave.command == HOST_CMD_WRITE_BLOCK)
                    {
                        i2c4_Slave.nDataBytes = i2c4_Slave.rxBuffer[readIndex++];
                        i2c4_Slave.txBuffer[0] = i2c4_Slave.nDataBytes;
                        memcpy((void*)&i2c4_Slave.txBuffer[1], (void*)&i2c4_Slave.rxBuffer[readIndex++], i2c4_Slave.nDataBytes);
                        /* Slave must transmit the number of data bytes back to the master */
                        i2c4_Slave.nDataBytes += 1;
                    }
                }
            }
            i2c4_Slave.transferInProgress = false;
            break;

        default:
            break;
    }
    return true;
}

bool i2c2_SlaveEventHandler (I2C_SLAVE_TRANSFER_EVENT event, uintptr_t contextHandle)
{
    switch (event)
    {
        case I2C_SLAVE_TRANSFER_EVENT_ADDR_MATCH:
            i2c2_Slave.rxIndex = i2c2_Slave.txIndex = 0;

            if (i2c2_Slave.transferInProgress == true)
            {
                /* This means this is a repeated start condition */
                i2c2_Slave.command = i2c2_Slave.rxBuffer[CMD_INDEX];
                if (i2c2_Slave.command == HOST_CMD_WRITE_READ_BLOCK)
                {
                    i2c2_Slave.nDataBytes = i2c2_Slave.rxBuffer[NUM_BYTES_INDEX];
                    i2c2_Slave.txBuffer[0] = i2c2_Slave.nDataBytes;
                    memcpy((void*)&i2c2_Slave.txBuffer[1], (void*)&i2c2_Slave.rxBuffer[NUM_BYTES_INDEX + 1], i2c2_Slave.nDataBytes);
                    /* Slave must transmit the number of data bytes back to the master */
                    i2c2_Slave.nDataBytes += 1;
                }
                else if (i2c2_Slave.command == HOST_CMD_PROCESS_CALL)
                {
                    i2c2_Slave.txBuffer[0] = i2c2_Slave.rxBuffer[DATA1_INDEX];
                    i2c2_Slave.txBuffer[1] = i2c2_Slave.rxBuffer[DATA2_INDEX];
                    i2c2_Slave.nDataBytes = 2;
                }
            }
            i2c2_Slave.transferInProgress = true;
            break;

        case I2C_SLAVE_TRANSFER_EVENT_RX_READY:
            i2c2_Slave.rxBuffer[i2c2_Slave.rxIndex++] = I2C2_SlaveReadByte();
            break;

        case I2C_SLAVE_TRANSFER_EVENT_TX_READY:
            if (i2c2_Slave.txIndex < i2c2_Slave.nDataBytes)
            {
                I2C2_SlaveWriteByte(i2c2_Slave.txBuffer[i2c2_Slave.txIndex++]);
            }
            else
            {
                /* All the data bytes are transmitted out, transmit PEC if enabled */
                if (ENABLE_PEC())
                {
                    I2C2_SlaveWriteByte(I2C2_SlaveCRCGet());
                    i2c2_Slave.txIndex++;
                }
            }
            break;

        case I2C_SLAVE_TRANSFER_EVENT_STOP_BIT_RECEIVED:
            if (i2c2_Slave.rxIndex > 0)
            {
                uint32_t readIndex = 0;
                
                if ((i2c2_Slave.rxIndex == 1) || ((ENABLE_PEC() && i2c2_Slave.rxIndex == 2)))
                {
                    /* This is a Send Byte packet */
                    i2c2_Slave.txBuffer[0] = i2c2_Slave.rxBuffer[readIndex++];
                    i2c2_Slave.nDataBytes = i2c2_Slave.rxIndex - (ENABLE_PEC()? 1: 0);
                }
                else
                {
                    /* Parse the data written by master */
                    i2c2_Slave.command = i2c2_Slave.rxBuffer[readIndex++];

                    if (i2c2_Slave.command == HOST_CMD_WRITE_BYTE)
                    {
                        i2c2_Slave.txBuffer[0] = i2c2_Slave.rxBuffer[readIndex++];
                        i2c2_Slave.nDataBytes = i2c2_Slave.rxIndex - 1 - (ENABLE_PEC()? 1: 0);
                    }
                    else if (i2c2_Slave.command == HOST_CMD_WRITE_WORD)
                    {
                        i2c2_Slave.txBuffer[0] = i2c2_Slave.rxBuffer[readIndex++];
                        i2c2_Slave.txBuffer[1] = i2c2_Slave.rxBuffer[readIndex++];
                        i2c2_Slave.nDataBytes = i2c2_Slave.rxIndex - 1 - (ENABLE_PEC()? 1: 0);
                    }
                    else if (i2c2_Slave.command == HOST_CMD_WRITE_BLOCK)
                    {
                        i2c2_Slave.nDataBytes = i2c2_Slave.rxBuffer[readIndex++];
                        i2c2_Slave.txBuffer[0] = i2c2_Slave.nDataBytes;
                        memcpy((void*)&i2c2_Slave.txBuffer[1], (void*)&i2c2_Slave.rxBuffer[readIndex++], i2c2_Slave.nDataBytes);
                        /* Slave must transmit the number of data bytes back to the master */
                        i2c2_Slave.nDataBytes += 1;
                    }
                }
            }
            i2c2_Slave.transferInProgress = false;
            break;

        default:
            break;
    }
    return true;
}

void i2c4_MasterEventHandler (uintptr_t contextHandle)
{
    i2c4_Master.transferDone = true;
}

void i2c4_MasterApp(void)
{
    switch(i2c4_Master.state)
    {
        case HOST_STATE_WAIT_FOR_TRANSFER_DONE:
            if (i2c4_Master.transferDone == true)
            {
                i2c4_Master.transferDone = false;

                if (I2C4_MasterErrorGet() != I2C_ERROR_NONE)
                {
                    /* Master probably lost arbitration. Keep retrying, until successful.*/
                    i2c4_Master.state = i2c4_Master.prevState;
                    break;
                }

                /* Read the received bytes, if any */
                i2c4_Master.nBytesRead = I2C4_MasterSMBUSBufferRead((void*)i2c4_Master.rdBuffer);

                uint32_t i = 0;
                uint32_t data = 1;

                if (i2c4_Master.blockRead == true)
                {
                    /* for block reads, the first byte indicates the number of bytes transmitted by the slave (excluding PEC byte) */
                    i2c4_Master.nBytesRead =  i2c4_Master.rdBuffer[i++];
                    i2c4_Master.blockRead = false;
                }
                else if (i2c4_Master.nBytesRead > 0)
                {
                    if (ENABLE_PEC())
                    {
                        /* Since last byte is PEC, actual data received is one less than the total bytes received */
                        i2c4_Master.nBytesRead -= 1;
                    }
                }
                /* Check if master received the requested number of bytes */
                if (i2c4_Master.nBytesRead != i2c4_Master.nBytesRequested)
                {
                    i2c4_Master.state = HOST_STATE_ERROR;
                    break;
                }
                if (i2c4_Master.nBytesRead > 0)
                {
                     /* Verify the PEC, if PEC is enabled */
                    if (ENABLE_PEC())
                    {
                        if (I2C4_MasterSMBUSIsPECMatch() == false)
                        {
                            i2c4_Master.state = HOST_STATE_ERROR;
                            break;
                        }
                    }
                    /* Verify if the received data is same as expected */
                    while (i2c4_Master.nBytesRead--)
                    {
                        if (i2c4_Master.rdBuffer[i++] != data++)
                        {
                            i2c4_Master.state = HOST_STATE_ERROR;
                            break;
                        }
                    }
                }
                i2c4_Master.nBytesRequested = 0;
                i2c4_Master.state = i2c4_Master.nextState;
            }
            break;

        case HOST_STATE_SEND_BYTE:
            i2c4_Master.wrBuffer[0] = 1;
            if (I2C4_MasterSMBUSSendByte(I2C2_SLAVE_ADDR, (void*)i2c4_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_RECEIVE_BYTE;
            }
            break;
            
        case HOST_STATE_RECEIVE_BYTE:
            if (I2C4_MasterSMBUSReceiveByte(I2C2_SLAVE_ADDR, ENABLE_PEC()) == true)
            {
                i2c4_Master.nBytesRequested = 1;
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_WRITE_BYTE;
            }
            break;    
            
        case HOST_STATE_WRITE_BYTE:
            i2c4_Master.wrBuffer[0] = 1;
            if (I2C4_MasterSMBUSWriteByte(I2C2_SLAVE_ADDR, HOST_CMD_WRITE_BYTE, (void*)i2c4_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_READ_BYTE;
            }
            break;

        case HOST_STATE_READ_BYTE:
            if (I2C4_MasterSMBUSReadByte(I2C2_SLAVE_ADDR, HOST_CMD_READ_BYTE, ENABLE_PEC()) == true)
            {
                i2c4_Master.nBytesRequested = 1;
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_WRITE_WORD;
            }
            break;

        case HOST_STATE_WRITE_WORD:
            i2c4_Master.wrBuffer[0] = 1;
            i2c4_Master.wrBuffer[1] = 2;
            if (I2C4_MasterSMBUSWriteWord(I2C2_SLAVE_ADDR, HOST_CMD_WRITE_WORD, (void*)i2c4_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_READ_WORD;
            }
            break;

        case HOST_STATE_READ_WORD:
            if (I2C4_MasterSMBUSReadWord(I2C2_SLAVE_ADDR, HOST_CMD_READ_WORD, ENABLE_PEC()) == true)
            {
                i2c4_Master.nBytesRequested = 2;
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_WRITE_BLOCK;
                /* Fill data for the next state */
                for (uint32_t j = 0; j < 32; j++)
                {
                    i2c4_Master.wrBuffer[j] = j+1;
                }
            }
            break;

        case HOST_STATE_WRITE_BLOCK:

            if (I2C4_MasterSMBUSWriteBlock(I2C2_SLAVE_ADDR, HOST_CMD_WRITE_BLOCK, (void*)i2c4_Master.wrBuffer, 32, ENABLE_PEC()) == true)
            {
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_READ_BLOCK;
            }
            break;

        case HOST_STATE_READ_BLOCK:
            if (I2C4_MasterSMBUSReadBlock(I2C2_SLAVE_ADDR, HOST_CMD_READ_BLOCK, ENABLE_PEC()) == true)
            {
                i2c4_Master.nBytesRequested = 32;
                i2c4_Master.blockRead = true;
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_WRITE_READ_BLOCK;
            }
            break;

        case HOST_STATE_WRITE_READ_BLOCK:
            if (I2C4_MasterSMBUSWriteReadBlock(I2C2_SLAVE_ADDR, HOST_CMD_WRITE_READ_BLOCK, (void*)i2c4_Master.wrBuffer, 32, ENABLE_PEC()) == true)
            {
                i2c4_Master.nBytesRequested = 32;
                i2c4_Master.blockRead = true;
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_PROCESS_CALL;
            }
            break;

        case HOST_STATE_PROCESS_CALL:
            i2c4_Master.wrBuffer[0] = 1;
            i2c4_Master.wrBuffer[1] = 2;
            if (I2C4_MasterSMBUSProcessCall(I2C2_SLAVE_ADDR, HOST_CMD_PROCESS_CALL, (void*)i2c4_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c4_Master.nBytesRequested = 2;
                i2c4_Master.prevState = i2c4_Master.state;
                i2c4_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c4_Master.nextState = HOST_STATE_SUCCESS;
            }
            break;
        case HOST_STATE_SUCCESS:

            break;

        case HOST_STATE_ERROR:

            break;
    }
}

void i2c2_MasterEventHandler (uintptr_t contextHandle)
{
    i2c2_Master.transferDone = true;
}

void i2c2_MasterApp(void)
{
    switch(i2c2_Master.state)
    {
        case HOST_STATE_WAIT_FOR_TRANSFER_DONE:
            if (i2c2_Master.transferDone == true)
            {
                i2c2_Master.transferDone = false;

                if (I2C2_MasterErrorGet() != I2C_ERROR_NONE)
                {
                    /* Master probably lost arbitration. Keep retrying, until successful.*/
                    i2c2_Master.state = i2c2_Master.prevState;
                    break;
                }

                /* Read the received bytes, if any */
                i2c2_Master.nBytesRead = I2C2_MasterSMBUSBufferRead((void*)i2c2_Master.rdBuffer);

                uint32_t i = 0;
                uint32_t data = 1;

                if (i2c2_Master.blockRead == true)
                {
                    /* for block reads, the first byte indicates the number of bytes transmitted by the slave (excluding PEC byte) */
                    i2c2_Master.nBytesRead =  i2c2_Master.rdBuffer[i++];
                    i2c2_Master.blockRead = false;
                }
                else if (i2c2_Master.nBytesRead > 0)
                {
                    if (ENABLE_PEC())
                    {
                        /* Since last byte is PEC, actual data received is one less than the total bytes received */
                        i2c2_Master.nBytesRead -= 1;
                    }
                }
                /* Check if master received the requested number of bytes */
                if (i2c2_Master.nBytesRead != i2c2_Master.nBytesRequested)
                {
                    i2c2_Master.state = HOST_STATE_ERROR;
                    break;
                }
                if (i2c2_Master.nBytesRead > 0)
                {
                     /* Verify the PEC, if PEC is enabled */
                    if (ENABLE_PEC())
                    {
                        if (I2C2_MasterSMBUSIsPECMatch() == false)
                        {
                            i2c2_Master.state = HOST_STATE_ERROR;
                            break;
                        }
                    }
                    /* Verify if the received data is same as expected */
                    while (i2c2_Master.nBytesRead--)
                    {
                        if (i2c2_Master.rdBuffer[i++] != data++)
                        {
                            i2c2_Master.state = HOST_STATE_ERROR;
                            break;
                        }
                    }
                }
                i2c2_Master.nBytesRequested = 0;
                i2c2_Master.state = i2c2_Master.nextState;
            }
            break;

        case HOST_STATE_SEND_BYTE:
            i2c2_Master.wrBuffer[0] = 1;
            if (I2C2_MasterSMBUSSendByte(I2C4_SLAVE_ADDR, (void*)i2c2_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_RECEIVE_BYTE;
            }
            break;
            
        case HOST_STATE_RECEIVE_BYTE:
            if (I2C2_MasterSMBUSReceiveByte(I2C4_SLAVE_ADDR, ENABLE_PEC()) == true)
            {
                i2c2_Master.nBytesRequested = 1;
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_WRITE_BYTE;
            }
            break;   
            
        case HOST_STATE_WRITE_BYTE:
            i2c2_Master.wrBuffer[0] = 1;
            if (I2C2_MasterSMBUSWriteByte(I2C4_SLAVE_ADDR, HOST_CMD_WRITE_BYTE, (void*)i2c2_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_READ_BYTE;
            }
            break;

        case HOST_STATE_READ_BYTE:
            if (I2C2_MasterSMBUSReadByte(I2C4_SLAVE_ADDR, HOST_CMD_READ_BYTE, ENABLE_PEC()) == true)
            {
                i2c2_Master.nBytesRequested = 1;
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_WRITE_WORD;
            }
            break;

        case HOST_STATE_WRITE_WORD:
            i2c2_Master.wrBuffer[0] = 1;
            i2c2_Master.wrBuffer[1] = 2;
            if (I2C2_MasterSMBUSWriteWord(I2C4_SLAVE_ADDR, HOST_CMD_WRITE_WORD, (void*)i2c2_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_READ_WORD;
            }
            break;

        case HOST_STATE_READ_WORD:
            if (I2C2_MasterSMBUSReadWord(I2C4_SLAVE_ADDR, HOST_CMD_READ_WORD, ENABLE_PEC()) == true)
            {
                i2c2_Master.nBytesRequested = 2;
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_WRITE_BLOCK;
                /* Fill data for the next state */
                for (uint32_t j = 0; j < 32; j++)
                {
                    i2c2_Master.wrBuffer[j] = j+1;
                }
            }
            break;

        case HOST_STATE_WRITE_BLOCK:
            if (I2C2_MasterSMBUSWriteBlock(I2C4_SLAVE_ADDR, HOST_CMD_WRITE_BLOCK, (void*)i2c2_Master.wrBuffer, 32, ENABLE_PEC()) == true)
            {
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_READ_BLOCK;
            }
            break;

        case HOST_STATE_READ_BLOCK:
            if (I2C2_MasterSMBUSReadBlock(I2C4_SLAVE_ADDR, HOST_CMD_READ_BLOCK, ENABLE_PEC()) == true)
            {
                i2c2_Master.nBytesRequested = 32;
                i2c2_Master.blockRead = true;
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_WRITE_READ_BLOCK;
            }
            break;

        case HOST_STATE_WRITE_READ_BLOCK:
            if (I2C2_MasterSMBUSWriteReadBlock(I2C4_SLAVE_ADDR, HOST_CMD_WRITE_READ_BLOCK, (void*)i2c2_Master.wrBuffer, 32, ENABLE_PEC()) == true)
            {
                i2c2_Master.nBytesRequested = 32;
                i2c2_Master.blockRead = true;
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_PROCESS_CALL;
            }
            break;

        case HOST_STATE_PROCESS_CALL:
            i2c2_Master.wrBuffer[0] = 1;
            i2c2_Master.wrBuffer[1] = 2;
            if (I2C2_MasterSMBUSProcessCall(I2C4_SLAVE_ADDR, HOST_CMD_PROCESS_CALL, (void*)i2c2_Master.wrBuffer, ENABLE_PEC()) == true)
            {
                i2c2_Master.nBytesRequested = 2;
                i2c2_Master.prevState = i2c2_Master.state;
                i2c2_Master.state = HOST_STATE_WAIT_FOR_TRANSFER_DONE;
                i2c2_Master.nextState = HOST_STATE_SUCCESS;
            }
            break;
        case HOST_STATE_SUCCESS:

            break;

        case HOST_STATE_ERROR:

            break;
    }
}

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    i2c4_Master.state = HOST_STATE_SEND_BYTE;

    i2c2_Master.state = HOST_STATE_SEND_BYTE;

    /* I2C4 will talk to the I2C2 slave */
    I2C4_MasterCallbackRegister(i2c4_MasterEventHandler, (uintptr_t)0);

    I2C4_SlaveCallbackRegister(i2c4_SlaveEventHandler, (uintptr_t)0);

    /* I2C2 will talk to the I2C4 slave */
    I2C2_MasterCallbackRegister(i2c2_MasterEventHandler, (uintptr_t)0);

    I2C2_SlaveCallbackRegister(i2c2_SlaveEventHandler, (uintptr_t)0);

    LED_OFF();

    while (1)
    {
        i2c4_MasterApp();

        i2c2_MasterApp();

        if (i2c2_Master.state == HOST_STATE_SUCCESS && i2c4_Master.state == HOST_STATE_SUCCESS)
        {
            LED_ON();
        }
    }


    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/
