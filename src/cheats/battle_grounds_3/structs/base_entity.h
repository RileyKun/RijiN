#pragma once


class c_base_entity{
public:
  union{
    // c_base_entity
    DATA(DT_BaseEntity_m_iTeamNum, u8 team);
    DATA(DT_LocalPlayerExclusive_m_nTickBase, i32 tick_base);
    DATA(DT_BaseEntity_m_flSimulationTime, float simulation_time);
    DATA(DT_BaseEntity_m_hOwnerEntity, i32 owner_handle);
    DATA(DT_BaseEntity_m_nRenderMode, u8 render_mode);
    DATA(DT_BaseEntity_m_clrRender + 3, u8 clr_render_a);
    DATA(DT_BaseEntity_m_clrRender - sizeof(u16), u16 ent_client_flags);
    DATA(DT_BaseAnimating_m_flModelScale, float model_scale);
    DATA(DT_BaseAnimating_m_nHitboxSet, i32 hitbox_set);
    DATA(DT_LocalPlayerExclusive_m_nWaterLevel, i8 water_level);

    // c_base_player
    DATA(DT_BasePlayer_m_iHealth, i32 health);
    DATA(DT_BasePlayer_m_lifeState, u8 life_state);
    DATA(DT_BasePlayer_m_fFlags - ((sizeof(float)*6) + sizeof(matrix3x4)), matrix3x4 rgfl_coordinate_frame);
    DATA(DT_BasePlayer_m_fFlags, i32 entity_flags);
    DATA(0x81C, float last_bone_setup_time);
    DATA(0x558, i32 model_bone_counter);

    // shared
    DATA(DT_SDKPlayer_m_Shared + DT_SDKPlayerShared_m_bSuperFalling, bool is_super_falling);
    DATA(DT_SDKPlayer_m_bThirdPerson, bool thirdperson);

    // c_base_combat_character
    DATA(DT_BaseCombatCharacter_m_hActiveWeapon, i32 weapon_handle);
  };

  VIRTUAL_TABLE_RW_IMPLEMENTS()

  ALWAYSINLINE void* renderable(){
    assert(this != nullptr);
    return (uptr)this + (uptr)0x4;
  }

  ALWAYSINLINE void* networkable(){
    assert(this != nullptr);
    return (uptr)this + 0x8;
  }

  ALWAYSINLINE void* from_renderable(){
    assert(this != nullptr);
    return (uptr)this - 0x4;
  }

  ALWAYSINLINE void* from_networkable(){
    assert(this != nullptr);
    return (uptr)this - 0x8;
  }

  ALWAYSINLINE i32 get_index(){
    return utils::virtual_call<9, i32>(networkable());
  }

  ALWAYSINLINE bool is_dormant(){
    return utils::virtual_call<8, bool>(networkable());
  }

  i32 draw_model(i32 flags = 0x00000001/*STUDIO_RENDER*/){
    return utils::virtual_call<10, i32, i32>(renderable(), flags);
  }

  ALWAYSINLINE vec3 get_thirdperson_camera_position(){
    return read<vec3>(DT_SDKPlayer_m_bUsingVR + 0x5);
  }

  ALWAYSINLINE vec3 origin(){
    return read<vec3>(DT_BaseEntity_m_vecOrigin);
  }

  ALWAYSINLINE vec3 obb_mins(){
    return read<vec3>(DT_BaseEntity_m_Collision + DT_CollisionProperty_m_vecMins);
  }

  ALWAYSINLINE vec3 obb_maxs(){
    return read<vec3>(DT_BaseEntity_m_Collision + DT_CollisionProperty_m_vecMaxs);
  }

  ALWAYSINLINE vec3 obb_center(){
    return origin() + ((obb_mins() + obb_maxs()) * 0.5f);
  }

  ALWAYSINLINE uptr* get_model(){
    return utils::virtual_call<9, uptr*>(renderable());
  }

  ALWAYSINLINE bool should_draw(){
    return utils::virtual_call<3, bool>( renderable() );
  }

  ALWAYSINLINE s_client_class* get_client_class(){
    return utils::virtual_call<2, s_client_class*>(networkable());
  }

  c_base_entity* get_owner();
  c_base_entity* first_move_child();
  c_base_entity* next_move_peer();

  s_player_info info();
  s_studio_hdr* get_studio_hdr();
  bool setup_bones(matrix3x4* matrix, bool used_by_anything);

  ALWAYSINLINE void disable_interpolation(){
    c_utlvector<s_var_map_entry>* map = (u32)this + (u32)20;

    for(i32 i = 0; i < map->size; i++)
      map->memory[i].needs_interpolation = 0;
  }

  ALWAYSINLINE bool is_active_grenade(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CBaseGrenadeProjectile;
  }

  std::wstring get_grenade_name();

  ALWAYSINLINE bool is_dropped_weapon(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    switch(cc->id){
      default: break;
      case CAkimboM1911:
      case CAkimboBeretta:
      case CWeaponBeretta:
      case CWeaponBrawl:
      case CWeaponCrowbar:
      case CWeaponFAL:
      case CWeaponGrenade:
      case CWeaponM16:
      case CWeaponM1911:
      case CWeaponMAC10:
      case CWeaponMossberg:
      case CWeaponMP5K:
      case CWeaponSawnoff:
      case CWeaponShotgun:
        return true;
    }

    return false;
  }

  // Copied from base weapon cause they use the same classes
  std::wstring get_weapon_name(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return L"";
    }

    switch(cc->id){
      default: break;
      case CAkimboM1911: return L"Akimbo M1911";
      case CAkimboBeretta: return L"Akimbo Beretta";
      case CWeaponBeretta: return L"Beretta";
      case CWeaponBrawl: return L"Fists";
      case CWeaponCrowbar: return L"Crowbar";
      case CWeaponFAL: return L"FAL";
      case CWeaponGrenade: return L"Grenade";
      case CWeaponM16: return L"M16";
      case CWeaponM1911: return L"M1911";
      case CWeaponMAC10: return L"MAC10";
      case CWeaponMossberg: return L"Mossberg";
      case CWeaponMP5K: return L"MP5";
      case CWeaponSawnoff: return L"Sawnoff shotgun";
      case CWeaponShotgun: return L"Shotgun";
    }

    DBG("[!] %s: %i\n", cc->name, cc->id);
    return L"UNK";
  }

  ALWAYSINLINE bool is_player(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == CSDKPlayer;
  }

  std::wstring get_objective_name();
  bool is_objective();
  bool is_considered_enemy(c_base_entity* target);
  void set_abs_origin(vec3 pos);
  void set_abs_angles(vec3 angles);
};