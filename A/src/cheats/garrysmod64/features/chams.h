#pragma once

class c_studio_render_context{
public:
  void set_colour_modulation(colour clr){
    float c[3];
    c[0] = clr.x != 0 ? (float)clr.x / 255.f : 0.f;
    c[1] = clr.y != 0 ? (float)clr.y / 255.f : 0.f;
    c[2] = clr.z != 0 ? (float)clr.z / 255.f : 0.f;
    utils::virtual_call64<27, void, float*>(this, c);
  }

  DATA(0x250, float alpha);
};

enum chams_type{
  CHAMS_TYPE_INVALID        = 1 << 0,
  CHAMS_TYPE_PLAYER         = 1 << 1,
  CHAMS_TYPE_ENTITY         = 1 << 2,
  CHAMS_TYPE_VIEW_HANDS     = 1 << 3,
  CHAMS_TYPE_VIEW_WEAPON    = 1 << 4,
  CHAMS_TYPE_OTHER          = 1 << 5,
  CHAMS_TYPE_GLOW           = 1 << 6,
  CHAMS_TYPE_DRAW_CHILDREN  = 1 << 7,
};

enum force_level{
  FORCE_LEVEL_NONE = 0,
  FORCE_LEVEL_GLOW = 95,
  FORCE_LEVEL_VIEWMODELS = 99,
  FORCE_LEVEL_CHAMS = 1,
};

struct s_chams_context{
  c_base_entity*            entity                  = nullptr;
  c_render_context*         render_context          = nullptr;
  c_material*               applied_mat             = nullptr;
  i32                       viewport_level          = 0;
  chams_type                type                    = CHAMS_TYPE_INVALID;
};

class c_chams{
public:

  bool is_rendering;
  c_material* material_base_default       = nullptr;
  c_material* material_base_flat          = nullptr;
  c_material* material_fresnel_overlay    = nullptr;
  c_material* material_wireframe_overlay  = nullptr;

  // glow related
  c_material* material_glow_colour        = nullptr;
  c_material* material_glow_blur_x        = nullptr;
  c_material* material_glow_blur_y        = nullptr;
  c_material* material_halo_add           = nullptr;
  c_texture*  texture_render_target_frame = nullptr;
  c_texture*  texture_glow_buffer1        = nullptr;
  c_texture*  texture_glow_buffer2        = nullptr;

  c_keyvalues* create_shader_kv(i8* name){
    c_keyvalues* kv = c_keyvalues::create(name);

    if(kv == nullptr){
      assert(kv != nullptr);
      return nullptr;
    }

    DBG("[!] Created shader KV %s\n", name);
    return kv;
  }

  c_material* create_material(i8* name, c_keyvalues* kv){
    if(name == nullptr || kv == nullptr)
      return nullptr;

    c_material* m = global->material_system->create_material(name, kv);

    // this should never happen
    assert(m != nullptr);

    if(m != nullptr){
      m->increment_reference_count();

/*
      u16 flags = *(u16*)(m+(0x18+0x4));
      DBG("MATERIAL_IS_PRECACHED %i\n", flags & 0x2 ? 1 : 0);
      DBG("MATERIAL_VARS_IS_PRECACHED %i\n", flags & 0x4 ? 1 : 0);
      DBG("MATERIAL_IS_MANUALLY_CREATED %i\n", flags & 0x10 ? 1 : 0);
      DBG("MATERIAL_USES_UNC_FILENAME %i\n", flags & 0x20 ? 1 : 0);
      DBG("MATERIAL_IS_PRELOADED %i\n", flags & 0x40 ? 1 : 0);
*/

      //*(u16*)(m+(0x18+0x4)) |= 0x10 | 0x40;
    }

    DBG("[!] Created material: %s\n", name);
    return m;
  }

  void create_all_required_materials(){
    // Make sure the game doesn't uncache this material.
    c_material* white_additive = global->material_system->find_material(XOR("vgui/white_additive"), XOR("Other textures"));
    assert(white_additive != nullptr);
    if(white_additive != nullptr)
      white_additive->increment_reference_count();

    // base default material
    if(material_base_default == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("VertexLitGeneric"));

      kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
      kv->set_string(XOR("$bumpmap"), XOR("vgui/white_additive"));
      kv->set_bool(XOR("$selfillum"), true);
      kv->set_bool(XOR("$selfillumfresnel"), true);
      kv->set_bool(XOR("$nodecal"), true);
      kv->set_string(XOR("$selfillumfresnelminmaxexp"), XOR("[-0.25 1 1]"));

      material_base_default = create_material(XOR("MBD1"), kv);
      assert(material_base_default != nullptr);
    }

    // base flat material
    if(material_base_flat == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));

      kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
      kv->set_bool(XOR("$nodecal"), true);
      material_base_flat = create_material(XOR("MBF1"), kv);
      assert(material_base_flat != nullptr);
    }

    // overlay fresnel material
    if(material_fresnel_overlay == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("VertexLitGeneric"));

      kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
      kv->set_string(XOR("$bumpmap"), XOR("vgui/white_additive"));
      kv->set_bool(XOR("$additive"), true);
      kv->set_string(XOR("$color2"), XOR("[255 255 255]"));
      kv->set_bool(XOR("$selfillum"), true);
      kv->set_bool(XOR("$selfillumfresnel"), true);
      kv->set_bool(XOR("$nodecal"), true);
      kv->set_string(XOR("$selfillumfresnelminmaxexp"), XOR("[0 0 0.015]"));

      material_fresnel_overlay = create_material(XOR("MFO1"), kv);
      assert(material_fresnel_overlay != nullptr);
    }

    // overlay wireframe material
    if(material_wireframe_overlay == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));

      kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
      kv->set_bool(XOR("$wireframe"), true);
      kv->set_bool(XOR("$nodecal"), true);

      material_wireframe_overlay = create_material(XOR("MWFO1"), kv);
      assert(material_wireframe_overlay != nullptr);
    }

    // glow related
    {
      if(material_glow_colour == nullptr){

        c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));

        kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
        kv->set_int(XOR("$ignorez"), 1);
        kv->set_int(XOR("$model"), 1);
        kv->set_int(XOR("$linearwrite"), 1);

        // We'll create the glow_color material!
        material_glow_colour = create_material(XOR("glow_color"), kv);
        assert(material_glow_colour != nullptr);
        material_glow_colour->increment_reference_count();
      }

      //if(material_glow_colour == nullptr){
      //  material_glow_colour = global->material_system->find_material(XOR("dev/glow_color"), XOR("Other textures"));
      //  assert(material_glow_colour != nullptr);
      //  material_glow_colour->increment_reference_count();
      //}

      // Do not anything with reference count with this texture. -Rud
      if(texture_render_target_frame == nullptr)
        texture_render_target_frame = global->material_system->find_texture(XOR("_rt_FullFrameFB"), XOR("RenderTargets"));

      // create_named_render_target_texture_ex sometimes returns nullptr for some people. -Rud
      if(texture_glow_buffer1 == nullptr){
        texture_glow_buffer1 = global->material_system->create_named_render_target_texture_ex(XOR("TGB1"), texture_render_target_frame->get_actual_width(), texture_render_target_frame->get_actual_height(), RT_SIZE_LITERAL, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA, 0x00000001);
        assert(texture_glow_buffer1 != nullptr);
        if(texture_glow_buffer1 != nullptr)
          texture_glow_buffer1->increment_reference_count();

      }

      if(texture_glow_buffer2 == nullptr){
        texture_glow_buffer2 = global->material_system->create_named_render_target_texture_ex(XOR("TGB2"), texture_render_target_frame->get_actual_width(), texture_render_target_frame->get_actual_height(), RT_SIZE_LITERAL, IMAGE_FORMAT_RGB888, MATERIAL_RT_DEPTH_SHARED, TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA, 0x00000001);
        assert(texture_glow_buffer2 != nullptr);
        if(texture_glow_buffer2 != nullptr)
          texture_glow_buffer2->increment_reference_count();
      }

      if(material_glow_blur_x == nullptr){
        c_keyvalues* kv = create_shader_kv(XOR("BlurFilterX"));

        kv->set_string(XOR("$basetexture"), XOR("TGB1"));
        kv->set_bool(XOR("$ignorez"), true);
        material_glow_blur_x = create_material(XOR("MGBX1"), kv);
        assert(material_glow_blur_x != nullptr);
      }

      if(material_glow_blur_y == nullptr){
        c_keyvalues* kv = create_shader_kv(XOR("BlurFilterY"));

        kv->set_string(XOR("$basetexture"), XOR("TGB2"));
        kv->set_int(XOR("$bloomamount"), 5);
        kv->set_bool(XOR("$ignorez"), true);
        material_glow_blur_y = create_material(XOR("MGBY1"), kv);
        assert(material_glow_blur_y != nullptr);
      }

      if(material_halo_add == nullptr){
        c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));

        kv->set_string(XOR("$basetexture"), XOR("TGB1"));
        kv->set_bool(XOR("$additive"), true);
        kv->set_bool(XOR("$ignorez"), true);
        material_halo_add = create_material(XOR("MHA1"), kv);
        assert(material_halo_add != nullptr);
      }
    }
  }

  void set_alpha(c_base_entity* entity, float alpha, bool set_for_children = false){
    if(entity == nullptr)
      return;

    entity->render_mode  = alpha < 1.f ? 4 : 0;
    entity->clr_render_a = entity->render_mode == 4 ? 254 : 255;
    global->render_view->set_blend(alpha);

    if(set_for_children){
      c_entity_data* data = entity->data();
      if(data == nullptr)
        return;

      for(u32 i = 0; i < data->attachment_count; i++){
        c_base_entity* attachment = global->entity_list->get_entity(data->attachment_index_table[i]);
        if(attachment == nullptr)
          continue;

        if(!is_valid_attachment_entity(entity, attachment))
          continue;

        if(attachment->should_draw())
          set_alpha(attachment, alpha, false);
      }
    }
  }

  bool is_valid_attachment_entity(c_base_entity* entity, c_base_entity* attachment){
    if(entity == nullptr || attachment == nullptr)
      return false;

    if(entity == attachment)
      return false;

    s_client_class* entity_cc     = entity->get_client_class();
    if(entity_cc == nullptr)
      return false;

    s_client_class* attachment_cc = attachment->get_client_class();
    if(attachment_cc == nullptr)
      return false;

    c_base_entity* owner = attachment->get_owner();
    if(owner == nullptr){
      //DBG("[!] Why is attachment %s being rendered for entity %s (no owner relation)\n", attachment->get_client_class()->name, entity->get_client_class()->name);
      return false;
    }

    if(owner == entity)
      return true;

    //DBG("[!] Why is attachment %s being rendered for entity %s (not owned by)\n", attachment->get_client_class()->name, entity->get_client_class()->name);
    return false;
  }
  #define RIJIN_HAS_RENDERED_CHAMS_FLAG (1<<15)
  // we can store a bit in entity flags that represents that the entity was already rendered this frame by our handler
  // when the game goes to render the entity it should see this in our hook and not render the model again.
  // this is alot better than storing entities into a table, alot lot better.
  void set_rendered_this_frame(c_base_entity* entity, bool rendered, bool set_for_children = false){
    if(entity == nullptr)
      return;

    if(rendered)
      entity->ent_client_flags |= RIJIN_HAS_RENDERED_CHAMS_FLAG;
    else
      entity->ent_client_flags &= ~RIJIN_HAS_RENDERED_CHAMS_FLAG;

    if(set_for_children){
      c_entity_data* data = entity->data();
      if(data == nullptr)
        return;

      for(u32 i = 0; i < data->attachment_count; i++){
        c_base_entity* attachment = global->entity_list->get_entity(data->attachment_index_table[i]);
        if(attachment == nullptr)
          continue;

        if(!is_valid_attachment_entity(entity, attachment))
          continue;

        if(attachment->should_draw())
          set_rendered_this_frame(attachment, rendered, false);
      }
    }
  }

  ALWAYSINLINE bool has_rendered_this_frame(c_base_entity* entity){
    if(entity == nullptr)
      return false;

    return entity->ent_client_flags & RIJIN_HAS_RENDERED_CHAMS_FLAG;
  }

  bool is_in_view_plane(vec3 obb_center){
    return (global->view_matrix[3][0] * obb_center.x + global->view_matrix[3][1] * obb_center.y + global->view_matrix[3][2] * obb_center.z + global->view_matrix[3][3]) > 0.001f;
  }

  void draw_model(c_base_entity* entity, i32 flags = 0x00000001/*STUDIO_RENDER*/, bool draw_children = false){
    if(entity == nullptr)
      return;

    set_rendered_this_frame(entity, false);
    entity->draw_model(flags);
    set_rendered_this_frame(entity, true);

    if(draw_children){
      c_entity_data* data = entity->data();
      if(data == nullptr)
        return;

      for(u32 i = 0; i < data->attachment_count; i++){
        c_base_entity* attachment = global->entity_list->get_entity(data->attachment_index_table[i]);
        if(attachment == nullptr)
          continue;

        if(!is_valid_attachment_entity(entity, attachment))
          continue;

        if(attachment->should_draw())
          draw_model(attachment, flags, false);
      }
    }
  }

  void force_viewport_level(i32 lvl, s_chams_context* ctx, c_render_context* rc = nullptr){
    if(ctx != nullptr && ctx->viewport_level == lvl || ctx == nullptr && rc == nullptr)
      return;

    if(ctx != nullptr)
      rc = ctx->render_context;

    if(lvl <= 0)
      rc->depth_range(0.f, 1.f);
    else
      rc->depth_range(0.f, 0.02f / math::biggest(lvl, 1));

    if(ctx != nullptr)
      ctx->viewport_level = lvl;
  }

  void force_material(c_material* mat, s_chams_context* ctx = nullptr){
    // I've added the mat != nullptr check because the player's material will be applied on our entities if the entity chams
    // had no base material set. -Rud
    if(ctx != nullptr && ctx->applied_mat == mat && mat != nullptr)
      return;

    global->engine_model->force_material_override(mat);

    if(ctx != nullptr)
      ctx->applied_mat = mat;
  }

  float get_alpha_for_entity(s_chams_context* ctx, bool overlay){
    if(ctx == nullptr)
      return 1.f;

    if(ctx->type & CHAMS_TYPE_PLAYER){
      colour clr = player_esp->get_draw_colour(ctx->entity, overlay ? PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY : PLAYER_ESP_RENDER_TYPE_CHAMS);
      return clr.w > 0 ? (float)clr.w / 100.f : 0.f;
    }
    else if(ctx->type & CHAMS_TYPE_ENTITY){
      colour clr = world_esp->get_draw_colour(ctx->entity, overlay ? WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY : WORLD_ESP_RENDER_TYPE_CHAMS);
      return clr.w > 0 ? (float)clr.w / 100.f : 0.f;
    }

    return 1.f;
  }

  c_material* get_material_for_entity(s_chams_context* ctx, bool overlay){
    if(ctx == nullptr)
      return nullptr;


    /* BUG: -Rud (FIXED) Check force_material
      When player chams are enabled they'll override entity cham settings if

      - You disable the chams option under one of the following entity options in the menu.

      - Or the base material for world entities is none.

      When overwritten they'll usually be white and mimic the material type whether being flat or shaded.
    */

    if(ctx->type & CHAMS_TYPE_PLAYER){
      c_player_chams_settings* settings = utils::get_player_chams_settings((c_base_player*)ctx->entity);
      if(settings == nullptr)
        return nullptr;

      if(!overlay){
        if(settings->base_material_none)
          return nullptr;

        if(settings->base_material_shaded)
          return material_base_default;

        if(settings->base_material_flat)
          return material_base_flat;
      }
      else{
        if(settings->overlay_material_none)
          return nullptr;

        if(settings->overlay_material_flat)
          return material_base_flat;

        if(settings->overlay_material_fresnel)
          return material_fresnel_overlay;
      }
    }
    else if(ctx->type & CHAMS_TYPE_ENTITY){
      c_world_chams_settings* settings = utils::get_world_chams_settings(ctx->entity);
      if(settings == nullptr)
        return nullptr;

      if(!overlay){
        if(settings->base_material_none)
          return nullptr;

        if(settings->base_material_shaded)
          return material_base_default;

        if(settings->base_material_flat)
          return material_base_flat;
      }
      else{
        if(settings->overlay_material_none)
          return nullptr;

        if(settings->overlay_material_flat)
          return material_base_flat;

        if(settings->overlay_material_fresnel)
          return material_fresnel_overlay;
      }
    }

    return nullptr;
  }

  bool get_ignore_z_for_entity(s_chams_context* ctx, bool overlay){
    if(ctx == nullptr)
      return false;

    if(ctx->type & CHAMS_TYPE_VIEW_HANDS || ctx->type & CHAMS_TYPE_VIEW_WEAPON)
      return true;

    if(ctx->type & CHAMS_TYPE_PLAYER){

      c_player_chams_settings* settings = utils::get_player_chams_settings((c_base_player*)ctx->entity);
      if(settings == nullptr)
        return false;

      return settings->ignore_z;
    }
    else if(ctx->type & CHAMS_TYPE_ENTITY){
      c_world_chams_settings* settings = utils::get_world_chams_settings(ctx->entity);
      if(settings == nullptr)
        return false;

      return settings->ignore_z;
    }

    return false;
  }

  bool should_render_team(c_base_entity* entity, bool glow = false){
    if(entity == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    if(!glow){
      if(entity->is_player()){
        c_base_player* player = (c_base_player*)entity;

        c_player_chams_settings* settings = utils::get_player_chams_settings(player);
        if(settings == nullptr)
          return false;

        if(player->get_index() == global->localplayer_index && !settings->show_localplayer)
          return false;

        if(settings->max_distance > 0 && localplayer->origin().distance_meter(player->origin()) > settings->max_distance)
          return false;

        return settings->enabled;
      }
      else{ // world chams
        
        c_entity_data* data = entity->data();
        if(data == nullptr)
          return false;

        if(!data->list_data.render)
          return false;

        c_world_chams_settings* settings = utils::get_world_chams_settings(entity);
        if(settings == nullptr)
          return false;

        if(settings->max_distance > 0 && localplayer->origin().distance_meter(entity->origin()) > settings->max_distance)
          return false;
      
        return settings->object_enabled;
      }
    }
    else{
      if(entity->is_player()){
        c_base_player* player = (c_base_player*)entity;

        if(player->get_index() == global->localplayer_index)
          return config->visual.player_glow_render_localplayer;

        if(config->visual.player_glow_max_distance > 0 && localplayer->origin().distance_meter(player->origin()) > config->visual.player_glow_max_distance)
          return false;

        if(config->visual.player_glow_render_steamfriends && localplayer->is_steam_friend(player))
          return true;

        if(config->visual.player_glow_render_friendlies && player->team == localplayer->team)
          return true;

        if(config->visual.player_glow_render_enemies && player->team != localplayer->team)
          return true;

        return false;
      }
      else{
        c_entity_data* data = entity->data();
        if(data == nullptr)
          return false;

        if(!data->list_data.render)
          return false;

        c_world_glow_settings* settings = utils::get_world_glow_settings(entity);
        if(settings == nullptr)
          return false;

        if(settings->max_distance > 0 && localplayer->origin().distance_meter(entity->origin()) > settings->max_distance)
          return false;
      
        return settings->object_enabled;
      }
    }
    return false;
  }

  chams_type get_chams_type(c_base_entity* entity){
    if(entity == nullptr)
      return CHAMS_TYPE_INVALID;

    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      return CHAMS_TYPE_INVALID;

    if(!is_in_view_plane(entity->obb_center()))
      return CHAMS_TYPE_INVALID;

    if(!entity->should_draw())
      return CHAMS_TYPE_INVALID;

    if(entity->is_player()){
      c_base_player* player = (c_base_player*)entity;
      if(player->is_alive()){
        u32 flags = 0;
        if(should_render_team(entity, false))
          flags |= CHAMS_TYPE_PLAYER;

        if((config->visual.glow_enabled && config->visual.player_glow_enabled) && should_render_team(entity, true))
          flags |= CHAMS_TYPE_GLOW;

        if(flags)
          return flags | CHAMS_TYPE_DRAW_CHILDREN;
      }
    }
    else{
      u32 flags = 0;
      if(should_render_team(entity, false))
        flags |= CHAMS_TYPE_ENTITY;

      if(should_render_team(entity, true) && (config->visual.glow_enabled))
        flags |= CHAMS_TYPE_GLOW;

      if(flags)
        return flags | CHAMS_TYPE_DRAW_CHILDREN;
    }


/*
    if(cc->id == CTFViewModel)
      return CHAMS_TYPE_VIEW_HANDS;

    if(cc->id == CBaseAnimating){
      c_base_entity* parent = entity->get_owner();

      // if it has a parent, check if its the weapon we are holding
      if(parent != nullptr){
        c_base_player* localplayer = utils::localplayer();

        if(localplayer != nullptr && localplayer->get_weapon() == (c_base_weapon*)parent)
          return CHAMS_TYPE_VIEW_WEAPON | CHAMS_TYPE_GLOW_BLUR;
      }
    }
*/

    return CHAMS_TYPE_INVALID;
  }

  bool handle_base_material(s_chams_context& ctx){
    if(ctx.entity == nullptr)
      return false;


    bool ignore_z = get_ignore_z_for_entity(&ctx, false);

    c_material* wish_material = get_material_for_entity(&ctx, false);

    if(wish_material == nullptr){
      // If the entity has a render mode of 4 and clr render alpha of 254
      // Presume they they had the alpha hack for fixing render ordering enabled.
      // Restore them to the original state that they should have been in
      if(ctx.entity->render_mode == 4 && ctx.entity->clr_render_a == 254)
        set_alpha(ctx.entity, 1.f, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);


      global->render_view->set_colour_modulation(rgb(255, 255,255));
      force_material(nullptr, &ctx);

      if(ignore_z){
        force_viewport_level(FORCE_LEVEL_CHAMS, &ctx);
        draw_model(ctx.entity, 0x00000001/*STUDIO_RENDER*/, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);
        force_viewport_level(FORCE_LEVEL_NONE, &ctx);
      }
      else
        draw_model(ctx.entity, 0x00000001/*STUDIO_RENDER*/, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);

      return false;
    }

    colour draw_col  = world_esp->get_draw_colour(ctx.entity, WORLD_ESP_RENDER_TYPE_CHAMS);

    if(ctx.entity != nullptr){
      if(ctx.entity->is_player())
        draw_col = player_esp->get_draw_colour(ctx.entity, PLAYER_ESP_RENDER_TYPE_CHAMS);
    }

    global->render_view->set_colour_modulation(draw_col);
    set_alpha(ctx.entity, get_alpha_for_entity(&ctx, false), ctx.type & CHAMS_TYPE_DRAW_CHILDREN);
    force_material(wish_material, &ctx);

    // draw the model
    {
      if(ignore_z)
        force_viewport_level(FORCE_LEVEL_CHAMS, &ctx);

      draw_model(ctx.entity, 0x00000001/*STUDIO_RENDER*/, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);

      if(ignore_z)
        force_viewport_level(FORCE_LEVEL_NONE, &ctx);
    }

    // restore alpha
    global->render_view->set_blend(1.f);

    return true;
  }

  bool handle_overlay_material(s_chams_context& ctx){
    if(ctx.entity == nullptr)
      return false;


    bool ignore_z = get_ignore_z_for_entity(&ctx, true);

    c_material* wish_material = get_material_for_entity(&ctx, true);
    if(wish_material == nullptr)
      return false;

    colour draw_col  = world_esp->get_draw_colour(ctx.entity, WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY);

    if(ctx.entity != nullptr){
      if(ctx.entity->is_player())
        draw_col = player_esp->get_draw_colour(ctx.entity, PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY);
    }

    global->render_view->set_colour_modulation(draw_col);

    set_alpha(ctx.entity, get_alpha_for_entity(&ctx, true), ctx.type & CHAMS_TYPE_DRAW_CHILDREN);
    force_material(wish_material, &ctx);

    // draw the model
    {
      if(ignore_z)
        force_viewport_level(FORCE_LEVEL_CHAMS, &ctx);

      draw_model(ctx.entity, 0x00000001, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);

      if(ignore_z)
        force_viewport_level(FORCE_LEVEL_NONE, &ctx);
    }

    // restore alpha
    global->render_view->set_blend(1.f);

    return true;
  }

  bool handle_backtrack_chams(c_base_player* entity, void* rcx, void* state, s_model_render_data* info){
    /*
    if(!config->visual.backtrack_chams)
      return false;

    if(entity == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    c_base_weapon* weapon = localplayer->get_weapon();

    if(weapon == nullptr)
      return false;

    if(weapon->is_projectile_weapon() || weapon->is_harmless_item() || weapon->is_wrangler())
      return false;

    if(!localplayer->is_considered_enemy(entity))
      return false;

    s_lc_data* data = lag_compensation->get_data(entity);

    if(data == nullptr)
      return false;

    // Find the last backtrackable tick or the most forward tick depending on settings
    s_lc_record* found_record = nullptr;

    if(global->aimbot_settings->position_adjust_history){
      for(i32 i = data->max_records - 1; i > 0; i--){
        s_lc_record* record = data->get_record(i);

        if(record == nullptr)
          continue;

        if(!record->valid())
          continue;

        found_record = record;
        break;
      }
    }
    else{
      for(i32 i = 0; i < data->max_records; i++){
        s_lc_record* record = data->get_record(i);

        if(record == nullptr)
          continue;

        if(!record->valid())
          continue;

        found_record = record;
        break;
      }
    }

    // Can't find any valid records?
    if(found_record == nullptr)
      return false;

    // There is no backtrackable records
    if(found_record->index <= 0)
      return false;

    // Backup original data
    vec3   old_origin = info->origin;
    colour old_colour = global->render_view->get_colour_modulation();
    float  old_blend  = global->render_view->get_blend();

    // calculate alpha for entity
    {
      float h  = math::abs(found_record->velocity.length());
            h += math::abs(entity->velocity().length());
            h += entity->origin().distance(found_record->origin);
            h  = math::clamp(h, 0.f, 200.f);

      if(h <= 1.f)
        return false;

      if(h < 200.f)
        set_alpha(entity, math::clamp(math::clamp(h / 200.f, 0.f, 1.f), 0.f, 0.25f), false);
      else
        set_alpha(entity, 0.25f, false);
    }

    // set colour for the entity
    global->render_view->set_colour_modulation(config->visual.backtrack_chams_colour);

    utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, (void*)&found_record->bone_matrix[0]);

    // restore the original data
    info->origin = old_origin;
    global->render_view->set_colour_modulation(old_colour);
    set_alpha(entity, old_blend, false);
    */
    //return true;
    return false;
  }

  bool handle_fake_angle_chams(c_base_player* entity, void* rcx, void* state, s_model_render_data* info, matrix3x4* matrix_out){
    /*if(!config->visual.fake_angle_chams)
      return false;

    if(!antiaim->is_faking_yaw())
      return false;

    if(!global->input->thirdperson)
      return false;

    if(global->engine->is_playing_demo())
      return false;

    // CHECK FOR ANTIAIM
    if(entity == nullptr)
      return false;

    if(entity->get_index() != global->engine->get_local_player())
      return false;

    colour old_colour = global->render_view->get_colour_modulation();
    float  old_blend  = global->render_view->get_blend();

    matrix3x4 bone_matrix[128];
    if(!utils::create_custom_matrix(entity, entity->viewangles(), (matrix3x4*)&bone_matrix))
      return false;

    set_alpha(entity, 0.25f, false);
    global->render_view->set_colour_modulation(config->visual.fake_angle_chams_colour);
    utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, (void*)&bone_matrix[0]);
    global->render_view->set_colour_modulation(old_colour);
    set_alpha(entity, old_blend, false);

    if(matrix_out != nullptr)
      entity->setup_bones(matrix_out, true, true);
    */
    //return true;
    return false;
  }

  void run_chams(c_render_context* rc){
    assert(rc != nullptr);

    for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_player* entity = global->entity_list->get_entity(i);

      if(entity == nullptr || entity->is_dormant())
        continue;

      chams_type type = get_chams_type(entity);
      if(type == CHAMS_TYPE_INVALID || type == CHAMS_TYPE_VIEW_HANDS || type == CHAMS_TYPE_VIEW_WEAPON || type == CHAMS_TYPE_GLOW)
        continue;

      s_chams_context ctx = {
        entity,   // entity
        rc,       // render_context
        nullptr,  // applied_mat
        0,        // viewport_level
        type      // type
      };

      handle_base_material(ctx);
      handle_overlay_material(ctx);
    }
  }

  void run_glow_start(c_render_context* rc, s_stencil_state& stencil_state){
    assert(rc != nullptr);

    stencil_state.enable          = true;
    stencil_state.fail_op         = STENCILOPERATION_KEEP;
    stencil_state.zfail_op        = STENCILOPERATION_REPLACE; // STENCILOPERATION_KEEP will fill occluded parts of the model, creating a two pass effect
    stencil_state.pass_op         = STENCILOPERATION_REPLACE;
    stencil_state.compare_func    = STENCILCOMPARISONFUNCTION_ALWAYS;
    stencil_state.reference_value = 1;
    stencil_state.test_mask       = 0xFFFFFFFF;
    stencil_state.write_mask      = 0xFFFFFFFF;
    rc->set_stencil_state(&stencil_state);
  }

  void run_glow_finish(c_render_context* rc, s_stencil_state& stencil_state){
    if(!stencil_state.enable)
      return;

    global->render_view->set_blend(1.f);
    global->render_view->set_colour_modulation(rgb(255, 255,255));

    stencil_state                 = s_stencil_state();
    stencil_state.enable          = true;
    stencil_state.reference_value = 1;
    stencil_state.compare_func    = STENCILCOMPARISONFUNCTION_ALWAYS;
    stencil_state.pass_op         = STENCILOPERATION_REPLACE;
    stencil_state.fail_op         = STENCILOPERATION_KEEP;
    stencil_state.zfail_op        = STENCILOPERATION_REPLACE;
    rc->set_stencil_state(&stencil_state);

    for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_player* entity = global->entity_list->get_entity(i);

      if(entity == nullptr || entity->is_dormant())
        continue;

      chams_type type = get_chams_type(entity);

      if(!(type & CHAMS_TYPE_GLOW))
        continue;

      // if the entity hasnt been rendered this frame, we need to do it manually.
      if(!has_rendered_this_frame(entity)){
        global->render_view->set_colour_modulation(rgb(255, 255,255));
        global->engine_model->force_material_override(nullptr);

        draw_model(entity, 0x00000001/*STUDIO_RENDER*/, type & CHAMS_TYPE_DRAW_CHILDREN);
      }
    }


    stencil_state.enable = false;
    rc->set_stencil_state(&stencil_state);

    global->engine_model->force_material_override(material_glow_colour);

    rc->push_render_target_and_viewport();
    {
      rc->set_render_target(texture_glow_buffer1);
      rc->viewport(0, 0, render->screen_size.x, render->screen_size.y);
      rc->clear_colour_4ub(0, 0, 0, 0);
      rc->clear_buffers(true, false, false);
      for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
        c_base_player* entity = global->entity_list->get_entity(i);

        if(entity == nullptr || entity->is_dormant())
          continue;

        chams_type type = get_chams_type(entity);

        if(!(type & CHAMS_TYPE_GLOW))
          continue;

        if(entity->is_player())
          global->render_view->set_colour_modulation(player_esp->get_draw_colour(entity, PLAYER_ESP_RENDER_TYPE_GLOW));
        else
          global->render_view->set_colour_modulation(world_esp->get_draw_colour(entity, WORLD_ESP_RENDER_TYPE_GLOW));
        
        draw_model(entity, 0x00000001/*STUDIO_RENDER*/ | 0x00000080/*STUDIO_NOSHADOWS*/, type & CHAMS_TYPE_DRAW_CHILDREN);
      }
    }
    rc->pop_render_target_and_viewport();

    // blur
    if(config->visual.glow_blur){
      rc->push_render_target_and_viewport();
      {
        rc->viewport(0, 0, render->screen_size.x, render->screen_size.y);
        rc->set_render_target(texture_glow_buffer2);
        rc->draw_screen_space_rectangle(material_glow_blur_x, 0, 0, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - 1, render->screen_size.y - 1, render->screen_size.x, render->screen_size.y);
        rc->set_render_target(texture_glow_buffer1);
        rc->draw_screen_space_rectangle(material_glow_blur_y, 0, 0, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - 1, render->screen_size.y - 1, render->screen_size.x, render->screen_size.y);
      }
      rc->pop_render_target_and_viewport();
    }
    stencil_state                 = s_stencil_state();
    stencil_state.enable          = true;
    stencil_state.write_mask      = 0;
    stencil_state.test_mask       = 0xFF;
    stencil_state.reference_value = 0;
    stencil_state.compare_func    = STENCILCOMPARISONFUNCTION_EQUAL;
    stencil_state.pass_op         = STENCILOPERATION_KEEP;
    stencil_state.fail_op         = STENCILOPERATION_KEEP;
    stencil_state.zfail_op        = STENCILOPERATION_KEEP;
    rc->set_stencil_state(&stencil_state);

    if(!config->visual.glow_stencil) // glow
      rc->draw_screen_space_rectangle(material_halo_add, 0, 0, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - 1, render->screen_size.y - 1, render->screen_size.x, render->screen_size.y);
    else{ // stencil

      i32 thickness = config->visual.glow_thickness;
      rc->draw_screen_space_rectangle(material_halo_add, -thickness, -thickness, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, -thickness, 0, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, 0, -thickness, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, 0, thickness, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, thickness, thickness, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, thickness, 0, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, thickness, -thickness, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, -thickness, thickness, render->screen_size.x, render->screen_size.y, 0.f, 0.f, render->screen_size.x - thickness, render->screen_size.y - thickness, render->screen_size.x, render->screen_size.y);
    }
    stencil_state.enable = false;
    rc->set_stencil_state(&stencil_state);
  }

  bool is_viewmodel_entity(c_base_entity* entity){
    if(entity == nullptr)
      return false;

    s_client_class* cc = entity->get_client_class();

    if(cc == nullptr)
      return false;

    if(cc->id == CBaseAnimating){
      c_base_entity* parent = entity->get_owner();

      // if it has a parent, check if its the weapon we are holding
      if(parent != nullptr){
        c_base_player* localplayer = utils::localplayer();

        if(localplayer != nullptr && localplayer->get_weapon() == (c_base_weapon*)parent)
          return true;
      }
    }

    return cc->id == CPredictedViewModel;
  }

  void cleanup(s_chams_context& ctx){
    global->render_view->set_blend(1.f);

    // reset viewport level
    if(ctx.viewport_level != 0)
      force_viewport_level(FORCE_LEVEL_NONE, &ctx);

    // restore material override
    if(ctx.applied_mat != nullptr)
      global->engine_model->force_material_override(nullptr);
  }

  bool is_chams_material(c_material* material){
    if(material == nullptr)
      return false;

    i8* material_name = material->get_name();
    if(material_name == nullptr)
      return false;

    u32 name_hash = HASH_RT(material_name);
    switch(name_hash){
      default: break;
      case HASH("vgui/white_additive"):
      case HASH("dev/glow_color"):
      case HASH("_rt_FullFrameFB"):
      case HASH("mbd1"):
      case HASH("mbf1"):
      case HASH("mfo1"):
      case HASH("mwfo1"):
      case HASH("tgb1"):
      case HASH("tgb2"):
        return true;
    }

    return false;
  }

  bool material_enforced = false;
};

CLASS_EXTERN(c_chams, chams);