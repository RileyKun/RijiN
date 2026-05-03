// custom
const sh           = require("./shared.js");
const cloud_config = require("./cloud_config.js");
const fs           = require("fs");
const colors       = require('colors');
const sync_server  = require("./synchronization.js");

exports.max_decay_time_amount           = 86400 * 7;
exports.max_confirms_considered_bot     = 200;
exports.max_confirms_considered_cheater = 0;

// What the sid entry json should look like

// last_submit_time = sh.time();
// last_seen_time = the last time the ID was queried.
// detections = a list of detection strings, intended as a log of what they were submitted by.
// submitters = a list of rijin users who submit cheaters. 
exports.cheater_data_json_schema = {
  force_always: false,
  last_submit_time: 0,
  last_seen_time: 0,
  detections: {},
  games:      [],
  submitters: []};


/*
detections
    {
      server_type: "Community",
      detection_type: "OB PITCH",
      time_submitted: 0,
      submitter_uid: 0
    }
*/
exports.steam_group_list = {};

// This is just used to add new entries to existing ones.
function translate_list(file_name){
  var data = fs.readFileSync(sh.tf_cheater_list_location + "/" + file_name);
  if(data == undefined)
    return;

  var json = JSON.parse(data);
  if(json == undefined)
    return;

  delete data;

  var list = {};
  try{
    for(var e in json){
      if(list[e] != undefined)
        continue;
      
      // Remove bugged game entries.
      var new_data = json[e];
      if(new_data.games != undefined){
        for(var i in new_data.games){
          var val = Number(new_data.games[i]);
          if(new_data.games[i] == null || val <= 1000)
            new_data.games.splice(i, 1);
        }
      }
      
      list[e] = new_data;
    }
  
    fs.writeFileSync(sh.tf_cheater_list_location + "/" + file_name, JSON.stringify(list, null, 2));
  }
  catch(err){
    console.log(err);
  }
}

// For now, keep so shipping, staging can have the new games section for detections.
translate_list("dev.json");
translate_list("pub.json");

function fix_table(table){
  if(table == undefined)
    return undefined;

  if(table.force_always == undefined)
    table.force_always = false;

  if(table.last_submit_time == undefined)
    table.last_submit_time = 0;

  if(table.last_seen_time == undefined)
    table.last_seen_time = 0;

  if(table.detections == undefined)
    table.detections = {};

  if(table.games == undefined)
    table.games = [];

  if(table.submitters == undefined)
    table.submitters = [];

  try{
    for(var entry of table.games){

    }
  }
  catch{
    console.log("table.games is not a list.");
    table.games = [];
  }

  try{
    for(var entry of table.submitters){

    }
  }
  catch{
    console.log("table.submitters is not a list.");
    table.submitters = [];
  }

  return table;
}

// helpers / fetchers
function print(str, error){
  if(error != undefined)
    str = colors.bgRed("ERROR: " + str);

  str = colors.brightRed("[TFCL] ") + str;

  console.log(str);
}
exports.print = print;

exports.make_folders_and_files = function(){
  if(!fs.existsSync(sh.tf_cheater_list_location))
    fs.mkdirSync(sh.tf_cheater_list_location);

  if(!fs.existsSync(sh.tf_cheater_list_location + "/dev.json"))
    fs.writeFileSync(sh.tf_cheater_list_location + "/dev.json", JSON.stringify({}, null, 2));

  if(!fs.existsSync(sh.tf_cheater_list_location + "/pub.json"))
    fs.writeFileSync(sh.tf_cheater_list_location + "/pub.json", JSON.stringify({}, null, 2));

  if(!fs.existsSync(sh.tf_cheater_list_location + "/alias.json"))
    fs.writeFileSync(sh.tf_cheater_list_location + "/alias.json", JSON.stringify([], null, 2));

  if(!fs.existsSync(sh.tf_cheater_list_location + "/dev_bot_list.json"))
    fs.writeFileSync(sh.tf_cheater_list_location + "/dev_bot_list.json", JSON.stringify({}, null, 2));

  if(!fs.existsSync(sh.tf_cheater_list_location + "/pub_bot_list.json"))
    fs.writeFileSync(sh.tf_cheater_list_location + "/pub_bot_list.json", JSON.stringify({}, null, 2));

  if(!fs.existsSync(sh.tf_cheater_list_location + "/bot_name_list.json"))
    fs.writeFileSync(sh.tf_cheater_list_location + "/bot_name_list.json", JSON.stringify([], null, 2));
}

exports.get_detection_reason = function(code){
  if(code == undefined)
    return undefined;

  switch(code){
    default: break;
    case 1: return "OBB PITCH";
    case 2: return "ANGLE ANALYTICAL";
    case 3: return "TICKBASE ABUSE";
    case 4: return "SPEEDHACK";
  }

  exports.print(`get_detection_reason: ${code} is not a known detection type.`);
  return undefined;
}

exports.get_server_type = function(server_type){
  if(server_type == undefined)
    return undefined;

  switch(server_type){
    default: break;
    case 1: return "(Listen server)";
    case 2: return "(Community server)";
    case 3: return "(Matchmaking server)";
    case 4: return "(Demo playback)";
  }

  exports.print(`get_server_type: ${server_type} is not a known server type.`);
  return undefined;
}

exports.print_response = function(key, alias, group, steam_group, is_cheater, is_cheater_any, is_cheater_bot, has_known_bot_name, game, sid){
  const license = cloud_config.find(key);
  if(license == undefined)
    return;

  let json_type = JSON.stringify({
    sid: sid,
    game_hash: game,
    is_cheater: is_cheater,
    is_cheater_bot: is_cheater_bot,
    is_cheater_any: is_cheater_any,
    has_known_bot_name: has_known_bot_name,
    alias: alias.toString(),
    group: group.toString(),
    steam_group: steam_group != undefined ? steam_group : null,
  }, null, 2);

  let response = `${license.identity} queried ${colors.brightRed(`[U:1:${sid}]`)} \n${json_type}`;

  exports.print(response);
  if(!is_cheater && !is_cheater_any && !is_cheater_any && alias.length == 0 && group.length == 0 && steam_group == undefined)
    return;

  let msg = `User: ${license.identity}\nID: ${license.uid}\n`;

  if(is_cheater_bot)
    msg += " > is a bot\n";
  else if(is_cheater || is_cheater_any)
    msg += " > is a cheater\n";

  if(alias.length > 0)
    msg += ` > Known alias: ${alias}\n`;

  if(group.length > 0)
    msg += ` > Known group: ${group}\n`;

  if(steam_group != undefined){
    if(steam_group.cheater_group_count > 0)
      msg += ` > Cheater group(s): ${steam_group.cheater_group_count}\n`;

    if(steam_group.bot_group_count > 0)
      msg += ` > Bot group(s): ${steam_group.bot_group_count}\n`;
  }

  msg += `https://steamcommunity.com/profiles/[U:1:${sid}]\n`;

  sh.send_discord_webhook("sid_query", msg);
}

exports.get_name_to_hash = function(name){
  if(name == undefined || name == null)
    return 0;

  name = name.toLowerCase();

  var new_str = "";
  for(var i = 0; i < name.length; i++){
    if(name[i] == ' ' || name.charCodeAt(i) > 128)
      continue;

    new_str += name[i];
  }

  const bad_list = ["shoppy.gg/@", "discord.gg", "dsc.gg", "youtube.com", "youtube", "myg0t", "myg(t", "royalhack" ];
  for(var entry of bad_list){
    if(new_str.includes(entry)){
      console.log("bot_name_generic");
      return sh.fnv1a.FNV1A("bot_name_generic");
    }
  }

  return new_str.length > 0 ? sh.fnv1a.FNV1A(new_str) : sh.fnv1a.FNV1A(name);
}

exports.is_sid_in_list = function(file_name, sid){
  const file_name_to_read = sh.tf_cheater_list_location + `/${file_name}`;

  var data = fs.readFileSync(file_name_to_read);

  if(data == undefined)
    return false;

  var json = JSON.parse(data);
  if(data == undefined)
    return false;

  var entry = json[sid.toString()];

  delete data;
  delete json;

  return entry != undefined;
}

// Avoid using this in loops. It's slow due to the fs.readFileSync
exports.get_sid_data = function(file_name, sid){
  if(file_name == undefined || sid == undefined){
    exports.print(`get_sid_data: invalid file_name or sid`, true);
    return undefined;
  }

  const file_name_to_read = sh.tf_cheater_list_location + `/${file_name}`;

  var data = fs.readFileSync(file_name_to_read);
  if(data == undefined){
    exports.print(`get_sid_data: failed to read data from ${file_name_to_read}`);
    return undefined;
  }

  delete data;
  var json = JSON.parse(data);
  if(json == undefined){
    exports.print(`get_sid_data: failed to parse json data from ${file_name_to_read}`);
    return undefined;
  }

  var entry = json[sid.toString()];

  delete json;
  if(entry == undefined){
    //exports.print(`get_sid_data: no data on [U:1:${sid}]`);
    return undefined;
  }

  return entry;
}

exports.should_be_considered_cheater = function(file_name, sid, max_commit_length, decay, game){
  var data = exports.get_sid_data(file_name, sid);
  if(data == undefined){
    //exports.print(`should_be_considered_cheater: failed to get data on [U:1:${sid}] from list ${file_name}`);
    return false;
  }
  
  if(data.submitters == undefined){
    exports.print(`${sid} submitters table is undefined`);
    return false;
  }

  if(game != 0){
    if(data.games != undefined){
      if(!data.games.includes(game))
        return false;
    }
  }

  if(decay != undefined){
    if(decay){
      if(!data.last_submit_time){
        exports.print(`should_be_considered_cheater: [U:1:${sid}] in list`)
        return false;
      }

      if(data.last_submit_time + exports.max_decay_time_amount <= sh.time())
        return false;
    }
  }

  exports.update_sid_last_seen_time(file_name, sid);
  return data.submitters.length >= max_commit_length || data.force_always;
}

exports.push_data_to_list = function(file_name, sid, server_type, detection_str, license, game){
  if(sid == undefined || detection_str == undefined || license == undefined){
    exports.print("[-] push_data_to_list: Invalid parameters\n");
    return false;
  }

  const file_name_to_read = `${sh.tf_cheater_list_location}/${file_name}`;

  let data = fs.readFileSync(file_name_to_read);
  if(data == undefined){
    exports.print(`[-] push_data_to_list: ${file_name_to_read} doesn't exist!`);
    return false;
  }

  let json = JSON.parse(data);
  if(json == undefined){
    exports.print(`[-] push_data_to_list: failed to parse json from ${file_name_to_read}`);
    return false;
  }
  
  const sid_str = sid.toString();
  const uid_str = license.uid.toString();

  let member = json[sid_str];
  if(member === undefined)
    member = exports.cheater_data_json_schema;


  member.last_submit_time = sh.time();
  member.last_seen_time   = sh.time();
  member.detections[uid_str] = {
    server_type:    server_type,
    detection_type: detection_str,
    time_submitted: sh.time()
  };

  // Make sure the game this detection occured on is logged.
  {
    if(member.games === undefined)
      member.games = [];

    if(!member.games.includes(game))
      member.games.push(Number(game));
  }

  // Record the RijiN user's userID for detection info.
  {
    if(member.submitters === undefined)
      member.submitters = [];

    if(!member.submitters.includes(uid_str))
      member.submitters.push(uid_str);
  }

  json[sid_str] = member;

  exports.print(`push_data_to_list: ${license.identity} updating cheater info: [U:1:${sid}] -> [${file_name}] [${detection_str}]`);
  fs.writeFileSync(file_name_to_read, JSON.stringify(json, null, 2));
  return true;
}

exports.update_sid_last_seen_time = function(file_name, sid){
  const file_name_to_read = sh.tf_cheater_list_location + `/${file_name}`;

  let data = fs.readFileSync(file_name_to_read);
  if(data == undefined)
    return false;

  let json = JSON.parse(data);
  if(json == undefined)
    return false;

  let sid_str = sid.toString();
  if(json[sid_str] == undefined)
    return false;

  json[sid_str].last_seen_time = sh.time();
  fs.writeFileSync(file_name_to_read, JSON.stringify(json, null, 2));

  return true;  
}

exports.get_sid_alias_from_list = function(sid){
  var data = fs.readFileSync(sh.tf_cheater_list_location + "/alias.json");
  if(data == undefined)
    return "";

  var json = JSON.parse(data);
  if(json == undefined)
    return "";

  var known_alias = "";
  for(var i in json){
    if(json[i].id == undefined)
      continue;

    if(json[i].id != sid)
      continue;

    if(json[i].name == undefined || json[i].name == null)
      continue;

    known_alias = json[i].name;
    break;
  };

  delete data;
  delete json;

  if(known_alias.length == 0)
    return "";

  return known_alias + '\0';
}

exports.get_sid_group_from_list = function(sid){
  var data = fs.readFileSync(sh.tf_cheater_list_location + "/alias.json");
  if(data == undefined)
    return "";

  var json = JSON.parse(data);
  if(json == undefined)
    return "";

  var known_group = "";
  for(var i in json){
    if(json[i].id == undefined)
      continue;

    if(json[i].id != sid)
      continue;

    // This should never happen unless I make a mistake.
    if(json[i].group == undefined || json[i].group == null)
      continue;

    known_group = json[i].group;
    break;
  };

  delete data;
  delete json;

  if(known_group.length == 0)
    return "";

  return known_group + '\0';
}

exports.is_blacklisted_json = undefined;
exports.is_sid_blacklisted = function(sid){
  if(exports.is_blacklisted_json)
    exports.is_blacklisted_json = sh.load_json_file(`${sh.tf_cheater_list_location}/blacklisted.json`);

  if(exports.is_blacklisted_json == undefined)
    return false;

  try{
    return exports.is_sid_blacklisted.includes(sid);
  }
  catch{
  }

  return false;
}

exports.is_sid_a_cheater = function(sid, game){
  if(exports.is_sid_blacklisted(sid))
    return false;
  
  return exports.should_be_considered_cheater("pub.json", sid, exports.max_confirms_considered_cheater, false, game) || exports.should_be_considered_cheater("dev.json", sid, exports.max_confirms_considered_cheater, false, game) || game == 0 && exports.is_sid_a_bot(sid);
}

exports.is_sid_a_bot = function(sid){
  if(exports.is_sid_blacklisted(sid))
    return false;

  return exports.should_be_considered_cheater("dev_bot_list.json", sid, exports.max_confirms_considered_bot, true) || exports.should_be_considered_cheater("pub_bot_list.json", sid, exports.max_confirms_considered_bot, true);
}

exports.get_cheater_by_sid = function(socket, key, sid, game){
  const license = cloud_config.find(key, socket);

  if(license == undefined){
    print(`get_cheater_by_sid: no license (IP: ${sh.ip(socket)}) ${key} [U:1:${sid}]`, true);
    return false;
  }

  exports.make_folders_and_files();
  const is_cheater = exports.is_sid_a_cheater(sid, game);

  return is_cheater;
}

exports.get_bot_by_sid = function(socket, key, sid){
  const license = cloud_config.find(key, socket);

  if(license == undefined){
    print(`get_bot_by_sid: no license (IP: ${sh.ip(socket)}) ${key} [U:1:${sid}]`, true);
    return false;
  }

  exports.make_folders_and_files();

  const is_bot = exports.is_sid_a_bot(sid);

  if(is_bot)
    print(`get_bot_by_sid: "${license.identity}" ran into bot: ${colors.brightRed(`[U:1:${sid}]`)}`);

  return is_bot;
}

exports.is_bot_name = function(socket, key, name_hash){
  if(name_hash == undefined || name_hash == 0)
    return false;
  
  const license = cloud_config.find(key);

  if(license == undefined){
    print(`is_bot_name: no license (IP: ${sh.ip(socket)}) ${key} name hash: ${name_hash}`, true);
    return false;
  }

  exports.make_folders_and_files();

  var data = fs.readFileSync(sh.tf_cheater_list_location + "/bot_name_list.json");
  if(data == undefined)
    return false;

  var json = JSON.parse(data);
  
  delete data;
  if(json == undefined)
    return false;

  var result = false;
  for(var hash of json){
    if(hash == name_hash){
      print(`is_bot_name: ${license.identity} found an sid using a bot name ${name_hash.toString(16)}`);
      result = true;
      break;
    }
  }

  delete json;
  return result;
}

exports.get_personal_alias_by_sid = function(socket, key, sid){
  const license = cloud_config.find(key, socket);

  if(license == undefined){
    print(`get_personal_alias_by_sid: no license (IP: ${sh.ip(socket)}) ${key} [U:1:${sid}]`, true);
    return "";
  }

  if(exports.is_sid_blacklisted(sid))
    return "";

  exports.make_folders_and_files();

  const alias = exports.get_sid_alias_from_list(sid);
  if(alias == undefined || alias.length > 32 || alias.length == 0)
    return "";

  return alias;
}

exports.get_group_by_sid = function(socket, key, sid){
  const license = cloud_config.find(key);
  if(license === undefined)
    return "";

  if(exports.is_sid_blacklisted(sid))
    return "";

  exports.make_folders_and_files();
  let alias = exports.get_sid_group_from_list(sid);
  if(alias == undefined || alias.length > 32 || alias.length == 0)
    return "";

  return alias;
}

exports.submit_cheater_by_sid = function(socket, key, sid, reason_code, server_type, game){
  const license = cloud_config.find(key);

  if(license == undefined){
    exports.print(`submit_cheater_by_sid: no license [U:1:${sid}] ${reason_code} ${server_type}`, true);
    return false;
  }

  if(game == undefined){
    exports.print(`submit_cheater_by_sid: invalid game pushed`, true);
    return false;
  }

  exports.make_folders_and_files();
  if(exports.is_sid_blacklisted(sid)){
    exports.print(`submit_cheater_by_sid: ${license.identity} the sid [U:1:${sid}] is blacklisted.`);
    return false;
  }

  const file_name_to_submit_to = (sh.dev_mode ? "/dev.json" : "/pub.json");

  // The reason code, server type and whatever else may be logged here is solely for anti-db posioning logs.
  const reason_str = exports.get_detection_reason(reason_code);
  if(reason_str == undefined)
    return false;

  // We want to log what type of server they were on for the sole purpose of anti-db posioning.
  const server_type_str = exports.get_server_type(server_type);
  if(server_type_str == undefined || (server_type == 4 || server_type == 1) && !sh.dev_mode)
    return false;

  exports.push_data_to_list(file_name_to_submit_to, sid, server_type_str, reason_str, license, game);

  let msg = `User: ${license.identity}\nID: ${license.uid}\n`;

  msg += `> Reason: ${reason_str}, Server: ${server_type_str}\n`;
  msg += `> Game: ${game.toString(16)}\n`;
  msg += `https://steamcommunity.com/profiles/[U:1:${sid}]\n`

  sh.send_discord_webhook("sid_submit", msg);

  print(`submit_cheater_by_sid: ${license.identity} submitted a cheater ${colors.brightRed(`[U:1:${sid}]`)} for the reason ${reason_str} on a server type of ${server_type_str}`);

  fs.appendFile(sh.tf_cheater_list_location + "/submitted_cheaters.log", `${license.identity} submitted [U:1:${sid}] as a cheater with the reason ${reason_str} ${server_type_str}\n`, err =>{
  });
  return true;
}

exports.submit_bot_by_sid = function(socket, key, sid){
  const license = cloud_config.find(key);

  print(`submit_bot_by_sid: ignoring detections, functionality is to not be restored.`);
  return false;

  if(license == undefined){
    print(`submit_bot_by_sid: no license (IP: ${sh.ip(socket)}) ${key} [U:1:${sid}]`, true);
    return false;
  }

  // Lets just assume now that bots are going to be on newer accounts so ignore low sid accounts as false positives.
  let sid_str = sid.toString();
  if(sid_str.length == undefined || sid_str.length < 10){
    print(`submit_bot_by_sid: ${license.identity} ignoring [U:1:${sid}] because it's too old to be considered a bot account. (1)`);
    return false;
  }

  let sid_start_amount = Number(sid_str[0] + sid_str[1]);
  if(sid_str.length == 10 && sid_start_amount < 17){
    print(`submit_bot_by_sid: ${license.identity} ignoring [U:1:${sid}] because it's too old to be considered a bot account. (2)`);
    return false;
  }

  exports.make_folders_and_files();
  if(exports.is_sid_blacklisted(sid))
    return false;

  const file_name_to_submit_to = (sh.dev_mode ? "/dev_bot_list.json" : "/pub_bot_list.json");
  print(`submit_bot_by_sid: "${license.identity}" Submitted bot: ${colors.brightRed(`[U:1:${sid}]`)}`);

  var game = undefined;
  exports.push_data_to_list(file_name_to_submit_to, sid, "UNKNOWN", "(BOT SUBMITTED)", license, game);

//  sh.send_discord_webhook(sh.discord_tfcl_submit_url, `${license.identity} submitted a bot\nhttps://steamcommunity.com/profiles/[U:1:${sid}]`);
  fs.appendFile(sh.tf_cheater_list_location + "/submitted_bots.log", `${license.identity} submitted [U:1:${sid}] as a bot.\n`, err =>{
  });

  exports.submit_cheater_by_sid(socket, key, sid, 6, 0);
  return true;
}

exports.get_steam_group_details = function(socket, key, sid){
  const license = cloud_config.find(key, socket);

  if(license == undefined){
    print(`get_steam_group_details: no license (IP: ${sh.ip(socket)}) ${key} [U:1:${sid}]`, true);
    return {cheater_group_count: 0, bot_group_count: 0};
  }

  exports.make_folders_and_files();
  if(exports.is_sid_blacklisted(sid))
    return {cheater_group_count: 0, bot_group_count: 0};

  var cc = 0;
  var bc = 0;
  var group_name = undefined;

  try{
    for(var name in exports.steam_group_list){
      if(name === undefined)
        continue;

      var entry = JSON.parse(exports.steam_group_list[name]);
   
      if(entry.members.includes(sid)){
        if(entry.cheater)
          cc++;

        if(entry.bc)
          bc++;

        if(entry.group_name != undefined){
          group_name = entry.group_name;
          break;
        }
      }
    }
  }
  catch(err){
    exports.print(`[-] Failed to parse steam group list: ${err}`);
  }

  if(cc > 0 || bc > 0)
    print(`get_steam_group_details: ${license.identity} found sid [U:1:${sid}] that's in ${cc} cheater groups and ${bc} bot groups.`);

  return {cheater_group_count: cc, bot_group_count: bc, auto_group_name: group_name};
}

exports.submit_cheater_from_sync_server = async function(cheater_list, server_name){
  if(cheater_list == undefined || server_name == undefined)
    return false;

  exports.make_folders_and_files();

  const file_name_to_submit_to = sh.tf_cheater_list_location + (sh.dev_mode ? "/dev.json" : "/pub.json");
  exports.print(`submit_cheater_from_sync_server: ${server_name}`);

  var data = fs.readFileSync(file_name_to_submit_to);
  if(data == undefined){
    print(`Can't find ${file_name_to_submit_to}`, true);
    return false;
  }

  var json = JSON.parse(data);
  delete data;
  if(json == undefined){
    print(`submit_cheater_from_sync_server: Can't parse json file ${file_name_to_submit_to}`, true);
    return false;
  }

  try{
    for(var sid in cheater_list){
      var data = cheater_list[sid];
      if(data == undefined)
        continue;

      // If needed, lets fix any issues.
      data = fix_table(data);
  
      // If it doesn't exist, lets go ahead just copy over the data.
      if(json[sid] == undefined){
        json[sid] = data;
        exports.print(`submit_cheater_from_sync_server: [U:1:${sid}] was added from (${server_name})`);
        continue;
      }
  
      // Add stuff
      if(json[sid].last_submit_time == undefined || json[sid].last_submit_time < data.last_submit_time)
        json[sid].last_submit_time = data.last_submit_time;
  
      if(json[sid].last_seen_time == undefined || json[sid].last_seen_time < data.last_seen_time)
        json[sid].last_seen_time   = data.last_seen_time;
  
      var announce_update = false;

      if(json[sid].detections == undefined)
        json[sid].detections = {};

      if(json[sid].submitters == undefined)
        json[sid].submitters = [];

      for(var uid in data.detections){
        if(json[sid] == undefined)
          json[sid].detections[uid] = data.detections[uid];
      }
    
      try{
        for(var submitter of data.submitters){
          if(!json[sid].submitters.includes(submitter)){
            json[sid].submitters.push(submitter);
            announce_update = true;
          }
        }
      }
      catch{}
  
      if(data.games != undefined){
        for(var h in data.games){
          if(json[sid].games == undefined)
            json[sid].games = [];

          var game_hash = Number(data.games[h]);
          if(game_hash <= 1000)
            continue;

          if(!json[sid].games.includes(game_hash))
            json[sid].games.push(game_hash);
        }
      }
  
      if(announce_update)
        exports.print(`submit_cheater_from_sync_server: [U:1:${sid}] was updated using data from (${server_name})`);
    }
  
    fs.writeFileSync(file_name_to_submit_to, JSON.stringify(json, null, 2));
  }
  catch(err){
    exports.print(`[-] submit_cheater_from_sync_server error: ${err}`);
  }

  delete json;
  return true;
}

exports.submit_bot_from_sync_server = async function(cheater_list, server_name){
  if(cheater_list == undefined || server_name == undefined)
    return false;

  exports.make_folders_and_files();

  const file_name_to_submit_to = sh.tf_cheater_list_location + (sh.dev_mode ? "/dev_bot_list.json" : "/pub_bot_list.json");
  exports.print(`submit_bot_from_sync_server: ${server_name}`);

  var data = fs.readFileSync(file_name_to_submit_to);
  if(data == undefined){
    print(`submit_bot_from_sync_server: Can't find ${file_name_to_submit_to}`, true);
    return false;
  }

  var json = JSON.parse(data);
  delete data;
  if(json == undefined){
    print(`submit_bot_from_sync_server: Can't parse json file ${file_name_to_submit_to}`, true);
    return false;
  }

  try{
    for(var sid in cheater_list){
    var data = cheater_list[sid];
    if(data == undefined)
      continue;

    // If it doesn't exist, lets go ahead just copy over the data.
    if(json[sid] == undefined){
      json[sid] = data;
      exports.print(`submit_bot_from_sync_server: [U:1:${sid}] was added from (${server_name})`);
      continue;
    }

    // Add stuff
    if(json[sid].last_submit_time < data.last_submit_time)
      json[sid].last_submit_time = data.last_submit_time;

    if(json[sid].last_seen_time < data.last_seen_time)
      json[sid].last_seen_time   = data.last_seen_time;

    var announce_update = false;
    for(var uid in data.detections){
      if(json[sid] == undefined)
        json[sid] = data.detections[uid];
    }

    for(var submitter of data.submitters){
      if(!json[sid].submitters.includes(submitter)){
        json[sid].submitters.push(submitter);
        announce_update = true;
      }
    }

    if(announce_update)
      exports.print(`submit_bot_from_sync_server: [U:1:${sid}] was updated using data from (${server_name})`);
    }
  
  
    fs.writeFileSync(file_name_to_submit_to, JSON.stringify(json, null, 2));
  }
  catch(err){
    exports.print(`[-] submit_cheater_from_sync_server error: ${err}`);
  }

  delete json;
  return true;
}

exports.send_cheater_list = function(){
  const list_file_name = sh.tf_cheater_list_location + (sh.dev_mode ? "/dev.json" : "/pub.json");
  const bot_file_name  = sh.tf_cheater_list_location + (sh.dev_mode ? "/dev_bot_list.json" : "/pub_bot_list.json");
  var   cheater_buffer = fs.readFileSync(list_file_name);
  var   bot_buffer     = fs.readFileSync(bot_file_name);

  if(cheater_buffer == undefined)
    return;

  var cheater_json = JSON.parse(cheater_buffer);
  if(cheater_json == undefined)
    return;

  delete cheater_buffer;
  if(bot_buffer == undefined)
    return;

  var bot_json = JSON.parse(bot_buffer);
  if(bot_json == undefined)
    return

  delete bot_buffer;
  var dat = sync_server.setup_response_json("cheater_list_data");

  dat.data     = JSON.stringify(cheater_json, null, 2);
  dat.bot_data = JSON.stringify(bot_json, null, 2);

  delete cheater_json;
  delete bot_json;

  sync_server.send_json_data(dat);
  exports.print("send_cheater_list: Transmitting");
}

exports.fetch_steam_group_list = async function(){
  var response = sync_server.setup_response_json("get_steam_group_data");
  
  var json = await sync_server.send_json_data(response); 
  if(json !== undefined)
    exports.steam_group_list = json;
}

exports.update_known_actors = async function(){
  var response = sync_server.setup_response_json("fetch_alias_list");

  var json = await sync_server.send_json_data(response);
  if(json !== undefined){
    if(json.ok !== undefined)
      return;
    
    fs.writeFileSync(sh.tf_cheater_list_location + "/alias.json", JSON.stringify(json, null, 2));
  }
}

exports.update_known_actors();
setInterval(exports.update_known_actors, 60000); // Update every 15 seconds.

exports.send_cheater_list();
setInterval(exports.send_cheater_list, 60000 * 15);

exports.fetch_steam_group_list();
setInterval(exports.fetch_steam_group_list, 60000);