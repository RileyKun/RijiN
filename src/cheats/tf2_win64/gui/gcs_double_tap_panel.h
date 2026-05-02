#pragma once

#include "settings.h"

class gcs_double_tap_panel : public gcs_scene{
public:
  vec3i window_size = vec3i(150, 15);

  bool setup() override {
    vec3i menu_pos = vec3i(render->screen_size.x / 2, (i32)((float)render->screen_size.y * 0.10f));

    gcs_component_window* window = gcs::window(this, WXOR(L"Double tap"), menu_pos, render->rescale(window_size));
    {
      assert(window != nullptr);
      window->set_pos_ptr(&config->double_tap_panel_pos);

      if(config->double_tap_panel_pos == vec3i())
        config->double_tap_panel_pos = menu_pos - (window_size / 2);

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        if(!config->misc.double_tap_enabled || !config->misc.double_tap_draw_panel)
          return false;

        if(global->menu_open)
          return true;

        if(!utils::is_in_game())
          return false;

        if(global->engine->is_playing_demo())
          return false;

        c_base_player* localplayer = utils::localplayer();

        if(localplayer == nullptr)
          return false;

        return localplayer->is_alive();
      });

      window->add_hook(GCS_HOOK_SHOULD_PAINT, [](gcs_component_base* c, void* p){
        return global->menu_open;
      });

      window->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
        return global->menu_open;
      });

      window->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        if(!utils::is_in_game() && !global->menu_open)
          return true;

        float         anim_timer   = math::fmod(globaldata->cur_time * 0.5f, 2.f);
        float         anim_frac    = math::clamp(anim_timer >= 1.f ? 1.f - math::abs(1.f - anim_timer) : anim_timer, 0.f, 1.f);
        static float  charge_frac  = 0.f;

        if(!utils::is_in_game() || double_tap->charged_ticks < 1)
          charge_frac = math::clamp(math::lerp(charge_frac, 0.f, 0.10f), 0.f, 1.f);
        else if(double_tap->charged_ticks > 0)
          charge_frac = math::clamp(math::lerp(charge_frac, (float)double_tap->charged_ticks / (float)double_tap->get_wanted_charge_ticks(), 0.10f), 0.f, 1.f);

        if(!global->menu_open){
          global->esp_font_small->draw(c->pos() + vec3i(0, -1), 10, WXOR(L"CHARGE"), c->scheme()->text, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_TOP);

          bool can_wep_dt = true;
          {
            c_base_player* localplayer = utils::localplayer();

            if(localplayer != nullptr){
              c_base_weapon* weapon = localplayer->get_weapon();

              if(weapon != nullptr)
                can_wep_dt = weapon->is_able_to_dt();
            }
          }

          i32 state = double_tap->get_charge_state();
          if(!can_wep_dt)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"WEAPON CANT DT"), math::fmod(globaldata->cur_time, 1.f) > 0.5f ? rgb(255, 63, 52) : rgb(255,255,255), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_READY)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"READY"), rgb(11, 232, 129), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_READY_ATTACK2_ONLY)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"READY - ATTACK2 ONLY"), rgb(11, 232, 129), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_READY_ATTACK1_2)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"READY - ATTACK1&2"), rgb(11, 232, 129), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_CHARGING)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"CHARGING"), rgb(255, 168, 1), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_NOT_ENOUGH_CHARGE)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"NOT ENOUGH CHARGE"), rgb(255, 63, 52), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_NO_CHARGE)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"NO CHARGE"), rgb(255, 63, 52), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_DT_IMPOSSIBLE)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"DT IMPOSSIBLE"), rgb(255, 63, 52), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else if(state == DT_STATE_GROUND_ONLY)
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"NOT ON GROUND"), rgb(255, 63, 52), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
          else
            global->esp_font_small->draw(c->pos() + vec3i(c->size().x, -1), 10, WXOR(L"UNKNOWN"), rgb(255,255,255), TSTYLE_OUTLINE, TALIGN_RIGHT, TALIGN_TOP);
        }

        {
          // render the same backbox when the window border is not showing
          if(!global->menu_open){
            colour alpha_background = c->scheme()->background * 0.75f;
            alpha_background.a = 255.f * 0.95f;
            render->filled_rect(c->pos(), c->size(), alpha_background);
          }

          colour left = flt_array2clr(config->visual.double_tap_main_colour);
          colour right = math::lerp(left, flt_array2clr(config->visual.double_tap_sheen_colour), math::clamp(anim_frac, 0.5f, 1.f));

          left.w  = 255;
          right.w = 255;

          i32 w = (i32)((float)c->size().x * charge_frac) + (double_tap->charged_ticks > 0 ? 1 : 0);
          render->gradient_rect(c->pos(), vec3i(math::clamp(w, 0, c->size().x), c->size().y), left, right, left, right);

          // draw outline after gradient
          if(!global->menu_open)
            render->outlined_rect(c->pos(), c->size(), c->scheme()->main * 1.25f);
        }

        return true;
      });

    }

    return true;
  }
};