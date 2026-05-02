
already_listed = []

print("Started")
def import_callback(ea, name, i):
  if name == "?EnterScope@CVProfile@@QAEXPBDH0_NH@Z":
    renamed = 0
    for x in CodeRefsTo(ea, False):
      vprof_name = str(ida_bytes.get_strlit_contents(idc.get_operand_value(x - 5, 0), -1, ida_nalt.STRTYPE_C))
      if vprof_name == "None":
        continue
        
      vprof_name = vprof_name.replace(" ","").replace("b'", "").replace(">","").replace("'","").replace("(","_").replace(")","_").replace("-","").strip()
      vprof_name = vprof_name.strip()
      print("Found " + vprof_name + " at", hex(x))

      for func in Functions(x, x - 5):
        found_count = already_listed.count(vprof_name)
        if found_count == 0:
          idc.set_name(func, vprof_name)
        else:
          idc.set_name(func, vprof_name + "_" + str(found_count))

      renamed = renamed + 1
      already_listed.append(vprof_name)
  elif name == "g_Telemetry":
    print("Cool",hex(ea))
    for x in CodeRefsTo(ea + 1, False):
      print("Ref to g_Telemetry:", hex(x))
      dbg_name = idc.find_binary(x, SEARCH_DOWN, "68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ?", 16, False)
      if dbg_name == idc.BADADDR:
        continue

      print("Got a good one here!")
      print("Here: ", hex(x))


  return True

while True:

  # Check through every import for "EnterScope" VPROF shit the source engine uses.
  import_amount = idaapi.get_import_module_qty()
  for i in range(0, import_amount):
    cur = idaapi.get_import_module_name(i)
    idaapi.enum_import_names(i, import_callback)
 

  break
print("Found " + str(len(already_listed)) + " functions")