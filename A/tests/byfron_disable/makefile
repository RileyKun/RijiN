COMPILER32      =g++
COMPILER64      =x86_64-w64-mingw32-g++
USE_COMPILER    =${COMPILER64}
COMPILER_FLAGS  =-O3 -std=c++20 -s -w -Wreturn-type ${MATH_EXTENSION} -Wno-attributes -Wno-enum-compare -Wno-write-strings -Wno-builtin-declaration-mismatch -fpermissive -fcheck-new -fno-exceptions -fno-rtti -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fno-ident -static
LINKER_FLAGS    =-s -lgcc -Wl,--exclude-all-symbols
OUTPUT_FLAGS    =-o $(OUTPUT_FILE)
OBJ_DIR         =obj/
OBJ_FILES       =$(wildcard $(OBJ_DIR)*.o)

# Add core and custom directories to compile list
CPP_FILES       +=$(wildcard ./*.cpp)

LINKER_FLAGS += -lWinmm

# Add git commit number
$(eval GIT_COMMIT_HASH = $(shell git rev-parse --short HEAD))
$(eval GIT_BRANCH_NAME = $(shell git rev-parse --abbrev-ref HEAD))
COMPILER_FLAGS += -DGIT_COMMIT_HASH=0x$(GIT_COMMIT_HASH)
COMPILER_FLAGS += -DGIT_BRANCH_NAME=\"$(GIT_BRANCH_NAME)\"

# Define extension name
COMPILER_FLAGS += -DMATH_EXTENSION_NAME=\"$(MATH_EXTENSION_NAME)\"

.PHONY: make_objects $(CPP_FILES)

make_objects: $(CPP_FILES)
$(CPP_FILES):
	@printf "[!] $(@F)\n"
	@$(eval count=$(shell echo $$(($(count)+1))))
	@$(USE_COMPILER) $@ -c -o $(OBJ_DIR)$(count).o $(COMPILER_FLAGS)
	
make_output: $(OBJ_FILES)
	@printf "[+] $(notdir $(OUTPUT_FILE))\n"
	@$(USE_COMPILER) $(OUTPUT_FLAGS) $(COMPILER_FLAGS) $^ $(LINKER_FLAGS)