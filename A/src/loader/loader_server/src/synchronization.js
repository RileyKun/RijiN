const sh             = require("./shared.js");
const cloud_config   = require("./cloud_config.js");
const http           = require("http");
const colors         = require('colors');
const tf_lobby       = require("./tf_lobby.js");
const iden           = require("./identifier.js");
const tf_cheater_list = require("./tf_cheater_list.js");
const sync_server    = require("./synchronization.js");

function print(str, error){
  if(error != undefined)
    str = colors.bgRed("ERROR: " + str);

  str = colors.brightBlue("[SYNC] ") + str;

  console.log(str);
}

exports.print = print;

const server = http.createServer(async (req, res) =>{
  if(req.method == undefined || req.method != "POST"){
    req.connection.destroy();
    return;
  }

  var data = "";
  req.on("data", text => data += text);
  req.on("end", () =>{
    if(data == undefined || data.length == 0){
      req.connection.destroy();
      return;
    }

    try{
      var json = JSON.parse(data);
      if(json == undefined){
        req.connection.destroy();
        return;
      }

      if(json.api_key == undefined || json.extra_key == undefined){
        exports.print("Invalid API keys (1)", true);
        req.connection.destroy();
        return;
      }

      if(json.api_key != "mournful-shopping-dayroom-perjurer-friend-dehydrate-knapsack-obtrusive-speak-rack-phrase-aftermost"
        || json.extra_key != "exterior-reuse-audacity-safari-wrongful-mangy-unknotted-deflation-parish-spiffy-renderer-departed"){   
        exports.print("Invalid API keys (2)", true);
        req.connection.destroy();
        return;
      }

      if(json.type == undefined || json.data == undefined){
        exports.print(`Received invalid type: ${json.type}, Possible invalid data: ${json.data}`, true);
        req.connection.destroy();
        return;
      }

      if(json.server_name == undefined){
        exports.print(`${json.server_name} received invalid server name`);
        req.connection.destroy();
        return;
      }

      if(json.type == "lobby_list"){

        if(json.data == undefined){
          exports.print(`${json.server_name}'s lobby list is empty`);
          res.statusCode = 200;
          res.setHeader("Content-Type", "application/json");
          res.end(JSON.stringify({"ok": true, "reason": "lobby list was empty"}));
          return;
        }

        var lobby_data  = JSON.parse(json.data);
        for(var i in lobby_data){
          var entry = lobby_data[i];
          if(entry == undefined)
            break;

          if(entry.expire == undefined || entry.license.identity == undefined)
            continue;

          // Avoid processing our own data again by mistake causing an inf loop on the sync server.
          if(entry.server_name == undefined || entry.server_name == sh.server_name)
            continue;

          // Only process data that coming from their data source.
          // So if we receive data about shipping's entries from staging we ignore it.
          if(entry.server_name != json.server_name)
            continue;

          if(entry.expire <= sh.time())
            continue;
          
          if(tf_lobby.lobby_sessions[entry.license.identity] == undefined)
            tf_lobby.lobby_sessions[entry.license.identity] = {sid: entry.sid, member_count: entry.member_count, lobby_state: entry.lobby_state, country_code: entry.country_code, server_name: entry.server_name, expire: entry.expire, license: entry.license};
          else{
            // (Jan 5th, 2024): I realized we weren't updating member count and other details here so I fixed it when working on the session list.
            tf_lobby.lobby_sessions[entry.license.identity].expire       = entry.expire;
            tf_lobby.lobby_sessions[entry.license.identity].sid          = entry.sid;
            tf_lobby.lobby_sessions[entry.license.identity].lobby_state  = entry.lobby_state;
            tf_lobby.lobby_sessions[entry.license.identity].member_count = entry.member_count;
            tf_lobby.lobby_sessions[entry.license.identity].country_code = entry.country_code;
          }
        }

        res.statusCode = 200;
        res.setHeader("Content-Type", "application/json");
        res.end(JSON.stringify({"ok": true}));
      }
      else if(json.type == "id_session_list"){

        if(json.data == undefined){
          exports.print(`${json.server_name}'s id session list was empty`);
          res.statusCode = 200;
          res.setHeader("Content-Type", "application/json");
          res.end(JSON.stringify({"ok": true, "reason": "id session list was empty"}));
          return;
        }

        var session_data = JSON.parse(json.data);
        for(var i in session_data){
          var entry = session_data[i];
          if(entry == undefined)
            break;

          if(entry.expire == undefined || entry.license.identity == undefined)
            continue;

          if(entry.id == undefined || entry.id == 0)
            continue;

          // We only wanna process data that came from other servers.
          if(entry.server_name == undefined || entry.server_name == sh.server_name)
            continue;

          // Only process data that coming from their data source.
          // So if we receive data about shipping's entries from staging we ignore it.
          if(entry.server_name != json.server_name)
            continue;

          if(entry.expire <= sh.time())
            continue;

          if(iden.session_idenitifer[entry.license.identity] == undefined)
            iden.session_idenitifer[entry.license.identity] = {server_name: entry.server_name, id: entry.id, expire: entry.expire, license: entry.license};
          else{
            iden.session_idenitifer[entry.license.identity].id      = entry.id;
            iden.session_idenitifer[entry.license.identity].expire  = entry.expire;
            iden.session_idenitifer[entry.license.identity].license = entry.license;
          }
        }

        res.statusCode = 200;
        res.setHeader("Content-Type", "application/json");
        res.end(JSON.stringify({"ok": true}));
      }
      else if(json.type == "cheater_list_data"){
        if(json.data == undefined || json.bot_data == undefined){
          exports.print(`${json.server_name}'s (bot data or data) was invalid`);
          res.statusCode = 200;
          res.setHeader("Content-Type", "application/json");
          res.end(JSON.stringify({"ok": true, "reason": "Either the cheater list or bot data were both empty"}));
          return;
        }

        // Make sure we receive data from another server.
        if(json.server_name == sh.server_name){
          res.statusCode = 200;
          res.setHeader("Content-Type", "application/json");
          res.end(JSON.stringify({"ok": true}));
          return;
        }

        var cheater_list = JSON.parse(json.data);
        var bot_list     = JSON.parse(json.bot_data);

        tf_cheater_list.submit_cheater_from_sync_server(cheater_list, json.server_name);
        tf_cheater_list.submit_bot_from_sync_server(bot_list, json.server_name);
        
        res.statusCode = 200;
        res.setHeader("Content-Type", "application/json");
        res.end(JSON.stringify({"ok": true}));
      }
    }
    catch(err){
      exports.print("There was an error processing a request");
      exports.print(err, true);
    }
  });
});

exports.start_listening = function(){
  server.listen(sh.sync_listen_port, sh.listen_addr, () =>{
    console.log("Listening for synchronization server responses " + sh.listen_addr + ":" + sh.sync_listen_port);
  });
}

exports.setup_response_json = function(type){
  return {server_name: sh.server_name, api_key: "mournful-shopping-dayroom-perjurer-friend-dehydrate-knapsack-obtrusive-speak-rack-phrase-aftermost", extra_key: "exterior-reuse-audacity-safari-wrongful-mangy-unknotted-deflation-parish-spiffy-renderer-departed", type: type, data: ""};
}

exports.send_json_data = async function(json_data){
  try{
    var result = await fetch("http://" + sh.sync_addr + ":" + sh.sync_master_port.toString(),{
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(json_data),
    });
  
    var response_json = await result.json();
    if(response_json == undefined)
      return undefined;

    return response_json;
  }
  catch(err){
  }

  return undefined;
}

exports.transmit_cloud_sessions = function(){
  if(cloud_config.license_list == undefined)
    return;

  var response = sync_server.setup_response_json("submit_cloud_sessions");

  var list = [];
  for(var username in cloud_config.license_list){
    var entry = cloud_config.license_list[username];
    if(entry == undefined)
      continue;

    // Avoid sending un-needed data hence why I did it this way.
    list.push({identity: entry.identity, uid: entry.uid, expire: entry.expire, name: entry.name});
  }

  response.data = JSON.stringify(list, null, 2);
  sync_server.send_json_data(response);
}

setInterval(exports.transmit_cloud_sessions, 10000);