/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <stdlib.h>

#include "plugin.h"
#include "internal.h"

zathura_error_t
pdf_page_get_form_fields(zathura_page_t* page, zathura_list_t** form_fields)
{
  if (page == NULL || form_fields == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  return error;
}
