unit uStealthDataArea;

interface

uses
  Winapi.Windows;

const
  AREA_SIZE_ONE_MB  = 1024 * 1024;
  STEALTH_AREA_SIZE = AREA_SIZE_ONE_MB * 3;  // 3Mb for Stealth Area

type
  TStealthData = record
     Header: array [0..3] of DWORD;
     Body:   array [0..STEALTH_AREA_SIZE] of BYTE;
     Tail:   array [0..3] of DWORD;
  end;

var
  StealthDataArea: TStealthData =
                    (
                      Header: ($a1a2a3a4, $a4a3a2a1, $b1a1b2a2, $b8a8a1a1);
                      Tail:   ($b6b5b3b6, $a2b2c2d2, $a9a8a2a2, $a0a9b9b8)
                    );

 implementation

 end.
