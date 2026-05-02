/*****************************************************************************
; File: Example_StealthInDATAsection.c
; Description: Simple example about how to insert a Stealth Data Area
;
; Authors: Rafael Ahucha 
; (c) 2014 Oreans Technologies 
;****************************************************************************/


/******************************************************************************
;                                Includes
;*****************************************************************************/

#include <stdio.h>
#include "VirtualizerSDK.h"
#include "StealthDataArea.h"


/******************************************************************************
;                                Defines
;*****************************************************************************/

#define STEALTH_AREA_SIZE 0x20000    


/******************************************************************************
;                               Global Data
;*****************************************************************************/

STEALTH_DATA_AREA(STEALTH_AREA_SIZE);


/******************************************************************************
;                                   Code
;*****************************************************************************/

/******************************************************************************
 * main
 *****************************************************************************/

int main(
    int argc, 
    const char * argv[])
{
    int DummyStealthVar = 0x11111111;

    // Put a dummy reference, so the compiler won't skip the generation of the 
    // Stealth data area for not being referenced. Notice this conditional 
    // is never executed
    if (DummyStealthVar == 0x11223344)
    {
        REFERENCE_STEALTH_DATA_AREA;
    }
    
    // Your code here

    return 0;
}


