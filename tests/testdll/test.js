const fs = require("fs");

function pe_valid(buffer){
  return buffer != undefined && buffer.length >= 0x400 && buffer.readInt16LE(0x0) == 0x5A4D;
}

function pe_lfanew(buffer){
  if(!pe_valid(buffer))
    return 0;

  return buffer.readUInt32LE(0x3C);
}

function pe_number_of_sections(buffer){
  if(!pe_valid(buffer))
    return 0;

  const lfanew = pe_lfanew(buffer);

  if(lfanew == 0)
    return 0;

  return buffer.readUInt16LE(lfanew + 0x6);
}

function pe_x86(buffer){
  if(!pe_valid(buffer))
    return 0;

  const lfanew = pe_lfanew(buffer);

  if(lfanew == 0)
    return 0;

  // 0x8664 = 64
  // 0x014C = 86
  return buffer.readUInt16LE(lfanew + 0x4) == 0x014C;
}

function strlen(buffer, ptr, max_len){
  if(max_len == undefined)
    max_len = 128;

  let len = 0;
  while(buffer.readUInt8(ptr + (++len)) != 0 && len <= max_len)
    continue;

  return len;
}

function memset(buffer, ptr, val, len){
  while(len-- > 0)
    buffer.writeUInt8(val, ptr + len);
}

function memcpy(from_buffer, to_buffer, to_buffer_offset, from_buffer_offset, len){
  return from_buffer.copy(to_buffer, to_buffer_offset, from_buffer_offset, from_buffer_offset + len);
}

function align_up(val, align){
  return (-(-(val) & -(align)));
}

function test1(){
  let buffer = fs.readFileSync("./testdll.dll");
  console.log(buffer);

  if(!pe_valid(buffer))
    return undefined;

  const lfanew = pe_lfanew(buffer);

  if(lfanew == 0)
    return undefined;

  const number_of_sections = pe_number_of_sections(buffer);

  if(number_of_sections == 0)
    return undefined;

  const size_of_optional_header = buffer.readUInt16LE(lfanew + 0x14);

  const new_section_ptr = (lfanew + (0x4 + 0x14) + size_of_optional_header) + (number_of_sections * 0x28);

  memset(buffer, new_section_ptr, 0, 0x28);

  const new_section_name = Buffer.from(".test");
  memcpy(new_section_name, buffer, new_section_ptr, 0, Math.min(new_section_name.length, 8));

  buffer.writeUInt16LE(number_of_sections + 1, lfanew + 0x6);

  //40
  //for(let i = 0; i < number_of_sections; i++){
  //  const section = (lfanew + (0x4 + 0x14) + size_of_optional_header) + (i * 0x28);
//
  //  const va                  = buffer.readUInt32LE(section + 0xC);
  //  const va_size             = buffer.readUInt32LE(section + 0x8);
  //  const pointer_to_raw_data = buffer.readUInt32LE(section + 0x14);
  //  const sizeof_raw_data     = buffer.readUInt32LE(section + 0x10);
  //  const characteristics     = buffer.readUInt32LE(section + 0x24);
//
  //  // https://i.imgur.com/pIy94bN.png
  //  const name_len = strlen(buffer, section, 8);
  //  const name = buffer.toString("utf8", section, section + name_len);
//
  //  console.log(name);
  //  //const exec  = ((characteristics & IMAGE_SCN_MEM_EXECUTE) != 0) ? 1 : 0;
  //  //const read  = ((characteristics & IMAGE_SCN_MEM_READ) != 0) ? 1 : 0;
  //  //const write = ((characteristics & IMAGE_SCN_MEM_WRITE) != 0) ? 1 : 0;
//
  //  //print(`Found section "${name}" ${va.toString(16)} \t${PAGE_PROTECTION[exec][read][write][1]}`);
  //}

  fs.writeFileSync("new.dll", buffer);
}

test1();