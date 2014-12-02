/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "plugin.h"
#include "utils.h"

static zathura_error_t
poppler_annotation_to_zathura_annotation(PopplerAnnot* poppler_annotation,
    zathura_annotation_t** annotation);

zathura_error_t
pdf_page_get_annotations(zathura_page_t* page, zathura_list_t** annotations)
{
  if (page == NULL || annotations == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  *annotations = NULL;

  zathura_document_t* document;
  if ((error = zathura_page_get_document(page, &document)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerPage* poppler_page;
  if ((error = zathura_page_get_data(page, (void**) &poppler_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK
      || poppler_document == NULL) {
    goto error_out;
  }

  GList* annotation_mapping = poppler_page_get_annot_mapping(poppler_page);
  if (annotation_mapping == NULL || g_list_length(annotation_mapping) == 0) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_free;
  }

  for (GList* annotation = annotation_mapping; annotation != NULL; annotation = g_list_next(annotation)) {
    PopplerAnnotMapping* poppler_annotation = (PopplerAnnotMapping*) annotation->data;
    zathura_annotation_t* annotation;
    if (poppler_annotation_to_zathura_annotation(poppler_annotation->annot,
          &annotation) != ZATHURA_ERROR_OK) {
      continue;
    }

    zathura_rectangle_t position = { {0, 0}, {0, 0} };
    position.p1.x = poppler_annotation->area.x1;
    position.p2.x = poppler_annotation->area.x2;
    position.p1.y = poppler_annotation->area.y2;
    position.p2.y = poppler_annotation->area.y1;

    if ((error = zathura_annotation_set_position(annotation, position)) != ZATHURA_ERROR_OK) {
      break;
    }

    *annotations = zathura_list_append(*annotations, annotation);
  }

  poppler_page_free_annot_mapping(annotation_mapping);

  return error;

error_free:

  if (annotation_mapping != NULL) {
    poppler_page_free_annot_mapping(annotation_mapping);
  }

error_out:

  return error;
}

static zathura_error_t
poppler_annotation_to_zathura_annotation(PopplerAnnot* poppler_annotation,
    zathura_annotation_t** annotation)
{
  PopplerAnnotType poppler_type = poppler_annot_get_annot_type(poppler_annotation);
  zathura_annotation_type_t zathura_type = ZATHURA_ANNOTATION_UNKNOWN;

  zathura_error_t error = ZATHURA_ERROR_OK;

  typedef struct annotation_type_mapping_s {
    PopplerAnnotType poppler;
    zathura_annotation_type_t zathura;
  } annotation_type_mapping_t;

  annotation_type_mapping_t type_mapping[] = {
    { POPPLER_ANNOT_UNKNOWN,         ZATHURA_ANNOTATION_UNKNOWN },
    { POPPLER_ANNOT_TEXT,            ZATHURA_ANNOTATION_TEXT },
    { POPPLER_ANNOT_FREE_TEXT,       ZATHURA_ANNOTATION_FREE_TEXT },
    { POPPLER_ANNOT_LINE,            ZATHURA_ANNOTATION_LINE },
    { POPPLER_ANNOT_SQUARE,          ZATHURA_ANNOTATION_SQUARE },
    { POPPLER_ANNOT_CIRCLE,          ZATHURA_ANNOTATION_CIRCLE },
    { POPPLER_ANNOT_POLYGON,         ZATHURA_ANNOTATION_POLYGON },
    { POPPLER_ANNOT_POLY_LINE,       ZATHURA_ANNOTATION_POLY_LINE },
    { POPPLER_ANNOT_HIGHLIGHT,       ZATHURA_ANNOTATION_HIGHLIGHT },
    { POPPLER_ANNOT_UNDERLINE,       ZATHURA_ANNOTATION_UNDERLINE },
    { POPPLER_ANNOT_SQUIGGLY,        ZATHURA_ANNOTATION_SQUIGGLY },
    { POPPLER_ANNOT_STRIKE_OUT,      ZATHURA_ANNOTATION_STRIKE_OUT },
    { POPPLER_ANNOT_STAMP,           ZATHURA_ANNOTATION_STAMP },
    { POPPLER_ANNOT_CARET,           ZATHURA_ANNOTATION_CARET },
    { POPPLER_ANNOT_INK,             ZATHURA_ANNOTATION_INK },
    { POPPLER_ANNOT_POPUP,           ZATHURA_ANNOTATION_POPUP },
    { POPPLER_ANNOT_FILE_ATTACHMENT, ZATHURA_ANNOTATION_FILE_ATTACHMENT },
    { POPPLER_ANNOT_SOUND,           ZATHURA_ANNOTATION_SOUND },
    { POPPLER_ANNOT_MOVIE,           ZATHURA_ANNOTATION_MOVIE },
    { POPPLER_ANNOT_WIDGET,          ZATHURA_ANNOTATION_WIDGET },
    { POPPLER_ANNOT_SCREEN,          ZATHURA_ANNOTATION_SCREEN },
    { POPPLER_ANNOT_PRINTER_MARK,    ZATHURA_ANNOTATION_PRINTER_MARK },
    { POPPLER_ANNOT_TRAP_NET,        ZATHURA_ANNOTATION_TRAP_NET },
    { POPPLER_ANNOT_WATERMARK,       ZATHURA_ANNOTATION_WATERMARK },
    { POPPLER_ANNOT_3D,              ZATHURA_ANNOTATION_3D }
  };

  for (unsigned int i = 0; i < LENGTH(type_mapping); i++) {
    if (type_mapping[i].poppler == poppler_type) {
      zathura_type = type_mapping[i].zathura;
      break;
    }
  }

  /* create new annotation */
  if ((error = zathura_annotation_new(annotation, zathura_type)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  /* set general properties */
  gchar* content = poppler_annot_get_contents(poppler_annotation);
  if (content != NULL && (error = zathura_annotation_set_content(*annotation, content) != ZATHURA_ERROR_OK)) {
    goto error_free;
  }

  gchar* name = poppler_annot_get_name(poppler_annotation);
  if (name != NULL && (error = zathura_annotation_set_name(*annotation, name) != ZATHURA_ERROR_OK)) {
    goto error_free;
  }

  gchar* modified = poppler_annot_get_modified(poppler_annotation);
  if (modified != NULL) {
    time_t modification_date;
    if (poppler_date_parse(modified, &modification_date) == TRUE && (error =
          zathura_annotation_set_modification_date(*annotation,
            modification_date) != ZATHURA_ERROR_OK)) {
      goto error_free;
    }
  }

  PopplerAnnotFlag poppler_flags = poppler_annot_get_flags(poppler_annotation);
  zathura_annotation_flag_t flags = ZATHURA_ANNOTATION_FLAG_UNDEFINED;

  typedef struct annotation_flag_mapping_s {
    PopplerAnnotFlag poppler;
    zathura_annotation_flag_t zathura;
  } annotation_flag_mapping_t;

  annotation_flag_mapping_t flag_mapping[] = {
    { POPPLER_ANNOT_FLAG_INVISIBLE,       ZATHURA_ANNOTATION_FLAG_INVISIBLE },
    { POPPLER_ANNOT_FLAG_HIDDEN,          ZATHURA_ANNOTATION_FLAG_HIDDEN },
    { POPPLER_ANNOT_FLAG_PRINT,           ZATHURA_ANNOTATION_FLAG_PRINT },
    { POPPLER_ANNOT_FLAG_NO_ZOOM,         ZATHURA_ANNOTATION_FLAG_NO_ZOOM },
    { POPPLER_ANNOT_FLAG_NO_ROTATE,       ZATHURA_ANNOTATION_FLAG_NO_ROTATE },
    { POPPLER_ANNOT_FLAG_NO_VIEW,         ZATHURA_ANNOTATION_FLAG_NO_VIEW },
    { POPPLER_ANNOT_FLAG_READ_ONLY,       ZATHURA_ANNOTATION_FLAG_READ_ONLY },
    { POPPLER_ANNOT_FLAG_LOCKED,          ZATHURA_ANNOTATION_FLAG_LOCKED },
    { POPPLER_ANNOT_FLAG_TOGGLE_NO_VIEW,  ZATHURA_ANNOTATION_FLAG_TOGGLE_NO_VIEW },
    { POPPLER_ANNOT_FLAG_LOCKED_CONTENTS, ZATHURA_ANNOTATION_FLAG_LOCKED_CONTENTS }
  };

  for (unsigned int i = 0; i < LENGTH(flag_mapping); i++) {
    if (poppler_flags & flag_mapping[i].poppler) {
      flags |= flag_mapping[i].zathura;
    }
  }

  if ((error = zathura_annotation_set_flags(*annotation, flags)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  PopplerColor* poppler_color = poppler_annot_get_color(poppler_annotation);
  if (poppler_color != NULL) {
    zathura_annotation_color_t color = {
      ZATHURA_ANNOTATION_COLOR_SPACE_RGB,
      {
        poppler_color->red,
        poppler_color->green,
        poppler_color->blue,
        0
      }
    };

    if ((error = zathura_annotation_set_color(*annotation, color)) != ZATHURA_ERROR_OK) {
      g_free(poppler_color);
      goto error_free;
    }
  }

  /* annotation type dependend properties */
  switch (zathura_type) {
    case ZATHURA_ANNOTATION_UNKNOWN:
      break;
    case ZATHURA_ANNOTATION_TEXT:
      break;
    case ZATHURA_ANNOTATION_LINK:
      break;
    case ZATHURA_ANNOTATION_FREE_TEXT:
      break;
    case ZATHURA_ANNOTATION_LINE:
      break;
    case ZATHURA_ANNOTATION_SQUARE:
      break;
    case ZATHURA_ANNOTATION_CIRCLE:
      break;
    case ZATHURA_ANNOTATION_POLYGON:
      break;
    case ZATHURA_ANNOTATION_POLY_LINE:
      break;
    case ZATHURA_ANNOTATION_HIGHLIGHT:
      break;
    case ZATHURA_ANNOTATION_UNDERLINE:
      break;
    case ZATHURA_ANNOTATION_SQUIGGLY:
      break;
    case ZATHURA_ANNOTATION_STRIKE_OUT:
      break;
    case ZATHURA_ANNOTATION_STAMP:
      break;
    case ZATHURA_ANNOTATION_CARET:
      break;
    case ZATHURA_ANNOTATION_INK:
      break;
    case ZATHURA_ANNOTATION_POPUP:
      break;
    case ZATHURA_ANNOTATION_FILE_ATTACHMENT:
      break;
    case ZATHURA_ANNOTATION_SOUND:
      break;
    case ZATHURA_ANNOTATION_MOVIE:
      break;
    case ZATHURA_ANNOTATION_WIDGET:
      break;
    case ZATHURA_ANNOTATION_SCREEN:
      break;
    case ZATHURA_ANNOTATION_PRINTER_MARK:
      break;
    case ZATHURA_ANNOTATION_TRAP_NET:
      break;
    case ZATHURA_ANNOTATION_WATERMARK:
      break;
    case ZATHURA_ANNOTATION_3D:
      break;
  }

  return error;

error_free:

    zathura_annotation_free(*annotation);

error_out:

    return error;
}
