// Run compile  - senator
// Resolves and executes the compile.bat file by going backwards in the current working directory to find the compile file
// This is used because in sublime text if you put your compile.bat in say loader/
// If you try to build when in loader/test/test.c It will attempt to run loader/test/compile.bat
// This script will resolve the compile.bat file fully without any manual adjustments

const fs                = require("fs");
const exec              = require('child_process').exec;
const resolve_relative  = require('path').resolve;

function clean(str){
  let tmp = ["gen_read_array", "gen_read_offset", "gen_read_pointer"];
  for(const entry of tmp)
    str = str.replaceAll(entry, "gen_read");

  str = str.replaceAll("gen_read_pointer_other", "gen_read_other");

  return str;
}

function fetch_inline_info(buffer){
  if(buffer == undefined)
    return undefined;

  let list = [];

  // Find all create_offset_hash
  {
    let pattern = "create_offset_hash(";
    let i = buffer.indexOf(pattern);
    while(i != -1){

      let str = "";
      for(let j = i + pattern.length; j < buffer.length; j++){
        if(buffer[j] == ')')
          break;

        str += buffer[j].toString();
      }

      str = str.replaceAll("\"", "").replaceAll("(", "");
      str = str.trim();

      //console.log(str);
      list.push(str);

      i = buffer.indexOf(pattern, i + 1);
    }
  }

  // Find all gen_read
  {
    let pattern = "gen_read(";
    let i = buffer.indexOf(pattern);
    while(i != -1){

      let str = "";
      let s_log = false;
      for(let j = i + pattern.length; j < buffer.length; j++){
        if(buffer[j] == ',' && !s_log){
          s_log = true;
          continue;
        }

        if((buffer[j] == ')' || buffer[j] == ',') && s_log)
          break;

        if(!s_log)
          continue;

        str += buffer[j].toString();
      }

      str = str.replaceAll("\"", "").replaceAll("(", "");
      str = str.trim();

      //console.log(str);
      list.push(str);

      i = buffer.indexOf(pattern, i + 1);
    }
  }

  // Find all gen_read_other
  {
    let pattern = "gen_read_other(";
    let i = buffer.indexOf(pattern);
    while(i != -1){

      let str = "";
      let s_log = 0;
      for(let j = i + pattern.length; j < buffer.length; j++){
        if(buffer[j] == ',' && s_log <= 1){
          s_log++;
          continue;
        }

        if((buffer[j] == ')' || buffer[j] == ',') && s_log >= 2)
          break;

        if(s_log < 2)
          continue;

        str += buffer[j].toString();
      }

      str = str.replaceAll("\"", "").replaceAll("(", "");
      str = str.trim();

      list.push(str);

      i = buffer.indexOf(pattern, i + 1);
    }
  }

  return list;
}

function find_excluded_offsets(product_name){
  let list = [];
  if(product_name != "tf2_win64"){
    list.push("DT_PlayerResource_m_iPlayerClass");
    list.push("DT_PlayerResource_m_iDamage");
    list.push("DT_PlayerResource_m_iMaxBuffedHealth");
  }
  else{
    list.push("DT_BasePlayer_m_hGroundEntity");
    list.push("DT_BasePlayer_m_nWaterLevel");
    list.push("DT_SPlayer_m_angEyeAngles0");
    list.push("DT_BaseCombatWeapon_m_iClip1");
  }

  if(product_name != "l4d2")
    list.push("DT_PlayerResource_m_maxHealth");
  else{
    // These have been handled in l4d2.
    list.push("DT_LocalPlayerExclusive_m_hGroundEntity");
    list.push("DT_LocalPlayerExclusive_m_nWaterLevel");
    list.push("DT_LocalPlayerExclusive_m_angEyeAngles0");
    list.push("DT_PlayerResource_m_iMaxHealth");
    list.push("DT_PlayerResource_m_iAccountID");
    list.push("DT_LocalWeaponData_m_iClip1");
  }

  return list;
}

function print(str){
  console.log(`\nNOTICE\n---\n---\n---\n---\n[!] ${str}\n---\n---\n---\n---`);
}

// We got inline data beinng written in some cheats, so we wanna try and detect mismatch offsets.
function on_base_directory(dir){
  let cmk = `${dir}\\CMakeLists.txt`;
  if(!fs.existsSync(dir) || !fs.existsSync(cmk)){
    console.log("Not a C product");
    return;
  }
  let cmk_content = fs.readFileSync(cmk).toString();

  if(!cmk_content.includes("set(BUILD_FLAG_SOURCE_ENGINE ON)"))
    return;

  let codebase_main = process.env.CODEBASE_MAIN;
  let loader_dir    = `${codebase_main}\\src\\loader\\loader_server\\inlined_data`;
  let se_base       = `${codebase_main}\\src\\base\\cheats\\source\\`
  let list          = ["features", "interfaces", "managers", "structs"];

  let product_name = dir.replaceAll(`${codebase_main}\\src\\cheats\\`, "").replaceAll("left4dead2", "l4d2");
  let inline_data_files = [`${loader_dir}\\base.json`, `${loader_dir}\\${product_name}_netvar.json`, `${loader_dir}\\${product_name}_base.json`]

  for(const dir of inline_data_files){
    if(!fs.existsSync(dir)){
      print(`${dir} does not exist and cheat will crash upon load. ${product_name}`);
      return;
    }
  }

  for(const dir of list){
    fs.readdir(`${se_base}${dir}\\`, function(err, files){
      if(err)
        return;

      for(const entry of files){
        let buffer = fs.readFileSync(`${se_base}${dir}\\${entry}`).toString();
        buffer = clean(buffer);

        let data = fetch_inline_info(buffer);
        if(data != undefined && data.length > 0){
          for(const offset of data){
            let found = false;
            for(const f of inline_data_files){
              try{
                let tmp = JSON.parse(fs.readFileSync(f).toString());
                if(tmp == undefined)
                  continue;

                for(const i in tmp){
                  const dat_entry = tmp[i];
                  if(dat_entry == undefined)
                    continue;

                  if(offset == dat_entry.name){
                    found = true;
                    break;
                  }
                }
                if(found)
                  break;
              }
              catch(err){
                print(`EXCEPTION! - ${err} - ${f} - ${product_name}`);
              }
            }

            if(!found){
              let excluded_list = find_excluded_offsets(product_name);
              if(!excluded_list.includes(offset))
                print(`WARNING: Unable to find ${offset} in ${product_name}!`);
            }

          }
        }
      }
    });
  }
}

let cwd = process.cwd();

for(let i = 0; i < 50; i++){
  const target_file = `${cwd}\\compile.bat`;

  if(fs.existsSync(target_file)){
    on_base_directory(cwd);
    console.error(`[+] run_compile.js: found compile.bat (${target_file})`);

    var cmd = exec(`cd ${cwd} && call ${target_file}`);

    // Show live output from stdout
    cmd.stdout.on('data', function(data) {
      process.stdout.write(data);
    });

    cmd.stderr.on('data', function(data) {
      process.stdout.write(data);
    });

    return;
  }

  cwd = resolve_relative(`${cwd}\\..`); // go back one directory
}

console.error("[-] run_compile.js: couldn't find compile.bat");