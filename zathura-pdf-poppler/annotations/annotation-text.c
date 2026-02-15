/* See LICENSE file for license and copyright information */

#include "annotations.h"

zathura_error_t
poppler_annotation_to_zathura_annotation_text(zathura_annotation_t* annotation, PopplerAnnot* poppler_annotation,
    zathura_page_t* page, zathura_rectangle_t position)
{
  (void) position;
  (void) page;

  PopplerAnnotText* poppler_annotation_text = POPPLER_ANNOT_TEXT(poppler_annotation);

  gchar* icon_text = poppler_annot_text_get_icon(poppler_annotation_text);
  if (icon_text != NULL && zathura_annotation_text_set_icon_name(annotation, icon_text) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  gboolean is_open = poppler_annot_text_get_is_open(poppler_annotation_text);
  if (zathura_annotation_text_set_open(annotation, (is_open == TRUE) ? true : false) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  PopplerAnnotTextState poppler_state = poppler_annot_text_get_state(poppler_annotation_text);
  zathura_annotation_text_state_t state = ZATHURA_ANNOTATION_TEXT_STATE_UNKNOWN;

  switch (poppler_state) {
    case POPPLER_ANNOT_TEXT_STATE_MARKED:
      state = ZATHURA_ANNOTATION_TEXT_STATE_MARKED;
      break;
    case POPPLER_ANNOT_TEXT_STATE_UNMARKED:
      state = ZATHURA_ANNOTATION_TEXT_STATE_UNMARKED;
      break;
    case POPPLER_ANNOT_TEXT_STATE_ACCEPTED:
      state = ZATHURA_ANNOTATION_TEXT_STATE_ACCEPTED;
      break;
    case POPPLER_ANNOT_TEXT_STATE_REJECTED:
      state = ZATHURA_ANNOTATION_TEXT_STATE_REJECTED;
      break;
    case POPPLER_ANNOT_TEXT_STATE_CANCELLED:
      state = ZATHURA_ANNOTATION_TEXT_STATE_CANCELLED;
      break;
    case POPPLER_ANNOT_TEXT_STATE_COMPLETED:
      state = ZATHURA_ANNOTATION_TEXT_STATE_COMPLETED;
      break;
    case POPPLER_ANNOT_TEXT_STATE_NONE:
      state = ZATHURA_ANNOTATION_TEXT_STATE_NONE;
      break;
    case POPPLER_ANNOT_TEXT_STATE_UNKNOWN:
      state = ZATHURA_ANNOTATION_TEXT_STATE_UNKNOWN;
      break;
  }

  if (zathura_annotation_text_set_state(annotation, state) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}
