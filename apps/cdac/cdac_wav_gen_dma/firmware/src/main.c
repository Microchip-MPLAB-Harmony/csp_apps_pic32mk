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
0x200,	0x210,	0x220,	0x230,	0x240,	0x24F,	0x25E,	0x26D,	0x27B,	0x289,
0x297,	0x2A3,	0x2AF,	0x2BB,	0x2C5,	0x2CF,	0x2D8,	0x2E0,	0x2E8,	0x2EE,
0x2F4,	0x2F8,	0x2FC,	0x2FE,	0x300,	0x300,	0x300,	0x2FE,	0x2FC,	0x2F8,
0x2F4,	0x2EE,	0x2E8,	0x2E0,	0x2D8,	0x2CF,	0x2C5,	0x2BB,	0x2AF,	0x2A3,
0x297,	0x289,	0x27B,	0x26D,	0x25E,	0x24F,	0x240,	0x230,	0x220,	0x210,
0x200,	0x1F0,	0x1E0,	0x1D0,	0x1C0,	0x1B1,	0x1A2,	0x193,	0x185,	0x177,
0x16A,	0x15D,	0x151,	0x146,	0x13B,	0x131,	0x128,	0x120,	0x118,	0x112,
0x10D,	0x108,	0x105,	0x102,	0x101,	0x100,	0x101,	0x102,	0x105,	0x108,
0x10D,	0x112,	0x118,	0x120,	0x128,	0x131,	0x13B,	0x146,	0x151,	0x15D,
0x16A,	0x177,	0x185,	0x193,	0x1A2,	0x1B1,	0x1C0,	0x1D0,	0x1E0,	0x1F0,
};

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    DMAC_ChannelTransfer(DMAC_CHANNEL_0, (const void *)&sine_wave, sizeof(sine_wave), (const void *)((uint16_t*)&DAC1CON +1), 2, 2);
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

