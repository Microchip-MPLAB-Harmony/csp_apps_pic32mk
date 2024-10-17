/*******************************************************************************
  Inter-Integrated Circuit (I2C) Library
  Source File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_i2c4_slave.c

  Summary:
    I2C PLIB Slave Implementation file

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
#include "plib_i2c4_slave.h"
#include "interrupts.h"
#include "peripheral/i2c/plib_i2c_smbus_common.h"

#include "peripheral/i2c/slave/plib_i2c4_slave_local.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************
#define I2C4_SLAVE_DATA_SETUP_TIME_CORE_TIMER_CNTS          6
#define I2C4_SLAVE_RISE_TIME_CORE_TIMER_CNTS                18

volatile static I2C_SLAVE_OBJ i2c4SlaveObj;

void I2C4_SlaveInitialize(void)
{

    I2C4CONSET = (_I2C4CON_STREN_MASK | _I2C4CON_AHEN_MASK | _I2C4CON_DHEN_MASK );

    I2C4ADD = 0x54;

    I2C4MSK = 0x00;

    /* Clear slave interrupt flag */
    IFS6CLR = _IFS6_I2C4SIF_MASK;

    /* Clear fault interrupt flag */
    IFS6CLR = _IFS6_I2C4BIF_MASK;

    /* Enable the I2C Slave interrupt */
    IEC6SET = _IEC6_I2C4SIE_MASK;

    /* Enable the I2C Bus collision interrupt */
    IEC6SET = _IEC6_I2C4BIE_MASK;

    i2c4SlaveObj.callback = NULL;


    i2c4SlaveObj.pec = 0;
}

static void I2C4_SlaveRiseAndSetupTime(uint8_t sdaState)
{
    uint32_t startCount, endCount;

    if (sdaState == 0U)
    {
        endCount = I2C4_SLAVE_DATA_SETUP_TIME_CORE_TIMER_CNTS;
    }
    else
    {
        endCount = I2C4_SLAVE_DATA_SETUP_TIME_CORE_TIMER_CNTS + I2C4_SLAVE_RISE_TIME_CORE_TIMER_CNTS;
    }

    startCount =_CP0_GET_COUNT();

    while((_CP0_GET_COUNT()- startCount) < endCount)
    {
           /* Wait for timeout */
    }
}

/* I2C slave state machine */
static void I2C4_SlaveTransferSM(void)
{
    uint32_t i2c_addr;
    uint8_t sdaValue = 0U;
    uintptr_t context = i2c4SlaveObj.context;

    /* ACK the slave interrupt */
    IFS6CLR = _IFS6_I2C4SIF_MASK;

    if ((I2C4STAT & _I2C4STAT_P_MASK) != 0U)
    {
        I2C4CONCLR = _I2C4CON_PCIE_MASK;

        if (i2c4SlaveObj.callback != NULL)
        {
            (void)i2c4SlaveObj.callback(I2C_SLAVE_TRANSFER_EVENT_STOP_BIT_RECEIVED, context);
        }

        /* Reset PEC calculation. Application is expected to readout the calculated PEC value in the callback for Stop bit. */
        i2c4SlaveObj.pec = 0;
    }
    else if ((I2C4STAT & _I2C4STAT_D_A_MASK) == 0U)
    {
        I2C4CONSET = _I2C4CON_PCIE_MASK;

        if ((I2C4STAT & _I2C4STAT_RBF_MASK) != 0U)
        {
            /* Received I2C address must be read out */
            i2c_addr = I2C4RCV;
            /* Update PEC calculation */
            i2c4SlaveObj.pec = SMBUSCRC8Byte(i2c4SlaveObj.pec, (uint8_t)i2c_addr);

            if (i2c4SlaveObj.callback != NULL)
            {
                /* Notify that a address match event has occurred */
                if (i2c4SlaveObj.callback(I2C_SLAVE_TRANSFER_EVENT_ADDR_MATCH, context) == true)
                {
                    if ((I2C4STAT & _I2C4STAT_R_W_MASK) != 0U)
                    {
                        /* I2C master wants to read */
                        if ((I2C4STAT & _I2C4STAT_TBF_MASK) == 0U)
                        {
                            /* In the callback, slave must write to transmit register by calling I2Cx_WriteByte() */
                            (void)i2c4SlaveObj.callback(I2C_SLAVE_TRANSFER_EVENT_TX_READY, context);
                        }
                    }
                    /* Send ACK */
                    I2C4CONCLR = _I2C4CON_ACKDT_MASK;
                }
                else
                {
                    /* Send NAK */
                    I2C4CONSET = _I2C4CON_ACKDT_MASK;
                    sdaValue = 1U;
                }
                I2C4_SlaveRiseAndSetupTime(sdaValue);
            }
        /* Data written by the application; release the clock stretch */
        I2C4CONSET = _I2C4CON_SCLREL_MASK;
        }
    }
    else
    {
        /* Master reads from slave, slave transmits */
        if ((I2C4STAT & _I2C4STAT_R_W_MASK) != 0U)
        {
            if (((I2C4STAT & (_I2C4STAT_TBF_MASK | _I2C4STAT_ACKSTAT_MASK))  == 0U))
            {
                if (i2c4SlaveObj.callback != NULL)
                {
                    /* I2C master wants to read. In the callback, slave must write to transmit register */
                    (void)i2c4SlaveObj.callback(I2C_SLAVE_TRANSFER_EVENT_TX_READY, context);

                    sdaValue = (i2c4SlaveObj.lastByteWritten & 0x80U);
                }

                I2C4_SlaveRiseAndSetupTime(sdaValue);

                /* Data written by the application; release the clock stretch */
                I2C4CONSET = _I2C4CON_SCLREL_MASK;
            }
        }
        /* Master writes to slave, slave receives */
        else
        {
            if ((I2C4STAT & _I2C4STAT_RBF_MASK) != 0U)
            {
                if (i2c4SlaveObj.callback != NULL)
                {
                    /* I2C master wants to write. In the callback, slave must read data by calling I2Cx_ReadByte()  */
                    if (i2c4SlaveObj.callback(I2C_SLAVE_TRANSFER_EVENT_RX_READY, context) == true)
                    {
                        /* Send ACK */
                        I2C4CONCLR = _I2C4CON_ACKDT_MASK;
                    }
                    else
                    {
                        /* Send NAK */
                        I2C4CONSET = _I2C4CON_ACKDT_MASK;
                        sdaValue = 1U;
                    }

                    I2C4_SlaveRiseAndSetupTime(sdaValue);
                }
                /* Data read by the application; release the clock stretch */
                I2C4CONSET = _I2C4CON_SCLREL_MASK;
            }
        }
    }
}

void I2C4_SlaveCallbackRegister(I2C_SLAVE_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback != NULL)
    {
        i2c4SlaveObj.callback = callback;
        i2c4SlaveObj.context = contextHandle;
    }
}

bool I2C4_SlaveIsBusy(void)
{
    return ((I2C4STAT & _I2C4STAT_S_MASK) != 0U);
}

uint8_t I2C4_SlaveReadByte(void)
{
    uint8_t readByte = (uint8_t)I2C4RCV;

    /* Update PEC calculation */
    i2c4SlaveObj.pec = SMBUSCRC8Byte(i2c4SlaveObj.pec, readByte);

    return readByte;
}

void I2C4_SlaveWriteByte(uint8_t wrByte)
{
    if ((I2C4STAT & _I2C4STAT_TBF_MASK)  == 0U)
    {
        I2C4TRN = wrByte;
        i2c4SlaveObj.lastByteWritten = wrByte;
        /* Update PEC calculation */
        i2c4SlaveObj.pec = SMBUSCRC8Byte(i2c4SlaveObj.pec, wrByte);
    }
}

I2C_SLAVE_TRANSFER_DIR I2C4_SlaveTransferDirGet(void)
{
    return ((I2C4STAT & _I2C4STAT_R_W_MASK) != 0U) ? I2C_SLAVE_TRANSFER_DIR_READ : I2C_SLAVE_TRANSFER_DIR_WRITE;
}

I2C_SLAVE_ACK_STATUS I2C4_SlaveLastByteAckStatusGet(void)
{
    return ((I2C4STAT & _I2C4STAT_ACKSTAT_MASK) != 0U) ? I2C_SLAVE_ACK_STATUS_RECEIVED_NAK : I2C_SLAVE_ACK_STATUS_RECEIVED_ACK;
}

I2C_SLAVE_ERROR I2C4_SlaveErrorGet(void)
{
    I2C_SLAVE_ERROR error;

    error = i2c4SlaveObj.error;
    i2c4SlaveObj.error = I2C_SLAVE_ERROR_NONE;

    return error;
}

uint8_t I2C4_SlaveCRCGet(void)
{
    uint8_t pec = i2c4SlaveObj.pec;


    return pec;
}

void __attribute__((used)) I2C4_SlaveBUS_InterruptHandler(void)
{
    i2c4SlaveObj.error = I2C_SLAVE_ERROR_BUS_COLLISION;

    if (i2c4SlaveObj.callback != NULL)
    {
        uintptr_t context = i2c4SlaveObj.context;

        (void) i2c4SlaveObj.callback(I2C_SLAVE_TRANSFER_EVENT_ERROR, context);
    }
    /* Reset PEC calculation. Application is expected to readout the calculated PEC value in the callback for Stop bit. */
    i2c4SlaveObj.pec = 0;
}

void __attribute__((used)) I2C4_SLAVE_InterruptHandler(void)
{
    I2C4_SlaveTransferSM();
}
