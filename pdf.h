/* See LICENSE file for license and copyright information */

#ifndef PDF_H
#define PDF_H

#include <stdbool.h>
#include <poppler.h>

#if HAVE_CAIRO
#include <cairo.h>
#endif

#include <zathura/document.h>

/**
 * Internal document container
 */
typedef struct pdf_document_s
{
  PopplerDocument *document; /**< Poppler document */
} pdf_document_t;

/**
 * Internal page container
 */
typedef struct poppler_page_s
{
  PopplerPage* page; /**< Poppler page */
} poppler_page_t;

/**
 * Open a pdf document
 *
 * @param document Zathura document
 * @return ZATHURA_PLUGIN_ERROR_OK when no error occured, otherwise see
 *    zathura_plugin_error_t
 */
zathura_plugin_error_t pdf_document_open(zathura_document_t* document);

/**
 * Closes and frees the internal document structure
 *
 * @param document Zathura document
 * @return ZATHURA_PLUGIN_ERROR_OK when no error occured, otherwise see
 *    zathura_plugin_error_t
 */
zathura_plugin_error_t pdf_document_free(zathura_document_t* document);

/**
 * Returns a reference to a page
 *
 * @param document Zathura document
 * @param page Page number
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return A page object or NULL if an error occurred
 */
zathura_page_t* pdf_page_get(zathura_document_t* document, unsigned int page,
		zathura_plugin_error_t* error);

/**
 * Frees a pdf page
 *
 * @param page Page
 * @return ZATHURA_PLUGIN_ERROR_OK when no error occured, otherwise see
 *    zathura_plugin_error_t
 */
zathura_plugin_error_t pdf_page_free(zathura_page_t* page);

/**
 * Saves the document to the given path
 *
 * @param document Zathura document
 * @param path File path
 * @return ZATHURA_PLUGIN_ERROR_OK when no error occured, otherwise see
 *    zathura_plugin_error_t
 */
zathura_plugin_error_t pdf_document_save_as(zathura_document_t* document,
		const char* path);

/**
 * Generates the index of the document
 *
 * @param document Zathura document
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return Tree node object or NULL if an error occurred (e.g.: the document has
 *   no index)
 */
girara_tree_node_t* pdf_document_index_generate(zathura_document_t* document,
		zathura_plugin_error_t* error);

/**
 * Returns a list of attachments included in the zathura document
 *
 * @param document Zathura document
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return List of attachments or NULL if an error occurred
 */
girara_list_t* pdf_document_attachments_get(zathura_document_t* document,
		zathura_plugin_error_t* error);

/**
 * Saves an attachment to a file
 *
 * @param document Zathura document
 * @param attachment Name of the attachment
 * @param filename Target file path where the attachment should be saved to
 * @return ZATHURA_PLUGIN_ERROR_OK when no error occured, otherwise see
 *    zathura_plugin_error_t
 */
zathura_plugin_error_t pdf_document_attachment_save(zathura_document_t*
		document, const char* attachment, const char* filename);

/**
 * Returns a list of images included on the zathura page
 *
 * @param page The page
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return List of images
 */
girara_list_t* pdf_page_images_get(zathura_page_t* page,
		zathura_plugin_error_t* error);

#if HAVE_CAIRO
/**
 * Gets the content of the image in a cairo surface
 *
 * @param page Page
 * @param image Image identifier
 * @param error Set to an error value (see \ref zathura_plugin_error_t) if an
 *   error occured
 * @return The cairo image surface or NULL if an error occured
 */
cairo_surface_t* pdf_page_image_get_cairo(zathura_page_t* page,
		zathura_image_t* image, zathura_plugin_error_t* error);
#endif

/**
 * Returns the content of a given meta field
 *
 * @param document Zathura document
 * @param meta Meta identifier
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return Value of the meta data or NULL if an error occurred
 */
char* pdf_document_meta_get(zathura_document_t* document,
		zathura_document_meta_t meta, zathura_plugin_error_t* error);

/**
 * Searches for a specific text on a page and returns a list of results
 *
 * @param page Page
 * @param text Search item
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return List of search results or NULL if an error occurred
 */
girara_list_t* pdf_page_search_text(zathura_page_t* page, const char* text,
		zathura_plugin_error_t* error);

/**
 * Returns a list of internal/external links that are shown on the given page
 *
 * @param page Page
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return List of links or NULL if an error occurred
 */
girara_list_t* pdf_page_links_get(zathura_page_t* page,
		zathura_plugin_error_t* error);

/**
 * Returns a list of form fields available on the given page
 *
 * @param page Page
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return List of form fields or NULL if an error occurred
 */
girara_list_t* pdf_page_form_fields_get(zathura_page_t* page,
		zathura_plugin_error_t* error);

/**
 * Get text for selection
 * @param page Page
 * @param rectangle Selection
 * @error Set to an error value (see \ref zathura_plugin_error_t) if an error
 * occured
 * @return The selected text (needs to be deallocated with g_free)
 */
char* pdf_page_get_text(zathura_page_t* page, zathura_rectangle_t rectangle,
		zathura_plugin_error_t* error);

#if !POPPLER_CHECK_VERSION(0,18,0)
/**
 * Renders a page and returns a allocated image buffer which has to be freed
 * with zathura_image_buffer_free
 *
 * @param page Page
 * @param error Set to an error value (see zathura_plugin_error_t) if an
 *   error occured
 * @return Image buffer or NULL if an error occurred
 */
zathura_image_buffer_t* pdf_page_render(zathura_page_t* page,
		zathura_plugin_error_t* error);
#endif

#if HAVE_CAIRO
/**
 * Renders a page onto a cairo object
 *
 * @param page Page
 * @param cairo Cairo object
 * @param printing Set to true if page should be rendered for printing
 * @return ZATHURA_PLUGIN_ERROR_OK when no error occured, otherwise see
 *    zathura_plugin_error_t
 */
zathura_plugin_error_t pdf_page_render_cairo(zathura_page_t* page, cairo_t* cairo,
		bool printing);
#endif

#endif // PDF_H
