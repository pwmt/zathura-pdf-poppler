/* See LICENSE file for license and copyright information */

#include "plugin.h"
#include "utils.h"

#if 0
zathura_error_t
pdf_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  /* format path */
  GError* gerror  = NULL;
  char* file_uri = g_filename_to_uri(zathura_document_get_path(document), NULL, &gerror);

  if (file_uri == NULL) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_free;
  }

  PopplerDocument* poppler_document = poppler_document_new_from_file(file_uri,
      zathura_document_get_password(document), &gerror);

  if (poppler_document == NULL) {
    if(gerror != NULL && gerror->code == POPPLER_ERROR_ENCRYPTED) {
      error = ZATHURA_ERROR_INVALID_PASSWORD;
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
pdf_document_free(zathura_document_t* document, PopplerDocument* poppler_document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  PopplerDocument* poppler_document = zathura_document_get_data(

  if (poppler_document != NULL) {
    g_object_unref(poppler_document);
    zathura_document_set_data(document, NULL);
  }

  return ZATHURA_ERROR_OK;
}

zathura_error_t
pdf_document_save_as(zathura_document_t* document, PopplerDocument* poppler_document, const char* path)
{
  if (document == NULL || poppler_document == NULL || path == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  char* file_path = g_strdup_printf("file://%s", path);
  gboolean ret = poppler_document_save(poppler_document, file_path, NULL);
  g_free(file_path);

  return (ret == true ? ZATHURA_ERROR_OK : ZATHURA_ERROR_UNKNOWN);
}
#endif
