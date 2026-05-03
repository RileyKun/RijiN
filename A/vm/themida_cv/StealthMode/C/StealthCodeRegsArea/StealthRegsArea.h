/*****************************************************************************
; Header: StealthDataArea.h
; Description: Header to create a "dummy" data area to insert the protection 
;              code
;****************************************************************************/


/******************************************************************************
;                                Defines
;*****************************************************************************/

#define STEALTH_REGS_AREA(num_vms) \
    typedef struct { \
        uint32_t start[4]; \
        uint32_t stealth[0x1000 * num_vms]; \
        uint32_t end[4]; \
    } __stealthregmarker_t; \
    \
    volatile __stealthregmarker_t __stealthregmarker = { \
        { 0xA5A6A7A8, 0xA8A7A6A5, 0xB5A5B6A6, 0xB7A7A5A5 }, \
        { 0 }, \
        { 0xB8B7B6B5, 0xA6B6C6D6, 0xA7A6A4A4, 0xA4A5B1B2 } \
    };

#define REFERENCE_STEALTH_REGS_AREA __stealthregmarker.stealth[0] = 1