/*******************************************************************************
  Inter-Integrated Circuit (I2C) Library
  Source File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_i2c2_master.c

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
#include "plib_i2c2_master.h"
#include "interrupts.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
#define NOP asm(" NOP")


static volatile I2C_OBJ i2c2MasterObj;


void I2C2_Initialize(void)
{
    /* Disable the I2C Master interrupt */
    IEC1CLR = _IEC1_I2C2MIE_MASK;

    /* Disable the I2C Bus collision interrupt */
    IEC1CLR = _IEC1_I2C2BIE_MASK;

    I2C2BRG = 595;

    I2C2CONCLR = _I2C2CON_SIDL_MASK;
    I2C2CONCLR = _I2C2CON_DISSLW_MASK;
    I2C2CONCLR = _I2C2CON_SMEN_MASK;

    /* Clear master interrupt flag */
    IFS1CLR = _IFS1_I2C2MIF_MASK;

    /* Clear fault interrupt flag */
    IFS1CLR = _IFS1_I2C2BIF_MASK;

    /* Turn on the I2C module */
    I2C2CONSET = _I2C2CON_ON_MASK;

    /* Set the initial state of the I2C state machine */
    i2c2MasterObj.state = I2C_STATE_IDLE;
}

/* I2C state machine */
static void I2C2_TransferSM(void)
{
    uint8_t tempVar = 0;
    bool isScanInProgress = false;
    IFS1CLR = _IFS1_I2C2MIF_MASK;

    switch (i2c2MasterObj.state)
    {
        case I2C_STATE_START_CONDITION:
            /* Generate Start Condition */
            I2C2CONSET = _I2C2CON_SEN_MASK;
            IEC1SET = _IEC1_I2C2MIE_MASK;
            IEC1SET = _IEC1_I2C2BIE_MASK;
            i2c2MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND;
            break;

        case I2C_STATE_ADDR_BYTE_1_SEND:
            /* Is transmit buffer full? */
            if ((I2C2STAT & _I2C2STAT_TBF_MASK) == 0U)
            {
                if (i2c2MasterObj.address > 0x007FU)
                {
                    tempVar = (((volatile uint8_t*)&i2c2MasterObj.address)[1] << 1);
                    /* Transmit the MSB 2 bits of the 10-bit slave address, with R/W = 0 */
                    I2C2TRN = (uint32_t)( 0xF0U | (uint32_t)tempVar);

                    i2c2MasterObj.state = I2C_STATE_ADDR_BYTE_2_SEND;
                }
                else
                {
                    /* 8-bit addressing mode */
                    I2C_TRANSFER_TYPE transferType = i2c2MasterObj.transferType;

                    I2C2TRN = (((uint32_t)i2c2MasterObj.address << 1) | transferType);

                    if (i2c2MasterObj.transferType == I2C_TRANSFER_TYPE_WRITE)
                    {
                        i2c2MasterObj.state = I2C_STATE_WRITE;
                    }
                    else
                    {
                        i2c2MasterObj.state = I2C_STATE_READ;
                    }
                }
            }
            break;

        case I2C_STATE_ADDR_BYTE_2_SEND:
            /* Transmit the 2nd byte of the 10-bit slave address */
            if ((I2C2STAT & _I2C2STAT_ACKSTAT_MASK) == 0U)
            {
                if ((I2C2STAT & _I2C2STAT_TBF_MASK) == 0U)
                {
                    /* Transmit the remaining 8-bits of the 10-bit address */
                    I2C2TRN = i2c2MasterObj.address;

                    if (i2c2MasterObj.transferType == I2C_TRANSFER_TYPE_WRITE)
                    {
                        i2c2MasterObj.state = I2C_STATE_WRITE;
                    }
                    else
                    {
                        i2c2MasterObj.state = I2C_STATE_READ_10BIT_MODE;
                    }
                }
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c2MasterObj.error = I2C_ERROR_NACK;
                I2C2CONSET = _I2C2CON_PEN_MASK;
                i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ_10BIT_MODE:
            if ((I2C2STAT & _I2C2STAT_ACKSTAT_MASK) == 0U)
            {
                /* Generate repeated start condition */
                I2C2CONSET = _I2C2CON_RSEN_MASK;
                i2c2MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c2MasterObj.error = I2C_ERROR_NACK;
                I2C2CONSET = _I2C2CON_PEN_MASK;
                i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY:
            /* Is transmit buffer full? */
            if ((I2C2STAT & _I2C2STAT_TBF_MASK) == 0U)
            {
                tempVar = (((volatile uint8_t*)&i2c2MasterObj.address)[1] << 1);
                /* Transmit the first byte of the 10-bit slave address, with R/W = 1 */
                I2C2TRN = (uint32_t)( 0xF1U | (uint32_t)tempVar);
                i2c2MasterObj.state = I2C_STATE_READ;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c2MasterObj.error = I2C_ERROR_NACK;
                I2C2CONSET = _I2C2CON_PEN_MASK;
                i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_WRITE:
            if ((I2C2STAT & _I2C2STAT_ACKSTAT_MASK) == 0U)
            {
                size_t writeCount = i2c2MasterObj.writeCount;

                /* ACK received */
                if (writeCount < i2c2MasterObj.writeSize)
                {
                    if ((I2C2STAT & _I2C2STAT_TBF_MASK) == 0U)
                    {
                        /* Transmit the data from writeBuffer[] */
                        I2C2TRN = i2c2MasterObj.writeBuffer[writeCount];
                        i2c2MasterObj.writeCount++;
                    }
                }
                else
                {
                    size_t readSize = i2c2MasterObj.readSize;

                    if (i2c2MasterObj.readCount < readSize)
                    {
                        /* Generate repeated start condition */
                        I2C2CONSET = _I2C2CON_RSEN_MASK;

                        i2c2MasterObj.transferType = I2C_TRANSFER_TYPE_READ;

                        if (i2c2MasterObj.address > 0x007FU)
                        {
                            /* Send the I2C slave address with R/W = 1 */
                            i2c2MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY;
                        }
                        else
                        {
                            /* Send the I2C slave address with R/W = 1 */
                            i2c2MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND;
                        }

                    }
                    else
                    {
                        /* Transfer Complete. Generate Stop Condition */
                        I2C2CONSET = _I2C2CON_PEN_MASK;
                        i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
                    }
                }
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c2MasterObj.error = I2C_ERROR_NACK;
                I2C2CONSET = _I2C2CON_PEN_MASK;
                i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ:
            if ((I2C2STAT & _I2C2STAT_ACKSTAT_MASK) == 0U)
            {
                /* Slave ACK'd the device address. Enable receiver. */
                I2C2CONSET = _I2C2CON_RCEN_MASK;
                i2c2MasterObj.state = I2C_STATE_READ_BYTE;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c2MasterObj.error = I2C_ERROR_NACK;
                I2C2CONSET = _I2C2CON_PEN_MASK;
                i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ_BYTE:
            /* Data received from the slave */
            if ((I2C2STAT & _I2C2STAT_RBF_MASK) != 0U)
            {
                size_t readCount = i2c2MasterObj.readCount;
                uint8_t readByte = (uint8_t)I2C2RCV;

                i2c2MasterObj.readBuffer[readCount] = readByte;


                readCount++;
                if (readCount == i2c2MasterObj.readSize)
                {
                    /* Send NAK */
                    I2C2CONSET = _I2C2CON_ACKDT_MASK;
                    I2C2CONSET = _I2C2CON_ACKEN_MASK;
                }
                else
                {

                    /* Send ACK */
                    I2C2CONCLR = _I2C2CON_ACKDT_MASK;
                    I2C2CONSET = _I2C2CON_ACKEN_MASK;
                }
                i2c2MasterObj.readCount = readCount;
                i2c2MasterObj.state = I2C_STATE_WAIT_ACK_COMPLETE;
            }
            break;

        case I2C_STATE_WAIT_ACK_COMPLETE:
            {
                size_t readSize = i2c2MasterObj.readSize;
                /* ACK or NAK sent to the I2C slave */
                if (i2c2MasterObj.readCount < readSize)
                {
                    /* Enable receiver */
                    I2C2CONSET = _I2C2CON_RCEN_MASK;
                    i2c2MasterObj.state = I2C_STATE_READ_BYTE;
                }
                else
                {
                    /* Generate Stop Condition */
                    I2C2CONSET = _I2C2CON_PEN_MASK;
                    i2c2MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
                }
            }
            break;

        case I2C_STATE_WAIT_STOP_CONDITION_COMPLETE:
            i2c2MasterObj.state = I2C_STATE_IDLE;
            IEC1CLR = _IEC1_I2C2MIE_MASK;
            IEC1CLR = _IEC1_I2C2BIE_MASK;

            isScanInProgress = i2c2MasterObj.busScanInProgress;

            if ((i2c2MasterObj.callback != NULL) && (isScanInProgress == false))
            {
                uintptr_t context = i2c2MasterObj.context;

                i2c2MasterObj.callback(context);
            }
            break;

        default:
                   /*Do Nothing*/
            break;
    }
}

static void I2C2_XferStart(void)
{
    I2C2CONSET      = _I2C2CON_SEN_MASK;
    IEC1SET        = _IEC1_I2C2MIE_MASK;
    IEC1SET           = _IEC1_I2C2BIE_MASK;
}

static bool I2C2_XferSetup(
    uint16_t address,
    uint8_t* wdata,
    size_t wlength,
    uint8_t* rdata,
    size_t rlength,
    bool forcedWrite,
    bool smbusReadBlk,
    bool smbusReadPEC
)
{
    bool status = false;
    uint32_t tempVar = I2C2STAT;

    /* State machine must be idle and I2C module should not have detected a start bit on the bus */

    if((i2c2MasterObj.state == I2C_STATE_IDLE) &&
       ((tempVar & _I2C2STAT_S_MASK) == 0U) &&
       ((wdata != NULL && wlength != 0) || (rdata != NULL && rlength != 0)))
    {
        i2c2MasterObj.address             = address;
        i2c2MasterObj.readBuffer          = rdata;
        i2c2MasterObj.readSize            = rlength;
        i2c2MasterObj.writeBuffer         = wdata;
        i2c2MasterObj.writeSize           = wlength;
        i2c2MasterObj.writeCount          = 0;
        i2c2MasterObj.readCount           = 0;
        if (wdata != NULL && wlength != 0)
        {
            i2c2MasterObj.transferType    = I2C_TRANSFER_TYPE_WRITE;
        }
        else
        {
            i2c2MasterObj.transferType    = I2C_TRANSFER_TYPE_READ;
        }
        i2c2MasterObj.error               = I2C_ERROR_NONE;
        i2c2MasterObj.state               = I2C_STATE_ADDR_BYTE_1_SEND;
        i2c2MasterObj.smbusReadBlk        = smbusReadBlk;
        i2c2MasterObj.smbusReadPEC        = smbusReadPEC;

        status = true;
    }
    return status;
}

void I2C2_CallbackRegister(I2C_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback != NULL)
    {
       i2c2MasterObj.callback = callback;
       i2c2MasterObj.context = contextHandle;
    }
    return;
}

bool I2C2_IsBusy(void)
{
    bool busycheck = false;
    uint32_t tempVar = I2C2CON;
    uint32_t tempVar1 = I2C2STAT;
    if( (i2c2MasterObj.state != I2C_STATE_IDLE ) || ((tempVar & 0x0000001FU) != 0U) ||
        ((tempVar1 & _I2C2STAT_TRSTAT_MASK) != 0U) || ((tempVar1 & _I2C2STAT_S_MASK) != 0U) )
    {
        busycheck = true;
    }
    return busycheck;
}

bool I2C2_Read(uint16_t address, uint8_t* rdata, size_t rlength)
{
    bool statusRead = false;
    statusRead = I2C2_XferSetup(address, NULL, 0, rdata, rlength, false, false, false);

    if (statusRead == true)
    {
        I2C2_XferStart();
    }

    return statusRead;
}

bool I2C2_Write(uint16_t address, uint8_t* wdata, size_t wlength)
{
    bool statusWrite = false;
    statusWrite = I2C2_XferSetup(address, wdata, wlength, NULL, 0, false, false, false);

    if (statusWrite == true)
    {
        I2C2_XferStart();
    }

    return statusWrite;
}


bool I2C2_WriteRead(uint16_t address, uint8_t* wdata, size_t wlength, uint8_t* rdata, size_t rlength)
{
    bool statusWriteRead = false;
    statusWriteRead = I2C2_XferSetup(address, wdata, wlength, rdata, rlength, false, false, false);

    if (statusWriteRead == true)
    {
        I2C2_XferStart();
    }

    return statusWriteRead;
}

bool I2C2_BusScan(uint16_t start_addr, uint16_t end_addr, void* pDevicesList, uint8_t* nDevicesFound)
{
    uint8_t* pDevList = (uint8_t*)pDevicesList;
    uint8_t nDevFound = 0;

    if (i2c2MasterObj.state != I2C_STATE_IDLE)
    {
        return false;
    }

    if ((pDevicesList == NULL) || (nDevicesFound == NULL))
    {
        return false;
    }

    i2c2MasterObj.busScanInProgress = true;

    *nDevicesFound = 0;

    for (uint16_t dev_addr = start_addr; dev_addr <= end_addr; dev_addr++)
    {
        while (I2C2_Write(dev_addr, NULL, 0) == false)
        {

        }

        while (i2c2MasterObj.state != I2C_STATE_IDLE)
        {
            /* Wait for the transfer to complete */
        }

        if (i2c2MasterObj.error == I2C_ERROR_NONE)
        {
            /* No error and device responded with an ACK. Add the device to the list of found devices. */
            if (dev_addr > 0x007FU)
            {
                ((uint16_t*)&pDevicesList)[nDevFound] = dev_addr;
            }
            else
            {
                pDevList[nDevFound] = (uint8_t)dev_addr;
            }

            nDevFound += 1;
        }
    }

    *nDevicesFound = nDevFound;

    i2c2MasterObj.busScanInProgress = false;

    return true;
}


I2C_ERROR I2C2_ErrorGet(void)
{
    I2C_ERROR error;

    error = i2c2MasterObj.error;
    i2c2MasterObj.error = I2C_ERROR_NONE;

    return error;
}

bool I2C2_TransferSetup(I2C_TRANSFER_SETUP* setup, uint32_t srcClkFreq )
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

    I2C2BRG = baudValue;

    /* Enable slew rate for 400 kHz clock speed; disable for all other speeds */

    if (i2cClkSpeed == 400000U)
    {
        I2C2CONCLR = _I2C2CON_DISSLW_MASK;;
    }
    else
    {
        I2C2CONSET = _I2C2CON_DISSLW_MASK;;
    }

    return true;
}

void I2C2_TransferAbort( void )
{
    i2c2MasterObj.error = I2C_ERROR_NONE;

    // Reset the PLib objects and Interrupts
    i2c2MasterObj.state = I2C_STATE_IDLE;
    IEC1CLR = _IEC1_I2C2MIE_MASK;
    IEC1CLR = _IEC1_I2C2BIE_MASK;

    // Disable and Enable I2C Master
    I2C2CONCLR = _I2C2CON_ON_MASK;
    NOP;NOP;
    I2C2CONSET = _I2C2CON_ON_MASK;
}

void __attribute__((used)) I2C2_BUS_InterruptHandler(void)
{
    bool isScanInProgress = i2c2MasterObj.busScanInProgress;

    /* Clear the bus collision error status bit */
    I2C2STATCLR = _I2C2STAT_BCL_MASK;

    /* ACK the bus interrupt */
    IFS1CLR = _IFS1_I2C2BIF_MASK;

    i2c2MasterObj.state = I2C_STATE_IDLE;

    i2c2MasterObj.error = I2C_ERROR_BUS_COLLISION;

    if ((i2c2MasterObj.callback != NULL) && (isScanInProgress == false))
    {
        uintptr_t context = i2c2MasterObj.context;

        i2c2MasterObj.callback(context);
    }
}

void __attribute__((used)) I2C2_MASTER_InterruptHandler(void)
{
    I2C2_TransferSM();
}
