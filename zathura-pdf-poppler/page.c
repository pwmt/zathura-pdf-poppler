/* See LICENSE file for license and copyright information */

#include "plugin.h"

#if 0
zathura_error_t
pdf_page_init(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_document_t* document           = zathura_page_get_document(page);
  PopplerDocument* poppler_document = zathura_document_get_data(document);

  if (poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  /* init poppler data */
  PopplerPage* poppler_page = poppler_document_get_page(poppler_document, zathura_page_get_index(page));

  if (poppler_page == NULL) {
    g_free(poppler_page);
    return ZATHURA_ERROR_UNKNOWN;
  }

  zathura_page_set_data(page, poppler_page);

  /* calculate dimensions */
  double width;
  double height;
  poppler_page_get_size(poppler_page, &width, &height);
  zathura_page_set_width(page, width);
  zathura_page_set_height(page, height);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
pdf_page_clear(zathura_page_t* page, PopplerPage* poppler_page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (poppler_page != NULL) {
    g_object_unref(poppler_page);
  }

  return ZATHURA_ERROR_OK;
}
#endif
