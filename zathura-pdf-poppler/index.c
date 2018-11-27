/* SPDX-License-Identifier: Zlib */

#include "plugin.h"
#include "utils.h"

static void build_index(PopplerDocument* poppler_document, girara_tree_node_t*
    root, PopplerIndexIter* iter);

girara_tree_node_t*
pdf_document_index_generate(zathura_document_t* document, void* data, zathura_error_t* error)
{
  if (document == NULL || data == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  PopplerDocument* poppler_document = data;
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
