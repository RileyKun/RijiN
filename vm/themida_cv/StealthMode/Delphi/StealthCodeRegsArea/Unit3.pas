unit Unit3;

interface

uses
  System.SysUtils, System.Types, System.UITypes, System.Classes, System.Variants,
  FMX.Types, FMX.Controls, FMX.Forms, FMX.Graphics, FMX.Dialogs,
  FMX.Controls.Presentation, FMX.StdCtrls;

type
  TForm3 = class(TForm)
    Button1: TButton;
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form3: TForm3;

implementation

{$R *.fmx}

uses
  uStealthRegsArea, VirtualizerAsmSDK;

{$I StealthAuxFunction.inc}

procedure TForm3.Button1Click(Sender: TObject);
var
  always_false: Boolean;

begin
  // We make a dummy access to the Stealth register area and code
  // to avoid the compiler omitting the generation of such code/variables
  // when it's not used. Notice this code is never executed to avoid
  // overwriting the generated virtualized code
  if (always_false = True) then
  begin

      // Here we create our Stealth area. Insert more entries if you
    // require more space for the protection code

    {$I StealthArea_Start.inc}

    {$I StealthArea_Chunk.inc}
    {$I StealthArea_Chunk.inc}
    {$I StealthArea_Chunk.inc}
    {$I StealthArea_Chunk.inc}

    {$I StealthArea_End.inc}

    StealthRegsArea.Header[0] := StealthRegsArea.Header[0];
  end;

  VIRTUALIZER_START();

  ShowMessage('I''m inside a Virtualizer macro');

  VIRTUALIZER_END();

end;

end.
