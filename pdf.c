/* See LICENSE file for license and copyright information */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <girara/datastructures.h>
#include <girara/utils.h>
#include <poppler/glib/poppler.h>

#include "pdf.h"

#if !POPPLER_CHECK_VERSION(0,15,0)
#define poppler_page_get_selected_text poppler_page_get_text
#endif

#if !defined(HAVE_CAIRO) && POPPLER_CHECK_VERSION(0,18,0)
#error "Cannot render without cairo and poppler >= 0.18"
#endif

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static zathura_link_t*
poppler_link_to_zathura_link(PopplerDocument* poppler_document, PopplerAction*
    poppler_action, zathura_rectangle_t position)
{
  zathura_link_type_t type     = ZATHURA_LINK_INVALID;
  zathura_link_target_t target = { ZATHURA_LINK_DESTINATION_UNKNOWN, NULL, 0, -1, -1, -1, -1, 0 };

  /* extract link */
  switch (poppler_action->type) {
    case POPPLER_ACTION_NONE:
      type = ZATHURA_LINK_NONE;
      break;
    case POPPLER_ACTION_GOTO_DEST: {
      PopplerDest* poppler_destination = poppler_action->goto_dest.dest;
      if (poppler_destination == NULL) {
        return NULL;
      }

      type = ZATHURA_LINK_GOTO_DEST;

      if (poppler_action->goto_dest.dest->type == POPPLER_DEST_NAMED) {
        poppler_destination = poppler_document_find_dest(poppler_document, poppler_destination->named_dest);
        if (poppler_destination == NULL) {
          return NULL;
        }
      }

      PopplerPage* poppler_page = poppler_document_get_page(poppler_document, poppler_destination->page_num - 1);
      double height = 0;
      poppler_page_get_size(poppler_page, NULL, &height);

      switch (poppler_destination->type) {
        case POPPLER_DEST_XYZ:
          target.destination_type = ZATHURA_LINK_DESTINATION_XYZ;
          target.page_number      = poppler_destination->page_num - 1;
          if (poppler_destination->change_zoom != 0) {
            target.scale          = poppler_destination->zoom;
          }
          if (poppler_destination->change_left != 0) {
            target.left           = poppler_destination->left;
          }
          if (poppler_destination->change_top != 0) {
            target.top            = height - MIN(height, poppler_destination->top);
          }
          break;
        case POPPLER_DEST_FIT:
          target.destination_type = ZATHURA_LINK_DESTINATION_FIT;
          target.page_number      = poppler_destination->page_num - 1;
          break;
        case POPPLER_DEST_FITH:
          target.destination_type = ZATHURA_LINK_DESTINATION_FITH;
          target.page_number      = poppler_destination->page_num - 1;
          if (poppler_destination->change_top != 0) {
            target.top            = height - MIN(height, poppler_destination->top);
          }
          break;
        case POPPLER_DEST_FITV:
          target.destination_type = ZATHURA_LINK_DESTINATION_FITV;
          target.page_number      = poppler_destination->page_num - 1;
          if (poppler_destination->change_left != 0) {
            target.left           = poppler_destination->left;
          }
          break;
        case POPPLER_DEST_FITR:
          target.destination_type = ZATHURA_LINK_DESTINATION_FITR;
          target.page_number      = poppler_destination->page_num - 1;
          if (poppler_destination->change_left != 0) {
            target.left           = poppler_destination->left;
          }
          if (poppler_destination->change_top != 0) {
            target.top            = height - MIN(height, poppler_destination->top);
          }
          target.right            = poppler_destination->right;
          target.bottom           = height - MIN(height, poppler_destination->bottom);
          break;
        case POPPLER_DEST_FITB:
          target.destination_type = ZATHURA_LINK_DESTINATION_FITB;
          target.page_number      = poppler_destination->page_num - 1;
          break;
        case POPPLER_DEST_FITBH:
          target.destination_type = ZATHURA_LINK_DESTINATION_FITBH;
          target.page_number      = poppler_destination->page_num - 1;
          if (poppler_destination->change_top != 0) {
            target.top            = height - MIN(height, poppler_destination->top);
          }
          break;
        case POPPLER_DEST_FITBV:
          target.destination_type = ZATHURA_LINK_DESTINATION_FITBV;
          target.page_number      = poppler_destination->page_num - 1;
          target.left             = poppler_destination->top;
          break;
        case POPPLER_DEST_UNKNOWN:
          target.destination_type = ZATHURA_LINK_DESTINATION_UNKNOWN;
          target.page_number      = poppler_destination->page_num - 1;
          break;
        default:
          return NULL;
      }
      break;
    }
    case POPPLER_ACTION_GOTO_REMOTE:
      type = ZATHURA_LINK_GOTO_REMOTE;
      if ((target.value = poppler_action->goto_remote.file_name) == NULL) {
        return NULL;
      }
      break;
    case POPPLER_ACTION_URI:
      type         = ZATHURA_LINK_URI;
      target.value = poppler_action->uri.uri;
      break;
    case POPPLER_ACTION_LAUNCH:
      type         = ZATHURA_LINK_LAUNCH;
      target.value = poppler_action->launch.file_name;
      break;
    case POPPLER_ACTION_NAMED:
      type         = ZATHURA_LINK_NAMED;
      target.value = poppler_action->named.named_dest;
      break;
    default:
      return NULL;
  }

  return zathura_link_new(type, position, target);
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

void
register_functions(zathura_plugin_functions_t* functions)
{
  functions->document_open            = (zathura_plugin_document_open_t) pdf_document_open;
  functions->document_free            = (zathura_plugin_document_free_t) pdf_document_free;
  functions->document_index_generate  = (zathura_plugin_document_index_generate_t) pdf_document_index_generate;
  functions->document_save_as         = (zathura_plugin_document_save_as_t) pdf_document_save_as;
  functions->document_attachments_get = (zathura_plugin_document_attachments_get_t) pdf_document_attachments_get;
  functions->document_attachment_save = (zathura_plugin_document_attachment_save_t) pdf_document_attachment_save;
  functions->document_get_information = (zathura_plugin_document_get_information_t) pdf_document_get_information;
  functions->page_init                = (zathura_plugin_page_init_t) pdf_page_init;
  functions->page_clear               = (zathura_plugin_page_clear_t) pdf_page_clear;
  functions->page_search_text         = (zathura_plugin_page_search_text_t) pdf_page_search_text;
  functions->page_links_get           = (zathura_plugin_page_links_get_t) pdf_page_links_get;
  functions->page_form_fields_get     = (zathura_plugin_page_form_fields_get_t) pdf_page_form_fields_get;
  functions->page_images_get          = (zathura_plugin_page_images_get_t) pdf_page_images_get;
  functions->page_get_text            = (zathura_plugin_page_get_text_t) pdf_page_get_text;
#if !POPPLER_CHECK_VERSION(0,18,0)
  functions->page_render              = (zathura_plugin_page_render_t) pdf_page_render;
#endif
#if HAVE_CAIRO
  functions->page_render_cairo        = (zathura_plugin_page_render_cairo_t) pdf_page_render_cairo;
  functions->page_image_get_cairo     = (zathura_plugin_page_image_get_cairo_t) pdf_page_image_get_cairo;
#endif
}

ZATHURA_PLUGIN_REGISTER(
  "pdf-poppler",
  VERSION_MAJOR, VERSION_MINOR, VERSION_REV,
  register_functions,
  ZATHURA_PLUGIN_MIMETYPES({
    "application/pdf"
  })
)

zathura_error_t
pdf_document_open(zathura_document_t* document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  /* format path */
  GError* gerror  = NULL;
  char* file_uri = g_filename_to_uri(zathura_document_get_path(document), NULL, &gerror);

  if (file_uri == NULL) {
    error = ZATHURA_ERROR_UNKNOWN;
    goto error_free;
  }

  PopplerDocument* poppler_document = poppler_document_new_from_file(file_uri,
      zathura_document_get_password(document), &gerror);

  if (poppler_document == NULL) {
    if(gerror != NULL && gerror->code == POPPLER_ERROR_ENCRYPTED) {
      error = ZATHURA_ERROR_INVALID_PASSWORD;
    } else {
      error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  zathura_document_set_data(document, poppler_document);

  zathura_document_set_number_of_pages(document,
      poppler_document_get_n_pages(poppler_document));

  g_free(file_uri);

  return ZATHURA_ERROR_OK;

error_free:

    if (gerror != NULL) {
      g_error_free(gerror);
    }

    if (file_uri != NULL) {
      g_free(file_uri);
    }

  return error;
}

zathura_error_t
pdf_document_free(zathura_document_t* document, PopplerDocument* poppler_document)
{
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (poppler_document != NULL) {
    g_object_unref(poppler_document);
    zathura_document_set_data(document, NULL);
  }

  return ZATHURA_ERROR_OK;
}

static void
build_index(PopplerDocument* poppler_document, girara_tree_node_t* root, PopplerIndexIter* iter)
{
  if (poppler_document == NULL || root == NULL || iter == NULL) {
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
      poppler_action_free(action);
      continue;
    }

    zathura_rectangle_t rect = { 0, 0, 0, 0 };
    index_element->link = poppler_link_to_zathura_link(poppler_document, action, rect);
    if (index_element->link == NULL) {
      poppler_action_free(action);
      continue;
    }

    poppler_action_free(action);

    girara_tree_node_t* node = girara_node_append_data(root, index_element);
    PopplerIndexIter* child  = poppler_index_iter_get_child(iter);

    if (child != NULL) {
      build_index(poppler_document, node, child);
    }

    poppler_index_iter_free(child);

  } while (poppler_index_iter_next(iter));
}

girara_tree_node_t*
pdf_document_index_generate(zathura_document_t* document, PopplerDocument* poppler_document, zathura_error_t* error)
{
  if (document == NULL || poppler_document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  PopplerIndexIter* iter = poppler_index_iter_new(poppler_document);

  if (iter == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  girara_tree_node_t* root = girara_node_new(zathura_index_element_new("ROOT"));
  // girara_node_set_free_function(root, (girara_free_function_t) zathura_index_element_free);
  build_index(poppler_document, root, iter);

  poppler_index_iter_free(iter);
  return root;
}

zathura_error_t
pdf_document_save_as(zathura_document_t* document, PopplerDocument* poppler_document, const char* path)
{
  if (document == NULL || poppler_document == NULL || path == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  char* file_path = g_strdup_printf("file://%s", path);
  gboolean ret = poppler_document_save(poppler_document, file_path, NULL);
  g_free(file_path);

  return (ret == true ? ZATHURA_ERROR_OK : ZATHURA_ERROR_UNKNOWN);
}

girara_list_t*
pdf_document_attachments_get(zathura_document_t* document, PopplerDocument* poppler_document, zathura_error_t* error)
{
  if (document == NULL || poppler_document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    girara_warning("PDF file has no attachments");
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    return NULL;
  }

  girara_list_t* res = girara_sorted_list_new2((girara_compare_function_t) g_strcmp0,
      (girara_free_function_t) g_free);
  if (res == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    return NULL;
  }

  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  GList* attachments;

  for (attachments = attachment_list; attachments; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    girara_list_append(res, g_strdup(attachment->name));
  }

  return res;
}

zathura_error_t
pdf_document_attachment_save(zathura_document_t* document,
    PopplerDocument* poppler_document, const char* attachmentname, const char* file)
{
  if (document == NULL || poppler_document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (poppler_document_has_attachments(poppler_document) == FALSE) {
    girara_warning("PDF file has no attachments");
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }


  GList* attachment_list = poppler_document_get_attachments(poppler_document);
  GList* attachments;

  for (attachments = attachment_list; attachments; attachments = g_list_next(attachments)) {
    PopplerAttachment* attachment = (PopplerAttachment*) attachments->data;
    if (g_strcmp0(attachment->name, attachmentname) != 0) {
      continue;
    }

    return poppler_attachment_save(attachment, file, NULL);
  }

  return ZATHURA_ERROR_OK;
}

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

#if HAVE_CAIRO
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
#endif

girara_list_t*
pdf_document_get_information(zathura_document_t* document, PopplerDocument*
    poppler_document, zathura_error_t* error)
{
  if (document == NULL || poppler_document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  girara_list_t* list = zathura_document_information_entry_list_new();
  if (list == NULL) {
    return NULL;
  }

  /* get string values */
  typedef struct info_value_s {
    char* property;
    zathura_document_information_type_t type;
  } info_value_t;

  static const info_value_t string_values[] = {
    { "title",    ZATHURA_DOCUMENT_INFORMATION_TITLE },
    { "author",   ZATHURA_DOCUMENT_INFORMATION_AUTHOR },
    { "subject",  ZATHURA_DOCUMENT_INFORMATION_SUBJECT },
    { "keywords", ZATHURA_DOCUMENT_INFORMATION_KEYWORDS },
    { "creator",  ZATHURA_DOCUMENT_INFORMATION_CREATOR },
    { "producer", ZATHURA_DOCUMENT_INFORMATION_PRODUCER }
  };

  char* string_value;
  for (unsigned int i = 0; i < LENGTH(string_values); i++) {
    g_object_get(poppler_document, string_values[i].property, &string_value, NULL);
    zathura_document_information_entry_t* entry = zathura_document_information_entry_new(
        string_values[i].type, string_value);
    if (entry != NULL) {
      girara_list_append(list, entry);
    }
  }

  /* get time values */
  static const info_value_t time_values[] = {
    { "creation-date", ZATHURA_DOCUMENT_INFORMATION_CREATION_DATE },
    { "mod-date",      ZATHURA_DOCUMENT_INFORMATION_MODIFICATION_DATE }
  };

  char* tmp;
  time_t time_value;
  for (unsigned int i = 0; i < LENGTH(time_values); i++) {
    g_object_get(poppler_document, string_values[i].property, &time_value, NULL);
    tmp = ctime(&time_value);
    if (tmp != NULL) {
      string_value = g_strndup(tmp, strlen(tmp) - 1);
      zathura_document_information_entry_t* entry = zathura_document_information_entry_new(
          time_values[i].type, string_value);
      if (entry != NULL) {
        girara_list_append(list, entry);
      }
      g_free(string_value);
    }
  }

  return list;
}

zathura_error_t
pdf_page_init(zathura_page_t* page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_document_t* document           = zathura_page_get_document(page);
  PopplerDocument* poppler_document = zathura_document_get_data(document);

  if (poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  /* init poppler data */
  PopplerPage* poppler_page = poppler_document_get_page(poppler_document, zathura_page_get_index(page));

  if (poppler_page == NULL) {
    g_free(poppler_page);
    return ZATHURA_ERROR_UNKNOWN;
  }

  zathura_page_set_data(page, poppler_page);

  /* calculate dimensions */
  double width;
  double height;
  poppler_page_get_size(poppler_page, &width, &height);
  zathura_page_set_width(page, width);
  zathura_page_set_height(page, height);

  return ZATHURA_ERROR_OK;
}

zathura_error_t
pdf_page_clear(zathura_page_t* page, PopplerPage* poppler_page)
{
  if (page == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  if (poppler_page != NULL) {
    g_object_unref(poppler_page);
  }

  return ZATHURA_ERROR_OK;
}

girara_list_t*
pdf_page_search_text(zathura_page_t* page, PopplerPage* poppler_page, const
    char* text, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL || text == NULL || strlen(text) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  GList* results      = NULL;
  girara_list_t* list = NULL;

  /* search text */
  results = poppler_page_find_text(poppler_page, text);
  if (results == NULL || g_list_length(results) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }

  list = girara_list_new2(g_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
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
pdf_page_links_get(zathura_page_t* page, PopplerPage* poppler_page, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  girara_list_t* list = NULL;
  GList* link_mapping = NULL;

  link_mapping = poppler_page_get_link_mapping(poppler_page);
  if (link_mapping == NULL || g_list_length(link_mapping) == 0) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_UNKNOWN;
    }
    goto error_free;
  }
  link_mapping = g_list_reverse(link_mapping);

  list = girara_list_new2((girara_free_function_t) zathura_link_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  zathura_document_t* zathura_document = (zathura_document_t*) zathura_page_get_document(page);
  PopplerDocument* poppler_document    = zathura_document_get_data(zathura_document);

  for (GList* link = link_mapping; link != NULL; link = g_list_next(link)) {
    PopplerLinkMapping* poppler_link       = (PopplerLinkMapping*) link->data;

    /* extract position */
    zathura_rectangle_t position = { 0, 0, 0, 0 };
    position.x1 = poppler_link->area.x1;
    position.x2 = poppler_link->area.x2;
    position.y1 = zathura_page_get_height(page) - poppler_link->area.y2;
    position.y2 = zathura_page_get_height(page) - poppler_link->area.y1;

    zathura_link_t* zathura_link =
      poppler_link_to_zathura_link(poppler_document, poppler_link->action,
          position);
    if (zathura_link != NULL) {
      girara_list_append(list, zathura_link);
    }
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
pdf_page_form_fields_get(zathura_page_t* page, PopplerPage* poppler_page,
    zathura_error_t* error)
{
  if (error != NULL) {
    *error = ZATHURA_ERROR_NOT_IMPLEMENTED;
  }
  return NULL;
}

char*
pdf_page_get_text(zathura_page_t* page, PopplerPage* poppler_page,
    zathura_rectangle_t rectangle, zathura_error_t* error)
{
  if (page == NULL || poppler_page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  PopplerRectangle rect;
  rect.x1 = rectangle.x1;
  rect.x2 = rectangle.x2;
#if !POPPLER_CHECK_VERSION(0,15,0)
  /* adapt y coordinates */
  rect.y1 = zathura_page_get_height(page) - rectangle.y1;
  rect.y2 = zathura_page_get_height(page) - rectangle.y2;
#else
  rect.y1 = rectangle.y1;
  rect.y2 = rectangle.y2;
#endif

  /* get selected text */
  return poppler_page_get_selected_text(poppler_page, POPPLER_SELECTION_GLYPH, &rect);
}

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
