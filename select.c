/* See LICENSE file for license and copyright information */

#include "plugin.h"

char*
pdf_page_get_text(zathura_page_t* page, PopplerPage* poppler_page,
    zathura_rectangle_t rectangle, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  PopplerRectangle rect = {
    .x1 = rectangle.x1,
    .x2 = rectangle.x2,
    .y1 = rectangle.y1,
    .y2 = rectangle.y2
  };

  /* get selected text */
  return poppler_page_get_selected_text(poppler_page, POPPLER_SELECTION_GLYPH, &rect);
}
