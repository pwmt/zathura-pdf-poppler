/* SPDX-License-Identifier: Zlib */

#include "plugin.h"

static PopplerRectangle poppler_rect_from_zathura(zathura_rectangle_t rectangle) {
  PopplerRectangle rect = {
      .x1 = rectangle.x1,
      .x2 = rectangle.x2,
      .y1 = rectangle.y1,
      .y2 = rectangle.y2,
  };
  return rect;
}

char* pdf_page_get_text(zathura_page_t* page, void* data, zathura_rectangle_t rectangle, zathura_error_t* error) {
  if (page == NULL || data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  PopplerRectangle rect = poppler_rect_from_zathura(rectangle);
  PopplerPage* poppler_page = data;

  /* get selected text */
  return poppler_page_get_selected_text(poppler_page, POPPLER_SELECTION_GLYPH, &rect);
}

girara_list_t* pdf_page_get_selection(zathura_page_t* page, void* data, zathura_rectangle_t rectangle,
                                      zathura_error_t* error) {
  if (page == NULL || data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  PopplerRectangle rect = poppler_rect_from_zathura(rectangle);
  PopplerPage* poppler_page = data;

  girara_list_t* list = girara_list_new2(g_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  cairo_region_t* region   = poppler_page_get_selected_region(poppler_page, 1.0, POPPLER_SELECTION_GLYPH, &rect);
  const int num_rectangles = cairo_region_num_rectangles(region);
  for (int n = 0; n < num_rectangles; ++n) {
    cairo_rectangle_int_t r;
    cairo_region_get_rectangle(region, n, &r);

    zathura_rectangle_t* inner_rectangle = g_malloc0(sizeof(zathura_rectangle_t));
    inner_rectangle->x1                  = r.x;
    inner_rectangle->x2                  = r.x + r.width;
    inner_rectangle->y1                  = r.y;
    inner_rectangle->y2                  = r.y + r.height;
    girara_list_append(list, inner_rectangle);
  }
  cairo_region_destroy(region);
  return list;

error_free:
  if (list != NULL) {
    girara_list_free(list);
  }
  return NULL;
}
