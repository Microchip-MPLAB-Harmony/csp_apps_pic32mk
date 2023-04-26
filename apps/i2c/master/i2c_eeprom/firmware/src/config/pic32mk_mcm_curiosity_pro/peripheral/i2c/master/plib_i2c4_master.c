/*******************************************************************************
  Inter-Integrated Circuit (I2C) Library
  Source File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_i2c4_master.c

  Summary:
    I2C PLIB Master Mode Implementation file

  Description:
    This file defines the interface to the I2C peripheral library.
    This library provides access to and control of the associated peripheral
    instance.

*******************************************************************************/
// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018-2019 Microchip Technology Inc. and its subsidiaries.
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

#include "device.h"
#include "plib_i2c4_master.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
#define NOP asm(" NOP")


volatile static I2C_OBJ i2c4Obj;

void I2C4_Initialize(void)
{
    /* Disable the I2C Master interrupt */
    IEC6CLR = _IEC6_I2C4MIE_MASK;

    /* Disable the I2C Bus collision interrupt */
    IEC6CLR = _IEC6_I2C4BIE_MASK;

    I2C4BRG = 595;

    I2C4CONCLR = _I2C4CON_SIDL_MASK;
    I2C4CONCLR = _I2C4CON_DISSLW_MASK;
    I2C4CONCLR = _I2C4CON_SMEN_MASK;

    /* Clear master interrupt flag */
    IFS6CLR = _IFS6_I2C4MIF_MASK;

    /* Clear fault interrupt flag */
    IFS6CLR = _IFS6_I2C4BIF_MASK;

    /* Turn on the I2C module */
    I2C4CONSET = _I2C4CON_ON_MASK;

    /* Set the initial state of the I2C state machine */
    i2c4Obj.state = I2C_STATE_IDLE;
}

/* I2C state machine */
static void I2C4_TransferSM(void)
{
    uint8_t tempVar = 0;
    IFS6CLR = _IFS6_I2C4MIF_MASK;

    switch (i2c4Obj.state)
    {
        case I2C_STATE_START_CONDITION:
            /* Generate Start Condition */
            I2C4CONSET = _I2C4CON_SEN_MASK;
            IEC6SET = _IEC6_I2C4MIE_MASK;
            IEC6SET = _IEC6_I2C4BIE_MASK;
            i2c4Obj.state = I2C_STATE_ADDR_BYTE_1_SEND;
            break;

        case I2C_STATE_ADDR_BYTE_1_SEND:
            /* Is transmit buffer full? */
            if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
            {
                if (i2c4Obj.address > 0x007FU)
                {
                    tempVar = (((volatile uint8_t*)&i2c4Obj.address)[1] << 1);
                    /* Transmit the MSB 2 bits of the 10-bit slave address, with R/W = 0 */
                    I2C4TRN = (uint32_t)( 0xF0U | (uint32_t)tempVar);

                    i2c4Obj.state = I2C_STATE_ADDR_BYTE_2_SEND;
                }
                else
                {
                    /* 8-bit addressing mode */
                    I2C_TRANSFER_TYPE transferType = i2c4Obj.transferType;

                    I2C4TRN = (((uint32_t)i2c4Obj.address << 1) | transferType);

                    if (i2c4Obj.transferType == I2C_TRANSFER_TYPE_WRITE)
                    {
                        i2c4Obj.state = I2C_STATE_WRITE;
                    }
                    else
                    {
                        i2c4Obj.state = I2C_STATE_READ;
                    }
                }
            }
            break;

        case I2C_STATE_ADDR_BYTE_2_SEND:
            /* Transmit the 2nd byte of the 10-bit slave address */
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
                {
                    /* Transmit the remaining 8-bits of the 10-bit address */
                    I2C4TRN = i2c4Obj.address;

                    if (i2c4Obj.transferType == I2C_TRANSFER_TYPE_WRITE)
                    {
                        i2c4Obj.state = I2C_STATE_WRITE;
                    }
                    else
                    {
                        i2c4Obj.state = I2C_STATE_READ_10BIT_MODE;
                    }
                }
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4Obj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ_10BIT_MODE:
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                /* Generate repeated start condition */
                I2C4CONSET = _I2C4CON_RSEN_MASK;
                i2c4Obj.state = I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4Obj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY:
            /* Is transmit buffer full? */
            if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
            {
                tempVar = (((volatile uint8_t*)&i2c4Obj.address)[1] << 1);
                /* Transmit the first byte of the 10-bit slave address, with R/W = 1 */
                I2C4TRN = (uint32_t)( 0xF1U | (uint32_t)tempVar);
                i2c4Obj.state = I2C_STATE_READ;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4Obj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_WRITE:
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                size_t writeCount = i2c4Obj.writeCount;

                /* ACK received */
                if (writeCount < i2c4Obj.writeSize)
                {
                    if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
                    {
                        /* Transmit the data from writeBuffer[] */
                        I2C4TRN = i2c4Obj.writeBuffer[writeCount];
                        i2c4Obj.writeCount++;
                    }
                }
                else
                {
                    size_t readSize = i2c4Obj.readSize;

                    if (i2c4Obj.readCount < readSize)
                    {
                        /* Generate repeated start condition */
                        I2C4CONSET = _I2C4CON_RSEN_MASK;

                        i2c4Obj.transferType = I2C_TRANSFER_TYPE_READ;

                        if (i2c4Obj.address > 0x007FU)
                        {
                            /* Send the I2C slave address with R/W = 1 */
                            i2c4Obj.state = I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY;
                        }
                        else
                        {
                            /* Send the I2C slave address with R/W = 1 */
                            i2c4Obj.state = I2C_STATE_ADDR_BYTE_1_SEND;
                        }

                    }
                    else
                    {
                        /* Transfer Complete. Generate Stop Condition */
                        I2C4CONSET = _I2C4CON_PEN_MASK;
                        i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
                    }
                }
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4Obj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ:
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                /* Slave ACK'd the device address. Enable receiver. */
                I2C4CONSET = _I2C4CON_RCEN_MASK;
                i2c4Obj.state = I2C_STATE_READ_BYTE;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4Obj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ_BYTE:
            /* Data received from the slave */
            if ((I2C4STAT & _I2C4STAT_RBF_MASK) != 0U)
            {
                size_t readCount = i2c4Obj.readCount;

                i2c4Obj.readBuffer[readCount] = (uint8_t)I2C4RCV;
                readCount++;
                if (readCount == i2c4Obj.readSize)
                {
                    /* Send NAK */
                    I2C4CONSET = _I2C4CON_ACKDT_MASK;
                    I2C4CONSET = _I2C4CON_ACKEN_MASK;
                }
                else
                {
                    /* Send ACK */
                    I2C4CONCLR = _I2C4CON_ACKDT_MASK;
                    I2C4CONSET = _I2C4CON_ACKEN_MASK;
                }
                i2c4Obj.readCount = readCount;
                i2c4Obj.state = I2C_STATE_WAIT_ACK_COMPLETE;
            }
            break;

        case I2C_STATE_WAIT_ACK_COMPLETE:
            {
                size_t readSize = i2c4Obj.readSize;
                /* ACK or NAK sent to the I2C slave */
                if (i2c4Obj.readCount < readSize)
                {
                    /* Enable receiver */
                    I2C4CONSET = _I2C4CON_RCEN_MASK;
                    i2c4Obj.state = I2C_STATE_READ_BYTE;
                }
                else
                {
                    /* Generate Stop Condition */
                    I2C4CONSET = _I2C4CON_PEN_MASK;
                    i2c4Obj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
                }
            }
            break;

        case I2C_STATE_WAIT_STOP_CONDITION_COMPLETE:
            i2c4Obj.state = I2C_STATE_IDLE;
            IEC6CLR = _IEC6_I2C4MIE_MASK;
            IEC6CLR = _IEC6_I2C4BIE_MASK;
            if (i2c4Obj.callback != NULL)
            {
                uintptr_t context = i2c4Obj.context;

                i2c4Obj.callback(context);
            }
            break;

        default:
                   /*Do Nothing*/
            break;
    }
}


void I2C4_CallbackRegister(I2C_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback != NULL)
    {
       i2c4Obj.callback = callback;
       i2c4Obj.context = contextHandle;
    }
    return;
}

bool I2C4_IsBusy(void)
{
    bool busycheck = false;
    uint32_t tempVar = I2C4CON;
    uint32_t tempVar1 = I2C4STAT;
    if( (i2c4Obj.state != I2C_STATE_IDLE ) || ((tempVar & 0x0000001FU) != 0U) ||
        ((tempVar1 & _I2C4STAT_TRSTAT_MASK) != 0U) || ((tempVar1 & _I2C4STAT_S_MASK) != 0U) )
    {
        busycheck = true;
    }
    return busycheck;
}

bool I2C4_Read(uint16_t address, uint8_t* rdata, size_t rlength)
{
    bool statusRead = false;
    uint32_t tempVar = I2C4STAT;
    /* State machine must be idle and I2C module should not have detected a start bit on the bus */
    if((i2c4Obj.state == I2C_STATE_IDLE) && (( tempVar & _I2C4STAT_S_MASK) == 0U))
    {
        i2c4Obj.address             = address;
        i2c4Obj.readBuffer          = rdata;
        i2c4Obj.readSize            = rlength;
        i2c4Obj.writeBuffer         = NULL;
        i2c4Obj.writeSize           = 0;
        i2c4Obj.writeCount          = 0;
        i2c4Obj.readCount           = 0;
        i2c4Obj.transferType        = I2C_TRANSFER_TYPE_READ;
        i2c4Obj.error               = I2C_ERROR_NONE;
        i2c4Obj.state               = I2C_STATE_ADDR_BYTE_1_SEND;

        I2C4CONSET                  = _I2C4CON_SEN_MASK;
        IEC6SET                     = _IEC6_I2C4MIE_MASK;
        IEC6SET                     = _IEC6_I2C4BIE_MASK;
        statusRead = true;
    }
    return statusRead;
}


bool I2C4_Write(uint16_t address, uint8_t* wdata, size_t wlength)
{
    bool statusWrite = false;
    uint32_t tempVar = I2C4STAT;
    /* State machine must be idle and I2C module should not have detected a start bit on the bus */
    if((i2c4Obj.state == I2C_STATE_IDLE) && (( tempVar & _I2C4STAT_S_MASK) == 0U))
    {
        i2c4Obj.address             = address;
        i2c4Obj.readBuffer          = NULL;
        i2c4Obj.readSize            = 0;
        i2c4Obj.writeBuffer         = wdata;
        i2c4Obj.writeSize           = wlength;
        i2c4Obj.writeCount          = 0;
        i2c4Obj.readCount           = 0;
        i2c4Obj.transferType        = I2C_TRANSFER_TYPE_WRITE;
        i2c4Obj.error               = I2C_ERROR_NONE;
        i2c4Obj.state               = I2C_STATE_ADDR_BYTE_1_SEND;

        I2C4CONSET                  = _I2C4CON_SEN_MASK;
        IEC6SET                     = _IEC6_I2C4MIE_MASK;
        IEC6SET                     = _IEC6_I2C4BIE_MASK;
        statusWrite = true;
    }
    return statusWrite;
}


bool I2C4_WriteRead(uint16_t address, uint8_t* wdata, size_t wlength, uint8_t* rdata, size_t rlength)
{
    bool statusWriteread = false;
    uint32_t tempVar = I2C4STAT;
    /* State machine must be idle and I2C module should not have detected a start bit on the bus */
    if((i2c4Obj.state == I2C_STATE_IDLE) &&
       ((tempVar & _I2C4STAT_S_MASK) == 0U))
    {
        i2c4Obj.address             = address;
        i2c4Obj.readBuffer          = rdata;
        i2c4Obj.readSize            = rlength;
        i2c4Obj.writeBuffer         = wdata;
        i2c4Obj.writeSize           = wlength;
        i2c4Obj.writeCount          = 0;
        i2c4Obj.readCount           = 0;
        i2c4Obj.transferType        = I2C_TRANSFER_TYPE_WRITE;
        i2c4Obj.error               = I2C_ERROR_NONE;
        i2c4Obj.state               = I2C_STATE_ADDR_BYTE_1_SEND;

        I2C4CONSET                  = _I2C4CON_SEN_MASK;
        IEC6SET                     = _IEC6_I2C4MIE_MASK;
        IEC6SET                     = _IEC6_I2C4BIE_MASK;
        statusWriteread = true;
    }
    return statusWriteread;
}

I2C_ERROR I2C4_ErrorGet(void)
{
    I2C_ERROR error;

    error = i2c4Obj.error;
    i2c4Obj.error = I2C_ERROR_NONE;

    return error;
}

bool I2C4_TransferSetup(I2C_TRANSFER_SETUP* setup, uint32_t srcClkFreq )
{
    uint32_t baudValue;
    uint32_t i2cClkSpeed;
    float fBaudValue;

    if (setup == NULL)
    {
        return false;
    }

    i2cClkSpeed = setup->clkSpeed;

    /* Maximum I2C clock speed cannot be greater than 1 MHz */
    if (i2cClkSpeed > 1000000U)
    {
        return false;
    }

    if( srcClkFreq == 0U)
    {
        srcClkFreq = 60000000UL;
    }

    fBaudValue = (((float)srcClkFreq / 2.0f) * ((1.0f / (float)i2cClkSpeed) - 0.000000130f)) - 1.0f;
    baudValue = (uint32_t)fBaudValue;

    /* I2CxBRG value cannot be from 0 to 3 or more than the size of the baud rate register */
    if ((baudValue < 4U) || (baudValue > 65535U))
    {
        return false;
    }

    I2C4BRG = baudValue;

    /* Enable slew rate for 400 kHz clock speed; disable for all other speeds */

    if (i2cClkSpeed == 400000U)
    {
        I2C4CONCLR = _I2C4CON_DISSLW_MASK;;
    }
    else
    {
        I2C4CONSET = _I2C4CON_DISSLW_MASK;;
    }

    return true;
}

void I2C4_TransferAbort( void )
{
    i2c4Obj.error = I2C_ERROR_NONE;

    // Reset the PLib objects and Interrupts
    i2c4Obj.state = I2C_STATE_IDLE;
    IEC6CLR = _IEC6_I2C4MIE_MASK;
    IEC6CLR = _IEC6_I2C4BIE_MASK;

    // Disable and Enable I2C Master
    I2C4CONCLR = _I2C4CON_ON_MASK;
    NOP;NOP;
    I2C4CONSET = _I2C4CON_ON_MASK;
}

void __attribute__((used)) I2C4_BUS_InterruptHandler(void)
{
    /* Clear the bus collision error status bit */
    I2C4STATCLR = _I2C4STAT_BCL_MASK;

    /* ACK the bus interrupt */
    IFS6CLR = _IFS6_I2C4BIF_MASK;

    i2c4Obj.state = I2C_STATE_IDLE;

    i2c4Obj.error = I2C_ERROR_BUS_COLLISION;

    if (i2c4Obj.callback != NULL)
    {
        uintptr_t context = i2c4Obj.context;

        i2c4Obj.callback(context);
    }
}

void __attribute__((used)) I2C4_MASTER_InterruptHandler(void)
{
    I2C4_TransferSM();
}
