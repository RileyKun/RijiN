#pragma once

struct s_sticky_entry{
  c_base_entity*  ptr;
  float           time;
};

struct s_sticky_list{
  s_sticky_entry  list[32];
  u32             index;

  void reset(){
    memset(this, 0, sizeof(*this));
  }

  s_sticky_entry* get_entry(c_base_entity* ptr){
    for(u32 i = 0; i < (sizeof(list) / sizeof(s_sticky_entry)); i++){
      s_sticky_entry* e = &list[i];

      if(e->ptr == nullptr)
        break;

      if(e->ptr == ptr)
        return e;
    }

    return nullptr;
  }

  s_sticky_entry* add_entry(c_base_entity* ptr){
    s_sticky_entry* e = get_entry(ptr);

    if(e != nullptr)
      return e;

    e = &list[index];
    {
      e->ptr  = ptr;
      e->time = 0.f;
    }

    index = (index + 1) % (sizeof(list) / sizeof(s_sticky_entry));
    DBG("[!] ADDED STICKY %X %i\n", ptr, index);
    return e;
  }
};

class c_auto_sticky{
public:
  s_sticky_list sticky_list;

  bool run();
  bool is_active(bool update_info = false);
  bool is_visible(c_base_player* localplayer, c_base_entity* entity, c_base_entity* sticky, vec3 sticky_pos, vec3 pos);
  bool is_target_entity(c_base_entity* entity, c_base_player* localplayer, c_base_weapon* weapon);

  // Used to avoid attempting to blow up stickies that fly near players but can't be detonated.
  bool is_sticky_armed(c_base_entity* entity, c_base_weapon* weapon);
  bool is_local_sticky(c_base_entity* entity);
  float get_blast_radius_ratio();
};

CLASS_EXTERN(c_auto_sticky, auto_sticky);