#include "../../../link.h"


s_log_window_metadata* s_log_window_metadata::on_click(){

  switch(type){
    default: break;
    case LOG_WINDOW_STEAM_URL:
    {
      #if defined(SOURCE_CHEAT)
        assert(global_internal != nullptr);
        assert(global_internal->steamcontext != nullptr);
        assert(global_internal->steamcontext->steamfriends != nullptr);
        i8 buffer[1024];
        I(wsprintfA)(buffer, XOR("https://steamcommunity.com/profiles/[U:1:%i]"), (u32)data);
        global_internal->steamcontext->steamfriends->game_overlay_to_web_page(buffer);
      #endif
      break;
    }
    case LOG_WINDOW_SERVER_IP:
    {
      #if defined(SOURCE_CHEAT)
        if(buf[0] == '\0')
          return this;

        assert(global_internal != nullptr);
        assert(global_internal->engine != nullptr);

        i8 buffer[1024];
        I(wsprintfA)(buffer, XOR("connect %s"), buf);
        global_internal->engine->client_cmd(buffer);

      #endif
      break;
    }
  }
  return this;
}