#pragma once


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

struct s_colour_backup{
  bool   pushed;
  colour clr;
  float  blend;
  float  depth;
  ALWAYSINLINE void push(){
    clr    = global->render_view->get_colour_modulation();
    blend  = global->render_view->get_blend();
    pushed = true;
  }

  ALWAYSINLINE void pop(){
    if(!pushed){
      assert(false && "Programmer error for s_colour_backup, call push before pop, otherwise you'll get ugly colours.");
      return;
    }

    global->render_view->set_colour_modulation(clr);
    global->render_view->set_blend(blend);

    pushed = false;
  }
};

class c_chams{
public:
  s_colour_backup col_frame;

  c_material* material_base_default       = nullptr;
  c_material* material_base_flat          = nullptr;
  c_material* material_base_selfillum     = nullptr;

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

  vec3i       screen_size;
  bool        is_draw_static_props_call = false;
  bool        is_rendering_glow         = false;
  bool        override_cloak            = false;

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

    DBG("[!] Created material: %s (0x%p)\n", name, m);
    return m;
  }

  NEVERINLINE CFLAG_O0 void create_all_required_materials(){
    // Make sure the game doesn't uncache this material.
    static c_material* white_additive = global->material_system->find_material(XOR("vgui/white_additive"), XOR("Other textures"));
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
      kv->set_string(XOR("$selfillumfresnelminmaxexp"), XOR("[-0.25 1 1]"));

      material_base_default = create_material(XOR("MBD1"), kv);
    }

    // base flat material
    if(material_base_flat == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));
      kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
      material_base_flat = create_material(XOR("MBF1"), kv);
    }

    // base selfillum
    if(material_base_selfillum == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("VertexLitGeneric"));
      kv->set_string(XOR("$basetexture"), XOR("models/player/shared/ice_player"));
      kv->set_string(XOR("$bumpmap"), XOR("models/player/shared/shared_normal"));
      kv->set_bool(XOR("$additive"), true);
      kv->set_bool(XOR("$selfillum"), true);
      kv->set_bool(XOR("$selfillumfresnel"), true);
      kv->set_string(XOR("$selfillumfresnelminmaxexp"), XOR("[0.17 0.2 0.5]"));
      kv->set_string(XOR("$selfillumtint"), XOR("[0 0 0]"));
      kv->set_string(XOR("$color2"), XOR("[1 1 1]"));
      material_base_selfillum = create_material(XOR("MBS1"), kv);
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
    }

    // overlay wireframe material
    if(material_wireframe_overlay == nullptr){
      c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));

      kv->set_string(XOR("$basetexture"), XOR("vgui/white_additive"));
      kv->set_bool(XOR("$wireframe"), true);
      kv->set_bool(XOR("$nodecal"), true);
      material_wireframe_overlay = create_material(XOR("MWFO1"), kv);
    }

    // glow related
    {
      if(material_glow_colour == nullptr){
        material_glow_colour = global->material_system->find_material(XOR("dev/glow_color"), XOR("Other textures"));
        assert(material_glow_colour != nullptr);
        material_glow_colour->increment_reference_count();
      }

      // Do not anything with reference count with this texture. -Rud
      if(texture_render_target_frame == nullptr){
        texture_render_target_frame = global->material_system->find_texture(XOR("_rt_FullFrameFB"), XOR("RenderTargets"));
        assert(texture_render_target_frame != nullptr);
      }

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
      }

      if(material_glow_blur_y == nullptr){
        c_keyvalues* kv = create_shader_kv(XOR("BlurFilterY"));

        kv->set_string(XOR("$basetexture"), XOR("TGB2"));
        kv->set_int(XOR("$bloomamount"), 5);
        kv->set_bool(XOR("$ignorez"), true);
        material_glow_blur_y = create_material(XOR("MGBY1"), kv);
      }

      if(material_halo_add == nullptr){
        c_keyvalues* kv = create_shader_kv(XOR("UnlitGeneric"));

        kv->set_string(XOR("$basetexture"), XOR("TGB1"));
        kv->set_bool(XOR("$additive"), true);
        kv->set_bool(XOR("$ignorez"), true);
        material_halo_add = create_material(XOR("MHA1"), kv);
      }
    }
  }

  CFLAG_Ofast bool is_in_view_plane(vec3 obb_center){
    return (global->view_matrix_untouched[3][0] * obb_center.x + global->view_matrix_untouched[3][1] * obb_center.y + global->view_matrix_untouched[3][2] * obb_center.z + global->view_matrix_untouched[3][3]) > 0.001f;
  }

  void process_material_vars();

  c_material* get_material_from_player_settings(c_player_chams_settings* settings, bool overlay){
    if(settings == nullptr)
      return nullptr;

    if(!overlay){
      if(settings->base_material_none)
        return nullptr;

      if(settings->base_material_shaded)
        return material_base_default;

      if(settings->base_material_flat)
        return material_base_flat;

      if(settings->base_material_frensel)
        return material_fresnel_overlay;

      if(settings->base_material_selfillum)
        return material_base_selfillum;

      if(settings->base_material_wireframe)
        return material_wireframe_overlay;
    }
    else{
      if(settings->overlay_material_none)
        return nullptr;

      if(settings->overlay_material_flat)
        return material_base_flat;

      if(settings->overlay_material_fresnel)
        return material_fresnel_overlay;

      if(settings->overlay_material_wireframe)
        return material_wireframe_overlay;

      if(settings->overlay_material_selfillum)
        return material_base_selfillum;

      if(settings->overlay_material_shaded)
        return material_base_default;
    }
    
    return nullptr;
  }

  void update_screen_size();
  bool is_valid_attachment_entity(c_base_entity* entity, c_base_entity* attachment);
  void set_alpha(c_base_entity* entity, float alpha, bool set_for_children = false);
  bool override_cloak_effective(c_base_player* player);

  #define RIJIN_HAS_RENDERED_CHAMS_FLAG (1<<15)
  // we can store a bit in entity flags that represents that the entity was already rendered this frame by our handler
  // when the game goes to render the entity it should see this in our hook and not render the model again.
  // this is alot better than storing entities into a table, alot lot better.
  CFLAG_Ofast void set_rendered_this_frame(c_base_entity* entity, bool rendered, bool set_for_children = false){
    if(entity == nullptr)
      return;

    if(rendered)
      entity->ent_client_flags() |= RIJIN_HAS_RENDERED_CHAMS_FLAG;
    else
      entity->ent_client_flags() &= ~RIJIN_HAS_RENDERED_CHAMS_FLAG;

    if(set_for_children){
      i32 count = 0;

      // By the looks of it sometimes next_move_peer will loop back around to first_move_child causing a loop lock
      // To solve thie issue when it happens rarely just make sure we dont render more than 32 attachments
      for(c_base_entity* attachment = entity->first_move_child(); attachment != nullptr; attachment = attachment->next_move_peer()){
        if(attachment == nullptr || count++ >= 32)
          break;

        if(!is_valid_attachment_entity(entity, attachment))
          continue;

        if(attachment->should_draw())
          set_rendered_this_frame(attachment, rendered, false);
      }
    }
  }

  CFLAG_Ofast ALWAYSINLINE bool has_rendered_this_frame(c_base_entity* entity){
    if(entity == nullptr)
      return false;

    return entity->ent_client_flags() & RIJIN_HAS_RENDERED_CHAMS_FLAG;
  }

  // Call when rendering attachments in our draw_model function below:
  CFLAG_Ofast bool handle_attachment_colour_override(c_base_entity* entity){
    if(entity == nullptr)
      return false;

    c_base_entity* owner = entity->get_owner();
    if(owner == nullptr)
      return false;

    if(!owner->is_player())
      return false;

    c_base_player* player = owner->get_player();
    if(player == nullptr)
      return false;

    c_player_data* data = player->data();
    if(data == nullptr)
      return false;

    if(data->tfdata.weapon_index == entity->get_index())
      global->render_view->set_colour_modulation(colour(255, 255, 255, 255));

    return true;
  }

  CFLAG_Ofast void draw_model(c_base_entity* entity, i32 flags = 0x00000001/*STUDIO_RENDER*/, bool draw_children = false){
    if(entity == nullptr)
      return;

    if(!entity->should_draw()){
      DBG("[-] chams draw_model when should_draw() %s returned false\n", entity->get_client_class()->name);
      return;
    }

    set_rendered_this_frame(entity, false);
    entity->draw_model(flags);
    set_rendered_this_frame(entity, true);

    if(draw_children){
      i32 count = 0;

      // By the looks of it sometimes next_move_peer will loop back around to first_move_child causing a loop lock
      // To solve thie issue when it happens rarely just make sure we dont render more than 32 attachments
      for(c_base_entity* attachment = entity->first_move_child(); attachment != nullptr; attachment = attachment->next_move_peer()){
        if(attachment == nullptr || count++ >= 32)
          break;

        if(!is_valid_attachment_entity(entity, attachment))
          continue;

        if(attachment->should_draw()){
          draw_model(attachment, flags, false);
        }
      }
    }
  }

  CFLAG_Ofast void force_viewport_level(i32 lvl, s_chams_context* ctx, c_render_context* rc = nullptr){
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

  CFLAG_Ofast void force_viewport_level(i32 lvl, c_render_context* rc, bool* enabled){
    if(rc == nullptr)
      return;

    if(enabled != nullptr){
      if(*enabled == false)
        return;
    }

    if(lvl <= 0)
      rc->depth_range(0.f, 1.f);
    else
      rc->depth_range(0.f, 0.02f / math::biggest(lvl, 1));
  }

  CFLAG_Ofast void force_material(c_material* mat, s_chams_context* ctx = nullptr){
    // I've added the mat != nullptr check because the player's material will be applied on our entities if the entity chams
    // had no base material set. -Rud
    if(ctx != nullptr && ctx->applied_mat == mat && mat != nullptr)
      return;

    force_material_override(mat);

    if(ctx != nullptr)
      ctx->applied_mat = mat;
  }

  CFLAG_Ofast c_base_entity* get_entity(s_draw_model_info* info){
    if(info == nullptr)
      return nullptr;

    if(info->client_entity == nullptr)
      return nullptr;

    c_client_unknown* client_unknown = info->client_entity->get_client_unknown();
    
    assert(client_unknown != nullptr);
    return client_unknown->get_base_entity();
  }

  CFLAG_Ofast c_material* get_material_for_entity(s_chams_context* ctx, bool overlay){
    if(ctx == nullptr)
      return nullptr;


    /* BUG: -Rud (FIXED) Check force_material
      When player chams are enabled they'll override entity cham settings if

      - You disable the chams option under one of the following entity options in the menu.

      - Or the base material for world entities is none.

      When overwritten they'll usually be white and mimic the material type whether being flat or shaded.
    */

    if(ctx->type & CHAMS_TYPE_PLAYER){

      // Don't do any custom material on ubercharged players.
      if(ctx->entity != nullptr){
        if(ctx->entity->is_player()){
          c_base_player* p = (c_base_player*)ctx->entity;
          if(p->is_ubercharged())
            return nullptr;
        }
      }

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

        if(settings->base_material_frensel)
          return material_fresnel_overlay;

        if(settings->base_material_selfillum)
          return material_base_selfillum;

        if(settings->base_material_wireframe)
          return material_wireframe_overlay;
      }
      else{
        if(settings->overlay_material_none)
          return nullptr;

        if(settings->overlay_material_flat)
          return material_base_flat;

        if(settings->overlay_material_fresnel)
          return material_fresnel_overlay;

        if(settings->overlay_material_wireframe)
          return material_wireframe_overlay;

        if(settings->overlay_material_selfillum)
          return material_base_selfillum;

        if(settings->overlay_material_shaded)
          return material_base_default;
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

        if(settings->base_material_frensel)
          return material_fresnel_overlay;

        if(settings->base_material_selfillum)
          return material_base_selfillum;

        if(settings->base_material_wireframe)
          return material_wireframe_overlay;
      }
      else{
        if(settings->overlay_material_none)
          return nullptr;

        if(settings->overlay_material_flat)
          return material_base_flat;

        if(settings->overlay_material_fresnel)
          return material_fresnel_overlay;

        if(settings->overlay_material_wireframe)
          return material_wireframe_overlay;

        if(settings->overlay_material_selfillum)
          return material_base_selfillum;

        if(settings->overlay_material_shaded)
          return material_base_default;
      }
    }

    return nullptr;
  }

  CFLAG_Ofast bool get_ignore_z_for_entity(s_chams_context* ctx, bool overlay){
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

  CFLAG_Ofast bool should_render_team(c_base_entity* entity, bool glow = false){
    if(entity == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    if(config->visual.render_medic_mode){
      bool hurt = false;
      if(utils::medic_mode(entity, hurt))
        return hurt;
    }

    if(!glow){
      if(entity->is_player()){
        c_base_player* player = entity->get_player();

        // Prevent transparent 'weapons' when spectating a player in first person.
        if(!localplayer->is_alive() || localplayer->team() == 0){
          c_base_player* local_observer_target = global->entity_list->get_entity_handle(localplayer->observer_target());
          if(local_observer_target != nullptr){
            if(local_observer_target == player && localplayer->observer_mode() == 4)
              return false;
          }
        }

        c_player_chams_settings* settings = utils::get_player_chams_settings(player);
        if(settings == nullptr)
          return false;

        if(!settings->render_cloaked_spies && player->is_cloaked())
          return false;

        if(!config->visual.player_chams_include_localplayer && player->get_index() == global->engine->get_local_player())
          return false;

        return settings->enabled;
      }
      else{

        c_world_chams_settings* settings = utils::get_world_chams_settings(entity);
        if(settings == nullptr)
          return false;

        if(config->visual.render_engineer_mode){
          bool hurt = false;
          if(utils::engineer_mode(entity, hurt))
            return hurt;
        }

        return settings->team_enabled;
      }
    }
    else{
      if(entity->is_player()){
        c_base_player* player = (c_base_player*)entity;

        if(!config->visual.player_glow_include_localplayer && player->get_index() == global->engine->get_local_player())
          return false;

        if(config->visual.player_glow_render_steamfriends && (player->is_steam_friend() || player->is_localplayer()))
          return true;

        if(!config->visual.player_glow_render_cloaked_spies && player->is_cloaked())
          return false;

        if(config->visual.player_glow_render_friendlies && player->team() == localplayer->team())
          return true;

        if(config->visual.player_glow_render_enemies && player->team() != localplayer->team())
          return true;
      }
      else{
        if(config->visual.render_engineer_mode){
          bool hurt = false;
          if(utils::engineer_mode(entity, hurt))
            return hurt;
        }

        if(entity->is_ammo_pack() || entity->is_health_pack() || entity->is_halloween_gift_pickup() || entity->is_halloween_pickup() || entity->is_money() || entity->is_lunchable() || entity->is_powerup() || entity->get_spellbook_type() == 1 || entity->get_spellbook_type() == 2 || (entity->is_boss() || entity->is_bomb()) && (entity->team() < 2 || entity->team() > 3)  || entity->is_tank_boss())
          return true;

        if(config->visual.world_glow_render_friendlies && entity->team() == localplayer->team())
          return true;

        if(config->visual.world_glow_render_enemies && entity->team() != localplayer->team())
          return true;
      }
    }
    return false;
  }

  CFLAG_Ofast bool should_render_extra_entity(c_base_entity* entity, u32 object_index, bool glow = false){
    if(entity == nullptr)
      return false;

    if(glow){
      if(!config->visual.glow_object_enabled[object_index])
        return false;

      if(entity->get_spellbook_type() != 0)
        return config->visual.world_glow_ent_spellbooks;
      else if(entity->is_lunchable())
        return config->visual.world_glow_ent_lunchables;
      else if(entity->is_powerup())
        return config->visual.world_glow_ent_powerup;
      else if(entity->is_boss())
        return config->visual.world_glow_ent_boss;
      else if(entity->is_bomb())
        return config->visual.world_glow_ent_bombs;
      else if(entity->is_money())
        return config->visual.world_glow_ent_money;
      else if(entity->is_revive_marker())
        return config->visual.world_glow_ent_revive_marker;
      else if(entity->is_halloween_pickup() || entity->is_halloween_gift_pickup())
        return config->visual.world_glow_ent_halloween_pickup;
      else{
        s_client_class* cc = entity->get_client_class();
        if(cc == nullptr)
          return false;

        switch(cc->id){
          default:{
            if(entity->is_arrow() && config->visual.world_glow_ent_projectile_arrow)
              return true;

            break;
          }
          case CTFProjectile_JarGas:
          case CTFProjectile_Cleaver:
          case CTFProjectile_JarMilk:
          case CTFProjectile_Jar:
          case CTFProjectile_MechanicalArmOrb:
            return config->visual.world_glow_ent_projectile_throwables;
          case CTFGrenadePipebombProjectile:
            return config->visual.world_glow_ent_projectile_sticky && entity->pipebomb_type() == 1 || config->visual.world_glow_ent_projectile_pipebomb && entity->pipebomb_type() != 1;
          case CTFProjectile_Rocket:
          case CTFProjectile_SentryRocket:
          case CTFProjectile_EnergyBall:
            return config->visual.world_glow_ent_projectile_rocket;
          case CTFProjectile_Flare:
            return config->visual.world_glow_ent_projectile_flare;
          case CTFProjectile_SpellKartBats:
          case CTFProjectile_SpellKartOrb:
          case CTFProjectile_SpellLightningOrb:
          case CTFProjectile_SpellMeteorShower:
          case CTFProjectile_SpellMirv:
          case CTFProjectile_SpellPumpkin:
          case CTFProjectile_SpellSpawnHorde:
          case CTFProjectile_SpellSpawnZombie:
          case CTFProjectile_SpellSpawnBoss:
          case CTFProjectile_SpellBats:
          case CTFProjectile_SpellFireball:
            return config->visual.world_glow_ent_projectile_spells;
        }
      }

      return true;
    }
    else{

      c_world_chams_settings* settings = utils::get_world_chams_settings(entity);
      if(settings == nullptr)
        return false;

      if(!settings->object_enabled[object_index])
        return false;

      if(entity->get_spellbook_type() != 0)
        return settings->ent_spellbooks;
      else if(entity->is_lunchable())
        return settings->ent_lunchables;
      else if(entity->is_powerup())
        return settings->ent_powerup;
      else if(entity->is_boss())
        return settings->ent_boss;
      else if(entity->is_bomb())
        return settings->ent_bombs;
      else if(entity->is_money())
        return settings->ent_money;
      else if(entity->is_revive_marker())
        return settings->ent_revive_marker;
      else if(entity->is_halloween_pickup() || entity->is_halloween_gift_pickup())
        return settings->ent_halloween_pickup;
      else{
        s_client_class* cc = entity->get_client_class();
        if(cc == nullptr)
          return false;

        switch(cc->id){
          default:{
            if(entity->is_arrow() && settings->ent_projectile_arrow)
              return true;

            break;
          }
          case CTFProjectile_JarGas:
          case CTFProjectile_Cleaver:
          case CTFProjectile_JarMilk:
          case CTFProjectile_Jar:
          case CTFProjectile_MechanicalArmOrb:
            return settings->ent_projectile_throwables;
          case CTFGrenadePipebombProjectile:
            return settings->ent_projectile_sticky && entity->pipebomb_type() == 1 || settings->ent_projectile_pipebomb && entity->pipebomb_type() != 1;
          case CTFProjectile_Rocket:
          case CTFProjectile_SentryRocket:
          case CTFProjectile_EnergyBall:
            return settings->ent_projectile_rocket;
          case CTFProjectile_Flare:
            return settings->ent_projectile_flare;
          case CTFProjectile_SpellKartBats:
          case CTFProjectile_SpellKartOrb:
          case CTFProjectile_SpellLightningOrb:
          case CTFProjectile_SpellMeteorShower:
          case CTFProjectile_SpellMirv:
          case CTFProjectile_SpellPumpkin:
          case CTFProjectile_SpellSpawnHorde:
          case CTFProjectile_SpellSpawnZombie:
          case CTFProjectile_SpellSpawnBoss:
          case CTFProjectile_SpellBats:
          case CTFProjectile_SpellFireball:
            return settings->ent_projectile_spells;
        }
      }

      return true;
    }

    return false;
  }

  CFLAG_Ofast chams_type get_chams_type(c_base_entity* entity){
    if(entity == nullptr)
      return CHAMS_TYPE_INVALID;

    if(!entity->should_draw())
      return CHAMS_TYPE_INVALID;
    
    s_client_class* cc = entity->get_client_class();
    if(cc == nullptr)
      return CHAMS_TYPE_INVALID;

    if(!is_in_view_plane(entity->obb_center()))
      return CHAMS_TYPE_INVALID;

    if(cc->id == CTFPlayer){
      c_base_player* player = (c_base_player*)entity;
      if(player->is_alive()){
        u32 flags = 0;
        if(should_render_team(entity, false))
          flags |= CHAMS_TYPE_PLAYER;

        if((config->visual.player_glow_enabled) && should_render_team(entity, true))
          flags |= CHAMS_TYPE_GLOW;

        if(flags)
          return flags | CHAMS_TYPE_DRAW_CHILDREN;
      }
    }
    else{
      u32 object_index = world_esp->get_object_index(entity);
      if(object_index == OBJECT_TYPE_INVALID)
        return CHAMS_TYPE_INVALID;

      // Fixes the sentry's range being all epic on the screen.
      if(entity->is_sentry()){
        if(entity->placing())
          return CHAMS_TYPE_INVALID;
      }
      else if(cc->id == CCaptureFlag){
        if(entity->flag_status() == 1)
          return CHAMS_TYPE_INVALID;
      }

      u32 flags = 0;
      if(should_render_extra_entity(entity, object_index, false) && should_render_team(entity, false))
        flags |= CHAMS_TYPE_ENTITY;

      if(should_render_extra_entity(entity, object_index, true) && should_render_team(entity, true) && (config->visual.world_glow_enabled))
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

  CFLAG_Ofast bool handle_base_material(s_chams_context& ctx){
    if(ctx.entity == nullptr)
      return false;

    bool ignore_z = get_ignore_z_for_entity(&ctx, false);

    c_material* wish_material = get_material_for_entity(&ctx, false);
    if(wish_material == nullptr){
      // If the entity has a render mode of 4 and clr render alpha of 254
      // Presume they they had the alpha hack for fixing render ordering enabled.
      // Restore them to the original state that they should have been in
      if(ctx.entity->render_mode() == 4 && ctx.entity->clr_render_a() == 254)
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

    u32 object_index = world_esp->get_object_index(ctx.entity);
    colour draw_col  = world_esp->get_draw_colour(ctx.entity, object_index, WORLD_ESP_RENDER_TYPE_CHAMS);

    if(ctx.entity != nullptr){
      if(ctx.entity->is_player())
        draw_col = player_esp->get_draw_colour(ctx.entity, PLAYER_ESP_RENDER_TYPE_CHAMS);
    }

    global->render_view->set_colour_modulation(draw_col);

    set_alpha(ctx.entity, (float)draw_col.w / 255.f, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);
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

  CFLAG_Ofast bool handle_overlay_material(s_chams_context& ctx){
    if(ctx.entity == nullptr)
      return false;

    bool ignore_z = get_ignore_z_for_entity(&ctx, true);

    c_material* wish_material = get_material_for_entity(&ctx, true);
    if(wish_material == nullptr)
      return false;

    u32 object_index = world_esp->get_object_index(ctx.entity);
    colour draw_col  = world_esp->get_draw_colour(ctx.entity, object_index, WORLD_ESP_RENDER_TYPE_CHAMS_OVERLAY);

    if(ctx.entity != nullptr){
      if(ctx.entity->is_player())
        draw_col = player_esp->get_draw_colour(ctx.entity, PLAYER_ESP_RENDER_TYPE_CHAMS_OVERLAY);
    }

    global->render_view->set_colour_modulation(draw_col);

    set_alpha(ctx.entity, (float)draw_col.w / 255.f, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);
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

  CFLAG_Ofast bool handle_backtrack_chams(c_render_context* rc, c_base_entity* entity, bool glow = false){
    if(entity == nullptr)
      return false;

    c_player_chams_settings* cham_settings = utils::get_player_chams_settings(WORLD_ESP_SETTING_TYPE_BACKTRACK);
    if(cham_settings == nullptr)
      return false;

    if(!config->visual.backtrack_glow_enabled && glow)
      return false;

    c_base_player* localplayer = utils::localplayer();
    if(localplayer == nullptr)
      return false;

    c_base_player* player = entity->get_player();
    if(player == nullptr)
      return false;

    bool invisible_color = false;
    if(player->is_cloaked()){
      if(glow && !config->visual.backtrack_glow_render_cloaked_spies)
        return false;

      if(!glow && !cham_settings->render_cloaked_spies)
        invisible_color = true;
    }

    c_player_data* player_data = player->data();
    if(player_data == nullptr)
      return false;

    if(!localplayer->is_considered_enemy(entity))
      return false;

    c_base_weapon* weapon = localplayer->get_weapon();
    if(weapon == nullptr)
      return false;

    if(!weapon->is_flamethrower()){
      if(weapon->is_projectile_weapon() || weapon->is_harmless_item() || weapon->is_wrangler())
        return false;
    }

    if(weapon->is_phlog())
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

    colour draw_col = flt_array2clr(config->visual.backtrack_chams_colour);
    if(glow)
      draw_col = flt_array2clr(config->visual.backtrack_glow_colour); 
    else{
      if(!cham_settings->enabled || invisible_color)
        draw_col[3] = 0;
    }

    col_frame.push();
    {
      // Custom alpha  Will respect the alpha decay on distance and velocity.
      float alpha = (float)draw_col[3] / 255.f;
  
      c_material* base_material    = get_material_from_player_settings(cham_settings, false);
      c_material* overlay_material = get_material_from_player_settings(cham_settings, true);
      
      // calculate alpha for entity
      {
        float h  = math::abs(found_record->velocity.length());
              h += math::abs(entity->velocity().length());
              h += entity->origin().distance(found_record->origin);
              h  = math::clamp(h, 0.f, 200.f);
  
        if(h <= 1.f)
          return false;
  
        if(h < 200.f)
          set_alpha(entity, math::clamp(math::clamp(h / 200.f, 0.f, 1.f), 0.f, alpha), false);
        else
          set_alpha(entity, alpha, false);
      }

      if(base_material != nullptr || glow)
        global->render_view->set_colour_modulation(draw_col);
      else if(base_material == nullptr)
        global->render_view->set_colour_modulation(rgb(255, 255, 255));

      // A little 'hack' to override the matrix.
      player_data->override_dme_matrix = true;
      player_data->dme_matrix          = (void*)&found_record->bone_matrix[0];

      if(!glow)
        force_material_override(base_material);

      // Base.
      override_cloak = true;
      force_viewport_level(FORCE_LEVEL_CHAMS, rc, &cham_settings->ignore_z);
      draw_model(entity, 0x1, false);
      force_viewport_level(FORCE_LEVEL_NONE, rc, &cham_settings->ignore_z);
      override_cloak = false;

      // Overlay.
      if(!glow && overlay_material != nullptr && cham_settings->enabled){

        draw_col = flt_array2clr(config->visual.backtrack_chams_overlay_colour);

        // Custom alpha  Will respect the alpha decay on distance and velocity.
        alpha = (float)draw_col[3] / 255.f;
        {  
          // calculate alpha for entity
          {
            float h  = math::abs(found_record->velocity.length());
                  h += math::abs(entity->velocity().length());
                  h += entity->origin().distance(found_record->origin);
                  h  = math::clamp(h, 0.f, 200.f);
      
            if(h <= 1.f)
              return false;
      
            if(h < 200.f)
              set_alpha(entity, math::clamp(math::clamp(h / 200.f, 0.f, 1.f), 0.f, alpha), false);
            else
              set_alpha(entity, alpha, false);
          }
        }

        global->render_view->set_colour_modulation(flt_array2clr(config->visual.backtrack_chams_overlay_colour));
        force_material_override(overlay_material);

        // A little 'hack' to override the matrix.
        player_data->override_dme_matrix = true;
        player_data->dme_matrix          = (void*)&found_record->bone_matrix[0];

        override_cloak = true;
        force_viewport_level(FORCE_LEVEL_CHAMS, rc, &cham_settings->ignore_z);
        draw_model(entity, 0x1, false);
        force_viewport_level(FORCE_LEVEL_NONE, rc, &cham_settings->ignore_z);
        override_cloak = false;
      }

      player_data->override_dme_matrix = false;
      player_data->dme_matrix          = nullptr;
    }
    col_frame.pop();

    return true;
  }

  CFLAG_Ofast bool handle_fake_angle_chams(c_render_context* rc, c_base_entity* entity, bool glow = false){
    if(entity == nullptr)
      return false;

    if(entity->get_index() != global->localplayer_index)
      return false;

    c_player_chams_settings* cham_settings = utils::get_player_chams_settings(WORLD_ESP_SETTING_TYPE_FAKEANGLE);
    if(cham_settings == nullptr)
      return false;

    if(!config->visual.fake_angle_glow_enabled && glow)
      return false;

    if(!antiaim->is_faking_yaw())
      return false;

    if(!utils::is_in_thirdperson())
      return false;

    if(global->engine->is_playing_demo())
      return false;

    c_player_data* player_data = utils::get_player_data(entity->get_index());
    if(player_data == nullptr)
      return false;

    colour draw_col = flt_array2clr(config->visual.fake_angle_chams_colour);
    if(glow)
      draw_col = flt_array2clr(config->visual.fake_angle_glow_colour);
    else{
      if(!cham_settings->enabled)
        draw_col[3] = 0;
    }

    vec3 fake_angle;
    if(!antiaim->get_fake_angle(fake_angle))
      return false;

    static matrix3x4 bone_matrix[128];
    if(!utils::create_custom_matrix(entity, fake_angle, (matrix3x4*)&bone_matrix))
      return false;

    col_frame.push();
    {
      // Custom alpha  Will respect the alpha decay on distance and velocity.
      float alpha = (float)draw_col[3] / 255.f;

      c_material* base_material    = get_material_from_player_settings(cham_settings, false);
      c_material* overlay_material = get_material_from_player_settings(cham_settings, true);
      
      if(glow || base_material != nullptr)
        global->render_view->set_colour_modulation(draw_col);
      else if(base_material == nullptr)
        global->render_view->set_colour_modulation(rgb(255, 255, 255));

      // A little 'hack' to override the matrix.
      player_data->override_dme_matrix = true;
      player_data->dme_matrix          = (void*)&bone_matrix[0];

      if(!glow)
        force_material_override(base_material);

      // Base.
      set_alpha(entity, alpha, false);
      force_viewport_level(FORCE_LEVEL_CHAMS, rc, &cham_settings->ignore_z);
      draw_model(entity, 0x1, false);
      force_viewport_level(FORCE_LEVEL_NONE, rc, &cham_settings->ignore_z);

      // Overlay.
      if(!glow && overlay_material != nullptr && cham_settings->enabled){

        draw_col = flt_array2clr(config->visual.fake_angle_chams_overlay_colour);

        // Custom alpha  Will respect the alpha decay on distance and velocity.
        alpha = (float)draw_col[3] / 255.f;

        set_alpha(entity, alpha, false);
        global->render_view->set_colour_modulation(draw_col);
        force_material_override(overlay_material);

        // A little 'hack' to override the matrix.
        player_data->override_dme_matrix = true;
        player_data->dme_matrix          = (void*)&bone_matrix[0];

        force_viewport_level(FORCE_LEVEL_CHAMS, rc, &cham_settings->ignore_z);
        draw_model(entity, 0x1, false);
        force_viewport_level(FORCE_LEVEL_NONE, rc, &cham_settings->ignore_z);
      }

      player_data->override_dme_matrix = false;
      player_data->dme_matrix          = nullptr;
    }
    col_frame.pop();

    // utils::create_custom_matrix calls setup bones, so we need to call it again to invalidate bone cache and undo the fake angle adjustment we did.
    matrix3x4 tmp[128];
    entity->setup_bones(tmp, true, true);

    return true;
  }

  CFLAG_Ofast bool handle_hand_chams(c_base_entity* entity, void* rcx, void* state, s_model_render_data* info, matrix3x4* matrix_out){
    if(!config->visual.hand_chams_enabled)
      return false;

    if(entity == nullptr)
      return false;

    if(!is_viewmodel_entity(entity, false, false))
      return false;

    if(config->visual.hand_chams_removal)
      return true;

    colour old_colour = global->render_view->get_colour_modulation();
    float  old_blend  = global->render_view->get_blend();

    c_material* base_material    = nullptr;
    c_material* overlay_material = nullptr;
    {
      if(!config->visual.hand_chams_base_material_none){
        if(config->visual.hand_chams_base_material_shaded)
          base_material = material_base_default;
        else if(config->visual.hand_chams_base_material_flat)
          base_material = material_base_flat;
        else if(config->visual.hand_chams_base_material_selfillum)
          base_material = material_base_selfillum;
        else if(config->visual.hand_chams_base_material_fresnel)
          base_material = material_fresnel_overlay;
        else if(config->visual.hand_chams_base_material_wireframe)
          base_material = material_wireframe_overlay;
      }

      if(!config->visual.hand_chams_overlay_material_none){
        if(config->visual.hand_chams_overlay_material_shaded)
          overlay_material = material_base_default;
        else if(config->visual.hand_chams_overlay_material_flat)
          overlay_material = material_base_flat;
        else if(config->visual.hand_chams_overlay_material_selfillum)
          overlay_material = material_base_selfillum;
        else if(config->visual.hand_chams_overlay_material_fresnel)
          overlay_material = material_fresnel_overlay;
        else if(config->visual.hand_chams_overlay_material_wireframe)
          overlay_material = material_wireframe_overlay;
      }
    }

    if(base_material != nullptr){
      float alpha = (float)config->visual.hand_chams_base_material_colour[3] / 255.f;
      set_alpha(entity, alpha, false);
      global->render_view->set_colour_modulation(flt_array2clr(config->visual.hand_chams_base_material_colour));
      force_material_override(base_material);
    }

    // Always call.
    utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, matrix_out);

    if(overlay_material != nullptr){
      float alpha = (float)config->visual.hand_chams_overlay_material_colour[3] / 255.f;
      set_alpha(entity, alpha, false);
      global->render_view->set_colour_modulation(flt_array2clr(config->visual.hand_chams_overlay_material_colour));
      force_material_override(overlay_material);
      utils::call_fastcall64<void, void*, s_model_render_data*, matrix3x4*>(gen_internal->decrypt_asset(global->draw_model_execute_hook_trp), rcx, state, info, matrix_out);
    }

    force_material_override(nullptr);
    global->render_view->set_colour_modulation(old_colour);
    set_alpha(entity, old_blend, false);

    return true;
  }

  CFLAG_Ofast bool handle_weapon_viewmodel_chams(c_base_entity* entity, c_studio_render_context* studio_render_context, void* results, s_draw_model_info* info, matrix3x4* bone_to_world, float* flex_weights, float* flex_delayed_weights, vec3* origin, i32 flags){
    if(!config->visual.weapon_chams_enabled)
      return false;

    if(entity == nullptr)
      return false;

    if(!is_viewmodel_entity(entity, true, true))
      return false;

    colour old_colour = global->render_view->get_colour_modulation();
    float old_blend  = studio_render_context->alpha;

    c_material* base_material    = nullptr;
    c_material* overlay_material = nullptr;
    {
      if(!config->visual.weapon_chams_base_material_none){
        if(config->visual.weapon_chams_base_material_shaded)
          base_material = material_base_default;
        else if(config->visual.weapon_chams_base_material_flat)
          base_material = material_base_flat;
        else if(config->visual.weapon_chams_base_material_selfillum)
          base_material = material_base_selfillum;
        else if(config->visual.weapon_chams_base_material_fresnel)
          base_material = material_fresnel_overlay;
        else if(config->visual.weapon_chams_base_material_wireframe)
          base_material = material_wireframe_overlay;
      }

      if(!config->visual.weapon_chams_overlay_material_none){
        if(config->visual.weapon_chams_overlay_material_shaded)
          overlay_material = material_base_default;
        else if(config->visual.weapon_chams_overlay_material_flat)
          overlay_material = material_base_flat;
        else if(config->visual.weapon_chams_overlay_material_selfillum)
          overlay_material = material_base_selfillum;
        else if(config->visual.weapon_chams_overlay_material_fresnel)
          overlay_material = material_fresnel_overlay;
        else if(config->visual.weapon_chams_overlay_material_wireframe)
          overlay_material = material_wireframe_overlay;
      }
    }

    if(base_material != nullptr){
      studio_render_context->alpha = (float)config->visual.weapon_chams_base_material_colour[3] / 255.f;
      studio_render_context->set_colour_modulation(flt_array2clr(config->visual.weapon_chams_base_material_colour));
      force_material_override(base_material);
    }

    utils::call_fastcall64<void, void*, s_draw_model_info*, matrix3x4*, float*, float*, vec3*, i32>(gen_internal->decrypt_asset(global->studio_render_context_drawmodel_hook_trp), studio_render_context, results, info, bone_to_world, flex_weights, flex_delayed_weights, origin, flags);

    if(overlay_material != nullptr){
      studio_render_context->alpha = (float)config->visual.weapon_chams_overlay_material_colour[3] / 255.f;
      studio_render_context->set_colour_modulation(flt_array2clr(config->visual.weapon_chams_overlay_material_colour));
      force_material_override(overlay_material);
      utils::call_fastcall64<void, void*, s_draw_model_info*, matrix3x4*, float*, float*, vec3*, i32>(gen_internal->decrypt_asset(global->studio_render_context_drawmodel_hook_trp), studio_render_context, results, info, bone_to_world, flex_weights, flex_delayed_weights, origin, flags);
    }

    force_material_override(nullptr);
    studio_render_context->set_colour_modulation(old_colour);
    studio_render_context->alpha = old_blend;

    return true;
  }

  CFLAG_Ofast void run_chams(c_render_context* rc){
    assert(rc != nullptr);

    running_chams = true;
    for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
      c_base_player* entity = global->entity_list->get_entity(i);

      if(entity == nullptr || entity->is_dormant())
        continue;

      if(i == global->localplayer_index)
        handle_fake_angle_chams(rc, entity);
      
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
     
      // Invisible Chams:
      {

        handle_backtrack_chams(rc, entity); // Called it up here so the depth of the real model isn't fucked.

        handle_base_material(ctx);
        handle_overlay_material(ctx);
      }

      // Visible Chams. (Do not mess with depth)
      // Bug, maybe? Idk: Fix it or I disable it.
      // The visible and invisible models are visible.
      if(false){
        global->render_view->set_colour_modulation(rgb(255, 64, 32));
        global->render_view->set_blend(0.2f);
        force_material_override(material_base_flat); 
  
        draw_model(ctx.entity, 0x00000001/*STUDIO_RENDER*/, ctx.type & CHAMS_TYPE_DRAW_CHILDREN);
  
        force_material_override(nullptr);
        global->render_view->set_blend(1.f);
        global->render_view->set_colour_modulation(rgb(255, 255, 255));
      }
    }
    running_chams = false;
  }

  CFLAG_Ofast void run_glow_start(c_render_context* rc, s_stencil_state& stencil_state){
    assert(rc != nullptr);

    is_rendering_glow = false;

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

  CFLAG_Ofast void run_glow_finish(c_render_context* rc, s_stencil_state& stencil_state){
    if(!stencil_state.enable)
      return;

    is_rendering_glow = true;

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


      bool needs_re_render = !has_rendered_this_frame(entity);
      if(!needs_re_render){
        if(entity->is_player()){
          if(entity->is_cloaked(false))
            needs_re_render = true;
        }
      }

      // if the entity hasnt been rendered this frame, we need to do it manually.
      if(needs_re_render){
        float old_blend = global->render_view->get_blend();
        global->render_view->set_blend(0.f);
        global->render_view->set_colour_modulation(rgb(255, 255, 255));
        force_material_override(nullptr);
        draw_model(entity, 0x00000001/*STUDIO_RENDER*/, type & CHAMS_TYPE_DRAW_CHILDREN);
        global->render_view->set_blend(old_blend);
      }
    }

    stencil_state.enable = false;
    rc->set_stencil_state(&stencil_state);

    force_material_override(material_glow_colour);

    rc->push_render_target_and_viewport();
    {
      rc->set_render_target(texture_glow_buffer1);
      rc->viewport(0, 0, screen_size.x, screen_size.y);
      rc->clear_colour_4ub(0, 0, 0, 0);
      rc->clear_buffers(true, false, false);

      for(i32 i = 1; i <= global->entity_list->get_highest_index(); i++){
        c_base_player* entity = global->entity_list->get_entity(i);

        if(entity == nullptr || entity->is_dormant())
          continue;

        if(i == global->localplayer_index)
          handle_fake_angle_chams(rc, entity, true);

        chams_type type = get_chams_type(entity);

        if(!(type & CHAMS_TYPE_GLOW))
          continue;

        handle_backtrack_chams(rc, entity, true);

        if(entity->is_player()){
          global->render_view->set_colour_modulation(player_esp->get_draw_colour(entity, PLAYER_ESP_RENDER_TYPE_GLOW));
        }
        else{
          u32 object_index = world_esp->get_object_index(entity);
          global->render_view->set_colour_modulation(world_esp->get_draw_colour(entity, object_index, WORLD_ESP_RENDER_TYPE_GLOW));
        }

        draw_model(entity, 0x00000001/*STUDIO_RENDER*/ | 0x00000080/*STUDIO_NOSHADOWS*/, type & CHAMS_TYPE_DRAW_CHILDREN);
      }
    }
    rc->pop_render_target_and_viewport();

    // blur
    if(config->visual.glow_blur){
      rc->push_render_target_and_viewport();
      {
        rc->viewport(0, 0, screen_size.x, screen_size.y);
        rc->set_render_target(texture_glow_buffer2);
        rc->draw_screen_space_rectangle(material_glow_blur_x, 0, 0, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - 1, screen_size.y - 1, screen_size.x, screen_size.y);
        rc->set_render_target(texture_glow_buffer1);
        rc->draw_screen_space_rectangle(material_glow_blur_y, 0, 0, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - 1, screen_size.y - 1, screen_size.x, screen_size.y);
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
      rc->draw_screen_space_rectangle(material_halo_add, 0, 0, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - 1, screen_size.y - 1, screen_size.x, screen_size.y);
    else{ // stencil
      rc->draw_screen_space_rectangle(material_halo_add, -config->visual.glow_thickness, -config->visual.glow_thickness, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, -config->visual.glow_thickness, 0, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, 0, -config->visual.glow_thickness, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, 0, config->visual.glow_thickness, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, config->visual.glow_thickness, config->visual.glow_thickness, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, config->visual.glow_thickness, 0, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, config->visual.glow_thickness, -config->visual.glow_thickness, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
      rc->draw_screen_space_rectangle(material_halo_add, -config->visual.glow_thickness, config->visual.glow_thickness, screen_size.x, screen_size.y, 0.f, 0.f, screen_size.x - config->visual.glow_thickness, screen_size.y - config->visual.glow_thickness, screen_size.x, screen_size.y);
    }

    stencil_state.enable = false;
    rc->set_stencil_state(&stencil_state);
  }

  CFLAG_Ofast bool is_viewmodel_entity(c_base_entity* entity, bool include_weapon = false, bool ignore_hands = false){
    if(entity == nullptr)
      return false;

    s_client_class* cc = entity->get_client_class();

    if(cc == nullptr)
      return false;

    if(cc->id == CBaseAnimating && include_weapon){
      c_base_entity* parent = entity->get_owner();

      // if it has a parent, check if its the weapon we are holding
      if(parent != nullptr){
        c_base_player* localplayer = utils::localplayer();

        if(localplayer != nullptr && localplayer->get_weapon() == (c_base_weapon*)parent)
          return true;
      }
    }

    return cc->id == CTFViewModel && !ignore_hands;
  }

  void cleanup(s_chams_context& ctx){
    global->render_view->set_blend(1.f);

    // reset viewport level
    if(ctx.viewport_level != 0)
      force_viewport_level(FORCE_LEVEL_NONE, &ctx);

    // restore material override
    if(ctx.applied_mat != nullptr)
      force_material_override(nullptr);
  }

  CFLAG_Ofast bool is_chams_material(c_material* material){
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

  bool is_door_model(void* model){
    if(model == nullptr)
      return false;

    i8* model_name = global->model_info->get_model_name(model);
    if(model_name == nullptr)
      return false;

    u32 hash = HASH_RT(model_name);
    switch(hash){
      default: break;
      case HASH("models/props_gameplay/door_slide_door.mdl"):
      case HASH("models/props_gameplay/door_slide_large_door.mdl"):
        return true;
    }

    return false;
  }

  CFLAG_Ofast void force_material_override(c_material* material){
    disallow_materials = false;
    global->engine_model->force_material_override(material);
    disallow_materials = material != nullptr;
  }

  bool running            = false;
  bool running_chams      = false;
  bool disallow_materials = false;
};

CLASS_EXTERN(c_chams, chams);