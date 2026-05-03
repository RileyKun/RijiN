#pragma once

class c_procedure_overwatch2_external : public c_procedure_base{
public:

  bool setup() override{
    DBG("[!] Waiting for overwatch.exe...\n");

    u32 pid = 0;
    while((pid = utils::get_pid(HASH("Overwatch.exe"))) == 0)
      I(Sleep)(XOR32(1000));

    void* remote_handle = nullptr;
    void* ow_handle     = I(OpenProcess)(XOR32(PROCESS_ALL_ACCESS), 0, pid);
    if(!I(DuplicateHandle)(utils::get_current_process(), ow_handle, inject->process, &remote_handle, 0, false, XOR32(DUPLICATE_SAME_ACCESS))){
      SHOW_ERROR_STR(ERR_DUPLICATE_HANDLE_FAILED, "Please try disabling your anti virus.", true);
      return false;
    }
    I(CloseHandle)(ow_handle);

    inject->input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), HASH("ow_handle"), 0, remote_handle);
    DBG("[+] Shared overwatch handle to DWM (%i)\n", remote_handle);

    // Get and share the overwatch.exe base address with the cheat
    {
      // Hack: swap process id so we can get module address of ow
      u32 old_pid         = inject->process_id;
      inject->process_id  = pid;
      u64 ow_address      = inject->get_module_address(HASH("overwatch.exe"));
      inject->process_id  = old_pid;

      if(ow_address == 0){
        SHOW_ERROR_STR(ERR_OW_MODULE_ADDRESS_FAILED, "Please try disabling your anti virus.", true);
        return false;
      }

      inject->input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), HASH("ow_base"), 0, ow_address);
      DBG("[+] Shared overwatch base to DWM (%p)\n", ow_address);
    }

    // send PID
    inject->input->add_pkg(XOR32(GEN_PKG_TYPE_PTR), HASH("ow_pid"), 0, pid);

    return true;
  }

  bool write_signatures() override{
    set_section(HASH(".text"));

    i32 get_ubr = utils::get_ubr();

    set_module(HASH("dwmcore.dll"));
    {
      // __int64 __fastcall CD2DContext::FlushDrawList(CD2DContext *this)
      if(WINVER >= 22621)
        add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF1\x48\x8D\x54\x24"));
      else if(WINVER >= 22000)
        add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8B\x89\x00\x00\x00\x00\x48\x85\xC9"));
      //else if(WINVER == 19041 && get_ubr != 1415)
        //add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x44\x24\x00\x48\x8B\x91"));
      else
      {
        bypass_pkg_errors = true;
        add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xF1\x48\x8D\x54\x24"), false, 0); // 19044.2006
        add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x91\x00\x00\x00\x00\x33\xFF\x48\x8B\xD9"), false, 0); // 17763.2928
        add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\xD9\x48\x8B\x89\x00\x00\x00\x00\x48\x85\xC9"), false, 0);
        add_signature(HASH("flush_draw_list"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x44\x24\x00\x48\x8B\x91"), false, 0);
        bypass_pkg_errors = false;

        if(inject->input->get_pkg_by_hash(HASH("flush_draw_list")) == nullptr){
          SHOW_ERROR_STR(ERR_SIGSCAN_FAILED, "Failed to fetch required memory (1).\n\nPlease contact the developers notifying them of this issue, please include your OS build date and time will be automatically compensated.", true);
          return false;
        }
      }
    }

    return true;
  }

  bool write_pointers() override{
    return true;
  }

  // 19042.2130 > WORKS

  bool write_hooks() override{
    set_section(HASH(".text"));

    i32 get_ubr = utils::get_ubr();

    set_module(HASH("dwmcore.dll"));
    {
      // __int64 __fastcall CWindowNode::RenderContent(CWindowNode *this, struct _LIST_ENTRY **a2, bool *a3)
      if(WINVER >= 22621)
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x45\x00\xF6\x41"));
      else if(WINVER >= 22000)
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x83\xEC\x00\x45\x33\xED\x4D\x8B\xE0"));
      //else if(WINVER == 19041 && get_ubr != 1415)
        //add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x44\x24\x00\x45\x33\xE4"));
      else
      {
        bypass_pkg_errors = true;
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x83\xEC\x00\x45\x33\xE4\x4D\x8B\xE8"), false, 0); // 19044.2006
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\x00\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x45\x00\xF6\x41"), false, 0);
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x83\xEC\x00\x45\x33\xED\x4D\x8B\xE0"), false, 0);
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x44\x24\x00\x45\x33\xE4"), false, 0);
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x8B\xC4\x48\x89\x58\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xA8\x00\x00\x00\x00\x48\x81\xEC\x00\x00\x00\x00\x0F\x29\x70\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x45"), false, 0);
        add_hook(HASH("dwm_render_content_hook"), SIG("\x48\x8B\xC4\x48\x89\x58\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\xA8\x00\x00\x00\x00\x48\x81\xEC\x00\x00\x00\x00\x0F\x29\x70\x00\x48\x8B\x05\x00\x00\x00\x00\x48\x33\xC4\x48\x89\x85\x00\x00\x00\x00\x45\x33\xE4"), false, 0); // 17763.2928
        bypass_pkg_errors = false;

        if(inject->input->get_pkg_by_hash(HASH("dwm_render_content_hook")) == nullptr){
          SHOW_ERROR_STR(ERR_SIGSCAN_FAILED, "Failed to fetch required memory (2).\n\nPlease contact the developers notifying them of this issue, please include your OS build date and time will be automatically compensated.", true);
          return false;
        }
      }
    }

    return true;
  }
};