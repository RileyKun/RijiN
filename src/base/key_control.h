#pragma once

class c_key_setting{
public:
  c_key_setting(bool b, u8 v){
    enabled = b;
    key     = v;
  }

  bool enabled            = false;
  bool use_key            = false;
  u8   key                = VK_LSHIFT;
  bool held_state         = true;
  bool toggle_state       = false;
  bool double_click_state = false;
  bool is_valid           = true;

  void fix(){
    if(is_valid)
      return;

    key = VK_LSHIFT;

    use_key            = false;
    enabled            = false;
    held_state         = true;
    toggle_state       = false;
    double_click_state = false;
    is_valid           = true;
  }
};

class c_key_control{
public:
  c_key_control(){
    memset(this, 0, sizeof(c_key_control));
    _valid = false;
  }

  ALWAYSINLINE c_key_control(c_key_setting* setting){
    assert(setting != nullptr);
    memset(this, 0, sizeof(c_key_control));
    setting->fix();

    _key                = &setting->key;
    _held_state         = &setting->held_state;
    _toggle_state       = &setting->toggle_state;
    _double_click_state = &setting->double_click_state;
    _valid              = true;
    state               = false;

  }

  ALWAYSINLINE c_key_control(u8* key, bool* held_state = nullptr, bool* toggle_state = nullptr, bool* double_click_state = nullptr){
    memset(this, 0, sizeof(c_key_control));
    _key                = key;
    _held_state         = held_state;
    _toggle_state       = toggle_state;
    _double_click_state = double_click_state;
    _valid              = true;
    state               = false;
  }

  bool is_toggled(){
    if(!_valid)
      return false;

    bool held = is_key_held();
    if(_held_state == nullptr || _held_state != nullptr && *_held_state == true){
      state = held;
    }
    else if(_toggle_state != nullptr && *_toggle_state == true){
      if(held && !last_key_held_state)
        state = !state;
    }
    else if(_double_click_state != nullptr && *_double_click_state == true){
      // Only increase the amount of clicks if we aren't holding our key down constantly.
      if(held && !last_key_held_state){
        _double_clicks++;
        _double_click_reset_time = math::time() + 0.25f;
      }

      // We have double clicked the key.
      if(_double_clicks >= 2){
        _double_clicks = 0;
        state = !state;
      }
    }
    else{
      // Assume there is an issue with our config.
      if(_held_state != nullptr)
        *_held_state         = true;

      if(_toggle_state != nullptr)
        *_toggle_state       = false;

      if(_double_click_state != nullptr)
        *_double_click_state = false;
    }

    // Expire our double clicks. After a certain amount of time and under a certain condition.
    if(last_key_held_state == held && _double_clicks > 0 && _double_click_reset_time <= math::time())
      _double_clicks = 0;

    last_key_held_state = held;
    return state;
  }

  ALWAYSINLINE bool is_key_held(){
    return _valid && input_system->held(*_key);
  }

  ALWAYSINLINE bool is_valid(){
    return _valid;
  }

  NEVERINLINE std::wstring get_toggle_state_str(){
    if(_held_state != nullptr){
      if(*_held_state)
        return WXOR(L"Held");
    }

    if(_toggle_state != nullptr){
      if(*_toggle_state)
        return WXOR(L"Toggle");
    }

    if(_double_click_state != nullptr){
      if(*_double_click_state)
        return WXOR(L"Double-click");
    }

    return WXOR(L"Unk");
  }

  NEVERINLINE u8 get_key(){
    if(_key == nullptr)
      return 0;

    return *_key;
  }

  NEVERINLINE std::wstring get_key_str(){
    if(_key == nullptr)
      return WXOR(L"Unk");

    switch(*_key){
      default: break;
      case VK_INSERT:
        return WXOR(L"[INS]");
      case VK_SHIFT:
      case VK_LSHIFT:
        return WXOR(L"[SHIFT]");
      case VK_CAPITAL:
        return WXOR(L"[CAPS]");
      case VK_RSHIFT:
        return WXOR(L"[RSHIFT]");
      case VK_LWIN:
      case VK_RWIN:
        return WXOR(L"[WIN]");
      case VK_LBUTTON:
        return WXOR(L"[M1]");
      case VK_MBUTTON:
        return WXOR(L"[M3]");
      case VK_RBUTTON:
        return WXOR(L"[M2]");
      case VK_XBUTTON1:
        return WXOR(L"[M4]");
      case VK_XBUTTON2:
        return WXOR(L"[M5]");
    }

    u32 ret = MapVirtualKeyW(*_key, MAPVK_VK_TO_VSC);
    if(ret != 0){
      wchar_t buf[128];
      if(GetKeyNameTextW(ret << 16, buf, sizeof(buf)) != 0)
        return (std::wstring)buf;
    }

    return WXOR(L"Unk");
  }

private:
  u8* _key;
  bool _valid;

  // Used for chaging modes.
  bool* _held_state;
  bool* _toggle_state;
  bool* _double_click_state;
  i32   _double_clicks;
  float _double_click_reset_time;

  // Current states.
  bool last_key_held_state;
  bool state;
};
