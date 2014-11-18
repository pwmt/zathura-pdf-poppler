/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "plugin.h"

zathura_error_t pdf_page_search_text(zathura_page_t* page, const char* text,
    zathura_search_flag_t flags, zathura_list_t** results)
{
  if (page == NULL || text == NULL || strlen(text) == 0 || results == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  *results = NULL;

  PopplerPage* poppler_page;
  if ((error = zathura_page_get_data(page, (void**) &poppler_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  /* handle flags */
  PopplerFindFlags poppler_flags = POPPLER_FIND_DEFAULT;
  if (flags & ZATHURA_SEARCH_CASE_SENSITIVE) {
    poppler_flags |= POPPLER_FIND_CASE_SENSITIVE;
  }
  if (flags & ZATHURA_SEARCH_WHOLE_WORDS_ONLY) {
    poppler_flags |= POPPLER_FIND_WHOLE_WORDS_ONLY;
  }

  GList* poppler_results = poppler_page_find_text_with_options(poppler_page, text, poppler_flags);
  if (poppler_results == NULL || g_list_length(poppler_results) == 0) {
    error = ZATHURA_ERROR_SEARCH_NO_RESULTS;
    goto error_out;
  }

  unsigned int page_height;
  if ((error = zathura_page_get_height(page, &page_height)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerRectangle* poppler_rectangle;
  ZATHURA_LIST_FOREACH(poppler_rectangle, poppler_results) {
    zathura_rectangle_t* rectangle = calloc(1, sizeof(zathura_rectangle_t));
    if (rectangle == NULL) {
      error = ZATHURA_ERROR_OUT_OF_MEMORY;
      goto error_free;
    }

    rectangle->p1.x = poppler_rectangle->x1;
    rectangle->p1.y = page_height - poppler_rectangle->y1;
    rectangle->p2.x = poppler_rectangle->x2;
    rectangle->p2.y = page_height - poppler_rectangle->y2;

    *results = zathura_list_prepend(*results, rectangle);
  }

  *results = zathura_list_reverse(*results);

  return error;

error_free:

  zathura_list_free_full(*results, free);
  *results = NULL;

error_out:

  return error;
}
