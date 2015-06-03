# See LICENSE file for license and copyright information

include ../config.mk
include ../colors.mk
include ../common.mk

include config.mk

PROJECT       = tests
SOURCE        = $(wildcard *.cpp)
OBJECTS       = $(addprefix ${BUILDDIR_RELEASE}/,${SOURCE:.cpp=.o})
OBJECTS_DEBUG = $(addprefix ${BUILDDIR_DEBUG}/,${SOURCE:.cpp=.o})
OBJECTS_GCOV  = $(addprefix ${BUILDDIR_GCOV}/,${SOURCE:.cpp=.o})

ifneq (${WITH_CAIRO},0)
INCS += ${CAIRO_INC}
LIBS += ${CAIRO_LIB}
CPPFLAGS += -DHAVE_CAIRO
endif

ifneq (${WITH_LIBFIU},0)
INCS += ${FIU_INC}
LIBS += ${FIU_LIB}
CPPFLAGS += -DWITH_LIBFIU
FIU_EXEC += ${FIU_RUN}
endif

ifneq ($(wildcard ${VALGRIND_SUPPRESSION_FILE}),)
VALGRIND_ARGUMENTS += --suppressions=${VALGRIND_SUPPRESSION_FILE}
endif

all: ${PROJECT}

options:
	$(ECHO) ${PROJECT} build options:
	$(ECHO) "CXXFLAGS  = ${CXXFLAGS}"
	$(ECHO) "LDFLAGS = ${LDFLAGS}"
	$(ECHO) "DFLAGS  = ${DFLAGS}"
	$(ECHO) "CXX      = ${CXX}"

# release

${PROJECT}: options ${OBJECTS}
	$(QUIET)${MAKE} WITH_LIBFIU=1 -C .. ${BUILDDIR_RELEASE}/${PLUGIN}.so
	$(call colorecho,CXX,$@)
	$(QUIET)${CXX} ${SFLAGS} ${LDFLAGS} -o $@ \
		${OBJECTS} ${LIBS} ${PLUGIN_RELEASE}

${OBJECTS}: config.mk ../config.mk

${BUILDDIR_RELEASE}/%.o: %.cpp
	$(call colorecho,CXX,$<)
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(QUIET)${CXX} -c ${CPPFLAGS} ${CXXFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

run: ${PROJECT}
	$(QUIET)${FIU_EXEC} ./${PROJECT}

# debug

debug: options ${PROJECT}-debug

${PROJECT}-debug: ${OBJECTS_DEBUG}
	$(QUIET)${MAKE} WITH_LIBFIU=1 -C .. ${BUILDDIR_DEBUG}/${PLUGIN}.so
	$(call colorecho,CXX,$@)
	$(QUIET)${CXX} ${LDFLAGS} -o $@ \
		${OBJECTS_DEBUG} ${LIBS} ${PLUGIN_DEBUG}

${OBJECTS_DEBUG}: config.mk ../config.mk

${BUILDDIR_DEBUG}/%.o: %.cpp
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CXX,$<)
	$(QUIET)${CXX} -c ${CPPFLAGS} -DBUILD_DEBUG=1 ${CXXFLAGS} ${DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

run-debug: ${PROJECT}-debug
	$(QUIET)${FIU_EXEC} ./${PROJECT}-debug

# gcov

gcov: options ${PROJECT}-gcov

${PROJECT}-gcov: options ${OBJECTS_GCOV}
	$(QUIET)${MAKE} WITH_LIBFIU=1 -C .. ${BUILDDIR_GCOV}/${PLUGIN}.so
	$(call colorecho,CXX,$@)
	$(QUIET)${CXX} ${LDFLAGS} ${GCOV_LDFLAGS} -o $@ \
		${OBJECTS_GCOV} ${LIBS} ${PLUGIN_GCOV}

${OBJECTS_GCOV}: config.mk ../config.mk

${BUILDDIR_GCOV}/%.o: %.cpp
	@mkdir -p ${DEPENDDIR}/$(dir $(abspath $@))
	@mkdir -p $(dir $(abspath $@))
	$(call colorecho,CXX,$<)
	$(QUIET)${CXX} -c ${CPPFLAGS} -DBUILD_GCOV=1 ${CXXFLAGS} ${GCOV_CXXFLAGS} ${DFLAGS} ${GCOV_DFLAGS} \
		-o $@ $< -MMD -MF ${DEPENDDIR}/$(abspath $@).dep

run-gcov: ${PROJECT}-gcov
	$(QUIET)${FIU_EXEC} ./${PROJECT}-gcov

valgrind: ${PROJECT}-debug
	 $(QUIET)G_SLICE=always-malloc G_DEBUG=gc-friendly ${FIU_EXEC} ${VALGRIND} ${VALGRIND_ARGUMENTS} ./${PROJECT}-debug

clean:
	$(call colorecho,RM, "Clean test files")
	$(QUIET)rm -rf ${PROJECT}
	$(QUIET)rm -rf ${PROJECT}-debug
	$(QUIET)rm -rf ${PROJECT}-gcov
	$(QUIET)rm -rf ${BUILDDIR}
	$(QUIET)rm -rf ${DEPENDDIR}

.PHONY: all options clean debug run

-include $(wildcard ${DEPENDDIR}/*.dep)
