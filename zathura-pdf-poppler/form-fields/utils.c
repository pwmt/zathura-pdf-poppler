/* See LICENSE file for license and copyright information */

#include "utils.h"

zathura_form_field_type_t
poppler_form_field_to_zathura_form_field_type(PopplerFormField* poppler_form_field)
{
  if (poppler_form_field == NULL) {
    return ZATHURA_FORM_FIELD_UNKNOWN;
  }

  switch (poppler_form_field_get_field_type(poppler_form_field)) {
    case POPPLER_FORM_FIELD_UNKNOWN:
      return ZATHURA_FORM_FIELD_UNKNOWN;
    case POPPLER_FORM_FIELD_BUTTON:
      return ZATHURA_FORM_FIELD_BUTTON;
    case POPPLER_FORM_FIELD_TEXT:
      return ZATHURA_FORM_FIELD_TEXT;
    case POPPLER_FORM_FIELD_CHOICE:
      return ZATHURA_FORM_FIELD_CHOICE;
    case POPPLER_FORM_FIELD_SIGNATURE:
      return ZATHURA_FORM_FIELD_SIGNATURE;
  }

  return ZATHURA_FORM_FIELD_UNKNOWN;
}
