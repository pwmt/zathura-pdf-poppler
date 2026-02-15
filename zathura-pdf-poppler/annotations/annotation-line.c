/* See LICENSE file for license and copyright information */

#include "annotations.h"

zathura_error_t
poppler_annotation_to_zathura_annotation_line(zathura_annotation_t* UNUSED(annotation),
    PopplerAnnot* UNUSED(poppler_annotation), zathura_page_t* UNUSED(page), zathura_rectangle_t UNUSED(position))
{
  return ZATHURA_ERROR_PLUGIN_NOT_IMPLEMENTED;
}
