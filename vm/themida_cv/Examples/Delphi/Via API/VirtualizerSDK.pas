unit VirtualizerSDK;

{$ALIGN ON}
{$MINENUMSIZE 4}

interface

uses
  Windows;

procedure VirtualizerStart();stdcall;
procedure VirtualizerEnd();stdcall;

procedure VirtualizerStrEncryptStart();stdcall;
procedure VirtualizerStrEncryptEnd();stdcall;
procedure VirtualizerStrEncryptWStart();stdcall;
procedure VirtualizerStrEncryptWEnd();stdcall;

{$I VirtualizerSDK_CustomVMsInterface.pas}


implementation

const

{$IFDEF WIN64}
  Virtualizer = 'VirtualizerSDK64.DLL';
{$ELSE}
  Virtualizer = 'VirtualizerSDK32.DLL';
{$ENDIF}

procedure VirtualizerStart; external Virtualizer name 'VirtualizerStart';
procedure VirtualizerEnd; external Virtualizer name 'VirtualizerEnd';

procedure VirtualizerStrEncryptStart; external Virtualizer name 'VirtualizerStrEncryptStart';
procedure VirtualizerStrEncryptEnd; external Virtualizer name 'VirtualizerStrEncryptEnd';
procedure VirtualizerStrEncryptWStart; external Virtualizer name 'VirtualizerStrEncryptWStart';
procedure VirtualizerStrEncryptWEnd; external Virtualizer name 'VirtualizerStrEncryptWEnd';

{$I VirtualizerSDK_CustomVMsImplementation.pas}

end.

