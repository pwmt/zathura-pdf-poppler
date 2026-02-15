/* See LICENSE file for license and copyright information */

#include "utils.h"

zathura_error_t
poppler_form_field_choice_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  PopplerFormChoiceType poppler_choice_type = poppler_form_field_choice_get_choice_type(poppler_form_field);
  zathura_form_field_choice_type_t choice_type = ZATHURA_FORM_FIELD_CHOICE_TYPE_COMBO;

  switch (poppler_choice_type) {
    case POPPLER_FORM_CHOICE_COMBO:
      choice_type = ZATHURA_FORM_FIELD_CHOICE_TYPE_COMBO;
      break;
    case POPPLER_FORM_CHOICE_LIST:
      choice_type = ZATHURA_FORM_FIELD_CHOICE_TYPE_LIST;
      break;
  }

  if (zathura_form_field_choice_set_type(form_field, choice_type) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_choice_set_multiselect(form_field,
          (bool) poppler_form_field_choice_can_select_multiple(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_choice_set_editable(form_field,
          (bool) poppler_form_field_choice_is_editable(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  if (zathura_form_field_choice_set_spell_check(form_field,
          (bool) poppler_form_field_choice_do_spell_check(poppler_form_field)) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  unsigned int number_of_items = poppler_form_field_choice_get_n_items(poppler_form_field);
  for (unsigned int i = 0; i < number_of_items; i++) {
    char* name = poppler_form_field_choice_get_item(poppler_form_field, i);

    zathura_form_field_choice_item_t* item;
    if (zathura_form_field_choice_item_new(form_field, &item, name) != ZATHURA_ERROR_OK) {
      return ZATHURA_ERROR_UNKNOWN;
    }

    if (poppler_form_field_choice_is_item_selected(poppler_form_field, i) == TRUE &&
        zathura_form_field_choice_item_select(item) != ZATHURA_ERROR_OK) {
      return ZATHURA_ERROR_UNKNOWN;
    }
  }

  return ZATHURA_ERROR_OK;
}

zathura_error_t
zathura_form_field_choice_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_list_t* choice_items;
  if (zathura_form_field_choice_get_items(form_field, &choice_items) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  poppler_form_field_choice_unselect_all(poppler_form_field);

  unsigned int i = 0;
  zathura_form_field_choice_item_t* choice_item;
  ZATHURA_LIST_FOREACH(choice_item, choice_items) {
    bool is_selected;
    if (zathura_form_field_choice_item_is_selected(choice_item, &is_selected) != ZATHURA_ERROR_OK) {
      continue;
    }

    if (is_selected == true) {
      poppler_form_field_choice_select_item(poppler_form_field, i);
    }

    i++;
  }

  return ZATHURA_ERROR_OK;
}
