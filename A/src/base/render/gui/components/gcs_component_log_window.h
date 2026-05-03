#pragma once

#define MAX_LOG_WINDOW_ENTRIES 8912

enum e_log_window_meta_data_type{
  LOG_WINDOW_STEAM_URL = 1,
  LOG_WINDOW_SERVER_IP = 2,
};

class s_log_window_metadata{
public:
  s_log_window_metadata(){
    data = 0;
    type = 0;
  }

  uptr  data;
  i8    buf[1024];
  u32   type;

  wchar_t* get_tooltip_text(){
    switch(type){
      default: break;
      case 0: return WXOR(L"Click to view profile.");
      case 1: return WXOR(L"Click to connect to server.");
    }

    return nullptr;
  }

  s_log_window_metadata* set_text_buffer(i8* buffer){
    memset(buf, 0, sizeof(buf));
    strcpy(buf, buffer);
    return this;
  }

  s_log_window_metadata* set_type(u32 t){
    type = t;
    return this;
  }

  s_log_window_metadata* set_value(float flt){
    data = (uptr)flt;
    return this;
  }

  s_log_window_metadata* set_value(u32 val){
    data = (uptr)val;
    return this;
  }

  s_log_window_metadata* set_value(i32 val){
    data = (uptr)val;
    return this;
  }

  s_log_window_metadata* on_click();
};

struct s_log_window_entry{
  wchar_t buf[1024];
  s_log_window_metadata* metadata;

  bool  cached_size;
  vec3i size;
};


class c_gcs_component_log_window : public gcs_component_base{
public:
  u32 font_size = 16;
  std::vector<s_log_window_entry> history;
  bool                            clearing;
  u32                             last_size_calculated;

  NEVERINLINE void clear_history(){
    if(history.empty())
      return;

    DBG("[!] c_gcs_component_log_window::clear_history\n");
    clearing = true;
    for(i32 i = 0; i < history.size(); i++){
      s_log_window_metadata* metadata = history[i].metadata;
      if(metadata == nullptr)
        continue;

      history[i].metadata = nullptr;
      free(metadata);
    }
    history.clear();
    clearing = false;
  }

  NEVERINLINE add_if_empty(wchar_t* text){
    static s_log_window_metadata empty{};
    if(!history.empty())
      return &empty;

    return add(text);
  }

  NEVERINLINE s_log_window_metadata* add(wchar_t* text){
    static s_log_window_metadata empty{};

    if(text == nullptr)
      return &empty;

    if(wcslen(text) >= 1024)
      return &empty;

    s_log_window_entry entry;
    memset(&entry, 0, sizeof(s_log_window_entry));

    I(wsprintfW)(entry.buf, WXOR(L"%ls"), text);
    entry.metadata = malloc(sizeof(s_log_window_metadata));
    history.push_back(entry);
    if(history.size() > MAX_LOG_WINDOW_ENTRIES){
      s_log_window_metadata* metadata = history.begin()->metadata;
      if(metadata != nullptr)
        free(metadata);

      history.erase(history.begin());
    }
    // TODO: need to clear after X amount of entries for performance.

    DBG("[!] c_gcs_component_log_window::add: %ls - %i\n", text, history.size());
    return entry.metadata;
  }

  NEVERINLINE void add(i8* text){
    static s_log_window_metadata empty{};
    if(text == nullptr)
      return &empty;

    wchar_t buf[1024];
    buf[1023] = L'\0';
    convert::str2wstr(text, buf, sizeof(buf));
    return add(buf);
  }

  void setup() override{
    set_size(vec3i(info()->size.x, font_size));
  }

  vec3i size() override{
    if(history.empty() || clearing)
      return info()->size;

    u32 height  = 0;
    i32 padding = render->rescale(4);

    // There is no reason to recalculate this every frame, if no member changes.
    if(history.size() == last_size_calculated)
      return info()->size;

    DBG("[!] c_gcs_component_log_window::size recalculating..\n");
    for(u32 i = history.size(); i-- > 0;){
      s_log_window_entry* entry = &history[i];
      bool  s    = false;
      vec3i size = entry->size;
      if(!entry->cached_size){
        entry->size = font()->get_size(font_size, entry->buf, &s) + vec3i(0, padding);
        size = entry->size;
        entry->cached_size = true;
      }

      height += (size.y) + (padding);
    }

    last_size_calculated = history.size();
    return vec3i(info()->size.x, height);
  }

  void draw(vec3i pos, vec3i size) override{
    if(history.empty() || clearing)
      return;

    i32   height_offset = math::abs(parent()->info()->auto_positioning_override_height);
    vec3i parent_size   = parent()->info()->size;

    float        time                     = math::time(false, true);
    static void* notice_link_pointer      = nullptr;
    static float notice_link_expire_time  = 0.f;

    i32 y_pos   = 0;
    i32 padding = render->rescale(4);

    static bool copy_all = false;
    if(input_system->held(VK_CONTROL) && input_system->held(VK_LSHIFT) && input_system->held('C')){
      if(!copy_all){
        copy_all = true;
        std::wstring info = WXOR(L"https://rijin.solutions : LOGS\n");

        for(u32 i = history.size(); i-- > 0;){
          s_log_window_entry* entry = &history[i];
          wchar_t temp[1024];
          I(wsprintfW)(temp, WXOR(L"%ls\n"), entry->buf);
          info += temp;
        }

        clipboard::set_text(info);
      }
    }
    else
      copy_all = false;

    for(u32 i = history.size(); i-- > 0;){
      s_log_window_entry* entry = &history[i];

      vec3i draw_pos     = pos + vec3i(0, y_pos);
      vec3i draw_size    = entry->size;
      i32   actual_y_pos = (y_pos - height_offset);

      // Optimization trick, only try and render the object where it would actually be visible in the window.
      // Since we got the scroll bar, yes it'll be clipped but we still process all of it which consumes CPU time.
      // the number: - 32 is a tolerance number to avoid popping in at the top.
      const bool should_render = (y_pos >= (height_offset - render->rescale(32))) && (actual_y_pos <= parent_size.y);
      if(!entry->cached_size){
         bool s = false;
        entry->size = font()->get_size(font_size, entry->buf, &s) + vec3i(0, padding);
        size = entry->size;
        entry->cached_size = true;
      }

      draw_size.x = (size.x);

      if(should_render){
        render->outlined_rect(draw_pos, draw_size, scheme()->grey3);

        static bool last_click1 = false;
        static bool last_click2 = false;
        if(is_in_rect(draw_pos, draw_size)){
          render->outlined_rect(draw_pos, draw_size, entry->metadata->type != 0 ? scheme()->error : scheme()->main);
          render->filled_rect(draw_pos, draw_size, scheme()->text * 0.45f);

          const bool clicked2 = input() & GCS_IN_M2_CLICK || input_system->held('C') && input_system->held(VK_CONTROL);
          const bool clicked1 = input() & GCS_IN_M1_CLICK;
          if(clicked1){
            if(!last_click1){
              if(entry->metadata != nullptr)
                entry->metadata->on_click();

              last_click1 = true;
            }
          }
          else if(clicked2){
            if(!last_click2){
              last_click2 = true;

              wchar_t cb_text[1024];
              I(wsprintfW)(cb_text, WXOR(L"%ls"), entry->buf);
              if(clipboard::set_text(cb_text)){
                notice_link_pointer     = entry;
                notice_link_expire_time = time + 1.f;
              }
            }
          }
          else{
            last_click1 = false;
            last_click2 = false;
          }
        }

        render->push_clip_region(draw_pos, draw_size);
        font()->draw(draw_pos + vec3i(padding / 2, padding / 2), font_size, entry->buf, scheme()->text, TSTYLE_SHADOW, TALIGN_CHEAP, TALIGN_CHEAP);
        render->pop_clip_region();

        if(entry == notice_link_pointer && notice_link_expire_time > time){
          render->outlined_rect(draw_pos, draw_size, rgba(255, 255, 255, 164));
          render->filled_rect(draw_pos, draw_size, rgba(192, 192, 192, 164));
          font()->draw(vec3i(draw_pos.x + draw_size.x / 2, draw_pos.y), font_size, WXOR(L"Copied to clipboard"), scheme()->text, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CHEAP);
        }
      }

      y_pos += (draw_size.y) + (padding);
    }
  }
};