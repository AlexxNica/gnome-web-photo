/*
 *  Copyright (C) 2005 Christian Persch
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is GNOME Web Photo code.
 *
 * The Initial Developer of the Original Code is
 * Christian Persch <chpe@gnome.org>.
 * Portions created by the Initial Developer are Copyright (C) 2004, 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK *****
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
                  const char *aFilename)
: mEmbed(aEmbed)
{
  NS_CopyNativeToUnicode (nsDependentCString(aFilename), mFilename);
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
#if 0
	aSettings->SetPrintRange (nsIPrintSettings::kRangeSpecifiedPageRange);
	aSettings->SetStartPageRange (1);
	aSettings->SetEndPageRange (9999);
#endif
	aSettings->SetPrintRange (nsIPrintSettings::kRangeAllPages);
	aSettings->SetPaperSize (nsIPrintSettings::kPaperSizeDefined);
	aSettings->SetPaperSizeUnit (nsIPrintSettings::kPaperSizeMillimeters);
	aSettings->SetPaperWidth (210.0);
	aSettings->SetPaperHeight (297.0);
	aSettings->SetPaperName (NS_LITERAL_STRING("a4").get());
	// aSettings->SetOrientation (nsIPrintSettings::kLandscapeOrientation);
	aSettings->SetOrientation (nsIPrintSettings::kPortraitOrientation);
	aSettings->SetPrintInColor (PR_TRUE);
	aSettings->SetPrintBGColors (PR_FALSE);
	aSettings->SetPrintBGImages (PR_FALSE);
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
  LOG ("(flags %x, status %x) ", aStateFlags, aStatus);
  if (aStateFlags & STATE_STOP) {
    LOG ("\n");
    g_idle_add ((GSourceFunc) gtk_widget_destroy,
                gtk_widget_get_toplevel (GTK_WIDGET (mEmbed)));
  }

  return NS_OK;
}

/* void onProgressChange (in nsIWebProgress aWebProgress, in nsIRequest aRequest, in long aCurSelfProgress, in long aMaxSelfProgress, in long aCurTotalProgress, in long aMaxTotalProgress); */
NS_IMETHODIMP
Printer::OnProgressChange(nsIWebProgress *aWebProgress, nsIRequest *aRequest, PRInt32 aCurSelfProgress, PRInt32 aMaxSelfProgress, PRInt32 aCurTotalProgress, PRInt32 aMaxTotalProgress)
{
  LOG ("[%d/%d, %d/%d] ", aCurSelfProgress, aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress);
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
