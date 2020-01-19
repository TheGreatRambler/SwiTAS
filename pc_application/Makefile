# A sick universal makefile
# Might be important https://stackoverflow.com/questions/26738025/gtk-icon-missing-when-running-in-ms-windows/34673860#34673860
# Same with this http://www.tarnyko.net/repo/gtk3_build_system/tutorial/gtk3_tutorial.htm
# https://github.com/tschoonj/GTK-for-Windows-Runtime-Environment-Installer
# This needs to be set on run TODO GTK_EXE_PREFIX=[root of project with trailing /]
# No extension for the executable
TARGET_EXEC ?= nxtas.exe

BUILD_DIR ?= ./bin
SRC_DIRS ?= ./wxsource

# Set compilers to MinGW64 compilers
CC := x86_64-w64-mingw32-gcc
CXX := x86_64-w64-mingw32-g++

# C flags
CFLAGS := -std=gnu11
# C++ flags
# Single precision sets all defined floating point numbers to floats, saves on memory
# Includes some neccessary flags for Gtkmm
# Also, import third party headers in `include`
# Libpath needs to be set right
CXXFLAGS := -std=c++11 $(shell wx-config --cxxflags) -I./include
# C/C++ flags (no -pendantic)
# -fpermissive needed for some misbehaving GTK code
CPPFLAGS := -Wall -fpermissive -ggdb
# Linker flags (-lpthread needed for threads)
# Includes some neccessary linker flags for Gtkmm
LDFLAGS := -lpthread $(shell wx-config --libs) -lws2_32

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	# Put here so files are read from the right location
	set GTK_EXE_PREFIX = .
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	# Copy required DLLs to folder
	ldd $(BUILD_DIR)/$(TARGET_EXEC) | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" $(BUILD_DIR)
	# Compile GTK schema
	glib-compile-schemas ./share/glib-2.0/schemas

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