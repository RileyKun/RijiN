unit uStealthRegsArea;

interface

const
  NUMBER_VMS  = 5;
  SINGLE_REG_AREA_SIZE = $1000;

type
  TStealthRegData = record
     Header: array [0..3] of LongWord;
     Body:   array [0..NUMBER_VMS * SINGLE_REG_AREA_SIZE] of BYTE;
     Tail:   array [0..3] of LongWord;
  end;

var
  StealthRegsArea: TStealthRegData =
                    (
                      Header: ($A5A6A7A8, $A8A7A6A5, $B5A5B6A6, $B7A7A5A5);
                      Tail:   ($B8B7B6B5, $A6B6C6D6, $A7A6A4A4, $A4A5B1B2)
                    );
 implementation

 end.
