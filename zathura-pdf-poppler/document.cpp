/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdio.h>

#include "plugin.h"

zathura_error_t
pdf_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  return error;
}

zathura_error_t
pdf_document_free(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
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
