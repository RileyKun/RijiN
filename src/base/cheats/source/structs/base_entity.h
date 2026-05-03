#pragma once

class s_client_class;
class c_client_networkable;
class c_client_renderable;
class c_client_networkable;
class c_internal_base_entity{
public:

  // Some entities don't have this, and just assuming this + offset is a bad ida.
  ALWAYSINLINE c_client_renderable* renderable(){
    return (uptr)this + sizeof(uptr);
  }

  ALWAYSINLINE c_client_networkable* networkable(){
    return (uptr)this + (sizeof(uptr) * 2);
  }

  // This is unsafe, since we're messing around with 2 different types.
  //ALWAYSINLINE void* from_renderable(){
  //  return (uptr)this - sizeof(uptr);
  //}
//
  //ALWAYSINLINE void* from_networkable(){
  //  return (uptr)this - (sizeof(uptr) * 2);
  //}

  ALWAYSINLINE i32& model_bone_counter(){
    gen_read(i32, "model_bone_counter");
  }

  ALWAYSINLINE float& last_bone_setup_time(){
    gen_read(float, "last_bone_setup_time");
  }

  ALWAYSINLINE matrix3x4 rgfl_coordinate_frame(){
    gen_read(matrix3x4, "rgfl_coordinate_frame");
  }

  ALWAYSINLINE float surface_friction(){
    gen_read(float, "surface_friction");
  }

  ALWAYSINLINE i32 move_type(){ 
    gen_read(i32, "move_type");
  }

  ALWAYSINLINE u8& team(){
    gen_read(u8, "DT_BaseEntity_m_iTeamNum");
  }

  ALWAYSINLINE float& simulation_time(){
    gen_read(float, "DT_BaseEntity_m_flSimulationTime");
  }

  ALWAYSINLINE float& old_simulation_time(){
    gen_read_offset(float, "DT_BaseEntity_m_flSimulationTime", 0x4);
  }

  ALWAYSINLINE i32 owner_handle(){
    gen_read(i32, "DT_BaseEntity_m_hOwnerEntity");
  }

  ALWAYSINLINE u8& render_mode(){
    gen_read(u8, "DT_BaseEntity_m_nRenderMode");
  }

  ALWAYSINLINE i32 sequence(){
    gen_read(i32, "DT_BaseAnimating_m_nSequence");
  }

  ALWAYSINLINE void* pose_parameters(){
    gen_read_pointer(void*, "DT_BaseAnimating_m_flPoseParameter");
  }

  ALWAYSINLINE i32 hitbox_set(){
    gen_read(i32, "DT_BaseAnimating_m_nHitboxSet");
  }

  ALWAYSINLINE float& model_scale(){
    gen_read(float, "DT_BaseAnimating_m_flModelScale");
  }

  ALWAYSINLINE vec3& origin(){
    gen_read(vec3, "DT_BaseEntity_m_vecOrigin");
  }

  NEVERINLINE void* collision(){
    gen_read_pointer(void*, "DT_BaseEntity_m_Collision");
  }

  NEVERINLINE vec3& obb_mins(){
    gen_read_other(collision(), vec3, "DT_CollisionProperty_m_vecMins");
  }

  NEVERINLINE vec3& obb_maxs(){
    gen_read_other(collision(), vec3, "DT_CollisionProperty_m_vecMaxs");
  }

  NEVERINLINE u16 solid_flags(){
    gen_read_other(collision(), u16, "DT_CollisionProperty_m_usSolidFlags");
  }

  NEVERINLINE i32 solid_type(){
    gen_read_other(collision(), i32, "DT_CollisionProperty_m_nSolidType");
  }

  ALWAYSINLINE colour& get_render_color(){
    gen_read(colour, "DT_BaseEntity_m_clrRender");
  }

  ALWAYSINLINE u8& clr_render_a(){
    gen_read_offset(u8, "DT_BaseEntity_m_clrRender", 3);
  }

  ALWAYSINLINE u16& ent_client_flags(){
    gen_read_offset(u16, "DT_BaseEntity_m_clrRender", -sizeof(u16));
  }

  // Vtable table functions here.
  ALWAYSINLINE vec3& get_abs_origin(){
    create_offset_hash("get_abs_origin");
    return utils::internal_virtual_call<vec3&>(offset, this);
  }

  ALWAYSINLINE vec3& get_abs_angles(){
    create_offset_hash("get_abs_angles");
    return utils::internal_virtual_call<vec3&>(offset, this);
  }


  NEVERINLINE vec3& old_origin(){
    create_offset_hash("get_old_origin");
    return utils::internal_virtual_call<vec3&>(offset, this);
  }

  CFLAG_O0 NEVERINLINE i32 get_index(){
    assert(this != nullptr && "common nullptr catch");

    create_offset_hash("get_index");
    return utils::internal_virtual_call<i32>(offset, networkable());
  }

  ALWAYSINLINE bool is_dormant(){
    create_offset_hash("is_dormant");
    return utils::internal_virtual_call<bool>(offset, networkable());
  }

  ALWAYSINLINE bool should_draw(){
    create_offset_hash("should_draw");
    return utils::internal_virtual_call<bool>(offset, renderable());
  }

  NEVERINLINE s_client_class* get_client_class(){
    create_offset_hash("get_client_class");
    return utils::internal_virtual_call<s_client_class*>(offset, networkable());
  }

  ALWAYSINLINE void* get_model(){
    create_offset_hash("get_model");
    return utils::internal_virtual_call<void*>(offset, renderable());
  }

  s_studio_hdr* get_studio_hdr();

  NEVERINLINE s_studio_bbox* get_studio_bbox(const i32 hitbox){
    if(hitbox < 0)
      return nullptr;

    s_studio_hdr* hdr = get_studio_hdr();
    if(hdr == nullptr)
      return nullptr;

    if(hitbox > hdr->num_bones){
      DBG("[-] c_internal_base_entity::get_studio_bbox: greater than bone count %i/%i\n", hitbox, hdr->num_bones);
      return nullptr;
    }

    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set());
    if(set == nullptr)
      return nullptr;

    if(hitbox >= set->num_hitboxes){
      DBG("[-] c_internal_base_entity::get_studio_bbox greater than hitbox count %i/%i\n", hitbox, set->num_hitboxes);
      return nullptr;
    }

    s_studio_bbox* box = set->get_hitbox(hitbox);
    if(box == nullptr)
      return nullptr;

    return box;
  }

  NEVERINLINE s_studio_bbox* get_studio_bbox_from_hitgroup(const i32 hitgroup){
    if(hitgroup < 0 || hitgroup > hitgroup_gear)
      return nullptr;

    s_studio_hdr* hdr = get_studio_hdr();
    if(hdr == nullptr)
      return nullptr;

    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set());
    if(set == nullptr)
      return nullptr;

    for(u32 i = 0; i < set->num_hitboxes; i++){
      s_studio_bbox* box = set->get_hitbox(i);
      if(box == nullptr)
        continue;

      if(box->group == hitgroup)
        return box;
    }

    DBG("[-] get_studio_bbox_from_hitgroup failed to find hitbox from hitgroup: %i\n", hitgroup);
    return nullptr;
  }

  NEVERINLINE i32 get_max_hitbox_count(){
    s_studio_hdr* hdr = get_studio_hdr();
    if(hdr == nullptr)
      return 0;

    s_studio_hitbox_set* set = hdr->get_hitbox_set(hitbox_set());
    if(set == nullptr)
      return 0;

    return set->num_hitboxes;
  }

  bool                    setup_bones(matrix3x4* matrix, bool used_by_anything, bool force_animation_update = false);
  void                    set_abs_origin(vec3& pos, bool allow_empty_vector = false);
  void                    set_abs_angles(vec3& angles);
  c_internal_base_entity* get_owner();

  #if defined(SOURCE_2018)
  NEVERINLINE i32 draw_model(i32 flags = (0x1 | 0x080) /*STUDIO_RENDER | STUDIO_NOSHADOWS*/){
    create_offset_hash("draw_model");
    s_renderable_instance inst;
    inst.alpha = 255;

    STACK_CHECK_START;
    auto r = utils::internal_virtual_call<i32, i32, s_renderable_instance&>(offset, renderable(), flags, inst);
    STACK_CHECK_END;
    return r;
  }
  #else
  NEVERINLINE i32 draw_model(i32 flags = 0x1 | 0x080 /*STUDIO_RENDER | STUDIO_NOSHADOWS*/){
    create_offset_hash("draw_model");
    return utils::internal_virtual_call<i32, i32>(offset, renderable(), flags);
  }
  #endif


  // Helpers
  NEVERINLINE vec3 obb_center(){
    return get_abs_origin() + ((obb_mins() + obb_maxs()) * 0.5f);
  }

  NEVERINLINE vec3 obb_top(){
    return get_abs_origin() + (obb_mins() + obb_maxs());
  }

  NEVERINLINE vec3 obb_custom(float val){
    val = math::clamp(val, 0.f, 1.f);
    return get_abs_origin() + ((obb_mins() + obb_maxs()) * val);
  }

  // Notice: This is not accurate, there's a function we need to call.
  NEVERINLINE void set_min_and_maxs(const vec3& mins, const vec3& maxs){
    obb_mins() = mins;
    obb_maxs() = maxs;
  }
};