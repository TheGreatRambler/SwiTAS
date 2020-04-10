UNAME := $(shell uname -o)

ifeq ($(UNAME),Msys)
	TARGET_EXEC ?= nxtas.exe
else
	TARGET_EXEC ?= nxtas
endif

BUILD_DIR ?= ./bin

SRC_DIRS ?= ./source ../sharedNetworkCode

ifeq ($(UNAME),Msys)
# Set compilers to MinGW64 compilers
CC := x86_64-w64-mingw32-gcc
CXX := x86_64-w64-mingw32-g++
else
CC := gcc
CXX := g++
endif

# C flags
CFLAGS := -std=gnu17

# C++ flags
CXXFLAGS := -std=gnu++17 $(shell wx-config --cxxflags base,core, net) $(shell pkg-config --cflags ffms2) -I./include

# C/C++ flags
# For some reason, the CLIENT_IMP doesn't seem to be passing down, oh well
CPPFLAGS := -Wall -Wno-maybe-uninitialized -ggdb -m64 -DCLIENT_IMP

ifeq ($(BUILD),release)
	# "Release" build - optimization, and no debug symbols
	CPPFLAGS += -O3 -s -DNDEBUG
else
	# "Debug" build - no optimization, and debugging symbols
	CPPFLAGS += -Og -g
endif

# Linker flags (-lpthread needed for threads)
# Includes some neccessary linker flags for win32-darkmode (will add in the future)
LDFLAGS := $(shell wx-config --libs base,core,net) $(shell pkg-config --libs ffms2) -lpthread
ifeq ($(UNAME),Msys)
	# Needed for sockets on windows
	LDFLAGS += -lws2_32
endif

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

ifeq ($(UNAME),Msys)
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(eval MOVED_OBJS := $(subst ..,.,$(OBJS)))
	$(CXX) $(MOVED_OBJS) -o $@ $(LDFLAGS)
	# Copy required DLLs to folder
	ldd $(BUILD_DIR)/$(TARGET_EXEC) | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" $(BUILD_DIR)
else
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(eval MOVED_OBJS := $(subst ..,.,$(OBJS)))
	$(CXX) $(MOVED_OBJS) -o $@ $(LDFLAGS)
endif

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(eval REALPATH := $(subst ..,.,$@))
	$(MKDIR_P) $(dir $(REALPATH))
	$(AS) $(ASFLAGS) -c $< -o $(REALPATH)

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(eval REALPATH := $(subst ..,.,$@))
	$(MKDIR_P) $(dir $(REALPATH))
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(REALPATH)

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(eval REALPATH := $(subst ..,.,$@))
	$(MKDIR_P) $(dir $(REALPATH))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $(REALPATH)


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p