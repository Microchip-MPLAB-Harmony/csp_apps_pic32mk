/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h UUUUUUUUU

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

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

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for APP_SLAVE_BUSY_PIN pin ***/
#define APP_SLAVE_BUSY_PIN_Set()               (LATDSET = (1U<<4))
#define APP_SLAVE_BUSY_PIN_Clear()             (LATDCLR = (1U<<4))
#define APP_SLAVE_BUSY_PIN_Toggle()            (LATDINV= (1U<<4))
#define APP_SLAVE_BUSY_PIN_OutputEnable()      (TRISDCLR = (1U<<4))
#define APP_SLAVE_BUSY_PIN_InputEnable()       (TRISDSET = (1U<<4))
#define APP_SLAVE_BUSY_PIN_Get()               ((PORTD >> 4) & 0x1U)
#define APP_SLAVE_BUSY_PIN_GetLatch()          ((LATD >> 4) & 0x1U)
#define APP_SLAVE_BUSY_PIN_PIN                  GPIO_PIN_RD4

/*** Macros for APP_SLAVE_CS pin ***/
#define APP_SLAVE_CS_Set()               (LATCSET = (1U<<0))
#define APP_SLAVE_CS_Clear()             (LATCCLR = (1U<<0))
#define APP_SLAVE_CS_Toggle()            (LATCINV= (1U<<0))
#define APP_SLAVE_CS_OutputEnable()      (TRISCCLR = (1U<<0))
#define APP_SLAVE_CS_InputEnable()       (TRISCSET = (1U<<0))
#define APP_SLAVE_CS_Get()               ((PORTC >> 0) & 0x1U)
#define APP_SLAVE_CS_GetLatch()          ((LATC >> 0) & 0x1U)
#define APP_SLAVE_CS_PIN                  GPIO_PIN_RC0

/*** Macros for SWITCH pin ***/
#define SWITCH_Set()               (LATGSET = (1U<<11))
#define SWITCH_Clear()             (LATGCLR = (1U<<11))
#define SWITCH_Toggle()            (LATGINV= (1U<<11))
#define SWITCH_OutputEnable()      (TRISGCLR = (1U<<11))
#define SWITCH_InputEnable()       (TRISGSET = (1U<<11))
#define SWITCH_Get()               ((PORTG >> 11) & 0x1U)
#define SWITCH_GetLatch()          ((LATG >> 11) & 0x1U)
#define SWITCH_PIN                  GPIO_PIN_RG11

/*** Macros for LED1 pin ***/
#define LED1_Set()               (LATGSET = (1U<<12))
#define LED1_Clear()             (LATGCLR = (1U<<12))
#define LED1_Toggle()            (LATGINV= (1U<<12))
#define LED1_OutputEnable()      (TRISGCLR = (1U<<12))
#define LED1_InputEnable()       (TRISGSET = (1U<<12))
#define LED1_Get()               ((PORTG >> 12) & 0x1U)
#define LED1_GetLatch()          ((LATG >> 12) & 0x1U)
#define LED1_PIN                  GPIO_PIN_RG12


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/


#define    GPIO_PORT_A  (0)
#define    GPIO_PORT_B  (1)
#define    GPIO_PORT_C  (2)
#define    GPIO_PORT_D  (3)
#define    GPIO_PORT_E  (4)
#define    GPIO_PORT_F  (5)
#define    GPIO_PORT_G  (6)
typedef uint32_t GPIO_PORT;

typedef enum
{
    GPIO_INTERRUPT_ON_MISMATCH,
    GPIO_INTERRUPT_ON_RISING_EDGE,
    GPIO_INTERRUPT_ON_FALLING_EDGE,
    GPIO_INTERRUPT_ON_BOTH_EDGES,
}GPIO_INTERRUPT_STYLE;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/


#define     GPIO_PIN_RA0  (0U)
#define     GPIO_PIN_RA1  (1U)
#define     GPIO_PIN_RA4  (4U)
#define     GPIO_PIN_RA7  (7U)
#define     GPIO_PIN_RA8  (8U)
#define     GPIO_PIN_RA10  (10U)
#define     GPIO_PIN_RA11  (11U)
#define     GPIO_PIN_RA12  (12U)
#define     GPIO_PIN_RA14  (14U)
#define     GPIO_PIN_RA15  (15U)
#define     GPIO_PIN_RB0  (16U)
#define     GPIO_PIN_RB1  (17U)
#define     GPIO_PIN_RB2  (18U)
#define     GPIO_PIN_RB3  (19U)
#define     GPIO_PIN_RB4  (20U)
#define     GPIO_PIN_RB5  (21U)
#define     GPIO_PIN_RB6  (22U)
#define     GPIO_PIN_RB7  (23U)
#define     GPIO_PIN_RB8  (24U)
#define     GPIO_PIN_RB9  (25U)
#define     GPIO_PIN_RB10  (26U)
#define     GPIO_PIN_RB11  (27U)
#define     GPIO_PIN_RB12  (28U)
#define     GPIO_PIN_RB13  (29U)
#define     GPIO_PIN_RB14  (30U)
#define     GPIO_PIN_RB15  (31U)
#define     GPIO_PIN_RC0  (32U)
#define     GPIO_PIN_RC1  (33U)
#define     GPIO_PIN_RC2  (34U)
#define     GPIO_PIN_RC6  (38U)
#define     GPIO_PIN_RC7  (39U)
#define     GPIO_PIN_RC8  (40U)
#define     GPIO_PIN_RC9  (41U)
#define     GPIO_PIN_RC10  (42U)
#define     GPIO_PIN_RC11  (43U)
#define     GPIO_PIN_RC12  (44U)
#define     GPIO_PIN_RC13  (45U)
#define     GPIO_PIN_RC15  (47U)
#define     GPIO_PIN_RD1  (49U)
#define     GPIO_PIN_RD2  (50U)
#define     GPIO_PIN_RD3  (51U)
#define     GPIO_PIN_RD4  (52U)
#define     GPIO_PIN_RD5  (53U)
#define     GPIO_PIN_RD6  (54U)
#define     GPIO_PIN_RD12  (60U)
#define     GPIO_PIN_RD13  (61U)
#define     GPIO_PIN_RD14  (62U)
#define     GPIO_PIN_RD15  (63U)
#define     GPIO_PIN_RE0  (64U)
#define     GPIO_PIN_RE1  (65U)
#define     GPIO_PIN_RE8  (72U)
#define     GPIO_PIN_RE9  (73U)
#define     GPIO_PIN_RE12  (76U)
#define     GPIO_PIN_RE13  (77U)
#define     GPIO_PIN_RE14  (78U)
#define     GPIO_PIN_RE15  (79U)
#define     GPIO_PIN_RF0  (80U)
#define     GPIO_PIN_RF1  (81U)
#define     GPIO_PIN_RF5  (85U)
#define     GPIO_PIN_RF6  (86U)
#define     GPIO_PIN_RF7  (87U)
#define     GPIO_PIN_RF9  (89U)
#define     GPIO_PIN_RF10  (90U)
#define     GPIO_PIN_RF12  (92U)
#define     GPIO_PIN_RF13  (93U)
#define     GPIO_PIN_RG0  (96U)
#define     GPIO_PIN_RG1  (97U)
#define     GPIO_PIN_RG6  (102U)
#define     GPIO_PIN_RG7  (103U)
#define     GPIO_PIN_RG8  (104U)
#define     GPIO_PIN_RG9  (105U)
#define     GPIO_PIN_RG10  (106U)
#define     GPIO_PIN_RG11  (107U)
#define     GPIO_PIN_RG12  (108U)
#define     GPIO_PIN_RG13  (109U)
#define     GPIO_PIN_RG14  (110U)
#define     GPIO_PIN_RG15  (111U)

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
#define    GPIO_PIN_NONE   (-1)

typedef uint32_t GPIO_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
     uint32_t xvalue = (uint32_t)value;
    GPIO_PortWrite((pin>>4U), (uint32_t)(0x1U) << (pin & 0xFU), (xvalue) << (pin & 0xFU));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return ((((GPIO_PortRead((GPIO_PORT)(pin>>4U))) >> (pin & 0xFU)) & 0x1U) != 0U);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (((GPIO_PortLatchRead((GPIO_PORT)(pin>>4U)) >> (pin & 0xFU)) & 0x1U) != 0U);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
