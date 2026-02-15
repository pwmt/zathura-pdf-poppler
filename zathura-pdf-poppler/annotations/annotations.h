/* See LICENSE file for license and copyright information */

#ifndef ZATHURA_PDF_POPPLER_ANNOTATIONS_H
#define ZATHURA_PDF_POPPLER_ANNOTATIONS_H

#include "../macros.h"
#include <libzathura/plugin-api.h>
#include <poppler.h>

zathura_error_t poppler_annotation_to_zathura_annotation_unknown(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_text(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_link(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_free_text(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_line(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_square(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_circle(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_polygon(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_poly_line(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_highlight(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_underline(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_squiggly(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_strike_out(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_stamp(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_caret(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_ink(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_popup(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_file_attachment(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_sound(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_movie(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_widget(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_screen(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_printer_mark(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_trap_net(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_watermark(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);
zathura_error_t poppler_annotation_to_zathura_annotation_3d(zathura_annotation_t* annotation,
    PopplerAnnot* poppler_annotation, zathura_page_t* page, zathura_rectangle_t position);

#endif
