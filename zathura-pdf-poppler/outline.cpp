/* See LICENSE file for license and copyright information */

#include <stdio.h>

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

  return error;
}
