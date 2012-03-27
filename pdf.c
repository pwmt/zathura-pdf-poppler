/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <girara/datastructures.h>
#include <girara/utils.h>
#include <string.h>
#include <time.h>
#include <poppler/glib/poppler.h>

#include "pdf.h"

#if !defined(HAVE_CAIRO) && POPPLER_CHECK_VERSION(0,18,0)
#error "Cannot render without cairo and poppler >= 0.18"
#endif

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

PLUGIN_REGISTER(
    "pdf-poppler",
    0, 1, 0,
    pdf_document_open,
    PLUGIN_MIMETYPES({
      "application/pdf"
    })
  )

zathura_plugin_error_t
pdf_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  zathura_plugin_error_t error = ZATHURA_PLUGIN_ERROR_OK;

  document->functions.document_free             = pdf_document_free;
  document->functions.document_index_generate   = pdf_document_index_generate;
  document->functions.document_save_as          = pdf_document_save_as;
  document->functions.document_attachments_get  = pdf_document_attachments_get;
  document->functions.document_attachment_save  = pdf_document_attachment_save;
  document->functions.document_meta_get         = pdf_document_meta_get;
  document->functions.page_init                 = pdf_page_init;
  document->functions.page_clear                = pdf_page_clear;
  document->functions.page_search_text          = pdf_page_search_text;
  document->functions.page_links_get            = pdf_page_links_get;
  document->functions.page_form_fields_get      = pdf_page_form_fields_get;
  document->functions.page_images_get           = pdf_page_images_get;
  document->functions.page_get_text             = pdf_page_get_text;
#if !POPPLER_CHECK_VERSION(0,18,0)
  document->functions.page_render               = pdf_page_render;
#endif
#if HAVE_CAIRO
  document->functions.page_render_cairo         = pdf_page_render_cairo;
  document->functions.page_image_get_cairo      = pdf_page_image_get_cairo;
#endif

  document->data = g_malloc0(sizeof(pdf_document_t));

  /* format path */
  GError* gerror  = NULL;
  char* file_uri = g_filename_to_uri(document->file_path, NULL, &gerror);

  if (file_uri == NULL) {
    error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    goto error_free;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;
  pdf_document->document       = poppler_document_new_from_file(file_uri, document->password, &gerror);

  if (pdf_document->document == NULL) {
    if(gerror != NULL && gerror->code == POPPLER_ERROR_ENCRYPTED) {
      error = ZATHURA_PLUGIN_ERROR_INVALID_PASSWORD;
    } else {
      error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  document->number_of_pages = poppler_document_get_n_pages(pdf_document->document);

  g_free(file_uri);

  return ZATHURA_PLUGIN_ERROR_OK;

error_free:

    if (gerror != NULL) {
      g_error_free(gerror);
    }

    if (file_uri != NULL) {
      g_free(file_uri);
    }

    if (document != NULL) {
      free(document->data);
      document->data = NULL;
    }

  return error;
}

zathura_plugin_error_t
pdf_document_free(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  if (document->data != NULL) {
    pdf_document_t* pdf_document = (pdf_document_t*) document->data;
    g_object_unref(pdf_document->document);
    free(document->data);
    document->data = NULL;
  }

  return ZATHURA_PLUGIN_ERROR_OK;
}

static void
build_index(pdf_document_t* pdf, girara_tree_node_t* root, PopplerIndexIter* iter)
{
  if (root == NULL || iter == NULL) {
    return;
  }

  do {
    PopplerAction* action = poppler_index_iter_get_action(iter);

    if (action == NULL) {
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
        PopplerDest* dest = poppler_document_find_dest(pdf->document,
            action->goto_dest.dest->named_dest);

        if (dest != NULL) {
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

    if (child != NULL) {
      build_index(pdf, node, child);
    }

    poppler_index_iter_free(child);

  } while (poppler_index_iter_next(iter));
}

girara_tree_node_t*
pdf_document_index_generate(zathura_document_t* document, zathura_plugin_error_t* error)
{
  if (document == NULL || document->data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;
  PopplerIndexIter* iter       = poppler_index_iter_new(pdf_document->document);

  if (iter == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  girara_tree_node_t* root = girara_node_new(zathura_index_element_new("ROOT"));
  // girara_node_set_free_function(root, (girara_free_function_t) zathura_index_element_free);
  build_index(pdf_document, root, iter);

  poppler_index_iter_free(iter);
  return root;
}

zathura_plugin_error_t
pdf_document_save_as(zathura_document_t* document, const char* path)
{
  if (document == NULL || document->data == NULL || path == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;

  char* file_path = g_strdup_printf("file://%s", path);
  gboolean ret = poppler_document_save(pdf_document->document, file_path, NULL);
  g_free(file_path);

  return (ret == true ? ZATHURA_PLUGIN_ERROR_OK : ZATHURA_PLUGIN_ERROR_UNKNOWN);
}

girara_list_t*
pdf_document_attachments_get(zathura_document_t* document, zathura_plugin_error_t* error)
{
  if (document == NULL || document->data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;
  if (poppler_document_has_attachments(pdf_document->document) == FALSE) {
    girara_warning("PDF file has no attachments");
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    return NULL;
  }

  girara_list_t* res = girara_sorted_list_new2((girara_compare_function_t) g_strcmp0,
      (girara_free_function_t) g_free);
  if (res == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  GList* attachment_list = poppler_document_get_attachments(pdf_document->document);
  GList* attachments;

  for (attachments = attachment_list; attachments; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    girara_list_append(res, g_strdup(attachment->name));
  }

  return res;
}

zathura_plugin_error_t
pdf_document_attachment_save(zathura_document_t* document, const char* attachmentname, const char* file)
{
  if (document == NULL || document->data == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  pdf_document_t* pdf_document = (pdf_document_t*) document->data;
  if (poppler_document_has_attachments(pdf_document->document) == FALSE) {
    girara_warning("PDF file has no attachments");
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }


  GList* attachment_list = poppler_document_get_attachments(pdf_document->document);
  GList* attachments;

  for (attachments = attachment_list; attachments; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    if (g_strcmp0(attachment->name, attachmentname) != 0) {
      continue;
    }

    return poppler_attachment_save(attachment, file, NULL);
  }

  return ZATHURA_PLUGIN_ERROR_OK;
}

girara_list_t*
pdf_page_images_get(zathura_page_t* page, zathura_plugin_error_t* error)
{
  if (page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);

  if (poppler_page == NULL) {
    goto error_ret;
  }

  girara_list_t* list  = NULL;
  GList* image_mapping = NULL;

  image_mapping = poppler_page_get_image_mapping(poppler_page->page);
  if (image_mapping == NULL || g_list_length(image_mapping) == 0) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  list = girara_list_new();
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
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

#if HAVE_CAIRO
cairo_surface_t*
pdf_page_image_get_cairo(zathura_page_t* page, zathura_image_t* image, zathura_plugin_error_t* error)
{
  if (page == NULL || image == NULL || image->data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  gint* image_id               = (gint*) image->data;

  if (poppler_page == NULL) {
    goto error_ret;
  }

  cairo_surface_t* surface = poppler_page_get_image(poppler_page->page, *image_id);
  if (surface == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    goto error_ret;
  }

  return surface;

error_ret:

  return NULL;
}
#endif

char*
pdf_document_meta_get(zathura_document_t* document, zathura_document_meta_t meta, zathura_plugin_error_t* error)
{
  if (document == NULL || document->data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
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
        if (error != NULL) {
          *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
        }
        return NULL;
      }
      break;
    case ZATHURA_DOCUMENT_MODIFICATION_DATE:
      g_object_get(pdf_document->document, "mod-date", &time_value, NULL);
      tmp = ctime(&time_value);
      if (tmp != NULL) {
        string_value = g_strndup(tmp, strlen(tmp) - 1);
      } else {
        if (error != NULL) {
          *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
        }
        return NULL;
      }
      break;
    default:
      if (error != NULL) {
        *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
      }
      return NULL;
  }

  if (string_value == NULL || strlen(string_value) == 0) {
    g_free(string_value);
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    return NULL;
  }

  return string_value;
}

zathura_plugin_error_t
pdf_page_init(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  zathura_document_t* document = zathura_page_get_document(page);
  pdf_document_t* pdf_document = (pdf_document_t*) document->data;

  /* init poppler data */
  poppler_page_t* poppler_page = g_malloc0(sizeof(poppler_page_t));
  poppler_page->page           = poppler_document_get_page(pdf_document->document, zathura_page_get_index(page));

  if (poppler_page->page == NULL) {
    g_free(poppler_page);
    return ZATHURA_PLUGIN_ERROR_UNKNOWN;
  }

  zathura_page_set_data(page, poppler_page);

  /* calculate dimensions */
  double width;
  double height;
  poppler_page_get_size(poppler_page->page, &width, &height);
  zathura_page_set_width(page, width);
  zathura_page_set_height(page, height);

  return ZATHURA_PLUGIN_ERROR_OK;
}

zathura_plugin_error_t
pdf_page_clear(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  if (poppler_page != NULL) {
    g_object_unref(poppler_page->page);
    g_free(poppler_page);
  }

  return ZATHURA_PLUGIN_ERROR_OK;
}

girara_list_t*
pdf_page_search_text(zathura_page_t* page, const char* text, zathura_plugin_error_t* error)
{
  if (page == NULL || text == NULL || strlen(text) == 0) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  GList* results      = NULL;
  girara_list_t* list = NULL;

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  if (poppler_page == NULL) {
    goto error_ret;
  }

  /* search text */
  results = poppler_page_find_text(poppler_page->page, text);
  if (results == NULL || g_list_length(results) == 0) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  list = girara_list_new2(g_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  GList* entry = NULL;
  for (entry = results; entry && entry->data; entry = g_list_next(entry)) {
    PopplerRectangle* poppler_rectangle = (PopplerRectangle*) entry->data;
    zathura_rectangle_t* rectangle      = g_malloc0(sizeof(zathura_rectangle_t));

    rectangle->x1 = poppler_rectangle->x1;
    rectangle->x2 = poppler_rectangle->x2;
    rectangle->y1 = zathura_page_get_height(page) - poppler_rectangle->y2;
    rectangle->y2 = zathura_page_get_height(page) - poppler_rectangle->y1;

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
pdf_page_links_get(zathura_page_t* page, zathura_plugin_error_t* error)
{
  if (page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  if (poppler_page == NULL) {
    goto error_ret;
  }

  girara_list_t* list = NULL;
  GList* link_mapping = NULL;

  link_mapping = poppler_page_get_link_mapping(poppler_page->page);
  if (link_mapping == NULL || g_list_length(link_mapping) == 0) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_UNKNOWN;
    }
    goto error_free;
  }
  link_mapping = g_list_reverse(link_mapping);

  list = girara_list_new2((girara_free_function_t) zathura_link_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  for (GList* link = link_mapping; link != NULL; link = g_list_next(link)) {
    PopplerLinkMapping* poppler_link     = (PopplerLinkMapping*) link->data;
    zathura_document_t* zathura_document = (zathura_document_t*) zathura_page_get_document(page);
    pdf_document_t* pdf_document         = (pdf_document_t*) zathura_document->data;

    /* extract position */
    zathura_rectangle_t position = { 0, 0, 0, 0 };
    position.x1 = poppler_link->area.x1;
    position.x2 = poppler_link->area.x2;
    position.y1 = zathura_page_get_height(page) - poppler_link->area.y2;
    position.y2 = zathura_page_get_height(page) - poppler_link->area.y1;

    /* extract type and target */
    zathura_link_type_t type         = ZATHURA_LINK_INVALID;
    zathura_link_target_t target     = { 0 };
    PopplerDest* poppler_destination = NULL;

    switch (poppler_link->action->type) {
      case POPPLER_ACTION_URI:
        type       = ZATHURA_LINK_EXTERNAL;
        target.uri = poppler_link->action->uri.uri;
        break;
      case POPPLER_ACTION_GOTO_DEST:
        type = ZATHURA_LINK_TO_PAGE;
        if (poppler_link->action->goto_dest.dest->type == POPPLER_DEST_NAMED) {
          poppler_destination =
            poppler_document_find_dest(pdf_document->document,
                poppler_link->action->goto_dest.dest->named_dest);

          if (poppler_destination != NULL) {
            target.page_number = poppler_destination->page_num - 1;
            poppler_dest_free(poppler_destination);
          }
        } else {
          target.page_number = poppler_link->action->goto_dest.dest->page_num - 1;
        }
        break;
      default:
        continue;
    }

    zathura_link_t* zathura_link = zathura_link_new(type, position, target);
    girara_list_append(list, zathura_link);
  }

  poppler_page_free_link_mapping(link_mapping);

  return list;

error_free:

  if (list != NULL) {
    girara_list_free(list);
  }

  if (link_mapping != NULL) {
    poppler_page_free_link_mapping(link_mapping);
  }

error_ret:

  return NULL;
}

girara_list_t*
pdf_page_form_fields_get(zathura_page_t* page, zathura_plugin_error_t* error)
{
  if (error != NULL) {
    *error = ZATHURA_PLUGIN_ERROR_NOT_IMPLEMENTED;
  }
  return NULL;
}

char* pdf_page_get_text(zathura_page_t* page, zathura_rectangle_t rectangle,
    zathura_plugin_error_t* error)
{
  if (page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  if (poppler_page == NULL) {
    return NULL;
  }

  PopplerRectangle rect;
  rect.x1 = rectangle.x1;
  rect.x2 = rectangle.x2;
#if !POPPLER_CHECK_VERSION(0,15,0)
  /* adapt y coordinates */
  rect.y1 = page->height - rectangle.y1;
  rect.y2 = page->height - rectangle.y2;
#else
  rect.y1 = rectangle.y1;
  rect.y2 = rectangle.y2;
#endif

  /* get selected text */
  return poppler_page_get_selected_text(poppler_page->page, POPPLER_SELECTION_GLYPH, &rect);
}

#ifdef HAVE_CAIRO
zathura_plugin_error_t
pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo, bool printing)
{
  if (page == NULL || cairo == NULL) {
    return ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
  }

  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  if (poppler_page == NULL) {
    return ZATHURA_PLUGIN_ERROR_UNKNOWN;
  }

  if (printing == false) {
    poppler_page_render(poppler_page->page, cairo);
  } else {
    poppler_page_render_for_printing(poppler_page->page, cairo);
  }

  return ZATHURA_PLUGIN_ERROR_OK;
}
#endif

#if !POPPLER_CHECK_VERSION(0,18,0)
zathura_image_buffer_t*
pdf_page_render(zathura_page_t* page, zathura_plugin_error_t* error)
{
  if (page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  zathura_document_t* document = zathura_page_get_document(page);
  poppler_page_t* poppler_page = (poppler_page_t*) zathura_page_get_data(page);
  if (poppler_page == NULL || document == NULL) {
    return NULL;
  }

  /* calculate sizes */
  unsigned int page_width  = document->scale * zathura_page_get_width(page);
  unsigned int page_height = document->scale * zathura_page_get_height(page);

  /* create pixbuf */
  GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
      page_width, page_height);

  if (pixbuf == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  poppler_page_render_to_pixbuf(poppler_page->page, 0, 0, page_width, page_height,
      document->scale, 0, pixbuf);

  /* create image buffer */
  zathura_image_buffer_t* image_buffer = zathura_image_buffer_create(page_width, page_height);

  if (image_buffer == NULL) {
    if (error != NULL) {
      *error = ZATHURA_PLUGIN_ERROR_OUT_OF_MEMORY;
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
