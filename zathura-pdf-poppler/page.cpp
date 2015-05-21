/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "plugin.h"
#include "internal.h"

zathura_error_t
pdf_page_init(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  return error;
}

zathura_error_t
pdf_page_clear(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  return ZATHURA_ERROR_OK;
}
