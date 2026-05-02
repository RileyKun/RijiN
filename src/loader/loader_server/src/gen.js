const sh = require("./shared.js");
// core
const fs     = require("fs");
const colors = require("colors");

// custom
const c      = sh.c;
const hash_func = require("./fnv1a.js");

const {Decoder, Instruction, Code, OpKind} = require('iced-x86');


var debug_print_mode = false;

// define constants
const GEN_MAX_PKG = 512; // Maximum number of instruction packages
const GEN_MAX_BASE_ADDRESSES = 1000; // Maximum number of base addresses
const GEN_MAX_HOOK_SAMPLE_ASM86 = 16; // Maximum number of bytes that will be scanned in x86
const GEN_MAX_HOOK_SAMPLE_ASM64 = 112; // Maximum number of bytes that will be scanned in x64
const GEN_HOOK_JMP_SIZE86 = 5; // E9 00 00 00 00
const GEN_HOOK_JMP_SIZE64 = 14; // FF 25 00 00 00 00 00 00 00 00 00 00 00 00
const GEN_MAX_PAGES = 8; // Maximum number of pages that can be allocated to be mapped
const GEN_MAX_ERASES = 64; // Maximum number of function that can be declared for deletion after initialization
const GEN_MAX_HOOKS = 192; // Maximum number of hooks that can be placed

const GEN_PKG_TYPE_PTR    = hash_func.FNV1A("GEN_PKG_TYPE_PTR");
const GEN_PKG_TYPE_IMPORT = hash_func.FNV1A("GEN_PKG_TYPE_IMPORT");
const GEN_PKG_TYPE_HOOK   = hash_func.FNV1A("GEN_PKG_TYPE_HOOK");

const gen_pkg = c.struct({
  // main
  type:   c.u32,
  hash:   c.u32,
  hash1:  c.u32,
  ptr:    c.u64,

  short_hook:     c.bool,
  short_hook_ptr: c.u64,
  short_hook_asm: c.u8 * GEN_HOOK_JMP_SIZE86,
  short_hook_len: c.u8,

  // hooks
  original_asm: c.u8 * GEN_MAX_HOOK_SAMPLE_ASM64,
});

const gen_required_import = c.struct({
  module_hash: c.u32,
  name_hash: c.u32,
});

const gen_input = c.struct({
  // main
  base_address: c.u64,
  hook_trp_base_address: c.u64,
  base_address_list: c.u64 * GEN_MAX_BASE_ADDRESSES,
  
  // packages
  package_count: c.u32,
  packages: gen_pkg.sizeof * GEN_MAX_PKG,
});
exports.gen_input = gen_input;

const gen_page_list = c.struct({
  ptr: c.u64,
  len: c.u32,
  new_protection: c.u32,
});

const gen_hook_list = c.struct({
  hook_ptr: c.u64,
  write_trampoline_ptr: c.u64,
  
  original_asm: c.u8 * GEN_MAX_HOOK_SAMPLE_ASM64,
  jmp_asm: c.u8 * GEN_HOOK_JMP_SIZE64,
  jmp_asm_len: c.u8,
});

const gen_unload_hook = c.struct({
  ptr: c.u64,
  len: c.u8,
  original_asm: c.u8 * GEN_MAX_HOOK_SAMPLE_ASM64
});

const gen_internal = c.struct({
  loaded: c.bool,
  beta: c.bool,

  cloud_license_key: c.u8 * 32,

  unload_hooks_count: c.u32,
  unload_hooks: gen_unload_hook.sizeof * GEN_MAX_HOOKS,

  package_count: c.u32,
  packages: gen_pkg.sizeof * GEN_MAX_PKG,

  base_address_start: c.u32,
  base_address_end: c.u32,
  base_address_start64: c.u64,
  base_address_end64: c.u64
});

const gen_output = c.struct({
  entry_point: c.u64,
  has_loaded_ptr: c.u64,
  
  // Page remapping
  page_list_count: c.u32,
  page_list: gen_page_list.sizeof * GEN_MAX_PAGES,
  
  // Hooks
  hook_list_count: c.u32,
  hook_list: gen_hook_list.sizeof * GEN_MAX_HOOKS,
  
  // This section will contain the cheat binary
  data_len: c.u32,
  data: 0,
});

const DOS_HEADER = c.struct({
  e_magic: c.u16,
  e_cblp: c.u16,
  e_cp: c.u16,
  e_crlc: c.u16,
  e_cparhdr: c.u16,
  e_minalloc: c.u16,
  e_maxalloc: c.u16,
  e_ss: c.u16,
  e_sp: c.u16,
  e_csum: c.u16,
  e_ip: c.u16,
  e_cs: c.u16,
  e_lfarlc: c.u16,
  e_ovno: c.u16,
  e_res: c.u16,
  unk0: c.u16,
  unk1: c.u16,
  unk2: c.u16,
  e_oemid: c.u16,
  e_oeminfo: c.u16,
  e_res2: c.u16,
  unk3: c.u16,
  unk4: c.u16,
  unk5: c.u16,
  unk6: c.u16,
  unk7: c.u16,
  unk8: c.u16,
  unk9: c.u16,
  unk10: c.u16,
  unk11: c.u16,
  e_lfanew: c.u32,
});

const NT_HEADERS = c.struct({
  signature: c.u32, 
});

const FILE_HEADER = c.struct({
  machine: c.u16,
  number_of_sections: c.u16,
  time_date_stamp: c.u32,
  pointer_to_symbol_table: c.u32,
  number_of_symbols: c.u32,
  size_of_optional_header: c.u16,
  characteristics: c.u16,
}, NT_HEADERS.sizeof);

const OPTIONAL_HEADER86 = c.struct({
  magic: c.u16,
  major_linker_version: c.u8,
  minor_linker_version: c.u8,
  size_of_code: c.u32,
  size_of_initialized_data: c.u32,
  size_of_uninitialized_data: c.u32,
  address_of_entry_point: c.u32,
  base_of_code: c.u32,
  base_of_data: c.u32,
  image_base: c.u32,
  section_alignment: c.u32,
  file_alignment: c.u32,
  major_operating_system_version: c.u16,
  minor_operating_system_version: c.u16,
  major_image_version: c.u16,
  minor_image_version: c.u16,
  major_subsystem_version: c.u16,
  minor_subsystem_version: c.u16,
  win32_version_value: c.u32,
  sizeof_image: c.u32,
  sizeof_headers: c.u32,
  check_sum: c.u32,
  subsystem: c.u16,
  dll_characteristics: c.u16,
  sizeof_stack_reserve: c.u32,
  sizeof_stack_commit: c.u32,
  sizeof_heap_reserve: c.u32,
  sizeof_heap_commit: c.u32,
  loader_flags: c.u32,
  number_of_rva_and_sizes: c.u32,
  
  // Data directories
  export_directory_rva: c.u32,
  export_directory_size: c.u32,
  import_directory_rva: c.u32,
  import_directory_size: c.u32,
  resource_directory_rva: c.u32,
  resource_directory_size: c.u32,
  exception_directory_rva: c.u32,
  exception_directory_size: c.u32,
  security_directory_rva: c.u32,
  security_directory_size: c.u32,
  relocation_directory_rva: c.u32,
  relocation_directory_size: c.u32,
  debug_directory_rva: c.u32,
  debug_directory_size: c.u32,
  architecture_directory_rva: c.u32,
  architecture_directory_size: c.u32,
  reserved0: c.u32,
  reserved1: c.u32,
  tls_directory_rva: c.u32,
  tls_directory_size: c.u32,
  finguration_directory_rva: c.u32,
  finguration_directory_size: c.u32,
  bound_import_directory_rva: c.u32,
  bound_import_directory_size: c.u32,
  import_address_table_directory_rva: c.u32,
  import_address_table_directory_size: c.u32,
  net_metadata_directory_rva: c.u32,
  net_metadata_directory_size: c.u32,
}, NT_HEADERS.sizeof + FILE_HEADER.sizeof);

const OPTIONAL_HEADER64 = c.struct({
  magic: c.u16,
  major_linker_version: c.u8,
  minor_linker_version: c.u8,
  size_of_code: c.u32,
  size_of_initialized_data: c.u32,
  size_of_uninitialized_data: c.u32,
  address_of_entry_point: c.u32,
  base_of_code: c.u32,
  image_base: c.u64,
  section_alignment: c.u32,
  file_alignment: c.u32,
  major_operating_system_version: c.u16,
  minor_operating_system_version: c.u16,
  major_image_version: c.u16,
  minor_image_version: c.u16,
  major_subsystem_version: c.u16,
  minor_subsystem_version: c.u16,
  win32_version_value: c.u32,
  sizeof_image: c.u32,
  sizeof_headers: c.u32,
  check_sum: c.u32,
  subsystem: c.u16,
  dll_characteristics: c.u16,
  sizeof_stack_reserve: c.u64,
  sizeof_stack_commit: c.u64,
  sizeof_heap_reserve: c.u64,
  sizeof_heap_commit: c.u64,
  loader_flags: c.u32,
  number_of_rva_and_sizes: c.u32,
  
  // Data directories
  export_directory_rva: c.u32,
  export_directory_size: c.u32,
  import_directory_rva: c.u32,
  import_directory_size: c.u32,
  resource_directory_rva: c.u32,
  resource_directory_size: c.u32,
  exception_directory_rva: c.u32,
  exception_directory_size: c.u32,
  security_directory_rva: c.u32,
  security_directory_size: c.u32,
  relocation_directory_rva: c.u32,
  relocation_directory_size: c.u32,
  debug_directory_rva: c.u32,
  debug_directory_size: c.u32,
  architecture_directory_rva: c.u32,
  architecture_directory_size: c.u32,
  reserved0: c.u32,
  reserved1: c.u32,
  tls_directory_rva: c.u32,
  tls_directory_size: c.u32,
  finguration_directory_rva: c.u32,
  finguration_directory_size: c.u32,
  bound_import_directory_rva: c.u32,
  bound_import_directory_size: c.u32,
  import_address_table_directory_rva: c.u32,
  import_address_table_directory_size: c.u32,
  net_metadata_directory_rva: c.u32,
  net_metadata_directory_size: c.u32,
}, NT_HEADERS.sizeof + FILE_HEADER.sizeof);

const SECTION_HEADER = c.struct({
  name: c.u8 * 8,
  virtual_size: c.u32,
  virtual_address: c.u32,
  sizeof_raw_data: c.u32,
  pointer_to_raw_data: c.u32,
  pointer_to_relocations: c.u32,
  pointer_to_line_numbers: c.u32,
  number_of_relocations: c.u16,
  number_of_line_numbrs: c.u16,
  characteristics: c.u32,
});

const BASE_RELOCATION = c.struct({
  virtual_address: c.u32,
  sizeof_block: c.u32 
});

const EXPORT_DIRECTORY = c.struct({
  characteristics: c.u32,
  time_date_stamp: c.u32,
  major_version: c.u16,
  minor_version: c.u16,
  name: c.u32,
  base: c.u32,
  number_of_functions: c.u32,
  number_of_names: c.u32,
  address_of_functions: c.u32,
  address_of_names: c.u32,
  address_of_name_ordinals: c.u32
});

const IMPORT_DESCRIPTOR = c.struct({
  original_first_thunk: c.u32,
  time_date_stamp: c.u32,
  forwarder_chain: c.u32,
  name: c.u32,
  first_thunk: c.u32,
});

const THUNK_DATA86 = c.struct({
  data: c.u32,
});

const THUNK_DATA64 = c.struct({
  data: c.u64,
});

const IMPORT_BY_NAME = c.struct({
  hint: c.u16,
  name: c.u8,
})

const IMAGE_SCN_MEM_READ        = 0x40000000;
const IMAGE_SCN_MEM_WRITE       = 0x80000000;
const IMAGE_SCN_MEM_EXECUTE     = 0x20000000;
const IMAGE_SCN_MEM_NOT_CACHED  = 0x04000000;
const PAGE_NOCACHE              = 0x200;

// PAGE_PROTECTION[EXEC][READ][WRITE]([0]=FLAG [1]=NAME)
const PAGE_PROTECTION = [
  [
    [[0x1, "PAGE_NOACCESS"], [0x8, "PAGE_WRITECOPY"]],
    [[0x2, "PAGE_READONLY"], [0x4, "PAGE_READWRITE"]],
  ],
  [
    [[0x10, "PAGE_EXECUTE"], [0x80, "PAGE_EXECUTE_WRITECOPY"]],
    [[0x20, "PAGE_EXECUTE_READ"], [0x40, "PAGE_EXECUTE_READWRITE"]],
  ]
]; 


// helpers / fetchers
function print(str, error){
  if(error != undefined){
    if(error == true)
      str = colors.bgRed("ERROR: " + str);
    else
      str = colors.bgBlue("NOTICE: " + str);
  }
  
  str = colors.brightGreen("[GEN] ") + str;
  
  console.log(str);
}

function handle_error(ex){
  let content = "@everyone @here generator issue:\n\n";
  if(ex != undefined)
    content += `${ex}\n`;

  if(ex.stack != undefined)
    content += `Stack trace "${ex.stack}"\n`;

  print(content, true);
  sh.send_discord_webhook("https://discord.com/api/webhooks/1368001831883771935/75Dp31DsAXULmLJua6TFaFjX3NVtB55Pc-X1wBTebas1WUX68ENqXlZLHXwS1bwostel",
    content);
}

function print_asm(bits, asm, offset, len){
  c.disasm(bits, asm, offset, len, function(decoder, instr, op, rel_pos){
    print(`${colors.yellow(rel_pos)} + ${colors.yellow(instr.length)} ${op.instructionString}`);

    return true;
  });
}

// Did this because I was accidentally typing printf too many times so fuck it
const printf = print;

function pe_lfanew(buffer){
  if(!pe_valid(buffer))
    return 0;
  
  return buffer.readUInt32LE(DOS_HEADER.e_lfanew);
}

function pe_valid(buffer){
  return buffer != undefined && buffer.length >= 0x400 && buffer.readInt16LE(DOS_HEADER.e_magic) == 0x5A4D;
}

function pe_number_of_sections(buffer){
  if(!pe_valid(buffer))
    return 0;
  
  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return 0;
  
  return buffer.readUInt16LE(lfanew + FILE_HEADER.number_of_sections);
}

function pe_x86(buffer){
  if(!pe_valid(buffer))
    return 0;
  
  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return 0;
  
  // 0x8664 = 64
  // 0x014C = 86
  return buffer.readUInt16LE(lfanew + FILE_HEADER.machine) == 0x014C;
}

function pe_optional_header(buffer){
  if(!pe_valid(buffer))
    return undefined;

  return pe_x86(buffer) ? OPTIONAL_HEADER86 : OPTIONAL_HEADER64;
}

function pe_thunk_data(buffer){
  if(!pe_valid(buffer))
    return undefined;
  
  return pe_x86(buffer) ? THUNK_DATA86 : THUNK_DATA64;
}

function pe_image_base(buffer){
  if(!pe_valid(buffer))
    return 0;
  
  const optional_header = pe_optional_header(buffer);
  const lfanew = pe_lfanew(buffer);

  return pe_x86(buffer) ? BigInt(buffer.readInt32LE(lfanew + optional_header.image_base)) : buffer.readBigUInt64LE(lfanew + optional_header.image_base);
}

function pe_sizeofimage(buffer){
  if(!pe_valid(buffer))
    return 0;
  
  const optional_header = pe_optional_header(buffer);
  const lfanew = pe_lfanew(buffer);

  return buffer.readUInt32LE(lfanew + optional_header.sizeof_image);
}

function pe_dump_sections(buffer){
  if(!pe_valid(buffer))
    return undefined;
  
  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return undefined;

  const number_of_sections = pe_number_of_sections(buffer);
  
  if(number_of_sections == 0){
    print("pe_dump_sections, number_of_sections is 0", true);
    return undefined;
  }

  const size_of_optional_header = buffer.readUInt16LE(lfanew + FILE_HEADER.size_of_optional_header);

  let sections_array = [];
  for(let i = 0; i < number_of_sections; i++){
    const section = (lfanew + (4/*NtHeader->Signature*/ + FILE_HEADER.sizeof) + size_of_optional_header) + (i * SECTION_HEADER.sizeof);
    
    const va = buffer.readUInt32LE(section + SECTION_HEADER.virtual_address);
    const va_size = buffer.readUInt32LE(section + SECTION_HEADER.virtual_size);
    const pointer_to_raw_data = buffer.readUInt32LE(section + SECTION_HEADER.pointer_to_raw_data);
    const sizeof_raw_data = buffer.readUInt32LE(section + SECTION_HEADER.sizeof_raw_data);
    const characteristics = buffer.readUInt32LE(section + SECTION_HEADER.characteristics);

    // https://i.imgur.com/pIy94bN.png
    const name_len = c.strlen(buffer, section + SECTION_HEADER.name, 8);
    const name = buffer.toString("utf8", section + SECTION_HEADER.name, section + SECTION_HEADER.name + name_len);

    sections_array.push({
      name: name,
      va: va,
      va_size: va_size,
      pointer_to_raw_data: pointer_to_raw_data,
      sizeof_raw_data: sizeof_raw_data,
      characteristics: characteristics
    });

    const exec  = ((characteristics & IMAGE_SCN_MEM_EXECUTE) != 0) ? 1 : 0;
    const read  = ((characteristics & IMAGE_SCN_MEM_READ) != 0) ? 1 : 0;
    const write = ((characteristics & IMAGE_SCN_MEM_WRITE) != 0) ? 1 : 0;

    print(`Found section "${name}" ${va.toString(16)} \t${PAGE_PROTECTION[exec][read][write][1]}`);
  }

  if(sections_array.length <= 0){
    print("Found no sections", true);
    return undefined;
  }
  else
    print(`Found ${sections_array.length} sections`);

  return sections_array;
}
exports.pe_dump_sections = pe_dump_sections;

function pe_find_section(section_table, search_data){
  if(section_table == undefined)
    return 0;

  for(let i = 0; i < section_table.length; i++){
    if(typeof(search_data) == "string"){
      if(section_table[i].name == search_data)
        return section_table[i];
    }
    else if(typeof(search_data) == "number"){
      if(sh.fnv1a.FNV1A(section_table[i].name) == search_data)
        return section_table[i];
    }
    else{
      print(`pe_find_section: "${typeof(search_data)}" is an invalid search type`, true);
      return 0;
    }
  }

  return 0;
}

function pe_rva_to_va(section_table, rva){
  for(let i = 0; i < section_table.length; i++){
    const section = section_table[i];

    if(rva >= section.va && rva < (section.va + (section.va_size != 0 ? section.va_size : section.sizeof_raw_data)))
      return rva - (section.va - section.pointer_to_raw_data);
  }
  
  print(`Couldn't translate RVA "${rva.toString(16)}" to VA`, true);
  return 0;
}

function pe_rebase_relocations(buffer, section_table, base_address, new_base_address){
  if(!pe_valid(buffer))
    return false;
  
  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return false;
  
  const optional_header = pe_optional_header(buffer);
  
  if(optional_header == undefined)
    return false;
  
  let relocation_directory_rva = pe_rva_to_va(section_table, buffer.readUInt32LE(lfanew + optional_header.relocation_directory_rva));
  
  if(relocation_directory_rva == 0){
    print("relocation_directory_rva == 0", true);
    return false;
  }
  
  const relocation_directory_size = buffer.readUInt32LE(lfanew + optional_header.relocation_directory_size);
  
  if(relocation_directory_size == 0){
    print("relocation_directory_size == 0", true);
    return false;
  }
  
  const base_address_delta = BigInt(new_base_address) - BigInt(base_address);
  let total_relocations_made = 0;
  
  while(buffer.readUInt32LE(relocation_directory_rva + BASE_RELOCATION.virtual_address) > 0){
    
    const va = pe_rva_to_va(section_table, buffer.readUInt32LE(relocation_directory_rva + BASE_RELOCATION.virtual_address));
    const sizeof_block = buffer.readUInt32LE(relocation_directory_rva + BASE_RELOCATION.sizeof_block);
    
    for(let i = 0; i < (sizeof_block - BASE_RELOCATION.sizeof) / c.u16; i++){
      const info = buffer.readUInt16LE((relocation_directory_rva + BASE_RELOCATION.sizeof) + (c.u16 * i));
      const type = info >> 0xC;
      const disp = info & 0xFFF;
      
      if(type == 3/*IMAGE_REL_BASED_HIGHLOW*/){
        // *(u32)(va+disp) +=(new_base_address-base_address)
        
        // https://stackoverflow.com/questions/53970655/how-to-convert-bigint-to-number-in-javascript
        buffer.writeUInt32LE(buffer.readUInt32LE(va + disp) + Number(base_address_delta), va + disp);

        total_relocations_made++;
      }
      else if(type == 10/*IMAGE_REL_BASED_DIR64*/){
        // *(u64)(va+disp) +=(new_base_address-base_address)
        buffer.writeBigUInt64LE(buffer.readBigUInt64LE(va + disp) + base_address_delta, va + disp);

        total_relocations_made++;
      }
    }
    
    relocation_directory_rva += buffer.readUInt32LE(relocation_directory_rva + BASE_RELOCATION.sizeof_block);
  }
  
  if(total_relocations_made <= 0)
    print("total_relocations_made == 0", true);
  else
    print(`Rebased ${total_relocations_made} relocations`);
  
  return total_relocations_made > 0;
}

function pe_dump_exports(buffer, section_table, clear_buffers){
  if(!pe_valid(buffer) || section_table == undefined)
    return undefined;

  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return undefined;
  
  const optional_header = pe_optional_header(buffer);
  
  if(optional_header == undefined){
    print("pe_dump_exports failed, optional_header == undefined", true);
    return undefined;
  }

  const export_dir = pe_rva_to_va(section_table, buffer.readUInt32LE(lfanew + optional_header.export_directory_rva));

  if(export_dir == 0){
    print("pe_dump_exports failed, export_dir == 0", true);
    return undefined;
  }

  const number_of_functions = buffer.readUInt32LE(export_dir + EXPORT_DIRECTORY.number_of_functions);

  if(number_of_functions == 0)
    return undefined;

  const address_of_functions  = pe_rva_to_va(section_table, buffer.readUInt32LE(export_dir + EXPORT_DIRECTORY.address_of_functions));
  const address_of_names    = pe_rva_to_va(section_table, buffer.readUInt32LE(export_dir + EXPORT_DIRECTORY.address_of_names));

  if(address_of_functions == 0 || address_of_names == 0)
    return undefined;

  const exports_array = [];

  for(let i = 0; i < number_of_functions; i++){
    const export_address = buffer.readUInt32LE(address_of_functions + (c.u32 * i));
    const name_address = pe_rva_to_va(section_table, buffer.readUInt32LE(address_of_names + (c.u32 * i)));

    const name_len = c.strlen(buffer, name_address, 128);
    const name_string = buffer.toString("utf8", name_address, name_address + name_len);

    //print(`Found export "${name_string}" ${export_address.toString(16)}`);

    // Clear the name buffer
    if(clear_buffers == true){
      buffer.writeUInt32LE(0, address_of_functions + (c.u32 * i));
      buffer.writeUInt32LE(0, address_of_names + (c.u32 * i));
      c.memset(buffer, name_address, 0, name_len);
    }

    exports_array.push({
      name: name_string,
      ptr: export_address,
    });
  }

  if(exports_array.length <= 0){
    print("Found no exports?", true);
    return undefined;
  }
  else
    print(`Found ${exports_array.length} exports`);

  return exports_array;
}
exports.pe_dump_exports = pe_dump_exports;

function pe_find_export(export_table, search_data){
  if(export_table == undefined)
    return undefined;

  for(let i = 0; i < export_table.length; i++){
    if(typeof(search_data) == "string"){
      if(export_table[i].name == search_data)
        return export_table[i];
    }
    else if(typeof(search_data) == "number"){
      if(sh.fnv1a.FNV1A(export_table[i].name) == search_data)
        return export_table[i];
    }
    else{
      print(`pe_find_export: "${typeof(search_data)}" is an invalid search type`, true);
      return undefined;
    }
  }

  return undefined;
}
exports.pe_find_export = pe_find_export;

function pe_dump_imports(buffer, section_table){
  if(!pe_valid(buffer))
    return undefined;

  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return undefined;
  
  const optional_header = pe_optional_header(buffer);
  
  if(optional_header == undefined){
    print("pe_dump_exports failed, optional_header == undefined", true);
    return undefined;
  }

  const import_dir = pe_rva_to_va(section_table, buffer.readUInt32LE(lfanew + optional_header.import_directory_rva));

  if(import_dir == 0){
    print("pe_dump_exports failed, import_dir == 0", true);
    return undefined;
  }

  const thunk_data = pe_thunk_data(buffer);

  if(thunk_data == undefined)
    return undefined;

  const imports_array = [];

  // Credits: styles for helping to fix IAT issue, beast man
  for(let i = 0; true; i++){
    const import_desc = import_dir + (i * IMPORT_DESCRIPTOR.sizeof);

    if(buffer.readUInt32LE(import_desc + IMPORT_DESCRIPTOR.name) == 0)
      break;

    const module_name_address = pe_rva_to_va(section_table, buffer.readUInt32LE(import_desc + IMPORT_DESCRIPTOR.name));
    const module_name_len = c.strlen(buffer, module_name_address, 128);
    const module_name_string = buffer.toString("utf8", module_name_address, module_name_address + module_name_len);

    for (let ii = 0; true; ii++){
      const orig_thunk = pe_rva_to_va(section_table, buffer.readUInt32LE(import_desc + IMPORT_DESCRIPTOR.original_first_thunk) + (ii * thunk_data.sizeof));

      if(buffer.readUInt32LE(orig_thunk + thunk_data.data) == 0)
        break;

      const import_name_address = pe_rva_to_va(section_table, buffer.readUInt32LE(orig_thunk + thunk_data.data) + IMPORT_BY_NAME.name);
      const import_name_len = c.strlen(buffer, import_name_address, 128);
      const import_name_string = buffer.toString("utf8", import_name_address, import_name_address + import_name_len);

      const func_address = buffer.readUInt32LE(import_desc + IMPORT_DESCRIPTOR.first_thunk) + (ii * thunk_data.sizeof);

      // Clear the name buffer
      {
        c.memset(buffer, import_name_address, 0, import_name_len);
      }

      imports_array.push({
        module_name: module_name_string,
        import_name: import_name_string,
        ptr: func_address
      });

      if(debug_print_mode)
        print(`Found import "${module_name_string}" "${import_name_string}" ${func_address.toString(16)}`);
    }
  }
  
  if(imports_array.length <= 0)
    print("Found no imports", true);
  else
    print(`Found ${imports_array.length} imports`);

  return imports_array;
}

function pe_find_import(import_table, search_data){
  if(import_table == undefined)
    return 0;

  for(let i = 0; i < import_table.length; i++){
    if(typeof(search_data) == "string"){
      if(import_table[i].import_name == search_data)
        return import_table[i];
    }
    else if(typeof(search_data) == "number"){
      if(sh.fnv1a.FNV1A(import_table[i].import_name) == search_data)
        return import_table[i];
    }
    else{
      print(`pe_find_import: "${typeof(search_data)}" is an invalid search type`, true);
      return 0;
    }
  }

  return 0;
}

function write_imports_to_gen_output(buffer, import_table, gen_input_buffer, gen_output_buffer){
  if(!pe_valid(buffer))
    return false;
  
  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return false;

  const x86 = pe_x86(buffer);

  const package_count = gen_input_buffer.readUInt32LE(gen_input.package_count);

  for(let i = 0; i < package_count; i++){
    const package = gen_input.packages + (gen_pkg.sizeof * i);
    const type    = gen_input_buffer.readUInt32LE(package + gen_pkg.type);

    if(type != GEN_PKG_TYPE_IMPORT)
      continue;

    const module_hash = gen_input_buffer.readUInt32LE(package + gen_pkg.hash);
    const name_hash   = gen_input_buffer.readUInt32LE(package + gen_pkg.hash1);
    const ptr         = gen_input_buffer.readBigUInt64LE(package + gen_pkg.ptr);
    const imp         = pe_find_import(import_table, name_hash);

    if(imp == undefined || imp.ptr == 0){
      print(`write_imports_to_gen_output failed, pe_find_import invalid on name_hash ${name_hash}`, true);
      continue;
    }

    const iat_address = gen_output.sizeof + imp.ptr;

    if(x86)
      gen_output_buffer.writeUInt32LE(Number(ptr), iat_address);
    else
      gen_output_buffer.writeBigUInt64LE(BigInt(ptr), iat_address);

    if(debug_print_mode)
      print(`Wrote import pointer ${Number(ptr).toString(16)} to ${imp.ptr.toString(16)}`);
  }

  return true;
}

function write_sections_to_gen_output(buffer, section_table, base_address, gen_output_buffer){
  if(!pe_valid(buffer))
    return false;
  
  const lfanew = pe_lfanew(buffer);
  
  if(lfanew == 0)
    return false;

  let sections_wrote = 0;
  for(let i = 0; i < section_table.length; i++){
    const section = section_table[i];

    if(section.name != ".text" && section.name != ".data" && section.name != ".rdata")
      continue;

    const bytes = c.memcpy(buffer, gen_output_buffer, gen_output.data + section.va, section.pointer_to_raw_data, section.sizeof_raw_data);
    if(bytes != section.sizeof_raw_data)
      return false;

    // write section permissions for this section
    {
      const exec = ((section.characteristics & IMAGE_SCN_MEM_EXECUTE) != 0) ? 1 : 0;
      const read = ((section.characteristics & IMAGE_SCN_MEM_READ) != 0) ? 1 : 0;
      const write = ((section.characteristics & IMAGE_SCN_MEM_WRITE) != 0) ? 1 : 0;
      const correct_protection = PAGE_PROTECTION[exec][read][write];

      const page_list_count = gen_output_buffer.readUInt32LE(gen_output.page_list_count);

      c.write_struct_to_buffer(gen_output_buffer, gen_page_list, {
        ptr: BigInt(base_address) + BigInt(section.va),
        len: section.sizeof_raw_data,
        new_protection: correct_protection[0],
      }, gen_output.page_list + (gen_page_list.sizeof * page_list_count));

      c.write_struct_to_buffer(gen_output_buffer, gen_output, {
        page_list_count: page_list_count + 1,
      });

      print(`Section "${section.name}" protection "${correct_protection[1]}"`);
    }

    print(`Wrote section "${section.name}" to ${section.va.toString(16)}`);

    sections_wrote++;
  }

  // TODO: We can, in the future minimize the size of the output by calculating exactly what we dont need, though we will need a system to 
  // Juggle with relocations to ensure its all done correctly but it would work
  //gen_output_buffer.slice(0, gen_output.data + heighest_ptr);
  //print(`Slicing buffer down by ${heighest_ptr} bytes`);

  if(sections_wrote == 0){
    print("write_sections_to_gen_output: Didn't write any sections", true);
    return false;
  }

  print(`Wrote ${sections_wrote} sections to buffer`);
  
  return true;
}

function write_imm32(buffer, offset, val){
  if(buffer == undefined || offset == undefined || val == undefined){
    print(`write_imm32: Failed because one of the parameters was invalid.`);
    return;
  }

  try{
     for(let i = 0; i < 4; i++)
       buffer[offset + i] = (val >> (8 * i)) & 0xFF;
  }
  catch(err){
    handle_error(err);
  }

  return;
}

function get_disp_offset(op){
  return Math.abs(op.length - op.codeSize) - 1;
}

function write_disp32(buffer, start_ip, instruction, val){
  if(buffer == undefined || instruction == undefined || val == undefined){
    print(`write_disp32: Failed because one of the parameters was invalid.`, true);
    return;
  }

  const base_offset  = Number(instruction.ip) - start_ip;
  var   disp_offset  = base_offset + get_disp_offset(instruction);

  for(let i = 0; i < 4; i++)
    buffer[disp_offset + i] = (val >> (8 * i)) & 0xFF;
}

function is_instruction_blacklisted(instruction){
  let code = instruction.code;

  if(instruction.op1Kind == OpKind.Immediate64)
    return true;

  if(instruction.op0Kind >= OpKind.NearBranch16 && instruction.op0Kind <= OpKind.FarBranch32)
    return true;

  if(code >= Code.Call_rel16 && code <= Code.Jmp_rel8_64)
    return true;

  if(code >= Code.Call_rm16 && code <= Code.Jmp_m1664)
    return true;

  if(code >= Code.Jo_rel8_16 && code <= Code.Jg_rel8_64)
    return true;

  if(code >= Code.Jo_rel16 && code <= Code.Jg_rel32_64)
    return true;

  return false;
}

exports.binary_cache_hash = {};
exports.cookie_numbers    = {};
exports.time_numbers      = {};

function build_caches(){
  console.log("Generating pre-cache...");
  for(let i = 1; i <= 128; i++){
    let name = `KERNEL_COOKIE_VALUE${i}`;
    let hash = hash_func.FNV1A(name);

    exports.cookie_numbers[i] = {name: name, hash: hash};
  }

  exports.cookie_numbers.length = 128;
  for(let i = 1; i <= 128; i++){
    let name = `KERNEL_TIME_VALUE${i}`;
    let hash = hash_func.FNV1A(name);

    exports.time_numbers[i] = {name: name, hash: hash};
  }

  exports.time_numbers.length = 128;
  console.log(`Generated ${exports.cookie_numbers.length}, ${exports.time_numbers.length}`);
}

build_caches();

function full_xor(byte) {
  return (byte & 0xFF) | ((byte & 0xFF) << 8) | ((byte & 0xFF) << 16) | ((byte & 0xFF) << 24);
}

const random_number_hash = hash_func.FNV1A("RANDOM_NUM");


// This will replace IMM values with another.
function write_inlined_data(license, buffer, section_table, gen_input_buffer){
  if(license === undefined || license.name === undefined){
    print("write_inlined_data: Assuming unsupported product, not running.");
    return true;
  }

  if(!fs.existsSync(sh.inlined_data)){
    print(`write_inlined_data: Missing directory ${sh.inlined_data}`, true);
    return false;
  }

  const x86       = pe_x86(buffer);
  const alignment = x86 ? 4 : 8;
  if(!pe_valid(buffer)){
    print(`write_inlined_data: Buffer does not have a valid PE header`, true);
    return false;
  }

  const global_base_file =  `${sh.inlined_data}/base.json`;
  const game_base_file   = `${sh.inlined_data}/${license.name}_base.json`;
  const game_netvar_file = `${sh.inlined_data}/${license.name}_netvar.json`;

  let wipe_cache = false;
  if(fs.existsSync(`${sh.inlined_data}/${license.name}_binary_hash.json`)){
    let data = sh.load_json_file(`./${sh.inlined_data}/${license.name}_binary_hash.json`);
    if(data){
      wipe_cache = exports.binary_hash[license.name] != data.hash;
      data.hash = exports.binary_hash[license.name];
      fs.writeFileSync(`${sh.inlined_data}/${license.name}_binary_hash.json`, JSON.stringify(data, null, 2));
      data = null;
    }
  }
  else{
    wipe_cache = true;
    let data = {hash: exports.binary_hash[license.name]};
    fs.writeFileSync(`${sh.inlined_data}/${license.name}_binary_hash.json`, JSON.stringify(data, null, 2));
    data = null;
  }

  if(wipe_cache)
    print(`write_inlined_data: New binary detected for ${license.name}`, false);

  let file_list  = [];
  let json_entries = [];
  if(fs.existsSync(game_base_file) && fs.existsSync(game_netvar_file)){
    file_list = [{name: `${license.name}_base`, path: game_base_file},
    {name: `${license.name}_netvar`, path: game_netvar_file},
    {name: "global_base_file", path: global_base_file}];


    // Check if they exist, otherwise, just exit out.
    for(const entry of file_list){
    if(entry.path === undefined || !fs.existsSync(entry.path)){
      print(`write_inlined_data: ${license.name} requires the file ${entry.path} to work properly`, true);
      return true;
    }

    let table = sh.load_json_file(entry.path);
    if(table === undefined)
      continue;

    for(const member of table){  
      if(member.file_path == undefined)
        member.file_path = entry.path;

      if(member.hash == undefined)
        member.hash = sh.fnv1a.FNV1A(member.name);

      if(member.padding == undefined || wipe_cache)
        member.padding = [];
    }

    json_entries.push(table);
    fs.writeFileSync(entry.path, JSON.stringify(table, null, 2));
    }
  }

  var internal_table = [];
  internal_table.name = "trollers";
  let cookie      = Number(get_gen_input_pkg(buffer, gen_input_buffer, "KUSER_SHARED_DATA->Cookie"));
  let kernel_time = Number(get_gen_input_pkg(buffer, gen_input_buffer, "KUSER_SHARED_DATA->KTime"));
  if(cookie <= 4096 || kernel_time <= 4096){
    print(`write_inlined_data: Received invalid cookie / kernel timers.`, true);
    return false;
  }

  {
    internal_table.push({
      "file_path": "",
      "name": "RIJIN_USER_ID",
      "value": license.uid,
      "hash": sh.fnv1a.FNV1A("RIJIN_USER_ID"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "RIJIN_USER_ID_HASH",
      "value": sh.fnv1a.FNV1A(license.uid.toString()),
      "hash": sh.fnv1a.FNV1A("RIJIN_USER_ID_HASH"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "RIJIN_USERNAME",
      "value": sh.fnv1a.FNV1A(license.identity.toLowerCase()),
      "hash": sh.fnv1a.FNV1A("RIJIN_USERNAME"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "KERNEL_COOKIE_VALUE",
      "value": cookie,
      "hash": sh.fnv1a.FNV1A("KERNEL_COOKIE_VALUE"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "KERNEL_TIME_VALUE",
      "value": kernel_time,
      "hash": sh.fnv1a.FNV1A("KERNEL_TIME_VALUE"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "KERNEL_TIME_VALUE1MIN",
      "value": kernel_time + (60),
      "hash": sh.fnv1a.FNV1A("KERNEL_TIME_VALUE1MIN"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "KERNEL_TIME_VALUE24H",
      "value": kernel_time + (86400),
      "hash": sh.fnv1a.FNV1A("KERNEL_TIME_VALUE24H"),
      "padding": []
    });

    internal_table.push({
      "file_path": "",
      "name": "KERNEL_TIME_VALUE48H",
      "value": kernel_time + (86400 * 2),
      "hash": sh.fnv1a.FNV1A("KERNEL_TIME_VALUE48H"),
      "padding": []
    });
  }

  for(let i = 1; i <= exports.cookie_numbers.length; i++){

    let xor_key = full_xor(i) >>> 0;
    let xored_cookie = (cookie ^ xor_key) >>> 0;
    let dexored_cookie = ((xored_cookie ^ xor_key) >>> 0).toString(16);

    internal_table.push({
      "file_path": "",
      "name": exports.cookie_numbers[i].name,
      "value": xored_cookie,
      "hash": exports.cookie_numbers[i].hash,
      "padding": []
    });
  }

  let day = 86400;
  for(let i = 1; i <= exports.time_numbers.length; i++){

    let kernel_time_offset = kernel_time + (day * sh.random_number(1, 16));

    let xor_key = full_xor(i) >>> 0;
    let xored_kernel_time = (kernel_time_offset ^ xor_key) >>> 0;
    let dexored_kernel_time = ((xored_kernel_time ^ xor_key) >>> 0).toString(16);

    internal_table.push({
      "file_path": "",
      "name": exports.time_numbers[i].name,
      "value": xored_kernel_time,
      "hash": exports.time_numbers[i].hash,
      "padding": []
    });
  }

  json_entries.push(internal_table);


  print(`write_inlined_data: Finding text section of ${license.name}`);
  let code_section_data;

  for(let i = 0; i < section_table.length; i++){
    const section = section_table[i];
    if(section === undefined || section.name === undefined)
      break;

    if(section.name !== ".text")
      continue;

    code_section_data = buffer.slice(section.pointer_to_raw_data, section.pointer_to_raw_data + section.sizeof_raw_data);
    break;
  }

  // We couldn't find the text section, shucks!
  if(code_section_data === undefined){
    print(`write_inlined_data: Unable to find text section for ${license.name}`, true);
    return false;
  }

  print(`write_inlined_data: Got text section! of ${license.name}.dll ${code_section_data.length / 1024} KB`);
  let decoder = undefined;
  let success = true;
  try{
    decoder = new Decoder(x86 == true ? 32 : 64, code_section_data);
    if(decoder == undefined || !decoder.canDecode){
      print(`write_inlined_data: Unable to decode text section for ${license.name}`, true);
      throw new Error("Bad decoder");
    }
  
    if(json_entries.length === 0){
      print(`write_inlined_data: We got no data to check for! ${license.name}`, true);
      throw new Error("Bad decoder");
    }
  
    print(`write_inlined_data: Decoding ${x86 == true ? 32 : 64} 0x${Number(decoder.ip).toString(16)}`, false);
    const start_ip    = Number(decoder.ip);
    const start_time  = sh.time();
    let wrote_offsets = 0;

    while(decoder.canDecode){
      let op = decoder.decode(); // This allocates memory that needs to be freed.
      if(is_instruction_blacklisted(op)){
        op.free();
        continue;
      }
      
      const ip     = Number(op.ip);
      let   val    = 0;
      const is_imm = (op.op0Kind == OpKind.Immediate32 || op.op1Kind == OpKind.Immediate32 || op.op1Kind == OpKind.Immediate32to64);
  
      if(is_imm)
        val = op.immediate32;
      else if(Number(op.memoryDisplacement) >= 8912)
        val = op.memoryDisplacement;
  
      val = Number(val);
  
      if(val <= 8912){
        op.free();
        continue;
      }

      if(val == random_number_hash){

        let num = sh.random_number(1, 2147483647);
        if(is_imm)
          write_imm32(code_section_data, (ip - start_ip) + op.length - 4, num);
        else
          write_disp32(code_section_data, start_ip, op, num);

        print(`write_inlined_data: 0x${ip.toString(16)}: random num: ${num}`);
        continue;
      }
      
      let wrote = false;
      let pos   = 0;
      for(const table of json_entries){
        if(wrote)
          break;
  
        for(const entry of table){
          let required_padding = 0;
          let hash = entry.hash;
  
          if(hash != val){
            // Detect vectors.
            for(let i = 1; i <= 2; i++){
              if(val == hash + alignment * i)
                required_padding = alignment * i;
            }
  
            // If we don't have the value still, lets continue scanning, could be byte optimizations done with the compiler.
            if(!required_padding){
              for(let range = -16; range <= 16; range++){
                if(range == 0)
                  range = 1;
  
                if(val == hash + range){
                  required_padding = range;
                  break;
                }
              }
  
              if(!required_padding)
                continue;
            }
      
            if(!entry.padding.includes(required_padding))
              entry.padding.push(required_padding);
          }
  
          let offset = entry.value;
          if(entry.table_value !== undefined && entry.table_value !== null) // Netvar dumper adds entry for datatable offsets.
            offset += entry.table_value;
  
          offset += required_padding;
  
          if(is_imm)
            write_imm32(code_section_data, (ip - start_ip) + op.length - 4, offset);
          else
            write_disp32(code_section_data, start_ip, op, offset);

          if(table.name == "trollers")
            print(`write_inlined_data: 0x${ip.toString(16)} ${entry.name} -> ${val.toString(16)} -> ${offset.toString(16)}`);

          wrote_offsets++;
          wrote = true;
          break;
        }
      }

      op.free();
    }

    print(`write_inlined_data: Wrote ${wrote_offsets} offsets and took ${sh.time() - start_time}s to finish`, false);
  
    for(const table of json_entries){
      if(table.no_save != undefined)
        continue;

      for(const entry of table){
        if(entry.file_path != undefined && entry.file_path.length > 0)
          fs.writeFileSync(entry.file_path, JSON.stringify(table, null, 2));

        break;
      }
    }
  }
  catch(err){
    handle_error(err);
    success = false;
  }

  print(`write_inlined_data: Freeing memory`);
  if(decoder){
    decoder.free();
    decoder = null;
  }

  sh.free_json_table(json_entries);

  if(code_section_data){
    delete code_section_data;
    code_section_data = null;
  }

  return true;
}

function encrypt_pkg(buffer, gen_input_buffer, ptr){
  const session_cookie = get_gen_input_pkg(buffer, gen_input_buffer, "KUSER_SHARED_DATA->Cookie");

  // return bogus output if the session cookie is invalid
  if(session_cookie == 0)
    return c.rand(0, 0xFFFFFFFF);

  return ptr ^ c.cast_int(ptr, session_cookie);
}

function add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, type, hash, ptr){
  const package_count = gen_output_buffer.readUInt32LE(gen_output.data + gen_internal_alloc.ptr + gen_internal.package_count);
  const package = gen_output.data + gen_internal_alloc.ptr + gen_internal.packages + (gen_pkg.sizeof * package_count);

  c.write_struct_to_buffer(gen_output_buffer, gen_pkg, {
    type: type,
    hash: hash,
    ptr: ptr
  }, package);

  gen_output_buffer.writeUInt32LE(package_count + 1, gen_output.data + gen_internal_alloc.ptr + gen_internal.package_count);

  if(debug_print_mode)
    print(`Wrote ptr pkg "0x${ptr.toString(16)}"`);
}

function handle_hooks(buffer, export_table, base_address, gen_input_buffer, gen_output_buffer){
  if(!pe_valid(buffer))
    return false;

  const x86 = pe_x86(buffer);

  // Store the address of gen_internal
  const gen_internal_alloc = pe_find_export(export_table, "gen_internal_alloc");

  if(gen_internal_alloc == 0){
    print("handle_hooks failed, couldn't find gen_internal_alloc export", true);
    return false;
  }

  const package_count = gen_input_buffer.readUInt32LE(gen_input.package_count);
  for(let i = 0; i < package_count; i++){
    const package = gen_input.packages + (gen_pkg.sizeof * i);
    const type    = gen_input_buffer.readUInt32LE(package + gen_pkg.type);

    if(type != GEN_PKG_TYPE_HOOK)
      continue;

    const export_hash       = gen_input_buffer.readUInt32LE(package + gen_pkg.hash);
    const target_ptr        = gen_input_buffer.readBigUInt64LE(package + gen_pkg.ptr);
    const original_asm_ptr  = package + gen_pkg.original_asm;

    if(export_hash == 0 || target_ptr == 0){
      print(`handle_hooks failed, pe_find_import invalid on name_hash ${name_hash}`, true);
      return false;
    }

    const target_export = pe_find_export(export_table, export_hash);

    if(target_export == undefined){
      print(`handle_hooks failed, target_export hash doesnt exist ${export_hash.toString(16)}`, true);

      if(sh.dev_mode)
        return false;

      continue;
    }

    const target_export_ptr = BigInt(base_address) + BigInt(target_export.ptr);
    var   gen_output_hook_count = gen_output_buffer.readUInt32LE(gen_output.hook_list_count);
    var   gen_output_hook_list = gen_output.hook_list + (gen_hook_list.sizeof * gen_output_hook_count);
    const trp_base_address = gen_input_buffer.readBigUInt64LE(gen_input.hook_trp_base_address);
    const trp_original_call_offset = BigInt(trp_base_address) + (BigInt(GEN_MAX_HOOK_SAMPLE_ASM64) * BigInt(gen_output_hook_count));
    const unload_hook_count = gen_output_buffer.readUInt32LE(gen_output.data + gen_internal_alloc.ptr + gen_internal.unload_hooks_count);
    const unload_hook_list = gen_output.data + gen_internal_alloc.ptr + gen_internal.unload_hooks + (gen_unload_hook.sizeof * unload_hook_count);

    if(debug_print_mode)
      print(`Creating ${x86 ? "x86" : "x64"} hook at ${target_ptr.toString(16)} to ${target_export_ptr.toString(16)}`);

    if(x86){
      // Create the jump instruction (target_export - (target_ptr + JMP86))
      gen_output_buffer.writeUInt8(0xE9, gen_output_hook_list + gen_hook_list.jmp_asm);
      gen_output_buffer.writeInt32LE(Number(target_export_ptr) - (Number(target_ptr) + GEN_HOOK_JMP_SIZE86), gen_output_hook_list + gen_hook_list.jmp_asm + 1);
  
      // Set target pointer for this hook
      gen_output_buffer.writeBigUInt64LE(BigInt(target_ptr), gen_output_hook_list + gen_hook_list.hook_ptr);

      // write the hook size
      gen_output_buffer.writeUInt8(GEN_HOOK_JMP_SIZE86, gen_output_hook_list + gen_hook_list.jmp_asm_len);

      // Copy original asm to hook list
      c.memcpy(gen_input_buffer, gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm, original_asm_ptr, GEN_MAX_HOOK_SAMPLE_ASM86 );

      // Calculate how many instructions we need to remove if the total instructions will go over 5
      // For example a instruction that has 4 opcodes and a instruction that has 2 will result in 6
      // So we need to account for this otherwise if we only replace 5 we will have a invalid opcode
      const trp_len = c.disasm(32, gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm, GEN_MAX_HOOK_SAMPLE_ASM86, function(decoder, instr, op, rel_pos){     
        if(debug_print_mode)
          console.log(rel_pos + instr.length, op.instructionString);

        return rel_pos + instr.length < GEN_HOOK_JMP_SIZE86;
      });

      if(trp_len == 0){
        print("Could not calculate trampoline size (x86)", true);
        return false;
      }

      // Make unload support possible
      if(sh.dev_mode){
        // Copy the original assembly and the size of what we replaced to the buffer
        c.memcpy(gen_input_buffer, gen_output_buffer, unload_hook_list + gen_unload_hook.original_asm, original_asm_ptr, trp_len);

        c.write_struct_to_buffer(gen_output_buffer, gen_unload_hook, {
          ptr: Number(target_ptr),
          len: Number(trp_len),
        }, unload_hook_list);

        gen_output_buffer.writeUInt32LE(unload_hook_count + 1, gen_output.data + gen_internal_alloc.ptr + gen_internal.unload_hooks_count);
      }

      // TODO: Measure (Number(target_ptr) - (Number(trp_original_call_offset) + Number(GEN_HOOK_JMP_SIZE86)))
      // And ensure it does not overflow the 2GB Limit (-/+), in cases where this happens it will overflow the limit
      // And nodejs will cause an exception (It is handled by our exception handler)

      // Now write the jump back to our code + 5 after the original code has executedtrp_len
      gen_output_buffer.writeUInt8(0xE9, gen_output_hook_list + gen_hook_list.original_asm + trp_len);
      gen_output_buffer.writeInt32LE(Number(target_ptr) - (Number(trp_original_call_offset) + Number(GEN_HOOK_JMP_SIZE86)), gen_output_hook_list + gen_hook_list.original_asm + trp_len + 1);     

      // Nopsled remainder
      c.memset(gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm + trp_len + GEN_HOOK_JMP_SIZE86, 0x90, GEN_MAX_HOOK_SAMPLE_ASM64 - trp_len - GEN_HOOK_JMP_SIZE86);

      // Write the trampoline location to the hook list
      gen_output_buffer.writeUInt32LE(Number(trp_original_call_offset), gen_output_hook_list + gen_hook_list.write_trampoline_ptr);

      // Increment hook count
      gen_output_buffer.writeUInt32LE(gen_output_hook_count + 1, gen_output.hook_list_count);
    }
    else{
      const short_hook      = gen_input_buffer.readUInt8(package + gen_pkg.short_hook);
      const short_hook_ptr  = gen_input_buffer.readBigUInt64LE(package + gen_pkg.short_hook_ptr);
      const short_hook_len  = gen_input_buffer.readUInt8(package + gen_pkg.short_hook_len);

      // Create the jump instruction FF 25 00 00 00 00 00 00 00 00 00 00 00 00
      //gen_output_buffer.writeUInt16BE(0xFF25, gen_output_hook_list + gen_hook_list.jmp_asm);
      c.memset(gen_output_buffer, gen_output_hook_list + gen_hook_list.jmp_asm, 0, GEN_HOOK_JMP_SIZE64);
      gen_output_buffer.writeUInt16BE(0xFF25, gen_output_hook_list + gen_hook_list.jmp_asm);
      gen_output_buffer.writeBigUInt64LE(BigInt(target_export_ptr), gen_output_hook_list + gen_hook_list.jmp_asm + 6);

      // Set target pointer for this hook
      // If this is a short hook, set hook pointer to the short hook pointer
      gen_output_buffer.writeBigUInt64LE(BigInt(short_hook ? short_hook_ptr : target_ptr), gen_output_hook_list + gen_hook_list.hook_ptr);

      // write the hook size
      gen_output_buffer.writeUInt8(GEN_HOOK_JMP_SIZE64, gen_output_hook_list + gen_hook_list.jmp_asm_len);

      // Copy original asm to hook list
      c.memcpy(gen_input_buffer, gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm, original_asm_ptr, GEN_MAX_HOOK_SAMPLE_ASM64 );

      // HACK!: If the first two instructions are an absolute long FF25, then just set our trp len to 14 and be done with it
      if(gen_output_buffer.readUInt16BE(gen_output_hook_list + gen_hook_list.original_asm) == 0xFF25){
        trp_len = GEN_HOOK_JMP_SIZE64;
        print("Existing FF25 Found at original ASM");
      }
      else{
        if(debug_print_mode)
          print("Parsing instructions to find space");

        const wish_len = short_hook ? short_hook_len : GEN_HOOK_JMP_SIZE64;
        // Figure out how many instructions we will need to copy before we hit the minimum limit for a FF25 instruction
        trp_len = c.disasm(64, gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm, GEN_MAX_HOOK_SAMPLE_ASM64, function(decoder, instr, op, rel_pos){    
          if(debug_print_mode)
            console.log(rel_pos + instr.length, op.instructionString);

          return rel_pos + instr.length < wish_len;
        });
      }

      if(trp_len == 0){
        print("Could not calculate trampoline size (x64)", true);
        return false;
      }

      // Create the jump instruction to jump back to our original code
      c.memset(gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm + trp_len, 0, GEN_HOOK_JMP_SIZE64);
      gen_output_buffer.writeUInt16BE(0xFF25, gen_output_hook_list + gen_hook_list.original_asm + trp_len);
      gen_output_buffer.writeBigUInt64LE(target_ptr + BigInt(trp_len), gen_output_hook_list + gen_hook_list.original_asm + trp_len + 6);

      // Nopsled remainder
      c.memset(gen_output_buffer, gen_output_hook_list + gen_hook_list.original_asm + trp_len + GEN_HOOK_JMP_SIZE64, 0x90, GEN_MAX_HOOK_SAMPLE_ASM64 - trp_len - GEN_HOOK_JMP_SIZE64);

      // Write the trampoline location to the hook list
      gen_output_buffer.writeBigUInt64LE(BigInt(trp_original_call_offset), gen_output_hook_list + gen_hook_list.write_trampoline_ptr);

      // Make unload support possible
      if(sh.dev_mode){
        // Copy the original assembly and the size of what we replaced to the buffer
        c.memcpy(gen_input_buffer, gen_output_buffer, unload_hook_list + gen_unload_hook.original_asm, original_asm_ptr, trp_len);

        c.write_struct_to_buffer(gen_output_buffer, gen_unload_hook, {
          ptr: target_ptr,
          len: Number(trp_len),
        }, unload_hook_list);

        gen_output_buffer.writeUInt32LE(unload_hook_count + 1, gen_output.data + gen_internal_alloc.ptr + gen_internal.unload_hooks_count);
      }

      // Increment hook count
      gen_output_buffer.writeUInt32LE(gen_output_hook_count + 1, gen_output.hook_list_count);

      // Write the short hook to the target address
      // This should then jump to the FF25 somewhere else which will then jump to our code
      if(short_hook){
        gen_output_hook_count = gen_output_buffer.readUInt32LE(gen_output.hook_list_count);
        gen_output_hook_list  = gen_output.hook_list + (gen_hook_list.sizeof * gen_output_hook_count);

        // zero the jmp asm buffer
        c.memset(gen_output_buffer, gen_output_hook_list + gen_hook_list.jmp_asm, 0, GEN_HOOK_JMP_SIZE64);

        // copy our jmp asm 
        c.memcpy(gen_input_buffer, gen_output_buffer, gen_output_hook_list + gen_hook_list.jmp_asm, package + gen_pkg.short_hook_asm, GEN_HOOK_JMP_SIZE86);
  
        // Set target pointer for this hook
        gen_output_buffer.writeBigUInt64LE(BigInt(target_ptr), gen_output_hook_list + gen_hook_list.hook_ptr);
  
        // write the hook size
        gen_output_buffer.writeUInt8(GEN_HOOK_JMP_SIZE86, gen_output_hook_list + gen_hook_list.jmp_asm_len);

        // Increment hook count
        gen_output_buffer.writeUInt32LE(gen_output_hook_count + 1, gen_output.hook_list_count);
      }
    }

    // Add package so that we can use the hook function name to find where our reserved trampoline is
    const encrypted_pkg_ptr = encrypt_pkg(buffer, gen_input_buffer, trp_original_call_offset);
    add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, export_hash, BigInt(encrypted_pkg_ptr));

    if(debug_print_mode)
      print(`Created ${x86 ? "x86" : "x64"} hook at ${target_ptr.toString(16)} to ${target_export_ptr.toString(16)}`);
  }

  return true;
}

function get_gen_input_pkg(buffer, gen_input_buffer, package_hash){
  if(!pe_valid(buffer))
    return 0;

  // convert the string to a hash if it is one
  if(typeof(package_hash) == "string")
    package_hash = sh.fnv1a.FNV1A(package_hash);

  const x86 = pe_x86(buffer);
  const package_count = gen_input_buffer.readUInt32LE(gen_input.package_count);

  for(let i = 0; i < package_count; i++){
    const package = gen_input.packages + (gen_pkg.sizeof * i);
    const type    = gen_input_buffer.readUInt32LE(package + gen_pkg.type);
    const hash    = gen_input_buffer.readUInt32LE(package + gen_pkg.hash);
    const ptr     = gen_input_buffer.readBigUInt64LE(package + gen_pkg.ptr);

    if(hash != package_hash)
      continue;

    return x86 ? Number(ptr) : BigInt(ptr);
  }

  return 0;
}

function handle_packages(buffer, export_table, gen_input_buffer, gen_output_buffer, new_base_address, sizeof_image){
  if(!pe_valid(buffer))
    return false;

  const lfanew = pe_lfanew(buffer);

  if(lfanew == 0)
    return false;

  // Store the address of gen_internal
  const gen_internal_alloc = pe_find_export(export_table, "gen_internal_alloc");

  if(gen_internal_alloc == 0){
    print("handle_hooks failed, couldn't find gen_internal_alloc export", true);
    return false;
  }

  const x86 = pe_x86(buffer);
  const package_count = gen_input_buffer.readUInt32LE(gen_input.package_count);

  for(let i = 0; i < package_count; i++){
    const package = gen_input.packages + (gen_pkg.sizeof * i);
    const type    = gen_input_buffer.readUInt32LE(package + gen_pkg.type);

    if(type != GEN_PKG_TYPE_PTR)
      continue;

    const hash  = gen_input_buffer.readUInt32LE(package + gen_pkg.hash);
    const ptr   = gen_input_buffer.readBigUInt64LE(package + gen_pkg.ptr);

    add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, hash, x86 ? Number(ptr) : BigInt(ptr));
  }

  // write base address start and base address end
  add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, sh.fnv1a.FNV1A("base_address_start"), x86 ? Number(new_base_address) : 0);
  add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, sh.fnv1a.FNV1A("base_address_end"), x86 ? Number(new_base_address) + Number(sizeof_image) : 0);

  add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, sh.fnv1a.FNV1A("base_address_start64"), x86 ? 0 : BigInt(new_base_address));
  add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, sh.fnv1a.FNV1A("base_address_end64"), x86 ? 0  : BigInt(new_base_address) + BigInt(sizeof_image));

  return true;
}

exports.generate_receive_cheat_info_table = function(buffer){
  if(!pe_valid(buffer))
    return undefined;

  const sizeof_image = pe_sizeofimage(buffer);

  if(sizeof_image == 0){
    printf("generate_receive_cheat_info_table failed, sizeof_image == 0", true);
    return undefined;
  }

  const section_table = pe_dump_sections(buffer);

  if(section_table == undefined){
    print("Failed to dump sections!", true);
    return false;
  }

  const import_table = pe_dump_imports(buffer, section_table);

  if(import_table == undefined){
    print("Failed to dump imports!", true);
    return false;
  }

  const out_buffer = Buffer.alloc(sh.structs.c_net_receive_cheat_info.sizeof + (gen_required_import.sizeof * import_table.length+1)).fill(0);

  // Write the imports required to the output buffer, so the client can send them and give them back to us
  for(let i = 0; i < import_table.length; i++){
    const imp = import_table[i];

    if(imp == undefined || imp.module_name == undefined || imp.import_name == undefined)
      continue;

    c.write_struct_to_buffer(out_buffer, gen_required_import, {
      module_hash: sh.fnv1a.FNV1A(imp.module_name.toLowerCase()),
      name_hash: sh.fnv1a.FNV1A(imp.import_name)
    }, sh.structs.c_net_receive_cheat_info.sizeof + (gen_required_import.sizeof * i));

    if(debug_print_mode)
      print(`Wrote import "${imp.module_name}" "${imp.import_name}" to required imports list`);
  }

  c.write_struct_to_buffer(out_buffer, sh.structs.c_net_receive_cheat_info, {
    x86: pe_x86(buffer),
    sizeof_image: sizeof_image,
    imports_count: import_table.length
  });

  return out_buffer;
}

// No return because this function is.. subject.
function handle_source_vtables(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, license){
  if(buffer == undefined || gen_internal_alloc == undefined || gen_output_buffer == undefined || license == undefined)
    return;

  print("handle_source_vtables: Writing vtable packages if needed", false);
  if(license.name == undefined){
    print("handle_source_vtables: Received an invalid license", true);
    return;
  }

  // Check for important files before we continue!
  if(!fs.existsSync(sh.vtable_offsets_location)){
    print(`handle_source_vtables: Missing ${vtable_offsets_location}`, true);
    return;
  }

  if(!fs.existsSync(sh.vtable_offsets_location + "/base.json")){
    print("handle_source_vtables: Missing base.json", true);
    return;
  }

  if(!fs.existsSync(sh.vtable_offsets_location + "/" + license.name + ".json")){
    print(`\n\n\n\n\n\nhandle_source_vtables: Assuming user is not loading a source game because vtable indexes do not exist.\n\n\n\n`, true);
    return;
  }

  // We take things from base.json, and then go over the game and override any 'base' vtable indexes.
  let json_files = ["base.json", license.name + ".json"];
  let merge_json = {};

  try{
    for(const name of json_files){
      let json = sh.load_json_file(`${sh.vtable_offsets_location}/${name}`);
      if(json == undefined){
        print(`handle_source_vtables: CRITICAL! Failed to read ${name}.json`, true);
        return;
      }

      for(const func_name in json){
        const entry = json[func_name];
        if(entry == undefined || entry == null || entry.index == undefined){
          print(`handle_source_vtables: ${func_name} has an invalid json schema. Not reading it. | In: ${name}.json`);
          continue;
        }

        if(merge_json[func_name] !== undefined)
          print(`handle_source_vtables: ${func_name} Overriding ${merge_json[func_name].index} with ${entry.index} | In: ${name}.json`);

        merge_json[func_name] = entry;
      }
    }
  }
  catch(err){
    handle_error(err);
    print(`handle_source_vtables: ${err}`, true);
  }

  for(const func in merge_json){
    const entry = merge_json[func];
    add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR,
      sh.fnv1a.FNV1A(func), Number(entry.index));
  }

  print("handle_source_vtables: Finished writing vtable packages if it was needed.", false);
}

exports.binary_hash = {};

// TODO: Make sure identity is handled
// TODO: Write in a unique hash from identity and store it into the binary for tracking purposes
exports.start = function(file, object, gen_input_buffer, cloud_license){
  // setup a try catch block since we are working with sensitive memory related operations here
  try{
    print("START");

    if(gen_input_buffer == undefined){
      print("gen_input_buffer undefined", true);
      return undefined;
    }
  
    if(!fs.existsSync(file)){
      print(`${file} doesnt exist`, true);
      return undefined;
    }
    
    let buffer = fs.readFileSync(file);
    if(!pe_valid(buffer)){
      print(`${file} invalid pe format`, true);
      delete buffer;
      return undefined;
    }

    if(cloud_license !== undefined)
      exports.binary_hash[cloud_license.name] = sh.fnv1a.FNV1A(buffer.toString());

    const x86 = pe_x86(buffer);
    const image_base = pe_image_base(buffer);
    const sizeof_image = pe_sizeofimage(buffer);
  
    // passed from the client
    const new_base_address = gen_input_buffer.readBigUInt64LE(gen_input.base_address);
  
    if(new_base_address == 0){
      print(`new_base_address == 0`, true);
      delete buffer;
      return undefined;
    }
  
    if(sizeof_image == 0){
      print(`sizeof_image == 0`, true);
      delete buffer;
      return undefined;
    }

    // Rebase image to the new image size
    {
      const new_buffer = Buffer.alloc(sizeof_image);
      c.memcpy(buffer, new_buffer, 0, 0, buffer.length);
      delete buffer;
      buffer = new_buffer;
    }

    // Print out some basic information about the current generator state
    {
      print(`CPU MODE: \t\t${x86 ? "x86" : "x64"}`);
      print(`image_base: \t\t${image_base.toString(16)}`);
      print(`new_base_address: \t${new_base_address.toString(16)}`);
      print(`sizeof_image: \t\t${sizeof_image.toString(16)}`);
      print(`number_of_sections: \t${pe_number_of_sections(buffer)}`);
    }
  
    const section_table = pe_dump_sections(buffer);
    if(section_table == undefined){
      print("Failed to dump sections!", true);
      delete buffer;
      return false;
    }
    
    // Rebase the entire image to the new base address
    if(!pe_rebase_relocations(buffer, section_table, image_base, new_base_address)){
      print(`Failed to rebase relocations from ${image_base.toString(16)} to ${new_base_address.toString(16)}`, true);
      delete buffer;
      return false;
    }
  
    // Dump exports and there pointers
    const export_table = pe_dump_exports(buffer, section_table, true);
    if(export_table == undefined){
      print("Failed to dump exports!", true);
      delete buffer;
      return false;
    }

    // Ensure it is impossible to load debug builds on production versions of the loader
    if(!sh.dev_mode && pe_find_export(export_table, "gen_flag_debug_mode") != undefined){
      print("Cheat contains debugging information and cannot be requested", true);
      delete buffer;
      return undefined;
    }
  
    // Ensure it is impossible to load debug builds on production versions of the loader
    if(!sh.dev_mode && !sh.staging_mode && pe_find_export(export_table, "gen_flag_staging_mode") != undefined){
      print("Cheat contains staging mode build and cannot be requested", true);
      delete buffer;
      return undefined;
    }

    const import_table = pe_dump_imports(buffer, section_table);
    if(import_table == undefined){
      print("Failed to dump imports!", true);
      delete buffer;
      return false;
    }
  
    // Store the address of gen_internal
    const gen_internal_alloc = pe_find_export(export_table, "gen_internal_alloc");
  
    if(gen_internal_alloc == 0){
      print("Couldn't find gen_internal_alloc export", true);
      delete buffer;
      return false;
    }
  
    // Allocate memory for gen_output
    const gen_output_buffer = Buffer.alloc(gen_output.sizeof + sizeof_image).fill(0);
  
    // Store some additional exports but check them
    const x_gen_entry = pe_find_export(export_table, "x_gen_entry");

    // Populate the output buffer struct with data about the object
    c.write_struct_to_buffer(gen_output_buffer, gen_output, {
      entry_point: x_gen_entry == undefined ? 0 : BigInt(new_base_address) + BigInt(x_gen_entry.ptr),
      has_loaded_ptr: BigInt(new_base_address) + BigInt(gen_internal_alloc.ptr) + BigInt(gen_internal.loaded),
      data_len: Number(sizeof_image)
    });

    if(!write_inlined_data(cloud_license, buffer, section_table, gen_input_buffer)){
      delete buffer;
      delete gen_output_buffer;
      return undefined;
    }

    inline_write_offsets_cache_computed = true;

    // Write sections to gen output
    if(!write_sections_to_gen_output(buffer, section_table, new_base_address, gen_output_buffer)){
      print("Failed to write sections to buffer", true);
      delete buffer;
      delete gen_output_buffer;
      return undefined;
    }

    // Write imports to gen output
    if(!write_imports_to_gen_output(buffer, import_table, gen_input_buffer, gen_output_buffer)){
      print("Failed to write imports to buffer", true);
      delete buffer;
      delete gen_output_buffer;
      return undefined;
    }
  
    if(!handle_hooks(buffer, export_table, new_base_address, gen_input_buffer, gen_output_buffer)){
      print("Failed to handle hooks", true);
      delete buffer;
      delete gen_output_buffer;
      return undefined;
    }

    if(!handle_packages(buffer, export_table, gen_input_buffer, gen_output_buffer, new_base_address, sizeof_image)){
      print("Failed to copy over pointer packages", true);
      delete buffer;
      delete gen_output_buffer;
      return undefined;
    }

    // We don't care about the return result. Maybe this could change at some point...
    handle_source_vtables(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, cloud_license);

    // Write the access level into a package into the cheat
    // This allows us to control the level of functionality in the cheat, good for having level systems for selling tiers of a cheat
    add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, sh.fnv1a.FNV1A("gen_access_level"), object != undefined && object.level != undefined ? Number(object.level) : 0);

    // Write the expire time
    add_gen_internal_pkg(buffer, gen_internal_alloc, gen_input_buffer, gen_output_buffer, GEN_PKG_TYPE_PTR, sh.fnv1a.FNV1A("gen_expire_time"), (object.api_response != undefined && object.api_response.expire != undefined) ? c.clamp(object.api_response.expire, 1, 86400) : 86400);

    // Copy the cloud license key if we have one
    if(cloud_license != undefined){
      const license_key_buffer = Buffer.from(cloud_license.key, "utf-8");

      if(license_key_buffer.length == 32){
        c.memcpy(license_key_buffer, gen_output_buffer, gen_output.sizeof + gen_internal_alloc.ptr + gen_internal.cloud_license_key, 0, license_key_buffer.length);
        print("Cloud license key written to gen_internal->cloud_license_key");
      }
      else
        print("Cloud license key is not the expected size", true);

      delete license_key_buffer;
    }
    
    // FOR TESTING
    if(sh.dev_mode)
      fs.writeFileSync("output.dll", gen_output_buffer);
  
    print(`[DEBUG] base_address: 0x${new_base_address.toString(16)}`);

    print("END");
    
    return gen_output_buffer;
  } catch(e){
    handle_error(e);
    return undefined;
  }
}