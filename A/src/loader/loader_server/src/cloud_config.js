// custom
const sh     = require("./shared.js");
const fs     = sh.fs;
const colors = sh.colors;

exports.license_list = {};
exports.license_expire_time = 86400 + 3600; // Allow for the game to close or cheat unload.
exports.license_cache_save_time = 10;
exports.license_clean_up_time   = 10;
exports.max_config_size_b = 500000/*500kb*/;
exports.playerlist_config_id = 7;
exports.max_config_playerlist_size_b = 20000000;/*20MB*/
exports.request_wait_time = 1;

// helpers / fetchers
function print(str, error){
  if(error != undefined)
    str = colors.bgRed("ERROR: " + str);

  str = colors.brightCyan("[CLOUD] ") + str;

  console.log(str);
}
exports.print = print;

exports.generate_key = function(len){
  const p = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  return [...Array(len)].reduce(a=>a+p[~~(Math.random()*p.length)],'');
}

exports.create = function(socket, identity, uid, name, max_configs){
  const license_key = exports.generate_key(32);
  let ip = sh.ip(socket);

  if(exports.license_list[identity] != undefined){
    if(exports.license_list[identity].expire > sh.time()){
      if(exports.license_list[identity].updates == undefined || exports.license_list[identity].updates < 3){

        exports.license_list[identity].identity          = identity;
        exports.license_list[identity].uid               = uid;
        exports.license_list[identity].expire_seconds    = exports.license_expire_time;
        exports.license_list[identity].expire            = sh.time() + exports.license_expire_time;
        exports.license_list[identity].next_request_time = sh.time() - 1;
        exports.license_list[identity].name              = name;
        exports.license_list[identity].max_configs       = max_configs;

        if(exports.license_list[identity].ips == undefined)
          exports.license_list[identity].ips = [ip];
        else{
          if(!exports.license_list[identity].ips.includes(ip))
            exports.license_list[identity].ips.push(ip);
        }

        if(exports.license_list[identity].updates == undefined)
          exports.license_list[identity].updates = 1;
        else
          exports.license_list[identity].updates++;

        let key = exports.license_list[identity].key;
        sh.print(socket, `[+] @IP: Updating cloud config license "${key}" for game "${name}"`);

        // Save license table to file.
        exports.save_sessions_to_file();

        return exports.license_list[identity];
      }
    }
  }

  // Delete any existing license if we have one
  delete exports.license_list[identity];

  exports.license_list[identity] = {
    identity:           identity,
    uid:                uid,
    key:                license_key,
    expire_seconds:     exports.license_expire_time,
    expire:             sh.time() + exports.license_expire_time,
    next_request_time:  sh.time() - 1,
    name:               name,
    max_configs:        max_configs,
    ips:                [ip],
  };

  sh.print(socket, `[+] @IP: Created cloud config license "${license_key}" for game "${name}"`);
  sh.print(socket, `[!] @IP: Cloud config license expires in ${exports.license_expire_time} seconds`);

  // Save the license table to a file
  exports.save_sessions_to_file();

  return exports.license_list[identity];
}

exports.valid = function(config_license){
  if(config_license == undefined)
    return false;

  return config_license.expire > sh.time();
}

exports.clean_up = function(){
  for(var k in exports.license_list){
    const entry = exports.license_list[k];

    if(!exports.valid(entry)){
      sh.print(undefined, `[+] @IP: Deleted expired cloud config license "${entry.key}" for "${entry.name}"`);
      delete exports.license_list[k];
    }
  }
}

exports.make_folders = function(license){
  var folder = sh.cloud_configs_location;

  if(!fs.existsSync(folder))
    fs.mkdirSync(folder);

  folder += `/${license.name}`;

  if(!fs.existsSync(folder))
    fs.mkdirSync(folder);

  let old_folder = folder + `/${license.identity}`;

  folder += `/${license.uid}`;

  if(fs.existsSync(old_folder) && !fs.existsSync(folder))
    fs.renameSync(old_folder, folder);
  else if(fs.existsSync(old_folder) && fs.existsSync(folder))
    fs.rmdir(old_folder, {recursive: true, force: true});
  else{
    if(!fs.existsSync(folder))
      fs.mkdirSync(folder);
  }

  return folder;
}

exports.find = function(key, socket){
  for(var k in exports.license_list)
    if(exports.license_list[k].key == key && exports.valid(exports.license_list[k])){
      if(socket != undefined){
        let ip = sh.ip(socket);
        if(exports.license_list[k].ips == undefined)
          exports.license_list[k].ips = [ip];
        else if(!exports.license_list[k].ips.includes(ip))
          exports.license_list[k].ips.push(ip);

      }
      return exports.license_list[k];
    }

  return undefined;
}

exports.find_from_ip = function(ip){;
  for(var k in exports.license_list){
    if(exports.license_list[k].ips == undefined)
      continue;

    if(exports.license_list[k].ips.includes(ip) && exports.valid(exports.license_list[k]))
      return exports.license_list[k];
  }
  return undefined;
}

exports.request_config = function(socket, key, id){
  const license = exports.find(key, socket);

  if(license == undefined){
    print(`${sh.ip(socket)}: No license could be found`, true);
    return undefined;
  }

  if(license.next_request_time >= sh.time()){
    print(`${license.identity}: Too small of a request time difference`, true);
    return undefined;
  }

  license.next_request_time = sh.time() + exports.request_wait_time;

  if(id < 0 || id >= license.max_configs){
    print(`${license.identity}: Config ID ${id} larger than ${license.max_configs - 1}`, true);
    return undefined;
  }

  const folder_location = exports.make_folders(license);
  const file_location   = `${folder_location}/${id}.cfg`;

  if(!fs.existsSync(file_location)){
    print(`${license.identity}: Configuration file ${file_location} does not exist`, true);
    return undefined;
  }

  let config_buffer = fs.readFileSync(file_location);

  if(config_buffer.length > exports.max_config_size_b && id != exports.playerlist_config_id || config_buffer.length > exports.max_config_playerlist_size_b && id == exports.playerlist_config_id ){
    delete config_buffer;
    config_buffer = undefined;
    print(`${license.identity}: Configuration file ${file_location} is too large`, true);
    return undefined;
  }

  print(`${license.identity}: Configuration file ${file_location} loaded`);

  // TODO: MAGIC FILE CHECK TO MAKE SURE THE FILE IS SOMEHOW NOT A FILE WE WONT WANT TO GIVE THEM
  // TODO: CLEAR THAT MAGIC FILE NUMBER BEFORE SENDING IT TO THE SERVER FOR REFERENCE

  return config_buffer;
}

exports.submit_config = function(socket, key, id, buffer){
  const license = exports.find(key, socket);

  if(license == undefined){
    print(`${sh.ip(socket)}: No license could be found`, true);
    return undefined;
  }

  if(license.next_request_time >= sh.time()){
    print(`${license.identity}: Too small of a request time difference`, true);
    return undefined;
  }

  if(id < 0 || id >= license.max_configs){
    print(`${sh.ip(socket)}: Config ID ${id} larger than ${license.max_configs - 1}`, true);
    return undefined;
  }

  const folder_location = exports.make_folders(license);
  const file_location   = `${folder_location}/${id}.cfg`;

  fs.writeFileSync(file_location, buffer);

  print(`${license.identity}: Configuration file ${file_location} saved`);

  license.next_request_time = exports.request_wait_time;

  return true;
}

exports.does_config_exist = function(id){
  const folder_location = exports.make_folders(license);
  const file_location   = `${folder_location}/${id}.cfg`;
  return fs.existsSync(file_location);
}

exports.load_sessions_from_file = function(){
  if(!fs.existsSync(sh.cloud_configs_cache_location))
    return;

  try{
    exports.license_list = JSON.parse(fs.readFileSync(sh.cloud_configs_cache_location).toString());
    print(`Loaded ${Object.keys(exports.license_list).length} sessions from ${sh.cloud_configs_cache_location}`);
    exports.clean_up();
  }
  catch(e){
    print(`[-] Failed to load sessions from ${sh.cloud_configs_cache_location}`);

    if(fs.existsSync(sh.cloud_configs_cache_location))
      fs.unlinkSync(sh.cloud_configs_cache_location);
  }
}

// Save all cloud licenses to a cache file so that if the server is restarted
// We can load all of these cloud sessions again
exports.save_sessions_to_file = function(){
  if(Object.keys(exports.license_list).length < 1){
    if(fs.existsSync(sh.cloud_configs_cache_location))
      fs.unlinkSync(sh.cloud_configs_cache_location);

    return;
  }

  if(!fs.existsSync(sh.cloud_configs_location))
    fs.mkdirSync(sh.cloud_configs_location);

  try{
    fs.writeFileSync(sh.cloud_configs_cache_location, JSON.stringify(exports.license_list, null, 4));
    //print(`[-] Saved sessions to ${sh.cloud_configs_cache_location}`);
  }
  catch(e){
    print(`[-] Failed to save sessions to ${sh.cloud_configs_cache_location}`);
  }
}


function clean_up_old_configs(){
  var folder = sh.cloud_configs_location + "/";
  if(!fs.existsSync(folder))
    return;

  print(`[+] Looking for old entries in terms of configs. ${folder}`);

  const now         = new Date();
  const expire_date = new Date();
  expire_date.setMonth(now.getMonth() - 3);

  const games = [];
  fs.readdir(folder, (err, files) =>{
    if(err){
      print(`[-] Failed to clean_up: ${err}`);
      return;
    }

    // Fetch all of the cloud config directories we support.
    for(const f of files){
      if(f.includes(".dat"))
        continue;

      games.push(f);
    }

    // Find our user's config files.
    for(const g of games){
      fs.readdir(folder + g + "/", (err, configs) =>{
        if(err){
          print(`[-] Failed to clean_up configs: ${err}`);
          return;
        }

        // We're gonna read their config files themselves, if they haven't been changed in a while, then we delete all of their configs.
        for(const c of configs){
          const config_path = folder + g + "/" + c;
          fs.readdir(config_path, (err, data_files) =>{
            if(err){
              print(`[-] Failed to data on ${err} -> ${config_path}`);
              return;
            }

            var should_delete = true;
            for(var f of data_files){
              const stats         = fs.statSync(config_path + "/" + f);
              const last_modified = new Date(stats.mtime)
 
              // We do not want to delete this user's config.
              if(last_modified > expire_date){
                should_delete = false;
                break;
              }
            }

            // Lets clean up their configs.
            if(should_delete){
              fs.rmdir(config_path, {recursive: true}, (err) =>{
                if(err){
                  print(`[-] Failed to delete: ${config_path}`);
                  return;
                }

                print(`[+] Deleted config: ${config_path}`);
              });
            }

          });
        }
      });
    }
  });
}

clean_up_old_configs();

setInterval(clean_up_old_configs, 60000 * 60);

setInterval(function(){
  exports.clean_up();
}, exports.license_clean_up_time * 1000);