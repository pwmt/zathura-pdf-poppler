/* SPDX-License-Identifier: Zlib */

#include <girara/utils.h>

#include "plugin.h"

girara_list_t* pdf_document_attachments_get(zathura_document_t* document, void* data, zathura_error_t* error) {
  if (document == NULL || data == NULL) {
    zathura_check_set_error(error, ZATHURA_ERROR_INVALID_ARGUMENTS);
    return NULL;
  }

  PopplerDocument* poppler_document = data;
  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    girara_warning("PDF file has no attachments");
    zathura_check_set_error(error, ZATHURA_ERROR_UNKNOWN);
    return NULL;
  }

  girara_list_t* res = girara_sorted_list_new_with_free((girara_compare_function_t)g_strcmp0, g_free);
  if (res == NULL) {
    zathura_check_set_error(error, ZATHURA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }

  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  for (GList* attachments = attachment_list; attachments != NULL; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*)attachments->data;
    girara_list_append(res, g_strdup(attachment->name));
  }

  return res;
}

zathura_error_t pdf_document_attachment_save(zathura_document_t* document, void* data, const char* attachmentname,
                                             const char* file) {
  if (document == NULL || data == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  PopplerDocument* poppler_document = data;
  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    girara_warning("PDF file has no attachments");
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  for (GList* attachments = attachment_list; attachments != NULL; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*)attachments->data;
    if (g_strcmp0(attachment->name, attachmentname) != 0) {
      continue;
    }

    return poppler_attachment_save(attachment, file, NULL) ? ZATHURA_ERROR_OK : ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}
