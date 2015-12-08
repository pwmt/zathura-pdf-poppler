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

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_user_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerPage* poppler_page = pdf_page->poppler_page;

  poppler_page_render_for_printing_with_options(poppler_page, cairo, POPPLER_PRINT_DOCUMENT);

error_out:

  return error;
}
#endif
