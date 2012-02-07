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
  PopplerDocument *document; /**< Poppler document */
} pdf_document_t;

typedef struct poppler_page_s
{
  PopplerPage* page; /**< Poppler page */
} poppler_page_t;

/**
 * Open a pdf document
 *
 * @param document Zathura document
 * @return true if no error occurred, otherwise false
 */
bool pdf_document_open(zathura_document_t* document);

/**
 * Closes and frees the internal document structure
 *
 * @param document Zathura document
 * @return true if no error occurred, otherwise false
 */
bool pdf_document_free(zathura_document_t* document);

/**
 * Returns a reference to a page
 *
 * @param document Zathura document
 * @param page Page number
 * @return A page object or NULL if an error occurred
 */
zathura_page_t* pdf_page_get(zathura_document_t* document, unsigned int page);

/**
 * Frees a pdf page
 *
 * @param page Page
 * @return true if no error occurred, otherwise false
 */
bool pdf_page_free(zathura_page_t* page);

/**
 * Saves the document to the given path
 *
 * @param document Zathura document
 * @param path File path
 * @return true if no error occurred otherwise false
 */
bool pdf_document_save_as(zathura_document_t* document, const char* path);

/**
 * Generates the index of the document
 *
 * @param document Zathura document
 * @return Tree node object or NULL if an error occurred (e.g.: the document has
 * no index)
 */
girara_tree_node_t* pdf_document_index_generate(zathura_document_t* document);

/**
 * Returns a list of attachments included in the zathura document
 *
 * @param document Zathura document
 * @return List of attachments or NULL if an error occurred
 */
girara_list_t* pdf_document_attachments_get(zathura_document_t* document);

/**
 * Saves an attachment to a file
 *
 * @param document Zathura document
 * @param attachment Name of the attachment
 * @param filename Target file path where the attachment should be saved to
 * @return true if no error occurred otherwise false
 */
bool pdf_document_attachment_save(zathura_document_t* document, const char* attachment, const char* filename);

/**
 * Returns a list of images included on the zathura page
 *
 * @param page The page
 * @return List of images
 */
girara_list_t* pdf_page_images_get(zathura_page_t* page);

/**
 * Saves the image to the given file path
 *
 * @param page The page
 * @param image The image
 * @param file Path to the file
 * @return true if no error occured, otherwise false
 */
#if HAVE_CAIRO
bool pdf_page_image_save(zathura_page_t* page, zathura_image_t* image, const char* file);
#endif

/**
 * Returns the content of a given meta field
 *
 * @param document Zathura document
 * @param meta Meta identifier
 * @return Value of the meta data or NULL if an error occurred
 */
char* pdf_document_meta_get(zathura_document_t* document, zathura_document_meta_t meta);

/**
 * Searches for a specific text on a page and returns a list of results
 *
 * @param page Page
 * @param text Search item
 * @return List of search results or NULL if an error occurred
 */
girara_list_t* pdf_page_search_text(zathura_page_t* page, const char* text);

/**
 * Returns a list of internal/external links that are shown on the given page
 *
 * @param page Page
 * @return List of links or NULL if an error occurred
 */
girara_list_t* pdf_page_links_get(zathura_page_t* page);

/**
 * Returns a list of form fields available on the given page
 *
 * @param page Page
 * @return List of form fields or NULL if an error occurred
 */
girara_list_t* pdf_page_form_fields_get(zathura_page_t* page);

#if !POPPLER_CHECK_VERSION(0,18,0)
/**
 * Renders a page and returns a allocated image buffer which has to be freed
 * with zathura_image_buffer_free
 *
 * @param page Page
 * @return Image buffer or NULL if an error occurred
 */
zathura_image_buffer_t* pdf_page_render(zathura_page_t* page);
#endif

#if HAVE_CAIRO
/**
 * Renders a page onto a cairo object
 *
 * @param page Page
 * @param cairo Cairo object
 * @return  true if no error occurred, otherwise false
 */
bool pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo, bool printing);
#endif

#endif // PDF_H
