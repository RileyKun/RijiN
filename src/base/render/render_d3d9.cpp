#include "../link.h"
// dev test
#if defined(RENDERING_MODE_D3D9_NEW)
c_render_d3d9* render = nullptr;
vec3i c_font_d3d9::get_size(u32 font_size, std::wstring wstr, bool* success){
  needs_created(font_size);
  if(!valid_font(font_size))
    return vec3i();
  
  auto size = font_data[font_size].text_extent<std::wstring>(wstr);
  return vec3i(size.x, size.y);
}

vec3i c_font_d3d9::draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y){
  needs_created(font_size);
  if(!valid_font(font_size))
    return vec3i();
  
  col.a *= render->alpha_mod_frac;
  col    = math::clamp(col);

  vec3i size;
  if(align_x != TALIGN_CHEAP || align_y != TALIGN_CHEAP){
    size = get_size(font_size, wstr, nullptr) + vec3i(1, 1);
    calc_align(pos, size, align_x, align_y);
  }

  if(style == TSTYLE_SHADOW)
    render->queue.push_text<std::wstring>(font_data[font_size], {pos.x + 1, pos.y + 1}, wstr, {0, 0, 0, col.a > 0 ? col.a / 2 : col.a});
  else if(style == TSTYLE_OUTLINE){
    vec3i draw_offsets[] = {
      vec3i(pos.x + 1, pos.y),
      vec3i(pos.x - 1, pos.y),
      vec3i(pos.x, pos.y + 1),
      vec3i(pos.x, pos.y - 1),
    };

    for(u8 i = 0; i < sizeof(draw_offsets) / sizeof(vec3i); i++)
      render->queue.push_text<std::wstring>(font_data[font_size], {draw_offsets[i].x, draw_offsets[i].y}, wstr, {0, 0, 0, col.a > 0 ? col.a / 2 : col.a});
  }

  render->queue.push_text<std::wstring>(font_data[font_size], {pos.x, pos.y}, wstr, {col.r, col.g, col.b, col.a});
  size = get_size(font_size, wstr, nullptr);

  return size;
}

#endif