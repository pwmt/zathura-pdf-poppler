/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>

#include <girara/datastructures.h>
#include <girara/utils.h>
#include <poppler/glib/poppler.h>

#include "pdf.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef struct zathura_poppler_text_icon_s {
  const char* poppler;
  zathura_annotation_text_icon_t zathura;
} zathura_poppler_text_icon_t;

static const zathura_poppler_text_icon_t zathura_poppler_text_icon_mapping[] = {
  { POPPLER_ANNOT_TEXT_ICON_NOTE,          ZATHURA_ANNOTATION_TEXT_ICON_NOTE },
  { POPPLER_ANNOT_TEXT_ICON_COMMENT,       ZATHURA_ANNOTATION_TEXT_ICON_COMMENT },
  { POPPLER_ANNOT_TEXT_ICON_KEY,           ZATHURA_ANNOTATION_TEXT_ICON_KEY },
  { POPPLER_ANNOT_TEXT_ICON_HELP,          ZATHURA_ANNOTATION_TEXT_ICON_HELP },
  { POPPLER_ANNOT_TEXT_ICON_NEW_PARAGRAPH, ZATHURA_ANNOTATION_TEXT_ICON_NEW_PARAGRAPH },
  { POPPLER_ANNOT_TEXT_ICON_PARAGRAPH,     ZATHURA_ANNOTATION_TEXT_ICON_PARAGRAPH },
  { POPPLER_ANNOT_TEXT_ICON_INSERT,        ZATHURA_ANNOTATION_TEXT_ICON_INSERT },
  { POPPLER_ANNOT_TEXT_ICON_CROSS,         ZATHURA_ANNOTATION_TEXT_ICON_CROSS },
  { POPPLER_ANNOT_TEXT_ICON_CIRCLE,        ZATHURA_ANNOTATION_TEXT_ICON_CIRCLE }
};

typedef struct zathura_poppler_annotation_type_s {
  PopplerAnnotType poppler;
  zathura_annotation_type_t zathura;
} zathura_poppler_annotation_type_t;

static const zathura_poppler_annotation_type_t zathura_poppler_annotation_type_mapping[] = {
  { POPPLER_ANNOT_UNKNOWN,         ZATHURA_ANNOTATION_UNKNOWN },
  { POPPLER_ANNOT_TEXT,            ZATHURA_ANNOTATION_TEXT },
  { POPPLER_ANNOT_LINK,            ZATHURA_ANNOTATION_LINK },
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
  { POPPLER_ANNOT_3D,              ZATHURA_ANNOTATION_3D },
  { POPPLER_ANNOT_TEXT,            ZATHURA_ANNOTATION_MARKUP }
};

typedef struct zathura_poppler_text_state_s {
  PopplerAnnotTextState poppler;
  zathura_annotation_text_state_t zathura;
} zathura_poppler_text_state_t;

static const zathura_poppler_text_state_t zathura_poppler_text_state_mapping[] = {
 { POPPLER_ANNOT_TEXT_STATE_UNKNOWN,   ZATHURA_ANNOTATION_TEXT_STATE_UNKNOWN },
 { POPPLER_ANNOT_TEXT_STATE_NONE,      ZATHURA_ANNOTATION_TEXT_STATE_NONE },
 { POPPLER_ANNOT_TEXT_STATE_MARKED,    ZATHURA_ANNOTATION_TEXT_STATE_MARKED },
 { POPPLER_ANNOT_TEXT_STATE_UNMARKED,  ZATHURA_ANNOTATION_TEXT_STATE_UNMARKED, },
 { POPPLER_ANNOT_TEXT_STATE_ACCEPTED,  ZATHURA_ANNOTATION_TEXT_STATE_ACCEPTED },
 { POPPLER_ANNOT_TEXT_STATE_REJECTED,  ZATHURA_ANNOTATION_TEXT_STATE_REJECTED },
 { POPPLER_ANNOT_TEXT_STATE_CANCELLED, ZATHURA_ANNOTATION_TEXT_STATE_CANCELLED },
 { POPPLER_ANNOT_TEXT_STATE_COMPLETED, ZATHURA_ANNOTATION_TEXT_STATE_COMPLETED }
};

#define ANNOTATION_ICON_SIZE 15

static zathura_annotation_t*
zathura_annotation_from_poppler_annotation(zathura_page_t* page,
    PopplerAnnotMapping* mapping);
static PopplerAnnot* poppler_annotation_from_zathura_annotation(zathura_page_t*
    page, zathura_annotation_t* annotation);
static zathura_annotation_type_t
poppler_annot_get_zathura_annot_type(PopplerAnnot* poppler_annotation);
static PopplerAnnotType
zathura_annot_get_poppler_annot_type(zathura_annotation_t* annotation);
static zathura_annotation_text_icon_t
poppler_get_zathura_text_icon(const char* icon);
static const char*
zathura_get_poppler_text_icon(zathura_annotation_text_icon_t icon);
static zathura_annotation_text_state_t
poppler_get_zathura_text_state(PopplerAnnotTextState state);
static PopplerAnnotTextState
zathura_get_poppler_text_state(zathura_annotation_text_state_t state);

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
    zathura_annotation_t* annotation =
      zathura_annotation_from_poppler_annotation(page, entry->data);

    if (annotation == NULL) {
      continue;
    }

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
zathura_annotation_from_poppler_annotation(zathura_page_t* page, PopplerAnnotMapping* mapping)
{
  if (page == NULL || mapping == NULL || mapping->annot == NULL) {
    return NULL;
  }

  PopplerAnnot* poppler_annotation = (PopplerAnnot*) mapping->annot;

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
  zathura_annotation_set_page(annotation,    page);
  zathura_annotation_set_data(annotation,    poppler_annotation);
  /*zathura_annotation_set_flags();*/
  /*zathura_annotation_set_modified();*/

  zathura_rectangle_t position;
  position.x1 = mapping->area.x1;
  position.x2 = mapping->area.x2;
  position.y1 = zathura_page_get_height(page) - mapping->area.y2;
  position.y2 = zathura_page_get_height(page) - mapping->area.y1;
  zathura_annotation_set_position(annotation, position);

  /* set type specific values */
  if (type == ZATHURA_ANNOTATION_MARKUP) {
    PopplerAnnotMarkup* annot_markup = POPPLER_ANNOT_MARKUP(poppler_annotation);

    zathura_annotation_markup_set_subject(annotation, poppler_annot_markup_get_subject(annot_markup));
    zathura_annotation_markup_set_label(annotation,   poppler_annot_markup_get_label(annot_markup));

    /* popup */
    if (poppler_annot_markup_has_popup(annot_markup) == TRUE) {
      zathura_annotation_popup_t* popup = zathura_annotation_popup_new();
      if (popup == NULL) {
        zathura_annotation_free(annotation);
        return NULL;
      }

      PopplerRectangle annot_rectangle;
      poppler_annot_markup_get_popup_rectangle(annot_markup, &annot_rectangle);

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

  if (type == ZATHURA_ANNOTATION_MARKUP || type == ZATHURA_ANNOTATION_TEXT) {
    PopplerAnnotText* annot_text = POPPLER_ANNOT_TEXT(poppler_annotation);

    zathura_annotation_text_icon_t icon = poppler_get_zathura_text_icon(
        poppler_annot_text_get_icon(annot_text));
    zathura_annotation_text_set_icon(annotation, icon);

    PopplerAnnotTextState poppler_state = poppler_annot_text_get_state(annot_text);
    zathura_annotation_text_state_t state = poppler_get_zathura_text_state(poppler_state);
    zathura_annotation_text_set_state(annotation, state);

    gboolean open_status = poppler_annot_text_get_is_open(annot_text);
    zathura_annotation_text_set_open_status(annotation, (open_status == TRUE) ? true : FALSE);
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

  /* set icon */
  zathura_annotation_text_icon_t icon = zathura_annotation_text_get_icon(annotation);
  if (icon != ZATHURA_ANNOTATION_TEXT_ICON_UNKNOWN) {
    const char* poppler_icon = zathura_get_poppler_text_icon(icon);
    poppler_annot_text_set_icon(POPPLER_ANNOT_TEXT(poppler_annotation), poppler_icon);
  }

  /* set state */
  /* TODO: poppler does not provide a set method right now */
  /*zathura_annotation_text_state_t state = zathura_annotation_text_get_state(annotation);*/

  /* set open status */
  bool opened = zathura_annotation_text_get_open_status(annotation);
  poppler_annot_text_set_is_open(POPPLER_ANNOT_TEXT(poppler_annotation), (opened
        == true) ? TRUE : FALSE);

  /* set markup information */
  if (POPPLER_IS_ANNOT_MARKUP(poppler_annotation) == TRUE
      && zathura_annotation_get_type(annotation) == ZATHURA_ANNOTATION_MARKUP) {
    /* set label */
    char* label = zathura_annotation_markup_get_label(annotation);
    if (label != NULL) {
      poppler_annot_markup_set_label(POPPLER_ANNOT_MARKUP(poppler_annotation), label);
    }

    /* set creation date */
    /* TODO: poppler does not provide a set method right now */
    /*time_t creation_date = zathura_annotation_markup_get_creation_date(annotation);*/

    zathura_annotation_popup_t* popup = zathura_annotation_markup_get_popup(annotation);
    if (popup != NULL) {
      /* set popup */
      zathura_rectangle_t position = zathura_annotation_popup_get_position(popup);
      PopplerRectangle popup_rectangle;
      popup_rectangle.x1 = position.x1;
      popup_rectangle.x2 = position.x2;
      popup_rectangle.y1 = zathura_page_get_height(page) - position.y1;
      popup_rectangle.y2 = zathura_page_get_height(page) - position.y2;

      poppler_annot_markup_set_popup(POPPLER_ANNOT_MARKUP(poppler_annotation), &popup_rectangle);

      /*set popup opacity */
      double opacity = zathura_annotation_popup_get_opacity(popup);
      poppler_annot_markup_set_opacity(POPPLER_ANNOT_MARKUP(poppler_annotation), opacity);

      /* set open status */
      bool opened = zathura_annotation_popup_get_open_status(popup);
      poppler_annot_markup_set_popup_is_open(POPPLER_ANNOT_MARKUP(poppler_annotation),
          (opened = true) ? TRUE : FALSE);
    }
  }

  return poppler_annotation;
}

static zathura_annotation_type_t
poppler_annot_get_zathura_annot_type(PopplerAnnot* poppler_annotation)
{
  if (poppler_annotation == NULL) {
    return ZATHURA_ANNOTATION_UNKNOWN;
  }

  /* exception */
  if (POPPLER_IS_ANNOT_MARKUP(poppler_annotation) == TRUE) {
    return ZATHURA_ANNOTATION_MARKUP;
  }

  PopplerAnnotType type = poppler_annot_get_annot_type(poppler_annotation);
  for (unsigned int i = 0; i < LENGTH(zathura_poppler_annotation_type_mapping); i++) {
    if (zathura_poppler_annotation_type_mapping[i].poppler == type) {
      return zathura_poppler_annotation_type_mapping[i].zathura;
    }
  }

  return ZATHURA_ANNOTATION_UNKNOWN;
}

static PopplerAnnotType
zathura_annot_get_poppler_annot_type(zathura_annotation_t* annotation)
{
  if (annotation == NULL) {
    return POPPLER_ANNOT_UNKNOWN;
  }

  zathura_annotation_type_t type = zathura_annotation_get_type(annotation);
  for (unsigned int i = 0; i < LENGTH(zathura_poppler_annotation_type_mapping); i++) {
    if (zathura_poppler_annotation_type_mapping[i].zathura == type) {
      return zathura_poppler_annotation_type_mapping[i].poppler;
    }
  }

  return POPPLER_ANNOT_UNKNOWN;
}

static zathura_annotation_text_icon_t
poppler_get_zathura_text_icon(const char* icon)
{
  if (icon == NULL) {
    return ZATHURA_ANNOTATION_TEXT_ICON_UNKNOWN;
  }

  for (unsigned int i = 0; i < LENGTH(zathura_poppler_text_icon_mapping); i++) {
    if (strcmp(zathura_poppler_text_icon_mapping[i].poppler, icon) == 0) {
      return zathura_poppler_text_icon_mapping[i].zathura;
    }
  }

  return ZATHURA_ANNOTATION_TEXT_ICON_UNKNOWN;
}

static const char*
zathura_get_poppler_text_icon(zathura_annotation_text_icon_t icon)
{
  if (icon == ZATHURA_ANNOTATION_TEXT_ICON_UNKNOWN) {
    return NULL;
  }

  for (unsigned int i = 0; i < LENGTH(zathura_poppler_text_icon_mapping); i++) {
    if (zathura_poppler_text_icon_mapping[i].zathura == icon) {
      return zathura_poppler_text_icon_mapping[i].poppler;
    }
  }

  return NULL;
}

static zathura_annotation_text_state_t
poppler_get_zathura_text_state(PopplerAnnotTextState state)
{
  for (unsigned int i = 0; i < LENGTH(zathura_poppler_text_state_mapping); i++) {
    if (zathura_poppler_text_state_mapping[i].poppler == state) {
      return zathura_poppler_text_state_mapping[i].zathura;
    }
  }

  return ZATHURA_ANNOTATION_TEXT_STATE_UNKNOWN;
}

static PopplerAnnotTextState
zathura_get_poppler_text_state(zathura_annotation_text_state_t state)
{
  for (unsigned int i = 0; i < LENGTH(zathura_poppler_text_state_mapping); i++) {
    if (zathura_poppler_text_state_mapping[i].zathura == state) {
      return zathura_poppler_text_state_mapping[i].poppler;
    }
  }

  return POPPLER_ANNOT_TEXT_STATE_UNKNOWN;
}
