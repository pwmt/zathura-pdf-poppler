/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "plugin.h"
#include "internal.h"

#if HAVE_CAIRO
zathura_error_t
pdf_image_get_cairo_surface(zathura_image_t* image, cairo_surface_t** surface)
{
  if (image == NULL || surface == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }


  return ZATHURA_ERROR_OK;
}
#endif

zathura_error_t
pdf_page_get_images(zathura_page_t* page, zathura_list_t** images)
{
  if (page == NULL || images == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_PLUGIN_NOT_IMPLEMENTED;

  return error;
}
