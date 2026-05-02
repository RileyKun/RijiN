#include "link.h"

LRESULT CALLBACK wnd_proc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

struct s_dxgi_offsets{
  bool x86 = false;
  u64 d3d9_reset   = 0;
  u64 d3d9_present = 0;
  u64 d3d11_present = 0;
};

static i8 d3d11_fill_shader[] =
  "struct VSOut"
  "{"
  " float4 Col : COLOR;"
  " float4 Pos : SV_POSITION;"
  "};"

  "VSOut VS(float4 Col : COLOR, float4 Pos : POSITION)"
  "{"
  " VSOut Output;"
  " Output.Pos = Pos;"
  " Output.Col = Col;"
  " return Output;"
  "}"

  "float4 PS(float4 Col : COLOR) : SV_TARGET"
  "{"
  " return Col;"
  "}";

i32 run_dxgi(i32 argc, const i8 *argv[]);
i32 main(i32 argc, const i8 *argv[]){
  VM_ULTRA_FAST_START();
  run_dxgi(argc, argv);
  VM_ULTRA_FAST_STOP();
  return 0;
}

i32 run_dxgi(i32 argc, const i8 *argv[]) {
  ShowWindow(GetConsoleWindow( ), 0);

  arg_count = argc;
  arg_array = argv;

  WNDCLASSA wnd;
  {
    wnd.style         = CS_OWNDC;
    wnd.lpfnWndProc   = static_cast<WNDPROC>( wnd_proc );
    wnd.cbClsExtra    = 0;
    wnd.cbWndExtra    = 0;
    wnd.hInstance     = GetModuleHandleA( nullptr );
    wnd.hIcon         = nullptr;
    wnd.hCursor       = nullptr;
    wnd.hbrBackground = nullptr;
    wnd.lpszMenuName  = nullptr;
    wnd.lpszClassName = "dxgidmp";
  }

  RegisterClassA( &wnd );

  void* window = CreateWindowExA(0, XOR("dxgidmp"), XOR("dxgidmp"), WS_SYSMENU, 0, 0, 0, 0, nullptr, nullptr, wnd.hInstance, nullptr);

  if(window == nullptr)
    return 0;

  s_dxgi_offsets offsets;
#if defined(__x86_64__)
  offsets.x86 = false;
#else
  offsets.x86 = true;
#endif

  // dump d3d9
  {
    _D3DPRESENT_PARAMETERS_ present_paramaters{};
    {
      present_paramaters.Windowed                   = true;
      present_paramaters.BackBufferFormat           = D3DFMT_A8R8G8B8;
      present_paramaters.BackBufferCount            = 1;
      present_paramaters.BackBufferHeight           = 1;
      present_paramaters.BackBufferWidth            = 1;
      present_paramaters.EnableAutoDepthStencil     = true;
      present_paramaters.AutoDepthStencilFormat     = D3DFMT_D24S8;
      present_paramaters.MultiSampleType            = D3DMULTISAMPLE_NONE;
      present_paramaters.MultiSampleQuality         = 0;
      present_paramaters.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
      present_paramaters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
      present_paramaters.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;
      present_paramaters.hDeviceWindow              = window;
    }

    IDirect3D9* d3d = Direct3DCreate9( D3D_SDK_VERSION );

    if ( d3d == nullptr )
      return 0;

    IDirect3DDevice9* device = nullptr;

    void* ret = d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, 64L, &present_paramaters, &device );

    if ( ret == 0 )
    {
#if defined(__x86_64__)
      *(u64*)&offsets.d3d9_reset   = (*(u64**)device)[16] - (u64)utils::get_module_handle(HASH("d3d9.dll"));
      *(u64*)&offsets.d3d9_present = (*(u64**)device)[17] - (u64)utils::get_module_handle(HASH("d3d9.dll"));
#else
      *(u32*)&offsets.d3d9_reset   = (*(u32**)device)[16] - (u32)utils::get_module_handle(HASH("d3d9.dll"));
      *(u32*)&offsets.d3d9_present = (*(u32**)device)[17] - (u32)utils::get_module_handle(HASH("d3d9.dll"));
#endif

      //printf("[!] d3d9_present: %p\n", offsets.d3d9_present);

      device->Release( );
      d3d->Release( );
    }
  }

  // dump d3d11
  {
    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    {
      swap_chain_desc.Windowed           = true;
      swap_chain_desc.BufferCount        = 1;
      swap_chain_desc.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
      swap_chain_desc.BufferUsage        = 0x20;
      swap_chain_desc.OutputWindow       = window;
      swap_chain_desc.SampleDesc.Count   = 1;
      swap_chain_desc.SampleDesc.Quality = 0;
    }

    IDXGISwapChain*      chain;
    ID3D11Device*        device;
    ID3D11DeviceContext* context;

    void* result = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,D3D11_SDK_VERSION, &swap_chain_desc, &chain, &device, nullptr, &context );

    if ( result == 0 )
    {
      {
        D3D_SHADER_MACRO shader_macro[] = { NULL, NULL };
        ID3DBlob* vs_blob_ptr = NULL, * ps_blob_ptr = NULL, * error_blob = NULL;

        D3DCompile(d3d11_fill_shader, strlen(d3d11_fill_shader), NULL, shader_macro, NULL, XOR("VS"), XOR("vs_4_0"), 0, 0, &vs_blob_ptr, &error_blob);
        D3DCompile(d3d11_fill_shader, strlen(d3d11_fill_shader), NULL, shader_macro, NULL, XOR("PS"), XOR("ps_4_0"), 0, 0, &ps_blob_ptr, &error_blob);
      }

#if defined(__x86_64__)
      *(u64*)&offsets.d3d11_present = (*(u64**)chain)[8] - (u64)utils::get_module_handle(HASH("dxgi.dll"));
#else
      *(u32*)&offsets.d3d11_present = (*(u32**)chain)[8] - (u32)utils::get_module_handle(HASH("dxgi.dll"));
#endif

      //printf("[!] d3d11_present: %p\n", offsets.d3d11_present);

      chain->Release( );
      device->Release( );
      context->Release( );
    }
  }

  CloseHandle( window );

  utils::write_file(XOR("dxgidmp.dat"), &offsets, sizeof(s_dxgi_offsets));

  return 0;
}
