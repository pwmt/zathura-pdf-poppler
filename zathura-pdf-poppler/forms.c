/* SPDX-License-Identifier: Zlib */

#include "plugin.h"

girara_list_t* pdf_page_form_fields_get(zathura_page_t* UNUSED(page), void* UNUSED(poppler_page),
                                        zathura_error_t* error) {
  if (error != NULL) {
    *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
  }
  return NULL;
}
