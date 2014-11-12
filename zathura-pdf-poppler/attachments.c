/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <girara/utils.h>

#include "plugin.h"
#include "macros.h"

zathura_error_t pdf_attachment_save(zathura_attachment_t* UNUSED(attachment), const char* path, void* user_data)
{
    if (poppler_attachment_save((PopplerAttachment*) user_data, path, NULL) ==
        FALSE) {
      return ZATHURA_ERROR_UNKNOWN;
    }

    return ZATHURA_ERROR_OK;
}

static void add_attachment(PopplerAttachment* poppler_attachment, zathura_list_t** list)
{
  zathura_attachment_t* attachment;
  if (zathura_attachment_new(&attachment) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  if (zathura_attachment_set_name(attachment, poppler_attachment->name)
      != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  if (zathura_attachment_set_user_data(attachment, poppler_attachment)
      != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  if (zathura_attachment_set_save_function(attachment, pdf_attachment_save) !=
      ZATHURA_ERROR_OK) {
    goto error_free;
  }

  *list = zathura_list_append(*list, attachment);

  return;

error_free:

    zathura_attachment_free(attachment);

error_out:

    return;
}

zathura_error_t
pdf_document_get_attachments(zathura_document_t* document, zathura_list_t** attachments)
{
  if (document == NULL || attachments == NULL) {
      return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK
      || poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    return ZATHURA_ERROR_DOCUMENT_HAS_NO_ATTACHMENTS;
  }

  *attachments = NULL;

  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  zathura_list_foreach(attachment_list, (GFunc) add_attachment, attachments);

  return error;
}

#if 0
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
  GList* attachments;

  for (attachments = attachment_list; attachments; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    if (g_strcmp0(attachment->name, attachmentname) != 0) {
      continue;
    }

    return poppler_attachment_save(attachment, file, NULL);
  }

  return ZATHURA_ERROR_OK;
}
#endif
