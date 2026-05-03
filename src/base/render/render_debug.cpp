#include "../link.h"

#if defined(RENDER_3D)

CLASS_ALLOC(c_render_debug, render_debug);

bool c_render_debug_obj_line::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);

  vec3i last_pos, this_pos;
  if(!render_debug->world2screen(pos1, last_pos) || !render_debug->world2screen(pos2, this_pos))
    return false;

  if(outline){
    render->solid_line(last_pos - vec3i(1, 1, 0), this_pos - vec3i(1, 1, 0), colour(0, 0, 0, 255), false, 0.f);
    render->solid_line(last_pos + vec3i(1, 1, 0), this_pos + vec3i(1, 1, 0), colour(0, 0, 0, 255), false, 0.f);
    //render->solid_line(last_pos - vec3i(0, 1, 0), this_pos - vec3i(0, 1, 0), colour(0, 0, 0, 255), false, 0.f);
    //render->solid_line(last_pos + vec3i(0, 1, 0), this_pos + vec3i(0, 1, 0), colour(0, 0, 0, 255), false, 0.f);
  }
  render->solid_line(last_pos, this_pos, col, false, 0.f);
  return true;
}

bool c_render_debug_obj_box::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);

  vec3i scr_pos;
  if(!render_debug->world2screen(pos, scr_pos))
    return false;

  if(filled)
    render->filled_rect(scr_pos, vec3i(w, h), col, 0.f);
  else
    render->outlined_rect(scr_pos, vec3i(w, h), col, 0.f);

  return true;
}

bool c_render_debug_obj_gradient_box::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);

  vec3i scr_pos;
  if(!render_debug->world2screen(pos, scr_pos))
    return false;

  render->gradient_rect(scr_pos, vec3i(w, h), top_left, top_right, bottom_left, bottom_right);
  return true;
}

bool c_render_debug_obj_circle::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);

  vec3i scr_pos;
  if(!render_debug->world2screen(pos, scr_pos))
    return false;

  if(filled)
    render->filled_circle(scr_pos, radius, sides, col);
  else
    render->outlined_circle(scr_pos, radius, sides, col);

  return true;
}

bool c_render_debug_obj_3dbox::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);

  #if defined(RENDERING_MODE_D3D9)
  matrix3x4 m;

  angles.x = math::normalize_angle(angles.x);
  angles.y = math::normalize_angle(angles.y);
  angles.z = math::normalize_angle(angles.z);

  math::angle_matrix(angles, m);

  vec3 points[8];
  for(i32 i = 0; i < 8; i++){
    points[i].x = (i & 0x1) ? maxs.x : mins.x;
    points[i].y = (i & 0x2) ? maxs.y : mins.y;
    points[i].z = (i & 0x4) ? maxs.z : mins.z;

    points[i] = math::transform(points[i], m);
    points[i] += pos;
  }

  i32 box_face_indices[6][4] =
  {
    { 0, 4, 6, 2 },
    { 5, 1, 3, 7 },
    { 0, 1, 5, 4 },
    { 2, 6, 7, 3 },
    { 0, 2, 3, 1 },
    { 4, 5, 7, 6 }
  };

  // Box
  if(face_col.w > 0){
#if !defined(RENDERING_MODE_D3D9_NEW)
    for(i32 i = 0; i < 6; i++){
      i32* face = box_face_indices[i];
      for(i32 j = 0; j < 3; j++){
        i32 i0 = face[0];
        i32 i1 = face[j];
        i32 i2 = face[j + 1];

        vec3i scr_pos1, scr_pos2, scr_pos3;
        if(!render_debug->world2screen(points[i0], scr_pos1) || !render_debug->world2screen(points[i1], scr_pos2) || !render_debug->world2screen(points[i2], scr_pos3))
          return false;

        c_vertex verts[3] = {
          {vec4(scr_pos1.x, scr_pos1.y, 0.f, 1.f), face_col.tohex()},
          {vec4(scr_pos3.x, scr_pos3.y, 0.f, 1.f), face_col.tohex()},
          {vec4(scr_pos2.x, scr_pos2.y, 0.f, 1.f), face_col.tohex()},
        };

        for(i32 a = 0; a < 3; a++){
          #if defined(RENDERING_MODE_D3D9)
            c_render_d3d9* d3d9 = (c_render_d3d9*)render;
            d3d9->add_vert(verts[a], D3DPT_TRIANGLELIST);
          #endif
        }
      }
    }
#endif
  }


  // box outline
  if(outline_col.w > 0)
  {
    for(i32 i = 0; i < 6; i++){
      i32* face_index = box_face_indices[i];
      for(i32 j = 0; j < 4; j++){
        vec3i scr_pos1, scr_pos2;
        if(!render_debug->world2screen(points[face_index[j]], scr_pos1) || !render_debug->world2screen(points[face_index[j == 3 ? 0 : j + 1]], scr_pos2))
          return false;

        render->solid_line(scr_pos1, scr_pos2, outline_col, false, 0.f);
      }
    }
  }

  #endif
  return true;
}

bool c_render_debug_obj_font::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);
  if(render_debug->font == nullptr){
    assert(false && "Forgot to call render_debug->setup_fonts()!");
    return false;
  }

  if(offset < 1)
    offset = 1;

  vec3i scr_pos;
  if(!render_debug->world2screen(pos, scr_pos))
    return false;

  #if defined(RENDERING_MODE_D3D9)
    c_font_d3d9* d3d9 = (c_font_d3d9*)render_debug->font;

    wchar_t new_buf[1024];
    convert::str2wstr(buf, new_buf, sizeof(new_buf));

    d3d9->draw(vec3i(scr_pos.x, scr_pos.y + (14 * offset)), 14, new_buf, face_col, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_TOP);
  #endif

  return true;
}

bool c_render_debug_obj_2d_font::paint(){
  assert(render != nullptr);
  assert(render_debug != nullptr);
  if(render_debug->font == nullptr){
    assert(false && "Forgot to call render_debug->setup_fonts()!");
    return false;
  }

  if(offset < 1)
    offset = 1;

  vec3i scr_pos = vec3i((i32)pos.x, (i32)pos.y, 0);
  #if defined(RENDERING_MODE_D3D9)
    c_font_d3d9* d3d9 = (c_font_d3d9*)render_debug->font;

    wchar_t new_buf[1024];
    convert::str2wstr(buf, new_buf, sizeof(new_buf));

    d3d9->draw(vec3i(scr_pos.x, scr_pos.y + (14 * offset)), 14, new_buf, face_col, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);
  #endif

  return true;
}

#endif