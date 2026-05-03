#pragma once

struct s_input_button{
  bool active  = false;
  bool held    = false;
  bool pressed = false;
};

class c_input_system{
public:
  s_input_button buttons[0xFF];

  void reset(){
    memset(&buttons, 0, sizeof(buttons));

    DBG("[!] c_input_system::reset\n");
  }

  void update(){
    for(u32 i = 1; i < sizeof(buttons) / sizeof(s_input_button); i++){
      s_input_button* button = &buttons[i];

      if(!button->active)
        continue;

      bool old_held   = button->held;
      button->held    = (I(GetAsyncKeyState)(i) & 0x8000) != 0;
      button->pressed = !old_held && button->held;
    }
  }

  ALWAYSINLINE s_input_button* get_button_info(u8 b){
    if(b == 0 || b >= 0xFF)
      return nullptr;

    s_input_button* button = &buttons[b];

    if(!button->active){
      button->active = true;
      update();

      //DBG("[+] input_system: added button 0x%X list\n", b);
    }

    return button;
  }

  ALWAYSINLINE bool held(u8 b){
    if(b == 0)
      return false;

    s_input_button* button = get_button_info(b);

    if(button == nullptr)
      return false;

    return button->held;
  }

  ALWAYSINLINE bool pressed(u8 b){
    if(b == 0)
      return false;

    s_input_button* button = get_button_info(b);

    if(button == nullptr)
      return false;

    return button->pressed;
  }
};

CLASS_EXTERN(c_input_system, input_system);