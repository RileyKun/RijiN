unit unit1;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls;

type
  TForm1 = class(TForm)
    Button1: TButton;
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

uses
  uStealthDataArea;

procedure TForm1.Button1Click(Sender: TObject);
var
  always_false: Boolean;

begin
  // We make a dummy access to the StealtDataArea variable
  // to avoid the compiler omitting the generation of such variable
  // when it's not used. Notice this code is never executed to avoid 
  // overwriting the generated virtualized code
  if (always_false = True) then  
  begin
    StealthDataArea.Header[0] := StealthDataArea.Header[0];
  end;
end;

end.
