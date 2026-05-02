// custom
const sh = require("./shared.js");
const cloud_config = require("./cloud_config.js");
const fs = require("fs");
const colors = require('colors');
const sync_server = require("./synchronization.js");

// helpers / fetchers
function print(str, error){
  if(error != undefined)
    str = colors.bgRed("ERROR: " + str);

  str = colors.brightBlue("[TFLOBBY] ") + str;

  console.log(str);
}
exports.print = print;

exports.lobby_sessions = [];
exports.lobby_expire_time = 25;
exports.submit_lobby_info = function(socket, key, leader_sid, member_count, country_code, lobby_state){
  const license = cloud_config.find(key);

  if(license == undefined){
    print(`${sh.ip(socket)}: No license could be found`, true);
    return false;
  }

  if(leader_sid == undefined || member_count == undefined || country_code == undefined){
    print(`"${license.identity}" invalid data`);
    return false;
  }

  if(country_code == null || country_code.length > 8 || country_code.length == 0){
    print(`"${license.identity}" Invalid country code. ${country_code} - ${country_code.length}`);
    return false;
  }

  if(leader_sid <= 1){
    print(`"${license.identity}" sent an invalid steamid`);
    return false;
  }

  if(member_count < 1 || member_count > 6){
    print(`"${license.identity}" sent an out of range member count: ${member_count}`);
    return false;
  }

  print(`"${license.identity}" is sharing his lobby ${colors.brightGreen(`[U:1:${leader_sid}]`)} - ${country_code} - ${member_count}/6`);
  if(exports.lobby_sessions[license.identity] == undefined)
    exports.lobby_sessions[license.identity] = {sid: leader_sid, member_count: member_count, country_code: country_code, lobby_state: lobby_state, server_name: sh.server_name, expire: sh.time() + exports.lobby_expire_time, license: license};
  else{
    exports.lobby_sessions[license.identity].member_count = member_count;
    exports.lobby_sessions[license.identity].sid          = leader_sid;
    exports.lobby_sessions[license.identity].country_code = country_code;
    exports.lobby_sessions[license.identity].lobby_state  = lobby_state;
    exports.lobby_sessions[license.identity].expire       = sh.time() + exports.lobby_expire_time;
  }

  exports.sync_list();

  // add to list or update keep alive timer to prevent removal.
  return true;
}

exports.clean_up = function(){
  var data_removed = false;
  for(var e in exports.lobby_sessions){
    var entry = exports.lobby_sessions[e];

    if(sh.time() >= entry.expire){
      print(`"${entry.license.identity}" lobby not renewed, removing from master list`);
      delete exports.lobby_sessions[e];
      data_removed = true;
    }
  }

  if(data_removed)
    exports.sync_list();
}

exports.sync_list = function(){
  var dat = sync_server.setup_response_json("lobby_list");
  if(exports.lobby_sessions != undefined){

    var list = [];
    for(var e in exports.lobby_sessions){
      var entry = exports.lobby_sessions[e];
      if(entry.server_name == undefined){
        print(`Entry had an invalid server name!`);
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

setInterval(exports.clean_up, 1000);
