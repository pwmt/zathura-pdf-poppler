/* See LICENSE file for license and copyright information */

#include "plugin.h"
#include "utils.h"

static void pdf_zathura_image_free(zathura_image_t* image);

girara_list_t*
pdf_page_images_get(zathura_page_t* page, PopplerPage* poppler_page, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  girara_list_t* list  = NULL;
  GList* image_mapping = NULL;

  image_mapping = poppler_page_get_image_mapping(poppler_page);
  if (image_mapping == NULL || g_list_length(image_mapping) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  list = girara_list_new();
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  girara_list_set_free_function(list, (girara_free_function_t) pdf_zathura_image_free);

  for (GList* image = image_mapping; image != NULL; image = g_list_next(image)) {
    zathura_image_t* zathura_image = g_malloc0(sizeof(zathura_image_t));

    PopplerImageMapping* poppler_image = (PopplerImageMapping*) image->data;

    /* extract id */
    zathura_image->data = g_malloc(sizeof(gint));

    gint* image_id = zathura_image->data;
    *image_id = poppler_image->image_id;

    /* extract position */
    zathura_image->position.x1 = poppler_image->area.x1;
    zathura_image->position.x2 = poppler_image->area.x2;
    zathura_image->position.y1 = poppler_image->area.y1;
    zathura_image->position.y2 = poppler_image->area.y2;

    girara_list_append(list, zathura_image);
  }

  poppler_page_free_image_mapping(image_mapping);

  return list;

error_free:

  if (list != NULL) {
    girara_list_free(list);
  }

  if (image_mapping != NULL) {
    poppler_page_free_image_mapping(image_mapping);
  }

error_ret:

  return NULL;
}

cairo_surface_t*
pdf_page_image_get_cairo(zathura_page_t* page, PopplerPage* poppler_page,
    zathura_image_t* image, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL || image == NULL || image->data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  gint* image_id = (gint*) image->data;

  cairo_surface_t* surface = poppler_page_get_image(poppler_page, *image_id);
  if (surface == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_ret;
  }

  return surface;

error_ret:

  return NULL;
}

static void
pdf_zathura_image_free(zathura_image_t* image)
{
  if (image == NULL) {
    return;
  }

  if (image->data != NULL) {
    g_free(image->data);
  }

  g_free(image);
}
