#include "link.h"
//#include "imgui/imgui_internal.h"

static HWND window = NULL;
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
  DWORD wndProcId;
  GetWindowThreadProcessId(handle, &wndProcId);

  if (GetCurrentProcessId() != wndProcId)
    return TRUE; // skip to next window

  window = handle;
  return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
  window = NULL;
  EnumWindows(EnumWindowsCallback, NULL);
  return window;
}

static ImFont* font;
bool setup(void* device){
  if(gen_internal->loaded)
    return true;

  // Fetch real device pointer from gfxtest
  //device = *(void**)((u64)utils::get_module_handle(HASH("gfxtest64.exe")) + (u64)0xC618);
  //device = *(void**)((u64)device + (u64)0x28);
  //device = *(void**)((u64)device + (u64)0x8);

  RENDER_SETUP();

  global->d3d9_hook_trp = gen_internal->get_pkg(HASH("d3d9_hook"));

  if(!gen_internal->setup())
    return false;

  //TEST* ctx = IM_NEW(TEST)(nullptr);

  do
    window = GetProcessWindow();
  while (window == NULL);

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
  ImGui_ImplWin32_Init(window);
  ImGui_ImplDX9_Init(device);
  ImGui::GetIO().ImeWindowHandle = window;

  //font = io.Fonts->AddFontFromFileTTF("C:\\segoeuisl.ttf", 18);

 ImFontConfig font_cfg;
 font_cfg.FontDataOwnedByAtlas = false;
 font_cfg.MergeMode = false;
 io.Fonts->AddFontFromFileTTF("C:\\segoeuisl.ttf", 25, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
 font_cfg.MergeMode = true;
 font_cfg.PixelSnapH = true;
 static const ImWchar icons_ranges_md[] = { 0, 65535, 0 };
 font = io.Fonts->AddFontFromFileTTF("C:\\segoeuisl.ttf", 18, &font_cfg, icons_ranges_md);
 io.Fonts->Build();

  return true;
}

EXPORT HRESULT __fastcall d3d9_hook(void* ecx, void* edx, void* device, RECT* source_rect, RECT* dest_rect, HWND window_override, void* region){
  if(!setup(device))
    return D3D_OK;

  input_system->update();
  if(gen_internal->should_unload())
    gen_internal->unload();

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImVec2(1024, 768));
  ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);

  auto draw_list = ImGui::GetWindowDrawList();

  //draw_list->AddRectFilled(ImVec2(50, 50), ImVec2(150, 150), ImColor(255, 0, 0, 120));

  // TODO: GET FREETYPE FONTS WORKING INSTEAD OF TRUETYPE

  ImGuiIO& io = ImGui::GetIO();
  draw_list->AddText(nullptr, 25, ImVec2(50 + 1, 50 + 1), ImColor(0,0,0), "Блять негр");
  draw_list->AddText(nullptr, 25, ImVec2(50, 50), ImColor(230,230,230), "Блять негр");

  ImGui::End();
  ImGui::EndFrame();
  ImGui::Render();
  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

  //IDirect3DStateBlock9* state_block = render->begin();
  //{
  //  render->gradient_rect(vec3i(0,0), vec3i(640, 480), rgb(255,0,0), rgb(0,255,0), rgb(0,0,255), rgb(255,255,255));
  //}
  //render->end(state_block);

  return utils::call_fastcall<HRESULT, void*, RECT*, RECT*, HWND, void*>(gen_internal->decrypt_asset(global->d3d9_hook_trp), ecx, edx, device, source_rect, dest_rect, window_override, region);
}