/* SPDX-License-Identifier: Zlib */

#include "plugin.h"
#include <cairo.h>

#define SIGNATURE_OVERLAY_OFFSET 3
#define SIGNATURE_OVERLAY_ADJUST .5
#define SIGNATURE_OVERLAY_OPACITY .9
#define CAIRO_LINE_OFFSET_VERTICAL 13
#define CAIRO_LINE_OFFSET_HORIZONTAL 5

void print_validation_result(PopplerSignatureInfo* sig_info) {
  const char* cert_status_strings[] = {
    "trusted",  // POPPLER_CERTIFICATE_TRUSTED
    "untrusted issuer",  // POPPLER_CERTIFICATE_UNTRUSTED_ISSUER
    "unknown issuer",  // POPPLER_CERTIFICATE_UNKNOWN_ISSUER
    "revoked",  // POPPLER_CERTIFICATE_REVOKED
    "expired",  // POPPLER_CERTIFICATE_EXPIRED
    "generic error",  // POPPLER_CERTIFICATE_GENERIC_ERROR
    "not verified"  // POPPLER_CERTIFICATE_NOT_VERIFIED
  };

  const char* sig_status_strings[] = {
    "valid",  // POPPLER_SIGNATURE_VALID
    "invalid",  // POPPLER_SIGNATURE_INVALID
    "digest mismatch",  // POPPLER_SIGNATURE_DIGEST_MISMATCH
    "decoding error",  // POPPLER_SIGNATURE_DECODING_ERROR
    "generic error",  // POPPLER_SIGNATURE_GENERIC_ERROR
    "not found",  // POPPLER_SIGNATURE_NOT_FOUND
    "not verified"  // POPPLER_SIGNATURE_NOT_VERIFIED
  };

  PopplerSignatureStatus sig_status = poppler_signature_info_get_signature_status(sig_info);
  PopplerCertificateStatus cert_status = poppler_signature_info_get_certificate_status(sig_info);
  
  printf("signature validation result is %s and certification validation result is %s\n", sig_status_strings[sig_status], cert_status_strings[cert_status]);
}

void cairo_set_color_success(cairo_t* cr) {
  cairo_set_source_rgba(cr, 0.18, 0.8, 0.33, SIGNATURE_OVERLAY_OPACITY); // green
}

void cairo_set_color_warning(cairo_t* cr) {
  cairo_set_source_rgba(cr, 1, 0.84, 0, SIGNATURE_OVERLAY_OPACITY); // yellow
}

void cairo_set_color_error(cairo_t* cr) {
  cairo_set_source_rgba(cr, 0.92, 0.11, 0.14, SIGNATURE_OVERLAY_OPACITY); // red
}

void hide_signatures(zathura_page_t* zathura_page, PopplerPage *poppler_page, cairo_t *cr) {
  const double page_height = zathura_page_get_height(zathura_page);
  GList* form_fields = poppler_page_get_form_field_mapping(poppler_page);

  for (GList* entry = form_fields; entry && entry->data; entry = g_list_next(entry)) {
    PopplerFormFieldMapping* mapping = (PopplerFormFieldMapping*) entry->data;
    PopplerRectangle area = mapping->area;
    PopplerFormField* form_field = mapping->field;

    if (poppler_form_field_get_field_type(form_field) == POPPLER_FORM_FIELD_SIGNATURE) {

      // build rectangle
      gdouble width = area.x2 - area.x1 - SIGNATURE_OVERLAY_OFFSET;
      gdouble height = area.y2 - area.y1 - SIGNATURE_OVERLAY_OFFSET;
      gdouble translated_y1 = page_height - area.y1 - height - SIGNATURE_OVERLAY_OFFSET/2 - SIGNATURE_OVERLAY_ADJUST; // translate because (0,0) is top left for pdf but bottom left for cairo
      gdouble translated_x1 = area.x1 + SIGNATURE_OVERLAY_OFFSET/2 + SIGNATURE_OVERLAY_ADJUST;
      cairo_rectangle(cr, translated_x1, translated_y1, width, height);

      // get signature info (Poppler appears to have issues with performing revocation check, therefore disabled for now)
      int flags = POPPLER_SIGNATURE_VALIDATION_FLAG_VALIDATE_CERTIFICATE | POPPLER_SIGNATURE_VALIDATION_FLAG_WITHOUT_OCSP_REVOCATION_CHECK | POPPLER_SIGNATURE_VALIDATION_FLAG_USE_AIA_CERTIFICATE_FETCH;
      PopplerSignatureInfo* sig_info = poppler_form_field_signature_validate_sync(form_field, flags, NULL, NULL);
      print_validation_result(sig_info);
      
      PopplerSignatureStatus sig_status = poppler_signature_info_get_signature_status(sig_info);
      PopplerCertificateStatus cert_status = poppler_signature_info_get_certificate_status(sig_info);

      // start building text
      GPtrArray* text = g_ptr_array_new();

      // define rectangle color and content
      switch (sig_status) {
      case POPPLER_SIGNATURE_VALID:
        
        switch (cert_status) {
        case POPPLER_CERTIFICATE_TRUSTED:
          g_ptr_array_add(text, g_string_new("Signature is valid."));

          // get signer information
          GDateTime* sig_time = poppler_signature_info_get_local_signing_time(sig_info);
          gchar* sig_time_str = g_date_time_format(sig_time, "%F %T");
          const gchar* sig_name = poppler_signature_info_get_signer_name(sig_info);
          
          g_ptr_array_add(text, g_string_new("This document is signed by"));
          g_ptr_array_add(text, g_string_append(g_string_new("  "), sig_name));
          g_ptr_array_add(text, g_string_append(g_string_new("on "), sig_time_str));

          g_free(sig_time_str);

          cairo_set_color_success(cr);
          break;
        case POPPLER_CERTIFICATE_UNTRUSTED_ISSUER:
        case POPPLER_CERTIFICATE_UNKNOWN_ISSUER:
          g_ptr_array_add(text, g_string_new("Signature certificate is not trusted."));
          cairo_set_color_error(cr);
          break;
        case POPPLER_CERTIFICATE_REVOKED:
          g_ptr_array_add(text, g_string_new("Signature certificate is invalid."));
          cairo_set_color_error(cr);
          break;
        case POPPLER_CERTIFICATE_EXPIRED:
          g_ptr_array_add(text, g_string_new("Signature certificate is expired."));
          cairo_set_color_warning(cr);
          break;  
        default: // CERTIFICATE NOT VERIFIED or GENERIC ERROR
          g_ptr_array_add(text, g_string_new("Signature certificate could not be verified."));
          cairo_set_color_error(cr);
          break;
        }

        break;
      case POPPLER_SIGNATURE_GENERIC_ERROR:
      case POPPLER_SIGNATURE_NOT_FOUND:
      case POPPLER_SIGNATURE_NOT_VERIFIED:
        g_ptr_array_add(text, g_string_new("Signature could not be verified."));
        cairo_set_color_error(cr);
        break;
      default: // SIGNATURE INVALID or DIGEST MISMATCH or DECODING ERROR
        g_ptr_array_add(text, g_string_new("Signature is invalid."));
        cairo_set_color_error(cr);
        break;
      }

      // fill rectangle
      cairo_fill(cr);

      // draw text
      cairo_set_source_rgb(cr, 0, 0, 0);
      for (guint i = 0; i < text->len; i++) {
        GString* line = (GString*) g_ptr_array_index(text, i);
        cairo_move_to(cr, translated_x1 + CAIRO_LINE_OFFSET_HORIZONTAL, translated_y1 + (i+1) * CAIRO_LINE_OFFSET_VERTICAL);
        cairo_show_text(cr, line->str);
        g_string_free(line, TRUE);
      }
      
      // free stuff
      g_ptr_array_unref(text);
      poppler_signature_info_free(sig_info);
    }
  }

  poppler_page_free_form_field_mapping(form_fields);
}


