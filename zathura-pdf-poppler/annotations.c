/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "plugin.h"
#include "utils.h"
#include "internal.h"

static zathura_error_t poppler_annotation_to_zathura_annotation(PopplerAnnot*
    poppler_annotation, zathura_annotation_t** annotation, zathura_rectangle_t position, double page_height);
static zathura_list_t*
create_quad_points_from_text_markup_annotation(PopplerAnnot*
    poppler_annotation, zathura_rectangle_t position, double page_height);

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

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerPage* poppler_page = pdf_page->poppler_page;

  unsigned int page_height;
  if ((error = zathura_page_get_height(page, &page_height)) != ZATHURA_ERROR_OK) {
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

    /* Position */
    zathura_rectangle_t position = { {0, 0}, {0, 0} };

    position.p1.x = poppler_annotation->area.x1;
    position.p2.x = poppler_annotation->area.x2;
    position.p1.y = page_height - poppler_annotation->area.y2;
    position.p2.y = page_height - poppler_annotation->area.y1;

    /* Annotation data */
    zathura_annotation_t* annotation;
    if (poppler_annotation_to_zathura_annotation(poppler_annotation->annot,
          &annotation, position, page_height) != ZATHURA_ERROR_OK) {
      continue;
    }

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
    zathura_annotation_t** annotation, zathura_rectangle_t position, double page_height)
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

  /* markup annotation */
  bool is_markup;
  if ((error = zathura_annotation_is_markup_annotation(*annotation, &is_markup)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  if (is_markup == true && POPPLER_IS_ANNOT_MARKUP(poppler_annotation) == TRUE) {
    PopplerAnnotMarkup* poppler_annotation_markup = POPPLER_ANNOT_MARKUP(poppler_annotation);
    gchar* label = poppler_annot_markup_get_label(poppler_annotation_markup);
    if (label != NULL && (error =
          zathura_annotation_markup_set_label(*annotation, label) !=
          ZATHURA_ERROR_OK)) {
      goto error_free;
    }

    gchar* subject = poppler_annot_markup_get_subject(poppler_annotation_markup);
    if (subject != NULL && (error =
          zathura_annotation_markup_set_text(*annotation, subject) !=
          ZATHURA_ERROR_OK)) {
      goto error_free;
    }

    GDate* created = poppler_annot_markup_get_date(poppler_annotation_markup);
    if (created != NULL) {
      struct tm cd = { 0 };
      cd.tm_year = g_date_get_year(created);
      cd.tm_mon  = g_date_get_month(created);
      cd.tm_mday = g_date_get_day(created);

      time_t creation_date = mktime(&cd);

      if ((error = zathura_annotation_markup_set_creation_date(*annotation,
              creation_date) != ZATHURA_ERROR_OK)) {
        goto error_free;
      }
    }

    gdouble opacity = poppler_annot_markup_get_opacity(poppler_annotation_markup);
    if ((error = zathura_annotation_markup_set_opacity(*annotation, opacity) !=
          ZATHURA_ERROR_OK)) {
      goto error_free;
    }

    if (poppler_annot_markup_has_popup(poppler_annotation_markup) == TRUE) {
      zathura_annotation_t* popup_annotation;
      if ((error = zathura_annotation_new(&popup_annotation, ZATHURA_ANNOTATION_POPUP)) != ZATHURA_ERROR_OK) {
        goto error_free;
      }

      PopplerRectangle annotation_rectangle;
      if (poppler_annot_markup_get_popup_rectangle(poppler_annotation_markup, &annotation_rectangle) != TRUE) {
        zathura_annotation_free(popup_annotation);
        goto error_free;
      }

      zathura_rectangle_t position = { {0, 0}, {0, 0} };
      position.p1.x = annotation_rectangle.x1;
      position.p2.x = annotation_rectangle.x2;
      position.p1.y = annotation_rectangle.y2;
      position.p2.y = annotation_rectangle.y1;

      if ((error = zathura_annotation_set_position(popup_annotation, position)) != ZATHURA_ERROR_OK) {
        zathura_annotation_free(popup_annotation);
        goto error_free;
      }

      gboolean is_open = poppler_annot_markup_get_popup_is_open(poppler_annotation_markup);
      if ((error = zathura_annotation_popup_set_open(popup_annotation, (is_open
                == TRUE) ? true : false)) != ZATHURA_ERROR_OK) {
          zathura_annotation_free(popup_annotation);
          goto error_free;
      }

      if ((error = zathura_annotation_popup_set_parent(popup_annotation, *annotation)) != ZATHURA_ERROR_OK) {
        zathura_annotation_free(popup_annotation);
        goto error_free;
      }

      if ((error = zathura_annotation_markup_set_popup_annotation(*annotation,
              popup_annotation)) != ZATHURA_ERROR_OK) {
        zathura_annotation_free(popup_annotation);
        goto error_free;
      }
    }

    PopplerAnnotMarkupReplyType poppler_reply_type = poppler_annot_markup_get_reply_to(poppler_annotation_markup);
    zathura_annotation_markup_reply_type_t reply_type = ZATHURA_ANNOTATION_MARKUP_REPLY_TYPE_REPLY;

    switch (poppler_reply_type) {
      case POPPLER_ANNOT_MARKUP_REPLY_TYPE_R:
        reply_type = ZATHURA_ANNOTATION_MARKUP_REPLY_TYPE_REPLY;
        break;
      case POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP:
        reply_type = ZATHURA_ANNOTATION_MARKUP_REPLY_TYPE_GROUP;
        break;
    }

    if ((error = zathura_annotation_markup_set_reply_type(*annotation,
            reply_type)) != ZATHURA_ERROR_OK) {
      goto error_free;
    }
  }

  /* annotation type dependend properties */
  switch (zathura_type) {
    case ZATHURA_ANNOTATION_UNKNOWN:
      break;
    case ZATHURA_ANNOTATION_TEXT:
      {
        PopplerAnnotText* poppler_annotation_text = POPPLER_ANNOT_TEXT(poppler_annotation);

        gchar* icon_text = poppler_annot_text_get_icon(poppler_annotation_text);
        if (icon_text != NULL && (error = zathura_annotation_text_set_icon_name(*annotation, icon_text)) != ZATHURA_ERROR_OK) {
          goto error_free;
        }

        gboolean is_open = poppler_annot_text_get_is_open(poppler_annotation_text);
        if ((error = zathura_annotation_text_set_open(*annotation, (is_open
                  == TRUE) ? true : false)) != ZATHURA_ERROR_OK) {
            goto error_free;
        }

        PopplerAnnotTextState poppler_state = poppler_annot_text_get_state(poppler_annotation_text);
        zathura_annotation_text_state_t state = ZATHURA_ANNOTATION_TEXT_STATE_UNKNOWN;
        switch (poppler_state) {
          case POPPLER_ANNOT_TEXT_STATE_MARKED:
            state = ZATHURA_ANNOTATION_TEXT_STATE_MARKED;
            break;
          case POPPLER_ANNOT_TEXT_STATE_UNMARKED:
            state = ZATHURA_ANNOTATION_TEXT_STATE_UNMARKED;
            break;
          case POPPLER_ANNOT_TEXT_STATE_ACCEPTED:
            state = ZATHURA_ANNOTATION_TEXT_STATE_ACCEPTED;
            break;
          case POPPLER_ANNOT_TEXT_STATE_REJECTED:
            state = ZATHURA_ANNOTATION_TEXT_STATE_REJECTED;
            break;
          case POPPLER_ANNOT_TEXT_STATE_CANCELLED:
            state = ZATHURA_ANNOTATION_TEXT_STATE_CANCELLED;
            break;
          case POPPLER_ANNOT_TEXT_STATE_COMPLETED:
            state = ZATHURA_ANNOTATION_TEXT_STATE_COMPLETED;
            break;
          case POPPLER_ANNOT_TEXT_STATE_NONE:
            state = ZATHURA_ANNOTATION_TEXT_STATE_NONE;
            break;
          case POPPLER_ANNOT_TEXT_STATE_UNKNOWN:
            state = ZATHURA_ANNOTATION_TEXT_STATE_UNKNOWN;
            break;
        }

        if ((error = zathura_annotation_text_set_state(*annotation, state))
              != ZATHURA_ERROR_OK) {
            goto error_free;
        }
      }
      break;
    case ZATHURA_ANNOTATION_LINK:
      break;
    case ZATHURA_ANNOTATION_FREE_TEXT:
      {
        PopplerAnnotFreeText *poppler_annotation_free_text = POPPLER_ANNOT_FREE_TEXT(poppler_annotation);

        /* callout line */
        PopplerAnnotCalloutLine* poppler_callout_line =
          poppler_annot_free_text_get_callout_line(poppler_annotation_free_text);

        if (poppler_callout_line != NULL) {
          zathura_annotation_callout_line_t callout_line = {
            { poppler_callout_line->x1, poppler_callout_line->y1 },
            { poppler_callout_line->x2, poppler_callout_line->y2 },
            { poppler_callout_line->x3, poppler_callout_line->y3 }
          };

          if ((error = zathura_annotation_free_text_set_callout_line(*annotation, callout_line))
                != ZATHURA_ERROR_OK) {
              goto error_free;
          }
        }

        /* quadding */
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

        if ((error = zathura_annotation_free_text_set_justification(*annotation, justification))
              != ZATHURA_ERROR_OK) {
            goto error_free;
        }
      }
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
      {
        zathura_list_t* list = create_quad_points_from_text_markup_annotation(poppler_annotation, position, page_height);
        if (zathura_annotation_highlight_set_quad_points(*annotation, list) != ZATHURA_ERROR_OK) {
          goto error_free;
        }
      }
      break;
    case ZATHURA_ANNOTATION_UNDERLINE:
      {
        zathura_list_t* list = create_quad_points_from_text_markup_annotation(poppler_annotation, position, page_height);
        if (zathura_annotation_underline_set_quad_points(*annotation, list) != ZATHURA_ERROR_OK) {
          goto error_free;
        }
      }
      break;
    case ZATHURA_ANNOTATION_SQUIGGLY:
      {
        zathura_list_t* list = create_quad_points_from_text_markup_annotation(poppler_annotation, position, page_height);
        if (zathura_annotation_squiggly_set_quad_points(*annotation, list) != ZATHURA_ERROR_OK) {
          goto error_free;
        }
      }
      break;
    case ZATHURA_ANNOTATION_STRIKE_OUT:
      {
        zathura_list_t* list = create_quad_points_from_text_markup_annotation(poppler_annotation, position, page_height);
        if (zathura_annotation_strike_out_set_quad_points(*annotation, list) != ZATHURA_ERROR_OK) {
          goto error_free;
        }
      }
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
      {
        PopplerAnnotFileAttachment *poppler_annotation_file_attachment =
          POPPLER_ANNOT_FILE_ATTACHMENT(poppler_annotation);

        PopplerAttachment* poppler_attachment =
          poppler_annot_file_attachment_get_attachment(poppler_annotation_file_attachment);
        if (poppler_attachment != NULL) {
          zathura_attachment_t* attachment;
          if (zathura_attachment_new(&attachment) != ZATHURA_ERROR_OK) {
            goto error_out;
          }

          if (zathura_attachment_set_name(attachment, poppler_attachment->name)
              != ZATHURA_ERROR_OK) {
            zathura_attachment_free(attachment);
            goto error_free;
          }

          if (zathura_attachment_set_user_data(attachment, poppler_attachment)
              != ZATHURA_ERROR_OK) {
            zathura_attachment_free(attachment);
            goto error_free;
          }

          if (zathura_attachment_set_save_function(attachment, pdf_attachment_save) !=
              ZATHURA_ERROR_OK) {
            zathura_attachment_free(attachment);
            goto error_free;
          }

          char* icon_name = poppler_annot_file_attachment_get_name(poppler_annotation_file_attachment);
          if (icon_name != NULL && (error =
                zathura_annotation_file_set_icon_name(*annotation, icon_name)) !=
              ZATHURA_ERROR_OK) {
            goto error_free;
          }

          if (zathura_annotation_file_set_attachment(*annotation, attachment) !=
              ZATHURA_ERROR_OK) {
            zathura_attachment_free(attachment);
            goto error_free;
          }
        }
      }
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

static zathura_list_t*
create_quad_points_from_text_markup_annotation(PopplerAnnot*
    poppler_annotation, zathura_rectangle_t position, double page_height)
{
  PopplerAnnotTextMarkup* poppler_annotation_text_markup = POPPLER_ANNOT_TEXT_MARKUP(poppler_annotation);
  GArray* quadrilaterals = poppler_annot_text_markup_get_quadrilaterals(poppler_annotation_text_markup);
  PopplerQuadrilateral* quadrilateral;

  zathura_list_t* list = NULL;

  for (unsigned int i = 0; i < quadrilaterals->len; i++) {
    quadrilateral = &g_array_index(quadrilaterals, PopplerQuadrilateral, i);

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
