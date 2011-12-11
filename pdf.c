/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara/datastructures.h>
#include <string.h>
#include <time.h>
#include <poppler/glib/poppler.h>

#include "pdf.h"

#if !defined(HAVE_CAIRO) && POPPLER_CHECK_VERSION(0,18,0)
#error "Cannot render without cairo and poppler >= 0.18"
#endif

void
plugin_register(zathura_document_plugin_t* plugin)
{
  girara_list_append(plugin->content_types, g_content_type_from_mime_type("application/pdf"));
  plugin->open_function  = pdf_document_open;
}

bool
pdf_document_open(zathura_document_t* document)
{
  if (!document) {
    goto error_out;
  }

  document->functions.document_free             = pdf_document_free;
  document->functions.document_index_generate   = pdf_document_index_generate;;
  document->functions.document_save_as          = pdf_document_save_as;
  document->functions.document_attachments_get  = pdf_document_attachments_get;
  document->functions.document_meta_get         = pdf_document_meta_get;
  document->functions.page_get                  = pdf_page_get;
  document->functions.page_search_text          = pdf_page_search_text;
  document->functions.page_links_get            = pdf_page_links_get;
  document->functions.page_form_fields_get      = pdf_page_form_fields_get;
#if !POPPLER_CHECK_VERSION(0,18,0)
  document->functions.page_render               = pdf_page_render;
#endif
#if HAVE_CAIRO
  document->functions.page_render_cairo         = pdf_page_render_cairo;
#endif
  document->functions.page_free                 = pdf_page_free;

  document->data = malloc(sizeof(pdf_document_t));
  if (!document->data) {
    goto error_out;
  }

  /* format path */
  GError* error  = NULL;
  char* file_uri = g_filename_to_uri(document->file_path, NULL, &error);

  if (!file_uri) {
    fprintf(stderr, "error: could not open file: %s\n", error->message);
    goto error_free;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;
  pdf_document->document       = poppler_document_new_from_file(file_uri, document->password, &error);

  if (!pdf_document->document) {
    fprintf(stderr, "error: could not open file: %s\n", error->message);
    goto error_free;
  }

  document->number_of_pages = poppler_document_get_n_pages(pdf_document->document);

  g_free(file_uri);

  return true;

error_free:

    if (error) {
      g_error_free(error);
    }

    if (file_uri) {
      g_free(file_uri);
    }

    free(document->data);
    document->data = NULL;

error_out:

  return false;
}

bool
pdf_document_free(zathura_document_t* document)
{
  if (!document) {
    return false;
  }

  if (document->data) {
    pdf_document_t* pdf_document = (pdf_document_t*) document->data;
    g_object_unref(pdf_document->document);
    free(document->data);
    document->data = NULL;
  }

  return true;
}

static void
build_index(pdf_document_t* pdf, girara_tree_node_t* root, PopplerIndexIter* iter)
{
  if (!root || !iter) {
    return;
  }

  do {
    PopplerAction* action = poppler_index_iter_get_action(iter);

    if (!action) {
      continue;
    }

    gchar* markup = g_markup_escape_text(action->any.title, -1);
    zathura_index_element_t* index_element = zathura_index_element_new(markup);

    g_free(markup);

    if (index_element == NULL) {
      continue;
    }

    if (action->type == POPPLER_ACTION_URI) {
      index_element->type = ZATHURA_LINK_EXTERNAL;
      index_element->target.uri = g_strdup(action->uri.uri);
    } else if (action->type == POPPLER_ACTION_GOTO_DEST) {
      index_element->type = ZATHURA_LINK_TO_PAGE;

      if (action->goto_dest.dest->type == POPPLER_DEST_NAMED) {
        PopplerDest* dest = poppler_document_find_dest(pdf->document, action->goto_dest.dest->named_dest);
        if (dest) {
          index_element->target.page_number = dest->page_num - 1;
          poppler_dest_free(dest);
        }
      } else {
        index_element->target.page_number = action->goto_dest.dest->page_num - 1;
      }
    } else {
      poppler_action_free(action);
      zathura_index_element_free(index_element);
      continue;
    }

    poppler_action_free(action);

    girara_tree_node_t* node = girara_node_append_data(root, index_element);
    PopplerIndexIter* child  = poppler_index_iter_get_child(iter);

    if (child) {
      build_index(pdf, node, child);
    }

    poppler_index_iter_free(child);

  } while (poppler_index_iter_next(iter));
}

girara_tree_node_t*
pdf_document_index_generate(zathura_document_t* document)
{
  if (!document || !document->data) {
    return NULL;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;
  PopplerIndexIter* iter       = poppler_index_iter_new(pdf_document->document);

  if (!iter) {
    // XXX: error message?
    return NULL;
  }

  girara_tree_node_t* root = girara_node_new(zathura_index_element_new("ROOT"));
  // girara_node_set_free_function(root, (girara_free_function_t) zathura_index_element_free);
  build_index(pdf_document, root, iter);

  poppler_index_iter_free(iter);
  return root;
}

bool
pdf_document_save_as(zathura_document_t* document, const char* path)
{
  if (!document || !document->data || !path) {
    return false;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;

  char* file_path = g_strdup_printf("file://%s", path);
  poppler_document_save(pdf_document->document, file_path, NULL);
  g_free(file_path);

  return false;
}

girara_list_t*
pdf_document_attachments_get(zathura_document_t* document)
{
  return NULL;
}

char*
pdf_document_meta_get(zathura_document_t* document, zathura_document_meta_t meta)
{
  if (document == NULL || document->data == NULL) {
    return NULL;
  }

  pdf_document_t* pdf_document  = (pdf_document_t*) document->data;

  char* string_value;
  char* tmp;
  time_t time_value;

  switch (meta) {
    case ZATHURA_DOCUMENT_TITLE:
      g_object_get(pdf_document->document, "title", &string_value, NULL);
      break;
    case ZATHURA_DOCUMENT_AUTHOR:
      g_object_get(pdf_document->document, "author", &string_value, NULL);
      break;
    case ZATHURA_DOCUMENT_SUBJECT:
      g_object_get(pdf_document->document, "subject", &string_value, NULL);
      break;
    case ZATHURA_DOCUMENT_KEYWORDS:
      g_object_get(pdf_document->document, "keywords", &string_value, NULL);
      break;
    case ZATHURA_DOCUMENT_CREATOR:
      g_object_get(pdf_document->document, "creator", &string_value, NULL);
      break;
    case ZATHURA_DOCUMENT_PRODUCER:
      g_object_get(pdf_document->document, "producer", &string_value, NULL);
      break;
    case ZATHURA_DOCUMENT_CREATION_DATE:
      g_object_get(pdf_document->document, "creation-date", &time_value, NULL);
      tmp = ctime(&time_value);
      if (tmp != NULL) {
        string_value = g_strndup(tmp, strlen(tmp) - 1);
      } else {
        return NULL;
      }
      break;
    case ZATHURA_DOCUMENT_MODIFICATION_DATE:
      g_object_get(pdf_document->document, "mod-date", &time_value, NULL);
      tmp = ctime(&time_value);
      if (tmp != NULL) {
        string_value = g_strndup(tmp, strlen(tmp) - 1);
      } else {
        return NULL;
      }
      break;
    default:
      return NULL;
  }

  if (string_value == NULL || strlen(string_value) == 0) {
    g_free(string_value);
    return NULL;
  }

  return string_value;
}

zathura_page_t*
pdf_page_get(zathura_document_t* document, unsigned int page)
{
  if (!document || !document->data) {
    return NULL;
  }

  pdf_document_t* pdf_document  = (pdf_document_t*) document->data;
  zathura_page_t* document_page = malloc(sizeof(zathura_page_t));

  if (!document_page) {
    return NULL;
  }

  document_page->document = document;
  document_page->data     = poppler_document_get_page(pdf_document->document, page);

  if (!document_page->data) {
    free(document_page);
    return NULL;
  }

  poppler_page_get_size(document_page->data, &(document_page->width), &(document_page->height));

  return document_page;
}

bool
pdf_page_free(zathura_page_t* page)
{
  if (!page) {
    return false;
  }

  g_object_unref(page->data);
  free(page);

  return true;
}

girara_list_t*
pdf_page_search_text(zathura_page_t* page, const char* text)
{
  if (page == NULL || page->data == NULL || text == NULL) {
    goto error_ret;
  }

  GList* results      = NULL;
  girara_list_t* list = NULL;

  /* search text */
  results = poppler_page_find_text(page->data, text);
  if (results == NULL || g_list_length(results) == 0) {
    goto error_free;
  }

  list = girara_list_new2(g_free);
  if (list == NULL) {
    goto error_free;
  }

  GList* entry = NULL;
  for (entry = results; entry && entry->data; entry = g_list_next(entry)) {
    PopplerRectangle* poppler_rectangle = (PopplerRectangle*) entry->data;
    zathura_rectangle_t* rectangle      = g_malloc0(sizeof(zathura_rectangle_t));

    rectangle->x1 = poppler_rectangle->x1;
    rectangle->x2 = poppler_rectangle->x2;
    rectangle->y1 = poppler_rectangle->y1;
    rectangle->y2 = poppler_rectangle->y2;

    girara_list_append(list, rectangle);
  }

  g_list_free(results);
  return list;

error_free:

  if (results != NULL) {
    g_list_free(results);
  }

  if (list != NULL) {
    girara_list_free(list);
  }

error_ret:

  return NULL;
}

girara_list_t*
pdf_page_links_get(zathura_page_t* page)
{
  return NULL;
}

girara_list_t*
pdf_page_form_fields_get(zathura_page_t* page)
{
  return NULL;
}

#ifdef HAVE_CAIRO
bool
pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo)
{
  if (page == NULL || page->data == NULL || page->document == NULL ||
      cairo == NULL) {
    return false;
  }

  poppler_page_render(page->data, cairo);

  return true;
}
#endif

#if !POPPLER_CHECK_VERSION(0,18,0)
zathura_image_buffer_t*
pdf_page_render(zathura_page_t* page)
{
  if (page == NULL || page->data == NULL || page->document == NULL) {
    return NULL;
  }

  /* calculate sizes */
  unsigned int page_width  = page->document->scale * page->width;
  unsigned int page_height = page->document->scale * page->height;

  /* create pixbuf */
  GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
      page_width, page_height);

  if (pixbuf == NULL) {
    return NULL;
  }

  poppler_page_render_to_pixbuf(page->data, 0, 0, page_width, page_height,
      page->document->scale, 0, pixbuf);

  /* create image buffer */
  zathura_image_buffer_t* image_buffer = zathura_image_buffer_create(page_width, page_height);

  if (image_buffer == NULL) {
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
