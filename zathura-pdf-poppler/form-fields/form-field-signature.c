/* See LICENSE file for license and copyright information */

#include "utils.h"

zathura_error_t
poppler_form_field_signature_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  return ZATHURA_ERROR_PLUGIN_NOT_IMPLEMENTED;
}

zathura_error_t
zathura_form_field_signature_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field)
{
  if (form_field == NULL || poppler_form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  return ZATHURA_ERROR_OK;
}
