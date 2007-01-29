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
#include <nsIPresShell.h>
#include <nsPresContext.h>
#include <nsIView.h>
#include <nsIViewManager.h>
#include <nsIScrollableView.h>
#include <nsIRenderingContext.h>
#include <nsIDrawingSurface.h>
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

  PRBool retval = PR_FALSE;

  nsresult rv;
  nsCOMPtr<nsIWebBrowser> browser;
  gtk_moz_embed_get_nsIWebBrowser (mEmbed, getter_AddRefs (browser));
  NS_ENSURE_TRUE (browser, retval);

  nsCOMPtr<nsIDOMWindow> domWin;
  rv = browser->GetContentDOMWindow (getter_AddRefs (domWin));
  NS_ENSURE_SUCCESS (rv, retval);

  nsCOMPtr<nsIDOMDocument> domDoc;
  rv = domWin->GetDocument (getter_AddRefs (domDoc));
  NS_ENSURE_SUCCESS (rv, retval);

  nsCOMPtr<nsIDocument> doc (do_QueryInterface (domDoc, &rv));
  NS_ENSURE_SUCCESS (rv, retval);

  nsCOMPtr<nsISupports> docShellAsISupports (doc->GetContainer ());
  nsCOMPtr<nsIDocShell> docShell (do_QueryInterface (docShellAsISupports, &rv));
  NS_ENSURE_SUCCESS (rv, retval);

  nsCOMPtr<nsIPresShell> presShell;
  rv = docShell->GetPresShell (getter_AddRefs (presShell));
  NS_ENSURE_SUCCESS (rv, retval);

  nsCOMPtr<nsPresContext> presContext;
  rv = docShell->GetPresContext (getter_AddRefs (presContext));
  NS_ENSURE_SUCCESS (rv, retval);

  rv = NS_ERROR_NULL_POINTER;
  nsIViewManager *viewManager = presShell->GetViewManager ();
  NS_ENSURE_TRUE (viewManager, retval);

  /* Get document information */
  CopyUTF16toUTF8 (doc->GetDocumentTitle(), mTitle);
  mTitle.CompressWhitespace(PR_TRUE, PR_TRUE);

  nsIURI *uri = doc->GetDocumentURI();
  if (uri) {
    uri->GetSpec(mSpec);
  }

  doc->FlushPendingNotifications (Flush_Display);

  nsIScrollableView* scrollableView = nsnull;
  viewManager->GetRootScrollableView (&scrollableView);
  nsIView* view;
  if (scrollableView) {
    scrollableView->GetScrolledView (view);
  } else {
    viewManager->GetRootView(view);
  }

  /* Get conversion factors */
  float p2t = presContext->PixelsToTwips();
  float t2p = presContext->TwipsToPixels();

  /* Limit the bitmap size */
  nscoord twipLimitW = NSIntPixelsToTwips(mWidth, p2t);
  nscoord twipLimitH = NSIntPixelsToTwips(mHeight, p2t);

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

  mWidth = NSTwipsToIntPixels(r.width, t2p);
  mHeight = NSTwipsToIntPixels(r.height, t2p);

  PRUint32 stripe = (2 << 20) / mWidth;
  nscoord twipStripe = NSIntPixelsToTwips(stripe, p2t);

  nsRect cutout(r);
  cutout.SizeTo(r.width, PR_MIN(r.height, twipStripe));

  PRUint32 roundCount = 0;
  const char* status = "";

  if (r.IsEmpty()) {
    status = "EMPTY";
  } else {
    PRBool failed = PR_FALSE;
    while (!cutout.IsEmpty() && !mHadError && !failed) {
      ++roundCount;

      PRUint32 width = NSTwipsToIntPixels(cutout.width, t2p);
      PRUint32 height = NSTwipsToIntPixels(cutout.height, t2p);

      nsCOMPtr<nsIRenderingContext> context;
      rv = viewManager->RenderOffscreen (view, cutout,
                                         PR_FALSE, PR_TRUE, NS_RGB(255, 255, 255),
                                         getter_AddRefs(context));
      if (NS_FAILED(rv)) {
        failed = PR_TRUE;
        status = "FAILEDRENDER";
      } else {
        nsIDrawingSurface* surface;
        context->GetDrawingSurface(&surface);
        if (!surface) {
          failed = PR_TRUE;
          status = "NOSURFACE";
        } else {
          PRUint32 w, h;
          surface->GetDimensions(&w,&h);
          /* these are computed from ScaleRoundOut(t2p) on cutout.Bounds(), and
           * may be one pixel too wide and/or hight
           */

          if (width > w || height > h) {
            failed = PR_TRUE;
            status = "SIZEMISMATCH";
          } else if (!Prepare(surface)) {
            failed = PR_TRUE;
            status = "PNGPREPAREFAILED";
          } else {
            PRUint8* data;
            PRInt32 rowLen, rowSpan;
            rv = surface->Lock(0, 0, w, h, (void**)&data, &rowSpan, &rowLen,
                               NS_LOCK_SURFACE_READ_ONLY);
            if (NS_FAILED(rv)) {
	      failed = PR_TRUE;
              status = "FAILEDLOCK";
            } else {
              LOG (".");
              WriteSurface(surface, width, height, data, rowLen, rowSpan, rowLen/w);
              surface->Unlock();
            }
          }
        }
        context->DestroyDrawingSurface(surface);
      }
      cutout.MoveBy(0, twipStripe);
      cutout.IntersectRect(r, cutout);
    }

    retval = Finish();
    status = retval ? "OK" : status[0] != '\0' ? status : "FAILED";
    LOG ("\n");
  }

  LOG ("%d round(s) of height %d, result: %s\n", roundCount, stripe, status);

  return retval;
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
PNGWriter::Prepare(nsIDrawingSurface *aSurface)
{
  if (mInitialised) return PR_TRUE;

  mFile = fopen (mFilename.get(), "wb");
  if (!mFile) return PR_FALSE;

  PRUint32 n_keys = 3;
  mText = (png_textp) malloc (sizeof (png_text) * n_keys);
  if (!mText) return PR_FALSE;

  mText[0].compression = PNG_TEXT_COMPRESSION_NONE;
  mText[0].key  = "Title";
  mText[0].text = mTitle.BeginWriting();
  mText[0].text_length = strlen (mText[0].text);
  mText[1].compression = PNG_TEXT_COMPRESSION_NONE;
  mText[1].key  = "URL";
  mText[1].text = mSpec.BeginWriting();
  mText[1].text_length = strlen (mText[1].text);
  mText[2].compression = PNG_TEXT_COMPRESSION_NONE;
  mText[2].key  = "Creation Time";
  mText[2].text = "FIXME";
  mText[2].text_length = strlen (mText[2].text);
 
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

  nsPixelFormat format;
  aSurface->GetPixelFormat(&format);
  /* FIXME: assert that subsequent surfaces have same format? */

  /* FIXME: do I need 8, or the format.m[R|G|B]Count here? */
  png_set_IHDR (mPNG, mInfo, mWidth, mHeight, 8 /* bits per sample */ /* FIXME? */,
                PNG_COLOR_TYPE_RGB /* FIXME: alpha? */, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  if (mHadError) return PR_FALSE;

  png_color_8 sig_bit;
  sig_bit.red = format.mRedCount;
  sig_bit.green = format.mGreenCount;
  sig_bit.blue = format.mBlueCount;
  sig_bit.alpha = format.mAlphaCount;
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
PNGWriter::WriteSurface(nsIDrawingSurface *aSurface,
			PRUint32 aWidth,
			PRUint32 aHeight,
			PRUint8 *aData,
			PRInt32 aRowLen,
			PRInt32 aRowSpan,
                        PRInt32 aPixelSpan)
{
  nsPixelFormat format;
  aSurface->GetPixelFormat(&format);

  PRUint8* buf = (PRUint8*) mRow;
  for (PRUint32 i = 0; i < aHeight; ++i)
    {
      PRUint8* src = aData + i*aRowSpan;

      PRUint8* dest = buf;
      for (PRUint32 j = 0; j < aWidth; ++j)
        {
          /* v is the pixel value */
#ifdef WORDS_BIGENDIAN
          PRUint32 v = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
          v >>= (32 - 8*aPixelSpan);
//maybe like this:  PRUint32 v = *((PRUint32*) src) >> (32 - 8*bytesPerPix);
#else
          PRUint32 v = *((PRUint32*) src);
#endif
	  dest[0] = ((v & format.mRedMask) >> format.mRedShift) << (8 - format.mRedCount);
	  dest[1] = ((v & format.mGreenMask) >> format.mGreenShift) << (8 - format.mGreenCount);
	  dest[2] = ((v & format.mBlueMask) >> format.mBlueShift) << (8 - format.mBlueCount);
          src += aPixelSpan;
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
PPMWriter::Prepare(nsIDrawingSurface *aSurface)
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
PPMWriter::WriteSurface(nsIDrawingSurface *aSurface,
                        PRUint32 aWidth,
                        PRUint32 aHeight,
                        PRUint8 *aData,
                        PRInt32 aRowLen,
                        PRInt32 aRowSpan,
                        PRInt32 aPixelSpan)
{
  nsPixelFormat format;
  aSurface->GetPixelFormat(&format);

  PRUint8* buf = (PRUint8*) mRow;
  for (PRUint32 i = 0; i < aHeight; ++i)
    {
      PRUint8* src = aData + i*aRowSpan;

      PRUint8* dest = mRow;
      for (PRUint32 j = 0; j < aWidth; ++j)
        {
          /* v is the pixel value */
#ifdef WORDS_BIGENDIAN
          PRUint32 v = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
          v >>= (32 - 8*aPixelSpan);
//maybe like this:  PRUint32 v = *((PRUint32*) src) >> (32 - 8*bytesPerPix);
#else
          PRUint32 v = *((PRUint32*) src);
#endif
          dest[0] = ((v & format.mRedMask) >> format.mRedShift) << (8 - format.mRedCount);
          dest[1] = ((v & format.mGreenMask) >> format.mGreenShift) << (8 - format.mGreenCount);
          dest[2] = ((v & format.mBlueMask) >> format.mBlueShift) << (8 - format.mBlueCount);
          src += aPixelSpan;
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
ThumbnailWriter::Prepare(nsIDrawingSurface *aSurface)
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
ThumbnailWriter::WriteSurface(nsIDrawingSurface *aSurface,
			      PRUint32 aWidth,
			      PRUint32 aHeight,
			      PRUint8 *aData,
			      PRInt32 aRowLen,
			      PRInt32 aRowSpan,
                              PRInt32 aPixelSpan)
{
  nsPixelFormat format;
  aSurface->GetPixelFormat(&format);

  for (PRUint32 i = 0; i < aHeight; ++i)
    {
      PRUint8* src = aData + i*aRowSpan;
      for (PRUint32 j = 0; j < aWidth; ++j)
        {
          /* v is the pixel value */
#ifdef WORDS_BIGENDIAN
          PRUint32 v = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
          v >>= (32 - 8*aPixelSpan);
#else
          PRUint32 v = *((PRUint32*) src);
#endif
	  mDest[0] = ((v & format.mRedMask) >> format.mRedShift) << (8 - format.mRedCount);
	  mDest[1] = ((v & format.mGreenMask) >> format.mGreenShift) << (8 - format.mGreenCount);
	  mDest[2] = ((v & format.mBlueMask) >> format.mBlueShift) << (8 - format.mBlueCount);
          src += aPixelSpan;
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
JPEGWriter::Prepare(nsIDrawingSurface *aSurface)
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
JPEGWriter::WriteSurface(nsIDrawingSurface *aSurface,
			 PRUint32 aWidth,
			 PRUint32 aHeight,
			 PRUint8 *aData,
			 PRInt32 aRowLen,
		         PRInt32 aRowSpan,
			 PRInt32 aPixelSpan)
{
  nsPixelFormat format;
  aSurface->GetPixelFormat(&format);

  PRUint8* buf = (PRUint8*) mRow;
  for (PRUint32 i = 0; i < aHeight; ++i)
    {
      PRUint8* src = aData + i*aRowSpan;

      PRUint8* dest = buf;
      for (PRUint32 j = 0; j < aWidth; ++j)
        {
          /* v is the pixel value */
#ifdef WORDS_BIGENDIAN
          PRUint32 v = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | src[3];
          v >>= (32 - 8*aPixelSpan);
//maybe like this:  PRUint32 v = *((PRUint32*) src) >> (32 - 8*bytesPerPix);
#else
          PRUint32 v = *((PRUint32*) src);
#endif
	  dest[0] = ((v & format.mRedMask) >> format.mRedShift) << (8 - format.mRedCount);
	  dest[1] = ((v & format.mGreenMask) >> format.mGreenShift) << (8 - format.mGreenCount);
	  dest[2] = ((v & format.mBlueMask) >> format.mBlueShift) << (8 - format.mBlueCount);
          src += aPixelSpan;
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
