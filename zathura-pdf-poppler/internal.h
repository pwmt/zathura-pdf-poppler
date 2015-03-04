/* See LICENSE file for license and copyright information */

#ifndef INTERNAL_H
#define INTERNAL_H

typedef struct pdf_page_s {
  PopplerPage* poppler_page;
  GList* form_field_mapping;
} pdf_page_t;

#endif /* INTERNAL_H */
