/* SPDX-License-Identifier: Zlib */

#include <cairo.h>
#include <girara/log.h>

#include "plugin.h"

#define SIGNATURE_OVERLAY_OFFSET 3
#define SIGNATURE_OVERLAY_ADJUST .5
#define SIGNATURE_OVERLAY_OPACITY .9
#define CAIRO_LINE_OFFSET_VERTICAL 13
#define CAIRO_LINE_OFFSET_HORIZONTAL 5

static void print_validation_result(PopplerSignatureInfo* sig_info) {
  static const char* const cert_status_strings[] = {
      "trusted",          // POPPLER_CERTIFICATE_TRUSTED
      "untrusted issuer", // POPPLER_CERTIFICATE_UNTRUSTED_ISSUER
      "unknown issuer",   // POPPLER_CERTIFICATE_UNKNOWN_ISSUER
      "revoked",          // POPPLER_CERTIFICATE_REVOKED
      "expired",          // POPPLER_CERTIFICATE_EXPIRED
      "generic error",    // POPPLER_CERTIFICATE_GENERIC_ERROR
      "not verified"      // POPPLER_CERTIFICATE_NOT_VERIFIED
  };

  static const char* const sig_status_strings[] = {
      "valid",           // POPPLER_SIGNATURE_VALID
      "invalid",         // POPPLER_SIGNATURE_INVALID
      "digest mismatch", // POPPLER_SIGNATURE_DIGEST_MISMATCH
      "decoding error",  // POPPLER_SIGNATURE_DECODING_ERROR
      "generic error",   // POPPLER_SIGNATURE_GENERIC_ERROR
      "not found",       // POPPLER_SIGNATURE_NOT_FOUND
      "not verified"     // POPPLER_SIGNATURE_NOT_VERIFIED
  };

  PopplerSignatureStatus sig_status    = poppler_signature_info_get_signature_status(sig_info);
  PopplerCertificateStatus cert_status = poppler_signature_info_get_certificate_status(sig_info);

  girara_debug("signature validation result: '%s'\ncertification validation: '%s'", sig_status_strings[sig_status],
               cert_status_strings[cert_status]);
}

static void signature_info_free(void* data) {
  zathura_signature_info_t* signature_info = data;
  zathura_signature_info_free(signature_info);
}

girara_list_t* pdf_page_get_signatures(zathura_page_t* page, void* data, zathura_error_t* error) {
  if (page == NULL || data == NULL) {
    if (error) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  girara_list_t* signatures = girara_list_new2(signature_info_free);

  PopplerPage* poppler_page = data;
  const double page_height  = zathura_page_get_height(page);
  GList* form_fields        = poppler_page_get_form_field_mapping(poppler_page);

  for (GList* entry = form_fields; entry && entry->data; entry = g_list_next(entry)) {
    PopplerFormFieldMapping* mapping = (PopplerFormFieldMapping*)entry->data;
    PopplerFormField* form_field     = mapping->field;
    if (poppler_form_field_get_field_type(form_field) != POPPLER_FORM_FIELD_SIGNATURE) {
      continue;
    }

    zathura_signature_info_t* signature = zathura_signature_info_new();
    signature->position.x1              = mapping->area.x1;
    signature->position.x2              = mapping->area.x2;
    signature->position.y1              = page_height - mapping->area.y2;
    signature->position.y2              = page_height - mapping->area.y1;

    // get signature info (Poppler appears to have issues with performing revocation check, therefore disabled for now)
    static const int flags = POPPLER_SIGNATURE_VALIDATION_FLAG_VALIDATE_CERTIFICATE |
                             POPPLER_SIGNATURE_VALIDATION_FLAG_WITHOUT_OCSP_REVOCATION_CHECK |
                             POPPLER_SIGNATURE_VALIDATION_FLAG_USE_AIA_CERTIFICATE_FETCH;
    PopplerSignatureInfo* sig_info = poppler_form_field_signature_validate_sync(form_field, flags, NULL, NULL);
    if (girara_get_log_level() == GIRARA_DEBUG) {
      print_validation_result(sig_info);
    }

    switch (poppler_signature_info_get_signature_status(sig_info)) {
    case POPPLER_SIGNATURE_VALID:
      switch (poppler_signature_info_get_certificate_status(sig_info)) {
      case POPPLER_CERTIFICATE_TRUSTED:
        signature->signer = g_strdup(poppler_signature_info_get_signer_name(sig_info));
        signature->time   = g_date_time_ref(poppler_signature_info_get_local_signing_time(sig_info));
        signature->state  = ZATHURA_SIGNATURE_VALID;
        break;
      case POPPLER_CERTIFICATE_UNTRUSTED_ISSUER:
      case POPPLER_CERTIFICATE_UNKNOWN_ISSUER:
        signature->state = ZATHURA_SIGNATURE_CERTIFICATE_UNTRUSTED;
        break;
      case POPPLER_CERTIFICATE_REVOKED:
        signature->state = ZATHURA_SIGNATURE_CERTIFICATE_REVOKED;
        break;
      case POPPLER_CERTIFICATE_EXPIRED:
        signature->state = ZATHURA_SIGNATURE_CERTIFICATE_EXPIRED;
        break;
      default: // CERTIFICATE NOT VERIFIED or GENERIC ERROR
        signature->state = ZATHURA_SIGNATURE_CERTIFICATE_INVALID;
        break;
      }

      break;
    case POPPLER_SIGNATURE_GENERIC_ERROR:
    case POPPLER_SIGNATURE_NOT_FOUND:
    case POPPLER_SIGNATURE_NOT_VERIFIED:
      signature->state = ZATHURA_SIGNATURE_ERROR;
      break;
    default: // SIGNATURE INVALID or DIGEST MISMATCH or DECODING ERROR
      signature->state = ZATHURA_SIGNATURE_INVALID;
      break;
    }

    poppler_signature_info_free(sig_info);
    girara_list_append(signatures, signature);
  }

  poppler_page_free_form_field_mapping(form_fields);
  return signatures;
}
