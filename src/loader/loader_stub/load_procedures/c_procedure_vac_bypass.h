#pragma once

class c_procedure_vac_bypass : public c_procedure_base{
public:

  bool setup() override{
    return true;
  }

  bool write_signatures() override{
    set_section(HASH(".text"));

    return true;
  }

  bool write_pointers() override{
    return true;
  }

  bool write_hooks() override{
    set_section(HASH(".text"));

    is_cheat_module = false;
    //set_module(HASH("crashhandler.dll"));
    {
      //add_hook(HASH("dll_main_hook"), SIG("\x33\xC0\x40\xC2\x0C\x00"));
    }

    set_module(HASH("kernel32.dll"));
    {

      uptr gsi = get_export_ptr(HASH("GetSystemInfo"));
      u8 asm_dmp[7];

      if(!inject->read_memory(gsi, &asm_dmp, 7)){
        SHOW_ERROR(ERR_VACDMPFL1, true);
        return false;
      }

      hde64s h;
      hde64_disasm(asm_dmp, &h);

      if(h.opcode == 0xFF && h.modrm == 0x25){
        gsi = gsi + h.len + *(u64*)(&h.imm);
        DBG("YEEEE %p", gsi);
        system("pause");
      }


      add_hook(HASH("get_system_info_hook"), gsi);
    }

    return true;
  }
};