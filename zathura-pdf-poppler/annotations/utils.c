/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "utils.h"

zathura_list_t*
create_quad_points_from_text_markup_annotation(PopplerAnnot* poppler_annotation, zathura_page_t* page,
    zathura_rectangle_t position)
{
  unsigned int page_height = 0;
  if (zathura_page_get_height(page, &page_height) != ZATHURA_ERROR_OK) {
    return NULL;
  }

  PopplerAnnotTextMarkup* poppler_annotation_text_markup = POPPLER_ANNOT_TEXT_MARKUP(poppler_annotation);
  GArray* quadrilaterals = poppler_annot_text_markup_get_quadrilaterals(poppler_annotation_text_markup);

  zathura_list_t* list = NULL;

  for (unsigned int i = 0; i < quadrilaterals->len; i++) {
    PopplerQuadrilateral* quadrilateral = &g_array_index(quadrilaterals, PopplerQuadrilateral, i);

    zathura_quad_point_t* quad_point = calloc(1, sizeof(zathura_quad_point_t));
    if (quad_point == NULL) {
      continue;
    }

    quad_point->p1.x = quadrilateral->p1.x - position.p1.x;
    quad_point->p1.y = (page_height - quadrilateral->p1.y) - position.p1.y;
    quad_point->p2.x = quadrilateral->p2.x - position.p1.x;
    quad_point->p2.y = (page_height - quadrilateral->p2.y) - position.p1.y;
    quad_point->p3.x = quadrilateral->p3.x - position.p1.x;
    quad_point->p3.y = (page_height - quadrilateral->p3.y) - position.p1.y;
    quad_point->p4.x = quadrilateral->p4.x - position.p1.x;
    quad_point->p4.y = (page_height - quadrilateral->p4.y) - position.p1.y;

    list = zathura_list_append(list, quad_point);
  }

  g_array_unref(quadrilaterals);

  return list;
}
