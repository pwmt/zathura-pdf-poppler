/* See LICENSE file for license and copyright information */

#include <stdio.h>
#include <stdlib.h>

#include "../plugin.h"
#include "../internal.h"
#include "utils.h"

static zathura_error_t
poppler_form_field_to_zathura_form_field(zathura_page_t* page, PopplerFormField* poppler_form_field,
    zathura_form_field_t** form_field);

zathura_error_t
pdf_page_get_form_fields(zathura_page_t* page, zathura_list_t** form_fields)
{
  if (page == NULL || form_fields == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  *form_fields = NULL;

  zathura_document_t* document;
  if ((error = zathura_page_get_document(page, &document)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  pdf_page_t* pdf_page;
  if ((error = zathura_page_get_user_data(page, (void**) &pdf_page)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerPage* poppler_page = pdf_page->poppler_page;

  unsigned int page_height;
  if ((error = zathura_page_get_height(page, &page_height)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_user_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK ||
      poppler_document == NULL) {
    goto error_out;
  }

  GList* form_field_mapping = pdf_page->form_field_mapping;
  if (form_field_mapping == NULL) {
    form_field_mapping = poppler_page_get_form_field_mapping(poppler_page);
    if (form_field_mapping == NULL || g_list_length(form_field_mapping) == 0) {
      error = ZATHURA_ERROR_UNKNOWN;
      goto error_free;
    }
  }

  for (GList* iter = form_field_mapping; iter != NULL; iter = g_list_next(iter)) {
    zathura_form_field_mapping_t* mapping = calloc(1, sizeof(zathura_form_field_mapping_t));
    if (mapping == NULL) {
      goto error_free;
    }

    PopplerFormFieldMapping* poppler_mapping = (PopplerFormFieldMapping*) iter->data;
    zathura_form_field_t* converted_form_field;
    if (poppler_form_field_to_zathura_form_field(page, poppler_mapping->field,
          &converted_form_field) != ZATHURA_ERROR_OK) {
      free(mapping);
      continue;
    }

    zathura_rectangle_t position = { {0, 0}, {0, 0} };
    position.p1.x = poppler_mapping->area.x1;
    position.p2.x = poppler_mapping->area.x2;
    position.p1.y = page_height - poppler_mapping->area.y2;
    position.p2.y = page_height - poppler_mapping->area.y1;

    mapping->position   = position;
    mapping->form_field = converted_form_field;

    *form_fields = zathura_list_append(*form_fields, mapping);
  }

  pdf_page->form_field_mapping = form_field_mapping;

  return error;

error_free:

  if (form_field_mapping != NULL) {
    poppler_page_free_form_field_mapping(form_field_mapping);
  }

error_out:

  return error;
}

zathura_error_t
pdf_form_field_save(zathura_form_field_t* form_field)
{
  if (form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_form_field_type_t type;
  if (zathura_form_field_get_type(form_field, &type) != ZATHURA_ERROR_OK) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  PopplerFormField* poppler_form_field;
  if (zathura_form_field_get_user_data(form_field, (void**) &poppler_form_field) != ZATHURA_ERROR_OK ||
      poppler_form_field == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  switch (type) {
    case ZATHURA_FORM_FIELD_UNKNOWN:
      return ZATHURA_ERROR_INVALID_ARGUMENTS;
    case ZATHURA_FORM_FIELD_BUTTON:
      return zathura_form_field_button_save_to_poppler_form_field(form_field, poppler_form_field);
    case ZATHURA_FORM_FIELD_TEXT:
      return zathura_form_field_text_save_to_poppler_form_field(form_field, poppler_form_field);
    case ZATHURA_FORM_FIELD_CHOICE:
      return zathura_form_field_choice_save_to_poppler_form_field(form_field, poppler_form_field);
    case ZATHURA_FORM_FIELD_SIGNATURE:
      return zathura_form_field_signature_save_to_poppler_form_field(form_field, poppler_form_field);
  }

  return ZATHURA_ERROR_UNKNOWN;
}

static zathura_error_t
poppler_form_field_to_zathura_form_field(zathura_page_t* page, PopplerFormField* poppler_form_field,
    zathura_form_field_t** form_field)
{
  if (page == NULL || poppler_form_field == NULL || form_field == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;
  zathura_form_field_type_t zathura_type = poppler_form_field_to_zathura_form_field_type(poppler_form_field);

  if ((error = zathura_form_field_new(page, form_field, zathura_type)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  if ((error = zathura_form_field_set_user_data(*form_field, poppler_form_field, NULL)) != ZATHURA_ERROR_OK) {
    goto error_out;
  }

  gchar* name = poppler_form_field_get_name(poppler_form_field);
  if (name != NULL && (error = zathura_form_field_set_name(*form_field, name)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  gchar* partial_name = poppler_form_field_get_partial_name(poppler_form_field);
  if (partial_name != NULL &&
      (error = zathura_form_field_set_partial_name(*form_field, partial_name)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  gchar* mapping_name = poppler_form_field_get_mapping_name(poppler_form_field);
  if (mapping_name != NULL &&
      (error = zathura_form_field_set_mapping_name(*form_field, mapping_name)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  if (poppler_form_field_is_read_only(poppler_form_field) == TRUE &&
      (error = zathura_form_field_set_flags(*form_field, ZATHURA_FORM_FIELD_FLAG_READ_ONLY)) != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  switch (zathura_type) {
    case ZATHURA_FORM_FIELD_UNKNOWN:
      return ZATHURA_ERROR_OK;
    case ZATHURA_FORM_FIELD_BUTTON:
      error = poppler_form_field_button_to_zathura_form_field(*form_field, poppler_form_field);
      break;
    case ZATHURA_FORM_FIELD_TEXT:
      error = poppler_form_field_text_to_zathura_form_field(*form_field, poppler_form_field);
      break;
    case ZATHURA_FORM_FIELD_CHOICE:
      error = poppler_form_field_choice_to_zathura_form_field(*form_field, poppler_form_field);
      break;
    case ZATHURA_FORM_FIELD_SIGNATURE:
      error = poppler_form_field_signature_to_zathura_form_field(*form_field, poppler_form_field);
      break;
  }

  if (error != ZATHURA_ERROR_OK) {
    goto error_free;
  }

  return ZATHURA_ERROR_OK;

error_free:

  zathura_form_field_free(*form_field);

error_out:

  return error;
}
