#pragma once

#define PLAYER_LIST_OPTION_SIZE_FONT 18
#define PLAYER_LIST_OPTION_SIZE render->rescale(PLAYER_LIST_OPTION_SIZE_FONT)

#if defined(DEV_MODE)
//#define RENDER_BOTS
#endif

enum GCS_PLAYER_LIST_ICON_FLAGS{
  GCS_ICON_KNOWN_CHEATER = 1 << 1,
  GCS_ICON_KNOWN_BOT = 1 << 2,
  GCS_ICON_KNOWN_ACTOR = 1 << 3,
  GCS_ICON_KNOWN_GROUP = 1 << 4,
  GCS_ICON_RIJIN = 1 << 5,
  GCS_ICON_FRIEND = 1 << 6,
  GCS_ICON_SUSPECT = 1 << 7,
  GCS_ICON_AIMBOT_IGNORE = 1 << 8,
  GCS_ICON_PRIORITY = 1 << 9,
};

struct s_tag_data{
  colour      first;
  std::wstring second;
};

inline s_tag_data make_tag(colour clr, std::wstring t){
  s_tag_data data;
  data.first = clr;
  data.second = t;
  return data;
}

class gcs_base_player_list : public gcs_component_base{
public:
  void setup() override{
    set_size(vec3i(info()->size.x, PLAYER_LIST_OPTION_SIZE));
  }

  vec3i size(){
    vec3i org_size = info()->size;

    assert(global_internal->global_data != nullptr);
    if(global_internal->global_data->max_clients <= 1 || !utils::is_in_game())
      return org_size;

    i32 count = 0;
    for(i32 i = 1; i <= global_internal->global_data->max_clients; i++){
      if(i == global_internal->localplayer_index)
        continue;

      s_player_info info;
      if(!global_internal->engine->get_player_info(i, &info))
        continue;

      #if !defined(RENDER_BOTS)
        if(info.fake_player)
          continue;
      #endif

      count++;
    }

    return count > 0 ?
      vec3i(org_size.x, (PLAYER_LIST_OPTION_SIZE + (info()->auto_positioning_padding.y / 2)) * count) : org_size;
  }

  // Same for this.
  virtual i32 compute_flags(i32 index){
    i32 flags = 0;
    assert(global_internal->base_cheat_detection != nullptr);
    if(global_internal->base_cheat_detection != nullptr){
      if(global_internal->base_cheat_detection->is_cheating(index))
        flags |= GCS_ICON_KNOWN_CHEATER;

      if(global_internal->base_cheat_detection->is_cheating_bot(index))
        flags |= GCS_ICON_KNOWN_BOT;

      if(global_internal->base_cheat_detection->is_friend(index))
        flags |= GCS_ICON_FRIEND;

      c_entity_info* entity_info = global_internal->base_cheat_detection->get_entity_info(index);
      if(entity_info != nullptr){
        if(entity_info->info->has_alias)
          flags |= GCS_ICON_KNOWN_ACTOR;

        if(entity_info->info->has_group)
          flags |= GCS_ICON_KNOWN_GROUP;

        if(entity_info->info->cheater_groups > 0 || entity_info->info->bot_groups > 0)
          flags |= GCS_ICON_SUSPECT;

        #if defined(DEV_MODE)
          if(entity_info->info->cheater_flags & CHEATER_FLAG_RIJIN_USER)
            flags |= GCS_ICON_RIJIN;
        #endif
      }
    }

    return flags;
  }

  virtual void on_player_list_selected_index(const i32 index){
    FUNCTION_OVERRIDE_NEEDED;
  }

  virtual void on_player_list_render_finished(){
    FUNCTION_OVERRIDE_NEEDED;
  }

  // This has to be handled by the game since colours can be dynamic.
  virtual colour get_team_colour_ent(c_internal_base_entity* entity){
    colour clr = colour(255, 255, 255, 255);
    if(entity == nullptr)
      return clr;

    return clr;
  }

  virtual i32 get_max_team_count(){
    return 4;
  }

  virtual bool input_allowed(i32 index){
    if(!index)
      return false;

    return global_internal->entity_list->get_entity(index) != nullptr;
  }

  inline colour get_team_clr(i32 index){
    assert(index > 0);
    return get_team_colour_ent(global_internal->entity_list->get_entity(index));
  }

  static s_tag_data get_emoji(const u32 icon_flags){
    colour clr = colour(255, 0, 0, 255);

    #if defined(DEV_MODE)
      if(icon_flags & GCS_ICON_RIJIN)
        return make_tag(colour(228, 32, 255, 255), WXOR(L"🌐"));
    #endif

    if(icon_flags & GCS_ICON_FRIEND)
      return make_tag(colour(96, 255, 132, 255), WXOR(L"♦️"));

    const bool known_actor = icon_flags & GCS_ICON_KNOWN_ACTOR || icon_flags & GCS_ICON_KNOWN_GROUP;
    if(known_actor)
      clr = colour(255, 255, 0, 255);

    if(icon_flags & GCS_ICON_KNOWN_CHEATER || icon_flags & GCS_ICON_KNOWN_BOT)
      return make_tag(clr, WXOR(L"🚩"));
    else if(known_actor)
      return make_tag(clr, WXOR(L"★"));

    if(icon_flags & GCS_ICON_SUSPECT)
      return make_tag(colour(255, 200, 0, 255), WXOR(L"❓"));

    if(icon_flags & GCS_ICON_PRIORITY)
      return make_tag(colour(255, 32, 32, 255), WXOR(L"🎯"));

    if(icon_flags & GCS_ICON_AIMBOT_IGNORE)
      return make_tag(colour(255, 32, 32, 255), WXOR(L"🚫"));

    return make_tag(colour(255, 255, 255, 255),  WXOR(L"➖"));
  }

  void draw(vec3i pos, vec3i size) override;

};