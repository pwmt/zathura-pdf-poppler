/* See LICENSE file for license and copyright information */

#include "plugin.h"

#if !defined(HAVE_CAIRO) && POPPLER_CHECK_VERSION(0,18,0)
#error "Cannot render without cairo and poppler >= 0.18"
#endif

#ifdef HAVE_CAIRO
zathura_error_t
pdf_page_render_cairo(zathura_page_t* page, PopplerPage* poppler_page, cairo_t*
    cairo, bool printing)
{
  if (page == NULL || poppler_page == NULL || cairo == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (printing == false) {
    poppler_page_render(poppler_page, cairo);
  } else {
    poppler_page_render_for_printing(poppler_page, cairo);
  }

  return ZATHURA_ERROR_OK;
}
#endif

#if !POPPLER_CHECK_VERSION(0,18,0)
zathura_image_buffer_t*
pdf_page_render(zathura_page_t* page, PopplerPage* poppler_page,
    zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  zathura_document_t* document = zathura_page_get_document(page);
  if (document == NULL) {
    return NULL;
  }

  /* calculate sizes */
  double scale             = zathura_document_get_scale(document);
  unsigned int page_width  = ceil(scale * zathura_page_get_width(page));
  unsigned int page_height = ceil(scale * zathura_page_get_height(page));

  /* create pixbuf */
  GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
      page_width, page_height);

  if (pixbuf == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  poppler_page_render_to_pixbuf(poppler_page, 0, 0, page_width, page_height,
      scale, 0, pixbuf);

  /* create image buffer */
  zathura_image_buffer_t* image_buffer = zathura_image_buffer_create(page_width, page_height);

  if (image_buffer == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    g_object_unref(pixbuf);
    return NULL;
  }

  /* copy buffer */
  guchar* pixels = gdk_pixbuf_get_pixels(pixbuf);
  int rowstride  = gdk_pixbuf_get_rowstride(pixbuf);
  int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

  for (unsigned int y = 0; y < page_height; y++) {
    for (unsigned int x = 0; x < page_width; x++) {
      unsigned char *s = pixels + y * rowstride + x * n_channels;
      guchar* p = image_buffer->data + y * image_buffer->rowstride + x * 3;
      p[0] = s[0];
      p[1] = s[1];
      p[2] = s[2];
    }
  }

  g_object_unref(pixbuf);

  return image_buffer;
}
#endif
