/* See LICENSE file for license and copyright information */

#include <string.h>
#include <stdio.h>

#include "plugin.h"
#include "utils.h"

zathura_error_t
pdf_document_get_metadata(zathura_document_t* document,
    zathura_list_t** metadata)
{
  if (document == NULL || metadata == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }

  zathura_error_t error = ZATHURA_ERROR_OK;

  PopplerDocument* poppler_document;
  if ((error = zathura_document_get_user_data(document, (void**) &poppler_document)) != ZATHURA_ERROR_OK
      || poppler_document == NULL) {
    return ZATHURA_ERROR_UNKNOWN;
  }

  *metadata = NULL;

  typedef struct info_value_s {
    const char* property;
    zathura_document_meta_type_t type;
  } info_value_t;

  /* get string values */
  static const info_value_t string_values[] = {
    { "title",    ZATHURA_DOCUMENT_META_TITLE },
    { "author",   ZATHURA_DOCUMENT_META_AUTHOR },
    { "subject",  ZATHURA_DOCUMENT_META_SUBJECT },
    { "keywords", ZATHURA_DOCUMENT_META_KEYWORDS },
    { "creator",  ZATHURA_DOCUMENT_META_CREATOR },
    { "producer", ZATHURA_DOCUMENT_META_PRODUCER }
  };

  char* string_value;
  for (unsigned int i = 0; i < LENGTH(string_values); i++) {
    g_object_get(poppler_document, string_values[i].property, &string_value, NULL);

    zathura_document_meta_entry_t* entry;
    if (zathura_document_meta_entry_new(&entry, string_values[i].type, string_value) != ZATHURA_ERROR_OK) {
      continue;
    }

    *metadata = zathura_list_append(*metadata, entry);
  }

  /* get time values */
  static const info_value_t time_values[] = {
    { "creation-date", ZATHURA_DOCUMENT_META_CREATION_DATE },
    { "mod-date",      ZATHURA_DOCUMENT_META_MODIFICATION_DATE }
  };

  for (unsigned int i = 0; i < LENGTH(time_values); i++) {
    gint time_value;
    g_object_get(poppler_document, time_values[i].property, &time_value, NULL);

    time_t r_time_value = time_value;
    char* tmp = ctime(&r_time_value);
    if (tmp != NULL) {
      string_value = g_strndup(tmp, strlen(tmp) - 1);

      zathura_document_meta_entry_t* entry;
      if (zathura_document_meta_entry_new(&entry, time_values[i].type, string_value) != ZATHURA_ERROR_OK) {
        g_free(string_value);
        continue;
      }

      *metadata = zathura_list_append(*metadata, entry);
      g_free(string_value);
    }
  }


  return ZATHURA_ERROR_OK;
}
