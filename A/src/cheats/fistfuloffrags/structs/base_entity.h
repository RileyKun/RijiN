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
    DATA(0x830, float last_bone_setup_time);
    DATA(0x56C, i32 model_bone_counter);

    // fof player
    DATA(DT_FoFLocalPlayerExclusive06_m_flDrunkness, float drunkness);

    // c_base_combat_character
    DATA(DT_BaseCombatCharacter_m_hActiveWeapon, i32 weapon_handle);
    DATA(DT_BaseCombatCharacter_m_hActiveWeapon2, i32 weapon_handle2);
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
    c_utlvector<s_var_map_entry>* map = (u32)this + (u32)64;

    for(i32 i = 0; i < map->size; i++)
      map->memory[i].needs_interpolation = 0;
  }

  ALWAYSINLINE bool is_active_grenade(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(cc != nullptr);
      return false;
    }

    return cc->id == CBaseGrenade;
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
      case CWhiskey1:
      case CWhiskey2:
      case CColtNavy1:
      case CColtNavy2:
      case CWalker1:
      case CWalker2:
      case CVolcanic1:
      case CVolcanic2:
      case CWeaponSpencer:
      case CWeaponShotgunFoF:
      case CWeaponSharps1874:
      case CSchofield1:
      case CSchofield2:
      case CSawedShotgun1:
      case CSawedShotgun2:
      case CRemington_Army:
      case CRemington_Army2:
      case CPeacemaker1:
      case CPeacemaker2:
      case CMauser1:
      case CMauser2:
      case CMaresLeg1:
      case CMaresLeg2:
      case CWeaponMachete:
      case CHammerless1:
      case CHammerless2:
      case CGhostGun1:
      case CGhostGun2:
      case CWeaponFistsGhost:
      case CWeaponFists:
      case CWeaponDynamiteBlack:
      case CWeaponDynamiteBelt:
      case CWeaponDynamite:
      case CDeringer1:
      case CDeringer2:
      case CWeaponCoachgun:
      case CWeaponCarbine:
      case CWeaponBowie:
      case CWeaponBowarrowBlack:
      case CWeaponBowarrow:
      case CWeaponAxe:
      case CWeaponYellowboy:
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
      case CWhiskey1:
      case CWhiskey2:
        return L"Whiskey";
      case CColtNavy1:
      case CColtNavy2:
        return L"Colt Navy";
      case CWalker1:
      case CWalker2:
        return L"Walker";
      case CVolcanic1:
      case CVolcanic2:
        return L"Volcanic";
      case CWeaponSpencer:
        return L"Spencer";
      case CWeaponShotgunFoF:
        return L"Shotgun";
      case CWeaponSharps1874:
        return L"Sharps";
      case CSchofield1:
      case CSchofield2:
        return L"Schofield";
      case CSawedShotgun1:
      case CSawedShotgun2:
        return L"Sawed Shotgun";
      case CRemington_Army:
      case CRemington_Army2:
        return L"Remington";
      case CPeacemaker1:
      case CPeacemaker2:
        return L"Peace Maker";
      case CMauser1:
      case CMauser2:
        return L"Mauser";
      case CMaresLeg1:
      case CMaresLeg2:
        return L"Mares Leg";
      case CWeaponMachete:
        return L"Machete";
      case CHammerless1:
      case CHammerless2:
        return L"Hammerless";
      case CGhostGun1:
      case CGhostGun2:
        return L"Ghost gun";
      case CWeaponFistsGhost:
      case CWeaponFists:
        return L"Fists";
      case CWeaponDynamiteBlack:
        return L"Dynamite black";
      case CWeaponDynamiteBelt:
        return L"Dynamite belt";
      case CWeaponDynamite:
        return L"Dynamite";
      case CDeringer1:
      case CDeringer2:
        return L"Deringer";
      case CWeaponCoachgun:
        return L"Coachgun";
      case CWeaponCarbine:
        return L"Carbine";
      case CWeaponBowie:
        return L"Bowie";
      case CWeaponBowarrowBlack:
      case CWeaponBowarrow:
        return L"Bow";
      case CWeaponAxe:
        return L"Axe";
      case CWeaponYellowboy:
        return L"Yellow boy";
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

    return cc->id == CFoF_Player;
  }

  ALWAYSINLINE bool is_weapon_box(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == FoF_Crate;
  }

  ALWAYSINLINE bool is_horse(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }
    return cc->id == CFoF_Horse;
  }

  bool is_whiskey_bottle();
  bool is_objective();
  bool is_considered_enemy(c_base_entity* target);
  void set_abs_origin(vec3 pos);
  void set_abs_angles(vec3 angles);
};