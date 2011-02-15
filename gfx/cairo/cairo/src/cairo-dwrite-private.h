/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* Cairo - a vector graphics library with display and print output
 *
 * Copyright � 2010 Mozilla Foundation
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is the Mozilla Foundation
 *
 * Contributor(s):
 *	Bas Schouten <bschouten@mozilla.com>
 */
#include <dwrite.h>
#include <D2d1.h>


// DirectWrite is not available on all platforms.
typedef HRESULT (WINAPI*DWriteCreateFactoryFunc)(
  __in   DWRITE_FACTORY_TYPE factoryType,
  __in   REFIID iid,
  __out  IUnknown **factory
);


class DWriteFactory
{
public:
    static IDWriteFactory *Instance()
    {
	if (!mFactoryInstance) {
	    DWriteCreateFactoryFunc createDWriteFactory = (DWriteCreateFactoryFunc)
		GetProcAddress(LoadLibraryW(L"dwrite.dll"), "DWriteCreateFactory");
	    if (createDWriteFactory) {
		HRESULT hr = createDWriteFactory(
		    DWRITE_FACTORY_TYPE_SHARED,
		    __uuidof(IDWriteFactory),
		    reinterpret_cast<IUnknown**>(&mFactoryInstance));
		assert(SUCCEEDED(hr));
	    }
	}
	return mFactoryInstance;
    }

    static IDWriteFontCollection *SystemCollection()
    {
	if (!mSystemCollection) {
	    if (Instance()) {
		HRESULT hr = Instance()->GetSystemFontCollection(&mSystemCollection);
		assert(SUCCEEDED(hr));
	    }
	}
	return mSystemCollection;
    }

    static IDWriteFontFamily *FindSystemFontFamily(const WCHAR *aFamilyName)
    {
	UINT32 idx;
	BOOL found;
	if (!SystemCollection()) {
	    return NULL;
	}
	SystemCollection()->FindFamilyName(aFamilyName, &idx, &found);
	if (!found) {
	    return NULL;
	}

	IDWriteFontFamily *family;
	SystemCollection()->GetFontFamily(idx, &family);
	return family;
    }

private:
    static IDWriteFactory *mFactoryInstance;
    static IDWriteFontCollection *mSystemCollection;
};

/* cairo_font_face_t implementation */
struct _cairo_dwrite_font_face {
    cairo_font_face_t base;
    IDWriteFont *font;
    IDWriteFontFace *dwriteface;
};
typedef struct _cairo_dwrite_font_face cairo_dwrite_font_face_t;

/* cairo_scaled_font_t implementation */
struct _cairo_dwrite_scaled_font {
    cairo_scaled_font_t base;
    cairo_matrix_t mat;
    cairo_matrix_t mat_inverse;
    cairo_antialias_t antialias_mode;
    DWRITE_MEASURING_MODE measuring_mode;
};
typedef struct _cairo_dwrite_scaled_font cairo_dwrite_scaled_font_t;

DWRITE_MATRIX _cairo_dwrite_matrix_from_matrix(const cairo_matrix_t *matrix);

// This will create a DWrite glyph run from cairo glyphs and a scaled_font.
// It is important to note the array members of DWRITE_GLYPH_RUN should be
// deleted by the caller.
void
_cairo_dwrite_glyph_run_from_glyphs(cairo_glyph_t *glyphs,
				    int num_glyphs,
				    cairo_dwrite_scaled_font_t *scaled_font,
				    DWRITE_GLYPH_RUN *run,
				    cairo_bool_t *transformed);
