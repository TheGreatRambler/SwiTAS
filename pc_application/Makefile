UNAME := $(shell uname -o)

ifeq ($(UNAME),Msys)
	TARGET_EXEC ?= nxtasui.exe
else
	TARGET_EXEC ?= nxtasui
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
CXXFLAGS := -std=gnu++17 $(shell wx-config --cxxflags base,core) $(shell pkg-config --cflags ffms2 luajit) -I./include

# C/C++ flags
# For some reason, the CLIENT_IMP doesn't seem to be passing down, oh well
CPPFLAGS := -Wall -Wno-maybe-uninitialized -DCLIENT_IMP

ifeq ($(BUILD),release)
	# "Release" build - optimization, and no debug symbols
	CPPFLAGS += -O3 -s -DNDEBUG
else
	# "Debug" build - no optimization, and debugging symbols
	CPPFLAGS += -Og -g -ggdb
endif

ifeq ($(ARCH),32)
	CPPFLAGS += -m32
else
	CPPFLAGS += -m64
endif

# Linker flags (-lpthread needed for threads)
# Includes some neccessary linker flags for win32-darkmode (will add in the future)
LDFLAGS := $(shell wx-config --libs base,core) $(shell pkg-config --libs ffms2 luajit) -lpthread
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

# Custom DLLs explanation
# To save filesize on the ffmpeg dlls, I used this configure line:
# ./configure --disable-static --enable-shared --disable-doc --disable-programs --disable-network --disable-avfilter --disable-avdevice --enable-small --disable-bsfs --disable-protocol=pipe --disable-bzlib --disable-devices --disable-filters --disable-iconv --disable-debug --disable-hwaccels --disable-encoders --enable-encoder=mpeg4 --extra-cflags="-arch i386" --extra-ldflags='-arch i386' --arch=x86_32
# At some point, should manually include only the decoders I need, the ones most common to youtube
# Notice: Build the DLLs for 32 bit

# WxGTK packages
# sudo apt-get install libwxbase3.1-0-unofficial3 libwxbase3.1unofficial3-dev libwxgtk3.1-0-unofficial3 libwxgtk3.1unofficial3-dev wx3.1-headers wx-common

# To build for linux, use snap:
# sudo snapcraft --use-lxd

ifeq ($(UNAME),Msys)
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	# Delete all DLLs already there
	find . -type f -iwholename $(BUILD_DIR)\*.dll -delete
	# Add custom DLLs
	cp -a customDlls/. $(BUILD_DIR)
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


.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p