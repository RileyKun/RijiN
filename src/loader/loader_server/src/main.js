// core
const net = require("http");
const colors = require('colors');

// custom
const sh = require("./shared.js");
const router = require("./router.js");
const cloud_config = require("./cloud_config.js");
const sync_sm = require("./synchronization.js");
const tf_lobby = require("./tf_lobby.js");
const stats    = require("./stats.js");

// print out args and if we are in debug mode 
{
  for (let i = 2; i < process.argv.length; i++){
    console.log(`[!] arg: ${process.argv[i]}`);
  }

  if(sh.dev_mode)
    sh.print(undefined, "[!] WARNING! You are running the server in developer mode.", true);

  if(sh.staging_mode)
    sh.print(undefined, "[!] WARNING! You are running the server in staging mode.", true);

  if(sh.test_shipping)
    sh.print(undefined, "[!] WARNING! You are running the server in test shipping mode.", true);
} 

// Fetch cidr list before starting server
sh.update_cloudflare_cidr_list();

// Load sessions from cache file, if any
cloud_config.load_sessions_from_file();

// Create a timer that saves our active sessions to a cache file
setInterval(cloud_config.save_sessions_to_file, cloud_config.license_cache_save_time * 1000);


const rate_limiter = new Map();
setInterval(() => {
  rate_limiter.clear();
}, 60000);

net.createServer(function(req, res){
  
  if(sh.cloudflare_only && !sh.is_cloudflare_connection(req)){
    req.connection.destroy();
    return;
  }

  let ip_addr   = sh.ip(req);
  let req_count = rate_limiter.get(ip_addr);
  if(req_count)
    req_count.count++;
  else{
    req_count = {count: 1};
    rate_limiter.set(ip_addr, req_count);
  }

  if(req_count.count > 1000){
    sh.general_log("ddos", `${$ip_addr} is being blocked for flooding: ${req_count.count}`);
    req.connection.destroy();
    return;
  }

  let buffer = Buffer.alloc(0);
  req.on("data", function(data){
    
    try{

      if (buffer.length + data.length > sh.max_input_buffer){
        sh.general_log("on_request", `${ip_addr}, data size: ${buffer.length + data.length} is too large ${sh.max_input_buffer}`);
        sh.print(req, `[-] MAIN: @IP: connection closed due to buffer too large (${buffer.length + data.length})`, true);
        delete buffer;
        req.connection.destroy();
      }
      else{
        const old_buffer = buffer;
        buffer = Buffer.concat([buffer, data], buffer.length + data.length);
        delete old_buffer;
      }
          
      if(sh.net_fragment_is_valid(buffer)){
        const final_len_pred = sh.net_fragment_len(buffer) + sh.structs.net_fragment.sizeof;
        
        if(buffer.length > final_len_pred){
          sh.general_log("on_request", `${ip_addr}, data size: ${buffer.length + data.length} length mismatch ${final_len_pred}`);
          sh.print(req, `[-] MAIN: @IP: net_fragment len mismatch! (${colors.brightGreen(buffer.length)} > ${colors.brightGreen(final_len_pred)})`, true);
          delete buffer;
          req.connection.destroy();
        }
        else if(buffer.length == final_len_pred){
          const cmd = sh.net_fragment_cmd(buffer);
          const len = sh.net_fragment_len(buffer);
          
          //sh.print(req, `[+] @IP: net_fragment received`);
          //sh.print(req, `[!] @IP: net_fragment->cmd: ${cmd.toString(16)}`);
          //sh.print(req, `[!] @IP: net_fragment->len: ${len}`);
          
          let fragment_response = sh.net_fragment_create(sh.NET_FRAGMENT_ERROR, undefined);
          const router_response = router.handle_net_fragment(req, buffer);

          delete buffer;
  
          if(router_response != undefined){
            delete fragment_response;
            fragment_response = router_response;
          }
          
          res.setHeader("Content-Type", "application/octet-stream");
          res.setHeader("Cache-Control", "no-store, no-cache, must-revalidate, proxy-revalidate");
          res.setHeader("Pragma", "no-cache");
          res.setHeader("Expires", "0");
        
          if(fragment_response != undefined){
            const content_len = Buffer.byteLength(fragment_response);
            
            if(content_len > 0)
              res.setHeader("Content-Length", content_len);
          }
          
          res.writeHead(200);
          res.write(fragment_response);
          res.end();

          delete fragment_response;
        }
      }
      else{
        sh.general_log("on_request", `${ip_addr} received invalid net_fragment`);
        sh.print(req, `[-] @IP: net_fragment is invalid!`, true);
        delete buffer;
        req.connection.destroy();
      }
    }
    catch(e){
      delete buffer;

      sh.general_log("exceptions", `${e}`);
      console.log("\n\n[CRITICAL ERROR\n\n");
      sh.print(req, `[-] MAIN: @IP Exception: ${e.message}`, true);
      sh.print(req, `[-] MAIN STACK TRACE: @IP Stack: ${e.stack}`, true);
      stats.send_backend_issue_alert(e);

      const fragment_response = sh.net_fragment_error_msg(req, "Connection refused, please try again.", true);

      if(fragment_response != undefined){
        const content_len = Buffer.byteLength(fragment_response);
        
        if(content_len > 0)
          res.setHeader("Content-Length", content_len);
      }
      
      res.writeHead(403);
      res.write(fragment_response);
      res.end();
    }
  });
  
  req.on("error", function(str){
    sh.print(req, `[-] NETWORK ERROR: ${str}`, true);
    sh.general_log("on_request", `${ip_addr} network error: ${err}`);
    delete buffer;
  });
  
  req.on("end", function(){
    delete buffer;

    //sh.gc();
  });
    
}).listen(sh.listen_port, sh.listen_addr);

sync_sm.start_listening();
sh.print(undefined, `[+] loader started on port ${sh.listen_port}`);
