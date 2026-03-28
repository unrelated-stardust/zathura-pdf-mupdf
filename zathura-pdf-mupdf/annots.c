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
  pdf_page* pdf_page           = pdf_page_from_fz_page(mupdf_page->ctx, mupdf_page->page);
  if (document == NULL || mupdf_page == NULL || mupdf_page->page == NULL || pdf_page == NULL) {
    goto error_ret;
  }

  mupdf_document_t* mupdf_document = zathura_document_get_data(document);

  girara_list_t* list = girara_list_new_with_free((girara_free_function_t)zathura_annot_free);
  if (list == NULL) {
    if (error != NULL) {
      *error = ZATHURA_ERROR_OUT_OF_MEMORY;
    }
    goto error_free;
  }

  g_mutex_lock(&mupdf_document->mutex);

  pdf_annot* annot = pdf_first_annot(mupdf_page->ctx, pdf_page);
  for (; annot != NULL; annot = pdf_next_annot(mupdf_page->ctx, annot)) {
    enum pdf_annot_type pdf_type = pdf_annot_type(mupdf_page->ctx, annot);
    switch (pdf_type) {
      case PDF_ANNOT_TEXT:
      case PDF_ANNOT_FREE_TEXT:
        break;
      default:
        continue;
        break;
    }
    zathura_annot_type_t zathura_type = ZATHURA_ANNOT_TEXT;

    fz_rect rect = pdf_annot_rect(mupdf_page->ctx, annot);
    zathura_rectangle_t position;
    position.x1 = rect.x0;
    position.x2 = rect.x1;
    position.y1 = rect.y0;
    position.y2 = rect.y1;

    const char* author;
    if (pdf_annot_has_author(mupdf_page->ctx, annot)) {
      author = pdf_annot_author(mupdf_page->ctx, annot);
    } else {
      author = "";
    }
    const char* contents = pdf_annot_contents(mupdf_page->ctx, annot);

    zathura_annot_t* zathura_annot = zathura_annot_new(zathura_type, position, author, contents);
    if (zathura_annot != NULL) {
      girara_list_append(list, zathura_annot);
    }
  }

  g_mutex_unlock(&mupdf_document->mutex);

  return list;

error_free:

  if (list != NULL) {
    girara_list_free(list);
  }

error_ret:

  return NULL;
}
