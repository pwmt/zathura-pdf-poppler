/* SPDX-License-Identifier: Zlib */

#include <string.h>

#include "plugin.h"

static void rectangle_free(void* data) {
  poppler_rectangle_free(data);
}

girara_list_t* pdf_page_search_text(zathura_page_t* page, void* data, const char* text, zathura_error_t* error) {
  if (page == NULL || data == NULL || text == NULL || strlen(text) == 0) {
    zathura_check_set_error(error, ZATHURA_ERROR_INVALID_ARGUMENTS);
    return NULL;
  }

  PopplerPage* poppler_page = data;
  GList* results            = NULL;
  girara_list_t* list       = NULL;

  /* search text */
  results = poppler_page_find_text_with_options(poppler_page, text, POPPLER_FIND_MULTILINE);
  if (results == NULL || g_list_length(results) == 0) {
    zathura_check_set_error(error, ZATHURA_ERROR_UNKNOWN);
    goto error_free;
  }

  list = girara_list_new_with_free(g_free);
  if (list == NULL) {
    zathura_check_set_error(error, ZATHURA_ERROR_OUT_OF_MEMORY);
    goto error_free;
  }

  const double page_height = zathura_page_get_height(page);
  for (GList* entry = results; entry && entry->data; entry = g_list_next(entry)) {
    PopplerRectangle* poppler_rectangle = (PopplerRectangle*)entry->data;
    zathura_rectangle_t* rectangle      = g_try_malloc0(sizeof(zathura_rectangle_t));
    if (rectangle == NULL) {
      zathura_check_set_error(error, ZATHURA_ERROR_OUT_OF_MEMORY);
      goto error_free;
    }

    rectangle->x1 = poppler_rectangle->x1;
    rectangle->x2 = poppler_rectangle->x2;
    rectangle->y1 = page_height - poppler_rectangle->y2;
    rectangle->y2 = page_height - poppler_rectangle->y1;

    girara_list_append(list, rectangle);
  }

  g_list_free_full(results, rectangle_free);
  return list;

error_free:
  if (results != NULL) {
    g_list_free_full(results, rectangle_free);
  }

  if (list != NULL) {
    girara_list_free(list);
  }

  return NULL;
}
