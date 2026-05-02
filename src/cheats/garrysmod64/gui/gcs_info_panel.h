#pragma once

#include "settings.h"

#define INFO_EXPIRE_TIME 1.0f

#define INFO_PANEL_WARNING_CLR   colour(255, 255, 64)
#define INFO_PANEL_LEGIT_CLR     colour(32, 255, 64)
#define INFO_PANEL_RAGE_CLR      colour(255, 32, 32)
#define INFO_PANEL_WARN_RAGE_CLR colour(255, 96, 32)


enum{
  INFO_PANEL_ANTISCREENGRAB,
  INFO_PANEL_AIMBOT,
  INFO_PANEL_ANTIAIM,
  INFO_PANEL_ANTIAIM_FAKEDUCK,
  INFO_PANEL_ANTIAIM_BREAK_ANIMATIONS,
  INFO_PANEL_FAKELAG,
  INFO_PANEL_NOSPREAD,
  INFO_PANEL_NORECOIL,
  INFO_PANEL_FREECAM,
  INFO_PANEL_THIRDPERSON,
  INFO_PANEL_BUNNYHOP,
  INFO_PANEL_AUTOSTRAFE,
  INFO_PANEL_VOICERECORD_PROTECT,
  INFO_PANEL_DEMO_PROTECT,

  INFO_PANEL_PEEK_FAKELAG,
  INFO_PANEL_ANTIBACKTRACK_FAKELAG,
};

class gcs_info_panel : public gcs_scene{
public:
  vec3i window_size = vec3i(224, 10);

  bool setup() override {
    vec3i menu_pos = vec3i(4, render->screen_size.y / 2);

    gcs_component_window* window = gcs::window(this, WXOR(L"Information"), menu_pos, window_size);
    {
      window->info()->disable_auto_positioning();

      assert(window != nullptr);
      window->set_pos_ptr(&config->information_panel_pos);

      if(config->information_panel_pos == vec3i())
        config->information_panel_pos = menu_pos;

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        if(!config->visual.show_information)
          return false;

        if(!global->menu_open && !utils::is_in_game())
          return false;

        if(global->engine->is_playing_demo())
          return false;

        return true;
      });

      window->add_hook(GCS_HOOK_SHOULD_PAINT, [](gcs_component_base* c, void* p){
        return global->menu_open || utils::is_in_game();
      });

      window->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
        return global->menu_open;
      });

      window->add_hook(GCS_HOOK_MAIN_PAINT, [](gcs_component_base* c, void* p){
        if(!utils::is_in_game() && !global->menu_open)
          return true;

        if(!utils::is_in_game()){
          c->font()->draw(c->pos() + (c->size() / 2) + vec3i(0, 1), 12, WXOR(L"NOT IN GAME"), c->scheme()->text, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_CENTER);
          c->info()->size = vec3i(150, 12);
        }
        else{
          i32 padding = 4;
          c->info()->size = vec3i(224, 0);

          float time = math::time();

          static wchar_t entry_name_buffer[128];
          static wchar_t entry_status_buffer[128];

          for(u32 i = 0; i < 128; i++){
            if(global->info[i].time <= time)
              continue;

            wsprintfW(entry_name_buffer, WXOR(L"%ls"), global->info[i].name.c_str());
            wsprintfW(entry_status_buffer, WXOR(L"%ls"), global->info[i].status.c_str());

            vec3i name_size = c->font()->draw(c->pos() + vec3i(padding, padding + c->size().y), 12, entry_name_buffer, rgb(255, 255, 255), TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);
            c->font()->draw(c->pos() + vec3i(c->size().x - padding, padding + c->size().y), 12, entry_status_buffer, global->info[i].clr, TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_RIGHT);

            c->info()->size.y += name_size.y;
          }

          {
            if(c->info()->size.y > 0)
              c->info()->size.y += padding * 2;
          }
        }


        return true;
      });

    }

    return true;
  }

  ALWAYSINLINE void add_entry(u32 id, std::wstring name, std::wstring status, colour clr = colour(248, 248, 248)){
    assert(id < 128);
    assert(id >= 0);
    assert(name.length() > 0);
    assert(name.length() < 128);

    clr.w = 255;
    global->info[id].name      = name;
    global->info[id].status    = status;
    global->info[id].clr       = clr;
    global->info[id].time      = math::time() + INFO_EXPIRE_TIME;
  }
};