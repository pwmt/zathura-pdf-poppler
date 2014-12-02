/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <girara/utils.h>

#include "plugin.h"
#include "macros.h"
#include "utils.h"

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
