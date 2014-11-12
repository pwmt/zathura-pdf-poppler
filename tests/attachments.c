/* See LICENSE file for license and copyright information */

#include <check.h>
#include <stdio.h>
#include <glib/gstdio.h>

#include <libzathura/plugin-manager.h>
#include <libzathura/plugin-api.h>
#include <libzathura/libzathura.h>

#include "plugin.h"
#include "utils.h"

zathura_document_t* document;
zathura_plugin_manager_t* plugin_manager;

static void setup_document_empty(void) {
  setup_document_with_path(&plugin_manager, &document, "files/empty.pdf");
}

static void setup_document_attachments(void) {
  setup_document_with_path(&plugin_manager, &document, "files/attachments.pdf");
}

static void teardown_document(void) {
  fail_unless(zathura_document_free(document) == ZATHURA_ERROR_OK);
  document = NULL;

  fail_unless(zathura_plugin_manager_free(plugin_manager) == ZATHURA_ERROR_OK);
  plugin_manager = NULL;
}

START_TEST(test_pdf_document_get_attachments_invalid) {
  fail_unless(pdf_document_get_attachments(NULL, NULL)     == ZATHURA_ERROR_INVALID_ARGUMENTS);
  fail_unless(pdf_document_get_attachments(document, NULL) == ZATHURA_ERROR_INVALID_ARGUMENTS);
} END_TEST

START_TEST(test_pdf_document_get_attachments) {
  zathura_list_t* attachments;
  fail_unless(pdf_document_get_attachments(document, &attachments) == ZATHURA_ERROR_OK);
} END_TEST

Suite*
suite_attachments(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("attachments");

  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup_document_empty, teardown_document);
  tcase_add_test(tcase, test_pdf_document_get_attachments_invalid);
  suite_add_tcase(suite, tcase);

  tcase = tcase_create("has-attachment");
  tcase_add_checked_fixture(tcase, setup_document_attachments, teardown_document);
  tcase_add_test(tcase, test_pdf_document_get_attachments);
  suite_add_tcase(suite, tcase);

  return suite;
}
