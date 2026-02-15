/* See LICENSE file for license and copyright information */

#include "../utils.h"
#include "annotations.h"

zathura_error_t
poppler_annotation_to_zathura_annotation_file_attachment(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position)
{
  (void) position;
  (void) page;

  PopplerAnnotFileAttachment* poppler_annotation_file_attachment =
    POPPLER_ANNOT_FILE_ATTACHMENT(poppler_annotation);

  PopplerAttachment* poppler_attachment =
    poppler_annot_file_attachment_get_attachment(poppler_annotation_file_attachment);
  if (poppler_attachment == NULL) {
    return ZATHURA_ERROR_OK;
  }

  zathura_attachment_t* attachment;
  zathura_error_t error = zathura_attachment_new(&attachment);
  if (error != ZATHURA_ERROR_OK) {
    return error;
  }

  if (zathura_attachment_set_name(attachment, poppler_attachment->name) != ZATHURA_ERROR_OK) {
    zathura_attachment_free(attachment);
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_attachment_set_user_data(attachment, poppler_attachment, g_object_unref) != ZATHURA_ERROR_OK) {
    zathura_attachment_free(attachment);
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_attachment_set_save_function(attachment, pdf_attachment_save) != ZATHURA_ERROR_OK) {
    zathura_attachment_free(attachment);
    return ZATHURA_ERROR_UNKNOWN;
  }

  char* icon_name = poppler_annot_file_attachment_get_name(poppler_annotation_file_attachment);
  if (icon_name != NULL && zathura_annotation_file_set_icon_name(annotation, icon_name) != ZATHURA_ERROR_OK) {
    zathura_attachment_free(attachment);
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_annotation_file_set_attachment(annotation, attachment) != ZATHURA_ERROR_OK) {
    zathura_attachment_free(attachment);
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}
