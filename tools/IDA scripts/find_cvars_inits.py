
base = idaapi.get_imagebase() + 1024

found_cvars  = []
found_cvars2 = []

# Signatures to the cvar init function.
sigs = ["68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 C3","68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 C3","68 ? ? ? ? 6A ? 6A ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 C3","68 ? ? ? ? 68 ? ? ? ? 6A ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 C3","68 ? ? ? ? 6A ? 68 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 C3","68 ? ? ? ? 6A ? 68 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 59 C3"]

print("Finding cvars init")

index = 0
while True:

  if index >= len(sigs):
    print("Stopping because " + str(index) + "/" + str(len(sigs)))
    break

  cur = idc.find_binary(base, SEARCH_DOWN | SEARCH_NEXT, sigs[index], 16)
  if cur == idc.BADADDR:
    base = idaapi.get_imagebase() + 1024
    index = index + 1
    continue


  # Find the next one.
  base = cur + 1

  name_location = idc.find_binary(base, SEARCH_DOWN | SEARCH_DOWN, "68 ? ? ? ? B9 ? ? ? ?", 16)
  if name_location == idc.BADADDR:
    print("Failed name location on " + str(index))
    break

  cvar_name = str(ida_bytes.get_strlit_contents(idc.get_operand_value(name_location, 0), -1, ida_nalt.STRTYPE_C))
  cvar_name = cvar_name.replace("b'", "").replace("'","").replace("+","").replace("-","").replace(" ","").strip()
  cvar_name = cvar_name.strip(" ")

  # Name the init function of the cvar.
  ref_count = found_cvars.count(cvar_name)
  if ref_count == 0:
    idc.set_name(cur, cvar_name + "_init")
  else:
    idc.set_name(cur, cvar_name + "_init" + str(ref_count))

  found_cvars.append(cvar_name);

  # Lets find the pointer of the cvar.
  cc = idc.find_binary(base, SEARCH_DOWN | SEARCH_NEXT, "B9 ? ? ? ?", 16);
  if cc == idc.BADADDR:
    continue;


  # Name the pointer of the cvar.
  cc_addr    = idc.get_operand_value(cc, 1) + 0x1C;
  ref_count2 = found_cvars2.count(cvar_name)
  if ref_count2 <= 1:
    idc.set_name(cc_addr, cvar_name)
  else:
    idc.set_name(cc_addr, cvar_name + str(ref_count2))

  if ref_count2 <= 1:
    idc.set_name(cc, cvar_name + "_ref")
  else:
    idc.set_name(cc, cvar_name + "_ref_" + str(ref_count2))

  print(cvar_name + " -> " + str(hex(cc_addr)));
  found_cvars2.append(cvar_name);


print("Finished! Found over " + str(len(found_cvars)) + " cvars init functions")