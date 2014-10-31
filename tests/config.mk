# See LICENSE file for license and copyright information

CHECK_INC ?= $(shell pkg-config --cflags check)
CHECK_LIB ?= $(shell pkg-config --libs check)

INCS += ${CHECK_INC} ${FIU_INC} -I../zathura-pdf-poppler
LIBS += ${CHECK_LIB} ${FIU_LIB} -Wl,--whole-archive -Wl,--no-whole-archive
LDFLAGS += -rdynamic

PLUGIN_RELEASE=../${BUILDDIR_RELEASE}/${PLUGIN}.so
PLUGIN_DEBUG=../${BUILDDIR_DEBUG}/${PLUGIN}.so
PLUGIN_GCOV=../${BUILDDIR_GCOV}/${PLUGIN}.so

# valgrind
VALGRIND = valgrind
VALGRIND_ARGUMENTS = --tool=memcheck --leak-check=yes --leak-resolution=high \
	--show-reachable=yes --log-file=${PROJECT}-valgrind.log
VALGRIND_SUPPRESSION_FILE = ${PROJECT}.suppression
