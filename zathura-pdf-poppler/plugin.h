/* SPDX-License-Identifier: Zlib */

#ifndef PDF_H
#define PDF_H

#include <stdbool.h>
#include <poppler.h>

#include <cairo.h>

#include <girara/macros.h>

#include <zathura/page.h>
#include <zathura/document.h>
#include <zathura/plugin-api.h>

extern gboolean g_signature_overlay_toggle;

/**
 * Open a pdf document
 *
 * @param document Zathura document
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_document_open(zathura_document_t* document);

/**
 * Closes and frees the internal document structure
 *
 * @param document Zathura document
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_document_free(zathura_document_t* document, void* poppler_document);

/**
 * Initializes the page with the needed values
 *
 * @param page The page object
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_page_init(zathura_page_t* page);

/**
 * Frees the data that is used by the plugin in the page
 *
 * @param page Page
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_page_clear(zathura_page_t* page, void* poppler_page);

/**
 * Saves the document to the given path
 *
 * @param document Zathura document
 * @param path File path
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_document_save_as(zathura_document_t* document,
    void* poppler_document, const char* path);

/**
 * Generates the index of the document
 *
 * @param document Zathura document
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return Tree node object or NULL if an error occurred (e.g.: the document has
 *   no index)
 */
GIRARA_HIDDEN girara_tree_node_t* pdf_document_index_generate(zathura_document_t* document,
    void* poppler_document, zathura_error_t* error);

/**
 * Returns a list of attachments included in the zathura document
 *
 * @param document Zathura document
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return List of attachments or NULL if an error occurred
 */
GIRARA_HIDDEN girara_list_t* pdf_document_attachments_get(zathura_document_t* document,
    void* poppler_document, zathura_error_t* error);

/**
 * Saves an attachment to a file
 *
 * @param document Zathura document
 * @param attachment Name of the attachment
 * @param filename Target file path where the attachment should be saved to
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_document_attachment_save(zathura_document_t*
    document, void* poppler_document, const char* attachment, const char* filename);

/**
 * Returns a list of images included on the zathura page
 *
 * @param page The page
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return List of images
 */
GIRARA_HIDDEN girara_list_t* pdf_page_images_get(zathura_page_t* page,
    void* poppler_page, zathura_error_t* error);

/**
 * Gets the content of the image in a cairo surface
 *
 * @param page Page
 * @param image Image identifier
 * @param error Set to an error value (see \ref zathura_error_t) if an
 *   error occurred
 * @return The cairo image surface or NULL if an error occurred
 */
GIRARA_HIDDEN cairo_surface_t* pdf_page_image_get_cairo(zathura_page_t* page,
    void* poppler_page, zathura_image_t* image, zathura_error_t* error);

/**
 * Returns a list of document information entries of the document
 *
 * @param document Zathura document
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return List of information entries or NULL if an error occurred
 */
GIRARA_HIDDEN girara_list_t* pdf_document_get_information(zathura_document_t* document,
    void* poppler_document, zathura_error_t* error);

/**
 * Searches for a specific text on a page and returns a list of results
 *
 * @param page Page
 * @param text Search item
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return List of search results or NULL if an error occurred
 */
GIRARA_HIDDEN girara_list_t* pdf_page_search_text(zathura_page_t* page, void*
    data, const char* text, zathura_error_t* error);

/**
 * Returns a list of internal/external links that are shown on the given page
 *
 * @param page Page
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return List of links or NULL if an error occurred
 */
GIRARA_HIDDEN girara_list_t* pdf_page_links_get(zathura_page_t* page,
    void* poppler_page, zathura_error_t* error);

/**
 * Returns a list of form fields available on the given page
 *
 * @param page Page
 * @param error Set to an error value (see zathura_error_t) if an
 *   error occurred
 * @return List of form fields or NULL if an error occurred
 */
GIRARA_HIDDEN girara_list_t* pdf_page_form_fields_get(zathura_page_t* page,
    void* poppler_page, zathura_error_t* error);

/**
 * Get text for selection
 * @param page Page
 * @param rectangle Selection
 * @error Set to an error value (see \ref zathura_error_t) if an error
 * occurred
 * @return The selected text (needs to be deallocated with g_free)
 */
GIRARA_HIDDEN char* pdf_page_get_text(zathura_page_t* page, void* poppler_page,
    zathura_rectangle_t rectangle, zathura_error_t* error);

GIRARA_HIDDEN girara_list_t*
pdf_page_get_selection(zathura_page_t* page, void* data, zathura_rectangle_t rectangle, zathura_error_t* error);

/**
 * Renders a page onto a cairo object
 *
 * @param page Page
 * @param cairo Cairo object
 * @param printing Set to true if page should be rendered for printing
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_page_render_cairo(zathura_page_t* page, void*
    poppler_page, cairo_t* cairo, bool printing);

/**
 * Get the page label
 *
 * @param page Page
 * @param poppler_page Internal page representation
 * @param label Label
 * @return ZATHURA_ERROR_OK when no error occurred, otherwise see
 *    zathura_error_t
 */
GIRARA_HIDDEN zathura_error_t pdf_page_get_label(zathura_page_t* page,
    void* data, char** label);


/**
 * Get signatures
 *
 * @param poppler_document Internal document representation
 * @param cairo Cairo object
 */
GIRARA_HIDDEN void check_signatures(void* poppler_document);
GIRARA_HIDDEN void hide_signatures(zathura_page_t* zathura_page, PopplerPage* poppler_page, cairo_t* cairo);

#endif // PDF_H
