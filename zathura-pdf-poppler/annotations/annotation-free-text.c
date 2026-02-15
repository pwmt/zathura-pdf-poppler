/* See LICENSE file for license and copyright information */

#include "annotations.h"

zathura_error_t
poppler_annotation_to_zathura_annotation_free_text(zathura_annotation_t* annotation, PopplerAnnot* poppler_annotation,
    zathura_page_t* page, zathura_rectangle_t position)
{
  (void) position;
  (void) page;

  PopplerAnnotFreeText* poppler_annotation_free_text = POPPLER_ANNOT_FREE_TEXT(poppler_annotation);

  PopplerAnnotCalloutLine* poppler_callout_line =
    poppler_annot_free_text_get_callout_line(poppler_annotation_free_text);

  if (poppler_callout_line != NULL) {
    zathura_annotation_callout_line_t callout_line = {
      { poppler_callout_line->x1, poppler_callout_line->y1 },
      { poppler_callout_line->x2, poppler_callout_line->y2 },
      { poppler_callout_line->x3, poppler_callout_line->y3 }
    };

    if (zathura_annotation_free_text_set_callout_line(annotation, callout_line) != ZATHURA_ERROR_OK) {
      return ZATHURA_ERROR_UNKNOWN;
    }
  }

  PopplerAnnotFreeTextQuadding poppler_quadding =
    poppler_annot_free_text_get_quadding(poppler_annotation_free_text);
  zathura_annotation_justification_t justification = ZATHURA_ANNOTATION_JUSTIFICATION_LEFT_JUSTIFIED;

  switch (poppler_quadding) {
    case POPPLER_ANNOT_FREE_TEXT_QUADDING_LEFT_JUSTIFIED:
      justification = ZATHURA_ANNOTATION_JUSTIFICATION_LEFT_JUSTIFIED;
      break;
    case POPPLER_ANNOT_FREE_TEXT_QUADDING_CENTERED:
      justification = ZATHURA_ANNOTATION_JUSTIFICATION_CENTERED;
      break;
    case POPPLER_ANNOT_FREE_TEXT_QUADDING_RIGHT_JUSTIFIED:
      justification = ZATHURA_ANNOTATION_JUSTIFICATION_RIGHT_JUSTIFIED;
      break;
  }

  if (zathura_annotation_free_text_set_justification(annotation, justification) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}
