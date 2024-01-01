.PHONY: all clean

CC=gcc
CFLAGS=-Wall -std=c99 -Wno-missing-braces -Wunused-result -I./include
LDFLAGS=-L./lib -lraylib -lcimgui -lm -lpthread -ldl -lGL -lstdc++

THIS_DIR=$(shell pwd)
BIN_DIR=$(THIS_DIR)/bin
SRC_DIR=$(THIS_DIR)/src
DEPS_DIR=$(THIS_DIR)/deps
INCLUDE_DIR=$(THIS_DIR)/include
LIB_DIR=$(THIS_DIR)/lib

PROJ_SRCS = $(shell find $(SRC_DIR) -type f -name '*.c')
PROJ_OBJS = $(patsubst %.c,%.o,$(PROJ_SRCS))
BIN_NAMES = \
	cimgui_example \
	scene_example

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
# Cimgui
CIMGUI_NAME=cimgui
CIMGUI_DIR=$(DEPS_DIR)/$(CIMGUI_NAME)

# ------------------------------------------------------------------------
# Project
all: \
	$(BIN_NAMES)
	rm -f $(PROJ_OBJS);
	rm -f $(BIN_DIR)/*.o;

$(BIN_NAMES): %: $(BIN_DIR)/%.o $(PROJ_OBJS); \
	$(CC) -o $(BIN_DIR)/$@ $^ -Wl,-rpath=$(THIS_DIR)/lib $(LDFLAGS)

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

download_cimgui:
	if [ ! -d $(CIMGUI_DIR) ]; then \
		git clone --recursive https://github.com/cimgui/cimgui.git $(CIMGUI_DIR); \
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

install_cimgui:
	cd $(CIMGUI_DIR)/backend_test/example_glfw_opengl3 \
	&& cmake .\
	&& make \
	&& cp libcimgui.so $(LIB_DIR) \
	&& cp $(CIMGUI_DIR)/cimgui.h $(INCLUDE_DIR) \
	&& cp $(CIMGUI_DIR)/generator/output/cimgui_impl.h $(INCLUDE_DIR); \

deps: \
	create_dirs \
	download_raylib \
	download_raygizmo \
	download_cimgui \
	install_raylib \
	install_raygizmo \
	install_cimgui
	rm -f $(DEPS_DIR)/*.tar.gz;

