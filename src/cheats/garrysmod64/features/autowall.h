#pragma once

struct s_autowall_trace_dump{
  bool            dumped = false;
  float           damage;
  i32             penetrations;
  c_base_player*  hit_entity;
};

class c_autowall{
public:
  bool                  is_simulating = false;
  s_autowall_trace_dump dump;

  float m9k_get_penetration_value(i32 ammo_id){
    switch ( ammo_id ) {
      case 14 : // SniperPenetratedRound
        return 20.f;
      case 3 : // pistol
        return 9.f;
      case 5 : // 357
        return 12.f;
      case 4 : // smg1
        return 14.f;
      case 1 : // ar2
        return 16.f;
      case 7 : // buckshot
        return 5.f;
      case 20 : // AirboatGun
        return 17.f;
      default :
        break;
    }

    return -1;
  }

  i32 m9k_get_bounce(i32 ammo_id){
    switch (ammo_id){
      case 14 : // SniperPenetratedRound
        return 10;
      case 3 : // pistol
        return 2;
      case 5 : // 357
        return 5;
      case 4 : // smg1
        return 4;
      case 1 : // ar2
        return 5;
      case 7 : // buckshot
        return 0;
      case 20 : // AirboatGun
        return 8;
      default :
        break;
    }

    return -1;
  }

  bool m9k_is_weak_material(u8 mat){
    return mat == CHAR_TEX_GLASS || mat == CHAR_TEX_PLASTIC || mat == CHAR_TEX_WOOD || mat == CHAR_TEX_FLESH || mat == CHAR_TEX_ALIENFLESH;
  }

  bool m9k_bullet_simulate(c_base_player* target, vec3 start, vec3 end, i32 ammo_type, float pen_val, i32 max_bounces, float& dmg_base, i32 bounce_count = 0){
    if(bounce_count > max_bounces)
      return false;

    s_trace start_trace = global->trace->ray(start, end, mask_shot, TRACE_EVERYTHING, target);
    //render_debug->draw_line(start_trace.start, start_trace.end, colour(255, 0, 0, 255), true, globaldata->interval_per_tick * 2.f);

    if(start_trace.hit_entity == target || start_trace.fraction >= 1.f)
      return true;

    surface_data* surface = global->physics_surface->get_surface_data( start_trace.surface.props );

    if(surface == nullptr)
      return false;

    u16 hit_mat = surface->game.material;

    float damage_mod = 0.5f;
    {
      if(ammo_type == 14)
        damage_mod = 1.f;
      else if(hit_mat == CHAR_TEX_CONCRETE || hit_mat == CHAR_TEX_METAL)
        damage_mod = 0.3f;
      else if (hit_mat == CHAR_TEX_WOOD || hit_mat == CHAR_TEX_PLASTIC || hit_mat == CHAR_TEX_GLASS)
        damage_mod = 0.8f;
      else if (hit_mat == CHAR_TEX_FLESH || hit_mat == CHAR_TEX_ALIENFLESH)
        damage_mod = 0.9f;
    }
    dmg_base *= damage_mod;
    if(dmg_base <= 10.f)
      return false;

    vec3 fwd = (end - start);
    fwd.normalize();

    s_trace mid_trace = global->trace->ray(start_trace.end + (fwd * pen_val), start_trace.end, mask_shot, TRACE_EVERYTHING, target);
    //render_debug->draw_line(start_trace.end + (fwd * pen_val), start_trace.end, colour(0, 255, 0, 255), true, globaldata->interval_per_tick * 2.f);

    if(mid_trace.started_solid)
      return false;

    float delta = (mid_trace.end - start_trace.end).length();

    if(delta > pen_val)
      return false;

    if(hit_mat == CHAR_TEX_GLASS)
      bounce_count--;

    return m9k_bullet_simulate(target, mid_trace.end, end, ammo_type, pen_val, max_bounces, dmg_base, ++bounce_count);
  }

  bool m9k_simulate_penetration(c_base_player* target, vec3 start, vec3 end, float* dmg_out = nullptr){
    if(dmg_out == nullptr)
      return false;

    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    c_base_weapon* wep = localplayer->get_weapon();

    if(wep == nullptr)
      return false;

    u8 wep_type = wep->scripted_type();

    if(wep_type != weapon_type_m9k)
      return false;

    i32   ammo_type  = wep->get_ammo_type();
    float pen_val    = m9k_get_penetration_value(ammo_type);
    i32   max_bounce = m9k_get_bounce(ammo_type);
    float dmg_base   = std::max(global->bullets_info_dump.damage * math::random_float(0.85f, 1.3f), 10.f);
    //DBG("dmg_base: %f\n", dmg_base);

    if(pen_val <= 0.f || max_bounce <= 0)
      return false;

    s_trace start_trace = global->trace->ray(start, end, mask_shot, TRACE_EVERYTHING, target);
    vec3    trace_start = start_trace.end;

    if(start_trace.hit_entity == target || start_trace.fraction >= 1.f){
      *dmg_out = dmg_base;
      return true;
    }

    surface_data* surface = global->physics_surface->get_surface_data(start_trace.surface.props);

    if(surface == nullptr)
      return false;

    u16   start_hit_mat = surface->game.material;
    bool  is_weak_mat   = m9k_is_weak_material( start_hit_mat );

    bool ret = m9k_bullet_simulate(target, start, end, ammo_type, pen_val, max_bounce, dmg_base, 0);

    *dmg_out = dmg_base;

    return ret;
  }
};

CLASS_EXTERN(c_autowall, autowall);