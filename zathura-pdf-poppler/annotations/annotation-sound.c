/* See LICENSE file for license and copyright information */

#include "annotations.h"

zathura_error_t
poppler_annotation_to_zathura_annotation_sound(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position)
{
  (void) poppler_annotation;
  (void) position;
  (void) page;

  return zathura_annotation_sound_set_icon_name(annotation, "Speaker");
}
