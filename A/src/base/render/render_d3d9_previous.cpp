#include "../link.h"

#if defined(RENDERING_MODE_D3D9)
#if !defined(RENDERING_MODE_D3D9_NEW)
c_render_d3d9* render = nullptr;

vec3i c_font_d3d9::get_size(u32 font_size, std::wstring wstr, bool* success){
  if(success != nullptr)
    *success = false;

  font_size = math::smallest(render->rescale(font_size), (sizeof(font) / sizeof(void*)) - 1);
  if(font_size >= sizeof(font) / sizeof(void*))
    return vec3i();

  ID3DXFont* use_font = (ID3DXFont*)font[font_size];

  if(use_font == nullptr)
    return vec3i();

  RECT rect = {};

  use_font->DrawTextW(nullptr, wstr.c_str(), wstr.length(), &rect, DT_CALCRECT, 0);
  if(success != nullptr)
    *success = true;

  return vec3i((rect.right - rect.left), (rect.bottom - rect.top));
}

CFLAG_Ofast vec3i c_font_d3d9::draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y){
  u32 o_font_size = font_size;
  font_size = math::smallest(render->rescale(font_size), (sizeof(font) / sizeof(void*)) - 1);
  if(font_size >= sizeof(font) / sizeof(void*)){
    DBG("[-] c_render_d3d9::draw font %i is too high for %i\n", font_size, sizeof(font) / sizeof(void*));
    return vec3i();
  }

  // draw anything remainding first
  render->flush_to_gpu(render->current_primitive_type);

  ID3DXFont* use_font = (ID3DXFont*)font[font_size];

  if(use_font == nullptr)
    return vec3i();

  col.a *= render->alpha_mod_frac;
  col   = math::clamp(col);

  vec3i size;
  if(align_x != TALIGN_CHEAP || align_y != TALIGN_CHEAP){
    size = get_size(o_font_size, wstr, nullptr) + vec3i(1,1);
    calc_align(pos, size, align_x, align_y);
  }

  RECT rect = {
    pos.x, pos.y
  };

  if(style == TSTYLE_SHADOW){
    RECT shadow_rect = {
      pos.x + 1, pos.y + 1
    };
    use_font->DrawTextW(nullptr, wstr.c_str(), wstr.length(), &shadow_rect, DT_NOCLIP, rgba(0,0,0,col.a > 0 ? col.a / 2 : col.a).tohex() );
  } else if(style == TSTYLE_OUTLINE){
    RECT draw_offsets[] = {
      {pos.x + 1, pos.y},
      {pos.x - 1, pos.y},
      {pos.x, pos.y + 1},
      {pos.x, pos.y - 1},
    };

    for(u8 i = 0; i < sizeof(draw_offsets) / sizeof(RECT); i++)
      use_font->DrawTextW(nullptr, wstr.c_str(), wstr.length(), &draw_offsets[i], DT_NOCLIP, rgba(0,0,0,col.a > 0 ? col.a / 2 : col.a).tohex() );
  }

  use_font->DrawTextW(nullptr, wstr.c_str(), wstr.length(), &rect, DT_NOCLIP, col.tohex() );

  return size;
}
#endif
#endif