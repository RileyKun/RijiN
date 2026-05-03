#pragma once

#include "settings.h"

class gcs_crit_hack_panel : public gcs_scene{
public:
  vec3i window_size = vec3i(100, 0);

  bool setup() override {
    vec3i menu_pos = vec3i(render->screen_size.x / 2, (i32)((float)render->screen_size.y * 0.75f));

    gcs_component_window* window = gcs::window(this, WXOR(L"Crit hack"), menu_pos, render->rescale(window_size));
    {
      assert(window != nullptr);
      window->set_pos_ptr(&config->crit_hack_panel_pos);

      if(config->crit_hack_panel_pos == vec3i())
        config->crit_hack_panel_pos = menu_pos - (window_size / 2);

      window->add_hook(GCS_HOOK_SHOULD_ENABLE, [](gcs_component_base* c, void* p){
        if(!config->misc.crithack_enabled || !config->misc.crithack_draw_panel)
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

        c_base_weapon* weapon = localplayer->get_weapon();

        if(weapon == nullptr)
          return false;

        return localplayer->is_alive();
      });

      window->add_hook(GCS_HOOK_SHOULD_PAINT, [](gcs_component_base* c, void* p){
        return global->menu_open;
      });

      window->add_hook(GCS_HOOK_INPUT_ENABLED, [](gcs_component_base* c, void* p){
        return global->menu_open;
      });

      //GCS_HOOK_POST_PAINT

      window->add_hook(GCS_HOOK_POST_PAINT, [](gcs_component_base* c, void* p){
        if(!utils::is_in_game() && !global->menu_open)
          return true;

        c_base_player* localplayer = utils::localplayer();

        if(localplayer == nullptr)
          return true;

        c_player_data* data = localplayer->data();
        if(data == nullptr)
          return true;

        c_base_weapon* weapon = localplayer->get_weapon();

        if(weapon == nullptr)
          return true;

        if(!weapon->can_randomly_crit())
          return true;


        i32 available_crits     = 0;
        i32 potential_crits     = 0;
        float remaining_bucket  = 0.f;
        float remaining_cost    = 0.f;

        if(!crit_hack->get_crit_counts(&available_crits, &potential_crits, &remaining_bucket, &remaining_cost))
          return true;

        s_cmd_list* force_list = crit_hack->get_cmd_list(true);
        s_cmd_list* skip_list  = crit_hack->get_cmd_list(false);
        assert(force_list != nullptr);
        assert(skip_list  != nullptr);

        vec3i                                         base_pos                 = c->pos(true) + vec3i(c->size().x / 2, 3);
        bool                                          should_draw_progress_bar = false;
        float                                         progress_bar_frac        = 0.f;
        bool                                          use_small_font           = false;
        std::vector<std::pair<std::wstring, colour>>  text;
        i32                                           text_height               = 0;
        if(utils::is_safe_mode_enabled() || !acm->should_allow_command_number_changes()){
          text.emplace_back(WXOR(L"COMMAND NUMBER CHANGE NOT ALLOWED"), rgb(188, 188, 188));

          should_draw_progress_bar = false;
          use_small_font           = true;
        }
        else if(data->tfdata.use_rapid_fire_crits && weapon->crit_time() > math::ticks_to_time(localplayer->tick_base())){
          text.emplace_back(WXOR(L"STREAM CRITS"), rgb(188, 188, 188));
          use_small_font = true;

          float t = weapon->crit_time() - math::ticks_to_time(localplayer->tick_base());
          if(t > 0.f){
            should_draw_progress_bar  = true;
            progress_bar_frac         = t / 2.f;
          }
        }
        else if(localplayer->is_crit_boosted()){
          float t = math::fmod(math::time(), 1.f);

          text.emplace_back(XOR(L"CRIT BOOST"), t >= 0.5f ? c->scheme()->text : rgb(100, 100, 100));
          should_draw_progress_bar = false;
          use_small_font           = true;
        }
        else if(weapon->is_melee() && global->tf_weapon_criticals_melee->val != 1 || global->tf_weapon_criticals->val != 1){
          text.emplace_back(WXOR(L"SERVER DISABLED CRITS"), rgb(188, 188, 188));

          should_draw_progress_bar = false;
          use_small_font           = true;
        }
        else if(utils::is_playing_mannpower()){
          text.emplace_back(WXOR(L"GAMEMODE DISABLED CRITS"), rgb(188, 188, 188));

          should_draw_progress_bar = false;
          use_small_font           = true;
        }
        else if(force_list != nullptr && (!force_list->filled || force_list->thread_running) || skip_list != nullptr && (!skip_list->filled || skip_list->thread_running)){
          text.emplace_back(WXOR(L"CALCULATING"), rgb(255,255,0));

          should_draw_progress_bar = false;
          use_small_font           = true;
        }
        else if(crit_hack->crit_banned){
          text.emplace_back(XOR(L"CRIT BANNED"), c->scheme()->error);

          static wchar_t crit_unban_dmg_str[32];
          formatW(crit_unban_dmg_str, WXOR(L"DEAL %i DAMAGE"), global->display_damage_to_unban);
          text.emplace_back((std::wstring)crit_unban_dmg_str, c->scheme()->warning);

          should_draw_progress_bar = false;
          use_small_font           = true;
        }
        else{
          static wchar_t crit_count_str[32];
          formatW(crit_count_str, WXOR(L"%i/%i crits"), available_crits, potential_crits);
          text.emplace_back(crit_count_str, available_crits > 0 ? c->scheme()->main : rgb(100, 100, 100));


          should_draw_progress_bar = !weapon->is_melee() && available_crits < potential_crits && remaining_bucket > 0.f && remaining_cost > 0.f;
          progress_bar_frac        = math::clamp(remaining_bucket / remaining_cost, 0.f, 1.f);
        }

        // draw the text
        {
          for(std::pair<std::wstring, colour> entry : text){
            c_font_d3d9* font = global->gui_font;
            text_height += font->draw(base_pos + vec3i(0, text_height), use_small_font ? 12 : 18, entry.first, entry.second, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_LEFT).y;
          }
          text.clear();
        }

        if(should_draw_progress_bar){
          colour alpha_background = c->scheme()->background * 0.75f;
          alpha_background.a = 255.f * 0.95f;

          vec3i bar_size = render->rescale(vec3i(50, 5));
          vec3i bar_pos = base_pos + vec3i(0, text_height) - vec3i(bar_size.x / 2, -2);

          render->filled_rect(bar_pos, bar_size, alpha_background);
          render->draw_progress_bar(bar_pos, bar_size, progress_bar_frac, available_crits > 0 ? c->scheme()->main : c->scheme()->text, PROGRESS_BAR_OUTLINE, 0, rgba(0,0,0,0));
          render->outlined_rect(bar_pos, bar_size, rgb(100, 100, 100));

          base_pos.y += bar_size.y;
        }

        // crit sync indicator
        if(crit_hack->tfstat_damage_ranged_crit_random.was_desynced && math::abs(math::time() - crit_hack->tfstat_damage_ranged_crit_random.last_sync_time) < 2.f){
          text.emplace_back(XOR(L"RESYNCED"), math::fmod(math::time(), 1.f) > 0.5f ? rgb(255,255,255) : (available_crits > 0 ? c->scheme()->main : rgb(100, 100, 100)));
          use_small_font = true;
        }

        // draw any text after progress bar text
        {
          if(should_draw_progress_bar)
            base_pos.y += 5;

          for(std::pair<std::wstring, colour> entry : text){
            c_font_d3d9* font = global->gui_font;
            text_height += font->draw(base_pos + vec3i(0, text_height), use_small_font ? 12 : 18, entry.first, entry.second, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_LEFT).y;
          }
        }

/*
        i32 text_height;
        if(crit_hack->crit_banned){
          std::vector<std::pair<std::wstring, colour>> text;

          // Handle adding text to buffer array
          {
            text.emplace_back(XOR(L"CRIT BANNED"), c->scheme()->error);

            static wchar_t crit_unban_dmg_str[32];
            formatW(crit_unban_dmg_str, WXOR(L"DEAL %i DAMAGE"), crit_hack->get_required_daamage_for_unban());
            text.emplace_back((std::wstring)crit_unban_dmg_str, c->scheme()->warning);
          }

          for(std::pair<std::wstring, colour> entry : text)
            text_height += global->esp_font_small->draw(base_pos + vec3i(0, text_height), 12, entry.first, entry.second, TSTYLE_OUTLINE, TALIGN_CENTER, TALIGN_CENTER).y;
        }
        else{
          if(!weapon->is_melee() && available_crits < potential_crits && remaining_bucket > 0.f && remaining_cost > 0.f){
            colour alpha_background = c->scheme()->background * 0.75f;
            alpha_background.a = 255.f * 0.95f;

            vec3i bar_pos = base_pos + vec3i(0, 2) - vec3i(1,2);
            vec3i bar_size = vec3i(3, 5);

            render->filled_rect(bar_pos, bar_size, alpha_background);
            render->draw_progress_bar(bar_pos, bar_size, math::clamp(remaining_bucket / remaining_cost, 0.f, 1.f), available_crits > 0 ? c->scheme()->main : c->scheme()->text, PROGRESS_BAR_OUTLINE, 0, rgba(0,0,0,0));
            render->outlined_rect(bar_pos, bar_size, available_crits > 0 ? c->scheme()->main : rgb(100, 100, 100));
          }
        }
*/

        //global->esp_font_large->draw(text_pos + vec3i(lightning_size.x, 0), lightning_size.y / 2, WXOR(L"2"), c->scheme()->text, TSTYLE_OUTLINE, TALIGN_LEFT, TALIGN_CENTER);

        return true;
      });

    }

    return true;
  }
};