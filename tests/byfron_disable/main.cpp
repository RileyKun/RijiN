#include "link.h"
#include <dbghelp.h>
#include <stdio.h>
#include "fnv1a.h"

DWORD get_process_id(LPCTSTR ProcessName){
    PROCESSENTRY32 pt;
    HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    pt.dwSize = sizeof(PROCESSENTRY32);
    if(Process32First(hsnap, &pt)){
      do{
        if(!lstrcmpi(pt.szExeFile, ProcessName)){
            CloseHandle(hsnap);
            return pt.th32ProcessID;
        }
      }while(Process32Next(hsnap, &pt));
    }
    CloseHandle(hsnap);
    return 0;
}

bool reload_text_section(HANDLE* handle, std::string target_module){
  // If we havent already, load the library
  LoadLibraryA(target_module.c_str());

  void* module = GetModuleHandleA(target_module.c_str());

  // Check magic
  if(((IMAGE_DOS_HEADER*)module)->e_magic != 0x5A4D)
    return false;

  // Get headers
  IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)(module + ((IMAGE_DOS_HEADER*)module)->e_lfanew);

  PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(nt_header);
  for(u32 i = 0; i < nt_header->FileHeader.NumberOfSections; i++){
    if(HASH_RT(section->Name) != 0x88A82EC2/*.text*/)
      continue;

    u32 old_protect = 0;
    u32 status = VirtualProtectEx(handle, module + section->VirtualAddress, section->SizeOfRawData, PAGE_EXECUTE_READWRITE, &old_protect);

    // Now write our text section to the target process text section, since ASLR is a thing this will work nicely
    if(WriteProcessMemory(handle, module + section->VirtualAddress, module + section->VirtualAddress, section->SizeOfRawData, nullptr))
      printf("[+] %s\t%-8s\t%p\t%p\n", target_module.c_str(), section->Name, module + section->VirtualAddress, section->SizeOfRawData);
    else
      printf("[-] Failed to patch %s\n", target_module.c_str());

    VirtualProtectEx(handle, module + section->VirtualAddress, section->SizeOfRawData, old_protect, &old_protect);

    section++;
  }

  return false;
};

typedef LONG(NTAPI* nt_suspend_process_fn)(HANDLE handle);
typedef LONG(NTAPI* nt_resume_process_fn)(HANDLE handle);

i32 main(i32 argc, const i8 *argv[]){
  u32 pid = get_process_id("Overwatch.exe");

  HANDLE* handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

  if(handle == nullptr){
    printf("[-] cant open process\n");
    Sleep(1000);
    return 0;
  }

  // 1. Suspend the process so byfron threads show themselves
  nt_suspend_process_fn nt_suspend_process = (nt_suspend_process_fn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSuspendProcess");
  nt_suspend_process(handle);

  // 2. Iterate through threads, attempt to suspend the thread again, if the suspend count is zero the thread is using 0x40 trick
  // Individually suspend the thread and resume all the others
  {
    HANDLE thread_snapshop = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    THREADENTRY32 thread_entry;
    thread_entry.dwSize = sizeof(THREADENTRY32);

    Thread32First(thread_snapshop, &thread_entry);

    while (Thread32Next(thread_snapshop, &thread_entry)){
      if(thread_entry.th32OwnerProcessID != pid)
        continue;

      // Open the thread
      HANDLE thread = OpenThread(THREAD_ALL_ACCESS, FALSE, thread_entry.th32ThreadID);

      i32 suspend_count = SuspendThread(thread);

      if(suspend_count == 0){
        u32 exit_code;
        GetExitCodeThread(thread, &exit_code);

        TerminateThread(thread, exit_code);
        printf("[+] Byfron Integrity check disabled (#%i)\n", thread_entry.th32ThreadID);
      }
      else
        ResumeThread(thread);

      CloseHandle(thread);
    }

    CloseHandle(thread_snapshop);
  }

  // 1. Resume the process
  nt_resume_process_fn nt_resume_process = (nt_resume_process_fn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtResumeProcess");
  nt_resume_process(handle);

  reload_text_section(handle, "Kernel32.dll");
  reload_text_section(handle, "kernelbase.dll");
  reload_text_section(handle, "User32.dll");
  reload_text_section(handle, "ntdll.dll");

  CloseHandle(handle);

  printf("[+] Byfron Integrity checks disabled\n");
  Sleep(5000);

  return 0;
}
