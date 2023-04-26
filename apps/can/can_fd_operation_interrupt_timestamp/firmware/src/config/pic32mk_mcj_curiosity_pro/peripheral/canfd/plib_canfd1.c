/*******************************************************************************
  CANFD Peripheral Library Interface Source File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_canfd1.c

  Summary:
    CANFD peripheral library interface.

  Description:
    This file defines the interface to the CANFD peripheral library. This
    library provides access to and control of the associated peripheral
    instance.

  Remarks:
    None.
*******************************************************************************/

//DOM-IGNORE-BEGIN
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
//DOM-IGNORE-END
// *****************************************************************************
// *****************************************************************************
// Header Includes
// *****************************************************************************
// *****************************************************************************
#include <sys/kmem.h>
#include "plib_canfd1.h"
#include "interrupts.h"


// *****************************************************************************
// *****************************************************************************
// Global Data
// *****************************************************************************
// *****************************************************************************
/* CAN1 Message memory size */
#define CANFD_MESSAGE_RAM_CONFIG_SIZE 232
/* Number of configured FIFO */
#define CANFD_NUM_OF_FIFO             2U
/* Maximum number of CAN Message buffers in each FIFO */
#define CANFD_FIFO_MESSAGE_BUFFER_MAX 32

#define CANFD_CONFIGURATION_MODE      0x4UL
#define CANFD_OPERATION_MODE          (0x0UL)
#define CANFD_NUM_OF_FILTER           1U
/* FIFO Offset in word (4 bytes) */
#define CANFD_FIFO_OFFSET             0xcU
/* Filter Offset in word (4 bytes) */
#define CANFD_FILTER_OFFSET           0x4U
#define CANFD_FILTER_OBJ_OFFSET       0x8U
/* Acceptance Mask Offset in word (4 bytes) */
#define CANFD_ACCEPTANCE_MASK_OFFSET  0x8U
#define CANFD_MSG_SID_MASK            (0x7FFU)
#define CANFD_MSG_EID_MASK            (0x1FFFFFFFU)
#define CANFD_MSG_DLC_MASK            (0x0000000FU)
#define CANFD_MSG_IDE_MASK            (0x00000010U)
#define CANFD_MSG_RTR_MASK            (0x00000020U)
#define CANFD_MSG_BRS_MASK            (0x00000040U)
#define CANFD_MSG_FDF_MASK            (0x00000080U)
#define CANFD_MSG_SEQ_MASK            (0xFFFFFE00U)
#define CANFD_MSG_TX_EXT_SID_MASK     (0x1FFC0000U)
#define CANFD_MSG_TX_EXT_EID_MASK     (0x0003FFFFU)
#define CANFD_MSG_RX_EXT_SID_MASK     (0x000007FFUL)
#define CANFD_MSG_RX_EXT_EID_MASK     (0x1FFFF800U)
#define CANFD_MSG_FLT_EXT_SID_MASK    (0x1FFC0000UL)
#define CANFD_MSG_FLT_EXT_EID_MASK    (0x0003FFFFU)

volatile static CANFD_OBJ can1Obj;
volatile static CANFD_RX_MSG can1RxMsg[CANFD_NUM_OF_FIFO][CANFD_FIFO_MESSAGE_BUFFER_MAX];
volatile static CANFD_CALLBACK_OBJ can1CallbackObj[CANFD_NUM_OF_FIFO + 1];
volatile static CANFD_CALLBACK_OBJ can1ErrorCallbackObj;
volatile static uint32_t can1MsgIndex[CANFD_NUM_OF_FIFO];
static uint8_t __attribute__((coherent, aligned(16))) can_message_buffer[CANFD_MESSAGE_RAM_CONFIG_SIZE];
static const uint8_t dlcToLength[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

/******************************************************************************
Local Functions
******************************************************************************/
static void CANLengthToDlcGet(uint8_t length, uint8_t *dlc)
{
    if (length <= 8U)
    {
        *dlc = length;
    }
    else if (length <= 12U)
    {
        *dlc = 0x9U;
    }
    else if (length <= 16U)
    {
        *dlc = 0xAU;
    }
    else if (length <= 20U)
    {
        *dlc = 0xBU;
    }
    else if (length <= 24U)
    {
        *dlc = 0xCU;
    }
    else if (length <= 32U)
    {
        *dlc = 0xDU;
    }
    else if (length <= 48U)
    {
        *dlc = 0xEU;
    }
    else
    {
        *dlc = 0xFU;
    }
}

static inline void CAN1_ZeroInitialize(volatile void* pData, size_t dataSize)
{
    volatile uint8_t* data = (volatile uint8_t*)pData;
    for (uint32_t index = 0; index < dataSize; index++)
    {
        data[index] = 0U;
    }
}

/* MISRAC 2012 deviation block start */
/* MISRA C-2012 Rule 11.6 deviated 5 times. Deviation record ID -  H3_MISRAC_2012_R_11_6_DR_1 */
/* MISRA C-2012 Rule 5.1 deviated 1 time. Deviation record ID -  H3_MISRAC_2012_R_5_1_DR_1 */

// *****************************************************************************
// *****************************************************************************
// CAN1 PLib Interface Routines
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
/* Function:
    void CAN1_Initialize(void)

   Summary:
    Initializes given instance of the CAN peripheral.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None
*/
void CAN1_Initialize(void)
{
    /* Switch the CAN module ON */
    CFD1CON |= _CFD1CON_ON_MASK;

    /* Switch the CAN module to Configuration mode. Wait until the switch is complete */
    CFD1CON = (CFD1CON & ~_CFD1CON_REQOP_MASK) | ((CANFD_CONFIGURATION_MODE << _CFD1CON_REQOP_POSITION) & _CFD1CON_REQOP_MASK);
    while(((CFD1CON & _CFD1CON_OPMOD_MASK) >> _CFD1CON_OPMOD_POSITION) != CANFD_CONFIGURATION_MODE)
    {
        /* Do Nothing */
    }

    /* Set the Data bitrate to 2000 Kbps */
    CFD1DBTCFG = ((1UL << _CFD1DBTCFG_BRP_POSITION) & _CFD1DBTCFG_BRP_MASK)
               | ((13UL << _CFD1DBTCFG_TSEG1_POSITION) & _CFD1DBTCFG_TSEG1_MASK)
               | ((4UL << _CFD1DBTCFG_TSEG2_POSITION) & _CFD1DBTCFG_TSEG2_MASK)
               | ((4UL << _CFD1DBTCFG_SJW_POSITION) & _CFD1DBTCFG_SJW_MASK);

    /* Set the Nominal bitrate to 500 Kbps */
    CFD1NBTCFG = ((7UL << _CFD1NBTCFG_BRP_POSITION) & _CFD1NBTCFG_BRP_MASK)
               | ((13UL << _CFD1NBTCFG_TSEG1_POSITION) & _CFD1NBTCFG_TSEG1_MASK)
               | ((4UL << _CFD1NBTCFG_TSEG2_POSITION) & _CFD1NBTCFG_TSEG2_MASK)
               | ((4UL << _CFD1NBTCFG_SJW_POSITION) & _CFD1NBTCFG_SJW_MASK);

    /* Set Message memory base address for all FIFOs/Queue */
    CFD1FIFOBA = (uint32_t)KVA_TO_PA(can_message_buffer);

    /* Tx Event FIFO Configuration */
    CFD1TEFCON = (((1UL - 1UL) << _CFD1TEFCON_FSIZE_POSITION) & _CFD1TEFCON_FSIZE_MASK) | _CFD1TEFCON_TEFTSEN_MASK;
    CFD1CON |= _CFD1CON_STEF_MASK;

    /* Tx Queue Configuration */
    CFD1TXQCON = (((1UL - 1UL) << _CFD1TXQCON_FSIZE_POSITION) & _CFD1TXQCON_FSIZE_MASK)
               | ((0x7UL << _CFD1TXQCON_PLSIZE_POSITION) & _CFD1TXQCON_PLSIZE_MASK)
               | ((0x0UL << _CFD1TXQCON_TXPRI_POSITION) & _CFD1TXQCON_TXPRI_MASK);
    CFD1CON |= _CFD1CON_TXQEN_MASK;


    /* Configure CAN FIFOs */
    CFD1FIFOCON1 = (((1UL - 1UL) << _CFD1FIFOCON1_FSIZE_POSITION) & _CFD1FIFOCON1_FSIZE_MASK) | _CFD1FIFOCON1_TXEN_MASK | ((0x0UL << _CFD1FIFOCON1_TXPRI_POSITION) & _CFD1FIFOCON1_TXPRI_MASK) | ((0x0UL << _CFD1FIFOCON1_RTREN_POSITION) & _CFD1FIFOCON1_RTREN_MASK) | ((0x7UL << _CFD1FIFOCON1_PLSIZE_POSITION) & _CFD1FIFOCON1_PLSIZE_MASK);
    CFD1FIFOCON2 = (((1UL - 1UL) << _CFD1FIFOCON2_FSIZE_POSITION) & _CFD1FIFOCON2_FSIZE_MASK) | _CFD1FIFOCON2_RXTSEN_MASK  | ((0x7UL << _CFD1FIFOCON2_PLSIZE_POSITION) & _CFD1FIFOCON2_PLSIZE_MASK);

    /* Configure CAN Filters */
    /* Filter 0 configuration */
    CFD1FLTOBJ0 = (0U & CANFD_MSG_SID_MASK);
    CFD1MASK0 = (0U & CANFD_MSG_SID_MASK);
    CFD1FLTCON0 |= (((0x2UL << _CFD1FLTCON0_F0BP_POSITION) & _CFD1FLTCON0_F0BP_MASK)| _CFD1FLTCON0_FLTEN0_MASK);

    /* Enable Timestamp */
    CFD1TSCON = (0U & _CFD1TSCON_TBCPRE_MASK)
                                | ((0x0UL << _CFD1TSCON_TSEOF_POSITION) & _CFD1TSCON_TSEOF_MASK)
| ((0x0UL << _CFD1TSCON_TSRES_POSITION) & _CFD1TSCON_TSRES_MASK)
                                | _CFD1TSCON_TBCEN_MASK;

    /* Set Interrupts */
    IEC5SET = _IEC5_CAN1IE_MASK;
    CFD1INT |= _CFD1INT_SERRIE_MASK | _CFD1INT_CERRIE_MASK | _CFD1INT_IVMIE_MASK;

    /* Initialize the CAN PLib Object */
   CAN1_ZeroInitialize(can1RxMsg, sizeof(can1RxMsg));

    /* Switch the CAN module to CANFD_OPERATION_MODE. Wait until the switch is complete */
    CFD1CON = (CFD1CON & ~_CFD1CON_REQOP_MASK) | ((CANFD_OPERATION_MODE << _CFD1CON_REQOP_POSITION) & _CFD1CON_REQOP_MASK);
    while(((CFD1CON & _CFD1CON_OPMOD_MASK) >> _CFD1CON_OPMOD_POSITION) != CANFD_OPERATION_MODE)
    {
        /* Do Nothing */
    }
}

// *****************************************************************************
/* Function:
    bool CAN1_MessageTransmit(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoQueueNum, CANFD_MODE mode, CANFD_MSG_TX_ATTRIBUTE msgAttr)

   Summary:
    Transmits a message into CAN bus.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    id           - 11-bit / 29-bit identifier (ID).
    length       - Length of data buffer in number of bytes.
    data         - Pointer to source data buffer
    fifoQueueNum - If fifoQueueNum is 0 then Transmit Queue otherwise FIFO
    mode         - CAN mode Classic CAN or CAN FD without BRS or CAN FD with BRS
    msgAttr      - Data frame or Remote frame to be transmitted

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_MessageTransmit(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoQueueNum, CANFD_MODE mode, CANFD_MSG_TX_ATTRIBUTE msgAttr)
{
    CANFD_TX_MSG_OBJECT *txMessage = NULL;
    static uint32_t sequence = 0;
    uint8_t count = 0;
    uint8_t dlc = 0;
    bool status = false;

    if (fifoQueueNum == 0U)
    {
        if ((CFD1TXQSTA & _CFD1TXQSTA_TXQNIF_MASK) == _CFD1TXQSTA_TXQNIF_MASK)
        {
            txMessage = (CANFD_TX_MSG_OBJECT *)PA_TO_KVA1(CFD1TXQUA);
            status = true;
        }
    }
    else if (fifoQueueNum <= CANFD_NUM_OF_FIFO)
    {
        if ((*(volatile uint32_t *)(&CFD1FIFOSTA1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) & _CFD1FIFOSTA1_TFNRFNIF_MASK) == _CFD1FIFOSTA1_TFNRFNIF_MASK)
        {
            txMessage = (CANFD_TX_MSG_OBJECT *)PA_TO_KVA1(*(volatile uint32_t *)(&CFD1FIFOUA1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)));
            status = true;
        }
    }
    else
    {
        /* Do Nothing */
    }

    if (status)
    {
        /* Check the id whether it falls under SID or EID,
         * SID max limit is 0x7FF, so anything beyond that is EID */
        if (id > CANFD_MSG_SID_MASK)
        {
            txMessage->t0 = (((id & CANFD_MSG_TX_EXT_SID_MASK) >> 18) | ((id & CANFD_MSG_TX_EXT_EID_MASK) << 11)) & CANFD_MSG_EID_MASK;
            txMessage->t1 = CANFD_MSG_IDE_MASK;
        }
        else
        {
            txMessage->t0 = id;
            txMessage->t1 = 0;
        }
        if (length > 64U)
        {
            length = 64;
        }

        CANLengthToDlcGet(length, &dlc);

        txMessage->t1 |= ((uint32_t)dlc & (uint32_t)CANFD_MSG_DLC_MASK);

        if(mode == CANFD_MODE_FD_WITH_BRS)
        {
            txMessage->t1 |= CANFD_MSG_FDF_MASK | CANFD_MSG_BRS_MASK;
        }
        else if (mode == CANFD_MODE_FD_WITHOUT_BRS)
        {
            txMessage->t1 |= CANFD_MSG_FDF_MASK;
        }
        else
        {
            /* Do Nothing */
        }
        if (msgAttr == CANFD_MSG_TX_REMOTE_FRAME)
        {
            txMessage->t1 |= CANFD_MSG_RTR_MASK;
        }
        else
        {
            while(count < length)
            {
                txMessage->data[count] = *data;
                count++;
                data++;
            }
        }

        ++sequence;
        txMessage->t1 |= ((sequence << 9) & CANFD_MSG_SEQ_MASK);

        if (fifoQueueNum == 0U)
        {
            CFD1TXQCON |= _CFD1TXQCON_TXQEIE_MASK;

            /* Request the transmit */
            CFD1TXQCON |= _CFD1TXQCON_UINC_MASK;
            CFD1TXQCON |= _CFD1TXQCON_TXREQ_MASK;
        }
        else
        {
            *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_TFERFFIE_MASK;

            /* Request the transmit */
            *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_UINC_MASK;
            *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_TXREQ_MASK;
        }
        CFD1INT |= _CFD1INT_TXIE_MASK;
    }
    return status;
}

// *****************************************************************************
/* Function:
    bool CAN1_MessageReceive(uint32_t *id, uint8_t *length, uint8_t *data, uint32_t *timestamp, uint8_t fifoNum, CANFD_MSG_RX_ATTRIBUTE *msgAttr)

   Summary:
    Receives a message from CAN bus.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    id          - Pointer to 11-bit / 29-bit identifier (ID) to be received.
    length      - Pointer to data length in number of bytes to be received.
    data        - Pointer to destination data buffer
    timestamp   - Pointer to Rx message timestamp, timestamp value is 0 if Timestamp is disabled in CFD1TSCON
    fifoNum     - FIFO number
    msgAttr     - Data frame or Remote frame to be received

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_MessageReceive(uint32_t *id, uint8_t *length, uint8_t *data, uint32_t *timestamp, uint8_t fifoNum, CANFD_MSG_RX_ATTRIBUTE *msgAttr)
{
    bool status = false;
    uint8_t msgIndex = 0;
    uint8_t fifoSize = 0;

    if ((fifoNum > CANFD_NUM_OF_FIFO) || (id == NULL))
    {
        return status;
    }

    fifoSize =(uint8_t)((*(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) & _CFD1FIFOCON1_FSIZE_MASK) >> _CFD1FIFOCON1_FSIZE_POSITION);
    for (msgIndex = 0; msgIndex <= fifoSize; msgIndex++)
    {
        if ((can1MsgIndex[fifoNum-1U] & (1UL << (msgIndex & 0x1FU))) == 0U)
        {
            can1MsgIndex[fifoNum-1U] |= (1UL << (msgIndex & 0x1FU));
            break;
        }
    }
    if(msgIndex > fifoSize)
    {
        /* FIFO is full */
        return false;
    }
    can1RxMsg[fifoNum-1U][msgIndex].id = id;
    can1RxMsg[fifoNum-1U][msgIndex].buffer = data;
    can1RxMsg[fifoNum-1U][msgIndex].size = length;
    can1RxMsg[fifoNum-1U][msgIndex].timestamp = timestamp;
    can1RxMsg[fifoNum-1U][msgIndex].msgAttr = msgAttr;
    *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_TFNRFNIE_MASK;
    CFD1INT |= _CFD1INT_RXIE_MASK;
    status = true;

    return status;
}

// *****************************************************************************
/* Function:
    void CAN1_MessageAbort(uint8_t fifoQueueNum)

   Summary:
    Abort request for a Queue/FIFO.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    fifoQueueNum - If fifoQueueNum is 0 then Transmit Queue otherwise FIFO

   Returns:
    None.
*/
void CAN1_MessageAbort(uint8_t fifoQueueNum)
{
    if (fifoQueueNum == 0U)
    {
        CFD1TXQCON &= ~_CFD1TXQCON_TXREQ_MASK;
    }
    else if (fifoQueueNum <= CANFD_NUM_OF_FIFO)
    {
        *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) &= ~_CFD1FIFOCON1_TXREQ_MASK;
    }
    else
    {
        /* Do Nothing */
    }
}

// *****************************************************************************
/* Function:
    void CAN1_MessageAcceptanceFilterSet(uint8_t filterNum, uint32_t id)

   Summary:
    Set Message acceptance filter configuration.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    filterNum - Filter number
    id        - 11-bit or 29-bit identifier

   Returns:
    None.
*/
void CAN1_MessageAcceptanceFilterSet(uint8_t filterNum, uint32_t id)
{
    uint32_t filterEnableBit = 0;
    uint8_t filterRegIndex = 0;

    if (filterNum < CANFD_NUM_OF_FILTER)
    {
        filterRegIndex = filterNum >> 2;
        filterEnableBit = _CFD1FLTCON0_FLTEN0_MASK;

        *(volatile uint32_t *)(&CFD1FLTCON0 + (filterRegIndex * CANFD_FILTER_OFFSET)) &= ~filterEnableBit;

        if (id > CANFD_MSG_SID_MASK)
        {
            *(volatile uint32_t *)(&CFD1FLTOBJ0 + (filterNum * CANFD_FILTER_OBJ_OFFSET)) = ((((id & CANFD_MSG_FLT_EXT_SID_MASK) >> 18) | ((id & CANFD_MSG_FLT_EXT_EID_MASK) << 11)) & CANFD_MSG_EID_MASK) | _CFD1FLTOBJ0_EXIDE_MASK;
        }
        else
        {
            *(volatile uint32_t *)(&CFD1FLTOBJ0 + (filterNum * CANFD_FILTER_OBJ_OFFSET)) = id & CANFD_MSG_SID_MASK;
        }
        *(volatile uint32_t *)(&CFD1FLTCON0 + (filterRegIndex * CANFD_FILTER_OFFSET)) |= filterEnableBit;
    }
}

// *****************************************************************************
/* Function:
    uint32_t CAN1_MessageAcceptanceFilterGet(uint8_t filterNum)

   Summary:
    Get Message acceptance filter configuration.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    filterNum - Filter number

   Returns:
    Returns Message acceptance filter identifier
*/
uint32_t CAN1_MessageAcceptanceFilterGet(uint8_t filterNum)
{
    uint32_t id = 0;

    if (filterNum < CANFD_NUM_OF_FILTER)
    {
        if ((*(volatile uint32_t *)(&CFD1FLTOBJ0 + (filterNum * CANFD_FILTER_OBJ_OFFSET)) & _CFD1FLTOBJ0_EXIDE_MASK) != 0U)
        {
            id = (*(volatile uint32_t *)(&CFD1FLTOBJ0 + (filterNum * CANFD_FILTER_OBJ_OFFSET)) & CANFD_MSG_RX_EXT_SID_MASK) << 18;
            id = (id | ((*(volatile uint32_t *)(&CFD1FLTOBJ0 + (filterNum * CANFD_FILTER_OBJ_OFFSET)) & CANFD_MSG_RX_EXT_EID_MASK) >> 11))
               & CANFD_MSG_EID_MASK;
        }
        else
        {
            id = (*(volatile uint32_t *)(&CFD1FLTOBJ0 + (filterNum * CANFD_FILTER_OBJ_OFFSET)) & CANFD_MSG_SID_MASK);
        }
    }
    return id;
}

// *****************************************************************************
/* Function:
    void CAN1_MessageAcceptanceFilterMaskSet(uint8_t acceptanceFilterMaskNum, uint32_t id)

   Summary:
    Set Message acceptance filter mask configuration.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    acceptanceFilterMaskNum - Acceptance Filter Mask number
    id                      - 11-bit or 29-bit identifier

   Returns:
    None.
*/
void CAN1_MessageAcceptanceFilterMaskSet(uint8_t acceptanceFilterMaskNum, uint32_t id)
{
    /* Switch the CAN module to Configuration mode. Wait until the switch is complete */
    CFD1CON = (CFD1CON & ~_CFD1CON_REQOP_MASK) | ((CANFD_CONFIGURATION_MODE << _CFD1CON_REQOP_POSITION) & _CFD1CON_REQOP_MASK);
    while(((CFD1CON & _CFD1CON_OPMOD_MASK) >> _CFD1CON_OPMOD_POSITION) != CANFD_CONFIGURATION_MODE)
    {
        /* Do Nothing */

    }

    if (id > CANFD_MSG_SID_MASK)
    {
        *(volatile uint32_t *)(&CFD1MASK0 + (acceptanceFilterMaskNum * CANFD_ACCEPTANCE_MASK_OFFSET)) = ((((id & CANFD_MSG_FLT_EXT_SID_MASK) >> 18) | ((id & CANFD_MSG_FLT_EXT_EID_MASK) << 11)) & CANFD_MSG_EID_MASK) | _CFD1MASK0_MIDE_MASK;
    }
    else
    {
        *(volatile uint32_t *)(&CFD1MASK0 + (acceptanceFilterMaskNum * CANFD_ACCEPTANCE_MASK_OFFSET)) = id & CANFD_MSG_SID_MASK;
    }

    /* Switch the CAN module to CANFD_OPERATION_MODE. Wait until the switch is complete */
    CFD1CON = (CFD1CON & ~_CFD1CON_REQOP_MASK) | ((CANFD_OPERATION_MODE << _CFD1CON_REQOP_POSITION) & _CFD1CON_REQOP_MASK);
    while(((CFD1CON & _CFD1CON_OPMOD_MASK) >> _CFD1CON_OPMOD_POSITION) != CANFD_OPERATION_MODE)
    {
        /* Do Nothing */

    }
}

// *****************************************************************************
/* Function:
    uint32_t CAN1_MessageAcceptanceFilterMaskGet(uint8_t acceptanceFilterMaskNum)

   Summary:
    Get Message acceptance filter mask configuration.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    acceptanceFilterMaskNum - Acceptance Filter Mask number

   Returns:
    Returns Message acceptance filter mask.
*/
uint32_t CAN1_MessageAcceptanceFilterMaskGet(uint8_t acceptanceFilterMaskNum)
{
    uint32_t id = 0;

    if ((*(volatile uint32_t *)(&CFD1MASK0 + (acceptanceFilterMaskNum * CANFD_ACCEPTANCE_MASK_OFFSET)) & _CFD1MASK0_MIDE_MASK) != 0U)
    {
        id = (*(volatile uint32_t *)(&CFD1MASK0 + (acceptanceFilterMaskNum * CANFD_ACCEPTANCE_MASK_OFFSET)) & CANFD_MSG_RX_EXT_SID_MASK) << 18;
        id = (id | ((*(volatile uint32_t *)(&CFD1MASK0 + (acceptanceFilterMaskNum * CANFD_ACCEPTANCE_MASK_OFFSET)) & CANFD_MSG_RX_EXT_EID_MASK) >> 11))
           & CANFD_MSG_EID_MASK;
    }
    else
    {
        id = (*(volatile uint32_t *)(&CFD1MASK0 + (acceptanceFilterMaskNum * CANFD_ACCEPTANCE_MASK_OFFSET)) & CANFD_MSG_SID_MASK);
    }
    return id;
}

// *****************************************************************************
/* Function:
    bool CAN1_TransmitEventFIFOElementGet(uint32_t *id, uint32_t *sequence, uint32_t *timestamp)

   Summary:
    Get the Transmit Event FIFO Element for the transmitted message.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    id          - Pointer to 11-bit / 29-bit identifier (ID) to be received.
    sequence    - Pointer to Tx message sequence number to be received
    timestamp   - Pointer to Tx message timestamp to be received, timestamp value is 0 if Timestamp is disabled in CFD1TSCON

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_TransmitEventFIFOElementGet(uint32_t *id, uint32_t *sequence, uint32_t *timestamp)
{
    CANFD_TX_EVENT_FIFO_ELEMENT *txEventFIFOElement = NULL;
    bool status = false;

    /* Check if there is a message available in Tx Event FIFO */
    if ((CFD1TEFSTA & _CFD1TEFSTA_TEFNEIF_MASK) == _CFD1TEFSTA_TEFNEIF_MASK)
    {
        /* Get a pointer to Tx Event FIFO Element */
        txEventFIFOElement = (CANFD_TX_EVENT_FIFO_ELEMENT *)PA_TO_KVA1(CFD1TEFUA);

        /* Check if it's a extended message type */
        if ((txEventFIFOElement->te1 & CANFD_MSG_IDE_MASK) != 0U)
        {
            *id = txEventFIFOElement->te0 & CANFD_MSG_EID_MASK;
        }
        else
        {
            *id = txEventFIFOElement->te0 & CANFD_MSG_SID_MASK;
        }

        *sequence = ((txEventFIFOElement->te1 & CANFD_MSG_SEQ_MASK) >> 9);

        if (timestamp != NULL)
        {
            *timestamp =  (((uint32_t)txEventFIFOElement->timestamp[3] << 24) | ((uint32_t)txEventFIFOElement->timestamp[2] << 16) | ((uint32_t)txEventFIFOElement->timestamp[1] << 8) | (uint32_t)txEventFIFOElement->timestamp[0]);
        }

        /* Tx Event FIFO Element read done, update the Tx Event FIFO tail */
        CFD1TEFCON |= _CFD1TEFCON_UINC_MASK;

        /* Tx Event FIFO Element read successfully, so return true */
        status = true;
    }
    return status;
}

// *****************************************************************************
/* Function:
    CANFD_ERROR CAN1_ErrorGet(void)

   Summary:
    Returns the error during transfer.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    None.

   Returns:
    Error during transfer.
*/
CANFD_ERROR CAN1_ErrorGet(void)
{
    return (CANFD_ERROR)can1Obj.errorStatus;
}

// *****************************************************************************
/* Function:
    void CAN1_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount)

   Summary:
    Returns the transmit and receive error count during transfer.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    txErrorCount - Transmit Error Count to be received
    rxErrorCount - Receive Error Count to be received

   Returns:
    None.
*/
void CAN1_ErrorCountGet(uint8_t *txErrorCount, uint8_t *rxErrorCount)
{
    *txErrorCount = (uint8_t)((CFD1TREC & _CFD1TREC_TERRCNT_MASK) >> _CFD1TREC_TERRCNT_POSITION);
    *rxErrorCount = (uint8_t)(CFD1TREC & _CFD1TREC_RERRCNT_MASK);
}

// *****************************************************************************
/* Function:
    bool CAN1_InterruptGet(uint8_t fifoQueueNum, CANFD_FIFO_INTERRUPT_FLAG_MASK fifoInterruptFlagMask)

   Summary:
    Returns the FIFO Interrupt status.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    fifoQueueNum          - FIFO number
    fifoInterruptFlagMask - FIFO interrupt flag mask

   Returns:
    true - Requested fifo interrupt is occurred.
    false - Requested fifo interrupt is not occurred.
*/
bool CAN1_InterruptGet(uint8_t fifoQueueNum, CANFD_FIFO_INTERRUPT_FLAG_MASK fifoInterruptFlagMask)
{
    if (fifoQueueNum == 0U)
    {
        return ((CFD1TXQSTA & (uint32_t)fifoInterruptFlagMask) != 0x0U);
    }
    else
    {
        return ((*(volatile uint32_t *)(&CFD1FIFOSTA1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) & (uint32_t)fifoInterruptFlagMask) != 0x0U);
    }
}

// *****************************************************************************
/* Function:
    bool CAN1_TxFIFOQueueIsFull(uint8_t fifoQueueNum)

   Summary:
    Returns true if Tx FIFO/Queue is full otherwise false.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    fifoQueueNum - FIFO/Queue number

   Returns:
    true  - Tx FIFO/Queue is full.
    false - Tx FIFO/Queue is not full.
*/
bool CAN1_TxFIFOQueueIsFull(uint8_t fifoQueueNum)
{
    if (fifoQueueNum == 0U)
    {
        return ((CFD1TXQSTA & _CFD1TXQSTA_TXQNIF_MASK) != _CFD1TXQSTA_TXQNIF_MASK);
    }
    else
    {
        return ((*(volatile uint32_t *)(&CFD1FIFOSTA1 + ((fifoQueueNum - 1U) * CANFD_FIFO_OFFSET)) & _CFD1FIFOSTA1_TFNRFNIF_MASK) != _CFD1FIFOSTA1_TFNRFNIF_MASK);
    }
}

// *****************************************************************************
/* Function:
    bool CAN1_AutoRTRResponseSet(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoNum)

   Summary:
    Set the Auto RTR response for remote transmit request.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.
    Auto RTR Enable must be set to 0x1 for the requested Transmit FIFO in MHC configuration.

   Parameters:
    id           - 11-bit / 29-bit identifier (ID).
    length       - Length of data buffer in number of bytes.
    data         - Pointer to source data buffer
    fifoNum      - FIFO Number

   Returns:
    Request status.
    true  - Request was successful.
    false - Request has failed.
*/
bool CAN1_AutoRTRResponseSet(uint32_t id, uint8_t length, uint8_t* data, uint8_t fifoNum)
{
    CANFD_TX_MSG_OBJECT *txMessage = NULL;
    uint8_t count = 0;
    bool status = false;

    if (fifoNum <= CANFD_NUM_OF_FIFO)
    {
        if ((*(volatile uint32_t *)(&CFD1FIFOSTA1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) & _CFD1FIFOSTA1_TFNRFNIF_MASK) == _CFD1FIFOSTA1_TFNRFNIF_MASK)
        {
            txMessage = (CANFD_TX_MSG_OBJECT *)PA_TO_KVA1(*(volatile uint32_t *)(&CFD1FIFOUA1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)));
            status = true;
        }
    }

    if (status)
    {
        /* Check the id whether it falls under SID or EID,
         * SID max limit is 0x7FF, so anything beyond that is EID */
        if (id > CANFD_MSG_SID_MASK)
        {
            txMessage->t0 = (((id & CANFD_MSG_TX_EXT_SID_MASK) >> 18) | ((id & CANFD_MSG_TX_EXT_EID_MASK) << 11)) & CANFD_MSG_EID_MASK;
            txMessage->t1 = CANFD_MSG_IDE_MASK;
        }
        else
        {
            txMessage->t0 = id;
            txMessage->t1 = 0U;
        }

        /* Limit length */
        if (length > 8U)
        {
            length = 8U;
        }
        txMessage->t1 |= length;

        while(count < length)
        {
            txMessage->data[count] = *data;
            count++;
            data++;
        }

        *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_TFERFFIE_MASK;

        /* Set UINC to respond to RTR */
        *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_UINC_MASK;
        CFD1INT |= _CFD1INT_TXIE_MASK;
    }
    return status;
}

bool CAN1_BitTimingCalculationGet(CAN_BIT_TIMING_SETUP *setup, CAN_BIT_TIMING *bitTiming)
{
    bool status = false;
    uint32_t numOfTimeQuanta;
    uint8_t tseg1;
    float temp1;
    float temp2;

    if ((setup != NULL) && (bitTiming != NULL))
    {
        if (setup->nominalBitTimingSet == true)
        {
            numOfTimeQuanta = CAN1_CLOCK_FREQUENCY / (setup->nominalBitRate * ((uint32_t)setup->nominalPrescaler + 1U));
            if ((numOfTimeQuanta >= 4U) && (numOfTimeQuanta <= 385U))
            {
                if (setup->nominalSamplePoint < 50.0f)
                {
                    setup->nominalSamplePoint = 50.0f;
                }
                temp1 = (float)numOfTimeQuanta;
                temp2 = (temp1 * setup->nominalSamplePoint) / 100.0f;
                tseg1 = (uint8_t)temp2;
                bitTiming->nominalBitTiming.nominalTimeSegment2 = (uint8_t)(numOfTimeQuanta - tseg1 - 1U);
                bitTiming->nominalBitTiming.nominalTimeSegment1 = tseg1 - 2U;
                bitTiming->nominalBitTiming.nominalSJW = bitTiming->nominalBitTiming.nominalTimeSegment2;
                bitTiming->nominalBitTiming.nominalPrescaler = setup->nominalPrescaler;
                bitTiming->nominalBitTimingSet = true;
                status = true;
            }
            else
            {
                bitTiming->nominalBitTimingSet = false;
            }
        }
        if (setup->dataBitTimingSet == true)
        {
            numOfTimeQuanta = CAN1_CLOCK_FREQUENCY / (setup->dataBitRate * ((uint32_t)setup->dataPrescaler + 1U));
            if ((numOfTimeQuanta >= 4U) && (numOfTimeQuanta <= 49U))
            {
                if (setup->dataSamplePoint < 50.0f)
                {
                    setup->dataSamplePoint = 50.0f;
                }
                temp1 = (float)numOfTimeQuanta;
                temp2 = (temp1 * setup->dataSamplePoint) / 100.0f;
                tseg1 = (uint8_t)temp2;
                bitTiming->dataBitTiming.dataTimeSegment2 = (uint8_t)(numOfTimeQuanta - tseg1 - 1U);
                bitTiming->dataBitTiming.dataTimeSegment1 = tseg1 - 2U;
                bitTiming->dataBitTiming.dataSJW = bitTiming->dataBitTiming.dataTimeSegment2;
                bitTiming->dataBitTiming.dataPrescaler = setup->dataPrescaler;
                bitTiming->dataBitTimingSet = true;
                status = true;
            }
            else
            {
                bitTiming->dataBitTimingSet = false;
                status = false;
            }
        }
    }

    return status;
}

bool CAN1_BitTimingSet(CAN_BIT_TIMING *bitTiming)
{
    bool status = false;
    bool nominalBitTimingSet = false;
    bool dataBitTimingSet = false;

    if ((bitTiming->nominalBitTimingSet == true)
    && (bitTiming->nominalBitTiming.nominalTimeSegment1 >= 0x1U)
    && (bitTiming->nominalBitTiming.nominalTimeSegment2 <= 0x7FU)
    && (bitTiming->nominalBitTiming.nominalSJW <= 0x7FU))
    {
        nominalBitTimingSet = true;
    }

    if  ((bitTiming->dataBitTimingSet == true)
    &&  (bitTiming->dataBitTiming.dataTimeSegment1 <= 0x1FU)
    &&  (bitTiming->dataBitTiming.dataTimeSegment2 <= 0xFU)
    &&  (bitTiming->dataBitTiming.dataSJW <= 0xFU))
    {
        dataBitTimingSet = true;
    }

    if ((nominalBitTimingSet == true) || (dataBitTimingSet == true))
    {
        /* Switch the CAN module to Configuration mode. Wait until the switch is complete */
        CFD1CON = (CFD1CON & ~_CFD1CON_REQOP_MASK) | ((CANFD_CONFIGURATION_MODE << _CFD1CON_REQOP_POSITION) & _CFD1CON_REQOP_MASK);
        while(((CFD1CON & _CFD1CON_OPMOD_MASK) >> _CFD1CON_OPMOD_POSITION) != CANFD_CONFIGURATION_MODE)
        {
            /* Do Nothing */
        }

        if (dataBitTimingSet == true)
        {
            /* Set the Data bitrate */
            CFD1DBTCFG = (((uint32_t)bitTiming->dataBitTiming.dataPrescaler << _CFD1DBTCFG_BRP_POSITION) & _CFD1DBTCFG_BRP_MASK)
                       | (((uint32_t)bitTiming->dataBitTiming.dataTimeSegment1 << _CFD1DBTCFG_TSEG1_POSITION) & _CFD1DBTCFG_TSEG1_MASK)
                       | (((uint32_t)bitTiming->dataBitTiming.dataTimeSegment2 << _CFD1DBTCFG_TSEG2_POSITION) & _CFD1DBTCFG_TSEG2_MASK)
                       | (((uint32_t)bitTiming->dataBitTiming.dataSJW << _CFD1DBTCFG_SJW_POSITION) & _CFD1DBTCFG_SJW_MASK);
        }

        if (nominalBitTimingSet == true)
        {
            /* Set the Nominal bitrate */
            CFD1NBTCFG = (((uint32_t)bitTiming->nominalBitTiming.nominalPrescaler << _CFD1NBTCFG_BRP_POSITION) & _CFD1NBTCFG_BRP_MASK)
                       | (((uint32_t)bitTiming->nominalBitTiming.nominalTimeSegment1 << _CFD1NBTCFG_TSEG1_POSITION) & _CFD1NBTCFG_TSEG1_MASK)
                       | (((uint32_t)bitTiming->nominalBitTiming.nominalTimeSegment2 << _CFD1NBTCFG_TSEG2_POSITION) & _CFD1NBTCFG_TSEG2_MASK)
                       | (((uint32_t)bitTiming->nominalBitTiming.nominalSJW << _CFD1NBTCFG_SJW_POSITION) & _CFD1NBTCFG_SJW_MASK);
        }

        /* Switch the CAN module to CANFD_OPERATION_MODE. Wait until the switch is complete */
        CFD1CON = (CFD1CON & ~_CFD1CON_REQOP_MASK) | ((CANFD_OPERATION_MODE << _CFD1CON_REQOP_POSITION) & _CFD1CON_REQOP_MASK);
        while(((CFD1CON & _CFD1CON_OPMOD_MASK) >> _CFD1CON_OPMOD_POSITION) != CANFD_OPERATION_MODE)
        {
            /* Do Nothing */
        }

        status = true;
    }
    return status;
}

// *****************************************************************************
/* Function:
    void CAN1_CallbackRegister(CANFD_CALLBACK callback, uintptr_t contextHandle, uint8_t fifoQueueNum)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given CAN's transfer events occur.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the CANFD_CALLBACK data type.
    fifoQueueNum - Tx Queue or Tx/Rx FIFO number

    context - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void CAN1_CallbackRegister(CANFD_CALLBACK callback, uintptr_t contextHandle, uint8_t fifoQueueNum)
{
    if (callback != NULL)
    {
        can1CallbackObj[fifoQueueNum].callback = callback;
        can1CallbackObj[fifoQueueNum].context = contextHandle;
    }
}

// *****************************************************************************
/* Function:
    void CAN1_ErrorCallbackRegister(CANFD_CALLBACK callback, uintptr_t contextHandle)

   Summary:
    Sets the pointer to the function (and it's context) to be called when the
    given CAN's transfer events occur.

   Precondition:
    CAN1_Initialize must have been called for the associated CAN instance.

   Parameters:
    callback - A pointer to a function with a calling signature defined
    by the CANFD_CALLBACK data type.

    context - A value (usually a pointer) passed (unused) into the function
    identified by the callback parameter.

   Returns:
    None.
*/
void CAN1_ErrorCallbackRegister(CANFD_CALLBACK callback, uintptr_t contextHandle)
{
    if (callback != NULL)
    {
        can1ErrorCallbackObj.callback = callback;
        can1ErrorCallbackObj.context = contextHandle;
    }
}

static void __attribute__((used)) CAN1_RX_InterruptHandler(void)
{
    uint8_t  msgIndex = 0;
    uint8_t  fifoNum = 0;
    uint8_t  fifoSize = 0;
    uint8_t  count = 0;
    CANFD_RX_MSG_OBJECT *rxMessage = NULL;
    uint8_t dataIndex = 4;
    /* Additional temporary variable used to prevent MISRA violations (Rule 13.x) */
    uintptr_t context;

    fifoNum = ((uint8_t)CFD1VEC & (uint8_t)_CFD1VEC_ICODE_MASK);
    if (fifoNum <= CANFD_NUM_OF_FIFO)
    {
        fifoSize = (uint8_t)((*(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) & _CFD1FIFOCON1_FSIZE_MASK) >> _CFD1FIFOCON1_FSIZE_POSITION);
        for (msgIndex = 0U; msgIndex <= fifoSize; msgIndex++)
        {
            if ((can1MsgIndex[fifoNum-1U] & (1UL << (msgIndex & 0x1FU))) == (1UL << (msgIndex & 0x1FU)))
            {
                can1MsgIndex[fifoNum-1U] &= ~(1UL << (msgIndex & 0x1FU));
                break;
            }
        }

        /* Get a pointer to RX message buffer */
        rxMessage = (CANFD_RX_MSG_OBJECT *)PA_TO_KVA1(*(volatile uint32_t *)(&CFD1FIFOUA1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)));


        /* Check if it's a extended message type */
        if ((rxMessage->r1 & CANFD_MSG_IDE_MASK) != 0U)
        {
            *can1RxMsg[fifoNum-1U][msgIndex].id = (((rxMessage->r0 & CANFD_MSG_RX_EXT_SID_MASK) << 18) | ((rxMessage->r0 & CANFD_MSG_RX_EXT_EID_MASK) >> 11)) & CANFD_MSG_EID_MASK;
        }
        else
        {
            *can1RxMsg[fifoNum-1U][msgIndex].id = rxMessage->r0 & CANFD_MSG_SID_MASK;
        }

        if (((rxMessage->r1 & CANFD_MSG_RTR_MASK) != 0U) && ((rxMessage->r1 & CANFD_MSG_FDF_MASK) == 0U))
        {
            *can1RxMsg[fifoNum-1U][msgIndex].msgAttr = CANFD_MSG_RX_REMOTE_FRAME;
        }
        else
        {
            *can1RxMsg[fifoNum-1U][msgIndex].msgAttr = CANFD_MSG_RX_DATA_FRAME;
        }

        *can1RxMsg[fifoNum-1U][msgIndex].size = dlcToLength[(rxMessage->r1 & CANFD_MSG_DLC_MASK)];

        if (can1RxMsg[fifoNum-1U][msgIndex].timestamp != NULL)
        {
            *can1RxMsg[fifoNum-1U][msgIndex].timestamp =  (((uint32_t)rxMessage->data[3] << 24) | ((uint32_t)rxMessage->data[2] << 16) | ((uint32_t)rxMessage->data[1] << 8) | (uint32_t)rxMessage->data[0]);
        }

        /* Copy the data into the payload */
        while (count < *can1RxMsg[fifoNum-1U][msgIndex].size)
        {
            *can1RxMsg[fifoNum-1U][msgIndex].buffer = rxMessage->data[dataIndex + count];
            can1RxMsg[fifoNum-1U][msgIndex].buffer++;
            count++;
        }

        /* Message processing is done, update the message buffer pointer. */
        *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) |= _CFD1FIFOCON1_UINC_MASK;

        if (((*(volatile uint32_t *)(&CFD1FIFOSTA1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) & _CFD1FIFOSTA1_TFNRFNIF_MASK) != _CFD1FIFOSTA1_TFNRFNIF_MASK) ||
            (can1MsgIndex[fifoNum-1U] == 0U))
        {
            *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) &= ~_CFD1FIFOCON1_TFNRFNIE_MASK;
        }
        can1Obj.errorStatus = 0U;
    }
    IFS5CLR = _IFS5_CAN1IF_MASK;

    if (can1CallbackObj[fifoNum].callback != NULL)
    {
        context = can1CallbackObj[fifoNum].context;
        can1CallbackObj[fifoNum].callback(context);
    }
}

static void __attribute__((used)) CAN1_TX_InterruptHandler(void)
{
    uint8_t  fifoNum = 0;
    /* Additional temporary variable used to prevent MISRA violations (Rule 13.x) */
    uintptr_t context;

    fifoNum = ((uint8_t)CFD1VEC & (uint8_t)_CFD1VEC_ICODE_MASK);
    if (fifoNum <= CANFD_NUM_OF_FIFO)
    {
        if (fifoNum == 0U)
        {
            CFD1TXQCON &= ~_CFD1TXQCON_TXQEIE_MASK;
        }
        else
        {
            *(volatile uint32_t *)(&CFD1FIFOCON1 + ((fifoNum - 1U) * CANFD_FIFO_OFFSET)) &= ~_CFD1FIFOCON1_TFERFFIE_MASK;
        }
        can1Obj.errorStatus = 0U;
    }
    IFS5CLR = _IFS5_CAN1IF_MASK;

    if (can1CallbackObj[fifoNum].callback != NULL)
    {
        context = can1CallbackObj[fifoNum].context;
        can1CallbackObj[fifoNum].callback(context);
    }
}

static void __attribute__((used)) CAN1_MISC_InterruptHandler(void)
{
    uint32_t errorStatus = 0;
    /* Additional temporary variable used to prevent MISRA violations (Rule 13.x) */
    uintptr_t context;

    CFD1INT &= ~(_CFD1INT_SERRIF_MASK | _CFD1INT_CERRIF_MASK | _CFD1INT_IVMIF_MASK);
    IFS5CLR = _IFS5_CAN1IF_MASK;
    errorStatus = CFD1TREC;

    /* Check if error occurred */
    can1Obj.errorStatus = ((errorStatus & _CFD1TREC_EWARN_MASK) |
                                                      (errorStatus & _CFD1TREC_RXWARN_MASK) |
                                                      (errorStatus & _CFD1TREC_TXWARN_MASK) |
                                                      (errorStatus & _CFD1TREC_RXBP_MASK) |
                                                      (errorStatus & _CFD1TREC_TXBP_MASK) |
                                                      (errorStatus & _CFD1TREC_TXBO_MASK));

    /* Client must call CAN1_ErrorGet and CAN1_ErrorCountGet functions to get errors */
    if (can1ErrorCallbackObj.callback != NULL)
    {
        context = can1ErrorCallbackObj.context;
        can1ErrorCallbackObj.callback(context);
    }
}

// *****************************************************************************
/* Function:
    void CAN1_InterruptHandler(void)

   Summary:
    CAN1 Peripheral Interrupt Handler.

   Description:
    This function is CAN1 Peripheral Interrupt Handler and will
    called on every CAN1 interrupt.

   Precondition:
    None.

   Parameters:
    None.

   Returns:
    None.

   Remarks:
    The function is called as peripheral instance's interrupt handler if the
    instance interrupt is enabled. If peripheral instance's interrupt is not
    enabled user need to call it from the main while loop of the application.
*/
void __attribute__((used)) CAN1_InterruptHandler(void)
{
    /* Call CAN MISC interrupt handler if SERRIF/CERRIF/IVMIF interrupt flag is set */
    if ((CFD1INT & (_CFD1INT_SERRIF_MASK | _CFD1INT_CERRIF_MASK | _CFD1INT_IVMIF_MASK)) != 0U)
    {
        CAN1_MISC_InterruptHandler();
    }

    /* Call CAN RX interrupt handler if RXIF interrupt flag is set */
    if ((CFD1INT & _CFD1INT_RXIF_MASK) != 0U)
    {
        CAN1_RX_InterruptHandler();
    }

    /* Call CAN TX interrupt handler if TXIF interrupt flag is set */
    if ((CFD1INT & _CFD1INT_TXIF_MASK) != 0U)
    {
        CAN1_TX_InterruptHandler();
    }
}

/* MISRAC 2012 deviation block end */
