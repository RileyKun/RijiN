#include "link.h"

c_esp esp_instance;
c_esp* g_esp = &esp_instance;

void c_esp::run() {
  _local = &_local_data;
  if (g_ow->_gm == 0)
    return;
  
  if (g_ow->_thread_lock == false) {
    memcpy(&_local_data, &(g_local->_local_data), sizeof(_local_data));
    memcpy(_objects, g_state->_objects, sizeof(_objects));
    _object_count = g_state->_object_count;
  }
  
  if (!g_state->is_alive(&(_local->object)))
    return;

  {
    float calc1 = tanf(_fov * (PI / 360.0f));
    float calc2 = atan2f(render->screen_size.y, render->screen_size.x / calc1) * (360.0f / PI);
    float calc3 = tanf(calc2 * (PI / 360.0f));

    _calc1 = calc1;
    _calc3 = calc3;
  }

  for (int i = 0; i < _object_count; ++i) {
    ow_object* obj = &(_objects[i]);
    if (obj->type != OBJECT_PLAYER)
      continue;

    if (!g_state->is_alive(obj))
      continue;

    c_esp_settings* esp_settings = &config->esp[(_local->team == 3 || _local->team != g_state->get_team(obj->filter_bits)) ? 0 : 1];

    if(!esp_settings->enabled)
      continue;

    vec3i size_out;
    vec3i pos_out;

    if(!calc_bounding_box(obj->model_min, vec3(-0.45f, 0.f, -0.45f), vec3(0.45f, calc_nameplate_correction(obj->hero, obj->nameplate_y), 0.45f), size_out, pos_out))
      continue;

    u32 esp_flags = ESP_FLAG_TEXT_OUTLINES;
    {
      if(esp_settings->box)
        esp_flags |= ESP_FLAG_BOX;

      if(esp_settings->box_outlines)
        esp_flags |= ESP_FLAG_BOX | ESP_FLAG_OUTLINES;
    }

    c_esp_data<32> ex_data;
    {
      if(esp_settings->health_bar && obj->total_hp > 0.f && obj->total_max_hp > 0.f){
        colour hp_col;
        float frac = math::clamp(obj->total_hp / obj->total_max_hp, 0.f, 1.f);

        if (obj->max_armor > 0.0f && obj->armor > 0.0f || obj->max_shields > 0.0f && obj->shields > 0.0f)
          hp_col = rgb(36, 192, 255);
        else if(frac >= 0.75f)
          hp_col = rgb(46, 204, 113);
        else if(frac >= 0.5f)
          hp_col = rgb(241, 196, 15);
        else if(frac >= 0.25f)
          hp_col = rgb(230, 126, 34);
        else
          hp_col = rgb(231, 76, 60);

        ex_data.add(ESP_BAR_TYPE_LEFT, frac, esp_settings->health_bar_fractions, hp_col);
      }

      if(esp_settings->name)
        ex_data.add(ESP_TEXT_TYPE_TOP, rgb(255,255,255), global->font_esp_big, 12, utils::str2wstr(g_hero_data[obj->hero].name));

      // Add the distance
      if(esp_settings->distance){
        static wchar_t dist_buf[17];
        wsprintfW(dist_buf, XOR(L"[%im]"), (i32)(_local->vieworigin - obj->model_center).length());

        ex_data.add(ESP_TEXT_TYPE_BOTTOM, rgb(255,255,255), global->font_esp_big, 12, dist_buf);
      }
      
      /*if(true){
        ow_condition* cond = g_state->lookup_condition(obj, HASH("cooldown_shift"));
        if (cond) {
          float shift_time = cond->data.fl;
          colour status_col;
          
          static wchar_t shift_buf[17];
          if (shift_time == 0.0f)
            wsprintfW(shift_buf, L"shift");
          else
            wsprintfW(shift_buf, L"shift %i", (int)(shift_time));
          
          if (shift_time == 0.0f)
            status_col = rgb(255, 255, 255);
          else
            status_col = rgb(128, 128, 128);

          ex_data.add(ESP_TEXT_TYPE_BOTTOM, status_col, global->font_esp_big, 12, shift_buf);
        }
      }*/
    }
    render->draw_esp_box(pos_out, size_out, esp_settings->box_colour, esp_flags, &ex_data);
  }
}

void c_esp::run_glow(CONTEXT* ctx){
  if(ctx == nullptr || gen_internal->has_sub_expired())
    return;

  u64 entity_pos = (u64)ctx->Rcx;

  for (int i = 0; i < g_state->_object_count; ++i) {
    ow_object* obj = &(g_state->_objects[i]);

    if (!g_state->is_alive(obj))
      continue;

    if(_local->team != 3 && _local->team == g_state->get_team(obj->filter_bits))
      continue;

    c_esp_settings* esp_settings = &config->esp[0];

    if(!esp_settings->glow_custom_color && !esp_settings->glow_health_based)
      continue;

    if(entity_pos != obj->pos)
      continue;

    // Ignore Z
    ctx->Rdx |= 1<<7;

    // Set colour register
    colour encode_colour = colour(esp_settings->glow_health_rgb);

    if(esp_settings->glow_health_based){
      float frac = math::clamp(obj->total_hp / obj->total_max_hp, 0.f, 1.f);

      if (obj->max_armor > 0.0f && obj->armor > 0.0f || obj->max_shields > 0.0f && obj->shields > 0.0f)
        encode_colour = rgb(36, 192, 255);
      else if(frac >= 0.75f)
        encode_colour = rgb(46, 204, 113);
      else if(frac >= 0.5f)
        encode_colour = rgb(241, 196, 15);
      else if(frac >= 0.25f)
        encode_colour = rgb(230, 126, 34);
      else
        encode_colour = rgb(231, 76, 60);
    }

    // Encode to 0xAABBGGRR
    ctx->R8 = (i32)encode_colour.r | (i32)encode_colour.g << 8 | (i32)encode_colour.b << 16 | (i32)encode_colour.a << 24;

    return;
  }
}

bool c_esp::w2s(Vector world, vec3i& screen){
  Vector vieworg = _local->vieworigin;
  Vector viewforward = _local->viewforward;
  Vector viewang = Vector(viewforward.pitch(), viewforward.yaw(), 0.0f);

  Vector dt = world - vieworg;

  float cx = (render->screen_size.x * 0.5f);
  float cy = (render->screen_size.y * 0.5f);

  Vector view_normal = (viewang + Vector(0.0f, 180.0f, 0.0f));
  if (view_normal.y > 360.0f)
    view_normal.y -= 360.0f;

  Vector fwd = viewforward;
  Vector right = view_normal.right();
  Vector up = viewang.up();

  Vector transform = Vector(dt.dot(right), dt.dot(up), dt.dot(fwd));

  if (transform.z < 0.001f)
    return false;

  screen.x = cx * (1.0f - (transform.x / _calc1 / transform.z));
  screen.y = cy * (1.0f - (transform.y / _calc3 / transform.z));

  return true;
}

bool c_esp::calc_bounding_box(Vector pos, vec3 min, vec3 max, vec3i& size_out, vec3i& pos_out){
  min += vec3(pos.x, pos.y, pos.z);
  max += vec3(pos.x, pos.y, pos.z);

  vec3i screen_boxes[8];
  vec3 points[] =
  {
    vec3(min.x, min.y, min.z),
    vec3(min.x, max.y, min.z),
    vec3(max.x, max.y, min.z),
    vec3(max.x, min.y, min.z),
    vec3(max.x, max.y, max.z),
    vec3(min.x, max.y, max.z),
    vec3(min.x, min.y, max.z),
    vec3(max.x, min.y, max.z)
  };

  for(u32 i = 0; i <= 7; i++){
    if(!w2s(Vector(points[i].x, points[i].y, points[i].z), screen_boxes[i]))
      return false;
  }

  vec3i box_array[] =
  {
    screen_boxes[3], // Front left
    screen_boxes[5], // Back right
    screen_boxes[0], // Back left
    screen_boxes[4], // Front right
    screen_boxes[2], // Front right
    screen_boxes[1], // Back right
    screen_boxes[6], // Back left
    screen_boxes[7]  // Front Left
  };

  i32 left = screen_boxes[3].x;
  i32 bottom = screen_boxes[3].y;
  i32 right = screen_boxes[3].x;
  i32 top = screen_boxes[3].y;

  for (u32 i = 0; i <= 7; i++){
    if (left > box_array[i].x)
      left = box_array[i].x;

    if (bottom < box_array[i].y)
      bottom = box_array[i].y;

    if (right < box_array[i].x)
      right = box_array[i].x;

    if (top > box_array[i].y)
      top = box_array[i].y;
  }

  pos_out  = vec3i(left, top);
  size_out = vec3i(right - left, bottom - top);

  return true;
}

float c_esp::calc_nameplate_correction(i32 hero_id, float nameplate_y){
  if(hero_id == HERO_DVA)
    nameplate_y *= 0.70f;
  else if(hero_id == HERO_ORISA)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_REINHARDT)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_ROADHOG)
    nameplate_y *= 0.75f;
  else if(hero_id == HERO_SIGMA)
    nameplate_y *= 0.90f;
  else if(hero_id == HERO_WINSTON)
    nameplate_y *= 0.80f;
  else if(hero_id == HERO_HAMMOND)
    nameplate_y *= 0.80f;
  else if(hero_id == HERO_ZARYA)
    nameplate_y *= 0.90f;
  else if(hero_id == HERO_ASHE)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_BASTION)
    nameplate_y *= 0.75f;
  else if(hero_id == HERO_MCCREE)
    nameplate_y *= 0.80f;
  else if(hero_id == HERO_DOOM)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_GENJI)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_HANZO)
    nameplate_y *= 0.75f;
  else if(hero_id == HERO_JUNK)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_MEI)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_PHARAH)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_REAPER)
    nameplate_y *= 0.95f;
  else if(hero_id == HERO_SOLDIER)
    nameplate_y *= 0.95f;
  else if(hero_id == HERO_SOMBRA)
    nameplate_y *= 0.90f;
  else if(hero_id == HERO_SYM)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_TORB)
    nameplate_y *= 0.60f;
  else if(hero_id == HERO_TRACER)
    nameplate_y *= 0.95f;
  else if(hero_id == HERO_WIDOW)
    nameplate_y *= 0.95f;
  else if(hero_id == HERO_ANA)
    nameplate_y *= 0.95f;
  else if(hero_id == HERO_BAPTISTE)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_BRIG)
    nameplate_y *= 0.80f;
  else if(hero_id == HERO_LUCIO)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_MERCY)
    nameplate_y *= 0.85f;
  else if(hero_id == HERO_MOIRA)
    nameplate_y *= 0.90f;
  else if(hero_id == HERO_ZEN)
    nameplate_y *= 0.90f;
  else if(hero_id == HERO_TRAININGBOT1 || hero_id == HERO_TRAININGBOT2 || hero_id == HERO_TRAININGBOT3 || hero_id == HERO_TRAININGBOT4)
    nameplate_y *= 0.95f;

  return nameplate_y;
}