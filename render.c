/* See LICENSE file for license and copyright information */

#include "plugin.h"

zathura_error_t
pdf_page_render_cairo(zathura_page_t* page, void* data, cairo_t*
    cairo, bool printing)
{
  if (page == NULL || data == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  PopplerPage* poppler_page = data;
  if (printing == false) {
    poppler_page_render(poppler_page, cairo);
  } else {
    poppler_page_render_for_printing(poppler_page, cairo);
  }

  return ZATHURA_ERROR_OK;
}
