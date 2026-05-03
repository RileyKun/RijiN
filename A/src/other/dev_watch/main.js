const simpleGit = require(`simple-git`);
const git       = simpleGit();


console.log(`Watching for commits`);

var last_commit = 0;

// Same for the names.
function translate_name(author){
  author = author.toLowerCase();
  if(author !== "senator" && !author.includes("styles"))
    return "rud";

  return author;
}

// That way certain commits can be censored and thus not leak anything important!
function correct_msg(msg){
  msg = msg.toLowerCase();
  if(msg.includes("rtp") 
    || msg.includes("security") 
    || msg.includes("securtiy")
    || msg.includes("protect")
    || msg.includes(".js")
    || msg.includes("bypass") 
    || msg.includes("loader") 
    || msg.includes("server") 
    || msg.includes("exploit")
    || msg.includes("generator") 
    || msg.includes("gen.js") 
    || msg.includes("iced-x86")
    || msg.includes("exception")
    || msg.includes("auto ban")
    || msg.includes("cache control")
    || msg.includes("ip address")
    || msg.includes("steamid")
    || msg.includes("backend")
    || msg.includes("steam id")
    || msg.includes("cheater list")
    || msg.includes("cheaterlist")
    || msg.includes("stack check")
    || msg.includes("their forum")
    || msg.includes("timeout")
    || msg.includes("imgui")
    || msg.includes("d3d9")
    || msg.includes("renderer")
    || msg.includes("timing checks")
    || msg.includes("stub")
    || msg.includes("dxgi")
    || msg.includes("xorstr")
    || msg.includes("xorptr")
    || msg.includes("kernel")
    || msg.includes("hyper visor")
    || msg.includes("hypervisor")
    || msg.includes("hyper visor")
    || msg.includes("ip_"))
    return "(Backend changes)";

  if(msg.includes("ph4ge") || msg.includes("phage") || msg.includes("pasted") || msg.includes("paste") || msg.includes("compiler error"))
    return "(Fixed an issue in a pushed commit)";

  if(msg.includes("merge branch"))
    return "(Branch merging)";

  if(msg.includes("crack") || msg.includes("debug") || msg.includes("detect") || msg.includes("staging") || msg.includes("development") || msg.includes("shipping"))
    return "(Redacted)";

  if(msg.includes(".cpp") || msg.includes(".h"))
    return "(Change pushed with no information)";

  return msg;
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
      "username": "RijiN",
      "avatar_url": "https://rijin.solutions/red_icon.png",
      "embeds": [
        {
          "footer": {
            "text": "Devlog",
            "icon_url": "https://rijin.solutions/red_icon.png"
          },
          "author":{
            "name": "RijiN",
            "icon_url": "https://rijin.solutions/red_icon.png"
          },
          "title": "Devlog",
          "description": content,
          "color": 11393254
        }
      ]
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

function check_commits(){
  git.log({ maxCount: 1 })
  .then(log => {
 
    log.all.forEach(commit => {
      if(last_commit != 0){
        if(last_commit != commit.hash){
          const fixed_msg = correct_msg(commit.message);
          var msg       = "``[" + commit.date + "]`` by **" + translate_name(commit.author_name) + "**\n```" +  fixed_msg + "```\n";
          send_discord_webhook("https://discord.com/api/webhooks/1340440762957168640/aNX0MrXwB-IeVW8zmONNctv7NANUmmatdve79fEodMcpBGRxcLriodN9qdUqKutuDurb", msg);
        }
      }
      last_commit = commit.hash;
    });
  })
  .catch(err => console.error('Error fetching logs:', err));
}

check_commits();
setInterval(check_commits, 5000);