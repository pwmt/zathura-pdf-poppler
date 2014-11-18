/* See LICENSE file for license and copyright information */

#include "plugin.h"

#if !POPPLER_CHECK_VERSION(0,15,0)
#define poppler_page_get_selected_text poppler_page_get_text
#endif

zathura_error_t pdf_page_get_text(zathura_page_t* page, char** text, zathura_rectangle_t rectangle)
{
  if (page == NULL || text == NULL) {
      return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  PopplerPage* poppler_page;
  if ((error = zathura_page_get_data(page, (void**) &poppler_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerRectangle rect;
  rect.x1 = rectangle.p1.x;
  rect.x2 = rectangle.p2.x;

#if !POPPLER_CHECK_VERSION(0,15,0)
  /* adapt y coordinates */
  unsigned int page_height;
  if ((error = zathura_page_get_height(page, &page_height)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  rect.y1 = height - rectangle.p1.y;
  rect.y2 = height - rectangle.p2.y;
#else
  rect.y1 = rectangle.p1.y;
  rect.y2 = rectangle.p2.y;
#endif

  /* get selected text */
  *text = poppler_page_get_selected_text(poppler_page, POPPLER_SELECTION_GLYPH, &rect);

error_out:

  return error;
}
