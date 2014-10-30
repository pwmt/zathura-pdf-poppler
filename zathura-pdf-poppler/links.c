/* See LICENSE file for license and copyright information */

#include "plugin.h"
#include "utils.h"

girara_list_t*
pdf_page_links_get(zathura_page_t* page, PopplerPage* poppler_page, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  girara_list_t* list = NULL;
  GList* link_mapping = NULL;

  link_mapping = poppler_page_get_link_mapping(poppler_page);
  if (link_mapping == NULL || g_list_length(link_mapping) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }
  link_mapping = g_list_reverse(link_mapping);

  list = girara_list_new2((girara_free_function_t) zathura_link_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  zathura_document_t* zathura_document = (zathura_document_t*) zathura_page_get_document(page);
  PopplerDocument* poppler_document    = zathura_document_get_data(zathura_document);

  for (GList* link = link_mapping; link != NULL; link = g_list_next(link)) {
    PopplerLinkMapping* poppler_link       = (PopplerLinkMapping*) link->data;

    /* extract position */
    zathura_rectangle_t position = { 0, 0, 0, 0 };
    position.x1 = poppler_link->area.x1;
    position.x2 = poppler_link->area.x2;
    position.y1 = zathura_page_get_height(page) - poppler_link->area.y2;
    position.y2 = zathura_page_get_height(page) - poppler_link->area.y1;

    zathura_link_t* zathura_link =
      poppler_link_to_zathura_link(poppler_document, poppler_link->action,
          position);
    if (zathura_link != NULL) {
      girara_list_append(list, zathura_link);
    }
  }

  poppler_page_free_link_mapping(link_mapping);

  return list;

error_free:

  if (list != NULL) {
    girara_list_free(list);
  }

  if (link_mapping != NULL) {
    poppler_page_free_link_mapping(link_mapping);
  }

error_ret:

  return NULL;
}
