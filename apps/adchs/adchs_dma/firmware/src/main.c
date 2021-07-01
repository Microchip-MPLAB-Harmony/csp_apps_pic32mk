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
#include "sys/kmem.h"

#define ADC_VREF                (3.3f)
#define ADC_MAX_COUNT           (4095)

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
/* Each ADC result takes 2 bytes. Reserve space for Buffer A and Buffer B. */
__COHERENT uint16_t adcResultBuffer[1][2][4];
__COHERENT uint8_t adcSampleCntBuffer[1][2];

volatile bool bufferA_Full = false;
volatile bool bufferB_Full = false;
float input_voltage;

void ADC_ResultHandler(ADCHS_DMA_STATUS dmaStatus, uintptr_t context)
{
    if (dmaStatus & ADCHS_DMA_STATUS_RAF0)
    {
        bufferA_Full = true;
    }
    
    if (dmaStatus & ADCHS_DMA_STATUS_RBF0)
    {
        bufferB_Full = true;
    }    
}


int main ( void )
{
    uint8_t i;
    
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    printf("\n\r---------------------------------------------------------");
    printf("\n\r                    ADC DMA Mode Demo                 ");
    printf("\n\r---------------------------------------------------------\n\r"); 
    
    ADCHS_DMACallbackRegister(ADC_ResultHandler, (uintptr_t)NULL);
    
    ADCHS_DMASampleCountBaseAddrSet((uint32_t)KVA_TO_PA(adcSampleCntBuffer));
    
    ADCHS_DMAResultBaseAddrSet((uint32_t)KVA_TO_PA(adcResultBuffer));
        
    TMR3_Start();
    
    while ( true )
    {               
        /* Print Buffer A ADC results once it is full */
        if (bufferA_Full == true)
        {
            bufferA_Full = false;
            
            printf ("---------ADC Buffer A Ready---------\r\n");
            
            for (i = 0; i < adcSampleCntBuffer[0][0]; i++)
            {
                input_voltage = (float)adcResultBuffer[0][0][i] * ADC_VREF / ADC_MAX_COUNT;

                printf("ADC Count[%d] = 0x%03x, ADC Input Voltage = %d.%02d V \r\n", i, adcResultBuffer[0][0][i], (int)input_voltage, (int)((input_voltage - (int)input_voltage)*100.0));        
            }
            
            /* Clear the sample counter */
            adcSampleCntBuffer[0][0] = 0;
        }
        /* Print Buffer B ADC results once it is full */
        if (bufferB_Full == true)
        {
            bufferB_Full = false;
            
            printf ("---------ADC Buffer B Ready---------\r\n");
            
            for (i = 0; i < adcSampleCntBuffer[0][1]; i++)
            {
                input_voltage = (float)adcResultBuffer[0][1][i] * ADC_VREF / ADC_MAX_COUNT;

                printf("ADC Count[%d] = 0x%03x, ADC Input Voltage = %d.%02d V \r\n", i, adcResultBuffer[0][1][i], (int)input_voltage, (int)((input_voltage - (int)input_voltage)*100.0));        
            }
            
            /* Clear the sample counter */
            adcSampleCntBuffer[0][1] = 0;
        }
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

