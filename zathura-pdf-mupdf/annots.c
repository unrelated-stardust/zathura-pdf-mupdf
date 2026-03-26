/* SPDX-License-Identifier: Zlib */

#include "plugin.h"
#include <mupdf/pdf.h>

zathura_error_t pdf_document_annot(zathura_document_t* document) {
  printf("INFO: Successfully called 'pdf_document_annot'\n");
  if (document == NULL) {
    return ZATHURA_ERROR_INVALID_ARGUMENTS;
  }
  printf("INFO: Passed valid document\n");
  mupdf_document_t* mupdf_document = zathura_document_get_data(document);

  g_mutex_lock(&mupdf_document->mutex);

  int n = zathura_document_get_number_of_pages(document);
  for (int i = 0; i < n; ++i) {
    zathura_page_t* page = zathura_document_get_page(document, i);
    mupdf_page_t* mupdf_page = zathura_page_get_data(page);
    pdf_page* pdf_page       = pdf_page_from_fz_page(mupdf_page->ctx, mupdf_page->page);
    pdf_annot* annot = pdf_first_annot(mupdf_page->ctx, pdf_page);
    for (int count = 0; annot != NULL; ++count) {
      enum pdf_annot_type type = pdf_annot_type(mupdf_page->ctx, annot);
      if (type != PDF_ANNOT_TEXT)
        continue;

      const char* contents = pdf_annot_contents(mupdf_page->ctx, annot);
      const char* author = pdf_annot_author(mupdf_page->ctx, annot);
      printf("Annotation (%i/%i) by %s:\n\t%s\n", count+1, i+1, author, contents);
      annot = pdf_next_annot(mupdf_page->ctx, annot);
    }
  }

  g_mutex_unlock(&mupdf_document->mutex);

  return ZATHURA_ERROR_OK;
}
