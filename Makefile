# See LICENSE file for license and copyright information

include config.mk
include common.mk

PROJECT  = zathura-pdf-poppler
PLUGIN   = pdf
SOURCE   = $(wildcard *.c)
HEADER   = $(wildcard *.h)
OBJECTS  = ${SOURCE:.c=.o}
DOBJECTS = ${SOURCE:.c=.do}

CPPFLAGS += "-DVERSION_MAJOR=${VERSION_MAJOR}"
CPPFLAGS += "-DVERSION_MINOR=${VERSION_MINOR}"
CPPFLAGS += "-DVERSION_REV=${VERSION_REV}"

all: options ${PLUGIN}.so

# pkg-config based version checks
.version-checks/%: config.mk
	$(QUIET)test $($(*)_VERSION_CHECK) -eq 0 || \
		${PKG_CONFIG} --atleast-version $($(*)_MIN_VERSION) $($(*)_PKG_CONFIG_NAME) || ( \
		echo "The minimum required version of $(*) is $($(*)_MIN_VERSION)" && \
		false \
	)
	@mkdir -p .version-checks
	$(QUIET)touch $@

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

${OBJECTS}: config.mk \
	.version-checks/ZATHURA \
	.version-checks/POPPLER
${DOBJECTS}: config.mk \
	.version-checks/ZATHURA \
	.version-checks/POPPLER

${PLUGIN}.so: ${OBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} -shared ${LDFLAGS} -o $@ ${OBJECTS} ${LIBS}

${PLUGIN}-debug.so: ${DOBJECTS}
	$(ECHO) LD $@
	$(QUIET)${CC} -shared ${LDFLAGS} -o $@ ${DOBJECTS} ${LIBS}

clean:
	$(QUIET)rm -rf ${OBJECTS} ${DOBJECTS} $(PLUGIN).so $(PLUGIN)-debug.so \
		doc .depend ${PROJECT}-${VERSION}.tar.gz zathura-version-check

debug: options ${PLUGIN}-debug.so

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
	$(QUIET)cp -f ${PLUGIN}.so ${DESTDIR}${PLUGINDIR}
	$(QUIET)mkdir -m 755 -p ${DESTDIR}${DESKTOPPREFIX}
	$(ECHO) installing desktop file
	$(QUIET)install -m 644 ${PROJECT}.desktop ${DESTDIR}${DESKTOPPREFIX}
	$(ECHO) installing AppData file
	$(QUIET)mkdir -m 755 -p $(DESTDIR)$(APPDATAPREFIX)
	$(QUIET)install -m 644 $(PROJECT).metainfo.xml $(DESTDIR)$(APPDATAPREFIX)

uninstall:
	$(ECHO) uninstalling ${PLUGIN} plugin
	$(QUIET)rm -f ${DESTDIR}${PLUGINDIR}/${PLUGIN}.so
	$(QUIET)rmdir --ignore-fail-on-non-empty ${DESTDIR}${PLUGINDIR} 2> /dev/null
	$(ECHO) removing desktop file
	$(QUIET)rm -f ${DESTDIR}${DESKTOPPREFIX}/${PROJECT}.desktop
	$(QUIET)rmdir --ignore-fail-on-non-empty ${DESTDIR}${DESKTOPPREFIX} 2> /dev/null
	$(ECHO) removing AppData file
	$(QUIET)rm -f $(DESTDIR)$(APPDATAPREFIX)/$(PROJECT).metainfo.xml

-include $(wildcard .depend/*.dep)

.PHONY: all options clean debug doc dist install uninstall
