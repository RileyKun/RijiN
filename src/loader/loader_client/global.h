#pragma once

class c_global{
public:
  bool                                                                  running           = false;
  bool                                                                  block_render      = false;
  up_data                                                               up;
  c_net_receive_license*                                                license_data      = nullptr;
  void*                                                                 window            = nullptr;
  gcs_scene*                                                            menu              = nullptr;
  bool                                                                  draw_loading_page = false;
  std::vector<std::pair<c_net_receive_image_resource*, ID2D1Bitmap**>>  bitmap_creation_queue;

  void handle_creds(bool* should_save_logins, bool* attempt_auto_login);
  void prompt_fail(uptr result);

  ALWAYSINLINE inline void hide_loading_page(){
    draw_loading_page = false;
    loading_page_str.clear();
  }

  ALWAYSINLINE inline void show_loading_page(std::wstring str = L""){
    draw_loading_page = true;
    for(i32 i = 0; i < str.length(); i++)
      str[i] ^= (*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) % XOR32(1024)) + HASH("BASE_KEY");

    loading_page_str  = str;
  }

  ALWAYSINLINE inline bool is_loading_str_empty(){
    return loading_page_str.empty();
  }

  ALWAYSINLINE inline std::wstring get_loading_page_str(){
    std::wstring str = loading_page_str;
    for(i32 i = 0; i < str.length(); i++)
      str[i] ^= (*(volatile u32*)(XOR32_IMPORTANT_ONLY(0x7FFE0330)) % XOR32(1024)) + HASH("BASE_KEY");

    return str;
  }

private:
    std::wstring loading_page_str;
};

CLASS_EXTERN(c_global, global);