const http = require("http");
const fs   = require("fs");


const config = JSON.parse(fs.readFileSync("./config.json"));
if(fs.existsSync("C:/"))
  config.server_ip = "127.0.0.1";

function make_request(res, data){
  if(res == undefined || data == undefined)
    return;

  res.statusCode = 200;
  res.setHeader("Content-Type", "application/json");
  res.end(JSON.stringify(data, null, 2));
}

async function send_discord_webhook(url, content){
  // Try catch cause discord is pasted sometimes.
  try{
    if(url == undefined || content == undefined || content == null || content.length == undefined){
      console.log("[-] send_discord_webhook bad data");
      return false;
    }

    // Setup the embed json.
    var json = {
      "username": "Notifications",
      "avatar_url": "https://rijin.solutions/blue_icon.png",
      "content": content
    };

    // Send off the request! We don't really care about the out come.
    var response = await fetch(url, {
      method: "POST",
      headers: {
          "Content-Type": "application/json",
      },
      body: JSON.stringify(json),
    });

    if(response == undefined)
      return false;

    var dat = await response.text();
    return true;
  }
  catch(err){

    console.log("[-] send_discord_webhook error sending message");
    console.log(err);
  }

  return false;
}

const server = http.createServer(async (req, res) =>{
  if(req == undefined || req == null)
    return;

  try{
    let ip_addr = req.socket.remoteAddress;
    if(req.headers['cf-connecting-ip'] != undefined)
      ip_addr = req.headers['cf-connecting-ip'];

    if(!config.allowed_ips.includes(ip_addr)){
      console.log(`Blocked ${ip_addr} because it's not allowed.`);
      make_request(res, {ok: false});
      return;
    }

    let secret = req.headers["xf-webhook-secret"];
    if(secret == undefined || secret == null || secret != config.secret){
      console.log(`Blocked ${ip_addr} because invalid secret ${secret}`);
      make_request(res, {ok: false});
      return;
    }

    console.log(`Processing request: ${ip_addr}`);
  }
  catch(err){
    make_request(res, {ok: false});
  }


  let data = "";
  req.on("data", (text) => data += text);
  req.on("end", async () => {
    try{
      let json = JSON.parse(data);
      if(  json == undefined
        || json.content_type == undefined
        || json.content_type == null
        || json.event == undefined
        || json.event == null){
        make_request(res, {ok: false});
        return;
      }

      let type  = json.content_type;
      let event = json.event;

      let ignoring = false;
      for(const entry of config.events){
        if(entry.type == type && entry.event == event){
          ignoring = true;
          break;
        }
      }

      if(json.data == undefined){
        make_request(res, {ok: false});
        return;
      }

      if(ignoring){
        console.log(`${type}: ${event} ignored.`);
        make_request(res, {ok: true});
        return;
      }

      // Handle requests here.
      let content = "";
      if(type == "post"){
        if(json.data.is_first_post)
          content += "New post: " + json.data.Thread.view_url + "\n";
        else{
          content += "New reply: " + json.data.view_url + "\n";
        }
      }
      else if(type == "nf_tickets_message"){
        let data = json.data;
        let ticket = data.Ticket;
        content += `Ticket by ${data.username}\n${ticket.view_url}\nMessage: **${data.message.replaceAll("`", "").replaceAll("*", "")}**\n\n\n`;
        if(ticket.custom_fields != undefined){
          try{
            for(var i in ticket.custom_fields){
              content += `${i.replaceAll("_", " ")}: ${ticket.custom_fields[i].replaceAll("_", " ")}\n`;
            }
          }
          catch{}
        }
      }
      else if(type == "report"){
        let data = json.data;
        if(data.report_state == "resolved")
          return;

        content += `@everyone\nContent abuse report\n${data.view_url}\n`;
      }
      else if(type == "profile_post_comment"){
        if(event != "insert")
          return;

        let data = json.data;
        content += `Profile post on ${data.username}\n${data.view_url}\n`;
      }
      else if(type == "thread"){
        if(event != "insert")
          return;

        let data = json.data;
        content += `New thread by ${data.username} - ${data.title}\n${data.view_url}\n`;
      }
      else if(type == "user"){
        if(event != "insert")
          return;

        let data = json.data;
        content += `New user ${data.username}\n${data.view_url}\n`;
      }

      //
      if(content.length > 0){
        for(const entry of config.types){
          if(entry.type == type && entry.event == event){
            console.log(`Sending webhook of desc "${entry.desc}"`);
            send_discord_webhook(entry.discord_wh, content);
          }
        }
      }
      else{
        console.log(`No handle for ${type} - ${event}`);
      }

      fs.writeFileSync("./" + json.content_type + ".json", JSON.stringify(json, null, 2));
      make_request(res, {ok: true});
    }
    catch(err){
      console.log(`Error processing request: ${err}`);
    }
  });
});

server.listen(config.server_port, config.server_ip, () =>{
  console.log("Running xenforo webhooks");
});
