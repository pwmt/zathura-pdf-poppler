/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "plugin.h"
#include "internal.h"

zathura_error_t
pdf_page_get_links(zathura_page_t* page, zathura_list_t** links)
{
  if (page == NULL || links == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_PLUGIN_NOT_IMPLEMENTED;

  return error;
}
