# See LICENSE file for license and copyright information

include config.mk
include common.mk

PROJECT  = zathura-pdf-poppler
PLUGIN   = pdf
SOURCE   = $(wildcard *.c)
HEADER   = $(wildcard *.h)
OBJECTS  = ${SOURCE:.c=.o}
DOBJECTS = ${SOURCE:.c=.do}

ifneq "$(WITH_CAIRO)" "0"
CPPFLAGS += -DHAVE_CAIRO
endif

ifeq ($(UNAME), Darwin)
SOFILE = ${PLUGIN}.dylib
SODEBUGFILE = ${PLUGIN}-debug.dylib
ZATHURAFILE = ${PREFIX}/bin/zathura
PLATFORMFLAGS = -Wl,-dylib_install_name,${DESTDIR}${PLUGINDIR}/pdf.dylib -Wl,-bundle_loader,${ZATHURAFILE} -bundle
else
SOFILE = ${PLUGIN}.so
SODEBUGFILE = ${PLUGIN}-debug.so
PLATFORMFLAGS = -shared
endif

CPPFLAGS += "-DVERSION_MAJOR=${VERSION_MAJOR}"
CPPFLAGS += "-DVERSION_MINOR=${VERSION_MINOR}"
CPPFLAGS += "-DVERSION_REV=${VERSION_REV}"

all: options ${SOFILE}

zathura-version-check:
ifneq ($(ZATHURA_VERSION_CHECK), 0)
	$(error "The minimum required version of zathura is ${ZATHURA_MIN_VERSION}")
endif
	$(QUIET)touch zathura-version-check

options:
	$(ECHO) ${PLUGIN} build options:
	$(ECHO) "CFLAGS     = ${CFLAGS}"
	$(ECHO) "LDFLAGS    = ${LDFLAGS}"
	$(ECHO) "DFLAGS     = ${DFLAGS}"
	$(ECHO) "CC         = ${CC}"
	$(ECHO) "PKG_CONFIG = ${PKG_CONFIG}"

%.o: %.c
	$(ECHO) CC $<
	@mkdir -p .depend
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

%.do: %.c
	$(ECHO) CC $<
	@mkdir -p .depend
	$(QUIET)${CC} -c ${CPPFLAGS} ${CFLAGS} ${DFLAGS} -o $@ $< -MMD -MF .depend/$@.dep

${OBJECTS}:  config.mk zathura-version-check
${DOBJECTS}: config.mk zathura-version-check

${SOFILE}: ${OBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} ${PLATFORMFLAGS} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

${SODEBUGFILE}: ${DOBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} ${PLATFORMFLAGS} ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

clean:
	$(QUIET)rm -rf ${OBJECTS} ${DOBJECTS} ${SOFILE} ${SODEBUGFILE} \
		doc .depend ${PROJECT}-${VERSION}.tar.gz zathura-version-check

debug: options ${SODEBUGFILE}

dist: clean
	$(QUIET)mkdir -p ${PROJECT}-${VERSION}
	$(QUIET)cp -R LICENSE Makefile config.mk common.mk Doxyfile \
		${HEADER} ${SOURCE} AUTHORS ${PROJECT}.desktop \
		${PROJECT}.metainfo.xml \
		${PROJECT}-${VERSION}
	$(QUIET)tar -cf ${PROJECT}-${VERSION}.tar ${PROJECT}-${VERSION}
	$(QUIET)gzip ${PROJECT}-${VERSION}.tar
	$(QUIET)rm -rf ${PROJECT}-${VERSION}

doc: clean
	$(QUIET)doxygen Doxyfile

install: all
	$(ECHO) installing ${PLUGIN} plugin
	$(QUIET)mkdir -p ${DESTDIR}${PLUGINDIR}
	$(QUIET)cp -f ${SOFILE} ${DESTDIR}${PLUGINDIR}
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${DESKTOPPREFIX}
	$(ECHO) installing desktop file
	$(QUIET)install -m 644 ${PROJECT}.desktop ${DESTDIR}${DESKTOPPREFIX}
	$(ECHO) installing AppData file
	$(QUIET)mkdir -m 755 -p $(DESTDIR)$(APPDATAPREFIX)
	$(QUIET)install -m 644 $(PROJECT).metainfo.xml $(DESTDIR)$(APPDATAPREFIX)

uninstall:
	$(ECHO) uninstalling ${PLUGIN} plugin
	$(QUIET)rm -f ${DESTDIR}${PLUGINDIR}/${SOFILE}
	$(QUIET)find ${DESTDIR}${PLUGINDIR} -type d -empty -delete 2> /dev/null
	$(ECHO) removing desktop file
	$(QUIET)rm -f ${DESTDIR}${DESKTOPPREFIX}/${PROJECT}.desktop
	$(QUIET)find ${DESTDIR}${DESKTOPPREFIX} -type d -empty -delete 2> /dev/null
	$(ECHO) removing AppData file
	$(QUIET)rm -f $(DESTDIR)$(APPDATAPREFIX)/$(PROJECT).metainfo.xml

-include $(wildcard .depend/*.dep)

.PHONY: all options clean debug doc dist install uninstall
