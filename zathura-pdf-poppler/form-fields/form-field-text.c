/* See LICENSE file for license and copyright information */

#include "utils.h"

zathura_error_t
poppler_form_field_text_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

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

  if (zathura_form_field_text_set_type(form_field, text_type) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_text_set_max_length(form_field,
          (unsigned int) poppler_form_field_text_get_max_len(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  gchar* text = poppler_form_field_text_get_text(poppler_form_field);
  if (text != NULL && zathura_form_field_text_set_text(form_field, text) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_text_set_scroll(form_field,
          (bool) poppler_form_field_text_do_scroll(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_text_set_spell_check(form_field,
          (bool) poppler_form_field_text_do_spell_check(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_text_set_password(form_field,
          (bool) poppler_form_field_text_is_password(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_text_set_rich_text(form_field,
          (bool) poppler_form_field_text_is_rich_text(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}

zathura_error_t
zathura_form_field_text_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  char* text;
  if (zathura_form_field_text_get_text(form_field, &text) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  poppler_form_field_text_set_text(poppler_form_field, text);

  return ZATHURA_ERROR_OK;
}
