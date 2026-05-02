/*****************************************************************************
; File: Example_StealthInCODEsection.c
; Description: Simple example about how to insert a Stealth Area
;
; Authors: Rafael Ahucha 
; (c) 2014 Oreans Technologies 
;****************************************************************************/


/******************************************************************************
;                                Includes
;*****************************************************************************/

#include <stdio.h>
#include "VirtualizerSDK.h"
#include "StealthCodeArea.h"


/******************************************************************************
;                               Global Data
;*****************************************************************************/

int MyGlobalVarStealth = 0;


/******************************************************************************
;                                   Code
;*****************************************************************************/


/******************************************************************************
 * STEALTH_AUX_FUNCTION
 *  "Fake" function that is referenced from the Stealth Area
 *  This function is defined as macro inside "StealthCodeArea.h"
 *****************************************************************************/

STEALTH_AUX_FUNCTION


/******************************************************************************
 * MyFunctionStealthCodeArea
 *  Defines the Stealth Area. Note that this function should never be executed.
 *****************************************************************************/

void 
MyFunctionStealthCodeArea(void)
{
    STEALTH_AREA_START

    // Here we create our Stealth area. Insert more entries if you
    // require more space for the protection code

    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    
    STEALTH_AREA_END
}


/******************************************************************************
 * main
 *****************************************************************************/

int main(
    int argc, 
    const char * argv[])
{
    // some compilers optimizations removes functions that are never called
    // in an application. In order to avoid that the Stealth area function is
    // not compiled (no code generated), we create a conditional condition that
    // will never be true.

    if (MyGlobalVarStealth == 0x11223344)
    {
        MyFunctionStealthCodeArea();
    }

    return 0;
}


