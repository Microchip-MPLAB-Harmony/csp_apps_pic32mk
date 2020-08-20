/*******************************************************************************
  SPI PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_spi6_slave.c

  Summary:
    SPI6 Slave Source File

  Description:
    This file has implementation of all the interfaces provided for particular
    SPI peripheral.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019-2020 Microchip Technology Inc. and its subsidiaries.
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
#include "plib_spi6_slave.h"
#include "peripheral/gpio/plib_gpio.h"
#include <string.h>
// *****************************************************************************
// *****************************************************************************
// Section: SPI6 Slave Implementation
// *****************************************************************************
// *****************************************************************************
#define SPI6_BUSY_PIN                    GPIO_PIN_RG6
#define SPI6_CS_PIN                      GPIO_PIN_RC0


#define SPI6_READ_BUFFER_SIZE            256
#define SPI6_WRITE_BUFFER_SIZE           256

static uint8_t SPI6_ReadBuffer[SPI6_READ_BUFFER_SIZE];
static uint8_t SPI6_WriteBuffer[SPI6_WRITE_BUFFER_SIZE];


/* Global object to save SPI Exchange related data */
SPI_SLAVE_OBJECT spi6Obj;

#define SPI6_CON_CKP                        (0 << _SPI6CON_CKP_POSITION)
#define SPI6_CON_CKE                        (1 << _SPI6CON_CKE_POSITION)
#define SPI6_CON_MODE_32_MODE_16            (0 << _SPI6CON_MODE16_POSITION)
#define SPI6_CON_ENHBUF                     (1 << _SPI6CON_ENHBUF_POSITION)
#define SPI6_CON_STXISEL                    (3 << _SPI6CON_STXISEL_POSITION)
#define SPI6_CON_SRXISEL                    (1 << _SPI6CON_SRXISEL_POSITION)

#define SPI6_ENABLE_RX_INT()                IEC7SET = 0x10
#define SPI6_CLEAR_RX_INT_FLAG()            IFS7CLR = 0x10

#define SPI6_DISABLE_TX_INT()               IEC7CLR = 0x20
#define SPI6_ENABLE_TX_INT()                IEC7SET = 0x20
#define SPI6_CLEAR_TX_INT_FLAG()            IFS7CLR = 0x20

#define SPI6_ENABLE_ERR_INT()               IEC7SET = 0x8
#define SPI6_CLEAR_ERR_INT_FLAG()           IEC7CLR = 0x8

/* Forward declarations */
static void SPI6_CS_Handler(GPIO_PIN pin, uintptr_t context);

void SPI6_Initialize ( void )
{
    /* Disable SPI6 Interrupts */
    IEC7CLR = 0x8;
    IEC7CLR = 0x10;
    IEC7CLR = 0x20;

    /* STOP and Reset the SPI */
    SPI6CON = 0;

    /* Clear SPI6 Interrupt flags */
    IFS7CLR = 0x8;
    IFS7CLR = 0x10;
    IFS7CLR = 0x20;

    /* CLear the receiver overflow error flag */
    SPI6STATCLR = _SPI6STAT_SPIROV_MASK;

    /*
    SRXISEL = 1 (Receive buffer is not empty)
    STXISEL = 3 (Transmit buffer is not full)
    MSTEN = 0
    CKP = 0
    CKE = 1
    MODE<32,16> = 0
    ENHBUF = 1
    */

    SPI6CONSET = (SPI6_CON_ENHBUF | SPI6_CON_MODE_32_MODE_16 | SPI6_CON_CKE | SPI6_CON_CKP | SPI6_CON_STXISEL | SPI6_CON_SRXISEL);

    /* Enable generation of interrupt on receiver overflow */
    SPI6CON2SET = _SPI6CON2_SPIROVEN_MASK;

    spi6Obj.rdInIndex = 0;
    spi6Obj.wrOutIndex = 0;
    spi6Obj.nWrBytes = 0;
    spi6Obj.errorStatus = SPI_SLAVE_ERROR_NONE;
    spi6Obj.callback = NULL ;
    spi6Obj.transferIsBusy = false ;
    spi6Obj.csInterruptPending = false;
    spi6Obj.rxInterruptActive = false;

    /* Set the Busy Pin to ready state */
    GPIO_PinWrite((GPIO_PIN)SPI6_BUSY_PIN, 0);

    /* Register callback and enable notifications on Chip Select logic level change */
    GPIO_PinInterruptCallbackRegister(SPI6_CS_PIN, SPI6_CS_Handler, (uintptr_t)NULL);
    GPIO_PinInterruptEnable(SPI6_CS_PIN);

    /* Enable SPI6 RX and Error Interrupts. TX interrupt will be enabled when a SPI write is submitted. */
    SPI6_ENABLE_RX_INT();
    SPI6_ENABLE_ERR_INT();

    /* Enable SPI6 */
    SPI6CONSET = _SPI6CON_ON_MASK;
}

/* For 16-bit/32-bit mode, the "size" must be specified in terms of 16-bit/32-bit words */
size_t SPI6_Read(void* pRdBuffer, size_t size)
{
    size_t rdSize = size;
    uint32_t rdInIndex = spi6Obj.rdInIndex;

    if (rdSize > rdInIndex)
    {
        rdSize = rdInIndex;
    }

    memcpy(pRdBuffer, SPI6_ReadBuffer, rdSize);

    return rdSize;
}

/* For 16-bit/32-bit mode, the "size" must be specified in terms of 16-bit/32-bit words */
size_t SPI6_Write(void* pWrBuffer, size_t size )
{
    size_t wrSize = size;

    SPI6_DISABLE_TX_INT();

    if (wrSize > SPI6_WRITE_BUFFER_SIZE)
    {
        wrSize = SPI6_WRITE_BUFFER_SIZE;
    }

    memcpy(SPI6_WriteBuffer, pWrBuffer, wrSize);

    spi6Obj.nWrBytes = wrSize;
    spi6Obj.wrOutIndex = 0;

    /* Fill up the FIFO as long as there are empty elements */
    while ((!(SPI6STAT & _SPI6STAT_SPITBF_MASK)) && (spi6Obj.wrOutIndex < spi6Obj.nWrBytes))
    {
        SPI6BUF = SPI6_WriteBuffer[spi6Obj.wrOutIndex++];
    }

    /* Enable TX interrupt */
    SPI6_ENABLE_TX_INT();

    return wrSize;
}

/* For 16-bit/32-bit mode, the return value is in terms of 16-bit/32-bit words */
size_t SPI6_ReadCountGet(void)
{
    return spi6Obj.rdInIndex;
}

/* For 16-bit/32-bit mode, the return value is in terms of 16-bit/32-bit words */
size_t SPI6_ReadBufferSizeGet(void)
{
    return SPI6_READ_BUFFER_SIZE;
}

/* For 16-bit/32-bit mode, the return value is in terms of 16-bit/32-bit words */
size_t SPI6_WriteBufferSizeGet(void)
{
    return SPI6_WRITE_BUFFER_SIZE;
}

void SPI6_CallbackRegister(SPI_SLAVE_CALLBACK callBack, uintptr_t context )
{
    spi6Obj.callback = callBack;

    spi6Obj.context = context;
}

/* The status is returned as busy when CS is asserted */
bool SPI6_IsBusy(void)
{
    return spi6Obj.transferIsBusy;
}

/* Drive the GPIO pin to indicate to SPI Master that the slave is ready now */
void SPI6_Ready(void)
{
    GPIO_PinWrite((GPIO_PIN)SPI6_BUSY_PIN, 0);
}

SPI_SLAVE_ERROR SPI6_ErrorGet(void)
{
    SPI_SLAVE_ERROR errorStatus = spi6Obj.errorStatus;

    spi6Obj.errorStatus = SPI_SLAVE_ERROR_NONE;

    return errorStatus;
}

static void SPI6_CS_Handler(GPIO_PIN pin, uintptr_t context)
{
    bool activeState = 0;

    if (GPIO_PinRead((GPIO_PIN)SPI6_CS_PIN) == activeState)
    {
        /* CS is asserted */
        spi6Obj.transferIsBusy = true;

        /* Drive busy line to active state */
        GPIO_PinWrite((GPIO_PIN)SPI6_BUSY_PIN, 1);
    }
    else
    {
        /* Give application callback only if RX interrupt is not preempted and RX interrupt is not pending to be serviced */

        if ((spi6Obj.rxInterruptActive == false) && ((IFS7 & _IFS7_SPI6RXIF_MASK) == 0))
        {
            /* CS is de-asserted */
            spi6Obj.transferIsBusy = false;

            spi6Obj.wrOutIndex = 0;
            spi6Obj.nWrBytes = 0;

            if(spi6Obj.callback != NULL)
            {
                spi6Obj.callback(spi6Obj.context);
            }

            /* Clear the read index. Application must read out the data by calling SPI6_Read API in the callback */
            spi6Obj.rdInIndex = 0;
        }
        else
        {
            /* If CS interrupt is serviced by either preempting the RX interrupt or RX interrupt is pending to be serviced,
             * then delegate the responsibility of giving application callback to the RX interrupt handler */

            spi6Obj.csInterruptPending = true;
        }
    }
}

void SPI6_ERR_InterruptHandler (void)
{
    spi6Obj.errorStatus = (SPI6STAT & _SPI6STAT_SPIROV_MASK);

    /* Clear the receive overflow flag */
    SPI6STATCLR = _SPI6STAT_SPIROV_MASK;

    SPI6_CLEAR_ERR_INT_FLAG();
}

void SPI6_TX_InterruptHandler (void)
{
    /* Fill up the FIFO as long as there are empty elements */
    while ((!(SPI6STAT & _SPI6STAT_SPITBF_MASK)) && (spi6Obj.wrOutIndex < spi6Obj.nWrBytes))
    {
        SPI6BUF = SPI6_WriteBuffer[spi6Obj.wrOutIndex++];
    }

    /* Clear the transmit interrupt flag */
    SPI6_CLEAR_TX_INT_FLAG();

    if (spi6Obj.wrOutIndex == spi6Obj.nWrBytes)
    {
        /* Nothing to transmit. Disable transmit interrupt. The last byte sent by the master will be shifted out automatically*/
        SPI6_DISABLE_TX_INT();
    }
}

void SPI6_RX_InterruptHandler (void)
{
    uint32_t receivedData = 0;

    spi6Obj.rxInterruptActive = true;

    while (!(SPI6STAT & _SPI6STAT_SPIRBE_MASK))
    {
        /* Receive buffer is not empty. Read the received data. */
        receivedData = SPI6BUF;

        if (spi6Obj.rdInIndex < SPI6_READ_BUFFER_SIZE)
        {
            SPI6_ReadBuffer[spi6Obj.rdInIndex++] = receivedData;
        }
    }

    /* Clear the receive interrupt flag */
    SPI6_CLEAR_RX_INT_FLAG();

    spi6Obj.rxInterruptActive = false;

    /* Check if CS interrupt occured before the RX interrupt and that CS interrupt delegated the responsibility to give
     * application callback to the RX interrupt */

    if (spi6Obj.csInterruptPending == true)
    {
        spi6Obj.csInterruptPending = false;
        spi6Obj.transferIsBusy = false;

        spi6Obj.wrOutIndex = 0;
        spi6Obj.nWrBytes = 0;

        if(spi6Obj.callback != NULL)
        {
            spi6Obj.callback(spi6Obj.context);
        }

        /* Clear the read index. Application must read out the data by calling SPI6_Read API in the callback */
        spi6Obj.rdInIndex = 0;
    }
}

