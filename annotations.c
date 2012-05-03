/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara/datastructures.h>
#include <girara/utils.h>
#include <poppler/glib/poppler.h>

#include "pdf.h"

#define ANNOTATION_ICON_SIZE 15

static zathura_annotation_t*
zathura_annotation_from_poppler_annotation(zathura_page_t* page, PopplerAnnot*
    poppler_annotation);
static PopplerAnnot* poppler_annotation_from_zathura_annotation(zathura_page_t*
    page, zathura_annotation_t* annotation);
static zathura_annotation_type_t
poppler_annot_get_zathura_annot_type(PopplerAnnot* poppler_annotation);
static PopplerAnnotType
zathura_annot_get_poppler_annot_type(zathura_annotation_t* annotation);

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

    if (annotation == NULL) {
      continue;
    }

    /* set position */
    zathura_rectangle_t rectangle;
    rectangle.x1 = mapping->area.x1;
    rectangle.x2 = mapping->area.x2;
    rectangle.y1 = zathura_page_get_height(page) - mapping->area.y2;
    rectangle.y2 = zathura_page_get_height(page) - mapping->area.y1;

    zathura_annotation_set_position(annotation, rectangle);

    /* add annotation */
    girara_list_append(list, annotation);
  }
  poppler_page_free_annot_mapping(results);

  return list;
}

zathura_error_t
pdf_page_set_annotations(zathura_page_t* page, PopplerPage* poppler_page,
    girara_list_t* annotations)
{
  if (page == NULL || poppler_page == NULL || annotations == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  GIRARA_LIST_FOREACH(annotations, zathura_annotation_t*, iter, annotation)
    if (annotation != NULL && zathura_annotation_get_data(annotation) == NULL) {
      PopplerAnnot* poppler_annotation = poppler_annotation_from_zathura_annotation(page, annotation);
      if (poppler_annotation != NULL) {
        poppler_page_add_annot(poppler_page, poppler_annotation);
      }
    }
  GIRARA_LIST_FOREACH_END(annotations, zathura_annotation_t*, iter, annotation);

  return ZATHURA_ERROR_OK;
}

static zathura_annotation_t*
zathura_annotation_from_poppler_annotation(zathura_page_t* page, PopplerAnnot* poppler_annotation)
{
  if (page == NULL || poppler_annotation == NULL) {
    return NULL;
  }

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

    zathura_annotation_markup_set_subject(annotation, poppler_annot_markup_get_subject(annot_markup));
    zathura_annotation_markup_set_label(annotation,   poppler_annot_markup_get_label(annot_markup));

    /* popup */
    if (poppler_annot_markup_get_popup_rectangle(annot_markup, &annot_rectangle) == TRUE) {
      zathura_annotation_popup_t* popup = zathura_annotation_popup_new();
      if (popup == NULL) {
        zathura_annotation_free(annotation);
        return NULL;
      }

      zathura_rectangle_t rectangle;
      rectangle.x1 = annot_rectangle.x1;
      rectangle.x2 = annot_rectangle.x2;
      rectangle.y1 = zathura_page_get_height(page) - annot_rectangle.y2;
      rectangle.y2 = zathura_page_get_height(page) - annot_rectangle.y1;

      zathura_annotation_popup_set_position(popup,    rectangle);
      zathura_annotation_popup_set_open_status(popup, poppler_annot_markup_get_popup_is_open(annot_markup));
      zathura_annotation_popup_set_opacity(popup,     poppler_annot_markup_get_opacity(annot_markup));

      zathura_annotation_markup_set_popup(annotation, popup);
    }
  }

  return annotation;
}

static PopplerAnnot*
poppler_annotation_from_zathura_annotation(zathura_page_t* page,
    zathura_annotation_t* annotation)
{
  if (page == NULL || annotation == NULL) {
    return NULL;
  }

  zathura_document_t* document      = zathura_page_get_document(page);
  PopplerDocument* poppler_document = zathura_document_get_data(document);

  if (poppler_document == NULL) {
    return NULL;
  }

  PopplerAnnotType type = zathura_annot_get_poppler_annot_type(annotation);
  if (type != POPPLER_ANNOT_TEXT) {
    return NULL;
  }

  /* set annotation position */
  zathura_rectangle_t position = zathura_annotation_get_position(annotation);
  PopplerRectangle rectangle;
  rectangle.x1 = position.x1 - (ANNOTATION_ICON_SIZE / 2);
  rectangle.x2 = position.x2 + (ANNOTATION_ICON_SIZE / 2);
  rectangle.y1 = zathura_page_get_height(page) - position.y1 + (ANNOTATION_ICON_SIZE / 2);
  rectangle.y2 = zathura_page_get_height(page) - position.y2 - (ANNOTATION_ICON_SIZE / 2);

  /* create new annotation */
  PopplerAnnot* poppler_annotation = poppler_annot_text_new(poppler_document, &rectangle);

  return poppler_annotation;
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

static PopplerAnnotType
zathura_annot_get_poppler_annot_type(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return POPPLER_ANNOT_UNKNOWN;
  }

  PopplerAnnotType annotation_type = POPPLER_ANNOT_UNKNOWN;
  switch (zathura_annotation_get_type(annotation)) {
    case ZATHURA_ANNOTATION_UNKNOWN:
      annotation_type = POPPLER_ANNOT_UNKNOWN;
      break;
    case ZATHURA_ANNOTATION_MARKUP:
      annotation_type = POPPLER_ANNOT_TEXT;
      break;
    case ZATHURA_ANNOTATION_TEXT:
      annotation_type = POPPLER_ANNOT_TEXT;
      break;
    case ZATHURA_ANNOTATION_LINK:
      annotation_type = POPPLER_ANNOT_LINK;
      break;
    case ZATHURA_ANNOTATION_FREE_TEXT:
      annotation_type = POPPLER_ANNOT_FREE_TEXT;
      break;
    case ZATHURA_ANNOTATION_LINE:
      annotation_type = POPPLER_ANNOT_LINE;
      break;
    case ZATHURA_ANNOTATION_SQUARE:
      annotation_type = POPPLER_ANNOT_SQUARE;
      break;
    case ZATHURA_ANNOTATION_CIRCLE:
      annotation_type = POPPLER_ANNOT_CIRCLE;
      break;
    case ZATHURA_ANNOTATION_POLYGON:
      annotation_type = POPPLER_ANNOT_POLYGON;
      break;
    case ZATHURA_ANNOTATION_POLY_LINE:
      annotation_type = POPPLER_ANNOT_POLY_LINE;
      break;
    case ZATHURA_ANNOTATION_HIGHLIGHT:
      annotation_type = POPPLER_ANNOT_HIGHLIGHT;
      break;
    case ZATHURA_ANNOTATION_UNDERLINE:
      annotation_type = POPPLER_ANNOT_UNDERLINE;
      break;
    case ZATHURA_ANNOTATION_SQUIGGLY:
      annotation_type = POPPLER_ANNOT_SQUIGGLY;
      break;
    case ZATHURA_ANNOTATION_STRIKE_OUT:
      annotation_type = POPPLER_ANNOT_STRIKE_OUT;
      break;
    case ZATHURA_ANNOTATION_STAMP:
      annotation_type = POPPLER_ANNOT_STAMP;
      break;
    case ZATHURA_ANNOTATION_CARET:
      annotation_type = POPPLER_ANNOT_CARET;
      break;
    case ZATHURA_ANNOTATION_INK:
      annotation_type = POPPLER_ANNOT_INK;
      break;
    case ZATHURA_ANNOTATION_POPUP:
      annotation_type = POPPLER_ANNOT_POPUP;
      break;
    case ZATHURA_ANNOTATION_FILE_ATTACHMENT:
      annotation_type = POPPLER_ANNOT_FILE_ATTACHMENT;
      break;
    case ZATHURA_ANNOTATION_SOUND:
      annotation_type = POPPLER_ANNOT_SOUND;
      break;
    case ZATHURA_ANNOTATION_MOVIE:
      annotation_type = POPPLER_ANNOT_MOVIE;
      break;
    case ZATHURA_ANNOTATION_WIDGET:
      annotation_type = POPPLER_ANNOT_WIDGET;
      break;
    case ZATHURA_ANNOTATION_SCREEN:
      annotation_type = POPPLER_ANNOT_SCREEN;
      break;
    case ZATHURA_ANNOTATION_PRINTER_MARK:
      annotation_type = POPPLER_ANNOT_PRINTER_MARK;
      break;
    case ZATHURA_ANNOTATION_TRAP_NET:
      annotation_type = POPPLER_ANNOT_TRAP_NET;
      break;
    case ZATHURA_ANNOTATION_WATERMARK:
      annotation_type = POPPLER_ANNOT_WATERMARK;
      break;
    case ZATHURA_ANNOTATION_3D:
      annotation_type = POPPLER_ANNOT_3D;
      break;
    default:
      annotation_type = POPPLER_ANNOT_UNKNOWN;
      break;
  }

  return annotation_type;
}
