const crc = require("./crc32s.js");
const fnv1a = require("./fnv1a.js");
const c = require("./c.js");
const gen = require("./gen.js");

const cmd_prefix = "d3fffdef8062daf1654cd70097eb9219";
exports.make_cmd = function(msg){
  return fnv1a.FNV1A(cmd_prefix + msg + cmd_prefix);
}

exports.net_fragment_magic = 0x9B0808;
exports.net_fragment = c.struct({
  magic:       c.u32,
  cmd:         c.u32,
  status:      c.u32,
  caller:      c.u32,
  owner:       c.u32,
  regression:  c.u32,
  security:    c.u32,
  extra_flags: c.u32,
  len:         c.u32,
  nonce:       c.u8 * 32,
  checksum:    c.u32,
  data:        0,
});

exports.c_net_request_detection_cmd = exports.make_cmd("NET_REQUEST_REPORT_DETECTION");
exports.c_net_request_detection = c.struct({
  detection_id: c.u32,
  current_steam_id: c.u32,
});

exports.c_net_receive_detection_result = c.struct({
  detection_result: c.u32,
});

exports.c_net_receive_info_cmd = exports.make_cmd("NET_REQUEST_INFO");
exports.c_net_receive_info = c.struct({
  timestamp:    c.u32,
  loader_hash:  c.u32,
  update_mode:  c.bool,
  data: 0,
});

exports.s_steam_id_list = c.struct({
  size: c.u32,
  id:   c.u32 * 1024
});

exports.c_net_request_license_cmd = exports.make_cmd("NET_REQUEST_LICENSE");
exports.sizeof_username = c.i8 * 32;
exports.sizeof_password = c.i8 * 256;
exports.sizeof_hwid_cpu_brand_name = c.i8 * 48;
exports.c_net_request_license = c.struct({
  client_version: c.u32,
  username: exports.sizeof_username,
  password: exports.sizeof_password,

  // hardware info
  hwid_mac_address_hash:      c.u32,
  hwid_arp_mac_address_hash:  c.u32,
  hwid_cpu_brand_name:        exports.sizeof_hwid_cpu_brand_name,
  steam_id_list:              exports.s_steam_id_list.sizeof,
});

exports.license_object = c.struct({
  pos:          c.u32,
  name:         c.i8 * 128,
  name_hash:    c.u32,
  short_name:   c.i8 * 128,
  target_hash:  c.u32, 
  expire:       c.u32,
  expire_str:   c.i8 * 128,
  disabled:     c.bool,
  beta:         c.bool,
  hypervisor:   c.bool,
  spoofer:      c.bool,
  vac_mode:     c.bool,
  vac_pos:      c.u32,
  appid:        c.u32,
  d3dx:         c.u32,
  runs_on_stub: c.bool,
});

exports.c_net_receive_license = c.struct({
  key:              c.i8 * 32,
  license_expire_seconds:  c.u32,
  objects_size:     c.u32, 
  objects:          0,
});

exports.c_net_request_image_resource_cmd = exports.make_cmd("NET_REQUEST_IMAGE_RESOURCE");
exports.c_net_request_image_resource = c.struct({
  key: c.i8 * 32,
  object_pos: c.u32,
});

exports.c_net_receive_image_resource = c.struct({
  len: c.u32,
  data: 0,
});

exports.c_net_request_stub_cmd = exports.make_cmd("NET_RECEIVE_STUB");
exports.c_net_request_stub = c.struct({
  key: c.i8 * 32,
});

exports.c_net_receive_stub = c.struct({
  len: c.u32,
  data: 0,
});

exports.cpu_extensions = c.struct({
  sse:    c.bool,
  sse2:   c.bool,
  sse3:   c.bool,
  ssse3:  c.bool,
  sse41:  c.bool,
  sse42:  c.bool,
  avx:    c.bool,
  avx2:   c.bool,
  avx512: c.bool,
});

exports.c_net_request_cheat_info_cmd = exports.make_cmd("NET_REQUEST_CHEAT_INFO");
exports.c_net_request_cheat_info = c.struct({
  key: c.i8 * 32,
  extensions: exports.cpu_extensions.sizeof,
  object_pos: c.u32,
});

exports.c_net_receive_cheat_info = c.struct({
  x86: c.bool,
  sizeof_image: c.u32,
  imports_count: c.u32,
  imports: 0,
});

exports.c_net_request_dxgidmp_cmd = exports.make_cmd("NET_REQUEST_DXGI");
exports.c_net_request_dxgidmp = c.struct({
  key: c.i8 * 32,
  x86: c.bool,
});

exports.c_net_receive_dxgidmp = c.struct({
  buffer_len: c.u32,
  buffer: 0,
});

// This structure is quite special, we needed to copy the gen input table to the struct
// Pretty neat since it saves us time baby sitting this specific implementation
exports.c_net_request_cheat_cmd = exports.make_cmd("NET_REQUEST_CHEAT");
exports.c_net_request_cheat = c.struct(Object.assign({
  key: c.i8 * 32,
  extensions: exports.cpu_extensions.sizeof,
  object_pos: c.u32,
  
  sizeof_HACK: 0, // HACK: get size of current struct (THIS IS NOT IN THE STRUCTURE)

  // gen_input
}, gen.gen_input.data_sizes)); // data_sizes maintains the original structure data

exports.net_error_message_cmd = exports.make_cmd("NET_ERROR_MESSAGE");
exports.net_error_message = c.struct({
  exit: c.bool,
  str_len: c.u32,
  data: 0,
});

// Cloud config related
exports.c_net_request_cloud_config_cmd = exports.make_cmd("NET_REQUEST_CLOUD_CONFIG");
exports.c_net_request_cloud_config = c.struct({
  config_key: c.i8 * 32,
  config_id:  c.u32,
});

exports.c_net_receive_cloud_config = c.struct({
  buffer_len: c.u32,
  buffer:     0,
});

exports.c_net_submit_cloud_config_cmd = exports.make_cmd("NET_SUBMIT_CLOUD_CONFIG");
exports.c_net_submit_cloud_config = c.struct({
  config_key: c.i8 * 32,
  config_id:  c.u32,
  buffer_len: c.u32,
  buffer:     0,
});

exports.c_net_submit_cloud_config_response = c.struct({
  ok: c.bool,
});

exports.c_net_submit_error_code_cmd = exports.make_cmd("NET_SUBMIT_ERROR_CODE");
exports.c_net_submit_error_code = c.struct({
  error_code:  c.u32,
  status_code: c.u32,
});

exports.c_net_submit_error_code_response = c.struct({
  ok: c.bool,
});

exports.c_net_submit_tf_cheater_sid32_cmd = exports.make_cmd("NET_SUBMIT_TF_CHEATER_SID32");
exports.c_net_submit_tf_cheater_sid32 = c.struct({
  config_key: c.i8 * 32,
  sid:        c.u32,
  reason:     c.u32,
  server_type: c.u32,
  game:        c.u32,
  reserved:    c.u32 * 128,
});

exports.c_net_submit_tf_bot_sid32_cmd = exports.make_cmd("NET_SUBMIT_TF_BOT_SID32");
exports.c_net_submit_tf_bot_sid32 = c.struct({
  config_key: c.i8 * 32,
  sid:        c.u32,
});


exports.c_net_submit_tf_cheater_sid32_response = c.struct({
  ok: c.bool,
});

exports.c_net_submit_tf_bot_sid32_response = c.struct({
  ok: c.bool,
});


exports.c_net_request_tf_cheater_sid32_cmd = exports.make_cmd("NET_REQUEST_TF_CHEATER_SID32");
exports.c_net_request_tf_cheater_sid32 = c.struct({
  config_key: c.i8 * 32,
  sid:        c.u32,
  name_hash:  c.u32,
  game:       c.u32,
  reserved:   c.u32 * 128,
});

exports.CHEATER_FLAG_IS_KNOWN_CHEATER            = 1 << 1;
exports.CHEATER_FLAG_IS_KNOWN_BOT                = 1 << 2;
exports.CHEATER_FLAG_HAS_KNOWN_BOT_NAME          = 1 << 3;
exports.CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME = 1 << 4;

exports.CHEATER_FLAG_RIJIN_USER                  = 1 << 29;
exports.CHEATER_FLAG_DEV_IGNORE                  = 1 << 30;
exports.CHEATER_FLAG_DEV_ESP                     = 1 << 31;

exports.c_net_request_tf_cheater_sid32_response = c.struct({
  cheater_steam_group_count: c.u32,
  bot_steam_group_count:     c.u32,
  cheater_flags:             c.u32,
  buffer_len:                c.u8,
  group_len:                 c.u8,
  buffer:                    c.i8 * 32,
  group_buffer:              c.i8 * 32,
  reserved:                  c.u32 * 128,
});

exports.c_net_submit_tf_lobby_broadcast_cmd = exports.make_cmd("NET_SUBMIT_TF_LOBBY");
exports.c_net_submit_tf_lobby_broadcast = c.struct({
  config_key:   c.i8 * 32,
  country_code: c.i8 * 8,
  leader:       c.u32,
  member_count: c.u32,
  lobby_state:  c.u32,
});

exports.c_net_submit_tf_lobby_broadcast_response = c.struct({
  ok: c.bool,
});

exports.c_net_request_tf_lobby_list_cmd = exports.make_cmd("NET_REQUEST_TF_LOBBY_LIST");
exports.c_net_request_tf_lobby_list = c.struct({
  key: c.i8 * 32,
});

exports.c_net_receive_tf_lobby_list = c.struct({
  buffer_len: c.u32,
  buffer: 0,
});

exports.s_lobby_list_entry = c.struct({
  username:     c.i8 * 32,
  country_code: c.i8 * 8,
  sid:          c.u32,
  member_count: c.u32,
  lobby_state:  c.u32,
});

exports.c_net_transmit_platform_identifier_cmd = exports.make_cmd("NET_TRANSMIT_PLATFORM_IDENTIFIER");
exports.c_net_transmit_platform_identifier = c.struct({
  config_key:   c.i8 * 32,
  id:           c.u32,
  id2:          c.u32,
});

exports.c_net_transmit_platform_identifier_response = c.struct({
  ok: c.bool,
  kill: c.bool,
});

exports.c_net_submit_unique_id_cmd = exports.make_cmd("NET_SUBMIT_UNIQUE_ID");
exports.c_net_submit_unique_id = c.struct({
  config_key:   c.i8 * 32,
  id:           c.u32,
});

exports.c_net_submit_unique_response = c.struct({
  is_user: c.bool,
});

exports.c_net_send_status_code_cmd = exports.make_cmd("NET_SEND_STATUS_CODE");
exports.c_net_send_status_code = c.struct({
  config_key: c.i8 * 32,
  name:       c.u32,
  what:       c.u32,
  reserve1:   c.u32,
  reserve2:   c.u32,
  reserve3:   c.u32,
});

exports.c_net_send_status_code_response = c.struct({
  ok: c.bool,
});

exports.c_net_request_exception_str_cmd = exports.make_cmd("NET_REQUEST_EXCEPTION_STR");
exports.c_net_request_exception_str = c.struct({
  buffer_len:    c.u32,
  buffer: 0,
});

exports.c_net_request_exception_str_response = c.struct({
  ok: c.bool,
});

exports.c_net_request_screen_shot_cmd = exports.make_cmd("NET_REQUEST_SCREEN_SHOT");
exports.c_net_request_screen_shot = c.struct({
  image_hash: c.u32,
  buffer_len: c.u32,
  buffer: 0,
});

exports.s_string_data = c.struct({
  buffer_len: c.u32,
  buffer: c.i8 * 1024,
});

exports.parse_string_data = function(buffer, start_offset, max_entries, xor_key = undefined){
  let table = [];
  try{
    for(let i = 0; i < max_entries; i++){
      const offset = start_offset + i * exports.s_string_data.sizeof;
      const string_data = buffer.slice(offset, offset + exports.s_string_data.sizeof);

      const str_len = string_data.readUInt32LE(0);
      const str_buf = string_data.slice(4, 4 + str_len);
      if(xor_key !== undefined){
        for (let j = 0; j < str_buf.length; j++)
          str_buf[j] ^= xor_key;
      }

      const str = str_buf.toString('utf8').replace(/\0/g, '');
      table.push(str);
    }
  }
  catch(err){
    console.log(err);
  }

  return table;
}

exports.c_net_request_challenge_cmd = exports.make_cmd("NET_REQUEST_CHALLENGE")
exports.c_net_request_challenge = c.struct({
  process_name:  c.i8 * 128,
  machine_guid:  c.i8 * 64,
  computer_name: c.i8 * 64,
  steam_active_user: c.u32,
  steam_pid: c.u32,
  running_appid: c.u32,
  flags: c.u32,
  ida_history_count: c.u32,
  ida_history: exports.s_string_data.sizeof * 128,
});

exports.c_net_request_query_cloud_config_cmd = exports.make_cmd("NET_REQUEST_QUERY_CLOUD_CONFIG")
exports.c_net_request_query_cloud_config = c.struct({
  config_key: c.i8 * 32,
  config_id: c.u32,
});

exports.c_receive_query_cloud_config = c.struct({
  ok: c.u8,
  exists: c.u8,
  status: c.u32,
});

