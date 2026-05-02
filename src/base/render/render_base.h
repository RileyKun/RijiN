#pragma once

enum ignore_rounding_corners{
  ROUNDING_IGNORE_TOP_LEFT = (1 << 1),
  ROUNDING_IGNORE_TOP_RIGHT = (1 << 2),
  ROUNDING_IGNORE_BOTTOM_LEFT = (1 << 3),
  ROUNDING_IGNORE_BOTTOM_RIGHT = (1 << 4)
};

enum render_mode{
  RENDER_MODE_INVALID = 0,
  RENDER_MODE_D2D1 = 1,
  RENDER_MODE_D3D9 = 2,
  RENDER_MODE_D3D11 = 3,
  RENDER_MODE_D3D11_IMGUI = 4,
};

enum render_text_align{
  TALIGN_CHEAP = 0,
  TALIGN_LEFT = 1,
  TALIGN_RIGHT = 2,
  TALIGN_CENTER = 3,
  TALIGN_TOP = 4,
  TALIGN_BOTTOM = 5,
};

enum render_text_style{
  TSTYLE_NONE = 0,
  TSTYLE_SHADOW = 1,
  TSTYLE_OUTLINE = 2,
};

enum esp_flags {
  ESP_FLAG_BOX = 1 << 0,
  ESP_FLAG_OUTLINES = 1 << 1,
  ESP_FLAG_TEXT_OUTLINES = 1 << 2,
};

enum esp_text_type{
  ESP_TEXT_TYPE_TOP     = 0,
  ESP_TEXT_TYPE_BOTTOM  = 1,
  ESP_TEXT_TYPE_RIGHT   = 2,

  ESP_BAR_TYPE_TOP      = 3,
  ESP_BAR_TYPE_BOTTOM   = 4,
  ESP_BAR_TYPE_LEFT     = 5,
  ESP_BAR_TYPE_RIGHT    = 6,
};

enum progress_bar_flags{
  PROGRESS_BAR_OUTLINE        = 1 << 1,
  PROGRESS_BAR_VERTICAL_DOWN  = 1 << 2,
  PROGRESS_BAR_VERTICAL_UP    = 1 << 3,
};

class c_font_base;
struct s_esp_data_entry{
  u8            type;
  colour        col;
  c_font_base*  font = nullptr;
  u8            font_size;
  std::wstring  text;

  float         bar_fraction;
  colour        bar_col;
  u8            bar_fraction_count = 0;
};

template<u8 c = 0>
class c_esp_data{
public:
  float             hp_fraction;
  colour            hp_col;
  u8                fraction_count = 0;
  u8                count = 0;
  s_esp_data_entry  list[c];

  CFLAG_Ofast ALWAYSINLINE void add(esp_text_type type, colour col){
    if(count >= c)
      return;

    s_esp_data_entry* e = &list[count];
    {
      e->type = type;
      e->col  = col;
      e->text.clear();
    }
    count++;
  }

  CFLAG_Ofast ALWAYSINLINE void add(esp_text_type type, float fraction, u8 fraction_count, colour col){
    if(count >= c)
      return;

    s_esp_data_entry* e = &list[count];
    {
      e->type               = type;
      e->col                = col;
      e->bar_fraction       = fraction;
      e->bar_col            = col;
      e->bar_fraction_count = fraction_count;
    }
    count++;
  }

  CFLAG_Ofast ALWAYSINLINE void add(esp_text_type type, colour col, c_font_base* font, u8 font_size, std::wstring text){
    if(count >= c)
      return;

    s_esp_data_entry* e = &list[count];
    {
      e->type      = type;
      e->col       = col;
      e->font      = font;
      e->font_size = font_size;
      e->text      = text;
    }
    count++;
  }
};

class c_render_base;
class c_font_base{
public:
  void* render_base;
  void* font[50];

  CFLAG_Ofast virtual void calc_align(vec3i& pos, vec3i size, u8 align_x, u8 align_y){
    // X
    switch(align_x){
      case TALIGN_RIGHT:
        pos.x -= size.x;
        break;
      case TALIGN_CENTER:
        pos.x -= size.x * .5f;
        break;
    }

    // Y
    switch(align_y){
      case TALIGN_CENTER:
        pos.y -= size.y * .5f;
        break;
      case TALIGN_TOP:
        pos.y -= size.y;
        break;
      case TALIGN_BOTTOM:
        pos.y += size.y;
        break;
    }
  }

  virtual vec3i get_size(u32 font_size, std::wstring wstr, bool* success){
    if(success != nullptr)
      *success = false;

    return vec3i();
  }

  virtual vec3i draw(vec3i pos, u32 font_size, std::wstring wstr, colour col, u8 style = TSTYLE_NONE, u8 align_x = TALIGN_LEFT, u8 align_y = TALIGN_LEFT){
    return vec3i();
  }

  virtual void draw_bytes(vec3i pos, u32 font_size, std::string str, colour col, u8 style = TSTYLE_NONE, u8 align_x = TALIGN_LEFT, u8 align_y = TALIGN_LEFT){
    draw(pos, font_size, convert::str2wstr(str), col, style, align_x, align_y);
  }
};

class c_render_base
{
public:
  bool          is_setup = false;
  vec3i         screen_size;
  u32           fps;
  float         frametime;
  u8            font_count;
  float         alpha_mod_frac;
  c_font_base*  fonts[16];
  bool          gradient_rounding_hack;

  c_render_base(){};

  virtual float scale(){
    // I think for now we just stay with up-scaling on 2k and above.
    if(screen_size.x >= 2560)
      return 1.25f;

    return 1.f;
  }

  virtual i32 rescale(i32 v){
    float cur_scale = scale();
    if(cur_scale == 1.0f)
      return v;

    v = (i32)math::round((float)v * scale());
    if(math::odd(v))
      v++;

    return v;
  }

  virtual vec3i rescale(vec3i v){
    return vec3i(rescale(v.x), rescale(v.y), 0);
  }

  virtual render_mode mode(){
    return RENDER_MODE_INVALID;
  }

  virtual void begin_draw(){

  }

  virtual void end_draw(){

  }

  virtual void unload(bool device_reset){

  }

  virtual float framtime_start(){
    return math::time(true);
  }

  virtual void frametime_stop(float start, u32 target_fps = 0){
    const float frametime_stop    = math::time(true);
    const float frametime_start   = start;
    float total_frametime         = frametime_stop - frametime_start;

    if(target_fps > 0){
      const float target_time = 1000.f / (float)target_fps;

      // If we managed to render the entire frame in under the target time
      // Take the target time and subtract it from the frametime which will
      // Give us the delta until we match the frametime for x frames per second
      if(total_frametime < target_time){
        I(Sleep)(target_time - total_frametime);
        total_frametime += target_time - total_frametime; // Add on the waited time to frametime
      }
    }

    fps                           = total_frametime > 0.f ? (u32)(1000.f / total_frametime) : 0;
    frametime                     = total_frametime / 1000.f;
  }

  virtual void filled_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0){
  }

  virtual void outlined_rect(vec3i pos, vec3i size, colour col, float rounding = 0.f, u32 rounding_flags = 0){

  }

  virtual void gradient_rect_linear(vec3i pos, vec3i size, vec3i start, vec3i end, colour from, colour to, float rounding = 0.f){

  }

  virtual void gradient_rect(vec3i pos, vec3i size, colour top_left, colour top_right, colour bottom_left, colour bottom_right, float rounding = 0.f){

  }

  virtual void solid_line(vec3i pos1, vec3i pos2, colour col, bool anti_alias = false, float width = 1.f){

  }

  virtual void filled_circle(vec3i pos, float radius, float sides, colour col){

  }

  virtual void outlined_circle(vec3i pos, float radius, float sides, colour col){

  }

  virtual void tick_mark(vec3i pos, colour col, float px){

  }

  virtual void push_clip_region(vec3i pos, vec3i size){

  }

  virtual void pop_clip_region(){

  }

  virtual c_font_base* create_font(std::wstring family_name, u32 font_weight, u32 font_style ){
    assert(false && "create_font no override");
    return nullptr;
  }

  virtual bool register_font_base(c_font_base* font_base){
    if(font_count >= sizeof(fonts) / sizeof(c_font_base*)){
      DBG("[-] create_font_base failed, maximum font count of %i\n", font_count);
      return false;
    }

    fonts[font_count++] = font_base;

    DBG("[+] Created font %i/%i\n", font_count, sizeof(fonts) / sizeof(c_font_base*));

    return true;
  }

  ALWAYSINLINE void draw_progress_bar(vec3i pos, vec3i size, float frac, colour col, u32 flags = 0, i32 max_fractions = 10, colour background_col = rgb(0,0,0)){
    if(flags & PROGRESS_BAR_OUTLINE)
      outlined_rect(pos - vec3i(1, 1), size + vec3i(2, 2), colour(background_col.x, background_col.y, background_col.z, background_col.w > 0 ? background_col.w / 2 : background_col.w));

    filled_rect(pos, size, colour(background_col.x, background_col.y, background_col.z, background_col.w > 0 ? background_col.w / 2 : background_col.w));

    if(frac >= 1.f)
      filled_rect(pos, size, col);
    else if(flags & PROGRESS_BAR_VERTICAL_DOWN)
      filled_rect(pos, vec3i(size.x, math::ceil((float)size.y * math::clamp(frac, 0.f, 1.f))), col);
    else if(flags & PROGRESS_BAR_VERTICAL_UP){
      i32 new_size_y = math::ceil(size.y * math::clamp(frac, 0.f, 1.f));
      filled_rect(pos + vec3i(0.f, (size.y - new_size_y), size.y), vec3i(size.x, new_size_y), col);
    }
    else
      filled_rect(pos, vec3i(math::ceil(size.x * math::clamp(frac, 0.f, 1.f)), size.y), col);

    if(max_fractions > 0){
      if(flags & PROGRESS_BAR_VERTICAL_UP || flags & PROGRESS_BAR_VERTICAL_DOWN){
        for(float i = 0.f; i <= 1.f; i += 1.f / (float)max_fractions)
          if(i > 0.f)
            filled_rect(vec3i(pos.x, pos.y + math::ceil((float)size.y * i)), vec3i(size.x, 1), background_col);

        if(frac < 1.f)
          filled_rect(pos, vec3i(size.x, math::ceil(size.y * (1.f - frac) + 0.5f)), background_col);
      }
      else{
        for(float i = 0.f; i <= 1.f; i += 1.f / (float)max_fractions)
          if(i > 0.f)
            filled_rect(vec3i(pos.x + math::ceil((float)size.x * i), pos.y), vec3i(1, size.y), background_col);

        if(frac < 1.f){
          i32 size_x = math::ceil(size.x * (1.f - frac) + 0.5f);
          filled_rect(pos + vec3i(size.x - size_x, 0), vec3i(size_x, size.y), background_col);
        }
      }
    }
  }

  template<auto c>
  CFLAG_Ofast ALWAYSINLINE void draw_esp_box(vec3i pos, vec3i size, colour box_col, u32 flags, c_esp_data<c>* esp_data = nullptr){
    if(flags & ESP_FLAG_BOX){
      outlined_rect(pos, size, box_col);

      if(flags & ESP_FLAG_OUTLINES){
        outlined_rect(pos + vec3i(1, 1), size - vec3i(2, 2), rgba(0,0,0,127));
        outlined_rect(pos - vec3i(1, 1), size + vec3i(2, 2), rgba(0,0,0,127));
      }
    }

    //if(esp_data != nullptr && flags & ESP_FLAG_HEALTH_BAR && esp_data->hp_fraction > 0.f)
      //draw_progress_bar(pos - vec3i(5, 0.f), vec3i(2, size.y), esp_data->hp_fraction, esp_data->hp_col, PROGRESS_BAR_OUTLINE | PROGRESS_BAR_VERTICAL_UP, esp_data == nullptr ? 0 : esp_data->fraction_count);

    if(esp_data != nullptr){
      i32 left_height  = 0;
      i32 right_height  = 0;
      i32 bottom_height = (mode() == RENDER_MODE_D2D1) ? 1 : 0;
      i32 top_height    = (mode() == RENDER_MODE_D2D1) ? 1 : 0;
      i32 left_offset   = 0;
      i32 right_offset  = 0;

      for(u8 i = 0; i < esp_data->count; i++){
        s_esp_data_entry* e = &esp_data->list[i];

        switch(e->type){
          case ESP_TEXT_TYPE_TOP:{
            top_height += e->font->draw(vec3i(pos.x + (size.x / 2), pos.y - (flags & ESP_FLAG_OUTLINES ? 1 : 0) - top_height), (u32)e->font_size, e->text, e->col, flags & ESP_FLAG_TEXT_OUTLINES ? TSTYLE_OUTLINE : TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_TOP).y;
            break;
          }
          case ESP_TEXT_TYPE_BOTTOM:{
            bottom_height += e->font->draw(vec3i(pos.x + (size.x / 2), pos.y + size.y + (flags & ESP_FLAG_OUTLINES ? 1 : 0) + bottom_height), (u32)e->font_size, e->text, e->col, flags & ESP_FLAG_TEXT_OUTLINES ? TSTYLE_OUTLINE : TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_LEFT).y;
            break;
          }
          case ESP_TEXT_TYPE_RIGHT:{
            right_height += e->font->draw(vec3i(pos.x + size.x + 1 + (flags & ESP_FLAG_OUTLINES ? 1 : 0) + right_offset, pos.y + right_height), (u32)e->font_size, e->text, e->col, flags & ESP_FLAG_TEXT_OUTLINES ? TSTYLE_OUTLINE : TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_LEFT).y;
            break;
          }
          case ESP_BAR_TYPE_LEFT:{
            left_offset += 5;
            draw_progress_bar(pos - vec3i(left_offset, 0.f), vec3i(2, size.y), e->bar_fraction, e->bar_col, PROGRESS_BAR_OUTLINE | PROGRESS_BAR_VERTICAL_UP, e->bar_fraction_count);

            break;
          }
          case ESP_BAR_TYPE_RIGHT:{
            draw_progress_bar(pos + vec3i(size.x + right_offset + 3, 0.f), vec3i(2, size.y), e->bar_fraction, e->bar_col, PROGRESS_BAR_OUTLINE | PROGRESS_BAR_VERTICAL_UP, e->bar_fraction_count);
            right_offset += 5;

            break;
          }
        };
      }
    }
  }

  void draw_watermark(c_font_base* font, bool should_show_watermark = true, i32 font_size = 14){
    if(font == nullptr || screen_size.x == 0 || screen_size.y == 0)
      return;

    #if defined(STAGING_MODE)
      should_show_watermark = true;
    #endif

    const i32 padding = 4;
    if(should_show_watermark){
      #if defined(INTERNAL_CHEAT)
        create_server_iden(rijin_user_id_hash, "RIJIN_USER_ID_HASH");
        wchar_t buf[1024];
        I(wsprintfW)(buf, WXOR(L"%s/%s/%X"), XOR(BUILD_VERSION_STR), convert::str2wstr(XOR(D3D_VERSION)).c_str(), rijin_user_id_hash_gen);
        font->draw(vec3i(padding, screen_size.y - padding), font_size, buf, rgba(255,255,255,100), TSTYLE_NONE, TALIGN_LEFT, TALIGN_TOP);
      #else
        wchar_t buf[1024];
        I(wsprintfW)(buf, WXOR(L"%s/%s"), XOR(BUILD_VERSION_STR), convert::str2wstr(XOR(D3D_VERSION)).c_str());
        font->draw(vec3i(padding, screen_size.y - padding), font_size, buf, rgba(255,255,255,100), TSTYLE_NONE, TALIGN_LEFT, TALIGN_TOP);
      #endif
    }

    // confidential build
    // TODO: DONT REMOVE ME, MAKE ME A TOGGLEABLE MACRO INSTEAD!
#if (defined(STAGING_MODE) || defined(DEV_MODE)) && !defined(CLIENT_LOADER)
    wchar_t buf[1024];
    vec3i pos = vec3i(padding, padding);
#if defined(TF2_CHEAT)
    pos.y += 64;
#endif

  #if !defined(CONFIDENTIAL_BUILD)
    font->draw(vec3i(padding, pos.y + padding), font_size, WXOR(L"DEVELOPMENT BUILD\nFOR TESTING PURPOSES ONLY"), rgba(230,126,34,180), TSTYLE_OUTLINE, TALIGN_CHEAP, TALIGN_CHEAP);
  #else
    font->draw(vec3i(padding, pos.y + padding), 24, WXOR(L"FOR TESTING PURPOSES ONLY\nDO NOT SHARE AND EXPECT ISSUES"), rgba(230, 126, 34, 255), TSTYLE_OUTLINE, TALIGN_CHEAP, TALIGN_CHEAP);
  #endif
#endif
  }
};