/* See LICENSE file for license and copyright information */

#ifndef ZATHURA_PDF_POPPLER_ANNOTATIONS_UTILS_H
#define ZATHURA_PDF_POPPLER_ANNOTATIONS_UTILS_H

#include <libzathura/plugin-api.h>
#include <poppler.h>

zathura_list_t* create_quad_points_from_text_markup_annotation(PopplerAnnot* poppler_annotation, zathura_page_t* page,
    zathura_rectangle_t position);

#endif
