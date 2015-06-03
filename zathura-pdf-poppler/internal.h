/* See LICENSE file for license and copyright information */

#ifndef INTERNAL_H
#define INTERNAL_H

#include "macros.h"

IGNORE_UNUSED_PARAMETER_BEGIN
#include <GlobalParams.h>
#include <PDFDoc.h>
#include <ErrorCodes.h>
#ifdef HAVE_CAIRO
#include <CairoOutputDev.h>
#endif
IGNORE_UNUSED_PARAMETER_END

typedef struct pdf_document_s {
  PDFDoc* poppler_document;

#ifdef HAVE_CAIRO
  CairoOutputDev* output_device;
#endif
} pdf_document_t;

typedef struct pdf_page_s {
  Page* poppler_page;
} pdf_page_t;

#endif /* INTERNAL_H */
