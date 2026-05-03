#include "link.h"

static c_font_base* font;
bool setup(IDXGISwapChain* swap_chain){
  if(gen_internal->loaded)
    return true;

  global->d3d11_hook_trp = gen_internal->get_pkg(HASH("d3d11_hook"));

  if(!gen_internal->setup())
    return false;

  RENDER_SETUP();
  render->setup(swap_chain);

  // Create test font
  {
    font = render->create_font("C:\\segoeui.ttf", 18);
    assert(font != nullptr);
    render->build_all_fonts();
  }

  return true;
}

EXPORT HRESULT __fastcall d3d11_hook(void* rcx, void* rdx, u32 sync_interval, u32 flags){
  if(!setup((IDXGISwapChain*)rcx))
    return 0;

  input_system->update();
  if(gen_internal->should_unload())
    gen_internal->unload();

  render->begin();
  render->screen_size = vec3i(1600,900);

  render->draw_watermark(font, true);

  static gcs_test* test = gcs::add_scene<gcs_test>();
  static void* window = utils::get_current_process_window();
  gcs::render_scene(test, math::mouse_pos(window), font);

  render->end();

  return utils::call_fastcall<HRESULT, u32, u32>(gen_internal->decrypt_asset(global->d3d11_hook_trp), rcx, rdx, sync_interval, flags);
}