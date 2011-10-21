/* See LICENSE file for license and copyright information */

#ifndef PDF_H
#define PDF_H

#include <stdbool.h>
#include <poppler.h>

#if HAVE_CAIRO
#include <cairo.h>
#endif

#include <zathura/document.h>

typedef struct pdf_document_s
{
  PopplerDocument *document;
} pdf_document_t;

bool pdf_document_open(zathura_document_t* document);
bool pdf_document_free(zathura_document_t* document);
girara_tree_node_t* pdf_document_index_generate(zathura_document_t* document);
bool pdf_document_save_as(zathura_document_t* document, const char* path);
girara_list_t* pdf_document_attachments_get(zathura_document_t* document);
char* pdf_document_meta_get(zathura_document_t* document, zathura_document_meta_t meta);
zathura_page_t* pdf_page_get(zathura_document_t* document, unsigned int page);
girara_list_t* pdf_page_search_text(zathura_page_t* page, const char* text);
girara_list_t* pdf_page_links_get(zathura_page_t* page);
girara_list_t* pdf_page_form_fields_get(zathura_page_t* page);
#if !POPPLER_CHECK_VERSION(0,18,0)
zathura_image_buffer_t* pdf_page_render(zathura_page_t* page);
#endif
#if HAVE_CAIRO
bool pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo);
#endif
bool pdf_page_free(zathura_page_t* page);

#endif // PDF_H
