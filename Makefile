XDIR:=/u/cs452/public/xdev
ARCH=cortex-a72
TRIPLE=aarch64-none-elf
XBINDIR:=$(XDIR)/bin
CC:=$(XBINDIR)/$(TRIPLE)-g++
OBJCOPY:=$(XBINDIR)/$(TRIPLE)-objcopy
OBJDUMP:=$(XBINDIR)/$(TRIPLE)-objdump

INC_DIR=include

ifeq ($(TRACK), none)
TRACK_FLAGS=-DNO_TRACK
else
TRACK_FLAGS=-DTRACK
endif

ifneq ($(PERFORMANCE_MEASUREMENT), on)
PERFORMANCE_FLAGS=-O3
else
PERFORMANCE_FLAGS=-DPERFORMANCE_MEASUREMENT

ifeq ($(OPTIMIZATION), on)
PERFORMANCE_FLAGS+=-O3
endif

ifeq ($(CACHE), both)
PERFORMANCE_FLAGS+=-DBCACHE
else ifeq ($(CACHE), icache)
PERFORMANCE_FLAGS+=-DICACHE
else ifeq ($(CACHE), dcache)
PERFORMANCE_FLAGS+=-DDCACHE
endif

ifeq ($(FIRST), sender)
PERFORMANCE_FLAGS+=-DSENDER
else ifeq ($(FIRST), receiver)
PERFORMANCE_FLAGS+=-DRECEIVER
endif

ifeq ($(MESSAGE_SIZE), 4)
PERFORMANCE_FLAGS+=-DMESSAGE_SIZE_4
else ifeq ($(MESSAGE_SIZE), 64)
PERFORMANCE_FLAGS+=-DMESSAGE_SIZE_64
else ifeq ($(MESSAGE_SIZE), 256)
PERFORMANCE_FLAGS+=-DMESSAGE_SIZE_256
endif

endif

# COMPILE OPTIONS
WARNINGS=-Wall -Wextra -Wpedantic -Wno-unused-const-variable
CFLAGS:=-g -pipe -static $(TRACK_FLAGS) $(PERFORMANCE_FLAGS) $(WARNINGS) -ffreestanding -nostartfiles\
	-mcpu=$(ARCH) -static-pie -mstrict-align -fno-builtin -mgeneral-regs-only -fno-exceptions -fno-rtti -std=c++23 -I$(INC_DIR)

# -Wl,option tells g++ to pass 'option' to the linker with commas replaced by spaces
# doing this rather than calling the linker ourselves simplifies the compilation procedure
LDFLAGS:=-Wl,-nmagic -Wl,-Tlinker.ld -Wl,--no-warn-rwx-segments

SRC_DIR = src
OBJ_DIR = build

# Source files and include dirs
SOURCES := $(wildcard $(SRC_DIR)/*.cc) $(wildcard $(SRC_DIR)/*.S)
# Create .o and .d files for every .cc and .S (hand-written assembly) file
OBJECTS := $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(patsubst $(SRC_DIR)/%.S, $(OBJ_DIR)/%.o, $(SOURCES)))
DEPENDS := $(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.d, $(patsubst $(SRC_DIR)/%.S, $(OBJ_DIR)/%.d, $(SOURCES)))

# The first rule is the default, ie. "make", "make all" and "make kernel.img" mean the same
all: kernel.img

clean:
	rm -f $(OBJECTS) $(DEPENDS) kernel.elf kernel.img
	rm -rf $(OBJ_DIR)

kernel.img: kernel.elf
	$(OBJCOPY) $< -O binary $@

kernel.elf: $(OBJECTS) linker.ld
	$(CC) $(CFLAGS) $(filter-out %.ld, $^) -o $@ $(LDFLAGS)
	@$(OBJDUMP) -d kernel.elf | grep -Fq q0 && printf "\n***** WARNING: SIMD INSTRUCTIONS DETECTED! *****\n\n" || true

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc Makefile
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.S Makefile
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)
