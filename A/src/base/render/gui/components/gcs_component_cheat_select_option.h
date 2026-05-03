#pragma once

#if defined(RENDERING_MODE_D2D1)

class license_object;

class gcs_component_cheat_select_option : public gcs_component_base{
  void*           param               = nullptr;
  bool            has_override_colour = false;
  colour          override_colour;
public:
  license_object* license_obj;
  ID2D1Bitmap*    bitmap_icon_data;

  void setup() override{
    set_size(vec3i(0, 50));
  }

  void think(vec3i pos, vec3i size) override{
  }

  void pre_run() override{
  }

  void post_run() override{
  }

  void set_param(void* p){
    param = p;
  }

  void set_license_object(license_object* obj){
    license_obj = obj;
  }

  void set_colour_override(colour col){
    has_override_colour = true;
    override_colour     = col;
  }

  void remove_colour_override(){
    has_override_colour = false;
    override_colour     = rgb(0, 0, 0);
  }

  void draw(vec3i pos, vec3i size){
    if(license_obj == nullptr)
      return;

    gcs_colour_scheme* _scheme = scheme();
    gcs_scene_info* _scene = scene()->info();

    colour draw_col;

    if(has_override_colour)
      draw_col = override_colour;
    else
      draw_col = is_in_rect() ? _scheme->main * 1.20f : rgb(70,77,92);

    render->filled_rect(pos, size, rgb(39, 46, 72), 3.f);
    render->outlined_rect(pos, size, draw_col, 3.f);
    draw_col.a = 255.f * 0.5f;

    // handle bitmap drawing of cheat icon
    vec3i bitmap_size = vec3i(32, 32);
    vec3i bitmap_pos  = pos + vec3i(7 + (bitmap_size.x / 2), size.y / 2) - (bitmap_size / 2);
    {
      //u32 buffer_len;
      //static void* buffer_test = utils::read_file(L"boss.png", buffer_len);
//
      //static ID2D1Bitmap* bitmap_test;
      //if(bitmap_test == nullptr){
      //  bitmap_test = render->create_bitmap(buffer_test, buffer_len);
      //}
      //else
      //  render->draw_bitmap(bitmap_test, bitmap_pos, bitmap_size);

      // If the bitmap is nullptr then display a buffering progress circle
      if(bitmap_icon_data == nullptr){
        float anim_x = (float)((i32)(math::time() * 360.f) % 360);

        render->progress_circle(bitmap_pos, bitmap_size, 5.f, anim_x, rgb(230,230,230), rgb(115,115,115));
      }
      else
        render->draw_bitmap(bitmap_icon_data, bitmap_pos, bitmap_size);
    }

    // Cheat name
    _scene->font->draw(pos + vec3i(10 + (bitmap_size.x + 3), size.y / 2), 20, name(), _scheme->text, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_CENTER);

    // status animation
    /*
    vec3i status_start_pos;
    {
      float radius     = 8.f;
      float anim_x     = (float)((i32)(math::time() * radius) % (i32)radius);
      float anim_p     = anim_x > 0.f ? (anim_x / radius) : 0.f;
      status_start_pos = pos + vec3i(size.x - (13 + (i32)radius), size.y / 2);

      colour target_col = license_obj->disabled ? _scheme->error : rgb(46, 204, 113);
      render->filled_circle(status_start_pos, radius, 100, license_obj->disabled ? _scheme->error : target_col);
      render->filled_circle(status_start_pos, anim_x, 100, math::lerp(target_col * 1.25f, target_col, anim_p));
    }
    */

    // Handle status
    {
      auto get_status_str = [](license_object* license_obj) -> std::wstring {
        assert(license_obj != nullptr);

        if(license_obj->disabled)
          return XOR(L"DISABLED / UPDATING");

#if defined(STAGING_MODE)
        return XOR(L"★ STAGING BUILD");
#elif defined(DEV_MODE)
        return XOR(L"★ DEV BUILD");
#endif

        return license_obj->expire_str[0] > 0 ? convert::str2wstr(license_obj->expire_str) : XOR(L"UNKNOWN");
      };

      auto get_status_foreground_col = [](license_object* license_obj) -> colour {
        assert(license_obj != nullptr);

        if(license_obj->disabled)
          return rgb(255,91,92);

#if defined(STAGING_MODE) || defined(DEV_MODE)
        return rgba(186, 220, 88, 200);
#endif

        return license_obj->expire_str[0] > 0 ? rgb(57, 218, 138) : rgba(253, 172, 65, 200);
      };

      auto get_status_background_col = [](license_object* license_obj) -> colour {
        assert(license_obj != nullptr);

        if(license_obj->disabled)
          return rgba(255,91,92, 43);

#if defined(STAGING_MODE) || defined(DEV_MODE)
        return rgba(106, 176, 76, 43);
#endif

        return license_obj->expire_str[0] > 0 ? rgba(57, 218, 138, 43) : rgba(253, 172, 65, 43);
      };

      // Expire & animation
      {
        std::wstring status_wstr = get_status_str(license_obj);

        vec3i size_padding   = vec3i(40, 4);
        vec3i pred_font_size = _scene->font->get_size(12, status_wstr, nullptr);

        vec3i start_pos      = pos + vec3i(size.x - (pred_font_size.x + size_padding.x + 10), (size.y / 2) - ((pred_font_size.y + size_padding.y) / 2));
        vec3i size           = pred_font_size + size_padding;

        render->filled_rect(start_pos, size, get_status_background_col(license_obj), 3.f);

        _scene->font->draw(start_pos + (size / 2), 12, status_wstr, get_status_foreground_col(license_obj), TSTYLE_NONE, TALIGN_CENTER, TALIGN_CENTER);
      }
    }

    //if(license_obj->expire_str[0] > 0)
      //_scene->font->draw(status_start_pos - vec3i(13, 0), 12, convert::str2wstr(license_obj->expire_str), _scheme->text, TSTYLE_SHADOW, TALIGN_RIGHT, TALIGN_CENTER);

    if(is_in_rect() && input() & GCS_IN_M1_CLICK){
      call_hook(GCS_HOOK_CLICK, param);

      emit_click_sound(1);
    }

    render->push_clip_region(vec3i(), vec3i());
    render->pop_clip_region();
  }
};

#endif