const sh           = require("./shared.js");
const cloud_config = require("./cloud_config.js");
const http         = require("http");
const colors       = require('colors');
const tf_lobby     = require("./tf_lobby.js");
const sync_server  = require("./synchronization.js");
const fs           = sh.fs;

function print(str, error){
  if(error != undefined)
    str = colors.bgRed("ERROR: " + str);

  str = colors.brightBlue("[ID] ") + str;

  console.log(str);
}

exports.print = print;

exports.session_idenitifer  = [];
exports.session_expire_time = 86400;

exports.is_rijin_user = function(sid){
  if(sid == undefined || sid == 0)
    return false;

  for(var e in exports.session_idenitifer){
    var entry = exports.session_idenitifer[e];
    if(entry == undefined)
      continue;

    if(entry.id == undefined)
      continue;

    if(sh.time() >= ((entry.expire - exports.session_expire_time) + 300))
      continue;

    if(entry.id == sid){
      print(`[RIJIN-ID] Ran into user: ${entry.license.identity} - ${sid}`);
      return true;
    }
  }

  return false;
}

exports.submit_session_info = function(socket, key, sid){
  const license = cloud_config.find(key, socket);
  if(license == undefined){
    print(`submit_session_info: ${sh.ip(socket)}: No license could be found`, true);
    return false;
  }

  if(license.uid == 0){
    print(`submit_session_info: ${sh.ip(socket)}: we shouldn't submit the developer.`, true);
    return false;
  }

  if(sid == undefined || sid == 0){
    print(`submit_session_info: ${sh.ip(socket)}: ID submitted is invalid`, true);
    return false;
  }

  let transmit_online_update = false;
  let transmit_steam_id      = false;
  print(`submit_session_info: ${license.identity} using ID [U:1:${sid}]`);

  if(exports.session_idenitifer[license.identity] == undefined){
    transmit_online_update = true;
    transmit_steam_id      =true;
    exports.session_idenitifer[license.identity] = {server_name: sh.server_name, id: sid, last_seen_time: sh.time(), expire: sh.time() + exports.session_expire_time, next_transmit_time: sh.time() + 60, license: license};
  }
  else{
    // I suppose someone could abuse this by somehow finding out how our networking works and constantly change their SteamID/ID to something else.
    // But if it's ever abused, which I don't see any reasons why any real customer would want to. Because, they'll getting banned.
    // Rather than assume it'll never get abused it'll employ a few 'checks' to counter act this.

    // We'll accept a change if their cloud key has changed. 
    if(license.key != exports.session_idenitifer[license.identity].license.key){
      print(`submit_session_info: Accepting "${license.identity}"'s ID update to ${colors.brightGreen(`${sid}`)}`);
      exports.session_idenitifer[license.identity].id      = sid;
      exports.session_idenitifer[license.identity].license = license;
      transmit_steam_id = true;
    }

    if(exports.session_idenitifer[license.identity].next_transmit_time <= sh.time()){
      transmit_online_update = true;
      exports.session_idenitifer[license.identity].next_transmit_time = sh.time() + 60;
    }

    exports.session_idenitifer[license.identity].last_seen_time = sh.time();
    exports.session_idenitifer[license.identity].expire         = sh.time() + exports.session_expire_time;
  }

  // Update their online status on the forums once in a while.
  if(transmit_online_update){
    sh.send_api_req({
        access_key: sh.rlm_api_key,
        cmd:        "update_online_status",
        ip:         sh.ip(socket),
        user_id:    license.uid,
      },
      true   
    );
  }

  if(transmit_steam_id){
    sh.send_api_req({
        access_key: sh.rlm_api_key,
        cmd:        "receive_steam_id",
        sid:        sid,
        ip:         sh.ip(socket),
        user_id:    license.uid,
      },
      true
    );
  }

  exports.sync_list();
  return true; 
}

exports.sync_list = function(){
  var dat = sync_server.setup_response_json("id_session_list");
  if(exports.session_idenitifer != undefined){

    var list = [];
    for(var e in exports.session_idenitifer){
      var entry = exports.session_idenitifer[e];
      if(entry.server_name == undefined){
        print(`sync_list: Entry had an invalid server name!`);
        continue;
      }

      // Only transmit entries from this server.
      if(entry.server_name != sh.server_name)
        continue;

      list.push(entry);
    }

    dat.data = JSON.stringify(list, null, 2);
  }
  sync_server.send_json_data(dat);
}

exports.clean_up = function(){
  var data_removed = false;
  for(var e in exports.session_idenitifer){
    var entry = exports.session_idenitifer[e];

    if(sh.time() >= entry.expire){
      print(`clean_up: "${entry.license.identity}" session ID not renewed, removing from list`);
      delete exports.session_idenitifer[e];
      data_removed = true;
    }
  }

  if(data_removed)
    exports.sync_list();
}

setInterval(exports.clean_up, 1000);