/* See LICENSE file for license and copyright information */

#include <check.h>

#include <libzathura/plugin-manager.h>
#include <libzathura/plugin-api.h>

#include "plugin.h"

zathura_document_t* document;
zathura_plugin_manager_t* plugin_manager;

static void setup_document(void) {
  fail_unless(zathura_plugin_manager_new(&plugin_manager) == ZATHURA_ERROR_OK);
  fail_unless(plugin_manager != NULL);
  fail_unless(zathura_plugin_manager_load(plugin_manager, "../build/gcov/pdf.so") == ZATHURA_ERROR_OK);

  zathura_plugin_t* plugin = NULL;
  fail_unless(zathura_plugin_manager_get_plugin(plugin_manager, &plugin, "application/pdf") == ZATHURA_ERROR_OK);
  fail_unless(plugin != NULL);

  fail_unless(zathura_plugin_open_document(plugin, &document, "files/empty.pdf", NULL) == ZATHURA_ERROR_OK);
  fail_unless(document != NULL);
}

static void teardown_document(void) {
  fail_unless(zathura_document_free(document) == ZATHURA_ERROR_OK);
  document = NULL;

  fail_unless(zathura_plugin_manager_free(plugin_manager) == ZATHURA_ERROR_OK);
  plugin_manager = NULL;
}

START_TEST(test_pdf_page_init) {
  /* basic invalid arguments */
  fail_unless(pdf_page_init(NULL) == ZATHURA_ERROR_INVALID_ARGUMENTS);
} END_TEST

START_TEST(test_pdf_page_clear) {
  /* basic invalid arguments */
  fail_unless(pdf_page_clear(NULL) == ZATHURA_ERROR_INVALID_ARGUMENTS);
} END_TEST

Suite*
suite_page(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("page");

  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup_document, teardown_document);
  tcase_add_test(tcase, test_pdf_page_init);
  tcase_add_test(tcase, test_pdf_page_clear);
  suite_add_tcase(suite, tcase);

  return suite;
}
