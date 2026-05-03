

exports.internal_tnd = function(code){
  if(code == null || code == undefined || code.length != undefined)
    code = 0;

  var str = "Internal error TND (" + code.toString() + ")\n\n";
  str += "1. Please disable any active anti-viruses.\n"
  str += "2. Please disable TESTSIGNING\n4. Reboot computer.";
  return str;
}

exports.invalid_request = function(code){
  if(code == null || code == undefined || code.length != undefined)
    code = 0;

  return "Invalid request (" + code.toString() + ")";
}

exports.license_invalid = function(code){
  if(code == null || code == undefined || code.length != undefined)
    code = 0;

  return "License is invalid (" + code.toString() + ")";
}

exports.license_expired = function(code){
  if(code == null || code == undefined || code.length != undefined)
    code = 0;

  return "License has expired (" + code.toString() + ")";
}

exports.no_active_subs = function(code){
  if(code == null || code == undefined || code.length != undefined)
    code = 0;

  return "No active subscriptions (" + code.toString() + ")";
}

exports.no_license_found = function(key_provided){
  if(key_provided == undefined)
    key_provided = "No key was provided";

  return "No license could be found. Key: (" + key_provided + ")";
}