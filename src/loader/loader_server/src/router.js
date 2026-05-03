// core
const net = require("net");
const fs = require('fs');

// custom
const sh = require("./shared.js");
const license = require("./license.js");
const cloud_config = require("./cloud_config.js");
const tf_cheater_list = require("./tf_cheater_list.js");
const tf_lobby = require("./tf_lobby.js");
const iden     = require("./identifier.js");
const gen = require("./gen.js");
const c = require("./c.js");
const stats      = require("./stats.js");
const cache      = require("./cache.js");
const error_str = require("./errors.js");

function find_best_dll(file, extensions_buffer){
  const struct = sh.structs.cpu_extensions;

  const sse     = extensions_buffer.readUInt8(struct.sse);
  const sse2    = extensions_buffer.readUInt8(struct.sse2);
  const sse3    = extensions_buffer.readUInt8(struct.sse3);
  const ssse3   = extensions_buffer.readUInt8(struct.ssse3);
  const sse41   = extensions_buffer.readUInt8(struct.sse41);
  const sse42   = extensions_buffer.readUInt8(struct.sse42);
  const avx     = extensions_buffer.readUInt8(struct.avx);
  const avx2    = extensions_buffer.readUInt8(struct.avx2);
  const avx512  = extensions_buffer.readUInt8(struct.avx512);

  // add prefix to bin folder
  file = "bin/" + file;

  if(sse2 && fs.existsSync(file + "_sse2.dll"))
    file += "_sse2.dll";
  else if(sse42 && fs.existsSync(file + "_sse4.2.dll")) // Temp staging support.
    file += "_sse4.2.dll";
  else
    file += ".dll";

  if(!fs.existsSync(file)){
    sh.print(undefined, `[-] find_best_dll: ${file} doesn't exist`, true);
    return undefined;
  }

  stats.submit_math_extension(file);
  sh.print(undefined, `[!] find_best_dll: Using file ${file}`);
  return file;
}

exports.handle_c_net_receive_info = function(socket, fragment){
  const struct = sh.structs.c_net_receive_info;
  const buffer = Buffer.alloc(struct.sizeof).fill(0);
  
  sh.general_log("receive_info", `${sh.ip(socket)}`);
  sh.print(socket, `handle_c_net_receive_info: @IP`);
  c.write_struct_to_buffer(buffer, struct, {
    timestamp: sh.fnv1a.FNV1A(new Date().toISOString().slice(0,10)),
    loader_hash: 0,
    update_mode: sh.update_mode,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, buffer);
}

// This is a table that is to prevent cred stuffing attacks.
exports.restrict_license_ips = {};

// This is a table to prevent license request spammage.
exports.license_request_time_limter = {};

exports.handle_c_net_request_license = function(socket, fragment){
  const struct    = sh.structs.c_net_request_license;
  const in_buffer = sh.net_fragment_data(fragment);
  const ip_addr   = sh.ip(socket);

  if(sh.is_backing_up)
    return sh.net_fragment_error_msg(socket, "(1): We're conducting maintenance. Please check back later.", true);

  license.delete_self(socket);

  if(false && exports.license_request_time_limter[ip_addr] != undefined){
    if(exports.license_request_time_limter[ip_addr] > sh.time()){
      sh.print(socket, "handle_c_net_request_license: @IP sent another license request too fast");
      return sh.net_fragment_error_msg(socket, "You are sending too many login requests.\nPlease wait and try again in 10 seconds.", true);
    }
  }

  exports.license_request_time_limter[ip_addr] = sh.time() + 3;

  // Related to anti-cred stuffing attacks.
  if(false && exports.restrict_license_ips[ip_addr] != undefined){
    if(exports.restrict_license_ips[ip_addr].limit_time != undefined){
      if(exports.restrict_license_ips[ip_addr].limit_time > sh.time()){
        sh.print(socket, "handle_c_net_request_license: @IP is being limited on login requests");
        return sh.net_fragment_error_msg(socket, "You've attempted login in too many accounts.\nPlease verify if your login details are correct.\nTry again in 10 minutes.", true);
      }
    }
  }

  console.log("\n[Request License]\n");
  sh.general_log("login", `${ip_addr} requesting a license`);

  sh.print(socket, `handle_c_net_request_license: @IP requesting license`);
  if(in_buffer == undefined || in_buffer.length != struct.sizeof){
    sh.print(socket, "handle_c_net_request_license: @IP buffer is undefined or the length mismatches");
    return sh.net_fragment_error_msg(socket, "Invalid login request.", false);
  }
  
  // Check that the client version meets our minimum requirements
  const client_version = in_buffer.readUInt32LE(struct.client_version);
  const client_version_delta = Math.abs(client_version - sh.min_client_version);
  if(client_version_delta > 100){
    sh.general_log("login", `${ip_addr} using a corrupted loader: ${client_version}/${sh.min_client_version}`);
    license.delete_self(socket);
    socket.destroy();
    return sh.net_fragment_error_msg(socket, "Your loader is corrupted.\n\nPlease redownload the loader from the dashboard.", true);
  }

  if(client_version < sh.min_client_version){
    sh.general_log("login", `${ip_addr} using an outdated loader: ${client_version}/${sh.min_client_version}`);
    sh.print(socket, "handle_c_net_request_license: @IP loader out of date.");
    return sh.net_fragment_error_msg(socket, "Your loader is out of date.\n\nPlease download the new loader from the dashboard.", true);
  }

  let   username = in_buffer.slice(struct.username, struct.username + sh.structs.sizeof_username).toString().replace(/\0[\s\S]*$/g,'').toLowerCase();
  const password = in_buffer.slice(struct.password, struct.password + sh.structs.sizeof_password).toString().replace(/\0[\s\S]*$/g,'');
  let   uid = 0;

  let username_hash = sh.fnv1a.FNV1A(username.toLowerCase().trim());

  // Prevent cred stuffing attacks by restricting a single IP to 3 logins per 5 minutes.
  if(exports.restrict_license_ips[ip_addr] == undefined){
    exports.restrict_license_ips[ip_addr] = {};
    exports.restrict_license_ips[ip_addr].limit_time = 0;
    exports.restrict_license_ips[ip_addr].list       = [username_hash];
  }
  else{
    if(!exports.restrict_license_ips[ip_addr].list.includes(username_hash))
      exports.restrict_license_ips[ip_addr].list.push(username_hash);
  }

  if(exports.restrict_license_ips[ip_addr].length >= 3){
    sh.general_log("login_security", `${ip_addr} tried to login to 3 types of accounts: ${JSON.stringify(xports.restrict_license_ips[ip_addr].list, null, 2)}`);
    exports.restrict_license_ips[ip_addr].limit_time = sh.time() + 300;
    exports.restrict_license_ips[ip_addr].list       = [];
  }

  // hardware related
  const hwid_mac_address_hash     = in_buffer.readUInt32LE(struct.hwid_mac_address_hash);
  const hwid_arp_mac_address_hash = in_buffer.readUInt32LE(struct.hwid_arp_mac_address_hash);
  const hwid_cpu_brand_name       = in_buffer.slice(struct.hwid_cpu_brand_name, struct.hwid_cpu_brand_name + sh.structs.sizeof_hwid_cpu_brand_name).toString().replace(/\0[\s\S]*$/g,'');

  sh.print(socket, `[!] handle_c_net_request_license: @IP hwid_mac_address_hash: "0x${hwid_mac_address_hash.toString(16).toUpperCase()}"`);
  sh.print(socket, `[!] handle_c_net_request_license: @IP hwid_arp_mac_address_hash: "0x${hwid_arp_mac_address_hash.toString(16).toUpperCase()}"`);
  sh.print(socket, `[!] handle_c_net_request_license: @IP hwid_cpu_brand_name: "${hwid_cpu_brand_name}"`);
  sh.print(socket, `[!] handle_c_net_request_license: @IP username: "${username}"`);

  if(hwid_cpu_brand_name == undefined || hwid_cpu_brand_name == null || hwid_cpu_brand_name.length <= 8){
    sh.report_detection(undefined, socket, "RTP_TRIP_CLIENT_REPORTED_INVALID_CPU");
    license.delete_self(socket);
    socket.destroy();
    return sh.net_fragment_error_msg(socket, "Client error, please try again.", true);
  }

  if(sh.is_server_cpu(hwid_cpu_brand_name)){
    sh.report_detection(undefined, socket, "RTP_TRIP_CLIENT_REPORTED_SERVER_CPU");
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `Error: RijiN does not support computers with Server CPUs.`, true);
  }

  let test_hwid_cpu_brand_name = hwid_cpu_brand_name.toLowerCase();
  if(!test_hwid_cpu_brand_name.includes("amd") && !test_hwid_cpu_brand_name.includes("intel")){
    sh.report_detection(undefined, socket, "RTP_TRIP_CLIENT_REPORTED_UNKNOWN_CPU_TYPE");
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `Error: We don't support your current CPU.`, true);
  }

  if(  hwid_mac_address_hash == undefined
    || hwid_arp_mac_address_hash == undefined
    || hwid_mac_address_hash <= 1000
    || hwid_arp_mac_address_hash <= 1000){
    sh.report_detection(undefined, socket, "RTP_TRIP_CLIENT_REPORTED_INVALID_MAC_ADDRESSES");
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `Client error 0xFFFF, please try again.`, true);
  }

  var license_list = [];

  // under dev mode we have all licenses
  if(sh.dev_mode && !sh.test_shipping){
    // for extra security in case dev_mode is some how activated on the remote server
    // do a basic username and password check as a failsafe to ensure no one can obtain all subscriptions for free
    if(username == "dev" && password == "Unaired Clique Judiciary Italicize9 Halt")
      license_list = license.license_list;
    else{
      license.delete_self(socket);
      return sh.net_fragment_error_msg(socket, "DEV_MODE: Invalid login", false);
    }
  }
  else{
    // convert the steamid list into a json ready table
    var steamid_list = [];
    let collect_steamids = sh.should_collect_info(socket, undefined, username);

    if(collect_steamids){
      var steamid_list_size = c.clamp(in_buffer.readUInt32LE(struct.steam_id_list + sh.structs.s_steam_id_list.size), 0, sh.structs.s_steam_id_list.data_sizes.id / c.u32);
          steamid_list_size = c.clamp(steamid_list_size, 0, 1024);

      for(var i = 0; i < steamid_list_size; i++){
        const id = in_buffer.readUInt32LE(struct.steam_id_list + sh.structs.s_steam_id_list.id + (i * c.u32));

        if(id < 1)
          continue;

        sh.print(socket, `[+] handle_c_net_request_license: @IP received steam account: [U:1:${id}]`);
        steamid_list.push(id);
      }
    }

    const time_to_login = sh.time();

    sh.print(socket, `[+] handle_c_net_request_license: @IP RLM Verification ${username}`);

    stats.track_login_start();
    const req = sh.request("POST", sh.rlm_api_url,
      {
        json: {
          access_key:                 sh.rlm_api_key,
          cmd:                        "auth",
          hwid_mac_address_hash:      hwid_mac_address_hash,
          hwid_arp_mac_address_hash:  hwid_arp_mac_address_hash,
          hwid_cpu_brand_name:        hwid_cpu_brand_name,
          steamid_list:               steamid_list,
          ip:                         sh.ip(socket),
          username:                   username,
          password:                   password,
          staging_mode:               sh.staging_mode
        },
      }
    );

    if(req == undefined || req.statusCode != 200){
      sh.general_log("login", `request failed for auth user (${username}): ${req != undefined ? req.statusCode.toString() : "req undefined"}`);
      console.log("\n\n[RLM ERROR]\n\n")
      sh.print(socket, `[+] handle_c_net_request_license: @IP RLM error: ${username} req failed or status code not 200`, true);
      license.delete_self(socket);

      if(req !== undefined && req.error === undefined){
        console.log(req.getBody().toString());
      }

      return sh.net_fragment_error_msg(socket, `Could not contact the RLM authentication service (${req != undefined ? req.statusCode : -1})`, true);
    }

    var time_taken = sh.time() - time_to_login;
    sh.print(socket, `[+] handle_c_net_request_license: @IP RLM request took: ${time_taken}`);
    stats.track_login_end(time_taken, steamid_list);

    sh.general_log("login", `${ip_addr} logged in as ${username}`)

    let privacy_username = username.includes("@") ? username.split('@')[0] + "@redacted.rijin" : username;
    sh.send_discord_webhook("login", `${privacy_username} logged in to ${sh.server_name}.`);

    // Cache steamids so the next possible request doesn't bother networking the steamid list again.
    if(cache.rlm_steamid_cache[username] == undefined){
      cache.rlm_steamid_cache[username] = {};
      cache.rlm_steamid_cache[username].expire = sh.time() + 86400; // 24 hours.
      cache.ip = sh.ip(socket);
      cache.rlm_steamid_cache[username].ids = [];
      for(var entry of steamid_list)
        cache.rlm_steamid_cache[username].ids.push(entry);
    }
    else{
      cache.rlm_steamid_cache[username].expire = sh.time() + 86400;
      for(var entry of steamid_list){
        if(!cache.rlm_steamid_cache[username].ids.includes(entry))
          continue;

        cache.rlm_steamid_cache[username].ids.push(entry);
      }
    }

    try{
      const json_object = JSON.parse(req.getBody().toString());

      console.log(json_object);

      // An invisible lock is essentially used to deter crackers and users with malicious intent
      // It should prove to be quite effective against preventing cheat crackers give up thinking our server
      // Is just timing out
      if(json_object.status == sh.RLM_INVISIBLE_LOCK){
        sh.general_log("login", `dropping: ${ip_addr} ${username} for being security locked.`);
        sh.print(socket, `handle_c_net_request_license: @IP ${username} security locked.`);
        license.delete_self(socket);
        socket.destroy();
        return sh.net_fragment_error_msg(socket, `Error ${sh.random_number(99, 200).toString()}`, true);
      }

      // if the rlm api has a status other than success and they have a message, show it to the user
      if(json_object.status != sh.RLM_SUCCESS){
        if(json_object.msg == undefined)
          json_object.msg = "Unknown";

        sh.general_log("login", `dropping: ${ip_addr} ${username} (status != RLM_SUCCESS) ${json_object.msg}`);
        sh.print(socket, `handle_c_net_request_license: @IP ${username} sending RLM message to user ${json_object.msg}`);
        license.delete_self(socket);
        return sh.net_fragment_error_msg(socket, `RLM: ${json_object.msg}`, true);
      }

      if(json_object.data.length < 1){
        sh.general_log("login", `dropping: ${ip_addr} ${username} (no active licenses)`);
        sh.print(socket, `handle_c_net_request_license: @IP ${username} no active licenses (0)`);
        license.delete_self(socket);
        return sh.net_fragment_error_msg(socket, `No active licenses (0)`, true);
      }

      if(json_object.uid == undefined){
        sh.print(socket, `handle_c_net_request_license: @IP ${username} transfer error (1)`);
        license.delete_self(socket);
        return sh.net_fragment_error_msg(socket, `Transfer error (1)`, true);
      }
      else
        uid = json_object.uid;

      // In cases where the user uses a email to login or spells there username incorrectly
      // We will just set there license username to the forum username
      if(username != json_object.msg.toLowerCase()){
        sh.print(socket, `[!] @IP: updated username from "${username}" to "${json_object.msg.toLowerCase()}"`);

        username = json_object.msg.toLowerCase();
      }

      var added_license_prefixes = [];
      for(const license_prefix in json_object.data){
        const api_license = json_object.data[license_prefix];

        if(api_license == undefined || api_license.access_id == undefined)
          continue;

        if(typeof(api_license.access_id) != "number")
          continue;

        for(const license_id in license.license_list){
          const current_license = license.license_list[license_id];

          if(current_license == undefined || current_license.requires_access_id == undefined)
            continue;

          if(typeof(current_license.requires_access_id) == "object"){
            if(Object.values(current_license.requires_access_id).indexOf(api_license.access_id) == -1)
              continue;
          }
          else if(typeof(current_license.requires_access_id) == "number"){
            if(current_license.requires_access_id != api_license.access_id)
              continue;
          }
          else
            continue;

          // we already added this license
          if(added_license_prefixes.includes(current_license.name))
            continue;

          // Add the api response into this table
          current_license.api_response = api_license;

          license_list.push(current_license);
          added_license_prefixes.push(current_license.name);

          sh.print(socket, `[+] @IP: Adding license "${current_license.name}"`);
        }
      }
    }
    catch(e){
      sh.print(socket, `[-] handle_c_net_request_license: RLM API JSON PARSE FAILED: "${e}"`, true);

      license.delete_self(socket);
      return sh.net_fragment_error_msg(socket, "RLM authentication service returned an unknown error", true);
    }
  }

  if(license_list.length < 1){
    sh.print(socket, `handle_c_net_request_license: @IP ${username} no active licenses (1)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "No active licenses (1)", true);
  }

  // issue the license
  const access = license.create(socket, username, uid, license_list);

  if(access == undefined || access.key == undefined){
    sh.print(socket, `handle_c_net_request_license: @IP ${username} couldn't issue license`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Couldn't issue license", true);
  }
  
  if(access.objects == undefined || access.objects.length < 1){
    sh.print(socket, `handle_c_net_request_license: @IP ${username} no active licenses (2)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "No active licenses (2)", true);
  }

  // Lets track how many people logged on a day.
  stats.submit_login(uid);

  sh.print(socket, `[+] handle_c_net_request_license: @IP ${username} license request accepted!`);
  
  // swap the current structure to the receive license section
  const receive_license = sh.structs.c_net_receive_license;
  const license_object = sh.structs.license_object;
  
  const out_buffer = Buffer.alloc(receive_license.sizeof + (license_object.sizeof * access.objects.length)).fill(0);
  {
    const key = Buffer.from(access.key);
    c.memcpy(key, out_buffer, receive_license.key, 0, receive_license.data_sizes.key);
    delete key;

    c.write_struct_to_buffer(out_buffer, receive_license, {
      license_expire_seconds:   access.expire_seconds,
      objects_size:             access.objects.length
    });

    let offset = receive_license.sizeof;
    let count = 0;

    const find_pos_for_pkg = function(table, name){
      let found_pos = 0;

      let count = 0;
      table.forEach(function(obj){
        if(obj["name"] == name)
          found_pos = count;

        count++;
      });

      return found_pos;
    }

    access.objects.forEach(function(obj){
      if(obj["background_module"] == undefined || !obj["background_module"]){

        // Configure what the name will be
        {
          const name = Buffer.from(obj["name"]);

          c.memcpy(name, out_buffer, offset + license_object.name, 0, Math.min(name.length, license_object.data_sizes.name) );

          if(obj["short_name"] == undefined)
            c.memcpy(name, out_buffer, offset + license_object.short_name, 0, Math.min(name.length, license_object.data_sizes.short_name) );
          else{
            const short_name = Buffer.from(obj["short_name"]);
            c.memcpy(short_name, out_buffer, offset + license_object.short_name, 0, Math.min(short_name.length, license_object.data_sizes.short_name) );
            delete short_name;
          }

          delete name;
        }

        // Override disabled flag from dashboard
        if(obj.api_response != undefined && obj.api_response.disabled != undefined)
          obj["disabled"] = obj.api_response.disabled;

        if(!sh.dev_mode || obj["hide_for_dev"] == undefined || !obj["hide_for_dev"]){
          c.write_struct_to_buffer(out_buffer, license_object, {
            pos:          count,
            name_hash:    sh.fnv1a.FNV1A(obj["name"]),
            target_hash:  sh.fnv1a.FNV1A(obj["target"]),
            expire:       obj["expire"],
            disabled:     obj["disabled"] ? 1 : 0,
            beta:         obj["beta"] ? 1 : 0,
            hypervisor:   obj["hypervisor"] ? 1 : 0,
            spoofer:      obj["spoofer"] ? 1 : 0,
            vac_mode:     obj["vac_mode"] ? 1 : 0,
            vac_pos:      obj["vac_mode"] ? find_pos_for_pkg(access.objects, "vac_bypass") : 0,
            appid:        obj["appid"],
            d3dx:         obj["d3dx"],
            runs_on_stub: obj["runs_on_stub"] ? 1 : 0,
          }, offset);

          if(obj.api_response != undefined && obj.api_response.expire_str != undefined){
            const expire_str = Buffer.from(obj.api_response.expire_str);
            c.memcpy(expire_str, out_buffer, offset + license_object.expire_str, 0, Math.min(expire_str.length, license_object.data_sizes.expire_str) );
          }
        }
      }

      offset += license_object.sizeof;
      count++;
    });
  }

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, out_buffer);
}

exports.handle_c_net_request_stub = function(socket, fragment){
  const struct = sh.structs.c_net_request_stub;
  const in_buffer = sh.net_fragment_data(fragment);

  sh.print(socket, `handle_c_net_request_stub: @IP requesting injection stub`);

  if(!sh.is_valid_request(socket, in_buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", true);

  const access = license.current_license(socket);

  sh.general_log("request_module", `${sh.ip(socket)} requested stub`);
  if(access == undefined){
    sh.print(socket, `handle_c_net_request_stub: @IP no license`);
    return sh.net_fragment_error_msg(socket, error_str.internal_tnd(1), true);
  }

  sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity}`);
  sh.connection_logs("request_module", socket, access, `requested stub`);

  if(access.loader_heart_beat_received == undefined){
    sh.report_detection(access, socket, "RTP_TRIP_REQUEST_MISSING_HEARTBEAT_LOADER");
    return sh.net_fragment_error_msg(socket, "License key invalid", true);
  }

  if(access.received_screenshot == undefined){
    sh.report_detection(access, socket, "RTP_TRIP_NO_SCREENSHOT_RECEIVED");
    return sh.net_fragment_error_msg(socket, "License key invalid 2", true);
  }

  if(access.first_challenge == undefined){
    //sh.report_detection(access, socket, "RTP_TRIP_MISSING_CHALLENGE");
    return sh.net_fragment_error_msg(socket, "License key invalid", true);
  }

  sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity} sending injection stub`);
  
  if(!fs.existsSync(sh.stub_location)){
    sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity} stub image buffer does not exist on the server. Expected location: ${sh.stub_location}`, true);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Stub image buffer does not exist on the server", true);
  }
  
  const stub_buffer = fs.readFileSync(sh.stub_location);
  
  if(stub_buffer == undefined || stub_buffer.length < 2){
    sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity} cannot open stub image buffer.`);
    delete stub_buffer;
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Cannot open the stub image buffer", true);
  }

  // Ensure no debug builds can be loaded on production builds
  if(!sh.dev_mode){
    var export_table = gen.pe_dump_exports(stub_buffer, gen.pe_dump_sections(stub_buffer), false);

    if(export_table == undefined){
      sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity} The stub is not valid.`);
      license.delete_self(socket);
      delete stub_buffer;
      return sh.net_fragment_error_msg(socket, "The stub is not valid", true);
    }

    if(gen.pe_find_export(export_table, "gen_flag_debug_mode") != undefined){
      sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity} stub contains debug information`, true);
      license.delete_self(socket);
      delete stub_buffer;
      return sh.net_fragment_error_msg(socket, "The stub currently contains debugging information which is not currently accessible from your usergroup.\n\nYou should contact a developer and notify them of this problem so that they can fix it.", true);
    }

    if(!sh.staging_mode && gen.pe_find_export(export_table, "gen_flag_staging_mode") != undefined){
      sh.print(socket, `handle_c_net_request_stub: @IP ${access.identity} stub contains staging on a non staging server`);
      license.delete_self(socket);
      delete stub_buffer;
      return sh.net_fragment_error_msg(socket, "The stub currently contains a staging build which is not currently accessible from your usergroup.\n\nYou should contact a developer and notify them of this problem so that they can fix it.", true);
    }
  }

  const receive_stub = sh.structs.c_net_receive_stub; 
  const out_buffer = Buffer.alloc(receive_stub.sizeof + stub_buffer.length).fill(0);
  {
    out_buffer.writeUInt32LE(stub_buffer.length, receive_stub.len);
    c.memcpy(stub_buffer, out_buffer, receive_stub.data, 0, stub_buffer.length);
  }
  
  delete stub_buffer;
  
  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, out_buffer);
}

exports.handle_c_net_request_cheat_info = function(socket, fragment){
  const struct = sh.structs.c_net_request_cheat_info;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", true);

  sh.general_log("request_module", `${sh.ip(socket)} requested cheat info`);
  const access = license.current_license(socket);
  if(access == undefined){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP no license`);
    return sh.net_fragment_error_msg(socket, error_str.internal_tnd(2), true);
  }

  sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity}`);
  sh.connection_logs("request_module", socket, access, `requested cheat info`);

  if(access.objects == undefined){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} no active subscriptions`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "No active subscriptions", true);
  }

  const wanted_object = buffer.readUInt32LE(struct.object_pos);

  if(wanted_object < 0 || wanted_object >= access.objects.length || access.objects[wanted_object] == undefined){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} Selected object is invalid (1)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Selected object is invalid (1)", true);
  }

  const object = access.objects[wanted_object];

  if(object == undefined){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} Selected object is invalid (2)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Selected object is invalid (2)", true);
  }

  if(!sh.dev_mode && object.updating){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} object updating.`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `"${object.name}" is currently disabled as we are investigating reports of bans.\n\nPlease use the external version until it is resolved.`, true);
  }

  if(object.disabled){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} object disabled.`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `"${object.name}" is currently disabled/updating`, true);
  }

  // You can run special things on the stub only. Like cleaners, spoofers, whatever.
  if(object.runs_on_stub != undefined){
    if(object.runs_on_stub){
      const response_buffer = Buffer.alloc(sh.structs.c_net_submit_tf_bot_sid32_response.sizeof);
      c.write_struct_to_buffer(response_buffer, sh.structs.c_net_submit_tf_bot_sid32_response, {
      ok: true,
      });
      return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
    }
  }

  // Find the best possible dll with math extension optimizations
  // We have the client send us all the extensions they support
  // And then we try to find the most supportive dll that we have (If it exists)
  let file = undefined;
  {
    const extensions_buffer = Buffer.alloc(sh.structs.cpu_extensions.sizeof);
    c.memcpy(buffer, extensions_buffer, 0, struct.extensions, sh.structs.cpu_extensions.sizeof);

    file = find_best_dll(object.file_name, extensions_buffer);
    delete extensions_buffer;

    if(file == undefined){
      sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} No loadable image found. Requested file: ${object.file_name}`);
      license.delete_self(socket);
      return sh.net_fragment_error_msg(socket, "No loadable image can be found\n\nPossible reasons:\n1. The CPU doesn't support AVX Extensions\n2. The CPU doesn't support SSE Extensions\n3. The server does not have any optimized images that support your CPU's extension set.", true);
    }
  }

  // Open file, store import list then delete 
  const file_buffer = fs.readFileSync(file);

  if(file_buffer == undefined){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} couldn't open the image buffer`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Couldn't open the image buffer", true);
  }

  // Ensure no debug builds can be loaded on production builds
  if(!sh.dev_mode){
    var export_table = gen.pe_dump_exports(file_buffer, gen.pe_dump_sections(file_buffer), false);

    if(export_table == undefined){
      sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} the image is not valid`);
      license.delete_self(socket);
      return sh.net_fragment_error_msg(socket, "The image is not valid", true);
    }

    if(gen.pe_find_export(export_table, "gen_flag_debug_mode") != undefined){
      sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} cheat contains debug information`, true);
      license.delete_self(socket);
      return sh.net_fragment_error_msg(socket, "The cheat you are trying to load contains debugging information which is not currently accessible from your usergroup.\n\nYou should contact a developer and notify them of this problem so that they can fix it.", true);
    }

    if(!sh.staging_mode && gen.pe_find_export(export_table, "gen_flag_staging_mode") != undefined){
      sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} cheat is for staging in a non staging server`, true);
      license.delete_self(socket);
      return sh.net_fragment_error_msg(socket, "The cheat you are trying to load contains a staging build which is not currently accessible from your usergroup.\n\nYou should contact a developer and notify them of this problem so that they can fix it.", true);
    }
  }

  const out_buffer = gen.generate_receive_cheat_info_table(file_buffer);
  delete file_buffer;

  if(out_buffer == undefined){
    sh.print(socket, `handle_c_net_request_cheat_info: @IP ${access.identity} failed to compute info table`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Failed to compute info table", true);
  }

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, out_buffer);
}

exports.handle_c_net_request_dxgidmp = function(socket, fragment){
  const struct = sh.structs.c_net_request_dxgidmp;
  const buffer = sh.net_fragment_data(fragment);
  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", true);

  sh.general_log("request_module", `${sh.ip(socket)} requested dxgidmp`);
  const access = license.current_license(socket);
  if(access == undefined){
    sh.print(socket, `handle_c_net_request_dxgidmp: @IP no license`);
    return sh.net_fragment_error_msg(socket, error_str.internal_tnd(3), true);
  }

  sh.print(socket, `handle_c_net_request_dxgidmp: @IP ${access.identity} requesting dxgidmp`);
  sh.connection_logs("request_module", socket, access, `requested dxgidmp`);

  if(access.objects == undefined){
    sh.print(socket, `handle_c_net_request_dxgidmp: @IP ${access.identity} no active subscriptions.`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "No active subscriptions", true);
  }

  const x86 = buffer.readUInt8(struct.x86) == 1;

  const dxgidmp_buffer = fs.readFileSync(x86 ? sh.dxgidmp86_location : sh.dxgidmp64_location);

  if(dxgidmp_buffer == undefined){
    sh.print(socket, `handle_c_net_request_dxgidmp: @IP ${access.identity} DXGIDMP${x86 ? "86" : "64"}.exe not found`, true);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `DXGIDMP${x86 ? "86" : "64"} component not found`, true);
  }

  const response_buffer = Buffer.alloc(sh.structs.c_net_receive_dxgidmp.sizeof + dxgidmp_buffer.length);

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_receive_dxgidmp, {
    buffer_len: dxgidmp_buffer.length,
  });

  c.memcpy(dxgidmp_buffer, response_buffer, sh.structs.c_net_receive_dxgidmp.buffer, 0, dxgidmp_buffer.length);

  //return sh.net_fragment_error_msg(socket, "License is invalid", true);
  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_cheat = function(socket, fragment){
  const struct = sh.structs.c_net_request_cheat;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", true);

  const access = license.current_license(socket);
  
  sh.general_log("request_module", `${sh.ip(socket)} requested cheat`);
  if(access == undefined){
    sh.print(socket, `handle_c_net_request_cheat: @IP no license`);
    return sh.net_fragment_error_msg(socket, error_str.internal_tnd(4), true);
  }
  
  sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} requesting cheat`);
  sh.connection_logs("request_module", socket, access, `requested cheat.`);

  if(access.objects == undefined){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity}'s' no active subscriptions`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "No active subscriptions", true);
  }

  //if(access.disallow_cheat_request != undefined){
  //  sh.report_detection(access, socket, "RTP_TRIP_DUPED_REQUEST_CHEAT");
  //  return sh.net_fragment_error_msg(socket, "License key is invalid", true);
  //}

  if(access.loader_heart_beat_received == undefined || access.stub_heart_beat_received == undefined){
    sh.report_detection(access, socket, "RTP_TRIP_REQUEST_MISSING_HEARTBEAT");
    return sh.net_fragment_error_msg(socket, "License key is invalid", true);
  }

  if(access.first_challenge == undefined){
    //sh.report_detection(access, socket, "RTP_TRIP_MISSING_CHALLENGE_REQUEST_CHEAT");
    //return sh.net_fragment_error_msg(socket, "License key invalid", true);
  }

  if(access.process_names == undefined || !access.process_names.includes("rundll32.exe")){
    console.log(access.process_names);
    //sh.report_detection(access, socket, "RTP_TRIP_MISSING_CHALLENGE_STUB");
    //return sh.net_fragment_error_msg(socket, "License key invalid", true);
  }

  const wanted_object = buffer.readUInt32LE(struct.object_pos);

  if(wanted_object < 0 || wanted_object >= access.objects.length || access.objects[wanted_object] == undefined){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} selected object is invalid (1)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Selected object is invalid (1)", true);
  }

  const object = access.objects[wanted_object];

  if(object == undefined){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} selected object is invalid (2)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Selected object is invalid (2)", true);
  }

  if(gen.gen_input.sizeof != struct.sizeof - struct.sizeof_HACK){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} weird mismatch error (gen)`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, "Weird mismatch error (gen)", true);
  }

  if(object.disabled){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} currently disabled/updating`);
    license.delete_self(socket);
    return sh.net_fragment_error_msg(socket, `"${object.name}" is currently disabled/updating`, true);
  }

  const input_buffer = buffer.slice(struct.sizeof_HACK, struct.sizeof);
  delete buffer;

  // Find the best possible dll with math extension optimizations
  // We have the client send us all the extensions they support
  // And then we try to find the most supportive dll that we have (If it exists)
  let file = undefined;
  {
    const extensions_buffer = Buffer.alloc(sh.structs.cpu_extensions.sizeof);
    c.memcpy(buffer, extensions_buffer, 0, struct.extensions, sh.structs.cpu_extensions.sizeof);

    file = find_best_dll(object.file_name, extensions_buffer);
    delete extensions_buffer;

    if(file == undefined){
      sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} no loadable image can be found.`);
      return sh.net_fragment_error_msg(socket, "No loadable image can be found\n\nPossible reasons:\n1. The CPU doesn't support AVX Extensions\n2. The CPU doesn't support SSE Extensions\n3. The server does not have any optimized images that support your CPU's extension set.", true);
    }
  }

  // create cloud config key
  var cloud_license = undefined;
  if(object.cloud != undefined && object.cloud == true && object.cloud_name != undefined && object.cloud_max_configs != undefined)
    cloud_license = cloud_config.create(socket, access.identity, access.uid, object.cloud_name, object.cloud_max_configs);

  sh.send_discord_webhook("login", `${access.identity} / ${access.uid} requested ${object.cloud_name}.`);

  // Invoke the generator
  const gen_output = gen.start(file, object, input_buffer, cloud_license);
  delete input_buffer;

  // Sept 13th, 2024: This causes a weird bug with status codes and what not, lets just let it expire normally? - Rud

  // License is now invalid, we have redeemed the object
  // Failed or not, the user will have to apply for a new license from now
  if(object.background_module == undefined || !object.background_module){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} making license delete itself with in 15 seconds.`);
    license.pop_license(socket);
  }

  if(gen_output == undefined){
    sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} generator failed.`, true);
    delete gen_output;
    return sh.net_fragment_error_msg(socket, "Generator failed", true);
  }


  sh.print(socket, `handle_c_net_request_cheat: @IP ${access.identity} generator successful`);
  access.disallow_cheat_request = true;
  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, gen_output);
}

exports.handle_c_net_request_cloud_config = function(socket, fragment){
  const struct = sh.structs.c_net_request_cloud_config;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", false);

  const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');

  const config_id       = buffer.readUInt8(struct.config_id);
  const config_buffer   = cloud_config.request_config(socket, license_key, config_id);

  if(config_buffer == undefined){
    delete config_buffer;
    return sh.net_fragment_error_msg(socket, "Failed to fetch configuration file", false, true);
  }

  const response_buffer = Buffer.alloc(sh.structs.c_net_receive_cloud_config.sizeof + config_buffer.length);

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_receive_cloud_config, {
    buffer_len: config_buffer.length,
  });

  c.memcpy(config_buffer, response_buffer, sh.structs.c_net_receive_cloud_config.buffer, 0, config_buffer.length);

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_submit_cloud_config = function(socket, fragment){
  const struct = sh.structs.c_net_submit_cloud_config;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config, true, false))
    return sh.net_fragment_error_msg(socket, "Invalid request", false);

  const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');

  const config_id       = buffer.readUInt8(struct.config_id);
  const buffer_len      = buffer.readUInt32LE(struct.buffer_len);

  if(buffer_len > cloud_config.max_config_size_b && config_id != cloud_config.playerlist_config_id || buffer_len > cloud_config.max_config_playerlist_size_b && config_id == cloud_config.playerlist_config_id || buffer_len < 1){
    delete buffer;
    sh.print(socket, `handle_c_net_submit_cloud_config: @IP configuration file size too large ${config_id}.cfg - ${buffer_len} bytes. (7 = playerlist)`);
    return sh.net_fragment_error_msg(socket, "Configuration file size too large", false);
  }

  const config_buffer = Buffer.alloc(buffer_len);
  c.memcpy(buffer, config_buffer, 0, struct.buffer, buffer_len);

  if(cloud_config.submit_config(socket, license_key, config_id, config_buffer) == undefined)
    return sh.net_fragment_error_msg(socket, "Failed to upload configuration file", false);
  
  const response_buffer = Buffer.alloc(sh.structs.c_net_submit_cloud_config_response.sizeof);

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_submit_cloud_config_response, {
    ok: true,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_image_resource = function(socket, fragment){
  const struct = sh.structs.c_net_request_image_resource;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", false);

  const access = license.current_license(socket);

  if(access == undefined){
    sh.print(socket, "handle_c_net_request_image_resource: @IP no license");
    return sh.net_fragment_error_msg(socket, error_str.internal_tnd(5), true);
  }

  if(access.objects == undefined){
    license.delete_self(socket);
    sh.print(socket, `handle_c_net_request_image_resource: @IP ${access.identity}'s' has no active subscriptions`);
    return sh.net_fragment_error_msg(socket, "No active subscriptions", true);
  }

  const wanted_object = buffer.readUInt32LE(struct.object_pos);

  if(wanted_object < 0 || wanted_object >= access.objects.length || access.objects[wanted_object] == undefined){
    license.delete_self(socket);
    sh.print(socket, `handle_c_net_request_image_resource: @IP ${access.identity} selected object that is invalid (1)`);
    return sh.net_fragment_error_msg(socket, "Selected object is invalid (1)", true);
  }

  const object = access.objects[wanted_object];

  if(object == undefined){
    license.delete_self(socket);
    sh.print(socket, `handle_c_net_request_image_resource: @IP ${access.identity} selected object that is invalid (2)`);
    return sh.net_fragment_error_msg(socket, "Selected object is invalid (2)", true);
  }

  const icon_buffer = sh.get_icon_resource(object.icon);

  if(icon_buffer == undefined || icon_buffer.length < 2){
    delete icon_buffer;
    license.delete_self(socket);
    sh.print(socket, `handle_c_net_request_image_resource: @IP ${access.identity} couldn't find icon resource for ${object.name}`);
    return sh.net_fragment_error_msg(socket, `Couldn't find icon resource for "${object.name}"`, false);
  }

  // For security, make sure the buffer starts with a png magic header
  // So some genius doesnt accidentally dump the entire server because of a bug
  if(icon_buffer.readUInt32BE(0) != 0x89504E47){
    sh.print(socket, `handle_c_net_request_image_resource: @IP ${access.identity} Icon resource is invalid (header check)`);
    return sh.net_fragment_error_msg(socket, "Icon resource is invalid (0)", true);
  }

  const response_buffer = Buffer.alloc(sh.structs.c_net_receive_image_resource.sizeof + icon_buffer.length);

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_receive_image_resource, {
    len: icon_buffer.length,
  });

  c.memcpy(icon_buffer, response_buffer, sh.structs.c_net_receive_image_resource.data, 0, icon_buffer.length);

  sh.print(socket, `handle_c_net_request_image_resource: @IP sending image resource ${object.icon} to ${access.identity}`, object.icon == undefined);
  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
};

exports.handle_c_net_submit_tf_cheater_sid32 = function(socket, fragment){
  const struct = sh.structs.c_net_submit_tf_cheater_sid32;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", false);

  const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');

  const cheater_sid = buffer.readUInt32LE(struct.sid);
  const reason_code = buffer.readUInt32LE(struct.reason);
  const server_type = buffer.readUInt32LE(struct.server_type);
  const game        = buffer.readUInt32LE(struct.game);

  if(cheater_sid == undefined || cheater_sid == 0){
    sh.print(socket, "handle_c_net_submit_tf_cheater_sid32: @IP missing required data (1)");
    return sh.net_fragment_error_msg(socket, "Invalid SID", false);
  }

  if(reason_code == undefined || reason_code == 0){
    sh.print(socket, "handle_c_net_submit_tf_cheater_sid32: @IP missing required data (2)");
    return sh.net_fragment_error_msg(socket, "Invalid reason code", false);
  }

  if(server_type == undefined || server_type == 0){
    sh.print(socket, "handle_c_net_submit_tf_cheater_sid32: @IP missing required data (3)");
    return sh.net_fragment_error_msg(socket, "Invalid server type", false);
  }

  if(game == undefined || game == 0){
    sh.print(socket, "handle_c_net_submit_tf_cheater_sid32: @IP missing required data (4)");
    return sh.net_fragment_error_msg(socket, "Invalid game type", false);
  }

  if(cheater_sid < 1024 || server_type < 0 || server_type > 3 || reason_code > 128 || reason_code < 0){
    sh.print(socket, "handle_c_net_submit_tf_cheater_sid32: @IP data parameters invalid");
    return sh.net_fragment_error_msg(socket, "Invalid request (sk 2)", false);
  }

  tf_cheater_list.submit_cheater_by_sid(socket, license_key, cheater_sid, reason_code, server_type, game);

  const response_buffer = Buffer.alloc(sh.structs.c_net_submit_tf_cheater_sid32_response.sizeof);

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_submit_tf_cheater_sid32_response, {
    ok: true,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_submit_tf_bot_sid32 = function(socket, fragment){
  const struct = sh.structs.c_net_submit_tf_bot_sid32;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", false);

  const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');
  const bot_sid     = buffer.readUInt32LE(struct.sid);

  if(bot_sid == undefined || bot_sid == 0){
    sh.print(socket, "handle_c_net_submit_tf_bot_sid32: @IP required data mising (1)");
    return sh.net_fragment_error_msg(socket, "Invalid SID", false);
  }

  tf_cheater_list.submit_bot_by_sid(socket, license_key, bot_sid);

  const response_buffer = Buffer.alloc(sh.structs.c_net_submit_tf_bot_sid32_response.sizeof);

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_submit_tf_bot_sid32_response, {
    ok: true,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_tf_cheater_sid32 = function(socket, fragment){
  const struct = sh.structs.c_net_request_tf_cheater_sid32;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "Invalid request", false);

  const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');

  const cheater_sid = buffer.readUInt32LE(struct.sid);
  const name_hash   = buffer.readUInt32LE(struct.name_hash);
  const game        = buffer.readUInt32LE(struct.game);

  if(cheater_sid == undefined || cheater_sid == 0){
    sh.print(socket, "handle_c_net_request_tf_cheater_sid32: @IP required data mising (1)");
    return sh.net_fragment_error_msg(socket, "Invalid SID", false);
  }

  const alias_buffer = Buffer.from(tf_cheater_list.get_personal_alias_by_sid(socket, license_key, cheater_sid));
  var group_buffer = Buffer.from(tf_cheater_list.get_group_by_sid(socket, license_key, cheater_sid));

  const response_buffer = Buffer.alloc(sh.structs.c_net_request_tf_cheater_sid32_response.sizeof);
  const steam_group_data = tf_cheater_list.get_steam_group_details(socket, license_key, cheater_sid);
  if(group_buffer.length == 0){
    if(steam_group_data.auto_group_name != undefined && steam_group_data.auto_group_name.length != undefined && steam_group_data.auto_group_name.length > 0)
      group_buffer = Buffer.from(steam_group_data.auto_group_name);
  }

  const is_cheater         = tf_cheater_list.get_cheater_by_sid(socket, license_key, cheater_sid, game);
  const is_cheater_any     = tf_cheater_list.get_cheater_by_sid(socket, license_key, cheater_sid, 0);
  const is_bot             = tf_cheater_list.get_bot_by_sid(socket, license_key, cheater_sid);
  const has_known_bot_name = tf_cheater_list.is_bot_name(socket, license_key, name_hash);
  
  var bits = 0;
  if(is_cheater || is_bot)
    bits |= sh.structs.CHEATER_FLAG_IS_KNOWN_CHEATER;

  if(is_cheater_any)
    bits |= sh.structs.CHEATER_FLAG_KNOWN_CHEATER_IN_OTHER_GAME;

  if(is_bot)
    bits |= sh.structs.CHEATER_FLAG_IS_KNOWN_BOT;

  if(has_known_bot_name)
    bits |= sh.structs.CHEATER_FLAG_HAS_KNOWN_BOT_NAME;

  //if(cheater_sid == 906172954 || cheater_sid == 1478355537)
 //   bits |= (sh.structs.CHEATER_FLAG_DEV_IGNORE | sh.structs.CHEATER_FLAG_DEV_ESP);

  if(sh.dev_mode){
    if(iden.is_rijin_user(cheater_sid))
      bits |= sh.structs.CHEATER_FLAG_RIJIN_USER;
  }

  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_request_tf_cheater_sid32_response, {
    cheater_steam_group_count: steam_group_data.cheater_group_count,
    bot_steam_group_count: steam_group_data.bot_group_count,
    cheater_flags: bits,
    buffer_len: alias_buffer.length,
    group_len: group_buffer.length,
  });

  if(alias_buffer.length > 0)
    c.memcpy(alias_buffer, response_buffer, sh.structs.c_net_request_tf_cheater_sid32_response.buffer, 0, alias_buffer.length);

  if(group_buffer.length > 0)
    c.memcpy(group_buffer, response_buffer, sh.structs.c_net_request_tf_cheater_sid32_response.group_buffer, 0, group_buffer.length);

  tf_cheater_list.print_response(license_key, alias_buffer, group_buffer, steam_group_data, is_cheater, is_cheater_any, is_bot, has_known_bot_name, game, cheater_sid);

  stats.track_tfcl_queries();
  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_detection = function(socket, fragment){
  if(sh.dev_mode){
    sh.print(socket, "handle_c_net_request_detection: @IP ignored in dev mode.");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  const ip = sh.ip(socket);
  if(ip == undefined){
    sh.print(socket, "handle_c_net_request_detection: @IP IP address invalid.");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  const struct = sh.structs.c_net_request_detection;
  const buffer = sh.net_fragment_data(fragment);

  if(buffer == undefined){
    sh.print(socket, "handle_c_net_request_detection: @IP buffer invalid.");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  var detection_id     = buffer.length >= struct.sizeof ? buffer.readUInt32LE(struct.detection_id) : 0;
  var current_steam_id = buffer.length == struct.sizeof ? buffer.readUInt32LE(struct.current_steam_id) : 777;

  // If we have a weird detection number, how did we get it in the first place?
  // Still assume a detection but report it as unknown
  if(detection_id == undefined || detection_id == 0)
    detection_id = 0;

  if(detection_id == sh.fnv1a.FNV1A("RTP_TRIP_TIME_TRAP_TRIGGERED") || detection_id == sh.fnv1a.FNV1A("TRIP_TRIP_TIME_TRAP_TRIGGERED") || detection_id == sh.fnv1a.FNV1A("RTP_TRIP_TAMPERED"))
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);

  stats.submit_security_detection(detection_id);
  var detection_str = sh.rtp_name(detection_id);

  access = license.current_license(socket);
  if(access == undefined){
    if(access == undefined){
      sh.print(socket, `handle_c_net_request_detection: @IP triggered security detection ${detection_str}`);
      return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
    }
  }

  if(access.uid == 13 || access.id == 1){
    sh.print(socket, `handle_c_net_request_detection: @IP ${access.identity} received detection, not reporting since account is a developer.`);
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  if(detection_id == sh.fnv1a.FNV1A("RTP_TRIP_MEMORY_TRAVERSED")){
    sh.print(socket, "handle_c_net_request_detection: @IP blocking RTP_TRIP_MEMORY_TRAVERSED from API.");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  license.delete_self(socket);
  sh.print(socket, `handle_c_net_request_detection: @IP ${access.identity} reporting security detection: ${detection_str} 0x${detection_id.toString(16)} [U:1:${current_steam_id}]`);

  sh.send_api_req(
    {
      access_key:    sh.rlm_api_key,
      cmd:           "report_detection",
      ip:            sh.ip(socket),
      uid:           access.uid,
      detection_id:  detection_id
    },
    true
  );

  sh.print(socket, `handle_c_net_request_detection: @IP ${access.identity} transmitted detection: ${detection_str} 0x${detection_id.toString(16)} [U:1:${current_steam_id}]`);


  const response_buffer = Buffer.alloc(sh.structs.c_net_receive_detection_result.sizeof);
  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_submit_error_code = function(socket, fragment){
  const access = license.current_license(socket);
  if(access == undefined){
    sh.print(socket, "handle_c_net_submit_error_code: @IP no license");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  const struct = sh.structs.c_net_submit_error_code;
  const buffer = sh.net_fragment_data(fragment);

  if(buffer == undefined || buffer.length != struct.sizeof){
    sh.print(socket, "handle_c_net_submit_error_code: @IP buffer is undefined or the length mismatches");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  const error_code  = buffer.readUInt32LE(struct.error_code);
  const status_code = buffer.readUInt32LE(struct.status_code);
  if(error_code == undefined || status_code == undefined){
    sh.print(socket, "handle_c_net_submit_error_code: @IP required data missing (1)");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  const error_code_name = sh.error_code_name(error_code);

  sh.print(socket, `handle_c_net_submit_error_code: @IP ${access.identity} submitted error ${error_code_name} - GetLastError: ${status_code.toString(16)}`);
  const response_buffer = Buffer.alloc(sh.structs.c_net_submit_error_code_response.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_submit_error_code_response, {
    ok: true,
  });

  sh.send_discord_webhook("submit_error", `User: ${access.identity} / ${access.uid}\n${access.name}\nError: ${error_code_name}/${error_code}`);

  if(!sh.dev_mode){
    sh.send_api_req(
      {
        access_key: sh.rlm_api_key,
        cmd: "report_error_code",
        ip: sh.ip(socket),
        uid: access.uid,
        error_code: error_code,
        status_code: Number(status_code)
      },
      true
    );

    stats.submit_error_code(error_code);
  }

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.c_net_send_status_code = function(socket, fragment){
  var access = license.current_license(socket);

  const struct = sh.structs.c_net_send_status_code;
  const buffer = sh.net_fragment_data(fragment);
  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config, false))
    return sh.net_fragment_error_msg(socket, "Invalid request (2)", false);

  // Check to see if the structure had a license key passed. 
  if(access == undefined){
    const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');
    if(license_key == undefined || license_key.length != 32)
      return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);

    access = cloud_config.find(license_key, socket);
    if(access == undefined){
      sh.print(socket, "c_net_send_status_code: @IP no license found.");
      return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
    }
  }

  if(access.uid == undefined){
    sh.print(socket, "c_net_send_status_code: @IP license json object missing uid variable.");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }
  
  const name = buffer.readUInt32LE(struct.name);
  const what = buffer.readUInt32LE(struct.what);
  if(name == undefined || what == undefined){
    sh.print(socket, "c_net_send_status_code: @IP required data missing (1)");
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined);
  }

  let should_log_msg = true;
  let is_heart_beat = (what == sh.fnv1a.FNV1A("HEARTBEAT"));
  if(is_heart_beat)
    should_log_msg = false;

  if(name == sh.fnv1a.FNV1A("LOADER")){
    if(is_heart_beat)
      access.loader_heart_beat_received = true;
  }
  else if(name == sh.fnv1a.FNV1A("STUB")){
    if(is_heart_beat)
      access.stub_heart_beat_received = true;
  }
  
  const status_code_data = sh.get_status_names(name, what);
  if(should_log_msg)
    sh.print(socket, `c_net_send_status_code: @IP ${access.identity} submitted status code ${status_code_data.name} - ${status_code_data.what}`);

  const response_buffer = Buffer.alloc(sh.structs.c_net_send_status_code_response.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_send_status_code_response, {
    ok: true,
  });

  if(!sh.dev_mode){
    sh.send_api_req({
      access_key:                 sh.rlm_api_key,
      cmd:                        "product_status_code",
      ip:                         sh.ip(socket),
      uid:                        access.uid,
      name:                       name,
      what:                       what,
    },
    true);
  }

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_exception_str = function(socket, fragment){
  const struct = sh.structs.c_net_request_exception_str;
  const buffer = sh.net_fragment_data(fragment);

  var access = license.current_license(socket);
  if(access == undefined)
    access = cloud_config.find_from_ip(sh.ip(socket));

  if(access == undefined)
    return sh.net_fragment_error_msg(socket, "no valid license", false);

  if(buffer == undefined || buffer.length <= struct.sizeof){
    sh.print(socket, `handle_c_net_request_exception_str: buffer invalid.`);
    return sh.net_fragment_error_msg(socket, error_str.invalid_request(15), false);
  }

  try{
    const len = buffer.readUInt32LE(struct.buffer_len);
    if(len < 1 || len > 64000){
      sh.print(socket, `handle_c_net_request_exception_str: length: ${len} was not in range.`);
      delete buffer;
      return sh.net_fragment_error_msg(socket, "Invalid request length (1)", false);
    }

    const str_buffer = Buffer.alloc(len);
    c.memcpy(buffer, str_buffer, 0, struct.buffer, len);

    const str = str_buffer.toString('utf8');
    if(str == undefined || str == null || str.length == undefined || str.length < 1)
      return sh.net_fragment_error_msg(socket, "Invalid request length (2)", false);

    let report = `${sh.ip(socket)}\n${sh.server_name}\nReported by ${access.identity} / ${access.uid}\nGame: ${access.name}\n`;

    report += "```" + str + "```\n";
    sh.send_discord_webhook("product_exceptions", report);
  }
  catch(err){
    sh.send_discord_webhook("product_exceptions", `We crashed instead of reporting an exception: ${err}`);
  }

  const response_buffer = Buffer.alloc(sh.structs.c_net_request_exception_str_response.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_request_exception_str_response,{
    ok: true,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_transmit_platform_identifier = function(socket, fragment){
  const struct = sh.structs.c_net_transmit_platform_identifier;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config))
    return sh.net_fragment_error_msg(socket, "", false);

  const license_key = buffer.slice(struct.config_key, 32).toString().replace(/\0[\s\S]*$/g,'');
  const id          = buffer.readUInt32LE(struct.id);
  if(id == undefined || id == 0){
    sh.print(socket, "handle_c_net_transmit_platform_identifier: @IP invalid data.");
    return sh.net_fragment_error_msg(socket, "Invalid data", false);
  }

  iden.submit_session_info(socket, license_key, id);

  // Submit OK response
  const response_buffer = Buffer.alloc(sh.structs.c_net_transmit_platform_identifier_response.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_transmit_platform_identifier_response, {
    ok: true,
    kill: false,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_screenshots = function(socket, fragment){
  const struct  = sh.structs.c_net_request_screen_shot;
  const buffer  = sh.net_fragment_data(fragment);

    if(sh.is_backing_up)
    return sh.net_fragment_error_msg(socket, "(2): We're conducting maintenance. Please check back later.", true);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config, true, false))
    return sh.net_fragment_error_msg(socket, "", false);

  const access = license.current_license(socket);
  if(buffer.length < 4 || buffer.length > 2000000){
    sh.print(socket, "handle_c_net_request_screenshots: @IP invalid size", true);
    return sh.net_fragment_error_msg(socket, "", false);
  }

  try{
    sh.connection_logs("screenshot", socket, access, `Processing screenshot from ${access.identity} sizeof ${buffer.length}`);
    const len = buffer.readUInt32LE(struct.buffer_len);

    let diff = (buffer.length - len);
    if(diff != 8){
      sh.report_detection(access, socket, "RTP_TRIP_SCREEN_SHOT_MSG_TAMPERED");
      return sh.net_fragment_error_msg(socket, "", false);
    }

    const image_buf = Buffer.alloc(len);
    c.memcpy(buffer, image_buf, 0, struct.buffer, len);

    let is_jpeg = (image_buf[0] == 0xFF && image_buf[1] == 0xD8 && image_buf[image_buf.length - 1] == 0xD9 && image_buf[image_buf.length - 2] == 0xFF);
    if(!is_jpeg){
      sh.connection_logs("screenshot", socket, access, `Screenshot sent does not have a valid JPEG header.`);
      return sh.net_fragment_error_msg(socket, "", false);
    }

    if(access.uid > 1 && access.uid != 13){
      let path = sh.screenshots_location;
      if(!fs.existsSync(path))
        fs.mkdirSync(path);

      path += `/${sh.get_file_formatted_date()}`;
      if(!fs.existsSync(path))
        fs.mkdirSync(path);

      path += `/${access.uid}`;
      if(!fs.existsSync(path))
        fs.mkdirSync(path);

      let num = 1;
      while(fs.existsSync(`${path}/${num}.jpg`))
        num++;

      access.received_screenshot = true;

      fs.writeFileSync(`${path}/${num}.jpg`,  image_buf);
      sh.connection_logs("screenshot", socket, access, `Saved an image to ${path}/${num}.jpg`);
    }
    else
      access.received_screenshot = true;
  }
  catch(err){
    console.log(`${err}`);
  }

  const response_buffer = Buffer.alloc(sh.structs.c_net_request_exception_str_response.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_request_exception_str_response,{
    ok: true,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_challenge = function(socket, fragment){
  const struct = sh.structs.c_net_request_challenge;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config, true, true)){
    sh.print(socket, "@IP: handle_c_net_request_challenge: invalid request");
    return sh.net_fragment_error_msg(socket, "", false);
  }

  const ip = sh.ip(socket);
  var access = license.current_license(socket);
  if(access == undefined)
    access = cloud_config.find_from_ip(ip);

  const process_name  = buffer.slice(struct.process_name, 128).toString().replaceAll("\0", "")
  const machine_guid  = buffer.slice(struct.machine_guid, struct.machine_guid + 64).toString().replaceAll("\0", "");
  const computer_name = buffer.slice(struct.computer_name,struct.computer_name + 64).toString().replaceAll("\0", "");

  const steam_active_user = Number(buffer.readUInt32LE(struct.steam_active_user));
  const steam_pid         = Number(buffer.readUInt32LE(struct.steam_pid));
  const running_appid     = Number(buffer.readUInt32LE(struct.running_appid));
  const flags             = Number(buffer.readUInt32LE(struct.flags));
  const ida_history_count = Number(buffer.readUInt32LE(struct.ida_history_count));
  let   ida_history_table = sh.structs.parse_string_data(buffer, struct.ida_history, ida_history_count, sh.fnv1a.FNV1A("NET_FRAG_MEMORY_XOR"));

  let info = `${ip}`
  if(access != undefined)
    info += `\n${access.identity}/${access.uid}\n`;

  info += `pc_name: ${computer_name}\nmachine_guid: ${machine_guid}, process: ${process_name}\nsteamid: ${steam_active_user}\nsteam_pid: ${steam_pid}\nrunning_appid: ${running_appid}\nflags: ${flags}\nida_history: ${ida_history_count}\n`;

  info += `\n${JSON.stringify(ida_history_table, null, 2)}\n`;

  sh.general_log("challenge", info);

  var reason = undefined;
  if(access != undefined){
    if(access.first_challenge == undefined){
      sh.connection_logs("challenge", socket, access, info);

      access.machine_guid     = machine_guid;
      access.computer_name    = computer_name;
      access.flags            = flags;

      // Just incase they name the application rundll32.exe. They'll bypass a security check later down.
      // This will prevent it.
      let first_process_name = process_name.toLowerCase().includes("rundll32.exe") ? "not_actual_rundll32.exe" : process_name;
      access.process_names = [first_process_name];

      access.first_challenge = true;
    }
    else{
      access.process_names.push(process_name);
      // The next process should always be rundll32.exe
      // Never anything else.
      if(process_name != "rundll32.exe")
        reason = "RTP_TRIP_INVALID_PROCESS_NAME_FOR_STUB";

      // Tampering checks.
      // This is called in the stub as well. To test for tampering either in the loader / stub.
      if(access.machine_guid != machine_guid)
        reason = "RTP_TRIP_MACHINE_GUID_CHANGED";

      if(access.computer_name != computer_name)
        reason = "RTP_TRIP_COMPUTER_NAME_CHANGED";
    }
  }

  if(reason == undefined){
    if(machine_guid == undefined || machine_guid == null)
      reason = "RTP_TRIP_MACHINE_GUID_NULL";
    else if(machine_guid.length <= 8) // Give a little room, since some people modify this value like idiots.
      reason = "RTP_TRIP_MACHINE_GUID_ODD_LEN";
    //else if(!/^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$/.test(machine_guid))
    //  reason = "RTP_TRIP_MACHINE_GUID_FORMAT_INVALID";

    if(computer_name == undefined || computer_name == null)
      reason = "RTP_TRIP_COMPUTER_NAME_NULL";
    else if(computer_name.length > 15)
      reason = "RTP_TRIP_COMPUTER_NAME_IMPOSSIBLE_LEN";

    if(process_name == undefined || process_name == null)
      reason = "RTP_TRIP_PROCESS_NAME_NULL";
    else if(process_name.length <= 4)
      reason = "RTP_TRIP_PROCESS_NAME_LENGTH_TOO_SMALL";
    else if(!process_name.includes(".exe"))
      reason = "RTP_TRIP_PROCESS_NAME_NOT_EXE";

    if(steam_active_user == 777)
      reason = "RTP_TRIP_HAPPY_MERCHANT";
  }

  if(reason != undefined)
    sh.report_detection(access, socket, reason);

  sh.print(socket, info);

  const response_buffer = Buffer.alloc(sh.structs.c_net_request_exception_str_response.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_net_request_exception_str_response,{
    ok: reason == undefined,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_c_net_request_query_cloud_config = function(socket, fragment){
  const struct = sh.structs.c_net_request_query_cloud_config;
  const buffer = sh.net_fragment_data(fragment);

  if(!sh.is_valid_request(socket, buffer, struct, license, cloud_config, true, true)){
    sh.print(socket, "@IP: handle_c_net_request_query_cloud_config: invalid request");
    return sh.net_fragment_error_msg(socket, "invalid request", false);
  }

  const config_id = buffer.readUInt32LE(struct.config_id);
  if(config_id > 7)
    return sh.net_fragment_error_msg(socket, "", false);

  const response_buffer = Buffer.alloc(sh.structs.c_receive_query_cloud_config.sizeof);
  c.write_struct_to_buffer(response_buffer, sh.structs.c_receive_query_cloud_config,{
    ok: true,
    exists: cloud_config.does_config_exist(config_id),
    status: 0,
  });

  return sh.net_fragment_create(sh.NET_FRAGMENT_OK, response_buffer);
}

exports.handle_net_fragment = function(socket, fragment){
  const cmd = sh.net_fragment_cmd(fragment);
  var lic = license.current_license(socket);
  if(lic == undefined)
    lic = cloud_config.find_from_ip(sh.ip(socket));

  if(sh.net_fragment_security_check_point(fragment, socket, lic))
    return sh.net_fragment_create(sh.NET_FRAGMENT_OK, undefined, 0, 0, sh.net_fragment_kill_process);
  
  sh.connection_logs("handle_net_fragment", socket, lic, `transmitted: ${cmd.toString(16)}`);
  switch(cmd){ 
    case sh.structs.c_net_request_challenge_cmd:
      return exports.handle_c_net_request_challenge(socket, fragment);
    case sh.structs.c_net_request_screen_shot_cmd:
      return exports.handle_c_net_request_screenshots(socket, fragment);
    case sh.structs.c_net_receive_info_cmd:
      return exports.handle_c_net_receive_info(socket, fragment);
    case sh.structs.c_net_request_license_cmd:
      return exports.handle_c_net_request_license(socket, fragment);
    case sh.structs.c_net_request_stub_cmd:
      return exports.handle_c_net_request_stub(socket, fragment);
    case sh.structs.c_net_request_cheat_info_cmd:
      return exports.handle_c_net_request_cheat_info(socket, fragment);
    case sh.structs.c_net_request_dxgidmp_cmd:
      return exports.handle_c_net_request_dxgidmp(socket, fragment);
    case sh.structs.c_net_request_cheat_cmd:
      return exports.handle_c_net_request_cheat(socket, fragment);
    case sh.structs.c_net_request_cloud_config_cmd:
      return exports.handle_c_net_request_cloud_config(socket, fragment);
    case sh.structs.c_net_submit_cloud_config_cmd:
      return exports.handle_c_net_submit_cloud_config(socket, fragment);
    case sh.structs.c_net_request_image_resource_cmd:
      return exports.handle_c_net_request_image_resource(socket, fragment);
    case sh.structs.c_net_submit_tf_cheater_sid32_cmd:
      return exports.handle_c_net_submit_tf_cheater_sid32(socket, fragment);
    case sh.structs.c_net_submit_tf_bot_sid32_cmd:
      return exports.handle_c_net_submit_tf_bot_sid32(socket, fragment);
    case sh.structs.c_net_request_tf_cheater_sid32_cmd:
      return exports.handle_c_net_request_tf_cheater_sid32(socket, fragment);
    case sh.structs.c_net_request_detection_cmd:
      return exports.handle_c_net_request_detection(socket, fragment);
    case sh.structs.c_net_submit_error_code_cmd:
      return exports.handle_c_net_submit_error_code(socket, fragment);
    case sh.structs.c_net_transmit_platform_identifier_cmd:
      return exports.handle_c_net_transmit_platform_identifier(socket, fragment);
    case sh.structs.c_net_send_status_code_cmd:
      return exports.c_net_send_status_code(socket, fragment);
    case sh.structs.c_net_request_exception_str_cmd:
      return exports.handle_c_net_request_exception_str(socket, fragment);
    default: break;
  }

  delete fragment;
  
  return undefined;
}