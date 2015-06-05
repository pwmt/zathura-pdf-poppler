/* See LICENSE file for license and copyright information */

#include <iostream>

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

  pdf_document_t* pdf_document;
  if ((error = zathura_document_get_data(document, (void**) &pdf_document)) != ZATHURA_ERROR_OK
      || pdf_document == NULL || pdf_document->poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  PDFDoc* poppler_document = pdf_document->poppler_document;

  /* init poppler data */
  unsigned int index;
  if ((error = zathura_page_get_index(page, &index)) != ZATHURA_ERROR_OK) {
    return error;
  }

  pdf_page_t* pdf_page = new pdf_page_t;
  if (pdf_page == NULL) {
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }


  pdf_page->poppler_page = poppler_document->getPage(index + 1);
  if (pdf_page->poppler_page == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if ((error = zathura_page_set_data(page, pdf_page)) != ZATHURA_ERROR_OK) {
    delete pdf_page;
    return error;
  }

  /* calculate dimensions */
  double width  = pdf_page->poppler_page->getMediaWidth();
  double height = pdf_page->poppler_page->getMediaHeight();

  if ((error = zathura_page_set_width(page, width)) != ZATHURA_ERROR_OK) {
    delete pdf_page;
    return error;
  }

  if ((error = zathura_page_set_height(page, height)) != ZATHURA_ERROR_OK) {
    delete pdf_page;
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

  return ZATHURA_ERROR_OK;
}
