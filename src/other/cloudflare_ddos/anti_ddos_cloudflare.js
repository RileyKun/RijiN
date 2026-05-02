const fs    = require("fs");
const http  = require("http");

var config_buffer = undefined;
var api_buffer    = undefined;
var dev_mode      = false;

// Load config file.
if(fs.existsSync("C:/")){
  dev_mode      = true;
  config_buffer = fs.readFileSync("./config.json");
}
else
  config_buffer = fs.readFileSync("/home/cloudflare_ddos/config.json");

// Don't allow the .privatekey file to be published to github.
if(fs.existsSync("C:/"))
  api_buffer = fs.readFileSync("./api.privatekey");
else
  api_buffer = fs.readFileSync("/home/cloudflare_ddos/api.privatekey");

var config = JSON.parse(config_buffer);
delete config_buffer;

var api_json = JSON.parse(api_buffer);
delete api_buffer;

if(fs.existsSync("C:\\"))
  config.webhook_ip = "127.0.0.1";

config.api_token       = api_json.api_token;
config.email           = api_json.email;
config.discord_webhook = api_json.discord_webhook;

// Global variables here.
var   under_attack_mode_ticks      = 0;
var   current_security_level       = undefined;
var   firewall_list                = undefined; // List of firewall rules gotten from cloudflare.
var   ddos_protection_active_ticks = 0; // In seconds, how long the ddos protection will remind enabled before being turned off.
var   ddos_connection_problems     = 0; // Used to trip ddos protection.
var   ddos_protection_status       = false; // If true, we enabled ddos protection.
var   webhook_url                  = config.discord_webhook;
const rate                         = 3600;
const max_rate                     = rate * 3;

// Discord webhook
var last_ping_message = undefined;
async function send_webhook(url, content, ping = false){
  if(url == undefined || content == undefined || content == null || content.length == undefined)
    return false;

  var raw_content = "";
  if(ping != undefined && ping){
    raw_content = "@everyone";

    if(last_ping_message != undefined){
      if(last_ping_message == content)
        return;

      last_ping_message = content;
    }
  }

  // Setup the embed json.
  var json = {
    "username": config.name + " Anti-DDoS",
    "content": raw_content,
    "embeds": [
      {
        "title": config.name + " website protection",
        "description": content,
        "color": 11393254
      }
    ]
  }; 

  try{
    // Send off the request! We don't really care about the out come.
    fetch(url, {
      method: "POST",
      headers: {
          "Content-Type": "application/json",
      },
      body: JSON.stringify(json),
    });
  }
  catch(error){
    console.log("send_webhook error: " + error);
  }

  return true;
}

// Rather than have to hard-code literally everything. Lets just do this.
async function get_firewall_data(){
  console.log("[+] get_firewall_data");
  var result = await fetch("https://api.cloudflare.com/client/v4/zones/" + config.zone_id + "/firewall/rules",{
    method: "GET",
    headers:{
      "Content-Type": "application/json",
      "X-Auth-Email": config.email,
      "X-Auth-Key": config.api_token,
    },
  });

  var json      = await result.json();
  if(json.result == undefined){
    console.log("[-] Failed to fetch WAF rules from cloudflare");
    return;
  }

  firewall_list = json.result;
}

async function run(){
  await get_firewall_data();

  current_security_level = get_security_level();
  if(firewall_list == undefined)
    return;

  // Avoid having the protection turned off and then turned back on.
  var is_protection_active = false;
  for(var i in firewall_list){
     for(var j in config.active_during_ddos){
        if(config.active_during_ddos[j] == firewall_list[i].id){
          if(!firewall_list[i].paused){
            is_protection_active = true;
            break;
          }
        }
     }
  }

  // This could be an error or not, either way. If an attack is still happening after 5 minutes this will get re-enabled for an hour or more.
  if(is_protection_active){
    console.log("[!] One of the active ddos rules is keep ddos attack protection for 5 minutes");
    ddos_protection_active_ticks = 300; 
  }

  for(var i in firewall_list)
    console.log(firewall_list[i].description + ": " + firewall_list[i].id);
  
  // Disable ddos attack rules.
  //for(var i in config.active_during_ddos)
  //  await state_pause_state_by_id(config.active_during_ddos[i], true);
}

async function get_rule_json_by_id(rule){
  if(firewall_list == undefined){
    console.log("[-] firewall_list is undefined (get_rule_json_by_id)");
    return undefined;
  }

  if(rule == undefined){
    console.log("[-] get_rule_json_by_id paramater 'rule' is undefined");
    return undefined;
  }

  if(firewall_list.length == 0){
    console.log("[-] get_rule_json_by_id firewall_list has no entries");
    return undefined;
  }

  for(var j in firewall_list){
    var id = firewall_list[j].id;
    if(id == undefined)
      continue;

    if(id == rule)
      return firewall_list[j];
  } 

  return undefined;
}

async function state_pause_state_by_id(rule, paused){
  console.log(`[+] state_pause_state_by_id: ${rule} - ${paused}`);
  var firewall_rule_json = await get_rule_json_by_id(rule);
  if(firewall_rule_json == undefined){
    console.log("[-] firewall_rule_json is undefined (state_pause_state_by_id)");
    return false;
  }
  
  firewall_rule_json.paused        = paused;
  firewall_rule_json.filter.paused = paused;

  try{
    console.log(`[+] WAF rule ${firewall_rule_json.description} paused state set to ${paused}`);
    var result = await fetch("https://api.cloudflare.com/client/v4/zones/" + config.zone_id + "/firewall/rules/" + rule,
    {
      method: "PUT",
      headers:{
        "Content-Type": "application/json",
        "X-Auth-Email": config.email,
        "X-Auth-Key": config.api_token,
      },
      body: JSON.stringify(firewall_rule_json),
    });

     console.log(`[+] WAF rule ${firewall_rule_json.description} api response ${result.status}`);
    return result.status == 200;
  }
  catch(err){
    console.log(`[-] state_pause_state_by_id error: ${err} ${rule}`);
    return false;
  }

  return false;
}

async function get_security_level(){

  try{
    console.log("[+] get_security_level");
    var result = await fetch("https://api.cloudflare.com/client/v4/zones/" + config.zone_id + "/settings/security_level", 
    {
      method: "GET",
      headers:{
        "Content-Type": "application/json",
        "X-Auth-Email": config.email,
        "X-Auth-Key": config.api_token,
      },
    });

    var json = await result.json();
    if(json == undefined)
      return undefined;

    if(json.success == undefined || !json.success)
      return undefined;

    if(json.result == undefined)
      return undefined;

    console.log(`[!] get_security_level success: ${json.result}`);
    return json.result;
  }
  catch(err){
    console.log(`[-] get_security_level error: ${err}`);
    return undefined;
  }

  return undefined;
}

async function set_security_level(level){
  if(level == undefined || level == null)
    return false;

  try{

    console.log(`[+] set_security_level: ${level}`);
    var result = await fetch("https://api.cloudflare.com/client/v4/zones/" + config.zone_id + "/settings/security_level", 
    {
      method: "PATCH",
      headers:{
        "Content-Type": "application/json",
        "X-Auth-Email": config.email,
        "X-Auth-Key": config.api_token,
      },
      body: JSON.stringify({value: level}),
    });

    var json = await result.json();
    if(json == undefined)
      return false;

    console.log(`[!] set_security_level -> ${level} success`);
    return true;
  }
  catch(err){
    console.log(`[-] set_security_level error: ${err}, details: ${level}`);
  }
  return false;
}

if(!dev_mode)
  send_webhook(webhook_url, "Service started");

run();

// 'Timer' functions
async function check_website(){
  if(firewall_list == undefined || firewall_list.length == 0)
    return;

  if(current_security_level == undefined)
    current_security_level = get_security_level();

  var last_status_code = -1;
  console.log("Checking website: " + config.website);
  try{
    var result = await fetch(config.website,
    {
      method: "GET",
      headers:{
        "Content-Type": "text/html",
        "User-Agent": config.health_check_ua,
      }
    });

    console.log(`[!] Checked website: ${result.status}, is_aborted: ${result.aborted}`);

    last_status_code = result.status;
    if(!result.aborted && result.status == 200){

      if(under_attack_mode_ticks > 0)
        under_attack_mode_ticks--;

      if(ddos_connection_problems > 0){
        ddos_connection_problems--;
        if(ddos_connection_problems <= 0){
          if(current_security_level != undefined && current_security_level.value !== "essentially_off" && under_attack_mode_ticks <= 0){
            console.log("Turning off security_level\n");
            set_security_level("essentially_off");
            current_security_level = get_security_level();
          }
        }
      }
    }
    else{
      ddos_connection_problems++;
    }
  }
  catch(err){
    console.log(`[-] Error checking website: ${err}`);
    ddos_connection_problems++;
  }

  if(ddos_connection_problems > 0 && last_status_code != -1 && (last_status_code == 502 || last_status_code == 503 || last_status_code == 500)){
    under_attack_mode_ticks = 500;
    ddos_protection_active_ticks = rate;
    if(current_security_level != undefined && current_security_level.value != "under_attack"){
      set_security_level("under_attack");
      current_security_level = get_security_level();
      send_webhook(webhook_url, config.name + " received an error ``" + last_status_code + "`` could be caused by ddos, high traffic, or nginx errors (Increased security_level)", true);
    }
    else{
      send_webhook(webhook_url, config.name + " received an error ``" + last_status_code + "`` (No action taken)", true);
    }
  }

  if(ddos_connection_problems >= 4){
    if(ddos_connection_problems == 1)
      send_webhook(webhook_url, "" + config.name + " website is having connection issues. Assuming an attack is occuring, turning on DDoS protection.", ddos_connection_problems >= 4 && ddos_connection_problems <= 5);
    
    console.log("[+] Too many connection problems setting active protection ticks");
    ddos_protection_active_ticks = rate;
    if(ddos_protection_active_ticks > max_rate)
      ddos_protection_active_ticks = max_rate;
  }

  console.log("[!] Checked website");
  console.log("[!] Anti-DDoS state: " + ddos_protection_status);
  console.log("[!] Active Ticks: " + ddos_protection_active_ticks);
  console.log("[!] Tracked connection issues: " + ddos_connection_problems.toString());
  console.log("\n\n\n");
}

async function ddos_protection_check(){
  if(ddos_protection_active_ticks <= 0 && !ddos_protection_status)
    return;

  console.log(`[+] ddos_protection_check: ${ddos_protection_active_ticks} ${ddos_protection_status}`);
  ddos_protection_active_ticks--;
  if(ddos_protection_active_ticks <= 0){
    if(ddos_protection_status){ 
      await get_firewall_data();

      console.log("[!] Disabling Anti-DDoS WAF rules");
      // DDoS protection is no longer needed.
      for(var i in config.active_during_ddos)
        await state_pause_state_by_id(config.active_during_ddos[i], true);

      ddos_protection_status = false;
      send_webhook(webhook_url, "Active ticks expired, disabling DDoS protection");
    }
  }
  else{
    if(!ddos_protection_status){
      await get_firewall_data();

      console.log("[!] Enabling Anti-DDoS WAF rules");
      ddos_protection_status = true;
      // Something is going on enable DDoS protection.
      for(var i in config.active_during_ddos)
        await state_pause_state_by_id(config.active_during_ddos[i], false);

      
      send_webhook(webhook_url, "Enabled DDoS Protection", true);
    }
  }

  if(ddos_protection_active_ticks < 0)
    ddos_protection_active_ticks = 0;
}

function status_report(){
  send_webhook("https://discord.com/api/webhooks/1183482600350818375/VC1g0-xu64m4pNmJEPUJLN00li5OWupnM0EZ6s2AM-Y6wIgqew0soW_mHtmNnjI9qNGc", "OK");
}

setInterval(check_website, 1000);
setInterval(ddos_protection_check, 1000);
setInterval(status_report, 60000 * 60);

const server = http.createServer(async (req, res) =>{
  var url = req.url;

  if(req.headers == undefined || url == undefined || url != "/events"){
    console.log("[-] cf webhook received an invalid request");
    close_connection(req);
    return;
  }

  if(req.method == undefined || req.method != "POST"){
    console.log("[-] cf webhook received a request with an invalid method");
    close_connection(req);
    return;
  }

  // TODO do a check for cloudflare ips here.

  var data = "";
  req.on("data", text => data += text);
  req.on("end", () => {
    if(data == undefined || data == null || data.length == 0){
      close_connection(req);
      return;
    }

    try{
      var json = JSON.parse(data);
      if(json == undefined){
        close_connection(req);
        return;
      }

      if(json.text == undefined || json.text == null || json.text.length == 0){
        console.log("[-] cf webhook did not have .text object");
        close_connection(req);
        return;
      }

      if(json.name == undefined || json.name == null || json.name.length == 0){
        console.log("[-] cf webhook did not have .text object");
        close_connection(req);
        return;
      }

      if(json.data == undefined){
        console.log("[-] cf webhook expected json.data (was undefined)");
        close_connection(req);
        return;
      }

      if(json.alert_type == undefined || json.alert_type == null || json.alert_type.length == 0){
        console.log("[-] cf webhook json.alert_type was undefined");
        close_connection(req);
        return;
      }

      if(json.data.action == undefined || json.data.action == null || json.data.action.length == 0){
        console.log("[-] cf webhook json.data.action was undefined");
        close_connection(req);
        return;
      }

      console.log(`[+] Cloudflare reporting ${json.alert_type} - (${json.data.action}`);
      // If the action is reported to be 'fake-action' this means it was a test.
      if(json.data.action != "fake-action"){
        if(json.alert_type == "dos_attack_l7"){
          ddos_protection_active_ticks += rate; // Turn on firewall for 1 hour.
          if(ddos_protection_active_ticks >= max_rate)
            ddos_protection_active_ticks = max_rate;

          send_webhook(webhook_url, "Cloudflare is reporting an attack on " + config.name, true);
        }
      }

    }
    catch{
      console.log("[-] An exception occured trying to read request");
      close_connection(req);
      return;
    }
  });

  res.statusCode = 200;
  res.setHeader("Content-Type", "application/json");
  res.end(JSON.stringify({"response": "ok"}));
});

function close_connection(req){
  if(req == undefined || req.connection == undefined)
    return;

  req.connection.destroy();
}

server.listen(config.webhook_port, config.webhook_ip, () =>{
  console.log(`Started server: ${config.webhook_ip}:${config.webhook_port}`)
});