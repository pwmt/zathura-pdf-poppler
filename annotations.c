/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara/datastructures.h>
#include <girara/utils.h>
#include <poppler/glib/poppler.h>

#include "pdf.h"

static zathura_annotation_t*
zathura_annotation_from_poppler_annotation(zathura_page_t* page, PopplerAnnot*
    poppler_annotation);

static zathura_annotation_type_t
poppler_annot_get_zathura_annot_type(PopplerAnnot* poppler_annotation);

girara_list_t*
pdf_page_get_annotations(zathura_page_t* page, PopplerPage* poppler_page,
    zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  girara_list_t* list = girara_list_new();
  if (list == NULL) {
    return NULL;
  }

  /* annotations */
  GList* results = poppler_page_get_annot_mapping(poppler_page);
  for (GList* entry = results; entry != NULL; entry = entry->next) {
    PopplerAnnotMapping* mapping = (PopplerAnnotMapping*) entry->data;

    PopplerAnnot* poppler_annotation = (PopplerAnnot*) mapping->annot;
    if (poppler_annotation == NULL) {
      continue;
    }

    zathura_annotation_t* annotation =
      zathura_annotation_from_poppler_annotation(page, poppler_annotation);

    if (annotation != NULL) {
      girara_list_append(list, annotation);
    }
  }
  poppler_page_free_annot_mapping(results);

  return list;
}

static zathura_annotation_t*
zathura_annotation_from_poppler_annotation(zathura_page_t* page, PopplerAnnot* poppler_annotation)
{
  /* get type */
  zathura_annotation_type_t type =
    poppler_annot_get_zathura_annot_type(poppler_annotation);

  /* create annotation */
  zathura_annotation_t* annotation = zathura_annotation_new(type);
  if (annotation == NULL) {
    return NULL;
  }

  /* set default values */
  zathura_annotation_set_name(annotation,    poppler_annot_get_name(poppler_annotation));
  zathura_annotation_set_content(annotation, poppler_annot_get_contents(poppler_annotation));
  zathura_annotation_set_data(annotation,    poppler_annotation);

  /* set type specific values */
  if (type == ZATHURA_ANNOTATION_MARKUP) {
    PopplerAnnotMarkup* annot_markup = POPPLER_ANNOT_MARKUP(poppler_annotation);
    PopplerRectangle annot_rectangle;

    if (poppler_annot_markup_get_popup_rectangle(annot_markup, &annot_rectangle) == TRUE) {
      zathura_rectangle_t rectangle;
      rectangle.x1 = annot_rectangle.x1;
      rectangle.x2 = annot_rectangle.x2;
      rectangle.y1 = zathura_page_get_height(page) - annot_rectangle.y2;
      rectangle.y2 = zathura_page_get_height(page) - annot_rectangle.y1;

      zathura_annotation_set_position(annotation, rectangle);
    }
  }

  return annotation;
}

static zathura_annotation_type_t
poppler_annot_get_zathura_annot_type(PopplerAnnot* poppler_annotation)
{
  if (poppler_annotation == NULL) {
    return ZATHURA_ANNOTATION_UNKNOWN;
  }

  zathura_annotation_type_t annotation_type = ZATHURA_ANNOTATION_UNKNOWN;
  switch (poppler_annot_get_annot_type(poppler_annotation)) {
    case POPPLER_ANNOT_UNKNOWN:
      annotation_type = ZATHURA_ANNOTATION_UNKNOWN;
      break;
    case POPPLER_ANNOT_TEXT:
      if (POPPLER_IS_ANNOT_MARKUP(poppler_annotation) == TRUE) {
        annotation_type = ZATHURA_ANNOTATION_MARKUP;
      } else {
        annotation_type = ZATHURA_ANNOTATION_TEXT;
      }
      break;
    case POPPLER_ANNOT_LINK:
      annotation_type = ZATHURA_ANNOTATION_LINK;
      break;
    case POPPLER_ANNOT_FREE_TEXT:
      annotation_type = ZATHURA_ANNOTATION_FREE_TEXT;
      break;
    case POPPLER_ANNOT_LINE:
      annotation_type = ZATHURA_ANNOTATION_LINE;
      break;
    case POPPLER_ANNOT_SQUARE:
      annotation_type = ZATHURA_ANNOTATION_SQUARE;
      break;
    case POPPLER_ANNOT_CIRCLE:
      annotation_type = ZATHURA_ANNOTATION_CIRCLE;
      break;
    case POPPLER_ANNOT_POLYGON:
      annotation_type = ZATHURA_ANNOTATION_POLYGON;
      break;
    case POPPLER_ANNOT_POLY_LINE:
      annotation_type = ZATHURA_ANNOTATION_POLY_LINE;
      break;
    case POPPLER_ANNOT_HIGHLIGHT:
      annotation_type = ZATHURA_ANNOTATION_HIGHLIGHT;
      break;
    case POPPLER_ANNOT_UNDERLINE:
      annotation_type = ZATHURA_ANNOTATION_UNDERLINE;
      break;
    case POPPLER_ANNOT_SQUIGGLY:
      annotation_type = ZATHURA_ANNOTATION_SQUIGGLY;
      break;
    case POPPLER_ANNOT_STRIKE_OUT:
      annotation_type = ZATHURA_ANNOTATION_STRIKE_OUT;
      break;
    case POPPLER_ANNOT_STAMP:
      annotation_type = ZATHURA_ANNOTATION_STAMP;
      break;
    case POPPLER_ANNOT_CARET:
      annotation_type = ZATHURA_ANNOTATION_CARET;
      break;
    case POPPLER_ANNOT_INK:
      annotation_type = ZATHURA_ANNOTATION_INK;
      break;
    case POPPLER_ANNOT_POPUP:
      annotation_type = ZATHURA_ANNOTATION_POPUP;
      break;
    case POPPLER_ANNOT_FILE_ATTACHMENT:
      annotation_type = ZATHURA_ANNOTATION_FILE_ATTACHMENT;
      break;
    case POPPLER_ANNOT_SOUND:
      annotation_type = ZATHURA_ANNOTATION_SOUND;
      break;
    case POPPLER_ANNOT_MOVIE:
      annotation_type = ZATHURA_ANNOTATION_MOVIE;
      break;
    case POPPLER_ANNOT_WIDGET:
      annotation_type = ZATHURA_ANNOTATION_WIDGET;
      break;
    case POPPLER_ANNOT_SCREEN:
      annotation_type = ZATHURA_ANNOTATION_SCREEN;
      break;
    case POPPLER_ANNOT_PRINTER_MARK:
      annotation_type = ZATHURA_ANNOTATION_PRINTER_MARK;
      break;
    case POPPLER_ANNOT_TRAP_NET:
      annotation_type = ZATHURA_ANNOTATION_TRAP_NET;
      break;
    case POPPLER_ANNOT_WATERMARK:
      annotation_type = ZATHURA_ANNOTATION_WATERMARK;
      break;
    case POPPLER_ANNOT_3D:
      annotation_type = ZATHURA_ANNOTATION_3D;
      break;
    default:
      annotation_type = ZATHURA_ANNOTATION_UNKNOWN;
      break;
  }

  return annotation_type;
}
