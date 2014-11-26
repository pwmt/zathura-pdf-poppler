/* See LICENSE file for license and copyright information */

#include <stdio.h>

#include "plugin.h"

static zathura_error_t
poppler_form_field_to_zathura_form_field(PopplerFormField* poppler_form_field,
    zathura_form_field_t** form_field);

zathura_error_t
pdf_page_get_form_fields(zathura_page_t* page, zathura_list_t** form_fields)
{
  if (page == NULL || form_fields == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  *form_fields = NULL;

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

  GList* form_field_mapping = poppler_page_get_form_field_mapping(poppler_page);
  if (form_field_mapping == NULL || g_list_length(form_field_mapping) == 0) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_free;
  }

  for (GList* form_field = form_field_mapping; form_field != NULL; form_field = g_list_next(form_field)) {
    PopplerFormFieldMapping* poppler_form_field = (PopplerFormFieldMapping*) form_field->data;
    zathura_form_field_t* form_field;
    if (poppler_form_field_to_zathura_form_field(poppler_form_field->field,
          &form_field) == ZATHURA_ERROR_OK) {
      *form_fields = zathura_list_append(*form_fields, form_field);
    }
  }

  poppler_page_free_form_field_mapping(form_field_mapping);

  return error;

error_free:

  if (form_field_mapping != NULL) {
    poppler_page_free_form_field_mapping(form_field_mapping);
  }

error_out:

  return error;
}

static zathura_error_t
poppler_form_field_to_zathura_form_field(PopplerFormField* poppler_form_field,
    zathura_form_field_t** form_field)
{
  PopplerFormFieldType poppler_type = poppler_form_field_get_field_type(poppler_form_field);
  zathura_form_field_type_t zathura_type = ZATHURA_FORM_FIELD_UNKNOWN;

  zathura_error_t error = ZATHURA_ERROR_OK;

  switch (poppler_type) {
    case POPPLER_FORM_FIELD_UNKNOWN:
      zathura_type = ZATHURA_FORM_FIELD_UNKNOWN;
      break;
    case POPPLER_FORM_FIELD_BUTTON:
      zathura_type = ZATHURA_FORM_FIELD_BUTTON;
      break;
    case POPPLER_FORM_FIELD_TEXT:
      zathura_type = ZATHURA_FORM_FIELD_TEXT;
      break;
    case POPPLER_FORM_FIELD_CHOICE:
      zathura_type = ZATHURA_FORM_FIELD_CHOICE;
      break;
    case POPPLER_FORM_FIELD_SIGNATURE:
      zathura_type = ZATHURA_FORM_FIELD_SIGNATURE;
      break;
  }

  /* create new form field */
  if ((error = zathura_form_field_new(form_field, zathura_type)) != ZATHURA_ERROR_OK) {
    return error;
  }

  /* set general properties */
  gchar* name = poppler_form_field_get_name(poppler_form_field);
  if (name != NULL && (error = zathura_form_field_set_name(*form_field, name))
      != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  gchar* partial_name = poppler_form_field_get_partial_name(poppler_form_field);
  if (partial_name != NULL && (error =
        zathura_form_field_set_partial_name(*form_field, partial_name)) !=
      ZATHURA_ERROR_OK) {
    goto error_free;
  }

  gchar* mapping_name = poppler_form_field_get_mapping_name(poppler_form_field);
  if (mapping_name != NULL && (error =
        zathura_form_field_set_mapping_name(*form_field, mapping_name)) !=
      ZATHURA_ERROR_OK) {
    goto error_free;
  }

  gboolean is_read_only = poppler_form_field_is_read_only(poppler_form_field);
  if (is_read_only == TRUE && (error =
        zathura_form_field_set_flags(*form_field, ZATHURA_FORM_FIELD_FLAG_READ_ONLY)) !=
      ZATHURA_ERROR_OK) {
    goto error_free;
  }

  switch (zathura_type) {
    /* button field */
    case ZATHURA_FORM_FIELD_BUTTON:
      {
        PopplerFormButtonType poppler_button_type = poppler_form_field_button_get_button_type(poppler_form_field);
        zathura_form_field_button_type_t button_type = ZATHURA_FORM_FIELD_BUTTON_TYPE_PUSH;

        switch (poppler_button_type) {
          case POPPLER_FORM_BUTTON_PUSH:
            button_type = ZATHURA_FORM_FIELD_BUTTON_TYPE_PUSH;
            break;
          case POPPLER_FORM_BUTTON_CHECK:
            button_type = ZATHURA_FORM_FIELD_BUTTON_TYPE_CHECK;
            break;
          case POPPLER_FORM_BUTTON_RADIO:
            button_type = ZATHURA_FORM_FIELD_BUTTON_TYPE_RADIO;
            break;
        }

        if ((error = zathura_form_field_button_set_type(*form_field, button_type)) != ZATHURA_ERROR_OK) {
          goto error_free;
        }

        if ((error = zathura_form_field_button_set_state(*form_field,
                (poppler_form_field_button_get_state(poppler_form_field) == TRUE) ? true : false))
            != ZATHURA_ERROR_OK) {
          goto error_free;
        }
      }
      break;
    case POPPLER_FORM_FIELD_TEXT:
      {
        PopplerFormTextType poppler_text_type = poppler_form_field_text_get_text_type(poppler_form_field);
        zathura_form_field_text_type_t text_type = ZATHURA_FORM_FIELD_TEXT_TYPE_NORMAL;

        switch (poppler_text_type) {
          case POPPLER_FORM_TEXT_NORMAL:
            text_type = ZATHURA_FORM_FIELD_TEXT_TYPE_NORMAL;
            break;
          case POPPLER_FORM_TEXT_MULTILINE:
            text_type = ZATHURA_FORM_FIELD_TEXT_TYPE_MULTILINE;
            break;
          case POPPLER_FORM_TEXT_FILE_SELECT:
            text_type = ZATHURA_FORM_FIELD_TEXT_TYPE_FILE_SELECT;
            break;
        }

        if ((error = zathura_form_field_text_set_type(*form_field, text_type)) != ZATHURA_ERROR_OK) {
          goto error_free;
        }

        unsigned int max_length = (unsigned int) poppler_form_field_text_get_max_len(poppler_form_field);
        if ((error = zathura_form_field_text_set_max_length(*form_field, max_length)) !=
            ZATHURA_ERROR_OK) {
          goto error_free;
        }

        gchar* text = poppler_form_field_text_get_text(poppler_form_field);
        if (text != NULL && (error =
              zathura_form_field_text_set_text(*form_field, text)) !=
            ZATHURA_ERROR_OK) {
          goto error_free;
        }

        bool do_scroll = (bool) poppler_form_field_text_do_scroll(poppler_form_field);
        if ((error = zathura_form_field_text_set_scroll(*form_field, do_scroll)) !=
            ZATHURA_ERROR_OK) {
          goto error_free;
        }

        bool do_spell_check = (bool) poppler_form_field_text_do_spell_check(poppler_form_field);
        if ((error = zathura_form_field_text_set_spell_check(*form_field, do_spell_check)) !=
            ZATHURA_ERROR_OK) {
          goto error_free;
        }

        bool is_password = (bool) poppler_form_field_text_is_password(poppler_form_field);
        if ((error = zathura_form_field_text_set_password(*form_field, is_password)) !=
            ZATHURA_ERROR_OK) {
          goto error_free;
        }

        bool is_rich_text = (bool) poppler_form_field_text_is_rich_text(poppler_form_field);
        if ((error = zathura_form_field_text_set_rich_text(*form_field, is_rich_text)) !=
            ZATHURA_ERROR_OK) {
          goto error_free;
        }
      }
      break;
  }

  return error;

error_free:

    zathura_form_field_free(*form_field);

error_out:

    return error;
}
