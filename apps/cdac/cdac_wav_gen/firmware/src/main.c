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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

#define NUM_OF_SAMPLES (100)

const uint16_t sine_wave[NUM_OF_SAMPLES] = {
        0x800, 0x840, 0x880, 0x8C0, 0x8FF, 0x93C, 0x979, 0x9B4, 0x9ED, 0xA25,
        0xA5A, 0xA8D, 0xABD, 0xAEA, 0xB15, 0xB3C, 0xB61, 0xB81, 0xB9F, 0xBB8,
        0xBCE, 0xBE0, 0xBEE, 0xBF8, 0xBFE, 0xC00, 0xBFE, 0xBF8, 0xBEE, 0xBE0,
        0xBCE, 0xBB8, 0xB9F, 0xB81, 0xB61, 0xB3C, 0xB15, 0xAEA, 0xABD, 0xA8D,
        0xA5A, 0xA25, 0x9ED, 0x9B4, 0x979, 0x93C, 0x8FF, 0x8C0, 0x880, 0x840,
        0x800, 0x7C0, 0x780, 0x740, 0x701, 0x6C4, 0x687, 0x64C, 0x613, 0x5DB,
        0x5A6, 0x573, 0x543, 0x516, 0x4EB, 0x4C4, 0x49F, 0x47F, 0x461, 0x448,
        0x432, 0x420, 0x412, 0x408, 0x402, 0x400, 0x402, 0x408, 0x412, 0x420,
        0x432, 0x448, 0x461, 0x47F, 0x49F, 0x4C4, 0x4EB, 0x516, 0x543, 0x573,
        0x5A6, 0x5DB, 0x613, 0x64C, 0x687, 0x6C4, 0x701, 0x740, 0x780, 0x7C0
};

volatile uint8_t sample_number = 0;
void TMR1_CallBack_Function (uint32_t status, uintptr_t context)
{ 
    CDAC1_DataWrite(sine_wave[sample_number]);

    sample_number++;

    if (sample_number >= 100)
    {
       sample_number = 0;
    }
}
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    TMR1_CallbackRegister(TMR1_CallBack_Function, (uintptr_t)NULL);
    TMR1_Start();
    
    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

