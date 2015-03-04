/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdio.h>

#include "plugin.h"
#include "utils.h"

zathura_error_t
pdf_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  char* path;
  if ((error = zathura_document_get_path(document, &path)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  char* password;
  if ((error = zathura_document_get_password(document, &password)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  /* format path */
  GError* gerror  = NULL;
  char* file_uri = g_filename_to_uri(path, NULL, &gerror);

  if (file_uri == NULL) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_free;
  }

  PopplerDocument* poppler_document =
    poppler_document_new_from_file(file_uri, password, &gerror);

  if (poppler_document == NULL) {
    if(gerror != NULL && gerror->code == POPPLER_ERROR_ENCRYPTED) {
      error = ZATHURA_ERROR_DOCUMENT_WRONG_PASSWORD;
    } else {
      error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  zathura_document_set_data(document, poppler_document);

  zathura_document_set_number_of_pages(document,
      poppler_document_get_n_pages(poppler_document));

  g_free(file_uri);

  return ZATHURA_ERROR_OK;

error_free:

    if (gerror != NULL) {
      g_error_free(gerror);
    }

    if (file_uri != NULL) {
      g_free(file_uri);
    }

  return error;
}

zathura_error_t
pdf_document_free(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  PopplerDocument* poppler_document;
  if (zathura_document_get_data(document, (void**) &poppler_document) == ZATHURA_ERROR_OK) {
    if (poppler_document != NULL) {
      g_object_unref(poppler_document);
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

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK) {
    return error;
  }

  char* file_path = g_filename_to_uri(path, NULL, NULL);
  gboolean ret = poppler_document_save(poppler_document, file_path, NULL);
  g_free(file_path);

  return (ret == true ? ZATHURA_ERROR_OK : ZATHURA_ERROR_UNKNOWN);
}
