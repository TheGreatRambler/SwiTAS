# A sick universal makefile
# Might be important https://stackoverflow.com/questions/26738025/gtk-icon-missing-when-running-in-ms-windows/34673860#34673860
# No extension for the executable
TARGET_EXEC ?= nxtas.exe

BUILD_DIR ?= ./bin
SRC_DIRS ?= ./source

# C flags
CFLAGS := -std=gnu11
# C++ flags
# Single precision sets all defined floating point numbers to floats, saves on memory
# Includes some neccessary flags for Gtkmm
CXXFLAGS := -std=c++11 -fsingle-precision-constant $(pkg-config --cflags gtk+-3.0)
# C/C++ flags (no -pendantic)
CPPFLAGS := -g -Wall -Wextra
# Linker flags (-lpthread needed for threads)
# Includes some neccessary linker flags for Gtkmm
LDFLAGS := -lpthread $(pkg-config --libs gtk+-3.0)

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	# Copy required DLLs to folder
	ldd $(TARGET_EXEC) | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" .

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