/* See LICENSE file for license and copyright information */

#include "plugin.h"

zathura_error_t
pdf_page_render_cairo(zathura_page_t* page, PopplerPage* poppler_page, cairo_t*
    cairo, bool printing)
{
  if (page == NULL || poppler_page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (printing == false) {
    poppler_page_render(poppler_page, cairo);
  } else {
    poppler_page_render_for_printing(poppler_page, cairo);
  }

  return ZATHURA_ERROR_OK;
}
