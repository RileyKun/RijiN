#pragma once

#include "settings.h"

#define INFO_EXPIRE_TIME 1.0f

#define INFO_PANEL_WARNING_CLR   colour(255, 255, 64)
#define INFO_PANEL_LEGIT_CLR     colour(32, 255, 64)
#define INFO_PANEL_RAGE_CLR      colour(255, 32, 32)
#define INFO_PANEL_WARN_RAGE_CLR colour(255, 96, 32)

enum{
  INFO_PANEL_ANTI_AFK,
  INFO_PANEL_SEED_PRED,
  INFO_PANEL_MVM_REVIVE,
  INFO_PANEL_ACP,
  INFO_PANEL_AUTO_VOTE,
  INFO_PANEL_SERVER_LAGGER,
  INFO_PANEL_FAKE_LATENCY,
  INFO_PANEL_AIMBOT,
  INFO_PANEL_TRIGGERBOT,
  INFO_PANEL_NOSPREAD,
  INFO_PANEL_ANTIAIM,
  INFO_PANEL_ANTIAIM_FAKEDUCK,
  INFO_PANEL_ANTIAIM_BREAK_ANIMATIONS,
  INFO_PANEL_FAKELAG,
  INFO_PANEL_FREECAM,
  INFO_PANEL_THIRDPERSON,
  INFO_PANEL_NOISEMAKER,
  INFO_PANEL_BACKTRACK_TO_CROSSHAIR,
  INFO_PANEL_AUTO_MEDIC,
  INFO_PANEL_AUTO_DISGUISE,
  INFO_PANEL_ANTI_BACKSTAB,
  INFO_PANEL_AUTO_ROCKET_JUMPER,
  INFO_PANEL_AUTO_MEDIC_SHIELD,
  INFO_PANEL_AUTO_MEDIC_SHIELD_AUTO_DEPLOY,
  INFO_PANEL_AUTO_VACCINATOR,
  INFO_PANEL_AUTO_VACCINATOR_MANUAL_CHARGE,
  INFO_PANEL_AUTO_UBER,
  INFO_PANEL_AUTO_KRITZ,
  INFO_PANEL_AUTO_AIRBLAST,
  INFO_PANEL_AUTO_DETONATOR,
  INFO_PANEL_AUTO_STICKY,
  INFO_PANEL_AUTO_STICKY_RELEASE,
  INFO_PANEL_AUTO_SAPPER,
  INFO_PANEL_HUO_LONG_HEATER,
  INFO_PANEL_CHARGE_BOT,
  INFO_PANEL_CHARGE_BOT_MEDIC_FOLLOW,

  INFO_PANEL_PEEK_FAKELAG,
  INFO_PANEL_ANTIBACKTRACK_FAKELAG,
};

#define assign_name(x, y) formatW(global->info[x].name, y);

class gcs_info_panel : public gcs_scene{
public:
  vec3i window_size = vec3i(224, 10);

  NEVERINLINE void precache(){
    static bool first_time = false;
    if(first_time)
      return;

    assign_name(INFO_PANEL_ANTI_AFK, WXOR(L"ANTI-AFK"));
    assign_name(INFO_PANEL_SEED_PRED, WXOR(L"SEED-PREDICTION"));
    assign_name(INFO_PANEL_MVM_REVIVE, WXOR(L"FORCE MVM REVIVE"));
    assign_name(INFO_PANEL_ACP, WXOR(L"ANTI-CHEAT COMPATIBILITY"));
    assign_name(INFO_PANEL_AUTO_VOTE, WXOR(L"AUTO-VOTE"));
    assign_name(INFO_PANEL_FAKE_LATENCY, WXOR(L"FAKE-LATENCY"));
    assign_name(INFO_PANEL_AIMBOT, WXOR(L"AIMBOT"));
    assign_name(INFO_PANEL_TRIGGERBOT, WXOR(L"TRIGGERBOT"));
    assign_name(INFO_PANEL_NOSPREAD, WXOR(L"NOSPREAD"));
    assign_name(INFO_PANEL_ANTIAIM, WXOR(L"ANTI-AIM"));
    assign_name(INFO_PANEL_ANTIAIM_FAKEDUCK, WXOR(L"FAKE-DUCK"));
    assign_name(INFO_PANEL_ANTIAIM_BREAK_ANIMATIONS, WXOR(L"BREAK ANIMATIONS"));
    assign_name(INFO_PANEL_FAKELAG, WXOR(L"FAKELAG"));
    assign_name(INFO_PANEL_FREECAM, WXOR(L"FREE-CAM"));
    assign_name(INFO_PANEL_THIRDPERSON, WXOR(L"THIRDPERSON"));
    assign_name(INFO_PANEL_NOISEMAKER, WXOR(L"INF-NOISEMAKER"));
    assign_name(INFO_PANEL_BACKTRACK_TO_CROSSHAIR, WXOR(L"BACKTRACK TO CROSSHAIR"));
    assign_name(INFO_PANEL_AUTO_MEDIC, WXOR(L"AUTO-MEDIC"));
    assign_name(INFO_PANEL_AUTO_DISGUISE, WXOR(L"AUTO-DISGUISE"));
    assign_name(INFO_PANEL_ANTI_BACKSTAB, WXOR(L"ANTI-BACKSTAB"));
    assign_name(INFO_PANEL_AUTO_ROCKET_JUMPER, WXOR(L"AUTO ROCKET-JUMPER"));
    assign_name(INFO_PANEL_AUTO_MEDIC_SHIELD, WXOR(L"AUTO MEDIC-SHIELD"));
    assign_name(INFO_PANEL_AUTO_MEDIC_SHIELD_AUTO_DEPLOY, WXOR(L"AUTO-DEPLOY-SHIELD"));
    assign_name(INFO_PANEL_AUTO_VACCINATOR, WXOR(L"AUTO-VACCINATOR"));
    assign_name(INFO_PANEL_AUTO_VACCINATOR_MANUAL_CHARGE, WXOR(L"VACCINATOR MANUAL CHARGE"));
    assign_name(INFO_PANEL_AUTO_UBER, WXOR(L"AUTO-UBER"));
    assign_name(INFO_PANEL_AUTO_KRITZ, WXOR(L"AUTO-KRITZ"));
    assign_name(INFO_PANEL_AUTO_AIRBLAST, WXOR(L"AUTO-REFLECT"));
    assign_name(INFO_PANEL_AUTO_DETONATOR, WXOR(L"AUTO-DETONATOR"));
    assign_name(INFO_PANEL_AUTO_STICKY, WXOR(L"AUTO-STICKY"));
    assign_name(INFO_PANEL_AUTO_STICKY_RELEASE, WXOR(L"AUTO-STICKY RELEASE"));
    assign_name(INFO_PANEL_AUTO_SAPPER, WXOR(L"AUTO-SAPPER"));
    assign_name(INFO_PANEL_HUO_LONG_HEATER, WXOR(L"HOU-LONG HEATER"));
    assign_name(INFO_PANEL_CHARGE_BOT, WXOR(L"CHARGEBOT"));
    assign_name(INFO_PANEL_CHARGE_BOT_MEDIC_FOLLOW, WXOR(L"QUICKFIX FOLLOW CHARGE"));
    assign_name(INFO_PANEL_PEEK_FAKELAG, WXOR(L"PEEK FAKELAG"));
    assign_name(INFO_PANEL_ANTIBACKTRACK_FAKELAG, WXOR(L"ANTI-BACKTRACK"));

    first_time = true;    
  }

  bool setup() override {
    precache();
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
          c->info()->size = render->rescale(vec3i(150, 12));
        }
        else{
          i32 padding = 4;
          c->info()->size = vec3i(render->rescale(224), 0);

          float time = math::time();

          static wchar_t entry_name_buffer[128];
          static wchar_t entry_status_buffer[128];

          for(u32 i = 0; i < 128; i++){
            if(global->info[i].time <= time)
              continue;

            formatW(entry_status_buffer, WXOR(L"%ls"), global->info[i].status.c_str());

            vec3i name_size = c->font()->draw(c->pos() + vec3i(padding, padding + c->size().y), 12, global->info[i].name, rgb(255, 255, 255), TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_LEFT);
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

  NEVERINLINE void add_entry(u32 id, std::wstring status, colour clr = colour(248, 248, 248)){
    assert(id >= 0 && id < 128);

    clr.w = 255;
    global->info[id].status    = status;
    global->info[id].clr       = clr;
    global->info[id].time      = math::time() + INFO_EXPIRE_TIME;
  }

  ALWAYSINLINE void clear(u32 id){
    global->info[id].time = -1.f;
  }
};