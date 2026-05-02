#include "link.h"
#include <tlhelp32.h>
#include <memoryapi.h>

u32 get_process_pid_by_name(std::string name){
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if(Process32First(snapshot, &entry))
    while(Process32Next(snapshot, &entry))
      if(stricmp(entry.szExeFile, name.c_str()) == 0)
        return entry.th32ProcessID;

  CloseHandle(snapshot);

  return 0;
}

u64 get_module_address(u32 process_id, u32 name_hash){
  void* handle = I(CreateToolhelp32Snapshot)((0x00000008) | (0x00000010), process_id);

  if(handle == nullptr)
    return 0;

  MODULEENTRY32 module_entry = MODULEENTRY32{ sizeof( MODULEENTRY32  ) };

  u64 module_address = 0;
  if(I(Module32First)(handle, &module_entry)){
    do{
      bool did_find_before_lowercase = HASH_RT(module_entry.szModule) == name_hash;

      // Lowercase module names
      if(!did_find_before_lowercase){
        for(u32 i = 0; i < I(strlen)(module_entry.szModule); i++)
          module_entry.szModule[i] = I(tolower)(module_entry.szModule[i]);
      }

      if(did_find_before_lowercase || HASH_RT(module_entry.szModule) == name_hash){
        module_address = module_entry.modBaseAddr;
        printf("[+] found module %s %p\n", module_entry.szModule, module_address);
      }

    } while(I(Module32Next)(handle, &module_entry));
  }

  I(CloseHandle)(handle);

  return module_address;
}

u64 alloc_mem(void* handle, u64 target_base, u32 len){
  if(handle == nullptr || len <= 0)
    return 0;

  while(true){
    MEMORY_BASIC_INFORMATION mbi;
    u32 ret = VirtualQueryEx(handle, target_base, &mbi, sizeof(mbi));
    if(ret == 0)
      return 0;

    if(true || mbi.State == MEM_FREE){
      u64 alloc = VirtualAllocEx(handle, target_base, len, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      if(alloc != 0){
        printf("%p ALLOC!\n", alloc);
  
        return alloc;
      }
    }

    printf("%p %p\n", mbi.BaseAddress, mbi.RegionSize);
    target_base += mbi.RegionSize;
  }

  printf("%p!!!\n", target_base);

  return 0;
}

i32 main(i32 argc, const i8 *argv[]) {

  std::string target_process = "tf_win64.exe";
  u32 target_pid = get_process_pid_by_name(target_process.c_str());
 
  if(target_pid != 0){
    printf("[+] %s: %i\n", target_process.c_str(), target_pid);

    void* handle = OpenProcess(PROCESS_ALL_ACCESS, false, target_pid);

    if(handle == nullptr){
      printf("[-] failed to open handle to %s!", target_process.c_str());
    }
    else
      printf("[+] %s handle: 0x%X\n", target_process.c_str(), target_pid);

    u64 target_base = get_module_address(target_pid, HASH("engine.dll"));
    u64 alloc_base = alloc_mem(handle, target_base, 2500);

    printf("[+] target_base: %p alloc_base: %p delta: %p\n", target_base, alloc_base, alloc_base - target_base);
  }
  else
    printf("[-] process tf_win64.exe not found\n");

  system("pause");

  return 0;
}
