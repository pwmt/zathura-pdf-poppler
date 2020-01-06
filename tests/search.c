/* See LICENSE file for license and copyright information */

#include <check.h>
#include <fiu.h>
#include <fiu-control.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib/gstdio.h>

#include <libzathura/plugin-manager.h>
#include <libzathura/plugin-api.h>
#include <libzathura/libzathura.h>
#include <libzathura/libzathura.h>

#include <zathura-pdf-poppler/plugin.h>
#include "utils.h"

zathura_document_t* document;
zathura_plugin_manager_t* plugin_manager;
zathura_page_t* page;

static void setup_document_empty(void) {
  setup_document_with_path(&plugin_manager, &document, "files/search.pdf");
  fail_unless(zathura_document_get_page(document, 0, &page) == ZATHURA_ERROR_OK);
}

static void teardown_document(void) {
  fail_unless(zathura_document_free(document) == ZATHURA_ERROR_OK);
  document = NULL;

  fail_unless(zathura_plugin_manager_free(plugin_manager) == ZATHURA_ERROR_OK);
  plugin_manager = NULL;
}

START_TEST(test_pdf_page_search_text_invalid) {
  zathura_list_t* results;

  fail_unless(pdf_page_search_text(NULL, NULL, ZATHURA_SEARCH_DEFAULT, NULL) == ZATHURA_ERROR_INVALID_ARGUMENTS);
  fail_unless(pdf_page_search_text(page, NULL, ZATHURA_SEARCH_DEFAULT, NULL) == ZATHURA_ERROR_INVALID_ARGUMENTS);
  fail_unless(pdf_page_search_text(page, "abc", ZATHURA_SEARCH_DEFAULT, NULL) == ZATHURA_ERROR_INVALID_ARGUMENTS);
  fail_unless(pdf_page_search_text(page, "", ZATHURA_SEARCH_DEFAULT, &results) == ZATHURA_ERROR_INVALID_ARGUMENTS);
} END_TEST

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static void
compare_rectangle(zathura_rectangle_t* rect1, zathura_rectangle_t* rect2)
{
  fail_unless(rect1->p1.x == rect2->p1.x);
  fail_unless(rect1->p1.y == rect2->p1.y);
  fail_unless(rect1->p2.x == rect2->p2.x);
  fail_unless(rect1->p2.y == rect2->p2.y);
}

START_TEST(test_pdf_page_search_text_default) {
  zathura_list_t* results;
  fail_unless(pdf_page_search_text(page, "abc", ZATHURA_SEARCH_DEFAULT, &results) == ZATHURA_ERROR_OK);
  fail_unless(results != NULL);
  fail_unless(zathura_list_length(results) == 3);

  zathura_rectangle_t result_rectangles[] = {
    { {56.799999, 70.491997}, {73.419998, 57.208000} },
    { {56.799999, 98.092003}, { 73.419998, 84.807999 } },
    { {56.799999, 125.692001}, {76.804001, 112.407997 } }
  };

  fail_unless(zathura_list_length(results) == LENGTH(result_rectangles));
  for (unsigned int i = 0; i < LENGTH(result_rectangles); i++) {
    zathura_rectangle_t* rectangle = zathura_list_nth_data(results, i);
    compare_rectangle(rectangle, &result_rectangles[i]);
  }

  zathura_list_free_full(results, free);

  /* fault injection */
  fiu_enable("libc/mm/calloc", 1, NULL, 0);
  fail_unless(pdf_page_search_text(page, "abc", ZATHURA_SEARCH_DEFAULT, &results) == ZATHURA_ERROR_OUT_OF_MEMORY);
  fail_unless(results == NULL);
  fiu_disable("libc/mm/calloc");
} END_TEST

START_TEST(test_pdf_page_search_text_case_sensitive) {
  zathura_list_t* results;
  fail_unless(pdf_page_search_text(page, "abc", ZATHURA_SEARCH_CASE_SENSITIVE, &results) == ZATHURA_ERROR_OK);
  fail_unless(results != NULL);
  fail_unless(zathura_list_length(results) == 2);

  zathura_rectangle_t result_rectangles[] = {
    { {56.799999, 70.491997}, {73.419998, 57.208000} },
    { {56.799999, 98.092003}, { 73.419998, 84.807999 } }
  };

  fail_unless(zathura_list_length(results) == LENGTH(result_rectangles));
  for (unsigned int i = 0; i < LENGTH(result_rectangles); i++) {
    zathura_rectangle_t* rectangle = zathura_list_nth_data(results, i);
    compare_rectangle(rectangle, &result_rectangles[i]);
  }

  zathura_list_free_full(results, free);
} END_TEST

START_TEST(test_pdf_page_search_text_case_whole_words) {
  zathura_list_t* results;

  /* no results */
  fail_unless(pdf_page_search_text(page, "ab", ZATHURA_SEARCH_WHOLE_WORDS_ONLY, &results) == ZATHURA_ERROR_SEARCH_NO_RESULTS);

  /* results */
  fail_unless(pdf_page_search_text(page, "Abc", ZATHURA_SEARCH_WHOLE_WORDS_ONLY, &results) == ZATHURA_ERROR_OK);
  fail_unless(results != NULL);
  fail_unless(zathura_list_length(results) == 3);

  zathura_rectangle_t result_rectangles[] = {
    { {56.799999, 70.491997}, {73.419998, 57.208000} },
    { {56.799999, 98.092003}, { 73.419998, 84.807999 } },
    { {56.799999, 125.692001}, {76.804001, 112.407997 } }
  };

  fail_unless(zathura_list_length(results) == LENGTH(result_rectangles));
  for (unsigned int i = 0; i < LENGTH(result_rectangles); i++) {
    zathura_rectangle_t* rectangle = zathura_list_nth_data(results, i);
    compare_rectangle(rectangle, &result_rectangles[i]);
  }

  zathura_list_free_full(results, free);
} END_TEST

Suite*
create_suite(void)
{
  TCase* tcase = NULL;
  Suite* suite = suite_create("search");

  tcase = tcase_create("basic");
  tcase_add_checked_fixture(tcase, setup_document_empty, teardown_document);
  tcase_add_test(tcase, test_pdf_page_search_text_invalid);
  tcase_add_test(tcase, test_pdf_page_search_text_default);
  tcase_add_test(tcase, test_pdf_page_search_text_case_sensitive);
  tcase_add_test(tcase, test_pdf_page_search_text_case_whole_words);
  suite_add_tcase(suite, tcase);

  return suite;
}
