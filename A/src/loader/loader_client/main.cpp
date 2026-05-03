#include "link.h"

i32         error_code        = 0;
i32         error_line        = 0;
ul64        error_status_code = 0;
std::string error_msg         = "";

// We do this to get the compiler to create a import entry
// This function in reality will never be called
void import_trick(){
  // USER32.dll
  MessageBoxA(nullptr, nullptr, nullptr, 0);
  
  // KERNEL32.dll
  CloseHandle(nullptr);
  
  // Credui.dll
  CredUnPackAuthenticationBufferW(0, nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

  // Gdi32.dll
  CreateSolidBrush(RGB(0,0,0));

  // Dwrite.dll
  DWriteCreateFactory(0, __uuidof(IDWriteFactory), (IUnknown**)nullptr);

  // d2d1.dll
  ID2D1Factory* dummy = nullptr;
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &dummy);

  // Rpcrt4.dll
  NdrContextHandleSize(nullptr, nullptr, nullptr);

  // Iphlpapi.dll
  GetIfEntry(nullptr);

  // Ws2_32.dll
  getprotobynumber(0);

  // AdvApi32.dll
  RegQueryReflectionKey(nullptr, nullptr);

  // Wininet.dll
  InternetHangUp(0, 0);
}

static u32 __stdcall license_timeout_worker_thread(c_net_receive_license* p){
  assert(p != nullptr);

  if(p == nullptr){
    I(DestroyWindow)(global->window);
    SHOW_ERROR_STR(ERR_LICENSE_HAS_EXPIRED, "(NAR) License has expired, please open the loader again.", true);
    return 0;
  }

  // Instead of predicting with ping, make an assumption that it took around a second to request and setup the license
  // This is just a simple assumption prediction of our estimated time
  // UPDATE: Technically looking at this we could just be doing a expire time, but I did this for the specific reason of
  // Being able to see a countdown in the console and also just because it works.
  // TODO: Change me to a expire time, but its not exactly high priority.
  p->license_expire_seconds--;

  while(p->license_expire_seconds-- > 0){
#if defined(DEV_MODE)
    if(p->license_expire_seconds % 10 == 0 || p->license_expire_seconds <= 10)
      DBG("[!] License expires in %i seconds...\n", p->license_expire_seconds);
#endif

    I(Sleep)(1000);
  }

  I(DestroyWindow)(global->window);
  SHOW_ERROR_STR(ERR_LICENSE_HAS_EXPIRED, "License has expired, please open the loader again.", true);

  return 0;
}

static void render_frame(){
  if(global->block_render){
    I(Sleep)(1);
    return;
  }

  u32 target_fps = XOR32(60);

  float start = render->framtime_start();
  {
    render->strict_no_antialias(false);
    render->begin_draw();

    static c_font_d2d1* font = render->create_font(WXOR(L"Segoe UI"), DWRITE_FONT_WEIGHT_MEDIUM);
    if(font == nullptr){
      SHOW_ERROR_STR(ERR_LOADER_D2D1_FAILED_TO_GET_FONT, "D2D1 failed to initialize. (2)", true);
      return;
    }

    if(global->license_data != nullptr){
      if(global->menu == nullptr)
        global->menu = gcs::add_scene<gcs_menu>();

      if(global->menu == nullptr){
        SHOW_ERROR_STR(ERR_LOADER_FAILED_TO_CREATE_MENU, "Failed to create window. (2)", true);
        return;
      }

      // create all the bitmaps we have queued
      {
        while(!global->bitmap_creation_queue.empty()){
          auto entry = global->bitmap_creation_queue.back();

          *entry.second = render->create_bitmap(entry.first->data, entry.first->len);
          global->bitmap_creation_queue.pop_back();
        }
      }

      gcs::render_scene(global->menu, math::mouse_pos(global->window), (c_font_base*)font );
    }

    if(global->draw_loading_page){
      {
        float anim_timer = math::fmod(math::time() * 0.25f, 2.f);
        float anim_frac  = math::clamp(anim_timer >= 1.f ? 1.f - math::abs(1.f - anim_timer) : anim_timer, 0.f, 1.f);

        colour low_col  =  rgb(26, 35, 58);
        colour high_col =  low_col * 1.5f;
        colour from = math::lerp(low_col, high_col, anim_frac);
        colour to   = math::lerp(high_col, low_col, anim_frac);

        render->gradient_rect_linear(vec3i(0,0), render->screen_size, vec3i(0, 0), render->screen_size, from, to);
      }

      // draw circle
      {
        float anim_x = (float)((i32)(math::time() * 360.f) % 360);

        vec3i circle_size = vec3i(50, 50);
        render->progress_circle((render->screen_size / 2) - (circle_size / 2), circle_size, 5.f, anim_x, rgb(230,230,230), rgb(115,115,115));

        if(!global->is_loading_str_empty())
          font->draw((render->screen_size / 2) + vec3i(0, (circle_size.y / 2) + 10), 10, global->get_loading_page_str(), rgb(230,230,230), TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CHEAP);
      }
    }

    render->draw_watermark(font, true, 8);
    render->end_draw();
  }
  render->frametime_stop(start, target_fps);
}

ALWAYSINLINE LRESULT __stdcall win_procedure(const HWND window_handle, const UINT msg, const WPARAM param, const LPARAM lparam){
  switch(msg){
    default: break;
    case WM_CLOSE: // So the process can close.
    case WM_DESTROY:{
      I(PostQuitMessage)(0);
      I(DestroyWindow)(window_handle);
      return 0;
    }
    case WM_MOVING:
    case WM_ENTERSIZEMOVE:
    {
      global->block_render = true;
      break;
    }
    case WM_EXITSIZEMOVE:
    {
      global->block_render = false;
      break;
    }
    case WM_NCLBUTTONDOWN:
      SetForegroundWindow(window_handle);
      break;
  }

  return DefWindowProcA(window_handle, msg, param, lparam);
}

static u32 __stdcall handle_login_thread(void* reserved){
  static bool first_time_sleep = false;
  if(!first_time_sleep){
    first_time_sleep = true;
    I(Sleep)(XOR32(1000));
  }

  // request server info table
  c_net_receive_info* info = nullptr;
  {
    global->show_loading_page(XOR(L"Establishing connection to master server."));

    for(u32 i = 0; i < XOR32(10); i++){
      if((info = server->get_serverinfo()) != nullptr)
        break;

      if(i > 3)
        global->show_loading_page(XOR(L"Still trying to connect to master server."));

      I(Sleep)(XOR32(1000));
    }

    if(info == nullptr){
      global->show_loading_page(XOR(L"Failed to connect to master server"));
      SHOW_ERROR_STR(ERR_TIMEOUT, "A connection to the master server cannot be established.", true);
      return;
    }
  }

  // Login
  bool should_save_logins = false;
  bool attempt_auto_login = true;

  global->show_loading_page(XOR(L"Waiting for user to login..."));
  global->handle_creds(&should_save_logins, &attempt_auto_login);
  global->show_loading_page(XOR(L"Logging in..."));

  bool did_time_out = false;
  global->license_data = server->request_license(global->up, &did_time_out);

  if(global->license_data == nullptr){
    if(!did_time_out){
      global->show_loading_page(XOR(L"Failed to login!\nCheck login details."));
      utils::delete_up();
      Sleep(XOR32(2500));
    }
    else{
      global->show_loading_page(XOR(L"Failed to connect to master server"));
      SHOW_ERROR_STR(ERR_TIMEOUT, "A connection to the master server cannot be established.", true);
    }
    I(ExitProcess)(0);
    return;
  }

  utils::create_worker(license_timeout_worker_thread, global->license_data);
  global->show_loading_page(WXOR(L"Verifiying..."));

  u32 limit = 0;
  while(!rtp_transmitted_heartbeat && limit <= XOR32(10))
    I(Sleep)(XOR32(1000));

  if(!rtp_transmitted_heartbeat){
    SHOW_ERROR_STR(ERR_TIMEOUT, "Connection error\nEnsure your anti-virus has the loader excluded and try again.", true);
    return 0;
  }

  global->show_loading_page(XOR(L"Successfully logged in!"));

  if(should_save_logins)
    utils::save_up();

  global->hide_loading_page();

  return 0;
}

#if defined(DEV_MODE)
static void alloc_console(){
  _iobuf* data;

  AllocConsole();
  freopen_s( &data, XOR( "CONIN$" ), XOR( "r" ), stdout );
  freopen_s( &data, XOR( "CONOUT$" ), XOR( "w" ), __acrt_iob_func( 1 ) );

  DBG("[!] loader - build %s\n", __DATE__);
}
#endif

static void map_check(){
  if(file_map->is_active(HASH("RIJIN_LOADER"))){
    I(MessageBoxA)(nullptr, XOR("An instance of the loader is running.\nIf it's stuck open then check task manager and close it."), "", XOR32(MB_TOPMOST | MB_ICONINFORMATION));
    SAFE_EXIT();
    return;
  }

  if(file_map->is_active(HASH("RIJIN_STUB"))){
    I(MessageBoxA)(nullptr, XOR("RijiN is currently loading, please wait.\nIf you're unable to resolve this, please restart your computer."), "", XOR32(MB_TOPMOST | MB_ICONINFORMATION));
    SAFE_EXIT();
  }

  file_map->create(HASH("RIJIN_LOADER"));
}

static void cleanup_files(){
  file::delete_file(XOR("dxgidmp.dat"));
  file::delete_file(XOR("dxgidmp.exe"));

  const wchar_t* current_dir = utils::get_current_directory();
  assert(current_dir != nullptr);
  if(current_dir != nullptr)
    file::delete_file_type(current_dir, WXOR(L".tmp"));
}


CFLAG_O0 i32 __stdcall WinMain(HINSTANCE instance, HINSTANCE prev, i8* cmd_line, i32 show_cmd){
  #if defined(DEV_MODE)
    alloc_console();
  #endif

  DBG("[!] de-virtualizing...\n");
  VM_FAST_START();
    jm::init_syscalls_list();
    rtp::pre_launch();
    rtp::runtime_protection();
    map_check();
    cleanup_files();
  VM_FAST_STOP();
  DBG("[!] setup\n");

  global->show_loading_page(XOR(L"Setup..."));

  // start login thread
  utils::create_worker(handle_login_thread, nullptr);

  LISTEN_FOR_ERRORS();

  // Start selection GUI
  {
    global->window = utils::create_window(win_procedure, rgb(50, 50, 50), vec3i(500, 300));
    if(global->window == nullptr){
      SHOW_ERROR_STR(ERR_LOADER_FAILED_TO_CREATE_WINDOW, "Failed to create window.\nSomething is blocking our Window creation process.\n", true);
      return 0;
    }

    if(!render->setup(global->window)){
      SHOW_ERROR_STR(ERR_LOADER_FAILED_D2D1_SETUP, "Failed to initialize D2D1.", true);
      return 0;
    }

    // Render in another thread so its sleeping functions don't lock up the main thread.
    utils::create_worker([](void* p){
      while(!global->running)
        Sleep(1);

      while(global->running)
        render_frame();

      return 0;
    }, nullptr);

    global->running = true;
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    I(DestroyWindow)(global->window);
  }

  return 0;
}