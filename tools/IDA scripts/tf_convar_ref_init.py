import hashlib

already_listed = []

print("\n\n\nFinding TF functions...\n\n\n");

# This can be used for anything that passes strings.
# Its just you may have to change some of the offets and code sadly.
base = idaapi.get_imagebase() + 1024
attrib_managers_sigs = ["55 8B EC 56 8B F1 8B 0D ? ? ? ? 85 C9 74 ? 8B 01 FF 75"]

for sig in attrib_managers_sigs:
  count = 0
  cur_base = base
  cur = idc.find_binary(cur_base, SEARCH_DOWN | SEARCH_UP | SEARCH_NEXT, sig, 16)
  if cur != idc.BADADDR and cur != 0xffffffff:
    #print("Attrib:" + hex(cur))
    for x in CodeRefsTo(cur, False):
      str_offset = idc.find_binary(x, SEARCH_UP, "68", 16)
      #print("String ASM offset: " + hex(str_offset));
      val = idc.get_operand_value(str_offset - 3, 0);
      if val <= 1024:
        val = idc.get_operand_value(str_offset, 0);

      #print("String: " + hex(val))
      sound_name = str(ida_bytes.get_strlit_contents(val, -1, ida_nalt.STRTYPE_C))
      if sound_name == "None" or sound_name.find(".") == -1:
        val = idc.get_operand_value(str_offset - 1, 0);
        sound_name = str(ida_bytes.get_strlit_contents(val, -1, ida_nalt.STRTYPE_C))
        if sound_name == "None" or sound_name.find(".") == -1:
          val = idc.get_operand_value(str_offset, 0);
          sound_name = str(ida_bytes.get_strlit_contents(val, -1, ida_nalt.STRTYPE_C))
          if sound_name == "None":
            #print("FAIL ASM Offset: " + hex(str_offset))
            #print("FAIL val: " + hex(val))
            continue

      # Remove IDA debug text.
      sound_name = sound_name.replace("b'","").replace('"','').replace("'","").replace(",","")
      sound_name = sound_name.strip()

      print(sound_name + ": " + hex(str_offset))
      count = count + 1
      if count > 100:
        break

      function_name = "None"

      #Do not continue if the function name is None
      if function_name == "None":
        continue

      # Do not continue if the function name has been seen before
      function_name_count = already_listed.count(function_name)
      if function_name_count != 0:
        continue

      for func in Functions(x, x - 5):
        idc.set_name(func, function_name)
        print("Found: " + function_name + " at", hex(func))

      already_listed.append(function_name)


print("Finished")