/* See LICENSE file for license and copyright information */

#include "utils.h"

zathura_error_t
poppler_form_field_button_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

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

  if (zathura_form_field_button_set_type(form_field, button_type) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_button_set_state(form_field,
          poppler_form_field_button_get_state(poppler_form_field) == TRUE ? true : false) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  return ZATHURA_ERROR_OK;
}

zathura_error_t
zathura_form_field_button_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_form_field_button_type_t button_type;
  if (zathura_form_field_button_get_type(form_field, &button_type) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  bool state;
  if (zathura_form_field_button_get_state(form_field, &state) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  poppler_form_field_button_set_state(poppler_form_field, state);

  return ZATHURA_ERROR_OK;
}
