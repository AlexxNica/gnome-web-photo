/*
 *  Copyright © 2005 Christian Persch
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
 *  $Id$
 */

#include "mozilla-config.h"
#include "config.h"

#include "Printer.h"

#include <nsIWebBrowser.h>
#include <nsIPrintSettings.h>
#include <nsIPrintSettingsService.h>
#include <nsIServiceManager.h>
#include <nsIInterfaceRequestorUtils.h>
#include <nsNativeCharsetUtils.h>
#include <gtkmozembed_internal.h>

#ifdef GNOME_ENABLE_DEBUG
#define LOG g_print
#else
#define LOG //
#endif

Printer::Printer (GtkMozEmbed *aEmbed,
                  const char *aFilename,
		  PRBool aPrintBG)
: mEmbed(aEmbed)
, mPrintBG(aPrintBG)
{
  if (aFilename[0] == '/') {
    NS_CopyNativeToUnicode (nsDependentCString(aFilename), mFilename);
  } else {
    char *path = g_build_filename (g_get_current_dir(), aFilename, NULL);
    NS_CopyNativeToUnicode (nsDependentCString(path), mFilename);
    g_free(path);
  }
}

Printer::~Printer()
{
}

nsresult
Printer::Print()
{
  NS_ENSURE_STATE (mEmbed);

  nsresult rv = NS_ERROR_FAILURE;
  nsCOMPtr<nsIWebBrowser> browser;
  gtk_moz_embed_get_nsIWebBrowser (mEmbed, getter_AddRefs (browser));
  NS_ENSURE_TRUE (browser, rv);

  nsCOMPtr<nsIWebBrowserPrint> print (do_GetInterface (browser, &rv));
  NS_ENSURE_SUCCESS (rv, rv);

  nsCOMPtr<nsIPrintSettingsService> pss (do_GetService("@mozilla.org/gfx/printsettings-service;1", &rv));
  NS_ENSURE_SUCCESS(rv, rv);
  
  nsCOMPtr<nsIPrintSettings> settings;
  rv = pss->GetNewPrintSettings(getter_AddRefs (settings));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = pss->InitPrintSettingsFromPrinter (NS_LITERAL_STRING("PostScript/default").get(), settings);
  NS_ENSURE_SUCCESS(rv, rv);

#if 0
  nsCOMPtr<nsIPrintSettings> settings;
  rv = print->GetGlobalPrintSettings (getter_AddRefs (settings));
  NS_ENSURE_SUCCESS (rv, rv);
#endif

  rv = SetSettings (settings);
  NS_ENSURE_SUCCESS (rv, rv);

  //return print->Print (settings, this);
  rv = print->Print(settings, this);
  g_return_val_if_fail (NS_SUCCEEDED (rv), rv);

  return rv;
}

nsresult
Printer::SetSettings(nsIPrintSettings *aSettings)
{
	aSettings->SetPrinterName (NS_LITERAL_STRING("PostScript/default").get());
	aSettings->SetPrintRange (nsIPrintSettings::kRangeAllPages);
	aSettings->SetPaperSize (nsIPrintSettings::kPaperSizeDefined);
	aSettings->SetPaperSizeUnit (nsIPrintSettings::kPaperSizeMillimeters);
	aSettings->SetPaperWidth (210.0);
	aSettings->SetPaperHeight (297.0);
	aSettings->SetPaperName (NS_LITERAL_STRING("a4").get());
	// aSettings->SetOrientation (nsIPrintSettings::kLandscapeOrientation);
	aSettings->SetOrientation (nsIPrintSettings::kPortraitOrientation);
	aSettings->SetPrintBGColors (mPrintBG);
	aSettings->SetPrintBGImages (mPrintBG);
	// SetStartPageRange
	// SetEndPageRange
	// SetHeaderStrLeft
	// SetHeaderStrCenter
	// SetHeaderStrRight
	// SetFooterStrLeft
	// SetFooterStrCenter
	// SetFooterStrRight
	// SetShrinkToFit
	// SetPlexName
	// SetColorspace
	// SetResolutionName
	// SetDownloadFonts
	// SetPrintReversed
	aSettings->SetPrintInColor (PR_TRUE);
	// SetNumCopies
	aSettings->SetPrintFrameType (nsIPrintSettings::kFramesAsIs);
	aSettings->SetPrintToFile (PR_TRUE);
	aSettings->SetToFileName (mFilename.get());
	aSettings->SetPrintSilent (PR_TRUE);
	aSettings->SetShowPrintProgress (PR_FALSE);

	return NS_OK;
}

NS_IMPL_ISUPPORTS2 (Printer, nsIWebProgressListener, nsIPrintProgressParams)
	
/* void onStateChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aStateFlags, in nsresult aStatus); */
NS_IMETHODIMP
Printer::OnStateChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aStateFlags, nsresult aStatus)
{
  if (aStateFlags & STATE_STOP) {
    g_signal_emit_by_name (mEmbed, "print-done", NS_SUCCEEDED (aStatus));
  }

  return NS_OK;
}

/* void onProgressChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long aCurSelfProgress, in long aMaxSelfProgress, in long aCurTotalProgress, in long aMaxTotalProgress); */
NS_IMETHODIMP
Printer::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
  return NS_OK;
}

/* void onLocationChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsIURI aLocation); */
NS_IMETHODIMP
Printer::OnLocationChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsIURI *aLocation)
{
  return NS_OK;
}

/* void onStatusChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in nsresult aStatus, in wstring aMessage); */
NS_IMETHODIMP
Printer::OnStatusChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, nsresult aStatus, const PRUnichar *aMessage)
{
  return NS_OK;
}

/* void onSecurityChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in unsigned long aState); */
NS_IMETHODIMP
Printer::OnSecurityChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRUint32 aState)
{
  return NS_OK;
}

/* attribute wstring docTitle; */
NS_IMETHODIMP
Printer::GetDocTitle(PRUnichar * *aDocTitle)
{
  return NS_OK;
}
NS_IMETHODIMP Printer::SetDocTitle(const PRUnichar * aDocTitle)
{
  return NS_OK;
}

/* attribute wstring docURL; */
NS_IMETHODIMP
Printer::GetDocURL(PRUnichar * *aDocURL)
{
  return NS_OK;
}
NS_IMETHODIMP
Printer::SetDocURL(const PRUnichar * aDocURL)
{
  return NS_OK;
}
