/* See LICENSE file for license and copyright information */

#include <stdio.h>

#include "utils.h"
#include "plugin.h"

static void build_index(PopplerDocument* poppler_document, zathura_node_t* root,
    PopplerIndexIter* iter);

zathura_error_t
pdf_document_get_outline(zathura_document_t* document, zathura_node_t** outline)
{
  if (document == NULL || outline == NULL) {
      return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_user_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK
      || poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  PopplerIndexIter* iter = poppler_index_iter_new(poppler_document);
  if (iter == NULL) {
    return ZATHURA_ERROR_DOCUMENT_OUTLINE_DOES_NOT_EXIST;
  }

  zathura_outline_element_t* root;
  if ((error = zathura_outline_element_new(&root, NULL, NULL)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  *outline = zathura_node_new(root);
  if (*outline == NULL) {
    error = ZATHURA_ERROR_OUT_OF_MEMORY;
    goto error_free;
  }

  build_index(poppler_document, *outline, iter);

  return ZATHURA_ERROR_OK;

error_free:

  poppler_index_iter_free(iter);

  return error;
}

static void
build_index(PopplerDocument* poppler_document, zathura_node_t* root, PopplerIndexIter* iter)
{
  if (poppler_document == NULL || root == NULL || iter == NULL) {
    return;
  }

  do {
    PopplerAction* poppler_action = poppler_index_iter_get_action(iter);
    if (poppler_action == NULL) {
      continue;
    }

    /* convert action */
    zathura_action_t* action;
    if (poppler_action_to_zathura_action(poppler_document, poppler_action, &action) != ZATHURA_ERROR_OK) {
      poppler_action_free(poppler_action);
      continue;
    }

    /* escape title */
    gchar* title = g_markup_escape_text(poppler_action->any.title, -1);
    fprintf(stderr, "%s\n", title);

    poppler_action_free(poppler_action);

    zathura_outline_element_t* outline_element;
    if (zathura_outline_element_new(&outline_element, title, action) !=
        ZATHURA_ERROR_OK) {
      g_free(title);
      continue;
    }

    g_free(title);

    zathura_node_t* node = zathura_node_append_data(root, outline_element);
    PopplerIndexIter* child  = poppler_index_iter_get_child(iter);

    if (child != NULL) {
      build_index(poppler_document, node, child);
    }

    poppler_index_iter_free(child);

  } while (poppler_index_iter_next(iter));
}
