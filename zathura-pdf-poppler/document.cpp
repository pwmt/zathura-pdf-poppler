/* See LICENSE file for license and copyright information */

#include <iostream>

#include "macros.h"
#include "plugin.h"
#include "internal.h"

zathura_error_t
pdf_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  /* Get path and password of file */
  char* path;
  if ((error = zathura_document_get_path(document, &path)) != ZATHURA_ERROR_OK) {
    return error;
  }

  char* password;
  if ((error = zathura_document_get_password(document, &password)) != ZATHURA_ERROR_OK) {
    return error;
  }

  /* Convert path and password to GooString's */
  GooString* path_goo = new GooString(path);
  if (path == NULL) {
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }

  GooString* password_goo = new GooString(password);
  if (password_goo == NULL) {
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }

  /* Open document */
  PDFDoc* poppler_document = new PDFDoc(path_goo, password_goo, password_goo);
  if (poppler_document == NULL) {
    delete password_goo;
    delete path_goo;
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }

  delete password_goo;

  if (poppler_document->isOk() == gFalse) {
    error = ZATHURA_ERROR_UNKNOWN;

    if (poppler_document->getErrorCode() == errEncrypted) {
      error = ZATHURA_ERROR_DOCUMENT_WRONG_PASSWORD;
    }

    delete poppler_document;
    return error;
  }

  /* Setup document */
  pdf_document_t* pdf_document = new pdf_document_t;
  if (pdf_document == NULL) {
    delete poppler_document;
    return ZATHURA_ERROR_OUT_OF_MEMORY;
  }

  pdf_document->poppler_document = poppler_document;

  zathura_document_set_data(document, pdf_document);

  if (zathura_document_set_number_of_pages(document, poppler_document->getNumPages()) != ZATHURA_ERROR_OK) {
    delete poppler_document;
    delete pdf_document;
    return ZATHURA_ERROR_UNKNOWN;
  }

#if 0
  pdf_document->output_device = new CairoOutputDev();
  if (pdf_document->output_device == NULL) {
    delete poppler_document;
    delete pdf_document;
    return ZATHURA_ERROR_UNKNOWN;
  }

  pdf_document->output_device->startDoc(poppler_document);
#endif

  return error;
}

zathura_error_t
pdf_document_free(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  pdf_document_t* pdf_document;
  if (zathura_document_get_data(document, (void**) &pdf_document) == ZATHURA_ERROR_OK) {
    if (pdf_document != NULL) {
      delete pdf_document->poppler_document;
      delete pdf_document;
      zathura_document_set_data(document, NULL);
    }
  }

  return ZATHURA_ERROR_OK;
}

zathura_error_t
pdf_document_save_as(zathura_document_t* document, const char* path)
{
  if (document == NULL || path == NULL || strlen(path) == 0) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  return error;
}
