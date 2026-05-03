#pragma once

typedef LONG(NTAPI* nt_suspend_process_fn)(HANDLE handle);
typedef LONG(NTAPI* nt_resume_process_fn)(HANDLE handle);

class c_procedure_overwatch2 : public c_procedure_base{
public:

  bool reinject_text_section(std::string target_module){
    // If we havent already, load the library
    I(LoadLibraryA)(target_module.c_str());

    void* module = utils::get_module_handle(HASH_RT(target_module.c_str()));

    if(module == nullptr){
      SHOW_ERROR(ERR_BYFRON_BYPASS_REINJECT_TARGET_MODULE_NOT_FOUND, true);
      return false;
    }

    // Check magic
    if(((IMAGE_DOS_HEADER*)module)->e_magic != XOR32(0x5A4D)){
      SHOW_ERROR(ERR_BYFRON_BYPASS_REINJECT_TARGET_MODULE_INVALID_PE_HEADER, true);
      return false;
    }

    // Get headers
    IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(module + ((IMAGE_DOS_HEADER*)module)->e_lfanew);

    PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
    for(u32 i = 0; i < nt_header->FileHeader.NumberOfSections; i++){
      if(HASH_RT(section->Name) != XOR32(0x88A82EC2)/*.text*/){
        section++;
        continue;
      }

      u32 old_protect = 0;
      u32 status = I(VirtualProtectEx)(inject->process, module + section->VirtualAddress, section->SizeOfRawData, XOR32(PAGE_EXECUTE_READWRITE), &old_protect);

      // Now write our text section to the target process text section, since ASLR is a thing this will work nicely
      bool ok = inject->write_memory(module + section->VirtualAddress, module + section->VirtualAddress, section->SizeOfRawData);

      I(VirtualProtectEx)(inject->process, module + section->VirtualAddress, section->SizeOfRawData, XOR32(PAGE_EXECUTE), &old_protect);

      if(!ok){
        SHOW_ERROR(ERR_BYFRON_BYPASS_REINJECT_MEM_FAILED, true);
        return false;
      }

      DBG("[+] [BYFRON] reinject_text_section: %s\t%-8s\t%p\t%p\n", target_module.c_str(), section->Name, module + section->VirtualAddress, section->SizeOfRawData);
      return true;
    }

    return false;
  };

  bool setup() override{
    if(global->dxgi_offset_data == nullptr)
      return false;

    // This module loads when the game is connecting to game servers
    set_module(HASH("schannel.dll"));

    // Bypass Byfron integrity checks
    {
      DBG("[!] [BYFRON] Bypassing Integrity checks...\n");

      // 1. Suspend the process so byfron threads show themselves
      void* suspend_process_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtSuspendProcess"));
      assert(suspend_process_ptr != nullptr);

      utils::call_stdcall<void*, void*>(suspend_process_ptr, inject->process);

      // 2. Iterate through threads, attempt to suspend the thread again, if the suspend count is zero the thread is using 0x40 trick
      // Individually suspend the thread and resume all the others
      {
        HANDLE thread_snapshop = I(CreateToolhelp32Snapshot)(TH32CS_SNAPTHREAD, 0);

        THREADENTRY32 thread_entry;
        thread_entry.dwSize = XOR32(sizeof(THREADENTRY32));

        I(Thread32First)(thread_snapshop, &thread_entry);

        while(I(Thread32Next)(thread_snapshop, &thread_entry)){
          if(thread_entry.th32OwnerProcessID != inject->process_id)
            continue;

          // Open the thread
          HANDLE thread = I(OpenThread)(THREAD_ALL_ACCESS, FALSE, thread_entry.th32ThreadID);

          i32 suspend_count = I(SuspendThread)(thread);

          if(suspend_count == 0){
            u32 exit_code = 0;
            I(GetExitCodeThread)(thread, &exit_code);

            I(TerminateThread)(thread, exit_code);
            DBG("[+] [BYFRON] Byfron Integrity check disabled (#%i)\n", thread_entry.th32ThreadID);
          }
          else
            I(ResumeThread)(thread);

          I(CloseHandle)(thread);
        }

        I(CloseHandle)(thread_snapshop);
      }

      // 1. Resume the process
      void* resume_process_ptr = utils::get_proc_address(utils::get_module_handle(HASH("ntdll.dll")), HASH("NtResumeProcess"));
      assert(resume_process_ptr != nullptr);

      utils::call_stdcall<void*, void*>(resume_process_ptr, inject->process);

      DBG("[!] [BYFRON] Bypassed Integrity checks...\n");
    }

    // Bypass byfron .text section hooks
    {
      DBG("[!] [BYFRON] Reinjecting text sections...\n");

      reinject_text_section(XOR("kernel32.dll"));
      reinject_text_section(XOR("kernelbase.dll"));
      reinject_text_section(XOR("user32.dll"));
      reinject_text_section(XOR("ntdll.dll"));
      reinject_text_section(XOR("gdi32.dll"));

      DBG("[!] [BYFRON] Reinjected text sections...\n");
    }

    return true;
  }

  bool write_signatures() override{
    set_section(HASH(".text"));

    set_module(HASH("overwatch.exe"));

    return true;
  }

  bool write_pointers() override{
    return true;
  }

  bool write_hooks() override{
    set_section(HASH(".text"));

    // Wait for game to fully load
    //set_module(HASH("webio.dll"));

    //set_module(HASH("dxgi.dll"));
    //{
    //  add_hook(HASH("d3d11_hook"), global->dxgi_offset_data->d3d11_present);
    //}

    set_module(HASH("gdi32.dll"));
    {
      u64 module = inject->get_module_address(HASH("gdi32.dll"));
      u64 export_ptr = inject->get_export(module, HASH("GdiEntry13"));

      add_hook(HASH("gdi_entry_hook"), export_ptr - module);
    }

    set_module(HASH("ntdll.dll"));
    {
      u64 module = inject->get_module_address(HASH("ntdll.dll"));
      u64 export_ptr = inject->get_export(module, HASH("KiUserExceptionDispatcher"));

      void* _asm = malloc(XOR32(256));
      inject->read_memory(export_ptr, _asm, XOR32(256));

      for(u64 i = _asm;; i < XOR32(256)){
        hde64s h;
        hde64_disasm(i, &h);

        if(h.opcode == XOR32(0xE8) && (h.flags & HDE64_F_IMM32)){
          add_hook(HASH("rtl_dispatch_exception_hook"), (export_ptr + (i - (u64)_asm) + (i32)h.imm.imm32 + h.len) - module);
          break;
        }

        DBG("%i\n", h.len);

        i += h.len;
      }

      free(_asm);
    }

    return true;
  }
};