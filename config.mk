# See LICENSE file for license and copyright information

VERSION_MAJOR = 0
VERSION_MINOR = 2
VERSION_REV = 8
VERSION = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}

PKG_CONFIG ?= pkg-config

# version checks
# If you want to disable any of the checks, set *_VERSION_CHECK to 0.

# zathura
ZATHURA_VERSION_CHECK ?= 1
ZATHURA_MIN_VERSION = 0.3.8
ZATHURA_PKG_CONFIG_NAME = zathura

# poppler
POPPLER_VERSION_CHECK ?= 1
POPPLER_MIN_VERSION = 0.18
POPPLER_PKG_CONFIG_NAME = poppler-glib

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
DESKTOPPREFIX ?= ${PREFIX}/share/applications
APPDATAPREFIX ?= ${PREFIX}/share/metainfo

# libs
CAIRO_INC ?= $(shell $(PKG_CONFIG) --cflags cairo)
CAIRO_LIB ?= $(shell $(PKG_CONFIG) --libs cairo)

PDF_INC ?= $(shell $(PKG_CONFIG) --cflags poppler-glib)
PDF_LIB ?= $(shell $(PKG_CONFIG) --libs poppler-glib)

GIRARA_INC ?= $(shell $(PKG_CONFIG) --cflags girara-gtk3)
GIRARA_LIB ?= $(shell $(PKG_CONFIG) --libs girara-gtk3)

ZATHURA_INC ?= $(shell $(PKG_CONFIG) --cflags zathura)
PLUGINDIR ?= $(shell $(PKG_CONFIG) --variable=plugindir zathura)
ifeq (,${PLUGINDIR})
PLUGINDIR = ${LIBDIR}/zathura
endif

INCS = ${CAIRO_INC} ${PDF_INC} ${ZATHURA_INC} ${GIRARA_INC}
LIBS = ${GIRARA_LIB} ${CAIRO_LIB} ${PDF_LIB}

# uname
UNAME := $(shell uname -s)

# compiler flags
CFLAGS += -std=c11 -fPIC -pedantic -Wall -Wno-format-zero-length $(INCS)

# linker flags
LDFLAGS += -fPIC
ifeq ($(UNAME), Darwin)
LDFLAGS += -fno-common
endif

# debug
DFLAGS ?= -g

# compiler
CC ?= gcc
LD ?= ld

# set to something != 0 if you want verbose build output
VERBOSE ?= 0
