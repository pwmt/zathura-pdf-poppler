# See LICENSE file for license and copyright information

VERSION_MAJOR = 0
VERSION_MINOR = 2
VERSION_REV = 5
VERSION = ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}

PKG_CONFIG ?= pkg-config

# minimum required zathura version
ZATHURA_MIN_VERSION = 0.2.0
ZATHURA_VERSION_CHECK ?= $(shell $(PKG_CONFIG) --atleast-version=$(ZATHURA_MIN_VERSION) zathura; echo $$?)
ZATHURA_GTK_VERSION ?= $(shell $(PKG_CONFIG) --variable=GTK_VERSION zathura)

# paths
PREFIX ?= /usr
LIBDIR ?= ${PREFIX}/lib
DESKTOPPREFIX ?= ${PREFIX}/share/applications

# libs
CAIRO_INC ?= $(shell $(PKG_CONFIG) --cflags cairo)
CAIRO_LIB ?= $(shell $(PKG_CONFIG) --libs cairo)

PDF_INC ?= $(shell $(PKG_CONFIG) --cflags poppler-glib)
PDF_LIB ?= $(shell $(PKG_CONFIG) --libs poppler-glib)

GIRARA_INC ?= $(shell $(PKG_CONFIG) --cflags girara-gtk${ZATHURA_GTK_VERSION})
GIRARA_LIB ?= $(shell $(PKG_CONFIG) --libs girara-gtk${ZATHURA_GTK_VERSION})

ZATHURA_INC ?= $(shell $(PKG_CONFIG) --cflags zathura)
PLUGINDIR ?= $(shell $(PKG_CONFIG) --variable=plugindir zathura)
ifeq (,${PLUGINDIR})
PLUGINDIR = ${LIBDIR}/zathura
endif

INCS = ${CAIRO_INC} ${PDF_INC} ${ZATHURA_INC} ${GIRARA_INC}
LIBS = ${GIRARA_LIB} ${CAIRO_LIB} ${PDF_LIB}

# flags
CFLAGS += -std=c99 -fPIC -pedantic -Wall -Wno-format-zero-length $(INCS)

# debug
DFLAGS ?= -g

# build with cairo support?
WITH_CAIRO ?= 1

# compiler
CC ?= gcc
LD ?= ld

# set to something != 0 if you want verbose build output
VERBOSE ?= 0
