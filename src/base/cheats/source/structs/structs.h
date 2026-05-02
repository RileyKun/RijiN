#pragma once

// Should keep this to a global scale only, in the event something here is outdated.
// You can use a marco or find way to override the class / struct.


class c_sfx_table;
class c_interface_steamfriends;
class c_interface_steamuser;

class c_internal_base_entity;
class c_internal_base_player;

typedef void*(*init_interface_fn)();
struct s_interface_list{
  void*             init_interface;
  i8*               name;
  s_interface_list* next;
};

#if defined(SOURCE_2018)
struct s_renderable_instance{
  u8 alpha;
};
#endif

class s_client_state{
public:
  ALWAYSINLINE i32& server_tick(){
    gen_read(i32, "client_state_server_tick");
  }

  ALWAYSINLINE i32& client_tick(){
    gen_read(i32, "client_state_client_tick");
  }

  ALWAYSINLINE i32& signon_state(){
    gen_read(i32, "client_state_signon_state");
  }

  ALWAYSINLINE i32& last_command_ack(){
    gen_read(i32, "client_state_last_command_ack");
  }

  ALWAYSINLINE i32& last_outgoing_command(){
    gen_read(i32, "client_state_last_outgoing_command");
  }

  ALWAYSINLINE i32& choked_commands(){
    gen_read(i32, "client_state_choked_commands");
  }
};

// If you're looking at this and the hook on the input::get_user_cmd is crashing.
// Its because the size does not match here.
class c_user_cmd{
public:
  #if !defined(GMOD_CHEAT)
  void* rtti;
  #endif

  i32   command_number;
  i32   tick_count;
  vec3  view_angles;
  vec3  move;
  i32   buttons;
  u8    impulse;
  i32   weapon_select;
  i32   weapon_subtype;
  i32   random_seed;
  i16   mouse_x;
  i16   mouse_y;
  bool  predicted;

  #if defined(TF2_CHEAT)
    PAD(4);
  #elif defined(L4D_CHEAT)
    PAD(0x18);
  #endif
};

struct s_netadr{
  i32 type;
  u8  ip[4];
  u16 port;
};

struct s_server_address{
  u16 connection_port;
  u16 query_port;
  u32 ip_bytes;
};

struct s_game_server_item{
  s_server_address ipaddr;

  i32  ping;
  bool has_sucessful_response;
  bool do_not_refresh;
  i8   game_dir[32];
  i8   map_name[32];
  i8   game_desc[64];
  u32  app_id;
  i32  players;
  i32  max_players;
  i32  bot_players;
  bool password;
  bool secure;
  u32  last_played_time;
  i32  server_version;
  i8   server_name[64];
};

struct s_client_class{
private:
  void* create_fn;
  void* create_event_fn;
public:
  i8* name;
private:
  void* table;
  void* next;
public:
  i32 id;
};

#if defined(SOURCE_2018)
/*struct s_view_setup{
  i32       x;
  i32       unscaled_x;
  i32       y;
  i32       unscaled_y;
  i32       width;
  i32       unscaled_width;
  i32       height;
  i32       unscaled_height;
  bool      ortho;
  float     ortho_left;
  float     ortho_top;
  float     ortho_right;
  float     ortho_bottom;
  bool      custom_view_matrix;
  matrix3x4 mat_custom_view_matrix;
  bool      custom_proj_matrix;
  void*     mat_custom_proj_matrix;
  void*     volume_culler;
  float     fov;
  float     viewmodel_fov;
  vec3      origin;
  vec3      angles;
  float     nearz;
  float     farz;
  float     viewmodel_z_near;
  float     viewmodel_z_far;
  float     aspect_ratio;
  float     near_blur_depth;
  float     near_focus_depth;
  float     far_focus_depth;
  float     far_blur_depth;
  float     near_blur_radius;
  float     far_blur_radius;
  i32       do_fquality;
  i32       motion_blur_mode;
  float     shutter_time;
  vec3      shutter_open_pos;
  vec3      shutter_open_angles;
  vec3      shutter_close_pos;
  vec3      shutter_close_angles;
  float     off_center_top;
  float     off_center_bottom;
  float     off_center_left;
  float     off_center_right;
  bool      off_center : 1;
  bool      render_to_subrect_of_larger_screen;
  bool      do_bloom_and_tone_mapping : 1;
  bool      do_depth_of_field : 1;
  bool      hdr_target : 1;
  bool      draw_world_normal : 1;
  bool      cull_front_faces : 1;
  bool      cache_full_scene_state : 1;
  bool      csm_view : 1;
};*/
struct s_view_setup {
  PAD(0x34);
  float fov;
  float viewmodel_fov;
  vec3  origin;
  vec3  angles;
  PAD(0x1F0);
};
#else
struct s_view_setup{
  i32       x;
  i32       unscaled_x;
  i32       y;
  i32       unscaled_y;
  i32       width;
  i32       unscaled_width;
  i32       height;
  i32       stereo_eye;
  i32       unscaled_height;
  bool      ortho;
  float     ortho_left;
  float     ortho_top;
  float     ortho_right;
  float     ortho_bottom;
  float     fov;
  float     viewmodel_fov;
  vec3      origin;
  vec3      angles;
  float     nearz;
  float     farz;
  float     viewmodel_z_near;
  float     viewmodel_z_far;
  bool      render_to_subrect_of_larger_screen;
  float     aspect_ratio;
  bool      off_center;
  float     off_center_top;
  float     off_center_bottom;
  float     off_center_left;
  float     off_center_right;
  bool      do_bloom_and_tone_mapping;
  bool      cache_full_scene_state;
  bool      view_to_projection_override;
  matrix4x4 view_to_projection;
};
#endif

struct s_global_data{
  float   real_time;
  i32     frame_count;
  float   absolute_frame_time;
  float   cur_time;
  float   frame_time;
  i32     max_clients;
  i32     tick_count;
  float   interval_per_tick;
  float   interpolation_amount;
  i32     sim_ticks_this_frame;
};

struct s_move_data{
  bool  first_run_of_functions;
  bool  game_code_moved_player;
  u32   player_handle;
  i32   impulse_command;
  vec3  view_angles;
  vec3  abs_view_angles;
  i32   buttons;
  i32   old_buttons;
  float foward_move;
  float old_forward_move;
  float side_move;
  float up_move;
  float max_speed;
  float client_max_speed;
  vec3  velocity;
  vec3  angles;
  vec3  old_angles;
  float out_step_height;
  vec3  out_wish_vel;
  vec3  out_jump_vel;
  vec3  constraint_center;
  float constraint_radius;
  float constraint_width;
  float constraint_speed_factor;
  vec3  abs_origin;
};

class s_bf_write {
public:
  uptr  data;
  i32  data_bytes;
  i32  data_bits;
  i32  cur_bit;
  bool overflow;
  bool assert_on_overflow;
  i8*  debug_name;

  void start_writing(void* _data, i32 bytes, i32 start_bit, i32 bits) {
    bytes &= ~3;

    data       = (uptr)_data;
    data_bytes = bytes;

    if ( bits == -1 )
      data_bits = bytes << 3;
    else
      data_bits = bits;

    cur_bit  = start_bit;
    overflow = false;
  }
};

class s_bf_read{
public:
  uptr data;
  i32  data_bytes;
  i32  data_bits;
  i32  cur_bit; //
  bool overflow;
  bool assert_on_overflow;
  i8*  debug_name;

  template < typename T = u8 > T read( i32 size = -1 ) {
    if ( size < 0 )
      size = sizeof( T );

    auto v = *(T*)( data );
    data += size;
    cur_bit += size;

    return v;
  }

  i8* read_str(bool skip_start = true) {
    // cuts off the start of the string.
    if(skip_start)
      data++;

    u32 str_len = strlen((i8*)data);

    i8* str_buf = (i8*)data;

    data += str_len + 1;
    if(skip_start)
      cur_bit += str_len + 2;
    else
      cur_bit += str_len + 1;

    return str_buf;
  }

  void reset(){
    data -= cur_bit;
    cur_bit = 0;
  }

  i32 get_num_bytes_left(){
    return data_bytes - cur_bit;
  }
};

struct s_recv_prop{
  i8*                 name;
  u32                 type;
  u32                 flags;
  i32                 string_buffer_len;
  bool                inside_aray;
  void*               extra_data;
  s_recv_prop*        array_prop;
  void*               array_length_proxy;
  void*               proxy;
};

struct s_ray{
  s_ray(){
    start   = vec3();
    delta   = vec3();
    extents = vec3();

    start_z        = 0.f;
    delta_z        = 0.f;
    extens_z       = 0.f;
    start_offset_z = 0.f;

    is_ray   = false;
    is_swept = false;
  }
  vec3  start;
  float start_z;
  vec3  delta;
  float delta_z;
  vec3  start_offset;
  float start_offset_z;
  vec3  extents;
  float extens_z;
#if defined(GMOD_CHEAT)
  const void* unk;
#endif
  bool  is_ray;
  bool  is_swept;
};

struct s_plane{
  s_plane(){
    normal     = vec3();
    dist       = 0.f;
    type       = 0;
    sign_bits  = 0;
  }

  vec3  normal;
  float dist;
  u8    type;
  u8    sign_bits;
  PAD(0x2);
};

struct s_surface{
  s_surface(){
    name  = nullptr;
    props = 0;
    flags = 0;
  }

  i8* name;
  i16 props;
  u16 flags;
};

struct s_vplane{
  s_vplane(){
    normal = vec3();
    dist   = 0.f;
  }

  vec3 normal;
  float dist;
};

typedef s_vplane s_frustum[6];

struct s_trace{
  s_trace(){
    start               = vec3();
    end                 = vec3();
    fraction            = 0.f;
    contents            = 0;
    disp_flags          = 0;
    solid               = false;
    started_solid       = false;
    fraction_left_solid = 0.f;
    hit_group           = 0;
    physics_bone        = 0;
    hit_entity          = nullptr;
    hitbox              = 0;
#if defined(SOURCE_2018)
    world_surface_index = 0;
#endif
  }

  vec3            start;
  vec3            end;
  s_plane         plane;
  float           fraction;
  i32             contents;
  u16             disp_flags;
  bool            solid;
  bool            started_solid;
  float           fraction_left_solid;
  s_surface       surface;
  i32             hit_group;
  i16             physics_bone;
#if defined(SOURCE_2018)
  u16             world_surface_index;
#endif
  c_internal_base_entity*  hit_entity;
  i32             hitbox;

  ALWAYSINLINE bool vis(void* ent = nullptr){
    return fraction >= 1.f || hit_entity == ent;
  }
};

struct s_studio_bbox{
  i32 bone;
  i32 group;
  vec3 mins;
  vec3 maxs;
  i32  hitbox_name_index;
  PAD(0xC);
  float radius;
  PAD(0x10);
};

struct s_studio_hitbox_set{
  i32 name_index;
  i32 num_hitboxes;
  i32 hitbox_index;

  ALWAYSINLINE s_studio_bbox* get_hitbox(i32 i){
    if(i < 0 || i >= num_hitboxes)
      return nullptr;

    assert(hitbox_index >= 0 && hitbox_index <= 128);
    return (s_studio_bbox*)((i8*)this + hitbox_index) + i;
  }
};

struct s_studio_hdr{
  i32     id;
  i32     version;
  i32     checksum;
  i8      name[64];
  i32     len;
  vec3    eye_pos;
  vec3    illum_pos;
  vec3    hull_min;
  vec3    hull_max;
  vec3    view_bbmin;
  vec3    view_bbmax;
  i32     flags;
  i32     num_bones;
  i32     bone_index;
  i32     num_bone_controllers;
  i32     bone_controller_index;
  i32     num_hitbox_sets;
  i32     hitbox_set_index;

  ALWAYSINLINE s_studio_hitbox_set* get_hitbox_set(i32 i){
    if(i < 0 || i >= num_hitbox_sets)
      return nullptr;

    assert(num_hitbox_sets >= 0 && num_hitbox_sets <= 512);
    assert(hitbox_set_index >= 0 && hitbox_set_index <= 65535);
    return (s_studio_hitbox_set*)((i8*)this + hitbox_set_index) + i;
  }
};

class c_steamid{
public:
  union u_steamid{
    struct s_steam{
      u32 account_id : 32;
      u32 account_instance : 20;
      u32 account_type : 4;
      i32 universe : 8;
    } comp;

    u64 bits;
  } m_steamid;
};

struct s_beam_info{
  i32             type;
  c_internal_base_entity*  start_ent;
  i32             start_attachment;
  c_internal_base_entity*  end_ent;
  i32             end_attachment;
  vec3            start;
  vec3            end;
  i32             model_index;
  i8*             model_name;
  i32             halo_index;
  i8*             halo_name;
  float           halo_scale;
  float           life;
  float           width;
  float           end_width;
  float           fade_length;
  float           amplitude;
  float           brightness;
  float           speed;
  i32             start_frame;
  float           frame_rate;
  float           red;
  float           green;
  float           blue;
  bool            renderable;
  i32             segments;
  i32             flags;
  vec3            center;
  float           start_radius;
  float           end_radius;
  PAD(200);

  s_beam_info(){
    type          = 0;
    segments      = -1;
    model_name    = nullptr;
    halo_name     = nullptr;
    model_index   = -1;
    halo_index    = -1;
    renderable    = true;
    flags         = 0;
  }
};

struct s_stencil_state{
  s_stencil_state()
  {
    enable = false;
    pass_op = fail_op = zfail_op = STENCILOPERATION_KEEP;
    compare_func = STENCILCOMPARISONFUNCTION_ALWAYS;
    reference_value = 0;
    test_mask = write_mask = 0xFFFFFFFF;
  }

  bool                        enable;
  shader_stencil_op           fail_op;
  shader_stencil_op           zfail_op;
  shader_stencil_op           pass_op;
  stencil_comparison_function compare_func;
  i32                         reference_value;
  u32                         test_mask;
  u32                         write_mask;
};

struct start_sound_params{
  bool  static_sound;
  i32   user_data;
  i32   sound_source;
  i32   ent_channel;
  c_sfx_table* sfx;
  vec3  pos;
  vec3  dir;
  bool  update_positions;
  float volume;
  i32   sound_level;
  i32   flags;
  i32   pitch;
  i32   dsp;
  bool  from_server;
  float delay;
  i32   speaker_entity;
  bool  suppress_recording;
  i32   initial_stream_position;
};

class c_steamapicontext {
public:
  void* steamclient; //0x0000
  c_interface_steamuser* steamuser; //0x0004
  c_interface_steamfriends* steamfriends; //0x0008
  void* steamutils; //0x000C
  void* steammatchmaking; //0x0010
  void* steamgamesearch; //0x0014
  void* steamuserstats; //0x0018
  void* steamapps; //0x001C
  void* steam_matching_servers; //0x0020
  void* steam_networking; //0x0024
  void* steam_remote_storage; //0x0028
  void* steam_screenshot; //0x002C
  void* steam_http; //0x0030
  void* steamcontroller; //0x0034
  void* steamugc; //0x0038
  void* steamapplist; //0x003C
  void* steammusic; //0x0040
  void* steam_music_remote; //0x0044
};

struct s_fire_bullets_info{
  i32             shots;
  vec3            src;
  vec3            dir;
  vec3            spread;
  float           distance;
  i32             ammo_type;
  i32             tracer_freq;
  float           damage;
  i32             player_damage;
  i32             flags;
  float           damage_force_scale;
  c_internal_base_entity*  attacker;
  c_internal_base_entity*  ignore_ent;
  bool            primary_attack;
  bool            use_server_random_seed;
};

struct s_net_cvar_data{
  i8 name[260];
  i8 value[260];
};

struct s_model_render_data{
  vec3        origin;
  vec3        angles;
  void*       renderable;
  void*       model;
  matrix3x4*  model_to_world;
  matrix3x4*  lighting_offset;
  vec3*       lighting_origin;
  i32         flags;
  i32         entity_index;
  i32         skin;
  i32         body;
  i32         hitbox_set;
  u16         instance;
};

class c_command_list{
public:
  enum {
    COMMAND_MAX_ARGC = 64,
    COMMAND_MAX_LENGTH = 512,
  };

  i32  arg_count;
  i32  argv0;
  i8   argsbuffer[ COMMAND_MAX_LENGTH ];
  i8   argvbuffer[ COMMAND_MAX_LENGTH ];
  i8*  pargv[ COMMAND_MAX_ARGC ];
};

struct s_cmd_alias{
  s_cmd_alias* next;
  i8           name[32];
  i8*          value;
};

struct s_effect_data{
  vec3  origin;
  vec3  start;
  vec3  normal;
  vec3  angles;
  i32   flags;
  i32   entity_handle;
  float scale;
  float magintude;
  float radius;
  i32   attachment_index;
  i16   surface_prop;
  i32   material;
  i32   damage_type;
  i32   hitbox;
};

struct s_send_prop{
  void*          rtti;
  void*          matching_recv_prop;
  i32            type; 
  i32            bits;
  float          low_value;
  float          high_value;
  s_send_prop*   array_prop;
  void*          array_length_proxy;
  i32            elements;
  i32            elements_stride;
  i8*            exclude_dt_name;
  i8*            parent_array_prop_name;
  i8*            var_name;
  float          high_low_mul;
  i32            flags;
  void*          proxy_fn;
  void*          data_table_proxy_fn;
  void*          data_table;
  i32            offset;
  const void*    extra_data; 
};

class d_variant{
public:
  union{
    float  flt;
    i32    val;
    i8*    str; 
    void*  data; 
    float  vec[3]; // largest member so its 12 bytes
  };
  i32 type;
};

struct c_recv_proxy_data{
  s_recv_prop* recv_prop;
  d_variant    value;
  i32          element;
  i32          object_id;
};

struct s_decode_info{
  c_recv_proxy_data   recv_proxy_data; 
  void*               struct_base;
  void*               data; 
  s_send_prop*        send_prop;
  s_bf_read*          in;
  i8                  temp_str[512];
};

class c_net_channel{
public: 
  void* rtti;
  i32   unk;
  i32   out_sequence_nr;
  i32   in_sequence_nr;
  i32   out_sequence_nr_ack;
  i32   out_reliable_state;
  i32   in_reliable_state;
  i32   choked_packets;
};

#if defined(SOURCE_2018)
#define ENGINE_USES_PROTO_BUFS
#endif

// Working on this.
class c_net_message_base{
public:
#if defined(ENGINE_USES_PROTO_BUFS)
  void* rtti;
  PAD(0x50); // This should be correct.
#else
  void* rtti; // 0x0
  bool  reliable; // 0x4
  void* net_channel; // 0x8
  void* unk; // 0xC
#if defined(TF2_CHEAT)
  void* unk2; // 0x10
#endif
#endif
};

class c_svc_print : public c_net_message_base{
public:
  i8* text;
private:
  i8  text_buffer[2048];
};

#pragma pack(push, 1)
class c_clc_move : public c_net_message_base{
public:
  i32        backup_commands;
  i32        new_commands;
  iptr       length;
#if defined(ENGINE_USES_PROTO_BUFS)
  PAD(0x24); // some protobuf thing.
#else
  s_bf_read  data_in;
#endif
  s_bf_write data_out;
};
#pragma pack(pop)


class c_clc_respond_cvar_value : public c_net_message_base{
public:
  iptr cookie;
  i8* name;
  i8* val;
  i32 query_status;
private:
  i8 cvar_name_buffer[256];
  i8 cvar_value_buffer[256];
};

struct light_desc{
  i32   type;
  vec3  color;
  vec3  pos;
  vec3  dir;
  float range;
  float falloff;
  vec3  attenutation;
  float theta;
  float phi;
  float theta_dot;
  float phi_dot;
  u32   flags;
  float oneover_theta_dot_minus_phi_dot;
  float range_squared;
};

struct s_studio_render_config{
  vec3  eye_shift;
  float eye_size;
  float eye_glint_pixel_width_lod_threshold;
  i32   max_decals_per_model;
  i32   draw_entities;
  i32   skin;
  i32   fullbright;

  bool eye_move : 1;
  bool software_skin : 1;
  bool no_hardware : 1;
  bool no_software : 1;
  bool teeth : 1;
  bool eyes : 1;
  bool flex : 1;
  bool wireframe : 1;
  bool draw_normals : 1;
  bool draw_tangent_frame : 1;
  bool draw_z_buffered_wire_frame : 1;
  bool software_lighting : 1;
  bool show_env_cubemap_only : 1;
  bool wire_frame_decals : 1;
  i32  reserved[4];
};

struct s_studio_render_config_internal : public s_studio_render_config{
  bool supports_vertex_and_pixel_shaders : 1;
  bool supports_over_bright : 1;
  bool enable_hw_morph : 1;
};

class c_material;
struct c_studio_render_context{
  s_studio_render_config_internal config;
  vec3                            view_target;
  vec3                            view_origin;
  vec3                            view_right;
  vec3                            view_up;
  vec3                            view_plane_normal;
  vec4                            light_box_colors[6];
  light_desc                      local_lights[4];
  i32                             num_local_lights;
#if defined(TF2_CHEAT)
  PAD(0x10); // This could be an sdk2013 thing, I don't know, we should see where this should be at in the class later on using reclass. But for now this fixes the issue.
#endif
  float                           color_mod[3];
  float                           alpha;
  c_material*                     forced_material;
  i32                             forced_material_type;

  void set_colour_modulation(colour clr){
    color_mod[0] = (float)(clr.r / 255);
    color_mod[1] = (float)(clr.g / 255);
    color_mod[2] = (float)(clr.b / 255);
  }
};

#if defined(L4D_CHEAT)
  struct s_player_info{
    u32 friends_id;
    u32 unk;
    i8  name[32];
    i32 user_id;
    i8 guid[36];
    u32 steamid32;
    i8 friends_name[32];
    bool fake_player;
    PAD(0xC8);
  };
#else
#if defined(GMOD_CHEAT)
  struct s_player_info{
    i8      name[128];
    i32     user_id;
    i8      guid[33];
    u32     friends_id;
    i8      friends_name[128];
    bool    fake_player;
    bool    is_hltv;
    u32     custofiles[4];
    u8      files_downloaded;
  };
#else
  struct s_player_info{
    i8      name[32];
    i32     user_id;
    i8      guid[33];
    u32     friends_id;
    i8      friends_name[32];
    bool    fake_player;
    bool    is_hltv;
    u32     custofiles[4];
    u8      files_downloaded;
  };
#endif
#endif
