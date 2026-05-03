#pragma once

#define RUN_PROCEDURE(t) ( new t )->run()
class c_procedure_base{
public:

  u32 target_module_hash = 0;
  u32 target_section_hash = 0;
  bool bypass_pkg_errors = false;
  bool is_cheat_module   = true;

  virtual bool setup(){
    return true;
  }

  virtual bool write_pointers(){
    return true;
  }

  virtual bool write_signatures(){
    return true;
  }

  virtual bool write_hooks(){
    return true;
  }

  bool run(){
    DBG("[!] c_procedure_base::setup()\n");
    if(!setup()){
      delete this;
      SHOW_ERROR(ERR_PROCEDURE_SETUP_FAILED, true);
      return false;
    }

    DBG("[!] c_procedure_base::write_pointers()\n");
    if(!write_pointers()){
      delete this;
      SHOW_ERROR(ERR_PROCEDURE_WRITE_POINTERS_FAILED, true);
      return false;
    }

    DBG("[!] c_procedure_base::write_signatures()\n");
    if(!write_signatures()){
      delete this;
      SHOW_ERROR(ERR_PROCEDURE_WRITE_SIGNATURES_FAILED, true);
      return false;
    }

    DBG("[!] c_procedure_base::write_hooks()\n");
    if(!write_hooks()){
      delete this;
      SHOW_ERROR(ERR_PROCEDURE_WRITE_HOOKS_FAILED, true);
      return false;
    }

    delete this;

    return true;
  }

  // Sets and waits for a specific module
  NEVERINLINE bool set_module(u32 module_hash){
    DBG("[!] set_module: waiting for module %p\n", module_hash);

    if(inject->get_module_address(module_hash) != 0){
      DBG("[+] set_module: target_module_hash set to %p\n", module_hash);
      target_module_hash = module_hash;
      return true;
    }

    target_module_hash = 0;

    DBG("[-] set_module: failed to find module hash %p\n", module_hash);
    SHOW_ERROR_STR(ERR_PROCEDURE_SET_MODULE_TIMEOUT, "A required memory module could not be located, please try again.", false);
    return false;
  }

  NEVERINLINE uptr get_export_ptr(u32 name_hash, bool abs = false){
    uptr mod_ptr = inject->get_module_address(target_module_hash);

    if(mod_ptr == 0){
      SHOW_ERROR_STR(ERR_PROCEDURE_IMPORT_FAILED, "1/2", true);
      return false;
    }

    uptr import_ptr = inject->get_export(mod_ptr, name_hash);

    if(import_ptr == 0){
      SHOW_ERROR_STR(ERR_PROCEDURE_IMPORT_FAILED, "2/2", true);
      return false;
    }

    if(!abs)
      import_ptr -= mod_ptr;

    return import_ptr;
  }

  ALWAYSINLINE void set_section(u32 section_hash){
    target_section_hash = section_hash;
  }

  ALWAYSINLINE void add_ptr(u32 identifier_hash, u64 ptr){
    inject->input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), identifier_hash, 0, ptr);
  }

  #define SIG(sig) sig, XOR32(sizeof(sig)-1)
  NEVERINLINE u64 find_signature_internal(u32 module_hash, u32 section, u8* sig, u32 sig_len, bool displacement = false, i32 offset = 0){
    u64 module_address = inject->get_module_address(module_hash);

    if(module_address == 0){
      DBG("[-] find_signature: module %p doesnt exist\n", module_hash);
      return 0;
    }

    u32 lfanew = 0;
    if(!inject->read_memory(module_address + XOR32(0x3C/*e_lfanew*/), &lfanew, sizeof(lfanew))){
      DBG("[-] find_signature: lfanew invalid\n");
      return 0;
    }

    u32 offset_to_sections = lfanew + XOR32(0x18/*Magic*/) + (inject->x86mode ? XOR32(sizeof(IMAGE_OPTIONAL_HEADER32)) : XOR32(sizeof(IMAGE_OPTIONAL_HEADER64)));

    u16 number_of_sections = 0;
    if(!inject->read_memory(module_address + lfanew + XOR32(0x6/*NumberOfSections*/), &number_of_sections, sizeof(number_of_sections))){
      DBG("[-] find_signature: number_of_sections invalid\n");
      return 0;
    }

    u64 section_ptr = 0;
    u64 section_len = 0;

    // Find the section and its virtual address
    for (u32 i = 0; i < number_of_sections; i++) {
      IMAGE_SECTION_HEADER section_header;
      if (!inject->read_memory( (void*)(module_address + offset_to_sections + i * sizeof( IMAGE_SECTION_HEADER )), (void*)&section_header, sizeof(section_header)))
        continue;

      if(HASH_RT(section_header.Name) != section){
        DBG("[!] find_signature: skipping section %s\n", section_header.Name);
        continue;
      }

      section_ptr = module_address + section_header.VirtualAddress;
      section_len = section_header.SizeOfRawData;

      DBG("[+] find_signature: found section %s\n", section_header.Name);
      DBG("[+] find_signature: section_ptr %p\n", section_ptr);
      DBG("[+] find_signature: section_len %p\n", section_len);

      break;
    }

    if(section_ptr == 0 || section_len == 0){
      DBG("[-] find_signature: couldn't find section %p\n", section);
      return 0;
    }

#define SIGNATURE_SCAN_BLOCK_SIZE 0xF4240/*1MB*/
    u64 found_ptr  = 0;
    u8* scan_cache = (u8*)I(malloc)(XOR32(SIGNATURE_SCAN_BLOCK_SIZE));

    // We do this so that the search_size will always be aligned with the section length
    // If we scan at 100000 but the section length left is only 500 we will only scan 500
    u32 search_size = math::smallest(SIGNATURE_SCAN_BLOCK_SIZE, section_len - search_size);
    for(u32 i = 0; i < section_len; i += search_size){
      search_size = math::smallest(SIGNATURE_SCAN_BLOCK_SIZE, section_len - i);

      if(search_size == 0)
        break;

      if(!inject->read_memory(section_ptr + i, scan_cache, search_size)){
        DBG("[-] find_signature: failed to copy memory to cache\n");
        break;
      }

      // We MUST scan like this, If we don't scan like this you can run into instruction comparison overlapping
      for(u32 ii = 0; ii < search_size - sig_len; ii++){

        // Scan through the signature and check if the signature matches
        // If it doesnt then lets just skip and go to the next
        for(u32 iii = 0; iii < sig_len; iii++){
          u8 cur_byte = scan_cache[ii + iii];
          u8 sig_byte = sig[iii];

          if(sig_byte != 0x00 && cur_byte != sig_byte)
            goto next_scan;
        }

        // Match found
        found_ptr = section_ptr + i + ii;
        break;

        next_scan:
          continue;
      }
    }

    I(free)(scan_cache);

    if(found_ptr == 0){
      DBG("[-] find_signature: could not find signature\n");
      return 0;
    }

    if(displacement){
      u8 code_sample[14];
      if(!inject->read_memory(found_ptr, &code_sample, sizeof(code_sample))){
        DBG("[-] find_signature: code sample could not be read %p\n", found_ptr);
        return 0;
      }

      if(inject->x86mode){
        hde32s h;
        hde32_disasm(code_sample, &h);

        if(h.flags & HDE32_F_ERROR || h.flags & HDE32_F_ERROR_LENGTH){
          DBG("[-] find_signature: HDE ERROR HDE32_F_ERROR || HDE32_F_ERROR_LENGTH");
          return 0;
        }

        if(h.flags & HDE32_F_IMM32 && h.imm.imm32 != 0)
          found_ptr += (u32)h.imm.imm32 + h.len;
        else if(h.flags & HDE32_F_IMM16 && h.imm.imm16 != 0)
          found_ptr += (u32)h.imm.imm16 + h.len;
        else if(h.flags & HDE32_F_IMM8 && h.imm.imm8 != 0)
          found_ptr += (u32)h.imm.imm8 + h.len;
        else{
          DBG("[-] find_signature: no imm displacement found");
          return 0;
        }
      }
      else{
        hde64s h;
        hde64_disasm(code_sample, &h);

        if(h.flags & HDE64_F_ERROR || h.flags & HDE64_F_ERROR_LENGTH){
          DBG("[-] find_signature: HDE ERROR HDE64_F_ERROR || HDE64_F_ERROR_LENGTH");
          return 0;
        }

        // Support for disp32 on 64bit
        if(h.flags & HDE64_F_DISP32)
          found_ptr += h.disp.disp32 + h.len;
        else if(h.flags & HDE64_F_IMM32 && h.imm.imm32 != 0)
          found_ptr += (u64)h.imm.imm32 + h.len;
        else if(h.flags & HDE64_F_IMM16 && h.imm.imm16 != 0)
          found_ptr += (u64)h.imm.imm16 + h.len;
        else if(h.flags & HDE64_F_IMM8 && h.imm.imm8 != 0)
          found_ptr += (u64)h.imm.imm8 + h.len;
        else{
          DBG("[-] find_signature: no imm displacement found\n");
          return 0;
        }
      }
    }

    if(offset > 0)
      found_ptr += offset;

    //found_ptr -= module_address;

    DBG("[+] find_signature: found pointer %p\n", found_ptr );

    return inject->x86mode ? (u32)found_ptr : (u64)found_ptr;
  }

  NEVERINLINE u64 find_signature(u32 module_hash, u32 section, u8* sig, u32 sig_len, bool displacement = false, i32 offset = 0){
    s_gmc_entry* gmc_e  = gmc->find_entry(HASH("find_signature"), FNV1A_RT(sig, sig_len));
    u64 ret             = gmc_e != nullptr ? gmc_e->ptr : find_signature_internal(module_hash, section, sig, sig_len, displacement, offset);

    if(ret == 0){
      DBG("[-] Failed find_signature, trying again for 5 seconds.\n");
      for(u32 i = 0; i < XOR32(50); i++){
        ret = find_signature_internal(module_hash, section, sig, sig_len, displacement, offset);
        if(ret != 0)
          break;

#if defined(DEV_MODE)
        if(bypass_pkg_errors)
          break;
#endif

        I(Sleep)(XOR32(100));
      }
    }

    if(gmc_e == nullptr && ret != 0)
      gmc->add_entry(HASH("find_signature"), FNV1A_RT(sig, sig_len), ret);

    if(ret == 0){
      DBG("[-] find_signature: signature scan failed, dumping signature below:\n\n");

#if defined(DEV_MODE)
      // Dump the signature to string so we can see whats failing
      for(u32 i = 0; i < sig_len; i++)
        DBG("\\x%02X", sig[i]);

      DBG("\n\n");
#endif

      std::string sig_str = XOR("Failed to fetch required memory.\n\nThe cheat may require an update or there's an anti-virus blocking this process.\n\nInfo:\n");
      i32 xor_key = XOR32(58901);
      for(u32 i = 0; i < sig_len; i++){
        i8 buf[128];
        #if !defined(DEV_MODE)
        I(wsprintfA)(buf, XOR(".%02X"), sig[i] ^ xor_key); // Protect what we're trying to hook in the first place.
        #else
         I(wsprintfA)(buf, XOR("\\x%02X"), sig[i]);
        #endif
        sig_str += buf;
      }

      // As for the encrypted signature I will make a nodejs script to decrypt it for us. -Rud
      // Ask for me it if its not in the rijin code base.
      if(bypass_pkg_errors)
        return 0;

      if(is_cheat_module){
        utils::error(XOR32(ERR_SIGSCAN_FAILED), XOR32(__LINE__), sig_str, true);
      }
      else{
        SHOW_ERROR_STR(ERR_VAC_BYPASS_SIG_FAIL, "Failed to fetch required memory.\n\nSteam agent may be outdated.", true);
      }

      return 0;
    }

    return ret;
  }

  NEVERINLINE bool add_signature(u32 identifier_hash, u8* sig, u32 sig_len, bool displacement = false, u32 offset = 0){
    if(inject->input->get_pkg_by_hash(identifier_hash) != nullptr)
      return false;

    u64 ret = find_signature(target_module_hash, target_section_hash, sig, sig_len, displacement, offset);

    if(ret == 0)
      return false;

    add_ptr(identifier_hash, ret);

    return true;
  }

  NEVERINLINE bool add_hook(u32 identifier_hash, u8* sig, u32 sig_len, bool displacement = false, u32 offset = 0){
    if(inject->input->get_pkg_by_hash(identifier_hash) != nullptr)
     return false;

    u64 module_ptr = 0;
    for(u32 i = 1; i <= XOR32(100); i++){
      module_ptr = inject->get_module_address(target_module_hash);
      if(module_ptr != 0)
        break;
  
      I(Sleep)(XOR32(100));
    }
    
    if(module_ptr == 0){
      if(bypass_pkg_errors)
        return false;

      DBG("[-] c_procedure_base::add_hook sig: module_ptr == 0\n");
      SHOW_ERROR(ERR_PROECURE_ADD_HOOK_FAILED_NO_MODULE_ADDR, true);
      return false;
    }

    u64 sig_ptr = find_signature(target_module_hash, target_section_hash, sig, sig_len, displacement, offset);

    if(sig_ptr == 0){
      if(bypass_pkg_errors)
        return false;

      DBG("[-] c_procedure_base::add_hook sig: sig_ptr == 0\n");
      SHOW_ERROR(ERR_PROECURE_ADD_HOOK_FAILED_SIG_FAIL, true);
      return false;
    }

    if(!inject->add_hook(this, module_ptr, identifier_hash, sig_ptr, false)){
      DBG("[-] c_procedure_base::add_hook sig: failed\n");
      SHOW_ERROR(ERR_INJECT_ADD_HOOK_FAILED, true);
      return false;
    }

    return true;
  }

  NEVERINLINE bool add_hook(u32 identifier_hash, u64 ptr){
    if(inject->input->get_pkg_by_hash(identifier_hash) != nullptr)
      return false;

    u64 module_ptr = 0;
    for(u32 i = 1; i <= XOR32(100); i++){
      module_ptr = inject->get_module_address(target_module_hash);
      if(module_ptr != 0)
        break;
  
      I(Sleep)(XOR32(100));
    }

    if(module_ptr == 0){
      if(bypass_pkg_errors)
        return false;

      DBG("[-] c_procedure_base::add_hook: module_ptr == 0\n");
      SHOW_ERROR(ERR_PROECURE_ADD_HOOK_FAILED_NO_MODULE_ADDR, true);
      return false;
    }

    if(ptr == 0){
      if(bypass_pkg_errors)
        return false;

      DBG("[-] c_procedure_base::add_hook: ptr == 0\n");
      SHOW_ERROR(ERR_PROECURE_ADD_HOOK_FAILED_SIG_FAIL, true);
      return false;
    }

    if(!inject->add_hook(this, module_ptr, identifier_hash, ptr, true)){
      DBG("[-] inject->add_hook: failed\n");
      SHOW_ERROR(ERR_INJECT_ADD_HOOK_FAILED, true);
      return false;
    }

    return true;
  }
};

// Include all the load procedures you require
#include "c_procedure_tf2.h"
#include "c_procedure_tf2_lite.h"
#include "c_procedure_tf2_win64.h"
#include "c_procedure_l4d2.h"
#include "c_procedure_dods.h"
#include "c_procedure_css.h"
#include "c_procedure_fof.h"
#include "c_procedure_hl2dm.h"
#include "c_procedure_hl2.h"
#include "c_procedure_syn.h"
#include "c_procedure_hlsdm.h"
#include "c_procedure_codename_cure.h"
#include "c_procedure_garrysmod64.h"
#include "c_procedure_garrysmod86.h"
#include "c_procedure_dab.h"
#include "c_procedure_overwatch.h"
#include "c_procedure_overwatch2.h"
#include "c_procedure_overwatch2_external.h"
#include "c_procedure_vac_bypass.h"
#include "c_procedure_gfxtest_dx9.h"
#include "c_procedure_gfxtest_dx11.h"
#include "c_procedure_bg3.h"
#include "c_procedure_clean_steam.h"