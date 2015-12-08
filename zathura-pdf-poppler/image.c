/* See LICENSE file for license and copyright information */

#include <stdlib.h>

#include "plugin.h"
#include "utils.h"
#include "internal.h"

typedef struct pdf_image_s {
  PopplerPage* poppler_page;
  gint image_id;
} pdf_image_t;

#if HAVE_CAIRO
zathura_error_t
pdf_image_get_cairo_surface(zathura_image_t* image, cairo_surface_t** surface)
{
  if (image == NULL || surface == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  pdf_image_t* pdf_image;
  if (zathura_image_get_user_data(image, (void**) &pdf_image) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  *surface = poppler_page_get_image(pdf_image->poppler_page, pdf_image->image_id);

  return ZATHURA_ERROR_OK;
}
#endif

zathura_error_t
pdf_page_get_images(zathura_page_t* page, zathura_list_t** images)
{
  if (page == NULL || images == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  *images = NULL;

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_user_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerPage* poppler_page = pdf_page->poppler_page;

  GList* image_mapping = poppler_page_get_image_mapping(poppler_page);
  if (image_mapping == NULL || g_list_length(image_mapping) == 0) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_out;
  }

  for (GList* image = image_mapping; image != NULL; image = g_list_next(image)) {
    PopplerImageMapping* poppler_image = (PopplerImageMapping*) image->data;

    zathura_rectangle_t position = { {poppler_image->area.x1, poppler_image->area.y1},
      {poppler_image->area.x2, poppler_image->area.y2 }};

    zathura_image_t* zathura_image;
    if (zathura_image_new(&zathura_image, position) != ZATHURA_ERROR_OK) {
      error = ZATHURA_ERROR_OUT_OF_MEMORY;
      goto error_free;
    }

    #if HAVE_CAIRO
    if (zathura_image_set_get_cairo_surface_function(zathura_image,
          pdf_image_get_cairo_surface) != ZATHURA_ERROR_OK) {
      zathura_image_free(zathura_image);
      error = ZATHURA_ERROR_UNKNOWN;
      goto error_free;
    }
    #endif

    pdf_image_t* pdf_image = calloc(1, sizeof(pdf_image_t));
    if (pdf_image == NULL) {
      zathura_image_free(zathura_image);
      error = ZATHURA_ERROR_OUT_OF_MEMORY;
      goto error_free;
    }

    pdf_image->poppler_page = poppler_page;
    pdf_image->image_id = poppler_image->image_id;

    if (zathura_image_set_user_data(zathura_image, pdf_image, free) != ZATHURA_ERROR_OK) {
      free(pdf_image);
      zathura_image_free(zathura_image);
      error = ZATHURA_ERROR_UNKNOWN;
      goto error_free;
    }

    *images = zathura_list_append(*images, zathura_image);
  }

  poppler_page_free_image_mapping(image_mapping);

  return error;

error_free:

  zathura_list_free_full(*images, (GDestroyNotify) zathura_image_free);

error_out:

  return error;
}
