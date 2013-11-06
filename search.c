/* See LICENSE file for license and copyright information */

#include <string.h>

#include "plugin.h"

girara_list_t*
pdf_page_search_text(zathura_page_t* page, PopplerPage* poppler_page, const
    char* text, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL || text == NULL || strlen(text) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  GList* results      = NULL;
  girara_list_t* list = NULL;

  /* search text */
  results = poppler_page_find_text(poppler_page, text);
  if (results == NULL || g_list_length(results) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  list = girara_list_new2(g_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  GList* entry = NULL;
  for (entry = results; entry && entry->data; entry = g_list_next(entry)) {
    PopplerRectangle* poppler_rectangle = (PopplerRectangle*) entry->data;
    zathura_rectangle_t* rectangle      = g_malloc0(sizeof(zathura_rectangle_t));

    rectangle->x1 = poppler_rectangle->x1;
    rectangle->x2 = poppler_rectangle->x2;
    rectangle->y1 = zathura_page_get_height(page) - poppler_rectangle->y2;
    rectangle->y2 = zathura_page_get_height(page) - poppler_rectangle->y1;

    girara_list_append(list, rectangle);
  }

  g_list_free(results);
  return list;

error_free:

  if (results != NULL) {
    g_list_free(results);
  }

  if (list != NULL) {
    girara_list_free(list);
  }

error_ret:

  return NULL;
}
