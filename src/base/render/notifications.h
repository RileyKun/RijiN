#pragma once
#define CREATE_NOTIFICATION_SYSTEM() if(notify == nullptr) notify = new c_notification; notify->render = render;

#define MAX_NOTIFICATIONS 64
#define MAX_DISPLAY_TIME_SEC 5.f
#define NOTIFICATION_WRAP_AROUND_TEXT_SIZE 60
#define NOTIFICATION_LEAVE_ANIMATION 0.5f

#define MAX_DISPLAYED_NOTIFICATIONS 4
#define NOTIFICATION_BASE_FONT_SIZE 30
#define NOTIFICATION_HEADER_FONT_SIZE (NOTIFICATION_BASE_FONT_SIZE - 10)
#define NOTIFICATION_SUB_TEXT_FONT_SIZE (NOTIFICATION_BASE_FONT_SIZE - 14)


enum NOTIFY_TYPE{
  NOTIFY_TYPE_INFO = 0,
  NOTIFY_TYPE_WARNING = 1,
  NOTIFY_TYPE_ALERT,
  NOTIFY_TYPE_ALERT_FLASH,
  NOTIFY_TYPE_ERROR,
  NOTIFY_TYPE_SUCCESS,
  NOTIFY_TYPE_TROLLERS,
  NOTIFY_TYPE_LOG,
  NOTIFY_TYPE_MAX,
};

enum notify_metadata_type{
  NOTIFY_METADATA_INVALID = 0,
  NOTIFY_METADATA_STEAM = 1,
};

class s_notification_metadata{
public:
  s_notification_metadata(){
    type = 0;
    data = 0;
  }
  u32  type;
  uptr data;

  NEVERINLINE s_notification_metadata* invalidate(){
    DBG("[!] s_notification_metadata::invalidate\n");
    type = NOTIFY_METADATA_INVALID;
    return this;
  }

  NEVERINLINE s_notification_metadata* set_data(u32 t, i32 i){
    DBG("[!] s_notification_metadata::set_data\n");
    type = t;
    data = (uptr)i;
    return this;
  }

  NEVERINLINE s_notification_metadata* set_data(u32 t, u32 ui){
    DBG("[!] s_notification_metadata::set_data\n");
    type = t;
    data = (uptr)ui;
    return this;
  }

  NEVERINLINE s_notification_metadata* set_data(u32 t, float fl){
    DBG("[!] s_notification_metadata::set_data\n");
    type = t;
    data = (uptr)fl;
    return this;
  }

#if defined(__x86_64__)
  NEVERINLINE s_notification_metadata* set_data(u32 t, uptr up){
    DBG("[!] s_notification_metadata::set_data\n");
    type = t;
    data = (uptr)up;
    return this;
  }
#endif

  NEVERINLINE std::wstring get_meta_data_str(){
    switch(type){
      default: return L"";
      case NOTIFY_METADATA_STEAM:
        return WXOR(L" - CLICK TO VIEW PROFILE");
    }

    return L"";
  }

  virtual s_notification_metadata* on_click();
};

struct s_active_notifications{
  ALWAYSINLINE void reset(){
    memset(this, 0, sizeof(s_active_notifications));
  }

  vec3i                    pos_offset;
  float                    time;
  u32                      type;
  wchar_t                  str[1024];
  vec3i                    str_size;
  u32                      hash;
  colour                   clr;
  float                    next_colour_animation_time;
  i32                      colour_animation_cycle;
  float                    next_position_animation_time;
  bool                     is_flipped_position_allowed;
  float                    is_not_first_time;
  s_notification_metadata* metadata;

  NEVERINLINE void animate(){
    float cur_time = math::time();
    if(next_position_animation_time > cur_time)
      return;

    float e_time = math::abs(time - cur_time);

    const float frame_rate     = (1.0f / 120.f);
    const float total_duration = 0.1f;
    

    next_position_animation_time = cur_time + frame_rate;
    if(e_time <= (total_duration * 5.f)){
      if(!is_flipped_position_allowed)
        return;

      const float speed = math::biggest(1.f, math::abs(((str_size.x) / total_duration) * frame_rate)); // Make the speed fast enough so we slide out with in 500ms!
      pos_offset.x -= (i32)speed;
    }
    else{

      const float speed = math::biggest(1.f, math::abs((pos_offset.x / total_duration) * frame_rate)); // Make the speed fast enough so we slide out with in 500ms!
      pos_offset.x += (i32)speed;
      pos_offset.x = math::smallest(pos_offset.x, 0);
      is_flipped_position_allowed = pos_offset.x == 0;
    }
  }

  NEVERINLINE bool valid(){
    const float cur_time = math::time(false, true);
    if(time > cur_time)
      return true;

    return false;
  }
};

class c_notification{
public:
  c_render_base*         render      = nullptr;
  c_font_base*           font        = nullptr;
  c_font_base*           font_bold   = nullptr;
  colour outline_col                 = colour(25, 118, 210, 242);
  colour background_col              = colour(41, 44, 51, 242);
  s_active_notifications notifications[MAX_NOTIFICATIONS];

  wchar_t notify_str[NOTIFY_TYPE_MAX][1024];
  colour  notify_clr[NOTIFY_TYPE_MAX];

  std::vector<s_active_notifications> history;
  bool                                clearing;

  NEVERINLINE void clear_history(){
    if(history.empty())
      return;

    DBG("[!] c_notification::clear_history\n");
    clearing = true;
    for(i32 i = 0; i < history.size(); i++){
      s_notification_metadata* metadata = history[i].metadata;
      if(metadata == nullptr)
        continue;

      history[i].metadata = nullptr;
      free(metadata);
    }
    history.clear();
    clearing = false;
  }

  bool is_ready(){
    return render != nullptr && font != nullptr && font_bold != nullptr;
  }

  void setup_notify_header();
  void setup_fonts();
  void update_theme(colour outline, colour background);

  s_notification_metadata* create(u32 type, wchar_t* str, colour clr = colour(255, 255, 255, 255), u32 ununsed = 0);

  // D3D9 is not thread safe unless the device is setup with multi-threading in mind. Either way lets just do it this way to avoid any crashing.
  void post_initialize(s_active_notifications* note){ // Only call inside d3d9_hook or in a function that is called with in that hook.
    if(note == nullptr)
      return;

    if(note->is_not_first_time == render->scale())
      return;

    note->is_not_first_time = render->scale();
    bool whatever  = false;

    note->str_size = font->get_size(NOTIFICATION_SUB_TEXT_FONT_SIZE, note->str, &whatever);
    note->pos_offset.x -= note->str_size.x * 2;
  }

  void draw_notifications();
};

class gcs_notify_component_history : public gcs_component_base{
public:
  void setup() override{
    set_size(vec3i(info()->size.x, NOTIFICATION_BASE_FONT_SIZE));
  }

  vec3i size() override;
  void draw(vec3i pos, vec3i size) override;
};

CLASS_EXTERN(c_notification, notify);