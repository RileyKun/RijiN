// core
exports.c              = require("./c.js");
exports.structs        = require("./structs.js");
exports.crc            = require("./crc32s.js");
exports.fnv1a          = require("./fnv1a.js");
exports.fs             = require("fs");
exports.colors         = require("colors");
exports.sync_request   = require("sync-request");
exports.ip_range_check = require("ip-range-check");


license_obj        = undefined;
cloud_config_obj   = undefined;

const fs             = exports.fs;
const colors         = exports.colors;
const sync_request   = exports.sync_request;
const ip_range_check = exports.ip_range_check;
const crypto         = require('crypto');
const compressing    = require("compressing");

var settings         = undefined;
try{
  if(!fs.existsSync("./config.json")){
    console.error("\n\n\n\n\n\n\n\n\nYou're missing config.json!\n\n\n\n\n\n\n\n\n");
    return;
  }

  settings = JSON.parse(fs.readFileSync("./config.json"));
  console.log("Loaded config.json");
}
catch(err){
  console.error(`ERROR LOADING SETTINGS: ${err}!!!!`);
  return;
}

// only allow dev mode on a windows operating system with explicit -dev
exports.update_mode     = false;
exports.dev_mode        = process.platform === "win32" && process.argv[2] == "-dev";
exports.staging_mode    = process.argv[2] == "-staging";
exports.test_shipping   = false;
exports.cloudflare_only = !exports.dev_mode;
exports.rlm_api_url     = (exports.dev_mode && exports.test_shipping) ? settings.rlm_api_url_test : settings.rlm_api_url;
exports.rlm_api_key     = settings.rlm_api_key;

exports.listen_addr = exports.dev_mode ? settings.dev_ip : settings.shipping_ip;
exports.server_name = exports.dev_mode ? "dev" : "shipping";

if(!exports.dev_mode && exports.staging_mode){
  exports.server_name = "staging";
  exports.listen_addr = settings.staging_ip;
}

// client version
exports.min_client_version = 330; // The client must be running >= to this version

// socket related
exports.listen_port      = exports.dev_mode ? settings.dev_port : settings.main_port;
exports.sync_addr        = settings.sync_server_ip;
exports.sync_master_port = settings.sync_server_master_port;
exports.sync_listen_port = settings.sync_server_listen_port;
exports.max_input_buffer = 25000000; // 25 MB.

// ms before general cleanup
exports.housekeeping_ms = 3600000/*1HR*/;

// stub
exports.stub_location = "./bin/stub.dll";
exports.stub_location_beta = "./bin/stub_beta.dll";
exports.dxgidmp64_location = "./bin/dxgidmp/dxgidmp64.exe";
exports.dxgidmp86_location = "./bin/dxgidmp/dxgidmp86.exe";
exports.cloud_configs_location = "./cloud";
exports.cloud_configs_cache_location = "./cloud/sessions.dat";
exports.session_iden_cache_location = "./cloud/sessions_iden.dat";
exports.tf_cheater_list_location = "./tf_cheater_list";
exports.vtable_offsets_location  = "./source_vtables";
exports.stats_data_location      = "./loader_stats";
exports.inlined_data             = "./inlined_data";
exports.logs_location            = "./logs";
exports.screenshots_location     = "./screenshots";

// struct related
exports.NET_FRAGMENT_ERROR        = 0;
exports.NET_FRAGMENT_OK           = 1;
exports.NET_FRAGMENT_DATA_DIFFERS = 2;

// rlm api related
exports.RLM_ERROR             = 0;
exports.RLM_SUCCESS           = 1;
exports.RLM_BAD_LOGIN_DATA    = 2;
exports.RLM_BANNED            = 3;
exports.RLM_PROFILE_NOT_SETUP = 4;
exports.RLM_INVISIBLE_LOCK    = 5;

exports.net_fragment_kill_process = (1 << 30);

exports.net_fragment_set_nonce = function(fragment, struct, nonce){
  nonce.copy(fragment, struct.nonce);
}

exports.get_formated_time = function(){
  let date = new Date();
  return `${date.getUTCMonth() + 1}/${date.getUTCDate()}/${date.getUTCFullYear()} ${date.getUTCHours()}:${date.getUTCMinutes()}:${date.getUTCSeconds()}`;
}

exports.get_file_formatted_date = function(go_back = undefined){
  let date = new Date();

  if(go_back !== undefined)
    date.setUTCDate(date.getUTCDate() - go_back);

  return `${date.getUTCMonth() + 1}_${date.getUTCDate()}_${date.getUTCFullYear()}`;
}

exports.connection_logs = function(where, socket, license, what){
  if(where == undefined || where == null || where.includes(".")
    || what == undefined || what == null){
    console.log("connection_logs: Cannot have any . in where");
    return;
  }

  if(!fs.existsSync(exports.logs_location))
    fs.mkdirSync(exports.logs_location);

  where = where.trim();
  where = where.replaceAll(".", "").replaceAll("/", "").replaceAll("%", "").replaceAll("\"", "").replaceAll("\\", "");

  let path = `${exports.logs_location}/${exports.get_file_formatted_date()}`;
  if(!fs.existsSync(path))
    fs.mkdirSync(path);

  path += `/${where}`;
  if(!fs.existsSync(path))
    fs.mkdirSync(path);

  try{
    let ip        = exports.ip(socket);
    let file_name = license != undefined && license.uid != undefined ? license.uid.toString() : ip;
    if(file_name != undefined)
      file_name = file_name.replaceAll(".", "_").replaceAll(":", "_");
    else
      file_name = "server_global";

    if(ip == undefined)
      ip = "unk";

    if(file_name != "server_global")
      fs.appendFileSync(`${path}/${file_name}.log`, `[${exports.get_formated_time()}] (${ip}) ${what}\n`);
    else
      fs.appendFileSync(`${path}/server.log`, `[${exports.get_formated_time()}] ${what}\n`);
  }
  catch(err){
    console.log(`connection_logs: ${err}`);
  }
}

exports.general_log = function(where, what){
  return exports.connection_logs(where, undefined, undefined, what);
}

const ENCRYPTION_KEY      = Buffer.from("ea4f761c42a8d98ca59572ce4d0f308999025de596b1078aafa1b3a2b9af3014");
const ENCRYPTION_KEY_SIZE = 64;
const NONCE_SIZE          = 32;

function encrypt_buffer(buffer, nonce){
  const len = buffer.length;
  const der_key = Buffer.alloc(ENCRYPTION_KEY_SIZE);

  for (let i = 0; i < ENCRYPTION_KEY_SIZE; i++) {
    der_key[i] = ENCRYPTION_KEY[i] ^ nonce[i % NONCE_SIZE];
  }

  for (let i = 0; i < len; i++) {
    buffer[i] ^= der_key[i % ENCRYPTION_KEY_SIZE];
  }
}

function decrypt_buffer(buffer, nonce){
  const len = buffer.length;
  const der_key = Buffer.alloc(ENCRYPTION_KEY_SIZE);

  for (let i = 0; i < ENCRYPTION_KEY_SIZE; i++) {
    der_key[i] = ENCRYPTION_KEY[i] ^ nonce[i % NONCE_SIZE];
  }

  for (let i = len - 1; i >= 0; i--) {
    buffer[i] ^= der_key[i % ENCRYPTION_KEY_SIZE];
  }
}

exports.net_fragment_create = function(status, buffer, cmd, regression = 0, extra_flags = 0){
  if(regression == undefined)
    console.error("WARNING: net_fragment_create regression parameter is undefined");

  if(extra_flags == undefined)
    console.error("WARNING: net_fragment_create extra_flags parameter is undefined");

  if(buffer == undefined)
    buffer = Buffer.alloc(0);

  const struct = exports.structs.net_fragment;
  const fragment = Buffer.alloc(struct.sizeof + buffer.length).fill(0);

  if(cmd != undefined)
    fragment.writeUInt32LE(cmd, struct.cmd);

  fragment.writeUInt32LE(exports.structs.net_fragment_magic, struct.magic);
  fragment.writeUInt32LE(status, struct.status);
  fragment.writeUInt32LE(0, struct.caller);
  fragment.writeUInt32LE(0, struct.owner);
  fragment.writeUInt32LE(regression, struct.regression);
  fragment.writeUInt32LE(exports.fnv1a.FNV1A("no idea whatever"), struct.security);
  fragment.writeUInt32LE(extra_flags, struct.extra_flags);
  fragment.writeUInt32LE(buffer.length, struct.len);

  const nonce = crypto.randomBytes(32);
  exports.net_fragment_set_nonce(fragment, struct, nonce);

  if(buffer.length > 0){
    encrypt_buffer(buffer, nonce);
    fragment.writeUInt32LE(exports.fnv1a.FNV1A(buffer), struct.checksum);
    exports.c.memcpy(buffer, fragment, struct.data, 0, buffer.length);
  }
  else
    fragment.writeUInt32LE(0, struct.checksum);

  delete buffer;

  return fragment;
}

exports.net_fragment_error_msg = function(socket, str, exit, dont_print){
  if(dont_print == undefined)
    exports.print(socket, `[!] @IP: sending error message "${str}" exit: "${exit ? "true" : "false"}`, true);

  exports.general_log("error_msg", `${exports.ip(socket)}: sending error message: "${str}"`);

  const struct = exports.structs.net_error_message;
  let str_buffer = Buffer.from(str);

  // Add null terminator
  const null_buffer = Buffer.alloc(1).fill(0);
  str_buffer = Buffer.concat([str_buffer, null_buffer]);

  // Write the struct
  const buffer = Buffer.alloc(struct.sizeof + str_buffer.length).fill(0);
  buffer.writeUInt8(exit, struct.exit);
  buffer.writeUInt32LE(str_buffer.length, struct.str_len);
  exports.c.memcpy(str_buffer, buffer, struct.data, 0, Math.min(str_buffer.length, 512)); // 512 max

  return exports.net_fragment_create(exports.NET_FRAGMENT_OK, buffer, exports.structs.net_error_message_cmd);
}

exports.get_calle_func_name = function(where) {
  const stack = new Error().stack;
  if (!stack)
    return "unk";

  const lines = stack.split('\n');

  if (lines.length < 3)
    return "unk";

  const caller_line = lines[where].trim();

  const match = caller_line.match(/at\s+(.*)\s+\(/);
  if (match && match[1])
    return match[1];

  const alt_match = caller_line.match(/at\s+(.*)/);
  if (alt_match && alt_match[1])
    return alt_match[1];

  return "unk";
}

exports.is_valid_request = function(socket, buffer, struct, license_obj, cloud_config_obj, verify_license, check_size){
  if(buffer == undefined)
    return false;

  let what = exports.get_calle_func_name(3);
  if(what == undefined)
    what = "unk";

  if(verify_license == undefined)
    verify_license = true;

  if(check_size == undefined)
    check_size = true;

  if(exports.dev_mode){
    if(!check_size || !verify_license)
      exports.print(socket, `[Some checks disabled] @IP: ${what}: verify license: ${verify_license}, check_size: ${check_size}`);
  }

  try{
    if(struct.sizeof == undefined)
      return false;

    let ip = exports.ip(socket);
    if(!exports.dev_mode){
      if(exports.is_private_ip(ip)){
        exports.print(socket, `@IP: local ip-address out-side of dev mode.`, true);
        return false;
      }
    }

    if(buffer == undefined || buffer.length != struct.sizeof && check_size){
      exports.print(socket, `@IP: ${what}-error: buffer undefined or wrong size ${buffer.length} != ${struct.sizeof}`, true);
      return false;
    }

    if(!verify_license)
      return true;

    if(struct.config_key != undefined){
      const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');
      if(license_key == undefined || license_key.length != 32){
        exports.print(socket, `@IP: ${what}-error: expected config_key but none.`);
        return false;
      }

      if(cloud_config_obj == undefined){
        exports.print(socket, `@IP: unable to fetch license (cc undefined)`);
        return false;
      }

      let license = cloud_config_obj.find(license_key);
      if(license == undefined){
        exports.print(socket, `@IP: has no cloud license`);
        return false;
      }

      if(license.key !== license_key){
        exports.print(socket, `${license.identity}'s key mismatches from reported one ${license.key} vs ${license_key}`);
        return false;
      }

      return true;
    }
    else{
      if(license_obj == undefined){
        exports.print(socket, `@IP: unable to fetch license (non key based)`);
        return false;
      }

      let license = license_obj.current_license(socket);
      if(license == undefined){
        exports.print(socket, `@IP: has no license`);
        return false;
      }

      if(!license_obj.valid(license)){
        exports.print(socket, `@IP: license has expired.`);
        return false;
      }

      if(struct.key != undefined){
        const license_key = buffer.slice(struct.key, 32).toString().replace(/\0[\s\S]*$/g,'');
        if(license.key == undefined || license_key != license.key){
          exports.print(socket, `@IP: license key is mismatched (non cloud)`);
        }
      }
    }
  }
  catch(err){
    exports.print(socket, `is_valid_request exception: ${what}\n${err.stack}`);
  }
  return true;
}

exports.net_fragment_nonce = function(buffer){
  return buffer.subarray(exports.structs.net_fragment.nonce, exports.structs.net_fragment.nonce + 32);
}

exports.net_fragment_checksum = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.checksum);
}

exports.net_fragment_data = function(buffer){
  if(buffer == undefined)
    return undefined;

  try{
    let new_buffer = buffer.slice(exports.structs.net_fragment.sizeof);
    let nonce      = exports.net_fragment_nonce(buffer);
    const incoming_checksum = exports.net_fragment_checksum(buffer);
    const outgoing_checksum = exports.fnv1a.FNV1A(new_buffer);
    if(incoming_checksum != outgoing_checksum){
      console.log(`net_fragment_data: checksum failed! ${incoming_checksum.toString(16)} != ${outgoing_checksum.toString(16)}`);
      return undefined;
    }

    decrypt_buffer(new_buffer, nonce);
    return new_buffer;
  }
  catch(err){
    console.log("net_fragment_data: --> " + err + "\n\n");
  }

  return buffer;
}

exports.net_fragment_magic = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.magic);
}

exports.net_fragment_cmd = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.cmd);
}

exports.net_fragment_caller = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.caller);
}

exports.net_fragment_owner = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.owner);
}

exports.net_fragment_regression = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.regression);
}

exports.net_fragment_security = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.security);
}

exports.net_fragment_extra_flags = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.extra_flags);
}

exports.net_fragment_len = function(buffer){
  return buffer.readUInt32LE(exports.structs.net_fragment.len);
}

exports.report_detection = function(license, socket, what, reason){
  exports.print(socket, `report_detection: ${what}`);

  const license_info = license != undefined ? license.identity + "/" + license.uid.toString() : "[No license]";
  exports.general_log("report_detection", `${exports.ip(socket)} - ${what} ${reason != undefined ? reason : ""} ${license_info}`);

  if(license == undefined || license.uid == undefined){
    console.log(`report_detection: Received invalid license / uid`);
    try{
      exports.send_discord_webhook("security", `[${exports.ip(socket)}]\nUser: (No license)\nWhat: ${what}`);
    }
    catch{}
    return;
  }

  if(reason != undefined){
    switch(Number(reason)){
      default: break;
      case 1:
      {
        what = "RTP_REASON_TIME_CHECK_FAILED";
        break;
      }
      case 2:
      {
        what = "RTP_REASON_SUSPENDED_THREAD_RAN";
        break;
      }
      case 3:
      {
        what = "RTP_REASON_PROCESS_PAUSED";
        break;
      }
      case 4:
      {
        what = "RTP_REASON_SLEEP_BP";
        break;
      }
      case 5:
      {
        what = "RTP_REASON_COMMON_BP_PLACEMENTS";
        break;
      }
      case 6:
      {
        what = "RTP_REASON_DEBUGGER_ATTACHED";
        break;
      }
      case 7:
      {
        what = "RTP_REASON_CONSOLE_WINDOW_PRESENT";
        break;
      }
      case 8:
      {
        what = "RTP_REASON_CONSOLE_WINDOW_CHECK_BP_HOOK_PRESENT";
        break;
      }
      case 9:
      {
        what = "RTP_REASON_QUERY_VIRUTAL_MEMORY_TAMPERED";
        break;
      }
      case 10:
      {
        what = "RTP_REASON_SCYLLA_HIDE_DETECTED";
        break;
      }
      case 11:
      {
        what = "RTP_REASON_TITAN_HIDE_DETECTED";
        break;
      }
      case 12:
      {
        what = "RTP_REASON_DLL_INJECTION_FROM_DESKTOP";
        break;
      }
      case 13:{
        what = "RTP_REASON_CREATE_THREAD_BP_OR_HOOKED";
        break;
      }
      case 14:
      {
        what = "RTP_REASON_KUSERSHARED_KD_DEBUGGER_ENABLED";
        break;
      }
      case 15:
      {
        what = "RTP_REASON_PROTECTION_SYSTEM_TAMPERED";
        break;
      }
      case 16:
      {
        what = "RTP_REASON_NTQUERYSYSTEMINFO_TAMPERED";
        break;
      }
      case 17:{
        what = "RTP_REASON_TEST_SIGNING_CHECK_BYPASSED";
        break;
      }
    }
  }

  exports.print(socket, `\n\nSecurity detection: ${license.uid}/${license.identity}`)

  license.loader_heart_beat_received = undefined;
  license.stub_heart_beat_received   = undefined;
  license.security_violation         = true;

  // Don't report detections to the API if dev, admin 1 or admin 2.
  if(license.uid == 0 || license.uid == 13 || license.uid == 1)
    return;

  let hash = exports.fnv1a.FNV1A(what);
  exports.send_api_req(
    {
      access_key:    exports.rlm_api_key,
      cmd:           "report_detection",
      ip:            exports.ip(socket),
      uid:           license.uid,
      detection_id:  hash
    },
    true
   );

  try{
    exports.send_discord_webhook("security", `[${exports.ip(socket)}]\nUser: ${license.identity}\nID: ${license.uid}\nWhat: ${what}`);
  }
  catch{}
}

exports.valid_caller_hashes = [exports.fnv1a.FNV1A("LOADER"), exports.fnv1a.FNV1A("STUB"), exports.fnv1a.FNV1A("INTERNAL"), exports.fnv1a.FNV1A("PRODUCT")];
exports.net_fragment_security_check_point = function(buffer, socket, license){
  if(!exports.net_fragment_is_valid(buffer))
    return false;

  let ip = exports.ip(socket);

  let caller      = exports.net_fragment_caller(buffer);
  let owner       = exports.net_fragment_owner(buffer); // We don't care what owner is it's just a hash to the name of the rijin account used.
  let regression  = exports.net_fragment_regression(buffer);
  let security    = exports.net_fragment_security(buffer);
  let extra_flags = exports.net_fragment_extra_flags(buffer);

  let extra_flags_byte2 = (extra_flags >> 8) & 0xFF;
  if(extra_flags_byte2 != 0){
    console.log(`\n\nSECURITY REASON: ${extra_flags_byte2}`);
  }

  let bits_set = 0;
  for(let i = 0; i < 31; i++){
    if(security & (1 << i))
      bits_set++;
  }

  if(license == undefined){
    if(bits_set != 2)
      exports.report_detection(undefined, socket, "RTP_TRIP_UNLICENSED_SECURITY_VIOLATION", extra_flags_byte2);

    return false;
  }

  // These should never be zero.
  if(security <= 0 || regression <= 0 || caller <= 0){
    exports.print(socket, `security: ${security}, regression: ${regression}, caller: ${caller}`);
    exports.report_detection(license, socket, "RTP_TRIP_NET_FRAG_TAMPERED", extra_flags_byte2);
    return true;
  }

  if(license.security_violation){
    license.security_violation = undefined;
    console.log("security_violation");
    return true;
  }

  if(license.regression == undefined || license.regression.has == undefined){
    license.regression = new Set();
  }
  else{
    // If this value repeats then it's abuse from:
    // Fiddler, wireshark or some other form of network related hack.
    if(license.regression.has(regression) && regression !== exports.fnv1a.prime86){
      exports.print(socket, `security: regression detected: ${regression.toString(16)}`);
      exports.connection_logs("regression_failure", socket, license, `regression is ${regression.toString(16)}`);
      //exports.report_detection(license, socket, "RTP_TRIP_NET_FRAG_TAMPERED_REGRESSION");
      return true;
    }

    let max_history = 2;
    // Remove entries after X.
    if(license.regression.size >= max_history){
      const values = license.regression.values();
      for(let i = 0; i <= max_history - 1; i++)
        values.next();

      values.next().done ? null : license.regression.delete(values.next().value);
    }

    license.regression.add(regression);
  }

  let valid_caller = false;
  for(const id of exports.valid_caller_hashes){
    if(id == caller){
      valid_caller = true;
      break;
    }
  }

  if(!valid_caller){
    exports.report_detection(license, socket, "RTP_TRIP_NET_FRAG_CALLER_INVALID");
    return true;
  }

  if(bits_set != 2){
    exports.report_detection(license, socket, "TRIP_TRIP_SECURITY_VIOLATION", extra_flags_byte2);
    return true;
  }

  return false;
}

exports.net_fragment_is_valid = function(buffer){
  if(buffer == undefined || buffer.len < exports.sizeof_net_fragment){
    exports.print(undefined, "[-] net_fragment_is_valid failed 0", true);
    return false;
  }

  if(exports.net_fragment_magic(buffer) != exports.structs.net_fragment_magic){
    exports.print(undefined, "[-] net_fragment_is_valid failed 1", true);
    console.log(exports.net_fragment_magic(buffer), exports.structs.net_fragment_magic);
    return false;
  }

  return true;
}

// This will wait for a response which can be bad in certain cases.
exports.request = function(method, url, header){
  try{
    return sync_request(method, url, header);
  }
  catch{
  }

  return {error: true, statusCode: 1337};
}

exports.request_async = async function(method, url, header){
  try{
    return sync_request(method, url, header);
  }
  catch{
  }

  return {error: true, statusCode: 1337};
}

exports.send_api_req = async function(json, debug_print){
  if(json === undefined){
    exports.print(undefined, "[-] You're missing the cmd_name or json data");
    return undefined;
  }

  if(json.access_key === undefined){
    exports.print(undefined, "[-] Missing access_key");
    return undefined;
  }

  if(json.cmd == undefined){
    exports.print(undefined, "[-] Missing cmd");
    return undefined;
  }

  if(json.ip == undefined){
    exports.print(undefined, "[-] send_api_req missing ip in json data");
    json.ip = "";
  }

  try{

    const str_json = JSON.stringify(json, null, 2);

    //exports.print(undefined, "[+] API Request: " + str_json.replaceAll(exports.rlm_api_key, "*removed for print*").replaceAll(json.ip, "*removed for print*"));

    const response = await fetch(exports.rlm_api_url,
    {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: str_json,
    });

    const text = await response.text();

    //if(debug_print != undefined){
    //  if(debug_print){
    //    exports.print(undefined, `${text}`);
    //  }
    //}

    return text;
  }
  catch(err){
    exports.print(undefined, `[-] API Request: ${err}\n${json}`);
    return "ERROR: " + err;
  }

  return undefined;
}


exports.ip = function(socket){
  if(socket == undefined)
    return undefined;

  return socket.headers['cf-connecting-ip'] || socket.headers['x-forwarded-for'] || socket.connection.remoteAddress;
}

exports.cloudflare_cidr_list = [];
exports.update_cloudflare_cidr_list = function(socket){
  if(!exports.cloudflare_only)
    return;

  console.log(`[!] ${colors.brightYellow("CLOUDFLARE")}: Fetching cloudflare IPv4/IPv6 list...`);

  const req = exports.request("GET", "https://api.cloudflare.com/client/v4/ips", {});

  if(req == undefined || req.statusCode != 200){
    exports.print(undefined, "[-] CLOUDFLARE: Failed to fetch cloudflare IPv4/IPv6 list!!!", true);
    return;
  }

  try{
    const json_object = JSON.parse(req.getBody().toString());

    if(json_object == undefined)
      throw "json_object == undefined";

    if(json_object.result == undefined)
      throw "json_object.result == undefined";

    if(json_object.result.ipv4_cidrs == undefined)
      throw "json_object.result.ipv4_cidrs == undefined";

    if(json_object.result.ipv6_cidrs == undefined)
      throw "json_object.result.ipv6_cidrs == undefined";

    // clear all entries
    while(exports.cloudflare_cidr_list.length > 0)
      exports.cloudflare_cidr_list.pop();

    for(const i in json_object.result.ipv4_cidrs)
      exports.cloudflare_cidr_list.push(json_object.result.ipv4_cidrs[i]);

    for(const i in json_object.result.ipv6_cidrs)
      exports.cloudflare_cidr_list.push(json_object.result.ipv6_cidrs[i]);

    console.log(`[+] ${colors.brightYellow("CLOUDFLARE")}: ${colors.brightGreen("OK")} -- ${colors.brightYellow(json_object.result.ipv4_cidrs.length)} IPv4, ${colors.brightYellow(json_object.result.ipv6_cidrs.length)} IPv6`);
  }
  catch(e){
    exports.print(undefined, `[-] CLOUDFLARE: Failed to parse list (${e})`, true);
  }
}

exports.is_cloudflare_connection = function(socket){
  if(socket.headers['cf-ray'] == undefined || socket.headers['cf-connecting-ip'] == undefined)
    return false;

  // we dont have a cidr list yet, just let them in...
  if(exports.cloudflare_cidr_list.length == 0)
    return true;

  // ip address must be within the cloudflare cidr range
  for(const i in exports.cloudflare_cidr_list)
    if(ip_range_check(socket.connection.remoteAddress, exports.cloudflare_cidr_list[i]))
      return true;

  return false;
}

exports.is_private_ip = function(ip) {
  return ip_range_check(ip, [
    '10.0.0.0/8',
    '172.16.0.0/12',
    '192.168.0.0/16',
    '127.0.0.0/8',
    '169.254.0.0/16',
    '::1/128',
    'fc00::/7'
  ]);
}

exports.get_icon_resource = function(icon_name){
  let icon_path = `./logos/${icon_name}.png`;
  let unk_path  = `./logos/unknown.png`;

  if(fs.existsSync(icon_path))
    return fs.readFileSync(icon_path);
  else if(fs.existsSync(unk_path))
    return fs.readFileSync(unk_path);

  return undefined;
}

exports.time = function(){
  return Math.floor(+new Date() / 1000);
  //const hr_time = process.hrtime();
  //return (hr_time[0] * 1000000 + hr_time[1] / 1000) / 1000 / 1000;
}

exports.print = function(socket, str, error){
  if(str == undefined){
    console.log("\n\n\n\n[-] You messed up the shared.print function call\n\n\n\n");
    return;
  }

  if(socket != undefined)
    str = str.replace("@IP", "[" + exports.ip(socket) + "]");
  else{
    // Hacky but it works
    str = str.replace("@IP: ", "");
    str = str.replace("@IP:", "");
    str = str.replace("@IP ", "");
    str = str.replace("@IP", "");
  }

  if(error != undefined && error == true)
    str = colors.bgRed(str);

  console.log(str);
}

exports.internal_send_discord_webhook = async function(url, content){
  if(true || exports.dev_mode)
    return;

  // Try catch cause discord is pasted sometimes.
  try{
    if(url == undefined || content == undefined || content == null || content.length == undefined){
      console.log("[-] send_discord_webhook bad data");
      return false;
    }
    
    // Setup the embed json.
    var json = {
      "username": "RijiN",
      "avatar_url": "https://rijin.solutions/red_icon.png",
      "content": content
    }; 

    // Send off the request! We don't really care about the out come.
    var response = await fetch(url, {
      method: "POST",
      headers: {
          "Content-Type": "application/json",
      },
      body: JSON.stringify(json),
    });

    if(response == undefined)
      return false;

    var dat = await response.text();
    return true;
  }
  catch(err){

    console.log("[-] send_discord_webhook error sending message");
    console.log(err);
  }

  return false;
}

exports.send_discord_webhook = async function(where, content){
  if(exports.dev_mode)
    return;

  if(where == "security")
    exports.internal_send_discord_webhook(settings.wh_security_url, content);
  else if(where == "sid_query")
    exports.internal_send_discord_webhook(settings.wh_sid_query_url, content);
  else if(where == "sid_submit")
    exports.internal_send_discord_webhook(settings.wh_sid_submit_cheater_url, content);
  else if(where.startsWith("login"))
    exports.internal_send_discord_webhook(settings.wh_logins_url, content);
  else if(where == "submit_error")
    exports.internal_send_discord_webhook(settings.wh_submit_error_code, content);
  else if(where == "product_exceptions")
    exports.internal_send_discord_webhook(settings.wh_product_exceptions, content);
}

exports.gc = function(){
  if(global.gc == undefined){
    console.log(colors.brightYellow("[-] WARNING: Memory cleanup cannot run because --expose-gc is not defined"));
    return;
  }

  const old_head = process.memoryUsage().heapUsed;
  global.gc();
  const new_heap = process.memoryUsage().heapUsed;

  if(old_head - new_heap > 0)
    console.log(colors.bgBlue(`[!] GC Free ${Math.round((old_head - new_heap) / 1024 / 1024 * 100) / 100} MB (new heap: ${Math.round(new_heap / 1024 / 1024 * 100) / 100}MB)`));
}

exports.rtp_name = function(detection_id){
  if(detection_id == undefined)
    return "(invalid)";

  switch(detection_id){
    default: break;
    case exports.fnv1a.FNV1A("RTP_TRIP_RIJIN_IN_IDA_HISTORY_LOADER"): return "IDA History has the loader inside.";
    case exports.fnv1a.FNV1A("RTP_TRIP_RIJIN_IN_IDA_HISTORY_STUB"): return "IDA History has the stub inside.";
    case exports.fnv1a.FNV1A("RTP_TRIP_MEMORY_TRAVERSED"): return "Memory traversed.";
    case exports.fnv1a.FNV1A("RTP_TRIP_TEST_SIGNING_ENABLED"): return "Test signing enabled.";
    case exports.fnv1a.FNV1A("RTP_TRIP_KERNEL_DEBUGMODE_ENABLED"): return "Kernel debugmode enabled.";
    case exports.fnv1a.FNV1A("RTP_TRIP_PATCH_FAIL"): return "Patch failed";
    case exports.fnv1a.FNV1A("RTP_TRIP_PATCH_REMOVED"): return "Patch removed";
    case exports.fnv1a.FNV1A("RTP_TRIP_PAUSED_PROCESS"): return "Process was paused.";
    case exports.fnv1a.FNV1A("RTP_THREAD_CREATE_FAIL"): return "Failed to create thread.";
    case exports.fnv1a.FNV1A("RTP_SUSPENDED_THREAD_RAN"): return "Suspended thread ran";
    case exports.fnv1a.FNV1A("RTP_TRIP_DBG_PRESENT_BP"): return "IsDebuggerPresent has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_CR_DBG_PRESENT_BP"): return "CheckForRemoteDebuggerPresent has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_FIND_WINDOW_BP"): return "FindWindow has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_GETPROCADDR_BP"): return "GetProcAddress has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_WPM_BP"): return "WriteProcessMemory has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_RPM_BP"): return "ReadProcessMemory has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TICKCOUNT_BP"): return "GetTickCount has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_WINAPI_NETWORK_BP"): return "A WinAPI network function has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_MISC_BP"): return "A WinAPI function has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_PEB_BEING_DBG"): return "PEB BeingDebugged is true (Process being debugged)";
    case exports.fnv1a.FNV1A("RTP_TRIP_WINAPI_HOOKED"): return "A WinAPI function has been hooked.";
    case exports.fnv1a.FNV1A("RTP_TRIP_PROTECTION_HOOKED"): return "An RTP function has been hooked";
    case exports.fnv1a.FNV1A("RTP_TRIP_PROTECTION_BP"): return "An RTP function has a breakpoint placed on it.";
    case exports.fnv1a.FNV1A("RTP_TRIP_DISGUISED_VM"): return "Was opened on a VM that is trying to be hidden from detections.";
    case exports.fnv1a.FNV1A("RTP_TRIP_RIJIN_IN_DBG_HISTORY"): return "x64 debugger had RijiN in its history.";
    case exports.fnv1a.FNV1A("RTP_TRIP_IDA_FILES_IN_BASE_DIR"): return "IDA database files present in base directory.";
    case exports.fnv1a.FNV1A("RTP_TRIP_DBG_PROCESS_PRESENT"): return "Debugger present was open.";
    case exports.fnv1a.FNV1A("RTP_TRIP_DBG_IDA_PROCESS_PRESENT"): return "IDA Process was open.";
    case exports.fnv1a.FNV1A("RTP_TRIP_DBG_X96_PROCESS_PRESENT"): return "x64 process was open.";
    case exports.fnv1a.FNV1A("RTP_TRIP_QEMU_VM"): return "QEMU VM";
    case exports.fnv1a.FNV1A("RTP_TRIP_WEB_DBG_PROCESS_PRESENT"): return "Web debugger was open.";
    case exports.fnv1a.FNV1A("RTP_TRIP_LUA_MODULES_DETECTED"): return "LUA modules was detected.";
    case exports.fnv1a.FNV1A("RTP_THREAD_CREATE_FAIL_SECONDARY"): return "Thread create failed secondary.";
    case exports.fnv1a.FNV1A("RTP_TRIP_TAMPERED"): return "Protection tampered";
    case exports.fnv1a.FNV1A("RTP_TRIP_TIME_TRAP_TRIGGERED"): return "Time check failure.";
    case exports.fnv1a.FNV1A("TRIP_TRIP_TIME_TRAP_TRIGGERED"): return "Time check failure.";
  }

  return detection_id.toString(16);
}

exports.error_code_name = function(error_code){
  const error_list = [
    "ERR_TIMEOUT",
    "ERR_LOGIN_USERNAME_TOO_SHORT",
    "ERR_LOGIN_PASSWORD_TOO_SHORT",
    "ERR_LOGIN_USERNAME_TOO_LARGE",
    "ERR_LOGIN_PASSWORD_TOO_LARGE",
    "ERR_ERROR_MESSAGE_FROM_SERVER",
    "ERR_STUB_FAILED_TO_CREATE_FILE_MAPPING",
    "ERR_STUB_SELECTED_OBJECT_BIGGER_THAN_MAX_OBJECTS",
    "ERR_STUB_FAILED_TO_MAP_VIEW_OF_FILE",
    "ERR_STUB_REQUEST_FAILED",
    "ERR_STUB_WRITE_BINARY_FAILED",
    "ERR_STUB_CREATE_FILE_FAILED",
    "ERR_STUB_FAILED_TO_OPEN_FILE_MAPPING",
    "ERR_STUB_FAILED_TO_LOAD_TIMEOUT",
    "ERR_INJECT_STUB_DATA_NULLPTR",
    "ERR_INJECT_OBJECT_NULLPTR",
    "ERR_INJECT_OUTPUT_FAILED",
    "ERR_INJECT_OPEN_PROCESS_FAILED",
    "ERR_INJECT_INFO_FETCH_FAILED",
    "ERR_INJECT_CREATE_TOOL_HELP_32_SNAPSHOT_FAILED",
    "ERR_INJECT_FETCH_REQUIRE_IMPORT_MODULE_FAILED",
    "ERR_INJECT_FETCH_REQUIRE_IMPORT_FUNCTION_FAILED",
    "ERR_INJECT_WAIT_FOR_CHEAT_TO_LOAD_TIMEOUT",
    "ERR_INJECT_CANNOT_FIND_TARGET_MODULE",
    "ERR_INJECT_CROSS_CHECK_CPU_MODE_FAILED",
    "ERR_INJECT_CANNOT_FIX_PAGE_PERMISSIONS",
    "ERR_INJECT_FAILED_TO_WRITE_BUFFER_TO_PAGE",
    "ERR_INJECT_ADD_HOOK_FAILED",
    "ERR_INJECT_FAILED_TO_RESOLVE_REQUIRED_IMPORTS",
    "ERR_INJECT_FAILED_TO_GET_MODULE_ADDRESS_FOR_REQUIRED_IMPORTS",
    "ERR_INJECT_FAILED_TO_GET_EXPORT_FOR_REQUIRED_IMPORTS",
    "ERR_INJECT_CANNOT_WRITE_HOOKS",
    "ERR_INJECT_CANNOT_WRITE_HOOK_TRAMPOLINE",
    "ERR_INJECT_CANNOT_WRITE_HOOK_ASM",
    "ERR_INJECT_STUB_LOAD_FROM_THREAD_FAILED",
    "ERR_CREATE_WORKER_THREAD_FAILED",
    "ERR_LICENSE_HAS_EXPIRED",
    "ERR_DXGI_DUMP_FAILED",
    "ERR_PROCEDURE_SETUP_FAILED",
    "ERR_PROCEDURE_WRITE_SIGNATURES_FAILED",
    "ERR_PROCEDURE_WRITE_HOOKS_FAILED",
    "ERR_SIGSCAN_FAILED",
    "ERR_PROCEDURE_SET_MODULE_TIMEOUT",
    "ERR_PROECURE_ADD_HOOK_FAILED",
    "ERR_PROCEDURE_WRITE_POINTERS_FAILED",
    "ERR_GLOBALDATA_FETCH_FAILED",
    "ERR_PROCEDURE_IMPORT_FAILED",
    "ERR_VAC_BYPASS_FAILED",
    "ERR_GET_MAC_ADDRESS_FAILED",
    "ERR_GET_CPU_BRAND_NAME_NO_SUPPORT",
    "ERR_GET_ARP_GET_SIZE_FAILED",
    "ERR_GET_ARP_GET_ADAPTER_FAILED",
    "ERR_GET_ARP_NO_ROUTER_FOUND",
    "ERR_GET_ARP_PING_FAILED",
    "ERR_FAILED_TO_LOAD_BITMAP",
    "ERR_NO_LOAD_PROCEDURE",
    "ERR_BYFRON_BYPASS_REINJECT_TARGET_MODULE_NOT_FOUND",
    "ERR_BYFRON_BYPASS_REINJECT_TARGET_MODULE_INVALID_PE_HEADER",
    "ERR_BYFRON_BYPASS_REINJECT_MEM_FAILED",
    "ERR_CREATE_PROCESS_FAILED",
    "ERR_DUPLICATE_HANDLE_FAILED",
    "ERR_OW_MODULE_ADDRESS_FAILED",
    "ERR_INJECT_WAIT_FOR_CHEAT_LOAD_PTR_FAIL",
    "ERR_VAC_BYPASS_SIG_FAIL",
    "ERR_ACTIVE_AV",
    "ERR_FAILED_TO_GET_WINDOW_PATH",
    "ERR_GMOD_WRONG_BRANCH",
    "ERR_SHORT_HOOK_IMPOSSIBLE",
    "ERR_TF2_WRONG_BRANCH",
    "ERR_TF2_HACK_FIX",
    "ERR_VAC_BYPASS_FAILED_STEAM_HANDLE",
    "ERR_VAC_BYPASS_FAILED_STEAM_SERVICE",
    "ERR_VAC_BYPASS_FAILED_CRASH_HANDLER_TIMEOUT",
    "ERR_STEAM_UNABLE_TO_RESUME_WHILE_SUSPENDED",
    "ERR_FAILED_TO_CLOSE_RUNDLL",
    "ERR_FAILED_TO_CREATE_LOGIN_PROMPT",
    "ERR_FAILED_BASE_ADDRESS_ZERO",
    "ERR_VAC_BYPASS_FAILED_BASE_ADDRESS_ZERO",
    "ERR_VAC_BYPASS_FAILED_TO_CLOSE_PROCESSES",
    "ERR_VAC_BYPASS_FAILED_TO_CLOSE_TARGET_PROCESS",
    "ERR_VAC_BYPASS_FAILED_TO_LAUNCH_STEAM",
    "ERR_VAC_BYPASS_FAILED_SUSPEND_STEAM",
    "ERR_INJECT_GET_TARGET_PID_FAIL",
    "ERR_PROECURE_ADD_HOOK_FAILED_NO_MODULE_ADDR",
    "ERR_PROECURE_ADD_HOOK_FAILED_SIG_FAIL"
  ];

  if(error_code < 0 || error_code >= error_list.length)
    return "(unknown code): " + error_code.toString();

  return error_list[error_code].replace("ERR_", "").replaceAll("_", " ") + " (" + error_code.toString() + ")";
}

exports.get_status_names = function(name_code, what_code){
  var name = "0x" + name_code.toString(16);
  var what = "0x" + what_code.toString(16);

  const names_list = ["LOADER", "STUB", "PRODUCT", "NOTICE", "VAC BYPASS"];

  const what_list = ["STUB_START_EXECUTE",
     "STUB_LOADED",
     "WAIT_FOR_EXECUTE",
     "DXGIDMP",
     "TF2_64_HL2_PROCESS",
     "TF2_64_32_VERSION",
     "UNABLE_TO_AUTO_LAUNCH_GAME",
     "RUN_VAC_BYPASS",
     "RUN_VAC_BYPASS_ALREADY_LOADED",
     "RUN_VAC_BYPASS_STARTING_STEAM",
     "RUN_VAC_BYPASS_WAITING_FOR_STEAM",
     "RUN_VAC_BYPASS_LICENSE_COPY",
     "RUN_VAC_BYPASS_STATUS",
     "RUN_VAC_BYPASS_ERROR",
     "START_TARGET_PROCESS",
     "WRONG_GMOD_VERSION",
     "WAITING_FOR_TARGET_PROCESS",
     "TARGET_PROCESS_HANDLE",
     "TARGET_PROCESS_X86_CHECK",
     "RUN_PROCEDURE",
     "PROCEDURE_RAN",
     "RECEIVE_AND_WRITE",
     "STUB_FINISHED",
     "PRODUCT_STAGING_LOADED",
     "PRODUCT_SHIPPING_LOADED",
     "PRODUCT_SUB_EXPIRED",
     "EXCEPTION",
     "TF2_SEQDESC_CRASH",
     "VAC_BYPASS_INIT",
     "VAC_BYPASS_GET_SYSTEM_INFO_WORKS",
     "VAC_BYPASS_SPOOFER_WORKS",
     "STEAM_SPOOFED",
     "VAC_BYPASS_SPOOFER_HOOKS_START",
     "VAC_BYPASS_SPOOFER_HOOKS_END",
     "VAC_BYPASS_PATCHING",
     "VAC_BYPASS_PATCHED",
     "VAC_BYPASS_PATCH_TOO_LATE",
     "RUN_VAC_BYPASS_STEAM_SERVICE_PRESENT_FAIL",
     "HEARTBEAT"];

  for(var n of names_list){
    if(exports.fnv1a.FNV1A(n) == name_code){
      name = n;
      break;
    }
  }

  for(var w of what_list){
    if(exports.fnv1a.FNV1A(w) == what_code){
      what = w;
      break;
    }
  }

  return {name: name, what: what};
}

// This will load a JSON table from file without the risk of memory leaking or slow GC collections.
exports.load_json_file = function(file){
  if(file == null || file == undefined)
    return undefined;

  try{
    if(!fs.existsSync(file))
      return undefined;

    let buffer = fs.readFileSync(file, {encoding: 'utf8'});
    if(buffer == undefined)
      return undefined;

    let json = JSON.parse(buffer);
    console.log(`shared.load_json_file: (loaded: ${file})`);
    return json;
  }
  catch(err){
    console.log(`shared.load_json_file: ${err} - ${file}`);
  }
  return undefined;
}

exports.free_json_table = function(table){
  if(table == undefined || table == null)
    return;

  try{
    for(var key in table){
      delete table[key];
      table[key] = null;
    }

    delete table;
    table.length = 0;
    table = null;
    exports.gc();
  }
  catch(err){
    console.log(`shared.free_json_table: Error: ${err}`);
  }
}

exports.random_number = function(min, max){
  min = Math.ceil(min);
  max = Math.floor(max);
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

exports.is_server_cpu = function(cpu_name){
  if(cpu_name == undefined)
    return false;

  const server_cpu = [
    "AMD EPYC 7252",
    "AMD EPYC 7302",
    "AMD EPYC 7402",
    "AMD EPYC 7502",
    "AMD EPYC 7642",
    "AMD EPYC 7702",
    "AMD EPYC 7742",
    "AMD EPYC 7F32",
    "AMD EPYC 7F52",
    "AMD EPYC 7F72",
    "AMD EPYC 7313",
    "AMD EPYC 7443",
    "AMD EPYC 7513",
    "AMD EPYC 7713",
    "AMD EPYC 9654",
    "AMD EPYC 9354",
    "AMD EPYC 9274F",
    "AMD EPYC 9174F",
    "Intel Xeon Platinum 8276",
    "Intel Xeon Platinum 8280",
    "Intel Xeon Gold 6230",
    "Intel Xeon Gold 6248",
    "Intel Xeon Silver 4210",
    "Intel Xeon Silver 4310",
    "Intel Xeon Bronze 3204",
    "Intel Xeon Platinum 8352V",
    "Intel Xeon Gold 6338",
    "Intel Xeon Gold 6348",
    "Intel Xeon Platinum 8460Y",
    "Intel Xeon Platinum 8490H",
    "Intel Xeon E-2136",
    "Intel Xeon E-2176G",
    "Intel Xeon E-2288G",
    "Intel Xeon E-2388G",
    "Intel Xeon D-2123IT",
    "Intel Xeon D-2146NT",
    "Intel Xeon D-2183IT",
    "Ampere Altra Q80-33",
    "Ampere Altra Max M128-30",
    "Amazon Graviton2",
    "Amazon Graviton3",
    "IBM POWER9",
    "IBM POWER10"
  ];

  cpu_name = cpu_name.toLowerCase();
  for(const l of server_cpu){
    let entry = l.toLowerCase();
    if(entry == cpu_name || cpu_name.includes(entry))
      return true;
  }

  return false;
}

exports.is_backing_up = false;

let last_file_date = undefined;
setInterval(async function(){
  if(last_file_date == exports.get_file_formatted_date())
    return;

  exports.is_backing_up = true;
  try{
    if(fs.existsSync("./screenshots_old"))
      fs.rmSync(base_dir, {recursive: true, force: true});
  }
  catch{}

  const process_table                = [exports.screenshots_location, exports.logs_location];
  const max_compression_history_days = 60;
  const max_year_length = 1825; // 5 years.

  try{
    for(const entry of process_table){
    if(!fs.existsSync(entry))
      continue;

    console.log(`Compressing old entries in ${entry}`);
    for(let i = 2; i <= max_compression_history_days; i++){
      let old_date = exports.get_file_formatted_date(i);
      let base_dir = `${entry}/${old_date}`;
      if(!fs.existsSync(base_dir)){
        continue;
      }

      await compressing.tgz.compressDir(base_dir, `${entry}/${old_date}.tar.gz`);
      fs.rmSync(base_dir, {recursive: true, force: true});
    }
    console.log(`Finished compressing entries in ${entry}`);

    for(let i = 365; i <= max_year_length; i++){
      let base_file = `${entry}/${exports.get_file_formatted_date(i)}.tar.gz`;
      if(!fs.existsSync(base_file))
        continue;

      try{
        console.log(`Deleting old log file: ${base_file}`);
        fs.unlinkSync(base_file);
        console.log(`Deleted old log file: ${base_file}`);
      }
      catch(err){
        console.log(`Failed to delete file ${base_file} -> ${err}`);
      }
    }
    }
  }
  catch{
    //
  }

  exports.is_backing_up = false;

  last_file_date = exports.get_file_formatted_date();
}, 1000);

exports.should_collect_info = function(socket, license, username = undefined){
  if(socket == undefined)
    return true;

  let ip_addr = exports.ip(socket);
  if(ip_addr.includes("99.22.22."))
    return false;

  if(license !== undefined)
    username = license.identity.toLowerCase();

  if(username !== undefined)
    username = username.toLowerCase();

  if(username == "senator" || username == "durrud" || username == "admin")
    return false;

  return true;
}

setInterval(function(){
  exports.update_cloudflare_cidr_list();
},exports.housekeeping_ms);

setInterval(async function(){
  exports.gc();
  let file_handles = process._getActiveHandles();
  let usage        = process.memoryUsage();

  console.log(`RSS: ${Math.round(usage.rss / 1024 / 1024)} MB, handles: ${file_handles.length}`);
}, 60000);