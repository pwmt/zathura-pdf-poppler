/* See LICENSE file for license and copyright information */

#include "plugin.h"
#include "macros.h"
#include "internal.h"

#ifdef HAVE_CAIRO
zathura_error_t pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo,
    double UNUSED(scale), int UNUSED(rotation), int UNUSED(flags))
{
  if (page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  return ZATHURA_ERROR_UNKNOWN;

error_out:

  return error;
}
#endif
