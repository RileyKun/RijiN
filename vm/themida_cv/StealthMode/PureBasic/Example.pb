;
; ------------------------------------------------------------
;
;   PureBasic - Code Virtualizer macros example file
;
;    (c) 2014 - Oreans Technologies
;
; ------------------------------------------------------------
;

XIncludeFile "VirtualizerSDK32.pbi"
XIncludeFile "StealthCodeArea.pbi"

VIRTUALIZER_FISH_WHITE_START

MessageRequester("Encode Macro", "Hello, I'm a VIRTUALIZER macro!", 0)

VIRTUALIZER_FISH_WHITE_END

If Sleep_  = 1
  
    STEALTH_AREA_START
  
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK
    STEALTH_AREA_CHUNK

    STEALTH_AREA_END
  
EndIf
