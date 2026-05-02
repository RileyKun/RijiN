#pragma once

#if !defined(LOADER_CLIENT)
#define INFO_PANEL_FONT_SIZE 16
struct s_info_panel_table{
  bool          valid;
  u32           id;
  wchar_t       name[1024];
  bool*         enabled_ptr;
  bool          enabled;
  c_key_control* key;

  bool is_active(){
    if(key != nullptr){
      if(key->is_valid())
        return key->is_toggled();
    }

    return enabled;
  }
};

class gcs_base_info_panel : public gcs_scene{
public:
  gcs_component_window* window         = nullptr;
  vec3i                 window_size    = vec3i(224, 0);
  vec3i*                window_pos_ptr = nullptr;
  bool*                 active_ptr     = nullptr;

  s_info_panel_table    table[128];

  virtual void pre_setup(const i32 id, wchar_t* name){
    assert(id < 128);
    s_info_panel_table* entry = &table[id];
    if(entry == nullptr)
      return;

    I(wsprintfW)(entry->name, WXOR(L"%ls"), name);
  }


  virtual void assign(i32 id, c_key_control* key, bool* enabled_ptr){
    assert(id < 128);
    s_info_panel_table* entry = &table[id];
    if(entry == nullptr)
      return;

    if(entry->valid)
      return;

    entry->enabled_ptr = enabled_ptr;
    entry->enabled     = true;
    entry->key         = key;
    entry->valid       = true;
  }

  virtual void update(const i32 id, bool enabled){
    assert(id < 128);
    s_info_panel_table* entry = &table[id];
    if(entry == nullptr)
      return;

    if(!entry->valid)
      return;

    entry->enabled = enabled;
  }

  virtual bool is_menu_open(){
    //DBG("[!] gcs_base_info_panel::is_menu_open needs overrided.\n");
    return true;
  }

  virtual bool is_in_game(){
    //DBG("[!] gcs_base_info_panel::is_in_game needs overrided.\n");
    return true;
  }

  void init(vec3i* pos_ptr, bool* _active_ptr){
    static bool is_setup = false;
    if(window == nullptr)
      return;

    if(is_setup)
      return;

    is_setup = true;

    window_pos_ptr = pos_ptr;
    active_ptr     = _active_ptr;
    //reset_data();

    if(*window_pos_ptr == vec3i())
      *window_pos_ptr = vec3i(4, render->screen_size.y / 2);

    window->set_pos_ptr(window_pos_ptr);
    DBG("[+] gcs_base_info_panel set pos ptr\n");
  }

  NEVERINLINE bool is_enabled(){
    if(active_ptr == nullptr){
      DBG("[!] active_ptr is nullptr\n");
      return false;
    }

    if(*active_ptr)
      return true;

    return false;
  }

  bool setup() override;
};
#endif