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
#include <string.h>
#include "peripheral/i2c/plib_i2c_smbus_common.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
#define NOP asm(" NOP")


volatile static I2C_OBJ i2c4MasterObj;

/* <cmd> <blocklen n> <data 1> ... <data n> <pec>*/
/* Total 1 + 1 + 255 + 1 = 258 bytes*/
volatile static uint8_t i2c4SMBUSWrBuffer[258];
/* <blocklen n> <data 1> ... <data n> <pec>*/
/* Total 1 + 255 + 1 = 257 bytes*/
volatile static uint8_t i2c4SMBUSRdBuffer[257];

void I2C4_MasterInitialize(void)
{
    /* Disable the I2C Master interrupt */
    IEC6CLR = _IEC6_I2C4MIE_MASK;

    /* Disable the I2C Bus collision interrupt */
    IEC6CLR = _IEC6_I2C4BIE_MASK;

    I2C4BRG = 595;


    /* Clear master interrupt flag */
    IFS6CLR = _IFS6_I2C4MIF_MASK;

    /* Clear fault interrupt flag */
    IFS6CLR = _IFS6_I2C4BIF_MASK;


    /* Set the initial state of the I2C state machine */
    i2c4MasterObj.state = I2C_STATE_IDLE;
}

/* I2C state machine */
static void I2C4_MasterTransferSM(void)
{
    uint8_t tempVar = 0;
    IFS6CLR = _IFS6_I2C4MIF_MASK;

    switch (i2c4MasterObj.state)
    {
        case I2C_STATE_START_CONDITION:
            /* Generate Start Condition */
            I2C4CONSET = _I2C4CON_SEN_MASK;
            IEC6SET = _IEC6_I2C4MIE_MASK;
            IEC6SET = _IEC6_I2C4BIE_MASK;
            i2c4MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND;
            break;

        case I2C_STATE_ADDR_BYTE_1_SEND:
            /* Is transmit buffer full? */
            if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
            {
                if (i2c4MasterObj.address > 0x007FU)
                {
                    tempVar = (((volatile uint8_t*)&i2c4MasterObj.address)[1] << 1);
                    /* Transmit the MSB 2 bits of the 10-bit slave address, with R/W = 0 */
                    I2C4TRN = (uint32_t)( 0xF0U | (uint32_t)tempVar);

                    i2c4MasterObj.state = I2C_STATE_ADDR_BYTE_2_SEND;
                }
                else
                {
                    /* 8-bit addressing mode */
                    I2C_TRANSFER_TYPE transferType = i2c4MasterObj.transferType;

                    I2C4TRN = (((uint32_t)i2c4MasterObj.address << 1) | transferType);

                    if (i2c4MasterObj.transferType == I2C_TRANSFER_TYPE_WRITE)
                    {
                        i2c4MasterObj.state = I2C_STATE_WRITE;
                    }
                    else
                    {
                        i2c4MasterObj.state = I2C_STATE_READ;
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
                    I2C4TRN = i2c4MasterObj.address;

                    if (i2c4MasterObj.transferType == I2C_TRANSFER_TYPE_WRITE)
                    {
                        i2c4MasterObj.state = I2C_STATE_WRITE;
                    }
                    else
                    {
                        i2c4MasterObj.state = I2C_STATE_READ_10BIT_MODE;
                    }
                }
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4MasterObj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ_10BIT_MODE:
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                /* Generate repeated start condition */
                I2C4CONSET = _I2C4CON_RSEN_MASK;
                i2c4MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4MasterObj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY:
            /* Is transmit buffer full? */
            if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
            {
                tempVar = (((volatile uint8_t*)&i2c4MasterObj.address)[1] << 1);
                /* Transmit the first byte of the 10-bit slave address, with R/W = 1 */
                I2C4TRN = (uint32_t)( 0xF1U | (uint32_t)tempVar);
                i2c4MasterObj.state = I2C_STATE_READ;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4MasterObj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_WRITE:
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                size_t writeCount = i2c4MasterObj.writeCount;

                /* ACK received */
                if (writeCount < i2c4MasterObj.writeSize)
                {
                    if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
                    {
                        /* Transmit the data from writeBuffer[] */
                        I2C4TRN = i2c4MasterObj.writeBuffer[writeCount];
                        i2c4MasterObj.writeCount++;
                    }
                }
                else
                {
                    size_t readSize = i2c4MasterObj.readSize;

                    if (i2c4MasterObj.readCount < readSize)
                    {
                        /* Generate repeated start condition */
                        I2C4CONSET = _I2C4CON_RSEN_MASK;

                        i2c4MasterObj.transferType = I2C_TRANSFER_TYPE_READ;

                        if (i2c4MasterObj.address > 0x007FU)
                        {
                            /* Send the I2C slave address with R/W = 1 */
                            i2c4MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND_10BIT_ONLY;
                        }
                        else
                        {
                            /* Send the I2C slave address with R/W = 1 */
                            i2c4MasterObj.state = I2C_STATE_ADDR_BYTE_1_SEND;
                        }

                    }
                    else
                    {
                        /* Transfer Complete. Generate Stop Condition */
                        I2C4CONSET = _I2C4CON_PEN_MASK;
                        i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
                    }
                }
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4MasterObj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ:
            if ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) == 0U)
            {
                /* Slave ACK'd the device address. Enable receiver. */
                I2C4CONSET = _I2C4CON_RCEN_MASK;
                i2c4MasterObj.state = I2C_STATE_READ_BYTE;
            }
            else
            {
                /* NAK received. Generate Stop Condition. */
                i2c4MasterObj.error = I2C_ERROR_NACK;
                I2C4CONSET = _I2C4CON_PEN_MASK;
                i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
            }
            break;

        case I2C_STATE_READ_BYTE:
            /* Data received from the slave */
            if ((I2C4STAT & _I2C4STAT_RBF_MASK) != 0U)
            {
                size_t readCount = i2c4MasterObj.readCount;
                uint8_t readByte = (uint8_t)I2C4RCV;

                i2c4MasterObj.readBuffer[readCount] = readByte;

                if (i2c4MasterObj.smbusReadBlk == true)
                {
                    i2c4MasterObj.readSize += readByte;

                    if (i2c4MasterObj.smbusReadPEC == true)
                    {
                        i2c4MasterObj.readSize += 1;
                    }

                    i2c4MasterObj.smbusReadBlk = false;
                }

                readCount++;
                if (readCount == i2c4MasterObj.readSize)
                {
                    /* Send NAK */
                    I2C4CONSET = _I2C4CON_ACKDT_MASK;
                    I2C4CONSET = _I2C4CON_ACKEN_MASK;
                }
                else
                {
                    if (i2c4MasterObj.smbusReadPEC == true)
                    {
                        i2c4MasterObj.pec = SMBUSCRC8Byte(i2c4MasterObj.pec, readByte);
                    }

                    /* Send ACK */
                    I2C4CONCLR = _I2C4CON_ACKDT_MASK;
                    I2C4CONSET = _I2C4CON_ACKEN_MASK;
                }
                i2c4MasterObj.readCount = readCount;
                i2c4MasterObj.state = I2C_STATE_WAIT_ACK_COMPLETE;
            }
            break;

        case I2C_STATE_WAIT_ACK_COMPLETE:
            {
                size_t readSize = i2c4MasterObj.readSize;
                /* ACK or NAK sent to the I2C slave */
                if (i2c4MasterObj.readCount < readSize)
                {
                    /* Enable receiver */
                    I2C4CONSET = _I2C4CON_RCEN_MASK;
                    i2c4MasterObj.state = I2C_STATE_READ_BYTE;
                }
                else
                {
                    /* Generate Stop Condition */
                    I2C4CONSET = _I2C4CON_PEN_MASK;
                    i2c4MasterObj.state = I2C_STATE_WAIT_STOP_CONDITION_COMPLETE;
                }
            }
            break;

        case I2C_STATE_WAIT_STOP_CONDITION_COMPLETE:
            i2c4MasterObj.state = I2C_STATE_IDLE;
            IEC6CLR = _IEC6_I2C4MIE_MASK;
            IEC6CLR = _IEC6_I2C4BIE_MASK;
            if (i2c4MasterObj.callback != NULL)
            {
                uintptr_t context = i2c4MasterObj.context;

                i2c4MasterObj.callback(context);
            }
            break;

        default:
                   /*Do Nothing*/
            break;
    }
}

static void I2C4_MasterXferStart(void)
{
    I2C4CONSET      = _I2C4CON_SEN_MASK;
    IEC6SET        = _IEC6_I2C4MIE_MASK;
    IEC6SET           = _IEC6_I2C4BIE_MASK;
}

static bool I2C4_MasterXferSetup(
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
    uint32_t tempVar = I2C4STAT;

    /* State machine must be idle and I2C module should not have detected a start bit on the bus */

    if((i2c4MasterObj.state == I2C_STATE_IDLE) &&
       ((tempVar & _I2C4STAT_S_MASK) == 0U) &&
       ((wdata != NULL && wlength != 0) || (rdata != NULL && rlength != 0)))
    {
        i2c4MasterObj.address             = address;
        i2c4MasterObj.readBuffer          = rdata;
        i2c4MasterObj.readSize            = rlength;
        i2c4MasterObj.writeBuffer         = wdata;
        i2c4MasterObj.writeSize           = wlength;
        i2c4MasterObj.writeCount          = 0;
        i2c4MasterObj.readCount           = 0;
        if (wdata != NULL && wlength != 0)
        {
            i2c4MasterObj.transferType    = I2C_TRANSFER_TYPE_WRITE;
        }
        else
        {
            i2c4MasterObj.transferType    = I2C_TRANSFER_TYPE_READ;
        }
        i2c4MasterObj.error               = I2C_ERROR_NONE;
        i2c4MasterObj.state               = I2C_STATE_ADDR_BYTE_1_SEND;
        i2c4MasterObj.smbusReadBlk        = smbusReadBlk;
        i2c4MasterObj.smbusReadPEC        = smbusReadPEC;

        status = true;
    }
    return status;
}

void I2C4_MasterCallbackRegister(I2C_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback != NULL)
    {
       i2c4MasterObj.callback = callback;
       i2c4MasterObj.context = contextHandle;
    }
    return;
}

bool I2C4_MasterIsBusy(void)
{
    bool busycheck = false;
    uint32_t tempVar = I2C4CON;
    uint32_t tempVar1 = I2C4STAT;
    if( (i2c4MasterObj.state != I2C_STATE_IDLE ) || ((tempVar & 0x0000001FU) != 0U) ||
        ((tempVar1 & _I2C4STAT_TRSTAT_MASK) != 0U) || ((tempVar1 & _I2C4STAT_S_MASK) != 0U) )
    {
        busycheck = true;
    }
    return busycheck;
}

bool I2C4_MasterRead(uint16_t address, uint8_t* rdata, size_t rlength)
{
    bool statusRead = false;
    statusRead = I2C4_MasterXferSetup(address, NULL, 0, rdata, rlength, false, false, false);

    if (statusRead == true)
    {
        I2C4_MasterXferStart();
    }

    return statusRead;
}

bool I2C4_MasterWrite(uint16_t address, uint8_t* wdata, size_t wlength)
{
    bool statusWrite = false;
    statusWrite = I2C4_MasterXferSetup(address, wdata, wlength, NULL, 0, false, false, false);

    if (statusWrite == true)
    {
        I2C4_MasterXferStart();
    }

    return statusWrite;
}


bool I2C4_MasterWriteRead(uint16_t address, uint8_t* wdata, size_t wlength, uint8_t* rdata, size_t rlength)
{
    bool statusWriteRead = false;
    statusWriteRead = I2C4_MasterXferSetup(address, wdata, wlength, rdata, rlength, false, false, false);

    if (statusWriteRead == true)
    {
        I2C4_MasterXferStart();
    }

    return statusWriteRead;
}



bool I2C4_MasterSMBUSSendByte(uint8_t address, void* pWrdata, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <data1> <pec_from_master> */
        i2c4SMBUSWrBuffer[xferLen++] = *((uint8_t*)pWrdata);
        if (enPEC)
        {
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Byte(crc, *((uint8_t*)pWrdata));

            i2c4SMBUSWrBuffer[xferLen++] = crc;
            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, xferLen, NULL, 0, false, false, false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSWriteByte(uint8_t address, uint8_t cmd, void* pWrdata, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <data1> <pec_from_master> */
        i2c4SMBUSWrBuffer[xferLen++] = cmd;
        i2c4SMBUSWrBuffer[xferLen++] = *((uint8_t*)pWrdata);
        if (enPEC)
        {
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);

            i2c4SMBUSWrBuffer[xferLen++] = crc;
            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, xferLen, NULL, 0, false, false, false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSWriteWord(uint8_t address, uint8_t cmd, void* pWrdata, bool enPEC)
{
    uint8_t* wrData = (uint8_t*)pWrdata;
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <data1> <data2> <pec_from_master> */
        i2c4SMBUSWrBuffer[xferLen++] = cmd;
        i2c4SMBUSWrBuffer[xferLen++] = wrData[0];
        i2c4SMBUSWrBuffer[xferLen++] = wrData[1];
        if (enPEC)
        {
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);

            i2c4SMBUSWrBuffer[xferLen++] = crc;
            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)(uint8_t*)i2c4SMBUSWrBuffer, xferLen, NULL, 0, false, false, false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSWriteBlock(uint8_t address, uint8_t cmd, void* pWrdata, uint32_t nWrBytes, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <wr_block_sz n> <data1> <data2> .. <datan> <pec_from_master>*/
        i2c4SMBUSWrBuffer[xferLen++] = cmd;
        i2c4SMBUSWrBuffer[xferLen++] = nWrBytes;

        memcpy((void*)&i2c4SMBUSWrBuffer[xferLen], (const void*)pWrdata, nWrBytes);
        xferLen += nWrBytes;

        if (enPEC)
        {
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);

            i2c4SMBUSWrBuffer[xferLen++] = crc;
            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, xferLen, NULL, 0, false, false, false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSReceiveByte(uint8_t address, bool enPEC)
{
    bool status = false;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <data1> <pec_from_slave>*/

        if (enPEC)
        {
            /*PEC will be sent by slave and will be calculated over all the bytes in this transfer. Here master only calculates the CRC on the bytes it is transmitting. */
            crc = SMBUSCRC8Byte(crc, ((address << 1) | 1));

            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, NULL, 0, (uint8_t*)i2c4SMBUSRdBuffer, enPEC == true? 2 : 1, false, false, enPEC == true? true : false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSReadByte(uint8_t address, uint8_t cmd, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <slave_add> <data1> <pec_from_slave>*/
        i2c4SMBUSWrBuffer[xferLen++] = cmd;

        if (enPEC)
        {
            /*PEC will be sent by slave and will be calculated over all the bytes in this transfer. Here master only calculates the CRC on the bytes it is transmitting. */
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);
            crc = SMBUSCRC8Byte(crc, ((address << 1) | 1));

            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, xferLen, (uint8_t*)i2c4SMBUSRdBuffer, enPEC == true? 2 : 1, false, false, enPEC == true? true : false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSReadWord(uint8_t address, uint8_t cmd, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <slave_add> <data1> <data2> <pec_from_slave> */
        i2c4SMBUSWrBuffer[xferLen++] = cmd;

        if (enPEC)
        {
            /*PEC will be sent by slave and will be calculated over all the bytes in this transfer. Here master only calculates the CRC on the bytes it is transmitting. */
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);
            crc = SMBUSCRC8Byte(crc, ((address << 1) | 1));

            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, 1, (uint8_t*)i2c4SMBUSRdBuffer, enPEC == true? 3 : 2, false, false, enPEC == true? true : false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSProcessCall(uint8_t address, uint8_t cmd, void* pWrdata, bool enPEC)
{
    uint8_t* wrData = (uint8_t*)pWrdata;
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <data1> <data2> <slave_add> <data1> <data2> <pec_from_slave> */
        i2c4SMBUSWrBuffer[xferLen++] = cmd;
        i2c4SMBUSWrBuffer[xferLen++] = wrData[0];
        i2c4SMBUSWrBuffer[xferLen++] = wrData[1];
        if (enPEC)
        {
            /*PEC will be sent by slave and will be calculated over all the bytes in this transfer. Here master only calculates the CRC on the bytes it is transmitting. */
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);
            crc = SMBUSCRC8Byte(crc, ((address << 1) | 1));

            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, xferLen, (uint8_t*)i2c4SMBUSRdBuffer, enPEC == true? 3 : 2, false, false, enPEC == true? true : false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSReadBlock(uint8_t address, uint8_t cmd, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <slave_add> <rd_block_sz n> <data1> <data2> .. <datan><pec_from_slave>*/
        i2c4SMBUSWrBuffer[xferLen++] = cmd;

        if (enPEC)
        {
            /*PEC will be sent by slave and will be calculated over all the bytes in this transfer. Here master only calculates the CRC on the bytes it is transmitting. */
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);
            crc = SMBUSCRC8Byte(crc, ((address << 1) | 1));

            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, 1, (uint8_t*)i2c4SMBUSRdBuffer, 1, false, true, enPEC == true? true : false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

bool I2C4_MasterSMBUSWriteReadBlock(uint8_t address, uint8_t cmd, void* pWrdata, uint32_t nWrBytes, bool enPEC)
{
    bool status = false;
    uint32_t xferLen = 0;
    uint8_t crc = 0;    //initial value of crc

    if (i2c4MasterObj.state == I2C_STATE_IDLE)
    {
        /* <slave_add> <cmd> <wr_block_sz n> <data1> <data2> .. <datan> <slave_add> <rd_block_sz n> <data1> <data1> .. <datan><pec_from_slave>*/
        i2c4SMBUSWrBuffer[xferLen++] = cmd;
        i2c4SMBUSWrBuffer[xferLen++] = nWrBytes;
        memcpy((void*)&i2c4SMBUSWrBuffer[xferLen], (const void*)pWrdata, nWrBytes);
        xferLen += nWrBytes;

        if (enPEC)
        {
            /*PEC will be sent by slave and will be calculated over all the bytes in this transfer. Here master only calculates the CRC on the bytes it is transmitting. */
            crc = SMBUSCRC8Byte(crc, (address << 1));
            crc = SMBUSCRC8Buffer(crc, (uint8_t*)i2c4SMBUSWrBuffer, xferLen);
            crc = SMBUSCRC8Byte(crc, ((address << 1) | 1));

            i2c4MasterObj.pec = crc;
        }

        status = I2C4_MasterXferSetup(address, (uint8_t*)i2c4SMBUSWrBuffer, xferLen, (uint8_t*)i2c4SMBUSRdBuffer, 1, false, true, enPEC == true? true : false);

        if (status == true)
        {
            I2C4_MasterXferStart();
        }
    }

    return status;
}

uint32_t I2C4_MasterSMBUSTransferCountGet(void)
{
    return i2c4MasterObj.readCount;
}

uint32_t I2C4_MasterSMBUSBufferRead(void* pBuffer)
{
    uint32_t i;
    uint32_t numBytesAvailable = i2c4MasterObj.readCount;

    for (i = 0; i < numBytesAvailable; i++)
    {
        ((uint8_t*)pBuffer)[i] = i2c4SMBUSRdBuffer[i];
    }

    return numBytesAvailable;
}

/* Must be called to check if the PEC sent by target matches with the PEC calculated by host */
bool I2C4_MasterSMBUSIsPECMatch(void)
{
    uint8_t lastRcvdByteIndex = i2c4MasterObj.readCount - 1;

    uint8_t rcvdPEC = i2c4SMBUSRdBuffer[lastRcvdByteIndex];

    return i2c4MasterObj.pec == rcvdPEC;
}


I2C_ERROR I2C4_MasterErrorGet(void)
{
    I2C_ERROR error;

    error = i2c4MasterObj.error;
    i2c4MasterObj.error = I2C_ERROR_NONE;

    return error;
}

bool I2C4_MasterTransferSetup(I2C_TRANSFER_SETUP* setup, uint32_t srcClkFreq )
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

void I2C4_MasterTransferAbort( void )
{
    i2c4MasterObj.error = I2C_ERROR_NONE;

    // Reset the PLib objects and Interrupts
    i2c4MasterObj.state = I2C_STATE_IDLE;
    IEC6CLR = _IEC6_I2C4MIE_MASK;
    IEC6CLR = _IEC6_I2C4BIE_MASK;

    // Disable and Enable I2C Master
    I2C4CONCLR = _I2C4CON_ON_MASK;
    NOP;NOP;
    I2C4CONSET = _I2C4CON_ON_MASK;
}

void __attribute__((used)) I2C4_MasterBUS_InterruptHandler(void)
{
    i2c4MasterObj.state = I2C_STATE_IDLE;

    i2c4MasterObj.error = I2C_ERROR_BUS_COLLISION;

    if (i2c4MasterObj.callback != NULL)
    {
        uintptr_t context = i2c4MasterObj.context;

        i2c4MasterObj.callback(context);
    }
}

void __attribute__((used)) I2C4_MASTER_InterruptHandler(void)
{
    I2C4_MasterTransferSM();
}
