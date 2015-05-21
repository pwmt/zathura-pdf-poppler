/* See LICENSE file for license and copyright information */

#include "plugin.h"
#include "macros.h"
#include "internal.h"

#if !defined(HAVE_CAIRO) && POPPLER_CHECK_VERSION(0,18,0)
#error "Cannot render without cairo and poppler >= 0.18"
#endif

#ifdef HAVE_CAIRO
zathura_error_t pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo,
    double UNUSED(scale), int UNUSED(rotation), int UNUSED(flags))
{
  if (page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  return error;
}
#endif
