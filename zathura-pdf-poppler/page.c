/* See LICENSE file for license and copyright information */

#include "plugin.h"

zathura_error_t
pdf_page_init(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  zathura_document_t* document;
  if ((error = zathura_page_get_document(page, &document)) != ZATHURA_ERROR_OK) {
    return error;
  }

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK
      || poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  /* init poppler data */
  unsigned int index;
  if ((error = zathura_page_get_index(page, &index)) != ZATHURA_ERROR_OK) {
    return error;
  }

  PopplerPage* poppler_page = poppler_document_get_page(poppler_document, index);
  if (poppler_page == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if ((error = zathura_page_set_data(page, poppler_page)) != ZATHURA_ERROR_OK) {
    g_object_unref(poppler_page);
    return error;
  }

  /* calculate dimensions */
  double width;
  double height;
  poppler_page_get_size(poppler_page, &width, &height);

  if ((error = zathura_page_set_width(page, width)) != ZATHURA_ERROR_OK) {
    g_object_unref(poppler_page);
    return error;
  }

  if ((error = zathura_page_set_height(page, height)) != ZATHURA_ERROR_OK) {
    g_object_unref(poppler_page);
    return error;
  }

  return error;
}

zathura_error_t
pdf_page_clear(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  PopplerPage* poppler_page;
  if ((error = zathura_page_get_data(page, (void**) &poppler_page)) != ZATHURA_ERROR_OK) {
    return error;
  }

  if (poppler_page != NULL) {
    g_object_unref(poppler_page);
  }

  return ZATHURA_ERROR_OK;
}
