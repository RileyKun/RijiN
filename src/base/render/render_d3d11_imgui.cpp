#include "../link.h"

#if defined(RENDERING_MODE_D3D11_IMGUI)
render_d3d11_imgui* render = nullptr;

vec3i c_font_d3d11::get_size(u32 font_size, std::wstring wstr, bool* success){
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  assert(draw_list != nullptr);

  assert(font[0] != nullptr);
  ImVec2 size = ((ImFont*)font[0])->CalcTextSizeA(font_size, FLT_MAX, 0.f, convert::wstr2str(wstr).c_str());

  return vec3i(size.x, size.y);
}

vec3i c_font_d3d11::draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style, u8 align_x, u8 align_y){
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  assert(draw_list != nullptr);
  assert(font[0] != nullptr);

  vec3i size;
  if(align_x != TALIGN_CHEAP || align_y != TALIGN_CHEAP){
    size = get_size(font_size, wstr, nullptr) + vec3i(1,1);
    calc_align(pos, size, align_x, align_y);
  }

  std::string str = convert::wstr2str(wstr);

  if(style == TSTYLE_SHADOW){
    draw_list->AddText((ImFont*)font[0], font_size, ImVec2(pos.x + 1, pos.y + 1), ImColor(0,0,0,(i32)col.a), str.c_str());
  }
  else if(style == TSTYLE_OUTLINE){
    draw_list->AddText((ImFont*)font[0], font_size, ImVec2(pos.x + 1, pos.y), ImColor(0,0,0,(i32)col.a), str.c_str());
    draw_list->AddText((ImFont*)font[0], font_size, ImVec2(pos.x - 1, pos.y), ImColor(0,0,0,(i32)col.a), str.c_str());
    draw_list->AddText((ImFont*)font[0], font_size, ImVec2(pos.x, pos.y + 1), ImColor(0,0,0,(i32)col.a), str.c_str());
    draw_list->AddText((ImFont*)font[0], font_size, ImVec2(pos.x, pos.y - 1), ImColor(0,0,0,(i32)col.a), str.c_str());
  }

  draw_list->AddText((ImFont*)font[0], font_size, ImVec2(pos.x, pos.y), ImColor((i32)col.r, (i32)col.g, (i32)col.b, (i32)col.a), str.c_str());

  return size;
}
#endif