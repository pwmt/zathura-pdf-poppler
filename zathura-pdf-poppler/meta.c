/* See LICENSE file for license and copyright information */

#include <string.h>

#include "plugin.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

girara_list_t*
pdf_document_get_information(zathura_document_t* document, PopplerDocument*
    poppler_document, zathura_error_t* error)
{
  if (document == NULL || poppler_document == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    return NULL;
  }

  girara_list_t* list = zathura_document_information_entry_list_new();
  if (list == NULL) {
    return NULL;
  }

  /* get string values */
  typedef struct info_value_s {
    const char* property;
    zathura_document_information_type_t type;
  } info_value_t;

  static const info_value_t string_values[] = {
    { "title",    ZATHURA_DOCUMENT_INFORMATION_TITLE },
    { "author",   ZATHURA_DOCUMENT_INFORMATION_AUTHOR },
    { "subject",  ZATHURA_DOCUMENT_INFORMATION_SUBJECT },
    { "keywords", ZATHURA_DOCUMENT_INFORMATION_KEYWORDS },
    { "creator",  ZATHURA_DOCUMENT_INFORMATION_CREATOR },
    { "producer", ZATHURA_DOCUMENT_INFORMATION_PRODUCER }
  };

  char* string_value;
  for (unsigned int i = 0; i < LENGTH(string_values); i++) {
    g_object_get(poppler_document, string_values[i].property, &string_value, NULL);
    zathura_document_information_entry_t* entry = zathura_document_information_entry_new(
        string_values[i].type, string_value);
    if (entry != NULL) {
      girara_list_append(list, entry);
    }
  }

  /* get time values */
  static const info_value_t time_values[] = {
    { "creation-date", ZATHURA_DOCUMENT_INFORMATION_CREATION_DATE },
    { "mod-date",      ZATHURA_DOCUMENT_INFORMATION_MODIFICATION_DATE }
  };

  for (unsigned int i = 0; i < LENGTH(time_values); i++) {
    /* the properties stored in PopplerDocument are gints */
    gint time_value;
    g_object_get(poppler_document, time_values[i].property, &time_value, NULL);
    /* but we need time_ts */
    time_t r_time_value = time_value;
    char* tmp = ctime(&r_time_value);
    if (tmp != NULL) {
      string_value = g_strndup(tmp, strlen(tmp) - 1);
      zathura_document_information_entry_t* entry = zathura_document_information_entry_new(
          time_values[i].type, string_value);
      if (entry != NULL) {
        girara_list_append(list, entry);
      }
      g_free(string_value);
    }
  }

  return list;
}
