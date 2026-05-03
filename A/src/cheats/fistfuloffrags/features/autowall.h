#pragma once

struct s_autowall_trace_dump{
  bool            dumped = false;
  float           damage;
  i32             hitgroup;
  c_base_player*  hit_entity;
};

class c_autowall{
public:
  bool                  is_simulating = false;
  s_autowall_trace_dump dump;

  bool simulate(vec3 start, vec3 end){
    c_base_player* localplayer = utils::localplayer();

    if(localplayer == nullptr)
      return false;

    c_base_weapon* weapon = localplayer->get_weapon();

    if(weapon == nullptr)
      return false;

    css_weapon_file_info* wpn_info = weapon->get_weapon_info();
    if(wpn_info == nullptr)
      return false;

    vec3  angles         = math::calc_view_angle(start, end);
    float range          = wpn_info->range;
    i32   penetration    = wpn_info->penetration;
    i32   bullet_type    = wpn_info->ammo_type;
    i32   damage         = wpn_info->damage;
    float range_modifier = wpn_info->range_modifier;

    // TODO look at FX_FireBullets and see how they reduce weapon damage and the range mod for certain fire types.

    memset(this, 0, sizeof(*this));
    is_simulating       = true;
    {
      // Call entity->FireBullet()
      utils::call_thiscall<void, vec3, vec3&, float, i32, i32, i32, float, c_base_player*, bool, float, float>(global->fire_bullet, localplayer, start, angles, range, penetration, bullet_type, damage, range_modifier, nullptr, true, 0.f, 0.f);
    }
    is_simulating = false;

    // Invalid dump?
    if(!dump.dumped || dump.hit_entity == nullptr)
      return false;

    switch(dump.hitgroup){
      default: break;
      case 1: // HEAD
      {
        dump.damage *= 4.f;
        break;
      }
      case 3: // CHEST
      {
        dump.damage *= 1.25f;
        break;
      }
      case 6: // LEGS
      case 7:
      {
        dump.damage *= 0.75f;
        break;
      }

    }
    // Got this from CCSPlayer::OnTakeDamage
    if(localplayer->team == dump.hit_entity->team)
      dump.damage *= 0.35f;

    return dump.damage >= 1.f;
  }
};

CLASS_EXTERN(c_autowall, autowall);