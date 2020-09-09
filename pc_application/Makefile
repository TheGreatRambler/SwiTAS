UNAME := $(shell uname -o)

ifeq ($(UNAME),Msys)
	TARGET_EXEC ?= switas.exe
else
	TARGET_EXEC ?= switas
endif

BUILD_DIR ?= ./bin

SRC_DIRS ?= ./source

ifeq ($(UNAME),Msys)
# Set compilers to MinGW64 compilers
CC := x86_64-w64-mingw32-gcc
CXX := x86_64-w64-mingw32-g++
else
CC := gcc
CXX := g++
endif

# C flags
CFLAGS := -std=c11

# C++ flags
CXXFLAGS := -std=gnu++17 $(shell wx-config --cxxflags base,core,net) $(shell pkg-config --cflags ffms2 libzstd) -I./include

# C/C++ flags
CPPFLAGS := -Wall -Wno-maybe-uninitialized -Wno-sign-compare -Wswitch-enum -DCLIENT_IMP

ifeq ($(BUILD),release)
	# "Release" build - optimization, and no debug symbols
	CPPFLAGS += -O3 -s -DNDEBUG
else
	# "Debug" build - no optimization, and debugging symbols
	CPPFLAGS += -Og -g -ggdb -DDEBUG
endif

ifeq ($(SYSTEM),debian)
	# "Release" build - optimization, and no debug symbols
	CPPFLAGS += -DDEBIAN_SYSTEM
endif

# Linker flags (-lpthread needed for threads)
# Includes some neccessary linker flags for win32-darkmode (will add in the future)
LDFLAGS := $(shell wx-config --libs base,core,net) $(shell pkg-config --libs ffms2 libzstd) -lpthread
ifeq ($(UNAME),Msys)
	# Needed for sockets on windows
	LDFLAGS += -lws2_32
endif

ifeq ($(ARCH),32)
	CPPFLAGS += -m32
	LDFLAGS += -m32
else
	CPPFLAGS += -m64
	LDFLAGS += -m64
endif

ifeq ($(UNAME),Msys)
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s -or -name *.rc)
else
SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

# Custom DLLs explanation
# To save filesize on the ffmpeg dlls, I used this configure line:
# ./configure --disable-static --enable-shared --disable-doc --disable-programs --disable-network --disable-avfilter --disable-avdevice --enable-small --disable-bsfs --disable-protocol=pipe --disable-devices --disable-filters --disable-iconv --disable-debug --disable-hwaccels --disable-encoders --enable-encoder=mpeg4 --enable-libvpx --enable-libvorbis
# At some point, should manually include only the decoders I need, the ones most common to youtube
# Notice: Build the DLLs for 32 bit

# App Imgur album https://imgur.com/a/26GvzqA https://imgur.com/a/Ps2Z8c6 https://imgur.com/a/9gBNrDd

# For assets
# https://unix.stackexchange.com/a/314823

# WxGTK packages
# sudo apt-get install libwxbase3.1-0-unofficial3 libwxbase3.1unofficial3-dev libwxgtk3.1-0-unofficial3 libwxgtk3.1unofficial3-dev wx3.1-headers wx-common

all: pre-build $(BUILD_DIR)/$(TARGET_EXEC)

ifeq ($(SNAPCRAFT_PROJECT_NAME),)
pre-build:
	# This runs before anything happens
	# Copy in shared_network_code
	# https://stackoverflow.com/a/1622186/9329945
	rm -r -f ./source/shared_network_code
	cp -p -r ../shared_network_code ./source/shared_network_code
else
pre-build: ;
endif

ifeq ($(UNAME),Msys)
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	# Delete all DLLs already there
	find . -type f -iwholename $(BUILD_DIR)\*.dll -delete
	# Add custom DLLs
	cp -a custom_dlls/. $(BUILD_DIR)
	# Copy required DLLs to folder, not copying the ones that are custom
	ldd $(BUILD_DIR)/$(TARGET_EXEC) | grep '\/mingw.*\.dll' -o | xargs -I{} cp -n "{}" $(BUILD_DIR)
else
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
endif

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

ifeq ($(UNAME),Msys)
# Windows RES file
$(BUILD_DIR)/%.rc.o: %.rc
	$(MKDIR_P) $(dir $@)
	windres $< $@
endif


.PHONY: all clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p