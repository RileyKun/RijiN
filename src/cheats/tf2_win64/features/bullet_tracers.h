#pragma once

struct s_beam_entry{
  vec3    start;
  vec3    end;
  colour  col;
  float   life;
};

class c_bullet_tracers{
public:
  std::vector<s_beam_entry> queue;

  void frame_stage_notify_run(u32 stage){
    if(stage != FRAME_RENDER_START)
      return;

    if(!global->valid())
      return;

    if(queue.empty())
      return;

    static s_beam_info beam_info;

    // Allocate a static data pointer and put our xor string in there
    if(beam_info.model_name == nullptr){
      static i8 model_name[21];
      beam_info.model_name = model_name;
      strcpy(beam_info.model_name, XOR("sprites/physbeam.vmt"));
    }

    for(u32 i = 0; i < queue.size(); i++){
      s_beam_entry entry = queue[i];

      //sprites/white.vmt
      beam_info.start         = entry.start;
      beam_info.end           = entry.end;
      beam_info.model_index   = -1;
      beam_info.halo_scale    = 0.f;
      beam_info.halo_index    = -1;
      beam_info.life          = entry.life;
      beam_info.width         = 2.f;
      beam_info.end_width     = 2.f;
      beam_info.fade_length   = 0.f;
      beam_info.amplitude     = 0.f;
      beam_info.brightness    = 255.f;
      beam_info.speed         = -0.5f;
      beam_info.start_frame   = 0;
      beam_info.frame_rate    = 0.f;
      beam_info.segments      = 2;
      beam_info.renderable    = true;
      beam_info.flags         = 0;
      beam_info.red           = entry.col.r;
      beam_info.green         = entry.col.g;
      beam_info.blue          = entry.col.b;

      void* handle = global->view_render_beams->create_beam_points(&beam_info);

      if(handle != nullptr)
        global->view_render_beams->draw_beam(handle);

      queue.erase(queue.begin() + i);
    }
  }

  void add(vec3 start, vec3 end, colour col, float life){
    s_beam_entry entry;
    {
      entry.start  = start;
      entry.end    = end;
      entry.col    = col;
      entry.life   = life;
    }
    queue.push_back(entry);
  }

  i8* get_particle_effect(u8 team){
    static i8 particle_effect[64];

    if(config->visual.tracer_raygun_effect){
      strcpy(particle_effect, team == 2 ? XOR("bullet_tracer_raygun_red") : XOR("bullet_tracer_raygun_blue"));
      return particle_effect;
    }
    else if(config->visual.tracer_machina_effect ){
      strcpy(particle_effect, team == 2 ? XOR("dxhr_sniper_rail_red") : XOR("dxhr_sniper_rail_blue"));
      return particle_effect;
    }
    else if(config->visual.tracer_lightning_effect){
      strcpy(particle_effect, team == 2 ? XOR("spell_lightningball_hit_red") : XOR("spell_lightningball_hit_blue"));
      return particle_effect;
    }
    else if(config->visual.tracer_bignasty_effect){
      strcpy(particle_effect, team == 2 ? XOR("bullet_bignasty_tracer01_red") : XOR("bullet_bignasty_tracer01_blue"));
      return particle_effect;
    }
    else if(config->visual.tracer_meramus_effect){
      strcpy(particle_effect, XOR("merasmus_zap"));
      return particle_effect;
    }
    else if(config->visual.tracer_distortion_effect){
      strcpy(particle_effect, XOR("tfc_sniper_distortion_trail"));
      return particle_effect;
    }

    return nullptr;
  }
};

CLASS_EXTERN(c_bullet_tracers, bullet_tracers);