// custom
const sh         = require("./shared.js");
const fs         = sh.fs;

exports.list = [];
exports.expire_time = 86400;

exports.MASTER_LICENSE_ID = 1;
exports.TF2_LICENSE_ID  = 2;
exports.TF2_BETA_LICENSE_ID = 12;
exports.OW2_LV1_LICENSE_ID = 5;
exports.OW2_LV2_LICENSE_ID = 6;
exports.OW2_EXT_LICENSE_ID = 13;
exports.DOD_SOURCE_LICENSE_ID = 9;
exports.CS_SOURCE_LICENSE_ID = 11;
exports.FOF_LICENSE_ID = 10;
exports.HL2DM_LICENSE_ID = 5;
exports.HLSDM_LICENSE_ID = 15;
exports.DAB_LICENSE_ID = 6;
exports.L4D2_LICENSE_ID = 4;
exports.GMOD_LICENSE_ID = 16;
exports.CODENAME_CURE_LICENSE_ID = 17;
exports.TF2_LITE_LICENSE_ID = 3;
exports.HL2_LICENSE_ID = 18;
exports.SYNERGY_LICENSE_ID = 21;
exports.BATTLE_GROUNDS_III = 22;

exports.license_list = [
  {
    "requires_access_id": [exports.TF2_LICENSE_ID, exports.HL2_LICENSE_ID, exports.SYNERGY_LICENSE_ID, exports.TF2_LITE_LICENSE_ID, exports.L4D2_LICENSE_ID, exports.TF2_BETA_LICENSE_ID, exports.DOD_SOURCE_LICENSE_ID, exports.CS_SOURCE_LICENSE_ID, exports.FOF_LICENSE_ID, exports.HL2DM_LICENSE_ID, exports.HLSDM_LICENSE_ID, exports.DAB_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "vac_bypass",
    "file_name":          "vac_bypass",
    "target":             "steam.exe",
    "background_module":  true,
  },
  {
    "requires_access_id": [exports.TF2_LICENSE_ID, exports.TF2_LITE_LICENSE_ID, exports.L4D2_LICENSE_ID, exports.TF2_BETA_LICENSE_ID, exports.DOD_SOURCE_LICENSE_ID, exports.CS_SOURCE_LICENSE_ID, exports.FOF_LICENSE_ID, exports.HL2DM_LICENSE_ID, exports.HLSDM_LICENSE_ID, exports.DAB_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "Clean Steam",
    "file_name":          "",
    "target":             "",
    "vac_mode":           false,
    "icon":               "steam",
    "runs_on_stub":       true
  },
  {
    "requires_access_id": [exports.GMOD_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "Garry's Mod 64-bit",
    "file_name":          "garrysmod64",
    "target":             "gmod.exe",
    "d3dx":               9,
    "vac_mode":           false,
    "appid":              4000,
    "cloud":              true,
    "cloud_name":         "garrysmod",
    "cloud_max_configs":  10,
    "icon":               "gmod"
  },
  //{
  //  "requires_access_id": [exports.GMOD_LICENSE_ID, exports.MASTER_LICENSE_ID],
  //  "disabled":           false,
  //  "beta":               false,
  //  "name":               "Garry's Mod 32-bit",
  //  "file_name":          "garrysmod86",
  //  "target":             "hl2.exe",
  //  "d3dx":               9,
  //  "vac_mode":           true,
  //  "appid":              4000,
  //  "cloud":              true,
  //  "cloud_name":         "garrysmod",
  //  "cloud_max_configs":  10,
  //  "icon":               "gmod"
  //},
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Battle Grounds III",
    "file_name":          "bg3",
    "target":             "bg3.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              1057700,
    "cloud":              true,
    "cloud_name":         "bg3",
    "cloud_max_configs":  10,
    "icon":               "bg3"
  },
  {
    "requires_access_id": [exports.TF2_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "Team Fortress 2 64-bit",
    "file_name":          "tf2_win64",
    "target":             "tf_win64.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              440,
    "cloud":              true,
    "cloud_name":         "tf2_win64",
    "cloud_max_configs":  10,
    "icon":               "tf2"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Team Fortress 2 Lite 64-bit",
    "file_name":          "tf2_lite_win64",
    "target":             "tf_win64.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              440,
    "cloud":              true,
    "cloud_name":         "tf2_win64",
    "cloud_max_configs":  10,
    "icon":               "tf2",
    "test_branch_only":   true
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Synergy",
    "file_name":          "synergy",
    "target":             "synergy.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              17520,
    "cloud":              true,
    "cloud_name":         "synergy",
    "cloud_max_configs":  10,
    "icon":               "synergy"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Half Life: 2",
    "file_name":          "hl2",
    "target":             "hl2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              220,
    "cloud":              true,
    "cloud_name":         "hl2",
    "cloud_max_configs":  10,
    "icon":               "hl2"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Half Life 2: Deathmatch",
    "file_name":          "hl2dm",
    "target":             "hl2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              320,
    "cloud":              true,
    "cloud_name":         "hl2dm",
    "cloud_max_configs":  10,
    "icon":               "hl2dm"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Half Life: 2 Episode One",
    "file_name":          "hl2_ep1",
    "target":             "hl2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              380,
    "cloud":              true,
    "cloud_name":         "hl2",
    "cloud_max_configs":  10,
    "icon":               "hl2"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Half Life: 2 Episode Two",
    "file_name":          "hl2_ep2",
    "target":             "hl2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              420,
    "cloud":              true,
    "cloud_name":         "hl2",
    "cloud_max_configs":  10,
    "icon":               "hl2"
  },
  {
    "requires_access_id": [exports.DAB_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "Double Action: Boogaloo",
    "file_name":          "dab",
    "target":             "hl2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              317360,
    "cloud":              true,
    "cloud_name":         "dab",
    "cloud_max_configs":  10,
    "icon":               "dab"
  },
  //{
  //  "requires_access_id": [exports.HLSDM_LICENSE_ID, exports.MASTER_LICENSE_ID],
  //  "disabled":           true,
  //  "beta":               false,
  //  "name":               "Half Life Source: Deathmatch",
  //  "file_name":          "hlsdm",
  //  "target":             "hl2.exe",
  //  "d3dx":               9,
  //  "vac_mode":           true,
  //  "appid":              360,
  //  "cloud":              true,
  //  "cloud_name":         "hlsdm",
  //  "cloud_max_configs":  5,
  //  "icon":               "hlsdm"
  //},
  {
    "requires_access_id": [exports.L4D2_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "Left 4 Dead 2",
    "file_name":          "l4d2",
    "target":             "left4dead2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              550,
    "cloud":              true,
    "cloud_name":         "l4d2",
    "cloud_max_configs":  10,
    "icon":               "l4d2"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Day of Defeat: Source",
    "file_name":          "dods",
    "target":             "dod_win64.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              300,
    "cloud":              true,
    "cloud_name":         "dods",
    "cloud_max_configs":  10,
    "icon":               "dods"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Counter Strike: Source",
    "file_name":          "css",
    "target":             "cstrike_win64.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              240,
    "cloud":              true,
    "cloud_name":         "css",
    "cloud_max_configs":  10,
    "icon":               "css"
  },
  {
    "requires_access_id": [exports.FOF_LICENSE_ID, exports.MASTER_LICENSE_ID],
    "disabled":           false,
    "beta":               false,
    "name":               "Fistful of Frags",
    "file_name":          "fof",
    "target":             "hl2.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              265630,
    "cloud":              true,
    "cloud_name":         "fof",
    "cloud_max_configs":  10,
    "icon":               "fof"
  },
  {
    "requires_access_id": [],
    "disabled":           false,
    "beta":               false,
    "name":               "Codename: Cure",
    "file_name":          "codename_cure",
    "target":             "cure.exe",
    "d3dx":               9,
    "vac_mode":           true,
    "appid":              355180,
    "cloud":              true,
    "cloud_name":         "codename_cure",
    "cloud_max_configs":  10,
    "icon":               "codename_cure",
    "test_branch_only":   true
  },
  // Dev packages
  {
    "disabled":  false,
    "beta":      false,
    "name":      "GFXTest DX9",
    "file_name": "gfxtest_dx9",
    "target":    "hl2.exe",
    "d3dx":      9
  },
  {
    "disabled":  false,
    "beta":      false,
    "name":      "GFXTest DX11",
    "file_name": "gfxtest_dx11",
    "target":    "Overwatch.exe",
    "d3dx":      11
  }
];

if(!sh.dev_mode && !sh.staging_mode){
  for(var i in exports.license_list){
    var entry = exports.license_list[i];
    if(entry.test_branch_only != undefined && entry.test_branch_only){
      console.log(`[!] Removing license ${entry.name} from list because it's marked for testing only.`);
      delete exports.license_list[i];
    }
  }
}

exports.clean_invalid_entries = function(){
  if(sh.dev_mode)
    return;

  for(var i in exports.license_list){
    const entry = exports.license_list[i];
    if(entry.runs_on_stub || entry.background_module)
      continue;

    if(fs.existsSync(`./bin/${entry.file_name}_sse2.dll`) || fs.existsSync(`./bin/${entry.file_name}_sse4.2.dll`))
      continue;

    console.log(`No DLL exists for ${entry.name}! Removing from lists`);
    delete exports.license_list[i];
  }
}

exports.clean_invalid_entries();

exports.generate_key = function(len){
  const p = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  return [...Array(len)].reduce(a=>a+p[~~(Math.random()*p.length)],'');
}

exports.delete_self = function(socket){
  if(socket == undefined)
      return undefined;

  const ip = sh.ip(socket);

  if(ip == undefined)
      return undefined;

  delete exports.list[ip];
}

// Rather than delete it instantly, we'll set the expire time to 15 sceonds.
exports.pop_license = function(socket){
  if(socket == undefined)
    return undefined;

  const ip = sh.ip(socket);

  if(ip == undefined)
    return undefined;

  if(exports.list[ip].expire <= sh.time() + 15)
    return;

  exports.list[ip].expire = sh.time() + 15;
}

exports.create = function(socket, identity, uid, objects){
  if(socket == undefined)
      return undefined;

  const ip = sh.ip(socket);

  if(ip == undefined)
      return undefined;

  if(exports.list[ip] != undefined){
    if(exports.list[ip].expire > sh.time()){
      if(exports.list[ip].updates == undefined || exports.list[ip].updates < 3){
        // Make sure that we UPDATE information in here. We have people constantly reloading every 10 seconds fucking annoying.
        exports.list[ip].identity                   = identity;
        exports.list[ip].uid                        = uid;
        exports.list[ip].expire_seconds             = exports.expire_time;
        exports.list[ip].expire                     = sh.time() + exports.expire_time;
        exports.list[ip].objects                    = objects;
        exports.list[ip].loader_heart_beat_received = undefined;
        exports.list[ip].stub_heart_beat_received   = undefined;
        exports.list[ip].security_violation         = undefined;
        exports.list[ip].received_screenshot        = undefined;
        exports.list[ip].first_challenge            = undefined;
        exports.list[ip].disallow_cheat_request     = undefined;
        exports.list[ip].process_names              = undefined;
    
        if(exports.list[ip].updates == undefined)
          exports.list[ip].updates = 1;
        else
          exports.list[ip].updates++;
      
        let key = exports.list[ip].key;
        sh.print(socket, `[+] @IP: Updating "${key}" for "${identity}" (USERID: ${uid})`);
        return exports.list[ip];
      }
    }
  }

  const license_key = exports.generate_key(32);

  // Delete any existing license if we have one
  delete exports.list[ip];

  exports.list[ip] = {
      identity: identity,
      uid: uid,
      key: license_key,
      expire_seconds: exports.expire_time,
      expire: sh.time() + exports.expire_time,
      objects: objects
  };

  sh.print(socket, `[+] @IP: Created license "${license_key}" for "${identity}" (USERID: ${uid})`);
  sh.print(socket, `[!] @IP: License expires in ${exports.expire_time} seconds`);

  return exports.list[ip];
}

exports.current_license = function(socket){
  if(socket == undefined)
      return undefined;

  const ip = sh.ip(socket);

  if(ip == undefined)
      return undefined;

  if(!exports.valid(exports.list[ip])){
      delete exports.list[ip];
      return undefined;
  }

  return exports.list[ip];
}

exports.valid = function(license){
  if(license == undefined)
    return true;

  return license.expire > sh.time() && license.objects != undefined && license.objects.length > 0;
}

exports.clean_up = function(){
  for(var k in exports.list){
    const entry = exports.list[k];

    if(!exports.valid(entry)){
      sh.print(undefined, `[+] @IP: Deleted expired license "${entry.key}" for "${entry.identity}"`);
      delete exports.list[k];
    }
  }
}

setInterval(function(){
  exports.clean_up();
}, sh.housekeeping_ms);