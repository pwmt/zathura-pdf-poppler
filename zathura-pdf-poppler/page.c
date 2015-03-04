/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "plugin.h"
#include "internal.h"

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

  pdf_page_t* pdf_page = calloc(1, sizeof(pdf_page_t));
  if (pdf_page == NULL) {
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }

  pdf_page->poppler_page = poppler_document_get_page(poppler_document, index);
  if (pdf_page->poppler_page == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if ((error = zathura_page_set_data(page, pdf_page)) != ZATHURA_ERROR_OK) {
    g_object_unref(pdf_page->poppler_page);
    free(pdf_page);
    return error;
  }

  /* calculate dimensions */
  double width;
  double height;
  poppler_page_get_size(pdf_page->poppler_page, &width, &height);

  if ((error = zathura_page_set_width(page, width)) != ZATHURA_ERROR_OK) {
    g_object_unref(pdf_page->poppler_page);
    free(pdf_page);
    return error;
  }

  if ((error = zathura_page_set_height(page, height)) != ZATHURA_ERROR_OK) {
    g_object_unref(pdf_page->poppler_page);
    free(pdf_page);
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

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    return error;
  }

  if (pdf_page->poppler_page != NULL) {
    g_object_unref(pdf_page->poppler_page);
  }

  if (pdf_page->form_field_mapping != NULL) {
    poppler_page_free_form_field_mapping(pdf_page->form_field_mapping);
  }

  return ZATHURA_ERROR_OK;
}
