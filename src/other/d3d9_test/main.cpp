#include "link.h"


#define DBG(x, ...) printf(x, ##__VA_ARGS__)

static vec3i base_size = vec3i(1920, 1080);

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 device;
LPDIRECT3DTEXTURE9 t;
CFLAG_O0 static void init_d3d(HWND wnd){
  d3d = Direct3DCreate9(D3D_SDK_VERSION);
  D3DPRESENT_PARAMETERS d3dpp;

  memset(&d3dpp, 0, sizeof(D3DPRESENT_PARAMETERS));
  d3dpp.Windowed = true;
  d3dpp.BackBufferCount = 1;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
  d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
  d3dpp.hDeviceWindow = wnd; 
  d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, 64L, &d3dpp, &device);

  RENDER_SETUP();
  CREATE_NOTIFICATION_SYSTEM();
  notify->setup_fonts();

  render->screen_size = base_size;

  assert(global != nullptr);
  assert(render != nullptr);
 // global->esp_font_large = render->create_font(XOR("Segoe UI Emoji"), 14, FW_MEDIUM, CLEARTYPE_QUALITY);


  if(global->gui_font == nullptr){
    global->gui_font  = render->create_font(XOR("Segoe UI"), 14, FW_MEDIUM, CLEARTYPE_QUALITY);
    assert(global->gui_font != nullptr);
  }

  if(global->gui_font_emoji == nullptr){
    global->gui_font_emoji = render->create_font(XOR("Segoe UI Emoji"), 14, FW_MEDIUM, CLEARTYPE_QUALITY);
    assert(global->gui_font_emoji != nullptr);
  }

  if(global->menu == nullptr){
    global->menu  = gcs::add_scene<gcs_menu>();
    assert(global->menu != nullptr);
  }

  if(global->info_panel == nullptr){
    global->info_panel = gcs::add_scene<gcs_base_info_panel>();

    static u8  key = VK_LSHIFT;
    static bool held = false;
    static bool toggle = true;
    static bool double_click  = false;
    static c_key_control test(&key, &held, &toggle, &double_click);

    //global->info_panel->assign(0, L"Noisemaker spam", &test, &global->is_option_enabled);
   // global->info_panel->assign(1, L"Aimhack", nullptr, &toggle);
    assert(global->info_panel != nullptr);
  }

  entity_list_data->add_entry(1,  3, WXOR(L"DurRudy"),             WXOR(L"DurRud"));
}

static void alloc_console(){
#if defined(DEV_MODE)
  _iobuf* data;

  AllocConsole();
  freopen_s( &data, XOR( "CONIN$" ), XOR( "r" ), stdout );
  freopen_s( &data, XOR( "CONOUT$" ), XOR( "w" ), __acrt_iob_func( 1 ) );

  DBG("[!] loader - build %s\n", __DATE__);

  //system("color 1f");
#endif
}


CFLAG_O0 static void render_d3d9_frame(){
  device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(188, 188, 188), 1.0f, 0);
  device->BeginScene(); 

  static bool enabled = true;
  static vec3i pos = vec3i(200, 200);

  static bool ff_once = false;
  
  auto* state = render->begin();
  {
    input_system->update();
    if(input_system->held(VK_HOME)){
      if(!ff_once){
        logs->server_log->add(L"EXAMPLE");
        ff_once = true;
      }
    }
    else
      ff_once = false;

    static bool beast = false;
    static bool what = false;
    if(input_system->held(VK_END)){
      if(!beast){
        what = !what;
        logs->player_log->add(L"TEST");
        //beast = true;
      }
    }
    else
      beast = false;

    vec3i m_pos = math::mouse_pos(global->target_window);

    colour clr  = rgba(255, 0, 0, 255);
    colour clr2 = rgba(0, 255, 0, 255);
    colour clr3 = rgba(0, 0, 255, 255);
    colour clr4 = rgba(255, 255, 255, 255);
    render->gradient_rect(vec3i(0, 0), base_size, clr, clr2, clr3, clr4);

    render->render_logo(vec3i(10, 10), vec3i(184, 184), colour(255, 255, 255, 6));

    gcs::render_scene(global->menu, m_pos, global->gui_font, global->gui_font_emoji);
    //global->info_panel->init(&pos, &enabled);
   // assert(global->info_panel != nullptr)
   // gcs::render_scene(global->info_panel, m_pos, global->gui_font, global->gui_font_emoji);
  }

  render->end(state);

  device->EndScene();
  device->Present(NULL, NULL, NULL, NULL);
}

LRESULT __stdcall WindowProc(HWND wnd, u32 message, WPARAM wparam, LPARAM lparam);

// IGNORE L0L
i32 __stdcall WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, i32 cmd_show){
  alloc_console();
  HWND wnd;
  WNDCLASSEX wc;
  memset(&wc, 0, sizeof(WNDCLASSEX));

  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = instance;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wc.lpszClassName = "BeastClass1776";

  RegisterClassEx(&wc);

  wnd = CreateWindowEx(NULL, "BeastClass1776", "GFX - D3D9 TEST", WS_OVERLAPPEDWINDOW, 0, 0, base_size.x, base_size.y, NULL, NULL, instance, NULL);
  ShowWindow(wnd, cmd_show);
  global->target_window = wnd;

  init_d3d(wnd);

  global->add_player(L"Senator");
  global->add_player(L"DurRud");
  global->add_player(L"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");
  global->add_player(L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

  MSG msg;

  while(true){
    while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if(msg.message == WM_QUIT)
      break;

    render_d3d9_frame();
  }

  return msg.wParam;
}


LRESULT __stdcall WindowProc(HWND wnd, u32 message, WPARAM wparam, LPARAM lparam){
  switch (message)
  {
    case WM_DESTROY:
    {     
      PostQuitMessage(0);
      return 0;
    }
  }

  return DefWindowProc(wnd, message, wparam, lparam);
}