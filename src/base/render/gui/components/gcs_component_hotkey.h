#pragma once

class gcs_component_hotkey : public gcs_component_base{
public:
  u8*   hotkey_ptr       = nullptr;
  bool  setting_hotkey   = false;
  bool  just_set_hotkey  = false;
  float next_clickable   = 0.f;

  bool is_module() override{
    return true;
  }

  void setup() override{
    info()->module_size       = render->rescale(40);
    info()->always_run_think  = true;
    hotkey_ptr          = nullptr;
  }

  gcs_component_base* set_hotkey_ptr(u8* new_hotkey_ptr){
    hotkey_ptr = new_hotkey_ptr;
    return this;
  }

  std::wstring get_hotkey_name(){
    if(hotkey_ptr == nullptr)
      return WXOR(L"[NULL]");

    if(setting_hotkey)
      return WXOR(L"[...]");

    switch(*hotkey_ptr){
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
      default :
        break;
    }

    u32 ret = MapVirtualKeyW(*hotkey_ptr, MAPVK_VK_TO_VSC);

    if(ret != 0){
        wchar_t key_name[32];
        wchar_t rich_name[32];

      if(GetKeyNameTextW(ret << 16, key_name, sizeof(key_name)) != 0){
        wsprintfW(rich_name, WXOR(L"[%ls]"), key_name);
        return (std::wstring)rich_name;
      }
    }

    static wchar_t unk_key_code[1024];
    memset(&unk_key_code, 0, sizeof(unk_key_code));
    I(wsprintfW)(unk_key_code, WXOR(L"%i-UNK"), *hotkey_ptr);

    return unk_key_code;
  }

  void think(vec3i pos, vec3i size) override{
    if(hotkey_ptr == nullptr)
      return;

    if(just_set_hotkey){
      scene()->info()->input_frozen = false;
      just_set_hotkey               = false;
      return;
    }

    if( !setting_hotkey && !just_set_hotkey &&
        ((is_in_rect() && (input_system->pressed(VK_LBUTTON) || input_system->pressed(VK_RBUTTON))) ||
        (parent()->is_in_rect() && input_system->pressed(VK_RBUTTON)))
      ){
      setting_hotkey  = true;
      return;
    }

    if(!setting_hotkey)
      return;

    scene()->info()->input_frozen = true;

    bool set_key = false;
    for(u8 i = 1; i <= VK_F12; i++){
      bool is_pressed = input_system->pressed(i);

      if(!is_pressed)
        continue;

      if(i != VK_ESCAPE && i != VK_INSERT)
        *hotkey_ptr = i;

      DBG("[!] hotkey set to code: %i\n", i);
      set_key = true;
      break;
    }

    if(!set_key)
      return;

    // don't be updating tons of buttons constantly
    input_system->reset();

    // HACK: Trick the input system into thinking these buttons are held
    // When the next update comes the input system these can never pass the "pressed" state and will be invalidated
    input_system->get_button_info(VK_LBUTTON)->held = true;
    input_system->get_button_info(VK_RBUTTON)->held = true;

    setting_hotkey  = false;
    just_set_hotkey = true;
  }

  void draw(vec3i pos, vec3i size){
    if(hotkey_ptr == nullptr)
      return;

    font()->draw(pos + vec3i(size.x, size.y / 2), 14, get_hotkey_name(), scheme()->text, TSTYLE_SHADOW, TALIGN_RIGHT, TALIGN_CENTER );
  }
};