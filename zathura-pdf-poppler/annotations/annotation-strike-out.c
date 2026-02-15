/* See LICENSE file for license and copyright information */

#include "utils.h"
#include "annotations.h"

zathura_error_t
poppler_annotation_to_zathura_annotation_strike_out(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position)
{
  zathura_list_t* list = create_quad_points_from_text_markup_annotation(poppler_annotation, page, position);
  if (zathura_annotation_strike_out_set_quad_points(annotation, list) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}
