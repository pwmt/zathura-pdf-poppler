/* See LICENSE file for license and copyright information */

#include "plugin.h"

#if !POPPLER_CHECK_VERSION(0,15,0)
#define poppler_page_get_selected_text poppler_page_get_text
#endif

#if 0
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

  PopplerRectangle rect;
  rect.x1 = rectangle.x1;
  rect.x2 = rectangle.x2;
#if !POPPLER_CHECK_VERSION(0,15,0)
  /* adapt y coordinates */
  rect.y1 = zathura_page_get_height(page) - rectangle.y1;
  rect.y2 = zathura_page_get_height(page) - rectangle.y2;
#else
  rect.y1 = rectangle.y1;
  rect.y2 = rectangle.y2;
#endif

  /* get selected text */
  return poppler_page_get_selected_text(poppler_page, POPPLER_SELECTION_GLYPH, &rect);
}
#endif
