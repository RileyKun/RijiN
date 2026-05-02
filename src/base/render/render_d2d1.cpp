#include "../link.h"

#if defined(RENDERING_MODE_D2D1)
CLASS_ALLOC(c_render_d2d1, render);

vec3i c_font_d2d1::get_size(u32 font_size, std::wstring wstr, bool* success){
  if(success != nullptr)
    *success = false;

  if(font_size >= sizeof(font) / sizeof(void*))
    return vec3i();

  IDWriteTextLayout* layout;

  u32 status = 0;
  if((status = render->dwrite_factory->CreateTextLayout( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], 4096.f, 4096.f, &layout )) != 0)
    return vec3i();

  DWRITE_TEXT_METRICS metrics;
  layout->GetMetrics(&metrics);
  layout->Release();

  if(success != nullptr)
    *success = true;

  return vec3i(metrics.width, metrics.height);
}

vec3i c_font_d2d1::draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y){
  render->apply_render_offset(pos);

#if defined(INTERNAL_CHEAT)
  font_size = (i32)((float)font_size * 0.75f);
#endif

  if(font_size >= sizeof(font) / sizeof(void*))
    return vec3i();

  col = math::clamp(col);

  vec3i size;

  if(align_x != TALIGN_CHEAP || align_y != TALIGN_CHEAP){
    size = get_size(font_size, wstr, nullptr) + vec3i(1,1);
    calc_align(pos, size, align_x, align_y);
  }

  if(style == TSTYLE_SHADOW){
    render->set_colour(rgb(0,0,0));
    render->render_target->DrawTextA( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], D2D1::RectF(pos.x + 1, pos.y + 1, pos.x + 1 + size.x, pos.y + 1 + size.y), render->brush );
  }
  else if(style == TSTYLE_OUTLINE){
    render->set_colour(rgb(0,0,0));
    render->render_target->DrawTextA( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], D2D1::RectF(pos.x + 1, pos.y, pos.x + 1 + size.x, pos.y + size.y), render->brush );
    render->render_target->DrawTextA( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], D2D1::RectF(pos.x - 1, pos.y, pos.x - 1 + size.x, pos.y + size.y), render->brush );
    render->render_target->DrawTextA( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], D2D1::RectF(pos.x, pos.y + 1, pos.x + size.x, pos.y + 1 + size.y), render->brush );
    render->render_target->DrawTextA( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], D2D1::RectF(pos.x, pos.y - 1, pos.x + size.x, pos.y - 1 + size.y), render->brush );
  }

  render->set_colour(col);
  render->render_target->DrawTextA( &wstr[0], wstr.length( ), (IDWriteTextFormat*)font[font_size], D2D1::RectF(pos.x, pos.y, pos.x + size.x, pos.y + size.y), render->brush );

  //render->outlined_rect(pos, size, rgb(255,0,0));

  return size;
}
#endif