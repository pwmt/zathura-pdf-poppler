/* See LICENSE file for license and copyright information */

#ifndef ZATHURA_PDF_POPPLER_FORM_FIELDS_UTILS_H
#define ZATHURA_PDF_POPPLER_FORM_FIELDS_UTILS_H

#include <libzathura/plugin-api.h>
#include <poppler.h>

zathura_form_field_type_t poppler_form_field_to_zathura_form_field_type(PopplerFormField* poppler_form_field);

zathura_error_t poppler_form_field_button_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);
zathura_error_t poppler_form_field_text_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);
zathura_error_t poppler_form_field_choice_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);
zathura_error_t poppler_form_field_signature_to_zathura_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);

zathura_error_t zathura_form_field_button_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);
zathura_error_t zathura_form_field_text_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);
zathura_error_t zathura_form_field_choice_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);
zathura_error_t zathura_form_field_signature_save_to_poppler_form_field(zathura_form_field_t* form_field,
    PopplerFormField* poppler_form_field);

#endif
