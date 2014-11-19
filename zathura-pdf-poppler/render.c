/* See LICENSE file for license and copyright information */

#include "plugin.h"

#if !defined(HAVE_CAIRO) && POPPLER_CHECK_VERSION(0,18,0)
#error "Cannot render without cairo and poppler >= 0.18"
#endif

#ifdef HAVE_CAIRO
zathura_error_t pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo,
    double scale, int rotation, int flags)
{
  if (page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  PopplerPage* poppler_page;
  if ((error = zathura_page_get_data(page, (void**) &poppler_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  poppler_page_render(poppler_page, cairo);

error_out:

  return error;
}
#endif
