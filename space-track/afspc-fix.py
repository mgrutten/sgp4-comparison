
import re
import sys

if len(sys.argv) != 2:
	raise NameError('Must have 2 arguments')

file = open(sys.argv[1],"r")

print("#pragma once")
print("#include <stdint.h>")

for line in file:
  line = line.strip()
  if len(line) > 0:
    if "STDCALL" in line:
      #print(line)
      line = re.sub(r'typedef (\w*) \(STDCALL \*fnPtr(\w*)\)',r'\1 \2',line)
      line = re.sub(r'__int64',r'int64_t',line)
      #print(line)
    #elif "DLL_H" in line:
    #  pass
    #elif "#endif" in line:
    #  pass
    elif line[0] != "/":
      line = "// " + line
  print(line)