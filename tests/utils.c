/* See LICENSE file for license and copyright information */

#include "utils.h"

const char*
get_plugin_path(void)
{
#ifdef BUILD_DEBUG
  return "../build/debug/pdf.so";
#elif BUILD_GCOV
  return "../build/gcov/pdf.so";
#else
  return "../build/release/pdf.so";
#endif
}
