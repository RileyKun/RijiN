base = idaapi.get_imagebase() + 1024

already_listed = []

netvar_init_sigs = ["55 8B EC 8B 45 ? 8B 4D ? 89 08 8B 4D ? 89 48 ? 8B 4D ? 89 48 ? 8B 4D ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C6 40 ? ? C7 40 ? ? ? ? ? 89 48 ? 5D C3 CC CC CC CC CC 55 8B EC 8B 45 ? 8B 4D","55 8B EC 8B 45 ? 8B 4D ? 89 08 8B 4D ? 89 48 ? 8B 4D ? 89 48 ? 8B 4D ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C7 40 ? ? ? ? ? C6 40 ? ? C7 40 ? ? ? ? ? 89 48 ? 5D C3 CC CC CC CC CC 55 8B EC 8B 45 ? 8B 55","55 8B EC FF 75 ? 6A ? FF 75 ? FF 75","55 8B EC 6A ? 6A ? FF 75 ? FF 75 ? FF 75 ? FF 75 ? E8 ? ? ? ? 8B 45","55 8B EC 8B 45 ? 8B 55 ? C7 40","55 8B EC 8B 55 ? 8B 4D ? 8B 45 ? 89 4A ? 8B 4D ? 89 4A ? 8B 4D ? 89 4A ? 8B 4D ? 89 02 8B C2 C7 42 ? ? ? ? ? C7 42 ? ? ? ? ? C7 42 ? ? ? ? ? C7 42 ? ? ? ? ? C7 42 ? ? ? ? ? C7 42 ? ? ? ? ? C6 42"]

print("Started")

for sig in netvar_init_sigs:
  cur = idc.find_binary(base, SEARCH_DOWN | SEARCH_UP | SEARCH_NEXT, sig)
  if cur == idc.BADADDR or cur == 0xffffffff:
    continue

  print("Found at " + hex(cur))
  for x in CodeRefsTo(cur, False):
    offset      = hex(idc.get_operand_value(x - 15, 0))
    netvar_name = str(ida_bytes.get_strlit_contents(idc.get_operand_value(x - 10, 0), -1, ida_nalt.STRTYPE_C))
    netvar_name = netvar_name.replace('"',"").replace(" ","").replace("b'", "").replace(">","").replace("'","").replace("(","_").replace(")","_").replace("-","").strip()
    netvar_name = netvar_name.strip();
    if netvar_name == "baseclass" or netvar_name == "None" or netvar_name == "":
      continue

    found_count = already_listed.count(netvar_name)
    if found_count == 0:
      idc.set_name(idc.get_operand_value(x - 5, 0), netvar_name + "_prop")
    else:
      idc.set_name(idc.get_operand_value(x - 5, 0), netvar_name + "_prop_" + str(found_count))

    print(hex(x - 5) + " to " + netvar_name + " " + offset);
    already_listed.append(netvar_name)
