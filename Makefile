.PHONY: all clean

PROJECT_NAME = golova

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

# ------------------------------------------------------------------------
# Crossover
LIB_PATHS = -L$(RAYLIB_BUILD_PATH)
INC_PATHS = -I$(RAYLIB_BUILD_PATH)
CFLAGS = -Wall -std=c99 -Wno-missing-braces -Wunused-result
LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

SRCS = $(shell find ./src -type f -name '*.c')
HDRS = $(shell find ./src -type f -name '*.h')
OBJS = $(patsubst %.c, $(BUILD_PATH)/%.o, $(SRCS))

all: \
	raylib \
	$(PROJECT_NAME)

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

$(PROJECT_NAME): $(OBJS); \
	$(CC) $(CFLAGS) -o $(PROJECT_NAME) $^ $(LIB_PATHS) $(LIBS)

$(BUILD_PATH)/%.o: %.c; \
	$(shell mkdir -p `dirname $@`) \
	$(CC) $(INC_PATHS) -c -o $@ $<

