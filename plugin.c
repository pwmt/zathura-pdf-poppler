/* See LICENSE file for license and copyright information */

#include "plugin.h"

static void
register_functions(zathura_plugin_functions_t* functions)
{
  functions->document_open            = pdf_document_open;
  functions->document_free            = pdf_document_free;
  functions->document_index_generate  = pdf_document_index_generate;
  functions->document_save_as         = pdf_document_save_as;
  functions->document_attachments_get = pdf_document_attachments_get;
  functions->document_attachment_save = pdf_document_attachment_save;
  functions->document_get_information = pdf_document_get_information;
  functions->page_init                = pdf_page_init;
  functions->page_clear               = pdf_page_clear;
  functions->page_search_text         = pdf_page_search_text;
  functions->page_links_get           = pdf_page_links_get;
  functions->page_form_fields_get     = pdf_page_form_fields_get;
  functions->page_images_get          = pdf_page_images_get;
  functions->page_get_text            = pdf_page_get_text;
  functions->page_render_cairo        = pdf_page_render_cairo;
  functions->page_image_get_cairo     = pdf_page_image_get_cairo;
}

ZATHURA_PLUGIN_REGISTER(
  "pdf-poppler",
  VERSION_MAJOR, VERSION_MINOR, VERSION_REV,
  register_functions,
  ZATHURA_PLUGIN_MIMETYPES({
    "application/pdf"
  })
)
