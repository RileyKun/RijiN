#pragma once

class c_entity_data;
class c_base_player;
struct s_glow_property;
struct s_base_button;
class c_base_entity : public c_internal_base_player{
public:
  bool& can_use_fast_path(){
    gen_read_offset(bool, "DT_BaseAnimating_m_flFrozen", 0xC);
  }

  float next_shove_time(){
    gen_read(float, "DT_LocalPlayerExclusive_m_flNextShoveTime");
  }

  float last_shove_time(){
    gen_read_offset(float, "DT_LocalPlayerExclusive_m_flNextShoveTime", 0x8);
  }

  u16& ent_client_flags(){
    gen_read(u16, "ent_client_flags");
  }

  float raged(){
    gen_read(float, "DT_Witch_m_rage");
  }

  float wander_rage(){
    gen_read(float, "DT_Witch_m_wanderRage");
  }

  bool is_alive();

  NEVERINLINE bool is_witch_raged(){
    u32 t = type();
    if(!(t & TYPE_WITCH))
      return false;

    return raged() >= 1.0f;
  }

  i32 weapon_id(){
    gen_read(i32, "DT_WeaponSpawn_m_weaponID");
  }

  bool is_burning(){
    gen_read(bool, "DT_Infected_m_bIsBurning");
  }

  bool is_ghost(){
    gen_read(bool, "DT_SPlayer_m_isGhost");
  }

  i32 get_zombie_class(){
    gen_read(i32, "DT_SPlayer_m_zombieClass");
  }

  i32 max_health(){
    gen_read(i32, "DT_SPlayer_m_iMaxHealth");
  }

  bool& survivor_glow_enabled(){
    gen_read(bool, "DT_SPlayer_m_bSurvivorGlowEnabled");
  }

  i8* get_sequence_name();
  c_entity_data* data();

  ALWAYSINLINE bool should_render(){
    return type() != 0;
  }

  NEVERINLINE u32 type(){
    s_client_class* cc = get_client_class();

    if(cc == nullptr)
      return 0;

    if(cc->name == nullptr){
      DBG("[!] Got a nullptr s_client_class\n");
      return 0;
    }

    u32 type_flags = 0;
    {
      if(cc->id == CTerrorPlayer)
        type_flags |= TYPE_REAL_PLAYER;

      if(cc->id == SurvivorBot)
        type_flags |= TYPE_BOT;

      // Skip if survivor
      if(team() == 2)
        return type_flags;

      // Are we a player and infected?
      if((type_flags & TYPE_REAL_PLAYER) && get_zombie_class() != ZOMBIE_NOT_INFECTED){
        switch(get_zombie_class()){
          default: break;
          case ZOMBIE_SMOKER:  type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_SMOKER);  break;
          case ZOMBIE_BOOMER:  type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_BOOMER);  break;
          case ZOMBIE_SPITTER: type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_SPITTER); break;
          case ZOMBIE_CHARGER: type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_CHARGER); break;
          case ZOMBIE_JOCKEY:  type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_JOCKEY);  break;
          case ZOMBIE_HUNTER:  type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_HUNTER);  break;
          case ZOMBIE_TANK:    type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_TANK);    break;
        }
      }
      else{

        u32 name_hash = HASH_RT(cc->name);
        if(name_hash == HASH("Boomer"))
          type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_BOOMER);

        switch(cc->id){
          default: break;
          case Jockey:   type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_JOCKEY);   break;
          case Spitter:  type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_SPITTER);  break;
          case Charger:  type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_CHARGER);  break;
          case Hunter:   type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_HUNTER);   break;
          case Smoker:   type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_SMOKER);   break;
          //case Boomer:   type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_BOOMER);   break; The boomer ID is zero and so is other entities.
          case Tank:     type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_TANK);     break;
          case Witch:    type_flags |= (TYPE_IS_ZOMBIE | TYPE_SPECIAL | TYPE_WITCH);    break;
          case Infected: type_flags |= (TYPE_IS_ZOMBIE | TYPE_INFECTED);                break;
        }
      }
    }

    return type_flags;
  }

  ALWAYSINLINE bool is_npc_or_human(){
    u32 t = type();
    if(t == 0)
      return false;

    return t & TYPE_REAL_PLAYER || t & TYPE_BOT || t & TYPE_IS_ZOMBIE;
  }

  ALWAYSINLINE bool is_player_entity(){
    u32 t = type();
    if(t == 0)
      return false;

    return t & TYPE_REAL_PLAYER || t & TYPE_BOT;
  }


  ALWAYSINLINE bool is_human(){
    u32 t = type();
    if(t == 0)
      return false;

    return t & TYPE_REAL_PLAYER;
  }

  NEVERINLINE bool is_invisible(){
    u32 t = type();
    if(t & TYPE_WITCH) // Well this could've been a case of a crash since the witch doesn't have the member of is_ghost
      return false;

    if(!is_human())
      return false;

    if(!is_special_infected())
      return false;

    return is_ghost();
  }

  ALWAYSINLINE bool is_infected(){
    u32 t = type();
    if(t == 0)
      return false;

    return t & TYPE_INFECTED;
  }

  ALWAYSINLINE bool is_infected_burning(){
    if(!is_infected())
      return false;

    return is_burning();
  }

  ALWAYSINLINE bool is_special_infected(){
    u32 t = type();
    if(t == 0)
      return false;

    return t & TYPE_JOCKEY || t & TYPE_SPITTER || t & TYPE_CHARGER || t & TYPE_HUNTER || t & TYPE_SMOKER || t & TYPE_BOOMER || t & TYPE_TANK || t & TYPE_WITCH;
  }

  ALWAYSINLINE void disable_interpolation(){
    c_utlvector<s_var_map_entry>* map = (u32)this + 24;

    for(i32 i = 0; i < map->size; i++)
      map->memory[i].needs_interpolation = 0;

    //u32 var_map = (u32)this + 20;

    //for ( i32 idx = 0; idx < *(i32*)( var_map + 20 ); idx++ )
    //  *(u32*)( *(u32*)( var_map ) + idx * 12 ) = 0;
  }

  bool is_active_grenade(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    switch(cc->id){
      default: break;
      case CVomitJarProjectile:
      case CSpitterProjectile:
      case CPipeBombProjectile:
      case CMolotovProjectile:
      case CGrenadeLauncher_Projectile:
        return true;
    }

    return false;
  }

  std::wstring get_grenade_name(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return L"";

    switch(cc->id){
      default: break;
      case CVomitJarProjectile:
      case CSpitterProjectile:
      case CPipeBombProjectile:
      case CMolotovProjectile:
        return get_dropped_item_name();
      case CGrenadeLauncher_Projectile:
        return L"Grenade";
    }

    return L"";
  }

  NEVERINLINE s_glow_property* get_glow_property(){
    gen_read_pointer(s_glow_property*, "DT_BaseEntity_m_Glow");
  }

  ALWAYSINLINE s_base_button* get_button_entity(){
    s_client_class* cc = get_client_class();
    if(cc->id != CBaseButton)
      return nullptr;

    return (s_base_button*)this;
  }

  NEVERINLINE c_base_player* get_player(){
    u32 t = type();
    if(t & TYPE_SPECIAL || !(t & TYPE_IS_ZOMBIE))
      return (c_base_player*)this;

    return nullptr;
  }

  std::wstring get_dropped_weapon_name();
  std::wstring get_dropped_item_name();

  bool is_health_item();
  bool is_grenade_item();
  bool is_ammo_item();
  bool is_objective();
  bool is_dropped_weapon();
  bool is_dropped_item();
  bool can_entity_give_ammo();
};