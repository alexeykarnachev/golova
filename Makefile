.PHONY: all clean

CC = gcc
CURR_DIR=$(shell pwd)
DEPS_PATH=$(CURR_DIR)/deps
BUILD_PATH=$(CURR_DIR)/build

# ------------------------------------------------------------------------
# Raylib
RAYLIB_VERSION=5.0
RAYLIB_URL=https://github.com/raysan5/raylib/archive/refs/tags/$(RAYLIB_VERSION).tar.gz
RAYLIB_NAME=raylib-$(RAYLIB_VERSION)
RAYLIB_PATH=$(DEPS_PATH)/$(RAYLIB_NAME)
RAYLIB_ARCHIVE_PATH=$(DEPS_PATH)/$(RAYLIB_NAME).tar.gz
RAYLIB_SRC_PATH=$(RAYLIB_PATH)/src
RAYLIB_BUILD_PATH=$(RAYLIB_PATH)/build
RAYLIB_PLATFORM=PLATFORM_DESKTOP

RAYGUI_VERSION=4.0
RAYGUI_URL=https://github.com/raysan5/raygui/archive/refs/tags/$(RAYGUI_VERSION).tar.gz
RAYGUI_NAME=raygui-$(RAYGUI_VERSION)
RAYGUI_PATH=$(DEPS_PATH)/$(RAYGUI_NAME)
RAYGUI_ARCHIVE_PATH=$(DEPS_PATH)/$(RAYGUI_NAME).tar.gz
RAYGUI_SRC_PATH=$(RAYGUI_PATH)/src

RAYGIZMO_VERSION=1.0
RAYGIZMO_URL=https://github.com/alexeykarnachev/raygizmo/archive/refs/tags/$(RAYGIZMO_VERSION).tar.gz
RAYGIZMO_NAME=raygizmo-${RAYGIZMO_VERSION}
RAYGIZMO_PATH=$(DEPS_PATH)/$(RAYGIZMO_NAME)
RAYGIZMO_ARCHIVE_PATH=$(DEPS_PATH)/$(RAYGIZMO_NAME).tar.gz
RAYGIZMO_SRC_PATH=$(RAYGIZMO_PATH)/src

# ------------------------------------------------------------------------
# Golova
LIB_PATHS = -L$(RAYLIB_BUILD_PATH)
INC_PATHS = -I$(RAYLIB_SRC_PATH) -I$(RAYGUI_SRC_PATH) -I$(RAYGIZMO_SRC_PATH)
CFLAGS = -Wall -std=c99 -Wno-missing-braces -Wunused-result
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRCS = $(shell find ./src -type f -name '*.c')
OBJS = $(patsubst %.c, $(BUILD_PATH)/%.o, $(SRCS))

BIN_SRCS = $(shell find ./bin -type f -name '*.c')
BIN_NAMES = $(patsubst ./bin/%.c, %, $(BIN_SRCS))

# ------------------------------------------------------------------------
# Targets
all: \
	raylib \
	$(BIN_NAMES)

$(BIN_NAMES): %: $(BUILD_PATH)/bin/%.o $(OBJS); \
	$(CC) $(CFLAGS) -o ./build/bin/$@ $^ $(LIB_PATHS) $(LIBS)

$(BUILD_PATH)/%.o: %.c; \
	$(shell mkdir -p `dirname $@`) \
	$(CC) $(INC_PATHS) -c -o $@ $<

raylib: SHELL:=/bin/bash
raylib:
	if [ ! -d $(RAYLIB_PATH) ]; then \
		mkdir -p $(DEPS_PATH); \
		wget $(RAYLIB_URL) -O $(RAYLIB_ARCHIVE_PATH); \
		tar zxvf $(RAYLIB_ARCHIVE_PATH) -C $(DEPS_PATH); \
	fi
	if [ ! -d $(RAYLIB_BUILD_PATH) ]; then \
		cd $(RAYLIB_SRC_PATH) \
		&& make PLATFORM=$(RAYLIB_PLATFORM) \
		&& sudo make install \
			RAYLIB_INSTALL_PATH=$(RAYLIB_BUILD_PATH) \
			RAYLIB_H_INSTALL_PATH=$(RAYLIB_BUILD_PATH); \
	fi
	if [ ! -d $(RAYGUI_PATH) ]; then \
		wget $(RAYGUI_URL) -O $(RAYGUI_ARCHIVE_PATH); \
		tar zxvf $(RAYGUI_ARCHIVE_PATH) -C $(DEPS_PATH); \
	fi
	if [ ! -d $(RAYGIZMO_PATH) ]; then \
		wget $(RAYGIZMO_URL) -O $(RAYGIZMO_ARCHIVE_PATH); \
		tar zxvf $(RAYGIZMO_ARCHIVE_PATH) -C $(DEPS_PATH); \
	fi
