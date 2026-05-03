#include "../link.h"

#if defined(RENDER_3D)
CLASS_ALLOC(c_notification, notify);


void c_notification::setup_notify_header(){
  memset(&notify_str, 0, sizeof(notify_str));

  wsprintfW(notify_str[0], WXOR(L"INFO"));
  wsprintfW(notify_str[1], WXOR(L"WARNING"));
  wsprintfW(notify_str[2], WXOR(L"ALERT"));
  wsprintfW(notify_str[3], WXOR(L"ATTENTION"));
  wsprintfW(notify_str[4], WXOR(L"ERROR"));
  wsprintfW(notify_str[5], WXOR(L"SUCCESS"));

  notify_clr[0] = colour(198, 198, 198, 255);
  notify_clr[1] = colour(255, 255, 148, 255);
  notify_clr[2] = colour(255, 64, 64, 255);
  notify_clr[4] = notify_clr[2];
  notify_clr[5] = colour(64, 255, 64, 255);

  for(i32 i = 0; i < MAX_NOTIFICATIONS; i++)
    notifications[i].reset();
}

void c_notification::setup_fonts(){
  assert(render != nullptr);
  setup_notify_header();

  font_bold = font = render->create_font(WXOR(L"Segoe UI"), FW_MEDIUM, CLEARTYPE_QUALITY);
  assert(font != nullptr);
}

void c_notification::update_theme(colour outline, colour background){
  outline_col    = outline;
  background_col = background;
}

CFLAG_O0 s_notification_metadata* c_notification::create(u32 type, wchar_t* str, colour clr, u32 unused){
  if(clearing)
    return;

  static s_notification_metadata empty {};

  if(font == nullptr || str == nullptr){
    assert(false && "Somebody's forgot to load the fonts for the notifications. Or the text is a nullptr either way good bye!");
    return &empty;
  }

  u32 str_len = wcslen(str);
  if(!str_len || str_len >= 1024)
    return &empty;

  type = math::clamp(type, 0, NOTIFY_TYPE_MAX - 1);
  clr.w = 255;

  float time = math::time(false, true);
  u32 hash = WHASH_RT(str);

  bool should_add = true;
  for(u32 i = 0; i < MAX_NOTIFICATIONS; i++){
    if(notifications[i].time <= time)
      continue;

    if(hash == notifications[i].hash){
      notifications[i].time = (time + MAX_DISPLAY_TIME_SEC);
      notifications[i].clr  = clr;
      should_add = false;
      return notifications[i].metadata;
    }
  }

  if(!should_add){
    DBG("[-] c_notification::create(%i, %ls) already exists, not adding.\n", type, str);
    return &empty;
  }

  for(u32 i = 0; i < MAX_NOTIFICATIONS; i++){
    s_active_notifications* note = &notifications[i];
    if(note->valid())
      continue;

    bool fake_entry = (type == NOTIFY_TYPE_LOG);
    if(fake_entry)
      type = 0;

    note->reset();
    note->type = type;
    note->hash = hash;
    note->clr  = clr;
    note->time = fake_entry ? 0.f : (time + MAX_DISPLAY_TIME_SEC);

    i32 len = math::smallest((sizeof(note->str) / sizeof(wchar_t)) - 1, (i32)str_len);

    u32 j = 0;
    u32 w = 0;
    u32 c = 0;
    while(j < len){
      if(c >= NOTIFICATION_WRAP_AROUND_TEXT_SIZE){
        if(str[j] == L' '){
          note->str[w++] = L'\n';
          j++; // Skip the space.
          c = 0;
        }
        else if(str[j] == L'.' || str[j] == L'!' || str[j] == L'?'){
          note->str[w++] = L'\n';
          c = 0;
        }
      }

      note->str[w++] = str[j++];
      c++;

      if(j >= MAX_WPRINTF_BUFFER_SIZE)
        break;
    }

    note->str[len] = L'\0';

    note->metadata = new s_notification_metadata;
    history.push_back(notifications[i]);
    if(history.size() > 1024){
      s_notification_metadata* metadata = history.begin()->metadata;
      history.begin()->metadata = nullptr;
      if(metadata != nullptr)
        free(metadata);

      history.erase(history.begin());
    }

    DBG("[+] created notifications: is fake: %i\n", fake_entry);
    return note->metadata;
  }

  DBG("[+] uh failed to create notifications\n");
  return &empty;
}

CFLAG_O0 void c_notification::draw_notifications(){
  if(clearing)
    return;

  if(render == nullptr || font == nullptr || font_bold == nullptr)
    return;

  i32   drawn_notifications = 0;
  i32   other_notifications = 0;
  i32   y_pos               = NOTIFICATION_BASE_FONT_SIZE + render->rescale(2);
  float time                = math::time(false, true);

  const i32 padding          = render->rescale(4);
  const u32 header_font_size = NOTIFICATION_HEADER_FONT_SIZE;
  const u32 sub_font_size    = NOTIFICATION_SUB_TEXT_FONT_SIZE;

  colour r_outline_col = outline_col * 0.75f;

  // todo: copy whatever the transparent value the menu should be.
  float frac = 0.8f;
  r_outline_col.w   = 255 * frac;
  background_col.w  = 255 * frac;
  outline_col.w     = 255 * frac;


  for(u32 i = 0; i < MAX_NOTIFICATIONS; i++){
    s_active_notifications* note = &notifications[i];
    if(!note->valid())
      continue;

    // If notifications are to be kept from viewing, then lets make sure they can't expire.
    if(drawn_notifications >= MAX_DISPLAYED_NOTIFICATIONS){
      if(note->time > time)
        note->time = time + MAX_DISPLAY_TIME_SEC;

      other_notifications++;
      continue;
    }

    post_initialize(note); // Make sure it's properly positioned.
    note->animate();

    const float e_time = math::abs(note->time - time);

    // Just an FYI: You can probably just use c_font_base instead, and c_render_base as well for the render.
    // This is just a TODO. - Rud
    #if defined(RENDERING_MODE_D3D9)
    c_font_d3d9* d3d9      = (c_font_d3d9*)font;
    c_font_d3d9* d3d9_bold = (c_font_d3d9*)font_bold;

    const u32 original_type = notifications[i].type;
    u32 type = original_type;
    if(type == NOTIFY_TYPE_ALERT_FLASH)
      type = NOTIFY_TYPE_ALERT;

    wchar_t* notify_text   = notify_str[original_type];
    colour   notify_colour = notify_clr[type];

    if(original_type == NOTIFY_TYPE_ALERT_FLASH){
      if(notifications[i].colour_animation_cycle)
        notify_colour = notify_clr[NOTIFY_TYPE_WARNING];
      else
        notify_colour = notify_clr[NOTIFY_TYPE_ALERT];

      if(notifications[i].next_colour_animation_time <= time){     
        notifications[i].colour_animation_cycle = notifications[i].colour_animation_cycle == 1 ? 0 : 1;
        notifications[i].next_colour_animation_time = time + 0.25f;
      }
    }

    vec3i notification_pos = notifications[i].pos_offset;

    bool  s             = false;
    vec3i txt_size      = d3d9->get_size(header_font_size, notify_text, &s); 
    vec3i sub_text_size = notifications[i].str_size;

    const i32 width     =  math::biggest(txt_size.x, sub_text_size.x) + padding + render->rescale(2);

    const vec3i header_pos  = notification_pos + vec3i(padding, y_pos);
    const vec3i header_size = vec3i(width, txt_size.y + padding);

    const vec3i pos  = notification_pos + vec3i(padding, y_pos + header_size.y);
    const vec3i size = vec3i(width, sub_text_size.y + (padding * 2));

    // Bottom rect.
    render->filled_rect(pos, size, background_col);
      
    // Header rect.
    render->filled_rect(header_pos, header_size, outline_col);

    // Outlines
    render->outlined_rect(header_pos, size + vec3i(0, header_size.y), outline_col);
    
    const float progress = math::biggest(0.f, ((e_time - NOTIFICATION_LEAVE_ANIMATION) / (MAX_DISPLAY_TIME_SEC)));

    render->solid_line(vec3i(pos.x, pos.y + size.y + 1), vec3i(pos.x + (size.x * progress) + 1, pos.y + size.y + 1), r_outline_col, false, 2.f);

    // Draw header (centered version)
    //d3d9_bold->draw(vec3i(header_pos.x + (size.x / 2) - txt_size.x / 2, header_pos.y), header_font_size, notify_text, notify_colour, TSTYLE_SHADOW, TALIGN_CHEAP, TALIGN_CHEAP);

    d3d9_bold->draw(vec3i(header_pos.x + padding, header_pos.y + (header_size.y / 2) - txt_size.y / 2), header_font_size, notify_text, notify_colour, TSTYLE_SHADOW, TALIGN_CHEAP, TALIGN_CHEAP);

    // Draw sub text below.
    d3d9->draw(vec3i(pos.x + padding, (pos.y + size.y) - (sub_text_size.y + padding)), sub_font_size, notifications[i].str, notifications[i].clr, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_LEFT);


    y_pos += (header_size.y + size.y) + (padding + 2); // + 2 accounts for the 2 width line.
    drawn_notifications++;

    #else
    assert(false && "NOT SUPPORTED, ADD SUPPORT");
    #endif
  }


  if(other_notifications > 0){
    #if defined(RENDERING_MODE_D3D9)
      c_font_d3d9* d3d9 = (c_font_d3d9*)font;

      wchar_t buf[1024];
      wsprintfW(buf, WXOR(L"%i+ more notifications"), other_notifications);
      d3d9->draw(vec3i(6, y_pos), 14, buf, colour(228, 228, 228, 255), TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);
    #endif
  }
}

vec3i gcs_notify_component_history::size(){
  if(notify->history.empty() || notify->clearing)
    return info()->size;

  i32 height = 0;
  i32 padding = render->rescale(4);
  const u32 header_font_size = NOTIFICATION_HEADER_FONT_SIZE;
  for(u32 i = notify->history.size(); i-- > 0;){
    s_active_notifications* note = &notify->history[i];
    if(note->metadata == nullptr)
      continue;


    notify->post_initialize(note);
    c_font_d3d9* d3d9      = (c_font_d3d9*)notify->font;
    c_font_d3d9* d3d9_bold = (c_font_d3d9*)notify->font_bold;

    const u32 original_type = note->type;
    wchar_t* notify_text    = notify->notify_str[original_type];

    bool  s             = false;
    vec3i txt_size      = d3d9->get_size(header_font_size, notify_text, &s);
    vec3i sub_text_size = note->str_size;
    const vec3i header_size = vec3i(0, txt_size.y + padding);
    const vec3i size = vec3i(0, sub_text_size.y + (padding * 2));

    height += (header_size.y + size.y) + (padding + render->rescale(2)); // + 2 accounts for the 2 width line.
  }

  assert(notify != nullptr);
  return vec3i(info()->size.x, height);
}

void gcs_notify_component_history::draw(vec3i pos, vec3i size){
  assert(notify != nullptr);
  if(notify->history.empty() || notify->clearing)
    return;

  float        time                            = math::time(false, true);
  static void*        notice_link_pointer             = nullptr;
  static float notice_link_expire_time  = 0.f;

  const i32 padding          = render->rescale(4);
  i32   y_pos                = 0;
  const u32 header_font_size = NOTIFICATION_HEADER_FONT_SIZE;
  const u32 sub_font_size    = NOTIFICATION_SUB_TEXT_FONT_SIZE;

  colour r_outline_col = notify->outline_col * 0.75f;

  // todo: copy whatever the transparent value the menu should be.
  float frac = 0.8f;
  r_outline_col.w   = 255 * frac;
  notify->background_col.w  = 255 * frac;
  notify->outline_col.w     = 255 * frac;

  for(u32 i = notify->history.size(); i-- > 0;){
    s_active_notifications* note = &notify->history[i];
    if(note->metadata == nullptr)
      continue;

    notify->post_initialize(note);
    c_font_d3d9* d3d9      = (c_font_d3d9*)notify->font;
    c_font_d3d9* d3d9_bold = (c_font_d3d9*)notify->font_bold;

    const u32 original_type = note->type;
    u32 type = original_type;
    if(type == NOTIFY_TYPE_ALERT_FLASH)
      type = NOTIFY_TYPE_ALERT;

    wchar_t* notify_text   = notify->notify_str[original_type];
    colour   notify_colour = notify->notify_clr[type];

    if(original_type == NOTIFY_TYPE_ALERT_FLASH){
      //if(note->colour_animation_cycle)
      //  notify_colour = notify->notify_clr[NOTIFY_TYPE_WARNING];
      //else
      notify_colour = notify->notify_clr[NOTIFY_TYPE_ALERT];

      //if(note->next_colour_animation_time <= time){
      //  note->colour_animation_cycle = note->colour_animation_cycle == 1 ? 0 : 1;
      //  note->next_colour_animation_time = time + 0.25f;
      //}
    }

    vec3i notification_pos = pos;

    bool  s             = false;
    vec3i txt_size      = d3d9->get_size(header_font_size, notify_text, &s);
    vec3i sub_text_size = note->str_size;

    const i32 width     =  size.x - (padding + render->rescale(2)); //math::biggest(txt_size.x, sub_text_size.x) + padding + 2;

    const vec3i header_pos  = notification_pos + vec3i(padding, y_pos);
    const vec3i header_size = vec3i(width, txt_size.y + padding);

    const vec3i pos  = notification_pos + vec3i(padding, y_pos + header_size.y);
    const vec3i size = vec3i(width, sub_text_size.y + (padding * 2));

    // Bottom rect.
    render->filled_rect(pos, size, notify->background_col);

    // Header rect.
    render->filled_rect(header_pos, header_size, notify->outline_col);

    // Outlines
    render->outlined_rect(header_pos, size + vec3i(0, header_size.y), notify->outline_col);

    static bool last_click = false;
    static bool last_click2 = false;
    if(is_in_rect(header_pos, header_size + size)){
      const bool clicked  = input() & GCS_IN_M1_CLICK;
      const bool clicked2 = input() & GCS_IN_M2_CLICK || input_system->held('C') && input_system->held(VK_CONTROL);

      render->outlined_rect(header_pos, size + vec3i(0, header_size.y), rgba(255, 255, 255, 128));
      render->filled_rect(header_pos, size + vec3i(0, header_size.y), rgba(255, 255, 255, 32));
      if(clicked){
        if(!last_click){
          last_click = true;

          note->metadata->on_click();
        }
      }
      else if(clicked2){
        if(!last_click2){
          last_click2 = true;

          if(I(OpenClipboard)(nullptr)){
            if(I(EmptyClipboard)()){
              wchar_t cb_text[1024];
              I(wsprintfW)(cb_text, WXOR(L"RijiN: %ls"), note->str);
              const u32 len = wcslen(cb_text);

              i8* handle = (i8*)I(GlobalAlloc)(GMEM_MOVEABLE, (len * 2) + 1);
              if(handle != nullptr){
                memcpy(I(GlobalLock)(handle), cb_text, len * 2);
                I(GlobalUnlock)(handle);
                I(SetClipboardData)(CF_UNICODETEXT, handle);
                I(CloseClipboard)();
                I(GlobalFree)(handle);

                notice_link_pointer     = note;
                notice_link_expire_time = time + 1.f;
              }
            }
          }
        }
      }
      else{
        last_click2 = false;
        last_click  = false;
      }
    }
    else{
      if(note == notice_link_pointer && math::abs(notice_link_expire_time - time) < 0.2f)
        notice_link_pointer = nullptr;
    }

    static wchar_t notify_text_dyn[1024];
    I(wsprintfW)(notify_text_dyn, WXOR(L"%ls%ls"), notify_text, note->metadata->get_meta_data_str().c_str());

    d3d9_bold->draw(vec3i(header_pos.x + padding, header_pos.y + (header_size.y / 2) - txt_size.y / 2), header_font_size, notify_text_dyn, notify_colour, TSTYLE_SHADOW, TALIGN_CHEAP, TALIGN_CHEAP);

    // Draw sub text below.
    render->push_clip_region(header_pos, size + vec3i(-padding, header_size.y));
    d3d9->draw(vec3i(pos.x + padding, (pos.y + size.y) - (sub_text_size.y + padding)), sub_font_size, note->str, note->clr, TSTYLE_SHADOW, TALIGN_LEFT, TALIGN_LEFT);
    render->pop_clip_region();


    if(note == notice_link_pointer && notice_link_expire_time > time){
      render->outlined_rect(header_pos, size + vec3i(0, header_size.y), rgba(255, 255, 255, 128));
      render->filled_rect(header_pos, size + vec3i(0, header_size.y), rgba(255, 255, 255, 128));
      d3d9->draw(vec3i(pos.x + size.x / 2, (pos.y + size.y) - (sub_text_size.y + padding)), sub_font_size, WXOR(L"Copied to clipboard"), note->clr, TSTYLE_SHADOW, TALIGN_CENTER, TALIGN_CENTER);
    }

    y_pos += (header_size.y + size.y) + (padding + render->rescale(2)); // + 2 accounts for the 2 width line.
  }
}

s_notification_metadata* s_notification_metadata::on_click(){
  DBG("[!] s_notification_metadata::on_click %i %i\n", type, data);

  #if defined(SOURCE_CHEAT)
    assert(global_internal != nullptr);
    assert(global_internal->steamcontext != nullptr);
    assert(global_internal->steamcontext->steamfriends != nullptr);

    switch(type){
      default: break;
      case NOTIFY_METADATA_STEAM:
      {
        i8 buf[1024];
        I(wsprintfA)(buf, XOR("https://steamcommunity.com/profiles/[U:1:%i]"), (u32)data);
        global_internal->steamcontext->steamfriends->game_overlay_to_web_page(buf);
        break;
      }
    }
  #else


  #endif

  return this;
}

#endif