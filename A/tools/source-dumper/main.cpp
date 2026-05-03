
#include "link.h"

#define FORMAT_TEXT "  {\n    \"name\": \"%s_%s\",\n    \"value\": %i,\n    \"table_value\": %i\n  },"

// FOR L4D1 & L4D2
// DEFINE 'L4D_GAME' in structs.h

inline void dump_data_table(recv_table* table, i32 base_offset = 0){
  if(table == nullptr)
    return;

  if(utils::is_table_processed(table->name))
    return;

  utils::processed_table.push_back(table->name);
  for(u32 i = 0; i < table->count; i++){
    recv_prop* prop = &table->props[i];
    if(prop == nullptr)
      continue;

    if(prop->type == 5)
      continue;

    if(prop->name == nullptr)
      continue;

    if(utils::is_number(prop->name))
      continue;

    if(prop->offset)
      utils::write_to_file("netvar.json", FORMAT_TEXT, utils::c_happy_str(table->name).c_str(), utils::c_happy_str(prop->name).c_str(), prop->offset, base_offset);

    // Does this data table... have a data table with in a data table?!!!
    dump_data_table(prop->data_table);
  }
}

inline bool dump_netvars(hl_client* client){
  client_class* c = client->get_all_classes();
  if(c == nullptr)
    return false;

  for(; c; c  = c->next){
    if(c->table == nullptr)
      continue;

    if(c->name == nullptr)
      continue;

    // Was this data_table already processed before?
    if(utils::is_table_processed(c->table->name))
      continue;

    // Push this into the table.
    utils::processed_table.push_back(c->table->name);

    // begin the process of dumping props.
    for(u32 i = 0; i < c->table->count; i++){
      recv_prop* prop = &c->table->props[i];
      if(prop == nullptr)
        continue;

      if(prop->type == 5)
        continue;

      if(prop->name == nullptr)
        continue;

      if(utils::is_number(prop->name))
        continue;

      if(prop->offset)
        utils::write_to_file("netvar.json", FORMAT_TEXT, utils::c_happy_str(c->table->name).c_str(), utils::c_happy_str(prop->name).c_str(), prop->offset, 0);

      // does this prop have a data_table? lets read it.
      dump_data_table(prop->data_table, prop->offset);
    }
  }

  return true;
}

inline bool dump_classid(hl_client* client){
  client_class* c = client->get_all_classes();
  if(c == nullptr)
    return false;

  bool non_zero = false;
  for(; c; c  = c->next){
    if(c->table == nullptr)
      continue;

    if(c->name == nullptr)
      continue;

    if(c->id == 0)
      continue;

    utils::write_to_file("classids.log", "%s = %i,", utils::c_happy_str(c->name).c_str(), c->id);
    non_zero = true;
  }

  return non_zero;
}

i32 __stdcall dumper(){

  void* client_dll       = nullptr;
  u32   client_dll_count = 0;
  while(client_dll == nullptr){
    client_dll = GetModuleHandleW(L"client.dll");
    client_dll_count++;
    if(client_dll_count >= 1024){
      MessageBoxW(0, L"Failed to get client.dll handle", L"Error", 0);
      ExitProcess(0);
    }
  }


  // Get the VClient interface.
  auto ci = (create_interface)GetProcAddress(client_dll, "CreateInterface");
  hl_client* client = (hl_client*)get_interface_address(ci, "VClient");
  if(client == nullptr){
    MessageBoxW(0, L"Failed to init interface \"VClient\"", L"Error", 0);
    ExitProcess(0);
  }

  utils::write_to_file("netvar.json", "[");
  if(!dump_netvars(client)){
    MessageBoxW(0, L"Failed to dump netvars!", L"Error", 0);
    return 0;
  }
  utils::write_to_file("netvar.json", "]");

  MessageBoxW(0, L"Success!\nNothing went wrong... Check the game directory for the netvar.json file and dont forget to the remove the last comma in the json table\nPress OK to start the ClassID dumping!\nPress the HOME key.", L"Success!", 0);
  while(true){

    if(!GetAsyncKeyState(VK_HOME)){
      Sleep(1);
      continue;
    }

    if(dump_classid(client)){
      MessageBoxW(0, L"Success!\nClassids were dumped!", L"Success", 0);
      break;
    }
    else
      MessageBoxW(0, L"Failed to properly dump classids!", L"Error", 0);
  }
  return 0;
}

bool __stdcall DllMain(void* base_addr, u32 reason, void*){
  if(reason != DLL_PROCESS_ATTACH)
    return false;

  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)dumper, 0, 0, 0);
  return true;
}