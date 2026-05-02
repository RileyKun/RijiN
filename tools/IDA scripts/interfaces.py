
base = idaapi.get_imagebase() + 1024

cur = idc.find_binary(base, SEARCH_DOWN | SEARCH_NEXT, "55 8B EC 83 EC 08 E8 ? ? ? ? D9 EE 6A 01 6A 01 6A 01 6A 01 6A 02 83 EC 0C D9 5C 24 08 D9 05 ? ? ? ? D9 54 24 04 D9 1C 24 E8 ? ? ? ? 6A 78", 16)
if cur == idc.BADADDR:
   cur = idc.find_binary(base, SEARCH_DOWN | SEARCH_NEXT, "55 8B EC 81 EC ? ? ? ? 8D 8D ? ? ? ? 53", 16)

if cur == idc.BADADDR:
  print("Failed to find main interface init function")

idc.set_name(cur, "CHLClient::Init") # This is the function we start in
print("Finding interfaces...")
while True:
   
   # This is where we get the name of the interface.
   name_base = idc.find_binary(cur, SEARCH_DOWN, "68 ? ? ? ? FF 55 08", 16)
   if name_base == idc.BADADDR:
     break

   # Make interface name more ida name friendly.
   interface_name = str(ida_bytes.get_strlit_contents(idc.get_operand_value(name_base, 0), -1, ida_nalt.STRTYPE_C))
   interface_name = interface_name.replace("b'", "").replace("'","").strip()
   interface_name = interface_name.strip(" ")

   # Print to the console of the interface we found along with the address.
   print("Found " + interface_name + " at", hex(name_base))

   addr = idc.find_binary(name_base, SEARCH_DOWN, "A3 ? ? ? ? 85 C0", 16)
   if addr == idc.BADADDR:
    break

   cur = name_base + 1
   idc.set_name(idc.get_operand_value(addr, 0), interface_name)

print("Finished getting interfaces")