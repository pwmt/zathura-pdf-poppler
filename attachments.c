/* See LICENSE file for license and copyright information */

#include <girara/utils.h>

#include "plugin.h"

girara_list_t*
pdf_document_attachments_get(zathura_document_t* document, PopplerDocument* poppler_document, zathura_error_t* error)
{
  if (document == NULL || poppler_document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    girara_warning("PDF file has no attachments");
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    return NULL;
  }

  girara_list_t* res = girara_sorted_list_new2((girara_compare_function_t) g_strcmp0,
      (girara_free_function_t) g_free);
  if (res == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  for (GList* attachments = attachment_list; attachments != NULL; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    girara_list_append(res, g_strdup(attachment->name));
  }

  return res;
}

zathura_error_t
pdf_document_attachment_save(zathura_document_t* document,
    PopplerDocument* poppler_document, const char* attachmentname, const char* file)
{
  if (document == NULL || poppler_document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    girara_warning("PDF file has no attachments");
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }


  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  for (GList* attachments = attachment_list; attachments != NULL; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    if (g_strcmp0(attachment->name, attachmentname) != 0) {
      continue;
    }

    return poppler_attachment_save(attachment, file, NULL);
  }

  return ZATHURA_ERROR_OK;
}
