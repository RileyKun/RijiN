#pragma once

class c_entity_data;
class c_base_entity{
public:
  union{
    // c_base_entity
    DATA(DT_BaseEntity_m_iTeamNum, u8 team);
    DATA(DT_LocalPlayerExclusive_m_nTickBase, i32 tick_base);
    DATA(DT_BaseEntity_m_flSimulationTime, float simulation_time);
    DATA(DT_BaseEntity_m_flSimulationTime + 0x4, float old_simulation_time);
    DATA(DT_BaseEntity_m_hOwnerEntity, i32 owner_handle);
    DATA(DT_BaseEntity_m_nRenderMode, u8 render_mode);
    DATA(DT_BaseEntity_m_clrRender + 3, u8 clr_render_a);
    DATA(DT_BaseEntity_m_clrRender - sizeof(u16), u16 ent_client_flags);
    DATA(DT_BaseAnimating_m_flModelScale, float model_scale);
    DATA(DT_BaseAnimating_m_nHitboxSet, i32 hitbox_set);
    DATA(DT_GMOD_Player_m_nWaterLevel, i8 water_level);

    // c_base_player
    DATA(DT_BaseEntity_m_iHealth, i32 health);
    DATA(DT_BasePlayer_m_lifeState, u8 life_state);
    DATA(0x3F8, matrix3x4 rgfl_coordinate_frame);
    DATA(DT_BasePlayer_m_fFlags, i32 entity_flags);
    DATA(DT_BaseEntity_m_bOnFire, bool on_fire);
    DATA(0x1AA0, float last_bone_setup_time);
    DATA(0x1730, i32 model_bone_counter);
    DATA(DT_LocalPlayerExclusive_m_Local + DT_Local_m_bDucked, bool ducked);
    DATA(DT_LocalPlayerExclusive_m_Local + DT_Local_m_flStepSize, float step_size);
    DATA(DT_BasePlayer_m_flMaxspeed, float max_speed);
    DATA(0x3390, float surface_friction);
    DATA(DT_LocalPlayerExclusive_m_vecBaseVelocity + (0x4*3), float gravity);

  
    DATA(0x1F4, i8 move_type);

    DATA(DT_BasePlayer_m_hVehicle, i32 player_vehicle);

    // c_base_combat_character
    DATA(DT_BaseCombatCharacter_m_hActiveWeapon, i32 weapon_handle);
  };

  VIRTUAL_TABLE_RW_IMPLEMENTS()

  void* get_scripted_entity();

  ALWAYSINLINE i8* get_scripted_class_name(bool use_special_mode = false){
    if(is_npc())
      return "NPC";

    if(is_prop())
      return "prop";

    void* scripted_entity = get_scripted_entity();
    if(scripted_entity == nullptr)
      return nullptr;

    if(use_special_mode){
      if(is_combat_weapon())
        return "weapons";
    }

    return (i8*)((uptr)scripted_entity + (uptr)DT_ScriptedEntity_m_strScriptName);
  }

  std::wstring get_print_name();

  ALWAYSINLINE void* renderable(){
    assert(this != nullptr);
    return (uptr)this + sizeof(uptr);
  }

  ALWAYSINLINE void* networkable(){
    assert(this != nullptr);
    return (uptr)this + (sizeof(uptr) * 2);
  }

  ALWAYSINLINE void* from_renderable(){
    assert(this != nullptr);
    return (uptr)this - sizeof(uptr);
  }

  ALWAYSINLINE void* from_networkable(){
    assert(this != nullptr);
    return (uptr)this - (sizeof(uptr) * 2);
  }
  
  ALWAYSINLINE i32 get_index(){
    return utils::virtual_call64<9, i32>(networkable());
  }

  ALWAYSINLINE bool is_dormant(){
    return utils::virtual_call64<8, bool>(networkable());
  }

  i32 draw_model(i32 flags = 0x00000001/*STUDIO_RENDER*/){
    return utils::virtual_call64<10, i32, i32>(renderable(), flags);
  }

  ALWAYSINLINE bool is_in_vehicle(){
    if(!is_player())
      return false;

    return this->player_vehicle > 0;
  }

  ALWAYSINLINE vec3 origin(bool skip = false){
    if(!skip){
      if(is_in_vehicle())
        return get_abs_origin();
    }

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
    return utils::virtual_call64<9, uptr*>(renderable());
  }

  ALWAYSINLINE bool should_draw(){
    return utils::virtual_call64<3, bool>( renderable() );
  }

  ALWAYSINLINE s_client_class* get_client_class(){
    return utils::virtual_call64<2, s_client_class*>(networkable());
  }

  // You can find IsPlayer() which will be near these functions by using the string "show_freezepanel"
  // Current IsPlayer index: 130
  // Just ignore IsBaseObject when counting indexes cause that's TF2 only.
  ALWAYSINLINE bool is_combat_weapon(){
    return utils::virtual_call64<136, bool>(this);
  }

  ALWAYSINLINE bool is_npc(){
    //IsNPC / IsNextBot
    return utils::virtual_call64<133, bool>(this) || utils::virtual_call64<135, bool>(this);
  }

  ALWAYSINLINE bool is_prop(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr)
      return false;

    return cc->id == CDynamicProp || cc->id == CPhysicsProp;
  }

  ALWAYSINLINE bool push_entity(){
    return utils::virtual_call64<172, bool>(this);
  }

  ALWAYSINLINE bool is_lua_entity(){
    return utils::virtual_call64<170, bool>(this);
  }

  c_base_entity* get_vehicle();
  c_base_entity* get_owner();

  s_player_info info();
  s_studio_hdr* get_studio_hdr();
  bool setup_bones(matrix3x4* matrix, bool used_by_anything);

  ALWAYSINLINE void disable_interpolation(){
    // This doesn't work, origin is stuck at one position!!!
    return;
    //c_utlvector<s_var_map_entry>* map = (uptr)this + (uptr)0x28;
//
    //for(i32 i = 0; i < map->size; i++)
    //  map->memory[i].needs_interpolation = 0;
  }

  ALWAYSINLINE bool is_player(){
    s_client_class* cc = get_client_class();
    if(cc == nullptr){
      assert(false);
      return false;
    }

    return cc->id == CGMOD_Player;
  }

  ALWAYSINLINE s_studio_bbox* get_studio_bbox(i32 hitbox){
    s_studio_hdr* hdr = get_studio_hdr();

    if(hdr == nullptr)
      return nullptr;

    if(hitbox > hdr->num_bones){
      DBG("[-] get_studio_bbox: hitbox bone id %i too large\n", hitbox);
      return nullptr;
    }

    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set);

    if(set == nullptr)
      return nullptr;

    if(hitbox >= set->num_hitboxes){
      DBG("[-] get_studio_bbox: hitbox id %i too large\n", hitbox);
      return nullptr;
    }

    s_studio_bbox* box = set->get_hitbox(hitbox);

    if(box == nullptr)
      return nullptr;

    return box;
  }

  ALWAYSINLINE s_studio_bbox* get_studio_bbox_from_hg(i32 hitgroup){
    s_studio_hdr* hdr = get_studio_hdr();
  
    if(hdr == nullptr)
      return nullptr;
  
    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set);
  
    if(set == nullptr)
      return nullptr;

    // If the model only has 1 hitbox, then return the center box
    // This would normally be for error models and completely broken models
    if(set->num_hitboxes == 1)
      return set->get_hitbox(0);

    for(i32 hitbox = 0; hitbox < set->num_hitboxes; hitbox++){
      s_studio_bbox* box = set->get_hitbox(hitbox);
      if(box == nullptr)
        continue;
  
      if(box->group != hitgroup)
        continue;
  
      return box;
    }
  
    DBG("[-] get_studio_bbox_from_hg failed to find hitbox from hitgroup: %i\n", hitgroup);
    return nullptr;
  }

  bool has_bones(){
    s_studio_hdr* hdr = get_studio_hdr();
    if(hdr == nullptr)
      return false;

    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set);
    if(set == nullptr)
      return false;

    return set->num_hitboxes > 0;
  }

  void invalidate_setup_bones(){
    last_bone_setup_time              = -(3.402823e+38f);
    model_bone_counter                = -1;
  }

  vec3& get_abs_origin();
  vec3& get_abs_angles();
  void set_abs_origin(vec3 pos);
  void set_abs_angles(vec3 angles);
  c_entity_data* data();
};