/* 
 * Copyright © 2005 Christian Persch
 * Copyright © 2005 Robert O'Callahan
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * * some parts of code copied from mozilla/layout/base/nsDocumentViewer.cpp:
 * *  Dan Rosen <dr@netscape.com>
 * *  Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
 *
 * $Id$
 */

#include "mozilla-config.h"
#include "config.h"

#include "Writer.h"

#include <nsCOMPtr.h>
#include <nsIWebBrowser.h>
#include <nsIDOMWindow.h>
#include <nsIDOMWindow2.h>
#include <nsIDOMDocument.h>
#include <nsIDocument.h>
#include <nsIDocShell.h>
#include <nsIDocumentViewer.h>
#include <nsIMarkupDocumentViewer.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsIPresShell.h>
#include <nsIDeviceContext.h>
#include <nsIViewManager.h>
#include <nsIScrollableView.h>
#include <nsIDOMNSDocument.h>
#include <gfxContext.h>
#include <gfxPlatform.h>
#include <nsRect.h>
#include <gtkmozembed_internal.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define MAX_WIDTH		1 << 12
#define MAX_HEIGHT		1 << 20
#define THUMBNAIL_WIDTH		1 << 10
#define THUMBNAIL_HEIGHT	1 << 10
#define STRIPE			1 << 8

#ifdef GNOME_ENABLE_DEBUG
#define LOG g_print
#else
#define LOG //
#endif

/* Writer */

Writer::Writer(GtkMozEmbed *aEmbed,
	       const char *aFilename)
: mEmbed(aEmbed)
, mFilename(aFilename)
, mWidth(MAX_WIDTH)
, mHeight(MAX_HEIGHT)
, mInitialised(PR_FALSE)
, mHadError(PR_FALSE)
{
}

PRBool
Writer::Write()
{
  NS_ENSURE_STATE (mEmbed);

  nsresult rv;
  nsCOMPtr<nsIWebBrowser> browser;
  gtk_moz_embed_get_nsIWebBrowser (mEmbed, getter_AddRefs (browser));
  NS_ENSURE_TRUE (browser, PR_FALSE);

  nsCOMPtr<nsIDOMWindow> domWin;
  rv = browser->GetContentDOMWindow (getter_AddRefs (domWin));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  nsCOMPtr<nsIDOMDocument> domDoc;
  rv = domWin->GetDocument (getter_AddRefs (domDoc));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  nsCOMPtr<nsIDocument> doc (do_QueryInterface (domDoc, &rv));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  /* Get document information */
  nsString title;
  nsCOMPtr<nsIDOMNSDocument> domNSDoc = do_QueryInterface(doc);
  if (domNSDoc)
    domNSDoc->GetTitle(title);
  CopyUTF16toUTF8 (title, mTitle);

  nsIURI *uri = doc->GetDocumentURI();
  if (uri) {
    uri->GetSpec(mSpec);
  }

  doc->FlushPendingNotifications (Flush_Display);

  nsCOMPtr<nsIDocShell> docShell (do_GetInterface (browser, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIPresShell> presShell;
  rv = docShell->GetPresShell (getter_AddRefs (presShell));
  NS_ENSURE_SUCCESS (rv, PR_FALSE);

  nsIViewManager *viewManager = presShell->GetViewManager ();
  NS_ENSURE_TRUE (viewManager, PR_FALSE);

  nsIDeviceContext *dc; 
  viewManager->GetDeviceContext (dc);
  NS_ENSURE_TRUE (dc, PR_FALSE);

  nsIScrollableView* scrollableView = nsnull;
  viewManager->GetRootScrollableView (&scrollableView);
  nsIView* view;
  if (scrollableView) {
    scrollableView->GetScrolledView (view);
  } else {
    viewManager->GetRootView(view);
  }

  PRInt32 p2a = dc->AppUnitsPerDevPixel();

  /* Limit the bitmap size */
  nscoord twipLimitW = NSIntPixelsToAppUnits(mWidth, p2a);
  nscoord twipLimitH = NSIntPixelsToAppUnits(mHeight, p2a);

  nsRect r = view->GetBounds() - view->GetPosition();

  if (r.height > twipLimitH) {
    r.height = twipLimitH;
  }
  if (r.width > twipLimitW) {
    r.width = twipLimitW;
  }

  /* sanity check */
  if (r.IsEmpty()) {
    LOG ("Rect is empty!\n");
    return PR_FALSE;
  }

  mWidth = NSAppUnitsToIntPixels(r.width, p2a);
  mHeight = NSAppUnitsToIntPixels(r.height, p2a);

  PRUint32 stripe = (2 << 20) / mWidth;
  nscoord twipStripe = NSIntPixelsToAppUnits(stripe, p2a);

  nsRect cutout(r);
  cutout.SizeTo(r.width, PR_MIN(r.height, twipStripe));

  PRUint32 roundCount = 0;
  const char* status = "";

  PRUint32 width = NSAppUnitsToIntPixels(cutout.width, p2a);
  PRUint32 height = NSAppUnitsToIntPixels(cutout.height, p2a);

  nsRefPtr<gfxImageSurface> imgSurface =
     new gfxImageSurface(gfxIntSize(width, height),
                         gfxImageSurface::ImageFormatRGB24);
  NS_ENSURE_TRUE(imgSurface, PR_FALSE);

  nsRefPtr<gfxContext> imgContext = new gfxContext(imgSurface);

  nsRefPtr<gfxASurface> surface = 
    gfxPlatform::GetPlatform()->
    CreateOffscreenSurface(gfxIntSize(width, height),
      gfxASurface::ImageFormatRGB24);
  NS_ENSURE_TRUE(surface, PR_FALSE);

  nsRefPtr<gfxContext> context = new gfxContext(surface);
  NS_ENSURE_TRUE(context, PR_FALSE);

  if (r.IsEmpty()) {
    status = "EMPTY";
  } else if (!Prepare(imgSurface)) {
    status = "PNGPREPAREFAILED";
  } else {
    PRBool failed = PR_FALSE;

    while (!cutout.IsEmpty() && !mHadError && !failed) {
      ++roundCount;

      width = NSAppUnitsToIntPixels(cutout.width, p2a);
      height = NSAppUnitsToIntPixels(cutout.height, p2a);

      rv = presShell->RenderDocument(cutout, PR_FALSE, PR_TRUE,
                                     NS_RGB(255, 255, 255), context);
      if (NS_SUCCEEDED(rv)) {
        imgContext->DrawSurface(surface, gfxSize(width, height));
      }

      if (NS_FAILED(rv)) {
        failed = PR_TRUE;
        status = "FAILEDRENDER";
      } else {
        WriteSurface(imgSurface, width, height);
      }
      cutout.MoveBy(0, twipStripe);
      cutout.height = PR_MIN(cutout.height, r.y + r.height - cutout.y);
    }

    rv = Finish();
    status = rv ? "OK" : status[0] != '\0' ? status : "FAILED";
    LOG ("\n");
  }

  LOG ("%d round(s) of height %d, result: %s\n", roundCount, stripe, status);

  return rv;
}

/* PNG Writer */

PNGWriter::PNGWriter(GtkMozEmbed *aEmbed,
		     const char *aFilename)
: Writer(aEmbed, aFilename)
, mFile(NULL)
, mPNG(NULL)
, mInfo(NULL)
, mText(NULL)
, mRow(NULL)
{
  LOG ("PNGWriter ctor\n");
}

PNGWriter::~PNGWriter()
{
  LOG ("PNGWriter dtor\n");

  if (mPNG && mInfo) {
    png_destroy_write_struct (&mPNG, &mInfo);
  }
  if (mText) {
    free (mText);
  }
  if (mRow) {
    free (mRow);
  }
  if (mFile) {
    fclose (mFile);
  }
};

PRBool
PNGWriter::Prepare(gfxImageSurface *aSurface)
{
  if (mInitialised) return PR_TRUE;

  mFile = fopen (mFilename.get(), "wb");
  if (!mFile) return PR_FALSE;

#ifdef PNG_iTXt_SUPPORTED
  PRUint32 n_keys = 3;
#else
  PRUint32 n_keys = 1;
#endif
  mText = (png_textp) malloc (sizeof (png_text) * n_keys);
  if (!mText) return PR_FALSE;

  memset (mText, 0, sizeof (png_text) * n_keys);

  mText[0].compression = PNG_TEXT_COMPRESSION_NONE;
  mText[0].key  = "Software";
  mText[0].text = "GNOME Web Photo " VERSION;
  mText[0].text_length = strlen (mText[0].text);

#ifdef PNG_iTXt_SUPPORTED
  mText[0].lang = NULL;
  mText[0].lang_key = NULL;

  /* These chunks need UTF-8 text, so we can only write them with iTXt support */
  mText[1].compression = PNG_ITXT_COMPRESSION_NONE;
  mText[1].key  = "Title";
  mText[1].text = mTitle.BeginWriting();
  mText[1].text_length = 0; /* iTXt */
  mText[1].itxt_length = strlen (mText[1].text);
  mText[1].lang = NULL;
  mText[1].lang_key = NULL;
  
  mText[2].compression = PNG_ITXT_COMPRESSION_NONE;
  mText[2].key  = "URL";
  mText[2].text = mSpec.BeginWriting();
  mText[2].text_length = 0; /* iTXt */
  mText[2].itxt_length = strlen (mText[2].text);
  mText[2].lang = NULL;
  mText[2].lang_key = NULL;
#endif /* PNG_iTXt_SUPPORTED */

	/*  The keywords that are given in the PNG Specification are:
	
	Title           Short (one line) mTitle or
			caption for image
	Author          Name of image's creator
	Description     Description of image (possibly long)
	Copyright       Copyright notice
	Creation Time   Time of original image creation
			(usually RFC 1123 format, see below)
	Software        Software used to create the image
	Disclaimer      Legal disclaimer
	Warning         Warning of nature of content
	Source          Device used to create the image
	Comment         Miscellaneous comment; conversion
			from other image format
	*/

  mPNG = png_create_write_struct (PNG_LIBPNG_VER_STRING,
                                  this,
                                  PNGWriter::ErrorCallback,
				  PNGWriter::WarnCallback);
  if (!mPNG || mHadError) return PR_FALSE;

  /* FIXME: do I need the weird jumpbuf stuff?? */

  mInfo = png_create_info_struct (mPNG);
  if (!mInfo || mHadError) return PR_FALSE;

  if (mText) {
    png_set_text (mPNG, mInfo, mText, n_keys);
    if (mHadError) return PR_FALSE;
  }

  png_init_io (mPNG, mFile);
  if (mHadError) return PR_FALSE;

//FIXME: store gamma?
//    png_set_gAMA(mPNG, mInfo, gamma);

//FIXME store time
//    png_set_tIME(mPNG, mInfo, mod_time);

//    png_set_pHYs(mPNG, mInfo, res_x, res_y,
//                   PNG_RESOLUTION_METER);
// 1000 * 72 / 25.4 = 2834

  /* FIXME: do I need 8, or the format.m[R|G|B]Count here? */
  png_set_IHDR (mPNG, mInfo, mWidth, mHeight, 8 /* bits per sample */ /* FIXME? */,
                PNG_COLOR_TYPE_RGB /* FIXME: alpha? */, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  if (mHadError) return PR_FALSE;

  png_color_8 sig_bit;
  sig_bit.red = 8;
  sig_bit.green = 8;
  sig_bit.blue = 8;
  sig_bit.alpha = 0;
  png_set_sBIT (mPNG, mInfo, &sig_bit);
  if (mHadError) return PR_FALSE;

  png_write_info (mPNG, mInfo);
  if (mHadError) return PR_FALSE;

  png_set_packing (mPNG);
  if (mHadError) return PR_FALSE;

  mRow = (png_bytep) malloc(mWidth*3);
  if (!mRow) return PR_FALSE;

  mInitialised = PR_TRUE;

  return PR_TRUE;
}

void
PNGWriter::WriteSurface(gfxImageSurface *aSurface,
			PRUint32 aWidth,
			PRUint32 aHeight)
{
  long cairoStride = aSurface->Stride();
  unsigned char* cairoData = aSurface->Data();

  for (PRUint32 row = 0; row < aHeight; ++row) {
    PRUint8* dest = mRow;
    for (PRUint32 col = 0; col < aWidth; ++col) {
      PRUint32* cairoPixel = reinterpret_cast<PRUint32*>
                             ((cairoData + row * cairoStride + 4 * col));

      dest[0] = (*cairoPixel >> 16) & 0xFF;
      dest[1] = (*cairoPixel >>  8) & 0xFF;
      dest[2] = (*cairoPixel >>  0) & 0xFF;
      dest += 3;
    }
    png_write_row (mPNG, mRow);
    if (mHadError) break;
  }
}

PRBool
PNGWriter::Finish()
{
  if (mPNG && mInfo) {
    png_write_end (mPNG, mInfo);
  } else {
    mHadError = PR_TRUE;
  }

  return !mHadError;
}

/* static */ void
PNGWriter::WarnCallback(png_structp mPNG,
			png_const_charp aMsg)
{
  LOG ("PNG warning: %s\n", aMsg);
}

/* static */ void
PNGWriter::ErrorCallback(png_structp mPNG,
			 png_const_charp aMsg)
{
  LOG ("PNG mHadError: %s\n", aMsg);

  Writer *writer = (Writer*) png_get_error_ptr (mPNG);
  NS_ENSURE_TRUE (writer, );

  writer->mHadError = PR_TRUE;

  /* FIXME: jumpbuf stuff? */
}

/* PPM Writer */

PPMWriter::PPMWriter(GtkMozEmbed *aEmbed,
                     const char *aFilename)
: Writer(aEmbed, aFilename)
, mFile(NULL)
, mRow(NULL)
{
  LOG ("PPMWriter ctor\n");
}

PPMWriter::~PPMWriter()
{
  LOG ("PPMWriter dtor\n");

  if (mFile) {
    fclose (mFile);
  }
  free (mRow);
};

PRBool
PPMWriter::Prepare(gfxImageSurface *aSurface)
{
  if (mInitialised) return PR_TRUE;

  mFile = fopen (mFilename.get(), "wb");
  if (!mFile) return PR_FALSE;

  fprintf(mFile, "P6 %d %d 255\n", mWidth, mHeight);

  mRow = (unsigned char*) malloc(mWidth*3);
  if (!mRow) return PR_FALSE;
  
  mInitialised = PR_TRUE;

  return PR_TRUE;
}

void
PPMWriter::WriteSurface(gfxImageSurface *aSurface,
                        PRUint32 aWidth,
                        PRUint32 aHeight)
{
  long cairoStride = aSurface->Stride();
  unsigned char* cairoData = aSurface->Data();

  for (PRUint32 row = 0; row < aHeight; ++row) {
    PRUint8* dest = mRow;
    for (PRUint32 col = 0; col < aWidth; ++col) {
      PRUint32* cairoPixel = reinterpret_cast<PRUint32*>
                             ((cairoData + row * cairoStride + 4 * col));

      dest[0] = (*cairoPixel >> 16) & 0xFF;
      dest[1] = (*cairoPixel >>  8) & 0xFF;
      dest[2] = (*cairoPixel >>  0) & 0xFF;
      dest += 3;
    }

    if (fwrite(mRow, 3, mWidth, mFile) != mWidth) {
      mHadError = PR_TRUE;
      break;
    }
  }
}

PRBool
PPMWriter::Finish()
{
  return !mHadError;
}

/* GdkGdkPixData context */

ThumbnailWriter::ThumbnailWriter(GtkMozEmbed *aEmbed,
				 const char *aFilename,
				 PRUint32 aSize)
: Writer(aEmbed, aFilename)
, mSize(aSize)
, mData(NULL)
{
  LOG ("ThumbnailWriter ctor\n");

  mWidth = THUMBNAIL_WIDTH;
  mHeight = THUMBNAIL_HEIGHT;
}

ThumbnailWriter::~ThumbnailWriter()
{
  LOG ("ThumbnailWriter dtor\n");
 
  if (mData) {
    free (mData);
  }
}

PRBool
ThumbnailWriter::Prepare(gfxImageSurface *aSurface)
{
  if (!mInitialised) {
    void *buf = malloc (sizeof (GdkPixdata) + 3 * THUMBNAIL_WIDTH * THUMBNAIL_HEIGHT);
    if (!buf) return PR_FALSE;

    mData = (GdkPixdata *) buf;
    mDest = (PRUint8*) (mData + sizeof (GdkPixdata));
    mData->magic = GDK_PIXBUF_MAGIC_NUMBER;
    mData->length = 0;
    mData->pixdata_type = GDK_PIXDATA_COLOR_TYPE_RGB | GDK_PIXDATA_SAMPLE_WIDTH_8 | GDK_PIXDATA_ENCODING_RAW;
    mData->pixel_data = (guint8*) mDest;
    mData->rowstride = 3 * mWidth;
    mData->width = mWidth;
    mData->height = mHeight;
 
    mInitialised = PR_TRUE;
  }

  return mInitialised;
}  

void
ThumbnailWriter::WriteSurface(gfxImageSurface *aSurface,
			      PRUint32 aWidth,
			      PRUint32 aHeight)
{
  long cairoStride = aSurface->Stride();
  unsigned char* cairoData = aSurface->Data();

  for (PRUint32 row = 0; row < aHeight; ++row) {
    for (PRUint32 col = 0; col < aWidth; ++col) {
      PRUint32* cairoPixel = reinterpret_cast<PRUint32*>
                             ((cairoData + row * cairoStride + 4 * col));

      mDest[0] = (*cairoPixel >> 16) & 0xFF;
      mDest[1] = (*cairoPixel >>  8) & 0xFF;
      mDest[2] = (*cairoPixel >>  0) & 0xFF;
      mDest += 3;
    }
    if (mHadError) break;
  }
}

PRBool
ThumbnailWriter::Finish()
{
  if (mHadError) return PR_FALSE;

  GError *error = NULL;
  GdkPixbuf *image;
  image = gdk_pixbuf_from_pixdata (mData, FALSE, &error);
  if (error) {
    LOG ("ThumbnailWriter error while converting GdkPixData to GtkPixBuf: %s\n", error->message);
    g_error_free (error);
    return PR_FALSE;
  }

  GdkPixbuf *scaled;
  double aspect = ((double) mHeight) / ((double) mWidth);
  int height = (int) (((double) mSize) * aspect + 0.5f);
  scaled = gdk_pixbuf_scale_simple (image, mSize, height, GDK_INTERP_HYPER);
  g_object_unref (image);
  if (!scaled) return PR_FALSE;

  PRBool retval;
  retval = (PRBool) gdk_pixbuf_save (scaled, mFilename.get(), "png", &error, NULL);
  if (error) {
    LOG ("ThumbnailWriter error while wrinting thumbnail: %s\n", error->message);
    g_error_free (error);
    return PR_FALSE;
  }

  g_object_unref (scaled);

  return retval;
}


#ifdef ENABLE_JPEG
/* JPEG Writer */

JPEGWriter::JPEGWriter(GtkMozEmbed *aEmbed,
		       const char *aFilename,
		       PRUint16 aQuality)
: Writer(aEmbed, aFilename)
, mQuality(aQuality)
, mFile(NULL)
{
  LOG ("JPEGWriter ctor\n");
}

JPEGWriter::~JPEGWriter()
{
  LOG ("JPEGWriter dtor\n");

  if (mFile) {
    fclose (mFile);
  }

  jpeg_destroy_compress (&mJPEG);
};

PRBool
JPEGWriter::Prepare(gfxImageSurface *aSurface)
{
  if (mInitialised) return PR_TRUE;

  mRow = (PRUint8*) malloc (3 * mWidth);
  if (!mRow) return PR_FALSE;

  mFile = fopen (mFilename.get(), "wb");
  if (!mFile) return PR_FALSE;

  mJPEG.err = jpeg_std_error (&mHandler);
  jpeg_create_compress (&mJPEG);
  if (mHadError) return PR_FALSE;

  jpeg_stdio_dest (&mJPEG, mFile);
  if (mHadError) return PR_FALSE;

  mJPEG.image_width = mWidth;
  mJPEG.image_height = mHeight;
  mJPEG.input_components = 3;
  mJPEG.in_color_space = JCS_RGB;

  jpeg_set_defaults (&mJPEG);
  if (mHadError) return PR_FALSE;

  jpeg_set_quality (&mJPEG, mQuality, TRUE /* limit to baseline-JPEG values */);
  if (mHadError) return PR_FALSE;

  jpeg_start_compress (&mJPEG, TRUE);
  if (mHadError) return PR_FALSE;

#if 0
  nsPixelFormat format;
  aSurface->GetPixelFormat(&format);
  /* FIXME: assert that subsequent surfaces have same format? */
#endif

  mInitialised = PR_TRUE;

  return PR_TRUE;
}

void
JPEGWriter::WriteSurface(gfxImageSurface *aSurface,
			 PRUint32 aWidth,
			 PRUint32 aHeight)
{
  long cairoStride = aSurface->Stride();
  unsigned char* cairoData = aSurface->Data();

  for (PRUint32 row = 0; row < aHeight; ++row) {
    PRUint8* dest = mRow;
    for (PRUint32 col = 0; col < aWidth; ++col) {
      PRUint32* cairoPixel = reinterpret_cast<PRUint32*>
                             ((cairoData + row * cairoStride + 4 * col));

      dest[0] = (*cairoPixel >> 16) & 0xFF;
      dest[1] = (*cairoPixel >>  8) & 0xFF;
      dest[2] = (*cairoPixel >>  0) & 0xFF;
      dest += 3;
    }

    JSAMPROW rows[1] = { (JSAMPLE*) mRow };
    (void) jpeg_write_scanlines (&mJPEG, rows, 1);
    if (mHadError) break;
  }
}

PRBool
JPEGWriter::Finish()
{
  if (mHadError) return PR_FALSE;

  jpeg_finish_compress (&mJPEG);
  return !mHadError;
}

#endif /* ENABLE_JPEG */
