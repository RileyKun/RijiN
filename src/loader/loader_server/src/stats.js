const fs      = require("fs");
const colors  = require('colors');
const sh      = require("./shared.js");


function get_date_format(){
  const date = new Date();
  return date.getFullYear() + "-" + (date.getMonth() + 1) + "-" + date.getDate();
}

var start_date_format = get_date_format();

exports.stats_table                                         = {};
exports.stats_table[start_date_format]                      = {};
exports.stats_table[start_date_format].error_code_stats     = {};
exports.stats_table[start_date_format].extension_type_stats = {};
exports.stats_table[start_date_format].security             = {};
exports.stats_table[start_date_format].tfcl_queries         = 0;
exports.stats_table[start_date_format].logins_pending       = 0;
exports.stats_table[start_date_format].logins_finished      = 0;
exports.stats_table[start_date_format].logins_uids          = [];

exports.has_loaded_stats_from_save                          = false;

// helpers / fetchers
function print(str, error){
  if(error != undefined)
    str = colors.bgRed("STATS-ERROR: " + str);

  str = colors.brightCyan("[STATS] ") + str;

  console.log(str);
}
exports.print = print;

exports.init_date = function(){
  start_date_format = get_date_format();
  if(exports.stats_table[start_date_format] != undefined)
    return;

  // Since the day of the tables are going to change, we should probably have a function to reinit the tables.
  exports.stats_table[start_date_format]                      = {};
  exports.stats_table[start_date_format].error_code_stats     = {};
  exports.stats_table[start_date_format].extension_type_stats = {};
  exports.stats_table[start_date_format].security             = {};
  exports.stats_table[start_date_format].tfcl_queries         = 0;
  exports.stats_table[start_date_format].logins_pending       = 0;
  exports.stats_table[start_date_format].logins_finished      = 0;
  exports.stats_table[start_date_format].logins_uids          = [];

  exports.print(`[+] Created new entries for date ${start_date_format}`);
}

exports.make_folders_and_files = function(){
  if(!fs.existsSync(sh.stats_data_location))
    fs.mkdirSync(sh.stats_data_location);
  
  if(!fs.existsSync(sh.stats_data_location + "/stat_data.json"))
    fs.writeFileSync(sh.stats_data_location + "/stat_data.json", JSON.stringify(exports.stats_table, null, 2));
  else{
    if(exports.has_loaded_stats_from_save)
      return;

    exports.has_loaded_stats_from_save = true;
    exports.print(`[+] Attempting to read data from file.`);
    try{
      exports.stats_table = JSON.parse(fs.readFileSync(sh.stats_data_location + "/stat_data.json"));
      exports.print(`[+] Assuming we succesfully read from file.`);
    }
    catch(err){
      exports.print(err, true);
    }
  }

  exports.init_date();
}

exports.make_folders_and_files();

exports.send_backend_issue_alert = function(what){
  if(sh.staging_mode)
    return;

  if(what == undefined)
    return;

  if(what.message == undefined)
    what.message = "";

  if(what.stack == undefined)
    what.stack = "";
}

exports.should_ignore_code = function(code){
  if(code == undefined)
    return false;

  // login errors from invalid username / password.
  if(code >= 2 && code <= 5)
    return true;

  return false;
}

exports.submit_error_code = function(error_code){
  if(error_code == undefined)
    return false;

  exports.make_folders_and_files();
  if(exports.should_ignore_code(error_code))
    return true;

  if(error_code == 55)
    exports.send_backend_issue_alert("Headsup, there's a product that does not have a stub procedure!");

  error_code = error_code.toString();

  try{
    if(exports.stats_table[start_date_format].error_code_stats[error_code] === undefined)
      exports.stats_table[start_date_format].error_code_stats[error_code] = {hits: 1, last_seen_time: sh.time()};
    else{
      exports.stats_table[start_date_format].error_code_stats[error_code].hits++;
      exports.stats_table[start_date_format].error_code_stats[error_code].last_seen_time = sh.time();
    }
  }
  catch(err){
    exports.print(err, true);
    exports.init_date();
  }

  return true;
}

exports.submit_math_extension = function(extension_type){
  if(extension_type === undefined || extension_type === null || extension_type.length == undefined || extension_type.length == 0)
    return false;

  exports.make_folders_and_files();
  extension_type = extension_type.replaceAll(".dll", "").replaceAll("bin/", "");
  extension_type = extension_type.trim();

  try{
    if(exports.stats_table[start_date_format].extension_type_stats[extension_type] == undefined)
      exports.stats_table[start_date_format].extension_type_stats[extension_type] = {hits: 1, last_seen_time: sh.time()};
    else{
      exports.stats_table[start_date_format].extension_type_stats[extension_type].hits++;
      exports.stats_table[start_date_format].extension_type_stats[extension_type].last_seen_time = sh.time();
    }
  }
  catch(err){
    exports.print(err, true);
    exports.init_date();
  }

  return true;
}

exports.submit_security_detection = function(code){
  if(code == undefined)
    return false;

  exports.make_folders_and_files();

  code = code.toString();

  try{
    if(exports.stats_table[start_date_format].security[code] == undefined)
      exports.stats_table[start_date_format].security[code] = {hits: 1, last_seen_time: sh.time()};
    else{
      exports.stats_table[start_date_format].security[code].hits++;
      exports.stats_table[start_date_format].security[code].last_seen_time = sh.time();
    }
  }
  catch(err){
    exports.print(err, true);
    exports.init_date();
  }

  return true;
}

exports.track_tfcl_queries = function(){
  exports.make_folders_and_files();
  try{
    if(exports.stats_table[start_date_format].tfcl_queries == undefined)
      exports.stats_table[start_date_format].tfcl_queries = 1;
    else
      exports.stats_table[start_date_format].tfcl_queries++;
  }
  catch(err){
    exports.print(err, true);
    exports.init_date();
  }
}

exports.track_login_start = function(){
  exports.make_folders_and_files();
  try{
    if(exports.stats_table[start_date_format].logins_pending == undefined)
      exports.stats_table[start_date_format].logins_pending = 1;
    else{
      exports.stats_table[start_date_format].logins_pending++;
      if(exports.stats_table[start_date_format].logins_pending >= 10) // This is bad if this is happening.
        exports.send_backend_issue_alert("Logins are not being processed currently ``" + exports.stats_table[start_date_format].logins_pending + "`` not being processed.");
    }
  }
  catch{}
}

exports.track_login_end = function(time_taken, steamids_list){
  exports.make_folders_and_files();
  try{

    if(time_taken >= 3)
      exports.send_backend_issue_alert("Bad server performance! A login took ``" + time_taken + "`` seconds to finished!\n> SID list count: ``" + steamids_list.length + "``");
    
    if(exports.stats_table[start_date_format].logins_finished == undefined)
      exports.stats_table[start_date_format].logins_finished = 1;
    else{
      exports.stats_table[start_date_format].logins_finished++;
      if(exports.stats_table[start_date_format].logins_pending > 0)
        exports.stats_table[start_date_format].logins_pending--;
    }
  }
  catch{}
}

// For now we're gonna report error codes, Something we kinda need.
exports.report_stats = function(){
  exports.make_folders_and_files();
  if(sh.dev_mode || sh.staging_mode)
    return;
  
  fs.writeFileSync(sh.stats_data_location + "/stat_data.json", JSON.stringify(exports.stats_table, null, 2));
}

exports.submit_login = function(uid){
  if(uid == undefined)
    return;

  if(exports.stats_table[start_date_format].logins_uids == undefined)
    return;

  try{
    uid = Number(uid);
    if(exports.stats_table[start_date_format].logins_uids.includes(uid))
      return;

    exports.stats_table[start_date_format].logins_uids.push(uid);
  }
  catch(err){
    exports.send_backend_issue_alert(err);
  }
}

setInterval(exports.report_stats, 60000 * 5);