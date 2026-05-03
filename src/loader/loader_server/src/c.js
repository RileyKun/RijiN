// core
const colors = require('colors');
const { Decoder, DecoderOptions, Instruction, OpKind } = require("iced-x86");

// c++ sizes
exports.i8 = 1;
exports.i16 = 2;
exports.i32 = 4;
exports.i64 = 8;
exports.u8 = 1;
exports.u16 = 2;
exports.u32 = 4;
exports.ul64 = 8;
exports.u64 = 8;
exports.bool = 1;

exports.strlen = function(buffer, ptr, max_len){
  if(max_len == undefined)
    max_len = 128;

  let len = 0;
  while(buffer.readUInt8(ptr + (++len)) != 0 && len <= max_len)
    continue;

  return len;
}

exports.memset = function(buffer, ptr, val, len){
  while(len-- > 0)
    buffer.writeUInt8(val, ptr + len);
}

exports.rand = function(min, max){
  return Math.floor(Math.random() * (max - min) + min);
}

exports.clamp = function(val, min, max){
  return Math.min(Math.max(val, min), max);
}

exports.cast_int = function(appending_to, val){
  return typeof(appending_to) == "bigint" ? BigInt(val) : Number(val);
}

// memcpy(dst, src, len);
exports.memcpy = function(from_buffer, to_buffer, to_buffer_offset, from_buffer_offset, len){
  return from_buffer.copy(to_buffer, to_buffer_offset, from_buffer_offset, from_buffer_offset + len);
}

exports.write_struct_to_buffer = function(buffer, struct, table, offset){
  if(buffer.length < table.sizeof){
    console.log("[-] write_struct_to_buffer failed, buffer.length < table.sizeof");
    return false;
  }
  
  if(buffer == undefined){
    console.log("[-] write_struct_to_buffer failed, buffer undefined");
    return false;
  }
  
  if(table == undefined){
    console.log("[-] write_struct_to_buffer failed, table undefined");
    return false;
  }
  
  for (let key in table){
    if(struct[key] == undefined){
      console.log(`[-] write_struct_to_buffer failed, struct ${key} doesnt exist`);
      continue;
    }
    
    if(struct.data_sizes[key] == undefined){
      console.log(`[-] write_struct_to_buffer failed, datatype ${key} unknown?`);
      return false;
    }
    
    let data_len = struct.data_sizes[key];
    let data_pos = struct[key];
    const value = table[key];
    
    if(offset != undefined)
      data_pos += offset;
    
    if(data_len == exports.u64 && typeof(value) == "number")
      data_len = exports.u32;

    switch(data_len){
      case exports.bool:{
        buffer.writeUInt8(value, data_pos);
        break;
      }
      case exports.u8:{
        buffer.writeUInt8(value, data_pos);
        break;
      }
      case exports.u16:{
        
        if(value >= 0)
          buffer.writeUInt16LE(value, data_pos);
        else
          buffer.writeInt16LE(value, data_pos);
        break;
      }
      case exports.u32:{
        
        if(value >= 0)
          buffer.writeUInt32LE(value, data_pos);
        else
          buffer.writeInt32LE(value, data_pos);

        break;
      }
      case exports.u64:{
        if(value >= 0)
          buffer.writeBigUInt64LE(value, data_pos);
        else
          buffer.writeBigInt64LE(value, data_pos);

        break;
      }
      default:{
        console.log(colors.bgRed("[-] Unrecognized data pos:", colors.green(data_pos), "size:", colors.green(data_len)));
        break;
      }
    }
  }

  delete tbl;

  return true;
}

// Calculates automatically all the offsets and sizes for a particular structure, saving time to manually do offsets
exports.struct = function(table, offset){
  if(table == undefined || table.sizeof != undefined)
    return table;
  
  table.data_sizes = {};
  
  offset = offset == undefined ? 0 : offset;
  
  let total_len = offset;
  for (let key in table){
    const sizeof = table[key];
    
    if(sizeof == undefined)
      sizeof = 0;
    
    if(key == "data_sizes")
      continue;
    
    table.data_sizes[key] = table[key];
    table[key] = total_len;
    total_len = total_len + sizeof;
  }
  
  table.sizeof = total_len - offset;
  
  //if(process.argv[2] == "-dev")
    //console.log("[+] struct:", table);
  
  return table;
}

exports.buffer_from_offset = function(buffer, offset, free_old){
  
  if(offset >= buffer.length)
    return undefined;
  
  const new_buffer = Buffer.alloc(buffer.length - offset).fill(0);
  buffer.copy(new_buffer, 0, offset, buffer.length - offset); 
  
  if(free_old)
    delete buffer;
  
  return new_buffer;
}

exports.disasm = function(bits, asm, offset, len, callback){
  
  if(offset > 0){
    asm = exports.buffer_from_offset(asm, offset, false);
    
    // remap failed
    if(asm == undefined)
      return false;
  }

  const decoder = new Decoder(bits, asm, DecoderOptions.None);
  
  let rel_pos = 0;
  while(decoder.canDecode){
    const instr = decoder.decode();
    
    if (instr.isInvalid){
      console.log("[-] instruction invalid!");
      rel_pos = 0;
      break;
    }
    
    const should_break = !callback(decoder, instr, instr.opCode, rel_pos);
    rel_pos += instr.length;

    instr.free();

    if(should_break || len > 0 && rel_pos >= len)
      break;
  };
  
  decoder.free();
  
  if(offset > 0)
    delete asm;
  
  return rel_pos;
}