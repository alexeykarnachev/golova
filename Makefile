.PHONY: all clean

CC=gcc
CFLAGS=-Wall -std=c99 -Wno-missing-braces -Wunused-result -I./include
LDFLAGS=-L./lib -lraylib -lm -lpthread -ldl

CURR_DIR=$(shell pwd)
BIN_DIR=$(CURR_DIR)/bin
SRC_DIR=$(CURR_DIR)/src
DEPS_DIR=$(CURR_DIR)/deps
INCLUDE_DIR=$(CURR_DIR)/include
LIB_DIR=$(CURR_DIR)/lib

PROJ_SRCS = $(shell find $(SRC_DIR) -type f -name '*.c')
PROJ_OBJS = $(patsubst %.c,%.o,$(PROJ_SRCS))
BIN_NAMES = \
	example

# ------------------------------------------------------------------------
# Raylib
RAYLIB_VERSION=5.0
RAYLIB_URL=https://github.com/raysan5/raylib/archive/refs/tags/$(RAYLIB_VERSION).tar.gz
RAYLIB_NAME=raylib-$(RAYLIB_VERSION)
RAYLIB_DIR=$(DEPS_DIR)/$(RAYLIB_NAME)
RAYLIB_ARCHIVE_PATH=$(DEPS_DIR)/$(RAYLIB_NAME).tar.gz
RAYLIB_SRC_DIR=$(RAYLIB_DIR)/src
RAYLIB_PLATFORM=PLATFORM_DESKTOP

# ------------------------------------------------------------------------
# Raygizmo
RAYGIZMO_VERSION=1.0
RAYGIZMO_URL=https://github.com/alexeykarnachev/raygizmo/archive/refs/tags/$(RAYGIZMO_VERSION).tar.gz
RAYGIZMO_NAME=raygizmo-${RAYGIZMO_VERSION}
RAYGIZMO_DIR=$(DEPS_DIR)/$(RAYGIZMO_NAME)
RAYGIZMO_ARCHIVE_PATH=$(DEPS_DIR)/$(RAYGIZMO_NAME).tar.gz
RAYGIZMO_SRC_DIR=$(RAYGIZMO_DIR)/src

# ------------------------------------------------------------------------
# Project
all: \
	$(BIN_NAMES)
	rm -f $(PROJ_OBJS);
	rm -f $(BIN_DIR)/*.o;

$(BIN_NAMES): %: $(BIN_DIR)/%.o $(PROJ_OBJS); \
	$(CC) -o $(BIN_DIR)/$@ $^ $(LDFLAGS)

%.o: %.c; \
	$(CC) $(CFLAGS) -c -o $@ $<

# ------------------------------------------------------------------------
# Dependencies
create_dirs:
	mkdir -p $(DEPS_DIR) $(INCLUDE_DIR) $(LIB_DIR);

download_raylib:
	if [ ! -d $(RAYLIB_DIR) ]; then \
		wget $(RAYLIB_URL) -O $(RAYLIB_ARCHIVE_PATH); \
		tar zxvf $(RAYLIB_ARCHIVE_PATH) -C $(DEPS_DIR); \
	fi

download_raygizmo:
	if [ ! -d $(RAYGIZMO_DIR) ]; then \
		wget $(RAYGIZMO_URL) -O $(RAYGIZMO_ARCHIVE_PATH); \
		tar zxvf $(RAYGIZMO_ARCHIVE_PATH) -C $(DEPS_DIR); \
	fi

install_raylib:
	cd $(RAYLIB_SRC_DIR) \
	&& make PLATFORM=$(RAYLIB_PLATFORM) \
	&& sudo make install \
		RAYLIB_INSTALL_PATH=$(LIB_DIR) \
		RAYLIB_H_INSTALL_PATH=$(INCLUDE_DIR) \
	&& cp --update $(RAYLIB_SRC_DIR)/rcamera.h $(INCLUDE_DIR);

install_raygizmo:
	cp $(RAYGIZMO_SRC_DIR)/raygizmo.h $(INCLUDE_DIR);

install_deps: \
	create_dirs \
	download_raylib \
	download_raygizmo \
	install_raylib \
	install_raygizmo
	rm -f $(DEPS_DIR)/*.tar.gz;

