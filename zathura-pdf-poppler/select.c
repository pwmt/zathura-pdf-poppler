/* SPDX-License-Identifier: Zlib */

#include "plugin.h"

char*
pdf_page_get_text(zathura_page_t* page, void* data,
    zathura_rectangle_t rectangle, zathura_error_t* error)
{
  if (page == NULL || data == NULL) {
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
  PopplerPage* poppler_page = data;

  /* get selected text */
  return poppler_page_get_selected_text(poppler_page, POPPLER_SELECTION_GLYPH, &rect);
}

girara_list_t*
pdf_page_get_selection(zathura_page_t* page, void* data, zathura_rectangle_t rectangle, zathura_error_t* error) {
  if (page == NULL || data == NULL) {
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
  PopplerPage* poppler_page = data;

  girara_list_t* list = girara_list_new2(g_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  GList* rectangles = poppler_page_get_selection_region(poppler_page, 1.0, POPPLER_SELECTION_GLYPH, &rect);
  for (GList* head = rectangles; head != NULL; head = head->next) {
    PopplerRectangle* r = head->data;
    zathura_rectangle_t* inner_rectangle = g_malloc0(sizeof(zathura_rectangle_t));
    inner_rectangle->x1 = r->x1;
    inner_rectangle->x2 = r->x2;
    inner_rectangle->y1 = r->y1;
    inner_rectangle->y2 = r->y2;
    girara_list_append(list, inner_rectangle);
  }
  poppler_page_selection_region_free(rectangles);
  return list;

error_free:
  if (list != NULL ) {
      girara_list_free(list);
  }
  return NULL;
}
