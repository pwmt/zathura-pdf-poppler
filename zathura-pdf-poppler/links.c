/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "plugin.h"
#include "utils.h"
#include "internal.h"

zathura_error_t
pdf_page_get_links(zathura_page_t* page, zathura_list_t** links)
{
  if (page == NULL || links == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  *links = NULL;

  zathura_document_t* document;
  if ((error = zathura_page_get_document(page, &document)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerPage* poppler_page = pdf_page->poppler_page;

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK
      || poppler_document == NULL) {
    goto error_out;
  }

  unsigned int page_height;
  if ((error = zathura_page_get_height(page, &page_height)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  GList* link_mapping = poppler_page_get_link_mapping(poppler_page);
  if (link_mapping == NULL || g_list_length(link_mapping) == 0) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_free;
  }

  link_mapping = g_list_reverse(link_mapping);

  for (GList* link = link_mapping; link != NULL; link = g_list_next(link)) {
    PopplerLinkMapping* poppler_link = (PopplerLinkMapping*) link->data;

    /* extract position */
    zathura_rectangle_t position = { {0, 0}, {0, 0} };
    position.p1.x = poppler_link->area.x1;
    position.p2.x = poppler_link->area.x2;
    position.p1.y = page_height - poppler_link->area.y2;
    position.p2.y = page_height - poppler_link->area.y1;

    zathura_action_t* action = NULL;
    if (poppler_action_to_zathura_action(poppler_document, poppler_link->action, &action)
        != ZATHURA_ERROR_OK) {
      continue;
    }

    zathura_link_mapping_t* link_mapping = calloc(1, sizeof(zathura_link_mapping_t));
    if (link_mapping == NULL) {
      error = ZATHURA_ERROR_OUT_OF_MEMORY;
      goto error_free;
    }

    link_mapping->position = position;
    link_mapping->action = action;

    *links = zathura_list_append(*links, link_mapping);
  }

  poppler_page_free_link_mapping(link_mapping);

  return ZATHURA_ERROR_OK;

error_free:

  if (*links != NULL) {
    zathura_list_free_full(*links, free);
  }

  if (link_mapping != NULL) {
    poppler_page_free_link_mapping(link_mapping);
  }

error_out:

  return error;
}
