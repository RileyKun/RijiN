const sh = require("./shared.js");


exports.rlm_steamid_cache     = {};

exports.decay_cache = function(){
  console.log("[+] Clearing old cached entries");
  for(var e in exports.rlm_steamid_cache){
    var entry = exports.rlm_steamid_cache[e];
    if(entry == undefined)
      continue;

    if(entry.expire > sh.time())
      continue;

    delete exports.rlm_steamid_cache[e];
  }

  for(var e in exports.rlm_proxy_check_cache){
    var entry = exports.rlm_proxy_check_cache[e];
    if(entry == undefined)
      continue;

    if(entry.expire > sh.time())
      continue;

    delete exports.rlm_proxy_check_cache[e];
  }
}

setInterval(exports.decay_cache, 60000 * 5);