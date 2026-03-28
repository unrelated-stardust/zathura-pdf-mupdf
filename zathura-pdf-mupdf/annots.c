/* SPDX-License-Identifier: Zlib */

#include "plugin.h"
#include <mupdf/pdf.h>

girara_list_t* pdf_page_annots_get(zathura_page_t* page, void* data, zathura_error_t* error) {
  if (page == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_INVALID_ARGUMENTS;
    }
    goto error_ret;
  }

  mupdf_page_t* mupdf_page     = data;
  zathura_document_t* document = zathura_page_get_document(page);
  pdf_page* pdf_page       = pdf_page_from_fz_page(mupdf_page->ctx, mupdf_page->page);
  if (document == NULL || mupdf_page == NULL || mupdf_page->page == NULL || pdf_page == NULL) {
    goto error_ret;
  }

  mupdf_document_t* mupdf_document = zathura_document_get_data(document);

  girara_list_t* list = girara_list_new_with_free((girara_free_function_t)zathura_link_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  g_mutex_lock(&mupdf_document->mutex);

  int count = 0;
  pdf_annot* annot = pdf_first_annot(mupdf_page->ctx, pdf_page);
  for (; annot != NULL; ++count) {
    enum pdf_annot_type type = pdf_annot_type(mupdf_page->ctx, annot);
    if (type != PDF_ANNOT_TEXT)
      continue;

    const char* contents = pdf_annot_contents(mupdf_page->ctx, annot);
    const char* author = pdf_annot_author(mupdf_page->ctx, annot);
    printf("Annotation (%i) by %s:\n\t%s\n", count+1, author, contents);
    annot = pdf_next_annot(mupdf_page->ctx, annot);
  }

  if (count == 0)
    printf("No annotations found on this page\n");

  g_mutex_unlock(&mupdf_document->mutex);

  return list;

error_free:

  if (list != NULL) {
    girara_list_free(list);
  }

error_ret:

  return NULL;
}
